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
#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/loop_peeling.h"

namespace panda::compiler {
/**
 * Loop-peeling optimization works with a loops with the following requirements:
 * - loop is not irreducible;
 * - there is only 1 back-edge;
 * - loop-header is a single loop-exit point;
 *
 *          [pre-header]
 *              |
 *              v
 *     /---->[header]--------\
 *     |        |            |
 *     |        v            v
 *     \----[back-edge]   [outer]
 *
 * There are two stages of the algorithm
 * 1 stage - insert pre-loop which is an if-block:
 *
 *         [pre-header]
 *              |
 *              v
 *          [pre-loop]--------\
 *              |             |
 *              v             v
 *     /---->[header]-------->|
 *     |        |             |
 *     |        v             v
 *     \----[back-edge]   [resolver]
 *                            |
 *                            v
 *                         [outer]
 *
 * 2 stage - move exit-edge form the header to the back-edge block:
 *
 *         [pre-header]
 *              |
 *              v
 *          [pre-loop]--------\
 *              |             |
 *              v             v
 *     /---->[header]         |
 *     |        |             |
 *     |        v             v
 *     \----[back-edge]-->[resolver]
 *                            |
 *                            v
 *                         [outer]
 *
 */
bool LoopPeeling::RunImpl()
{
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    return is_appied_;
}

void LoopPeeling::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

static inline void CleanDeadPhis(BasicBlock *block)
{
    for (auto phi : block->PhiInstsSafe()) {
        if (!phi->HasUsers()) {
            block->RemoveInst(phi);
        }
    }
}

static bool HeaderHasInlinedCalls(const BasicBlock *header)
{
    auto check_inlined_call = [](auto inst) {
        return inst->IsCall() && static_cast<const CallInst *>(inst)->IsInlined();
    };
    auto insts = header->AllInsts();
    return std::find_if(insts.begin(), insts.end(), check_inlined_call) != insts.end();
}

bool LoopPeeling::TransformLoop(Loop *loop)
{
    ASSERT(loop->GetBackEdges().size() == 1);
    ASSERT(loop->GetHeader()->GetLastInst()->GetOpcode() == Opcode::IfImm ||
           loop->GetHeader()->GetLastInst()->GetOpcode() == Opcode::If);
    auto header = loop->GetHeader();
    // If header contains inlined call this call will be cloned and stay unpaired without `Return.inlined` instruction
    if (HeaderHasInlinedCalls(header)) {
        return false;
    }
    auto back_edge = loop->GetBackEdges()[0];
    InsertPreLoop(loop);
    auto moved_inst_count = MoveLoopExitToBackEdge(header, back_edge);
    CleanDeadPhis(header);
    is_appied_ = true;
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Loop was peeled, id = " << loop->GetId();
    GetGraph()->GetEventWriter().EventLoopPeeling(loop->GetId(), header->GetGuestPc(), back_edge->GetGuestPc(),
                                                  moved_inst_count);
    return true;
}

/**
 * Clone the loop-header and insert before it
 */
void LoopPeeling::InsertPreLoop(Loop *loop)
{
    auto header = loop->GetHeader();
    auto pre_header = loop->GetPreHeader();
    auto graph_cloner =
        GraphCloner(header->GetGraph(), header->GetGraph()->GetAllocator(), header->GetGraph()->GetLocalAllocator());
    graph_cloner.CloneLoopHeader(header, GetLoopOuterBlock(header), pre_header);
}

/*
 *  Make back-edge loop-exit point
 */
size_t LoopPeeling::MoveLoopExitToBackEdge(BasicBlock *header, BasicBlock *back_edge)
{
    size_t moved_inst_count = 0;
    auto outer_block = GetLoopOuterBlock(header);
    size_t outer_idx = header->GetSuccBlockIndex(outer_block);

    // Create exit block
    BasicBlock *exit_block = nullptr;
    if (header != back_edge) {
        ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
        exit_block = back_edge->InsertNewBlockToSuccEdge(header);
        outer_block->ReplacePred(header, exit_block);
        header->RemoveSucc(outer_block);
        exit_block->SetTry(header->IsTry());

        auto loop = header->GetLoop();
        loop->AppendBlock(exit_block);
        loop->ReplaceBackEdge(back_edge, exit_block);

        // Check the order of true-false successors
        if (exit_block->GetSuccBlockIndex(outer_block) != outer_idx) {
            exit_block->SwapTrueFalseSuccessors();
        }

        // Fix Dominators info
        auto &dom_tree = GetGraph()->GetAnalysis<DominatorsTree>();
        dom_tree.SetValid(true);
        dom_tree.SetDomPair(back_edge, exit_block);
        back_edge = exit_block;
    }

    // Use reverse order to keep domination relation between instructions in the header-block
    for (auto inst : header->InstsSafeReverse()) {
        if (exit_block != nullptr) {
            header->EraseInst(inst);
            exit_block->PrependInst(inst);
            moved_inst_count++;
        }
        UpdateClonedInstInputs(inst, header, back_edge);
    }

    // Update outer phis
    for (auto phi : outer_block->PhiInsts()) {
        size_t header_idx = phi->CastToPhi()->GetPredBlockIndex(back_edge);
        auto header_inst = phi->GetInput(header_idx).GetInst();
        if (header_inst->IsPhi()) {
            phi->SetInput(header_idx, header_inst->CastToPhi()->GetPhiInput(back_edge));
        }
    }

    return moved_inst_count;
}

void LoopPeeling::UpdateClonedInstInputs(Inst *inst, BasicBlock *header, BasicBlock *back_edge)
{
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        auto input_inst = inst->GetInput(i).GetInst();
        if (input_inst->IsPhi() && input_inst->GetBasicBlock() == header) {
            auto phi_input = input_inst->CastToPhi()->GetPhiInput(back_edge);
            // Replace phi by its input, if this input will NOT be moved to the exit block
            bool is_moved = phi_input->GetBasicBlock() == header && !phi_input->IsPhi();
            if (phi_input->IsDominate(inst) && !is_moved) {
                inst->SetInput(i, phi_input);
            }
        }
    }
}
}  // namespace panda::compiler
