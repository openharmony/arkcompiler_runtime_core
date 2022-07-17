/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
      chains_ {graph->GetLocalAllocator()->Adapter()}
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
    for (const auto &bb : GetGraph()->GetBlocksRPO()) {
        if (bb->GetLoop()->IsRoot()) {
            continue;
        }

        for (auto inst : bb->Insts()) {
            if (IsRefAdjustable(inst)) {
                if (defs_.find(inst->GetInput(0).GetInst()) != defs_.end()) {
                    continue;
                }
                defs_.insert(inst->GetInput(0).GetInst());
            }
        }
    }

    for (auto def : defs_) {
        workset_.clear();
        for (auto &user : def->GetUsers()) {
            auto i = user.GetInst();
            if (!IsRefAdjustable(i) || i->GetBasicBlock()->GetLoop()->IsRoot()) {
                continue;
            }
            workset_.insert(i);
        }
        ProcessArrayUses();
    }

    return added_;
}

void AdjustRefs::ProcessArrayUses()
{
    chains_.clear();
    for (auto inst : GetHeads()) {
        ASSERT(IsRefAdjustable(inst));
        ASSERT(workset_.find(inst) != workset_.end());
        loop_ = inst->GetBasicBlock()->GetLoop();
        WalkChainDown(inst, nullptr);
    }
}

/* Create the list of "heads" - the instructions that are
 * not dominated by any other instruction in the workset */
ArenaVector<Inst *> AdjustRefs::GetHeads()
{
    ArenaVector<Inst *> heads(GetGraph()->GetLocalAllocator()->Adapter());
    for (const auto i : workset_) {
        auto comp = [i](const Inst *i1) { return i1->IsDominate(i) && i != i1; };
        if (workset_.end() == std::find_if(workset_.begin(), workset_.end(), comp)) {
            heads.emplace_back(i);
        }
    }
    return heads;
}

void AdjustRefs::WalkChainDown(Inst *inst, Inst *head)
{
    if (inst == nullptr || (inst->GetBasicBlock()->GetLoop() != loop_)) {
        return;
    }

    auto cur = inst;
    for (; cur != nullptr; cur = cur->GetNext()) {
        /* potential switch to VM, the chain breaks here */
        if (cur->RequireState() || cur->GetOpcode() == Opcode::SafePoint) {
            head = nullptr;
        } else if (IsRefAdjustable(cur) && workset_.find(cur) != workset_.end()) {
            if (head == nullptr) {
                head = cur;
            } else {
                ASSERT(head->IsDominate(cur));
            }
            chains_.emplace(head, cur);
            break;
        }
    }
    if (cur == nullptr) {
        auto bb = inst->GetBasicBlock();
        for (auto succ : bb->GetDominatedBlocks()) {
            WalkChainDown(succ->GetFirstInst(), head);
        }
    } else {
        WalkChainDown(cur->GetNext(), head);
    }

    if (cur == head && chains_.count(head) > 1) {
        ProcessChain(head);
    }
}

void AdjustRefs::ProcessChain(Inst *head)
{
    Inst *def = head->GetInput(0).GetInst();
    auto *bb = head->GetBasicBlock();
    auto arr_data = GetGraph()->CreateInst(Opcode::AddI);
    arr_data->SetPc(def->GetPc());
    arr_data->SetInput(0, def);
    auto off = GetGraph()->GetRuntime()->GetArrayDataOffset(GetGraph()->GetArch());
    arr_data->CastToAddI()->SetImm(off);
    arr_data->SetType(DataType::POINTER);
    bb->InsertBefore(arr_data, head);

    auto range = chains_.equal_range(head);
    for (auto it = range.first; it != range.second; ++it) {
        Inst *ldst = nullptr;
        Inst *org = it->second;
        /* we don't do arrays of references anyway so the arch-dependent
         * difference in referece size does not really matter */
        auto scale = DataType::ShiftByType(org->GetType(), static_cast<panda::Arch>(0));

        ASSERT(arr_data->IsDominate(org));

        if (org->IsStore()) {
            constexpr auto value_idx = 2;
            ldst = bb->GetGraph()->CreateInst(Opcode::Store);
            ldst->SetInput(value_idx, org->GetInput(value_idx).GetInst());
            ldst->CastToStore()->SetScale(scale);
        } else if (org->IsLoad()) {
            ldst = bb->GetGraph()->CreateInst(Opcode::Load);
            ldst->CastToLoad()->SetScale(scale);
        } else {
            UNREACHABLE();
        }
        ldst->SetInput(0, arr_data);
        ldst->SetInput(1, org->GetInput(1).GetInst());
        ldst->SetType(org->GetType());
        org->ReplaceUsers(ldst);
        org->RemoveInputs();
        org->GetBasicBlock()->ReplaceInst(org, ldst);
        ASSERT(ldst->GetBasicBlock() != nullptr);
    }

    added_ = true;
}
}  // namespace panda::compiler
