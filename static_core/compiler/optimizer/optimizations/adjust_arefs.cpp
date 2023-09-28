/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "adjust_arefs.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace panda::compiler {
AdjustRefs::AdjustRefs(Graph *graph)
    : Optimization {graph},
      defs_ {graph->GetLocalAllocator()->Adapter()},
      workset_ {graph->GetLocalAllocator()->Adapter()},
      heads_ {graph->GetLocalAllocator()->Adapter()},
      insts_to_replace_ {graph->GetLocalAllocator()->Adapter()}
{
}

static bool IsRefAdjustable(const Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::StoreArray:
            return !inst->CastToStoreArray()->GetNeedBarrier();
        case Opcode::LoadArray:
            return !inst->CastToLoadArray()->GetNeedBarrier() && !inst->CastToLoadArray()->IsString();
        default:
            break;
    }

    return false;
}

bool AdjustRefs::RunImpl()
{
    auto def_marker = GetGraph()->NewMarker();
    for (const auto &bb : GetGraph()->GetBlocksRPO()) {
        if (bb->GetLoop()->IsRoot()) {
            continue;
        }

        for (auto inst : bb->Insts()) {
            if (IsRefAdjustable(inst)) {
                auto def = inst->GetInput(0).GetInst();
                if (!def->SetMarker(def_marker)) {
                    defs_.push_back(def);
                }
            }
        }
    }
    GetGraph()->EraseMarker(def_marker);
    for (auto def : defs_) {
        workset_.clear();
        auto marker_holder = MarkerHolder(GetGraph());
        workset_marker_ = marker_holder.GetMarker();
        for (auto &user : def->GetUsers()) {
            auto i = user.GetInst();
            if (!IsRefAdjustable(i) || i->GetBasicBlock()->GetLoop()->IsRoot()) {
                continue;
            }
            workset_.push_back(i);
            i->SetMarker(workset_marker_);
        }
        ProcessArrayUses();
    }
    // We make second pass, because some LoadArrays and StoreArrays can be removed
    for (const auto &bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (IsRefAdjustable(inst)) {
                ProcessIndex(inst);
            }
        }
    }

    return added_;
}

void AdjustRefs::ProcessArrayUses()
{
    ASSERT(heads_.empty());
    auto entered_holder = MarkerHolder(GetGraph());
    block_entered_ = entered_holder.GetMarker();
    GetHeads();
    while (!heads_.empty()) {
        auto head = heads_.back();
        heads_.pop_back();
        ASSERT(IsRefAdjustable(head));
        ASSERT(head->IsMarked(workset_marker_));
        ASSERT(head->GetBasicBlock() != nullptr);
        loop_ = head->GetBasicBlock()->GetLoop();
        auto processed_holder = MarkerHolder(GetGraph());
        block_processed_ = processed_holder.GetMarker();
        insts_to_replace_.clear();
        ASSERT(!head->GetBasicBlock()->IsMarked(block_processed_));
        WalkChainDown(head->GetBasicBlock(), head, head);
        if (insts_to_replace_.size() > 1) {
            ProcessChain(head);
        }
    }
}

/* Create the list of "heads" - the instructions that are not dominated by any other
 * instruction in the workset, or have a potential GC trigger after the dominating
 * instruction and thus cannot be merged with it. In both cases "head" can potentially be
 * the first instruction in a chain. */
void AdjustRefs::GetHeads()
{
    for (const auto i : workset_) {
        auto comp = [i](const Inst *i1) { return i1->IsDominate(i) && i != i1; };
        if (workset_.end() == std::find_if(workset_.begin(), workset_.end(), comp)) {
            heads_.push_back(i);
            i->GetBasicBlock()->SetMarker(block_entered_);
        }
    }
}

/* Add instructions which can be merged with head to insts_to_replace_
 * Instructions which are visited but cannot be merged are added to heads_ */
void AdjustRefs::WalkChainDown(BasicBlock *bb, Inst *start_from, Inst *head)
{
    bb->SetMarker(block_entered_);
    for (auto cur = start_from; cur != nullptr; cur = cur->GetNext()) {
        /* potential switch to VM, the chain breaks here */
        if (cur->IsRuntimeCall() || cur->GetOpcode() == Opcode::SafePoint) {
            head = nullptr;
        } else if (cur->IsMarked(workset_marker_)) {
            if (head == nullptr) {
                heads_.push_back(cur);
                return;
            }
            ASSERT(head->IsDominate(cur));
            insts_to_replace_.push_back(cur);
        }
    }
    if (head != nullptr) {
        bb->SetMarker(block_processed_);
    }
    for (auto succ : bb->GetSuccsBlocks()) {
        if (succ->GetLoop() != loop_ || succ->IsMarked(block_entered_)) {
            continue;
        }
        auto all_preds_visited = true;
        if (head != nullptr) {
            for (auto pred : succ->GetPredsBlocks()) {
                if (!pred->IsMarked(block_processed_)) {
                    all_preds_visited = false;
                    break;
                }
            }
        }
        // If all predecessors of succ were walked with the current value of head,
        // we can be sure that there are no SafePoints or runtime calls
        // on any path from block with head to succ
        if (all_preds_visited) {
            WalkChainDown(succ, succ->GetFirstInst(), head);
        }
    }
}

void AdjustRefs::ProcessChain(Inst *head)
{
    Inst *def = head->GetInput(0).GetInst();
    auto off = GetGraph()->GetRuntime()->GetArrayDataOffset(GetGraph()->GetArch());
    auto arr_data = InsertPointerArithmetic(def, off, head, def->GetPc(), true);
    ASSERT(arr_data != nullptr);

    for (auto inst : insts_to_replace_) {
        auto scale = DataType::ShiftByType(inst->GetType(), GetGraph()->GetArch());
        InsertMem(inst, arr_data, inst->GetInput(1).GetInst(), scale);
    }

    added_ = true;
}

Inst *AdjustRefs::InsertPointerArithmetic(Inst *input, uint64_t imm, Inst *insert_before, uint32_t pc, bool is_add)
{
    uint32_t size = DataType::GetTypeSize(DataType::POINTER, GetGraph()->GetArch());
    if (!GetGraph()->GetEncoder()->CanEncodeImmAddSubCmp(imm, size, false)) {
        return nullptr;
    }
    Inst *new_inst;
    if (is_add) {
        new_inst = GetGraph()->CreateInstAddI(DataType::POINTER, pc, imm);
    } else {
        new_inst = GetGraph()->CreateInstSubI(DataType::POINTER, pc, imm);
    }
    new_inst->SetInput(0, input);
    insert_before->InsertBefore(new_inst);
    return new_inst;
}

void AdjustRefs::InsertMem(Inst *org, Inst *base, Inst *index, uint8_t scale)
{
    Inst *ldst = nullptr;

    ASSERT(base->IsDominate(org));

    if (org->IsStore()) {
        constexpr auto VALUE_IDX = 2;
        ldst = GetGraph()->CreateInst(Opcode::Store);
        ldst->SetInput(VALUE_IDX, org->GetInput(VALUE_IDX).GetInst());
        ldst->CastToStore()->SetScale(scale);
    } else if (org->IsLoad()) {
        ldst = GetGraph()->CreateInst(Opcode::Load);
        ldst->CastToLoad()->SetScale(scale);
    } else {
        UNREACHABLE();
    }
    ldst->SetInput(0, base);
    ldst->SetInput(1, index);
    ldst->SetType(org->GetType());
    org->ReplaceUsers(ldst);
    org->RemoveInputs();
    org->GetBasicBlock()->ReplaceInst(org, ldst);
}

// from
//   3.i32 AddI v2, 0xN -> v4
//   4.i64 LoadArray v1, v3 -> ....
// to
//   5.ptr AddI v1, 0x10 + (0xN << 3) -> v6
//   6.i64 Load v5, v2 -> ....
//
void AdjustRefs::ProcessIndex(Inst *mem)
{
    Inst *index = mem->GetInput(1).GetInst();
    bool is_add;
    uint64_t imm;
    if (index->GetOpcode() == Opcode::AddI) {
        is_add = true;
        imm = index->CastToAddI()->GetImm();
    } else if (index->GetOpcode() == Opcode::SubI) {
        is_add = false;
        imm = index->CastToSubI()->GetImm();
    } else {
        return;
    }
    auto scale = DataType::ShiftByType(mem->GetType(), GetGraph()->GetArch());
    uint64_t off = GetGraph()->GetRuntime()->GetArrayDataOffset(GetGraph()->GetArch());
    Inst *base = mem->GetInput(0).GetInst();

    Inst *new_base;
    if (!is_add) {
        if (off > (imm << scale)) {
            uint64_t new_off = off - (imm << scale);
            new_base = InsertPointerArithmetic(base, new_off, mem, mem->GetPc(), true);
        } else if (off < (imm << scale)) {
            uint64_t new_off = (imm << scale) - off;
            new_base = InsertPointerArithmetic(base, new_off, mem, mem->GetPc(), false);
        } else {
            ASSERT(off == (imm << scale));
            new_base = base;
        }
    } else {
        uint64_t new_off = off + (imm << scale);
        new_base = InsertPointerArithmetic(base, new_off, mem, mem->GetPc(), true);
    }
    if (new_base == nullptr) {
        return;
    }
    InsertMem(mem, new_base, index->GetInput(0).GetInst(), scale);

    added_ = true;
}

}  // namespace panda::compiler
