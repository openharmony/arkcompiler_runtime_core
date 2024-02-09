/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

namespace ark::compiler {
LoopUnswitcher::LoopUnswitcher(Graph *graph, ArenaAllocator *allocator, ArenaAllocator *localAllocator)
    : GraphCloner(graph, allocator, localAllocator), conditions_(allocator->Adapter())
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
    ASSERT(cloneMarker_ == UNDEF_MARKER);

    auto markerHolder = MarkerHolder(GetGraph());
    cloneMarker_ = markerHolder.GetMarker();
    auto unswitchData = PrepareLoopToUnswitch(loop);

    conditions_.clear();
    for (auto bb : loop->GetBlocks()) {
        if (bb->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
            continue;
        }

        auto ifImm = bb->GetLastInst();
        if (IsConditionEqual(ifImm, inst, false) || IsConditionEqual(ifImm, inst, true)) {
            // will replace all equal or oposite conditions
            conditions_.push_back(ifImm);
        }
    }

    CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, false>(*unswitchData->blocks, GetGraph());
    BuildLoopUnswitchControlFlow(unswitchData);
    BuildLoopUnswitchDataFlow(unswitchData, inst);
    MakeLoopCloneInfo(unswitchData);
    GetGraph()->RunPass<DominatorsTree>();

    auto cloneLoop = GetClone(loop->GetHeader())->GetLoop();
    ASSERT(cloneLoop != loop && cloneLoop->GetOuterLoop() == loop->GetOuterLoop());
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Loop " << loop->GetId() << " is copied";
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Created new loop, id = " << cloneLoop->GetId();
    return cloneLoop;
}

GraphCloner::LoopClonerData *LoopUnswitcher::PrepareLoopToUnswitch(Loop *loop)
{
    auto preHeader = loop->GetPreHeader();
    ASSERT(preHeader->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    // If `outside_succ` has more than 2 predecessors, create a new one
    // with loop header and back-edge predecessors only and insert it before `outside_succ`
    auto outsideSucc = GetLoopOutsideSuccessor(loop);
    constexpr auto PREDS_NUM = 2;
    if (outsideSucc->GetPredsBlocks().size() > PREDS_NUM) {
        auto backEdge = loop->GetBackEdges()[0];
        outsideSucc = CreateNewOutsideSucc(outsideSucc, backEdge, preHeader);
    }
    // Split outside succ after last phi
    // create empty block before outside succ if outside succ don't contain phi insts
    if (outsideSucc->HasPhi() && outsideSucc->GetFirstInst() != nullptr) {
        auto lastPhi = outsideSucc->GetFirstInst()->GetPrev();
        auto block = outsideSucc->SplitBlockAfterInstruction(lastPhi, true);
        // if `outside_succ` is pre-header replace it by `block`
        for (auto inLoop : loop->GetOuterLoop()->GetInnerLoops()) {
            if (inLoop->GetPreHeader() == outsideSucc) {
                inLoop->SetPreHeader(block);
            }
        }
    } else if (outsideSucc->GetFirstInst() != nullptr) {
        auto block = outsideSucc->InsertEmptyBlockBefore();
        outsideSucc->GetLoop()->AppendBlock(block);
        outsideSucc = block;
    }
    // Populate `LoopClonerData`
    auto allocator = GetGraph()->GetLocalAllocator();
    auto unswitchData = allocator->New<LoopClonerData>();
    unswitchData->blocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
    unswitchData->blocks->resize(loop->GetBlocks().size() + 1);
    unswitchData->blocks->at(0) = preHeader;
    std::copy(loop->GetBlocks().begin(), loop->GetBlocks().end(), unswitchData->blocks->begin() + 1);
    unswitchData->blocks->push_back(outsideSucc);
    unswitchData->outer = outsideSucc;
    unswitchData->header = loop->GetHeader();
    unswitchData->preHeader = loop->GetPreHeader();
    return unswitchData;
}

void LoopUnswitcher::BuildLoopUnswitchControlFlow(LoopClonerData *unswitchData)
{
    ASSERT(unswitchData != nullptr);
    auto outerClone = GetClone(unswitchData->outer);
    auto preHeaderClone = GetClone(unswitchData->preHeader);

    auto commonOuter = GetGraph()->CreateEmptyBlock();

    while (!unswitchData->outer->GetSuccsBlocks().empty()) {
        auto succ = unswitchData->outer->GetSuccsBlocks().front();
        succ->ReplacePred(unswitchData->outer, commonOuter);
        unswitchData->outer->RemoveSucc(succ);
    }
    unswitchData->outer->AddSucc(commonOuter);
    outerClone->AddSucc(commonOuter);

    auto commonPredecessor = GetGraph()->CreateEmptyBlock();
    while (!unswitchData->preHeader->GetPredsBlocks().empty()) {
        auto pred = unswitchData->preHeader->GetPredsBlocks().front();
        pred->ReplaceSucc(unswitchData->preHeader, commonPredecessor);
        unswitchData->preHeader->RemovePred(pred);
    }
    commonPredecessor->AddSucc(unswitchData->preHeader);
    commonPredecessor->AddSucc(preHeaderClone);

    for (auto &block : *unswitchData->blocks) {
        if (block != unswitchData->preHeader) {
            CloneEdges<CloneEdgeType::EDGE_PRED>(block);
        }
        if (block != unswitchData->outer) {
            CloneEdges<CloneEdgeType::EDGE_SUCC>(block);
        }
    }
    ASSERT(unswitchData->outer->GetPredBlockIndex(unswitchData->preHeader) ==
           outerClone->GetPredBlockIndex(preHeaderClone));
    ASSERT(unswitchData->header->GetPredBlockIndex(unswitchData->preHeader) ==
           GetClone(unswitchData->header)->GetPredBlockIndex(preHeaderClone));
}

void LoopUnswitcher::BuildLoopUnswitchDataFlow(LoopClonerData *unswitchData, Inst *ifInst)
{
    ASSERT(unswitchData != nullptr);
    for (const auto &block : *unswitchData->blocks) {
        for (const auto &inst : block->AllInsts()) {
            if (inst->GetOpcode() == Opcode::NOP) {
                continue;
            }
            if (inst->IsMarked(cloneMarker_)) {
                SetCloneInputs<false>(inst);
                UpdateCaller(inst);
            }
        }
    }

    auto commonOuter = unswitchData->outer->GetSuccessor(0);
    for (auto phi : unswitchData->outer->PhiInsts()) {
        auto phiClone = GetClone(phi);
        auto phiJoin = commonOuter->GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        phi->ReplaceUsers(phiJoin);
        phiJoin->AppendInput(phi);
        phiJoin->AppendInput(phiClone);
        commonOuter->AppendPhi(phiJoin);
    }

    auto commonPredecessor = unswitchData->preHeader->GetPredecessor(0);
    auto ifInstUnswitch = ifInst->Clone(commonPredecessor->GetGraph());
    for (size_t i = 0; i < ifInst->GetInputsCount(); i++) {
        auto input = ifInst->GetInput(i);
        ifInstUnswitch->SetInput(i, input.GetInst());
    }
    commonPredecessor->AppendInst(ifInstUnswitch);

    ReplaceWithConstantCondition(ifInstUnswitch);
}

void LoopUnswitcher::ReplaceWithConstantCondition(Inst *ifInst)
{
    auto graph = ifInst->GetBasicBlock()->GetGraph();
    auto i1 = graph->FindOrCreateConstant(1);
    auto i2 = graph->FindOrCreateConstant(0);

    auto ifImm = ifInst->CastToIfImm();
    ASSERT(ifImm->GetCc() == ConditionCode::CC_NE || ifImm->GetCc() == ConditionCode::CC_EQ);
    if ((ifImm->GetImm() == 0) != (ifImm->GetCc() == ConditionCode::CC_NE)) {
        std::swap(i1, i2);
    }

    for (auto inst : conditions_) {
        if (IsConditionEqual(inst, ifInst, true)) {
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
        auto ifInst = bb->GetLastInst();
        if (AllInputsConst(ifInst)) {
            continue;
        }
        if (IsHoistable(ifInst, loop)) {
            return ifInst;
        }
    }
    return nullptr;
}

bool LoopUnswitcher::IsSmallLoop(Loop *loop)
{
    auto loopParser = CountableLoopParser(*loop);
    auto loopInfo = loopParser.Parse();
    if (!loopInfo.has_value()) {
        return false;
    }
    auto iterations = CountableLoopParser::GetLoopIterations(*loopInfo);
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

static uint32_t EstimateUnswitchInstructionsCount(BasicBlock *bb, const BasicBlock *backEdge, const Inst *unswitchInst,
                                                  bool trueCond, Marker marker)
{
    if (bb->IsMarked(marker)) {
        return 0;
    }
    bb->SetMarker(marker);

    uint32_t count = bb->CountInsts();
    if (bb == backEdge) {
        return count;
    }

    if (bb->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        count += EstimateUnswitchInstructionsCount(bb->GetSuccsBlocks()[0], backEdge, unswitchInst, trueCond, marker);
    } else if (IsConditionEqual(unswitchInst, bb->GetLastInst(), false)) {
        auto succ = trueCond ? bb->GetTrueSuccessor() : bb->GetFalseSuccessor();
        count += EstimateUnswitchInstructionsCount(succ, backEdge, unswitchInst, trueCond, marker);
    } else if (IsConditionEqual(unswitchInst, bb->GetLastInst(), true)) {
        auto succ = trueCond ? bb->GetFalseSuccessor() : bb->GetTrueSuccessor();
        count += EstimateUnswitchInstructionsCount(succ, backEdge, unswitchInst, trueCond, marker);
    } else {
        for (auto succ : bb->GetSuccsBlocks()) {
            count += EstimateUnswitchInstructionsCount(succ, backEdge, unswitchInst, trueCond, marker);
        }
    }
    return count;
}

void LoopUnswitcher::EstimateInstructionsCount(const Loop *loop, const Inst *unswitchInst, int64_t *loopSize,
                                               int64_t *trueCount, int64_t *falseCount)
{
    ASSERT(loop->GetBackEdges().size() == 1);
    ASSERT(loop->GetInnerLoops().empty());
    *loopSize = static_cast<int64_t>(CountLoopInstructions(loop));
    auto backEdge = loop->GetBackEdges()[0];
    auto graph = backEdge->GetGraph();

    auto trueMarker = graph->NewMarker();
    *trueCount = static_cast<int64_t>(
        EstimateUnswitchInstructionsCount(loop->GetHeader(), backEdge, unswitchInst, true, trueMarker));
    graph->EraseMarker(trueMarker);

    auto falseMarker = graph->NewMarker();
    *falseCount = static_cast<int64_t>(
        EstimateUnswitchInstructionsCount(loop->GetHeader(), backEdge, unswitchInst, false, falseMarker));
    graph->EraseMarker(falseMarker);
}
}  // namespace ark::compiler
