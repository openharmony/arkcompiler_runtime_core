/**
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

#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "compiler_logger.h"
#include "loop_unswitcher.h"

namespace panda::compiler {
LoopUnswitcher::LoopUnswitcher(Graph *graph, ArenaAllocator *allocator, ArenaAllocator *local_allocator)
    : GraphCloner(graph, allocator, local_allocator), conditions_(allocator->Adapter())
{
}

/**
 * Unswitch loop in selected branch instruction.
 * Return pointer to new loop or nullptr if cannot unswitch loop.
 */
Loop *LoopUnswitcher::UnswitchLoop(Loop *loop, Inst *inst)
{
    ASSERT(loop != nullptr && !loop->IsRoot());
    ASSERT_PRINT(IsLoopSingleBackEdgeExitPoint(loop), "Cloning blocks doesn't have single entry/exit point");
    ASSERT(loop->GetPreHeader() != nullptr);
    ASSERT(!loop->IsIrreducible());
    ASSERT(!loop->IsOsrLoop());
    if (loop->GetPreHeader()->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        return nullptr;
    }
    ASSERT(clone_marker_ == UNDEF_MARKER);

    auto marker_holder = MarkerHolder(GetGraph());
    clone_marker_ = marker_holder.GetMarker();
    auto unswitch_data = PrepareLoopToUnswitch(loop);

    conditions_.clear();
    for (auto bb : loop->GetBlocks()) {
        if (bb->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
            continue;
        }

        auto if_imm = bb->GetLastInst();
        if (IsConditionEqual(if_imm, inst, false) || IsConditionEqual(if_imm, inst, true)) {
            // will replace all equal or oposite conditions
            conditions_.push_back(if_imm);
        }
    }

    CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, false>(*unswitch_data->blocks, GetGraph());
    BuildLoopUnswitchControlFlow(unswitch_data);
    BuildLoopUnswitchDataFlow(unswitch_data, inst);
    MakeLoopCloneInfo(unswitch_data);
    GetGraph()->RunPass<DominatorsTree>();

    auto clone_loop = GetClone(loop->GetHeader())->GetLoop();
    ASSERT(clone_loop != loop && clone_loop->GetOuterLoop() == loop->GetOuterLoop());
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Loop " << loop->GetId() << " is copied";
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Created new loop, id = " << clone_loop->GetId();
    return clone_loop;
}

GraphCloner::LoopClonerData *LoopUnswitcher::PrepareLoopToUnswitch(Loop *loop)
{
    auto pre_header = loop->GetPreHeader();
    ASSERT(pre_header->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    // If `outside_succ` has more than 2 predecessors, create a new one
    // with loop header and back-edge predecessors only and insert it before `outside_succ`
    auto outside_succ = GetLoopOutsideSuccessor(loop);
    constexpr auto PREDS_NUM = 2;
    if (outside_succ->GetPredsBlocks().size() > PREDS_NUM) {
        auto back_edge = loop->GetBackEdges()[0];
        outside_succ = CreateNewOutsideSucc(outside_succ, back_edge, pre_header);
    }
    // Split outside succ after last phi
    // create empty block before outside succ if outside succ don't contain phi insts
    if (outside_succ->HasPhi() && outside_succ->GetFirstInst() != nullptr) {
        auto last_phi = outside_succ->GetFirstInst()->GetPrev();
        auto block = outside_succ->SplitBlockAfterInstruction(last_phi, true);
        // if `outside_succ` is pre-header replace it by `block`
        for (auto in_loop : loop->GetOuterLoop()->GetInnerLoops()) {
            if (in_loop->GetPreHeader() == outside_succ) {
                in_loop->SetPreHeader(block);
            }
        }
    } else if (outside_succ->GetFirstInst() != nullptr) {
        auto block = outside_succ->InsertEmptyBlockBefore();
        outside_succ->GetLoop()->AppendBlock(block);
        outside_succ = block;
    }
    // Populate `LoopClonerData`
    auto allocator = GetGraph()->GetLocalAllocator();
    auto unswitch_data = allocator->New<LoopClonerData>();
    unswitch_data->blocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
    unswitch_data->blocks->resize(loop->GetBlocks().size() + 1);
    unswitch_data->blocks->at(0) = pre_header;
    std::copy(loop->GetBlocks().begin(), loop->GetBlocks().end(), unswitch_data->blocks->begin() + 1);
    unswitch_data->blocks->push_back(outside_succ);
    unswitch_data->outer = outside_succ;
    unswitch_data->header = loop->GetHeader();
    unswitch_data->pre_header = loop->GetPreHeader();
    return unswitch_data;
}

void LoopUnswitcher::BuildLoopUnswitchControlFlow(LoopClonerData *unswitch_data)
{
    ASSERT(unswitch_data != nullptr);
    auto outer_clone = GetClone(unswitch_data->outer);
    auto pre_header_clone = GetClone(unswitch_data->pre_header);

    auto common_outer = GetGraph()->CreateEmptyBlock();

    while (!unswitch_data->outer->GetSuccsBlocks().empty()) {
        auto succ = unswitch_data->outer->GetSuccsBlocks().front();
        succ->ReplacePred(unswitch_data->outer, common_outer);
        unswitch_data->outer->RemoveSucc(succ);
    }
    unswitch_data->outer->AddSucc(common_outer);
    outer_clone->AddSucc(common_outer);

    auto common_predecessor = GetGraph()->CreateEmptyBlock();
    while (!unswitch_data->pre_header->GetPredsBlocks().empty()) {
        auto pred = unswitch_data->pre_header->GetPredsBlocks().front();
        pred->ReplaceSucc(unswitch_data->pre_header, common_predecessor);
        unswitch_data->pre_header->RemovePred(pred);
    }
    common_predecessor->AddSucc(unswitch_data->pre_header);
    common_predecessor->AddSucc(pre_header_clone);

    for (auto &block : *unswitch_data->blocks) {
        if (block != unswitch_data->pre_header) {
            CloneEdges<CloneEdgeType::EDGE_PRED>(block);
        }
        if (block != unswitch_data->outer) {
            CloneEdges<CloneEdgeType::EDGE_SUCC>(block);
        }
    }
    ASSERT(unswitch_data->outer->GetPredBlockIndex(unswitch_data->pre_header) ==
           outer_clone->GetPredBlockIndex(pre_header_clone));
    ASSERT(unswitch_data->header->GetPredBlockIndex(unswitch_data->pre_header) ==
           GetClone(unswitch_data->header)->GetPredBlockIndex(pre_header_clone));
}

void LoopUnswitcher::BuildLoopUnswitchDataFlow(LoopClonerData *unswitch_data, Inst *if_inst)
{
    ASSERT(unswitch_data != nullptr);
    for (const auto &block : *unswitch_data->blocks) {
        for (const auto &inst : block->AllInsts()) {
            if (inst->GetOpcode() == Opcode::NOP) {
                continue;
            }
            if (inst->IsMarked(clone_marker_)) {
                SetCloneInputs<false>(inst);
                UpdateCaller(inst);
            }
        }
    }

    auto common_outer = unswitch_data->outer->GetSuccessor(0);
    for (auto phi : unswitch_data->outer->PhiInsts()) {
        auto phi_clone = GetClone(phi);
        auto phi_join = common_outer->GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        phi->ReplaceUsers(phi_join);
        phi_join->AppendInput(phi);
        phi_join->AppendInput(phi_clone);
        common_outer->AppendPhi(phi_join);
    }

    auto common_predecessor = unswitch_data->pre_header->GetPredecessor(0);
    auto if_inst_unswitch = if_inst->Clone(common_predecessor->GetGraph());
    for (size_t i = 0; i < if_inst->GetInputsCount(); i++) {
        auto input = if_inst->GetInput(i);
        if_inst_unswitch->SetInput(i, input.GetInst());
    }
    common_predecessor->AppendInst(if_inst_unswitch);

    ReplaceWithConstantCondition(if_inst_unswitch);
}

void LoopUnswitcher::ReplaceWithConstantCondition(Inst *if_inst)
{
    auto graph = if_inst->GetBasicBlock()->GetGraph();
    auto i1 = graph->FindOrCreateConstant(1);
    auto i2 = graph->FindOrCreateConstant(0);

    auto if_imm = if_inst->CastToIfImm();
    ASSERT(if_imm->GetCc() == ConditionCode::CC_NE || if_imm->GetCc() == ConditionCode::CC_EQ);
    if ((if_imm->GetImm() == 0) != (if_imm->GetCc() == ConditionCode::CC_NE)) {
        std::swap(i1, i2);
    }

    for (auto inst : conditions_) {
        if (IsConditionEqual(inst, if_inst, true)) {
            inst->SetInput(0, i2);
            GetClone(inst)->SetInput(0, i1);
        } else {
            inst->SetInput(0, i1);
            GetClone(inst)->SetInput(0, i2);
        }
    }
}

static bool AllInputsConst(Inst *inst)
{
    for (auto input : inst->GetInputs()) {
        if (!input.GetInst()->IsConst()) {
            return false;
        }
    }
    return true;
}

static bool IsHoistable(Inst *inst, Loop *loop)
{
    for (auto input : inst->GetInputs()) {
        if (!input.GetInst()->GetBasicBlock()->IsDominate(loop->GetPreHeader())) {
            return false;
        }
    }
    return true;
}

Inst *LoopUnswitcher::FindUnswitchInst(Loop *loop)
{
    for (auto bb : loop->GetBlocks()) {
        if (bb->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
            continue;
        }
        auto if_inst = bb->GetLastInst();
        if (AllInputsConst(if_inst)) {
            continue;
        }
        if (IsHoistable(if_inst, loop)) {
            return if_inst;
        }
    }
    return nullptr;
}

bool LoopUnswitcher::IsSmallLoop(Loop *loop)
{
    auto loop_parser = CountableLoopParser(*loop);
    auto loop_info = loop_parser.Parse();
    if (!loop_info.has_value()) {
        return false;
    }
    auto iterations = CountableLoopParser::GetLoopIterations(*loop_info);
    if (!iterations.has_value()) {
        return false;
    }
    return *iterations <= 1;
}

static uint32_t CountLoopInstructions(const Loop *loop)
{
    uint32_t count = 0;
    for (auto block : loop->GetBlocks()) {
        count += block->CountInsts();
    }
    return count;
}

static uint32_t EstimateUnswitchInstructionsCount(BasicBlock *bb, const BasicBlock *back_edge,
                                                  const Inst *unswitch_inst, bool true_cond, Marker marker)
{
    if (bb->IsMarked(marker)) {
        return 0;
    }
    bb->SetMarker(marker);

    uint32_t count = bb->CountInsts();
    if (bb == back_edge) {
        return count;
    }

    if (bb->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        count +=
            EstimateUnswitchInstructionsCount(bb->GetSuccsBlocks()[0], back_edge, unswitch_inst, true_cond, marker);
    } else if (IsConditionEqual(unswitch_inst, bb->GetLastInst(), false)) {
        auto succ = true_cond ? bb->GetTrueSuccessor() : bb->GetFalseSuccessor();
        count += EstimateUnswitchInstructionsCount(succ, back_edge, unswitch_inst, true_cond, marker);
    } else if (IsConditionEqual(unswitch_inst, bb->GetLastInst(), true)) {
        auto succ = true_cond ? bb->GetFalseSuccessor() : bb->GetTrueSuccessor();
        count += EstimateUnswitchInstructionsCount(succ, back_edge, unswitch_inst, true_cond, marker);
    } else {
        for (auto succ : bb->GetSuccsBlocks()) {
            count += EstimateUnswitchInstructionsCount(succ, back_edge, unswitch_inst, true_cond, marker);
        }
    }
    return count;
}

void LoopUnswitcher::EstimateInstructionsCount(const Loop *loop, const Inst *unswitch_inst, uint32_t *loop_size,
                                               uint32_t *true_count, uint32_t *false_count)
{
    ASSERT(loop->GetBackEdges().size() == 1);
    ASSERT(loop->GetInnerLoops().empty());
    *loop_size = CountLoopInstructions(loop);
    auto back_edge = loop->GetBackEdges()[0];
    auto graph = back_edge->GetGraph();

    auto true_marker = graph->NewMarker();
    *true_count = EstimateUnswitchInstructionsCount(loop->GetHeader(), back_edge, unswitch_inst, true, true_marker);
    graph->EraseMarker(true_marker);

    auto false_marker = graph->NewMarker();
    *false_count = EstimateUnswitchInstructionsCount(loop->GetHeader(), back_edge, unswitch_inst, false, false_marker);
    graph->EraseMarker(false_marker);
}
}  // namespace panda::compiler
