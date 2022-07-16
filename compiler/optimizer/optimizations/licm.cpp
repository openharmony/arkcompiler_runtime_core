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
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/optimizations/licm.h"

namespace panda::compiler {
Licm::Licm(Graph *graph, uint32_t hoist_limit)
    : Optimization(graph), HOIST_LIMIT(hoist_limit), hoistable_instructions_(graph->GetLocalAllocator()->Adapter())
{
    hoistable_instructions_.reserve(HOIST_LIMIT);
}

// TODO (a.popov) Use `LoopTransform` base class similarly `LoopUnroll`, `LoopPeeling`
bool Licm::RunImpl()
{
    if (!GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid()) {
        GetGraph()->GetAnalysis<LoopAnalyzer>().Run();
    }

    ASSERT(GetGraph()->GetRootLoop() != nullptr);
    if (!GetGraph()->GetRootLoop()->GetInnerLoops().empty()) {
        marker_loop_exit_ = GetGraph()->NewMarker();
        MarkLoopExits(GetGraph(), marker_loop_exit_);
        for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
            LoopSearchDFS(loop);
        }
        GetGraph()->EraseMarker(marker_loop_exit_);
    }
    return (hoisted_inst_count_ != 0);
}

void Licm::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

/*
 * Check if block is loop exit, exclude loop header
 */
bool Licm::IsBlockLoopExit(BasicBlock *block)
{
    return block->IsMarked(marker_loop_exit_) && !block->IsLoopHeader();
}

/*
 * Search loops in DFS order to visit inner loops firstly
 */
void Licm::LoopSearchDFS(Loop *loop)
{
    ASSERT(loop != nullptr);
    for (auto inner_loop : loop->GetInnerLoops()) {
        LoopSearchDFS(inner_loop);
    }
    if (IsLoopVisited(*loop)) {
        VisitLoop(loop);
    }
}

bool Licm::IsLoopVisited(const Loop &loop) const
{
    if (loop.IsIrreducible()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Irreducible loop isn't visited, id = " << loop.GetId();
        return false;
    }
    if (loop.IsOsrLoop()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "OSR entry isn't visited, loop id = " << loop.GetId();
        return false;
    }
    if (loop.IsTryCatchLoop()) {
        COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Try-catch loop isn't visited, loop id = " << loop.GetId();
        return false;
    }
    if (hoisted_inst_count_ >= HOIST_LIMIT) {
        COMPILER_LOG(DEBUG, LICM_OPT) << "Limit is exceeded, loop isn't visited, id = " << loop.GetId();
        return false;
    }
    COMPILER_LOG(DEBUG, LICM_OPT) << "Visit Loop, id = " << loop.GetId();
    return true;
}

/*
 * Iterate over all instructions in the loop and move hoistable instructions to the loop preheader
 */
void Licm::VisitLoop(Loop *loop)
{
    auto marker_holder = MarkerHolder(GetGraph());
    marker_hoist_inst_ = marker_holder.GetMarker();
    hoistable_instructions_.clear();
    // Collect instruction, which can be hoisted
    for (auto block : loop->GetBlocks()) {
        ASSERT(block->GetLoop() == loop);
        for (auto inst : block->InstsSafe()) {
            if (IsInstHoistable(inst)) {
                if (hoisted_inst_count_ >= HOIST_LIMIT) {
                    COMPILER_LOG(DEBUG, LICM_OPT)
                        << "Limit is exceeded, Can't hoist instruction with id = " << inst->GetId();
                    continue;
                }
                COMPILER_LOG(DEBUG, LICM_OPT) << "Hoist instruction with id = " << inst->GetId();
                hoistable_instructions_.push_back(inst);
                inst->SetMarker(marker_hoist_inst_);
                hoisted_inst_count_++;
            }
        }
    }
    auto pre_header = loop->GetPreHeader();
    ASSERT(pre_header != nullptr);
    auto header = loop->GetHeader();
    if (pre_header->GetSuccsBlocks().size() > 1) {
        ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
        // Create new pre-header with single successor: loop header
        auto new_pre_header = pre_header->InsertNewBlockToSuccEdge(header);
        pre_header->GetLoop()->AppendBlock(new_pre_header);
        // Fix Dominators info
        auto &dom_tree = GetGraph()->GetAnalysis<DominatorsTree>();
        dom_tree.SetValid(true);
        ASSERT(header->GetDominator() == pre_header);
        pre_header->RemoveDominatedBlock(header);
        dom_tree.SetDomPair(new_pre_header, header);
        dom_tree.SetDomPair(pre_header, new_pre_header);
        // Change pre-header
        loop->SetPreHeader(new_pre_header);
        pre_header = new_pre_header;
    }
    // Find position to move: either add first instruction to the empty block or after the last instruction
    Inst *last_inst = pre_header->GetLastInst();
    // Move instructions
    for (auto inst : hoistable_instructions_) {
        inst->GetBasicBlock()->EraseInst(inst);
        if (last_inst == nullptr || last_inst->IsPhi()) {
            pre_header->AppendInst(inst);
        } else {
            ASSERT(last_inst->GetBasicBlock() == pre_header);
            last_inst->InsertAfter(inst);
        }
        GetGraph()->GetEventWriter().EventLicm(inst->GetId(), inst->GetPc(), loop->GetId(),
                                               pre_header->GetLoop()->GetId());
        last_inst = inst;
    }
}

/*
 * Instruction is not hoistable if it has input which is defined in the same loop and not mark as hoistable
 * Or if it has input which is not dominate instruction's loop preheader
 */
bool Licm::InstInputDominatesPreheader(Inst *inst)
{
    auto inst_loop = inst->GetBasicBlock()->GetLoop();
    for (auto input : inst->GetInputs()) {
        auto input_loop = input.GetInst()->GetBasicBlock()->GetLoop();
        if (inst_loop == input_loop) {
            if (!input.GetInst()->IsMarked(marker_hoist_inst_) || !input.GetInst()->IsDominate(inst)) {
                return false;
            }
        } else if (inst_loop->GetOuterLoop() == input_loop->GetOuterLoop()) {
            if (!inst->GetBasicBlock()->IsDominate(inst_loop->GetPreHeader())) {
                return false;
            }
        } else if (!inst_loop->IsInside(input_loop)) {
            return false;
        }
    }
    return true;
}

/*
 * Hoistable instruction must dominate all uses in the loop
 */
static bool InstDominatesUsesInLoop(Inst *inst)
{
    auto inst_loop = inst->GetBasicBlock()->GetLoop();
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->GetBasicBlock()->GetLoop() == inst_loop && !inst->IsDominate(user.GetInst())) {
            return false;
        }
    }
    return true;
}

/*
 * Hoistable instruction must dominate all loop exits
 */
bool Licm::InstDominatesLoopExits(Inst *inst)
{
    auto inst_loop = inst->GetBasicBlock()->GetLoop();
    for (auto block : inst_loop->GetBlocks()) {
        if (IsBlockLoopExit(block) && !inst->GetBasicBlock()->IsDominate(block)) {
            return false;
        }
    }
    return true;
}

/*
 * Check if instruction can be moved to the loop preheader
 */
bool Licm::IsInstHoistable(Inst *inst)
{
    ASSERT(!inst->IsPhi());
    return !inst->IsNotHoistable() && InstInputDominatesPreheader(inst) && InstDominatesUsesInLoop(inst) &&
           InstDominatesLoopExits(inst) && !GetGraph()->IsInstThrowable(inst);
}
}  // namespace panda::compiler
