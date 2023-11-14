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
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/licm.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/analysis.h"

namespace panda::compiler {
Licm::Licm(Graph *graph, uint32_t hoist_limit)
    : Optimization(graph), hoist_limit_(hoist_limit), hoistable_instructions_(graph->GetLocalAllocator()->Adapter())
{
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
    if (hoisted_inst_count_ >= hoist_limit_) {
        COMPILER_LOG(DEBUG, LICM_OPT) << "Limit is exceeded, loop isn't visited, id = " << loop.GetId();
        return false;
    }
    COMPILER_LOG(DEBUG, LICM_OPT) << "Visit Loop, id = " << loop.GetId();
    return true;
}

bool AllInputsDominate(Inst *inst, Inst *dom)
{
    for (auto input : inst->GetInputs()) {
        if (!input.GetInst()->IsDominate(dom)) {
            return false;
        }
    }
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
                if (hoisted_inst_count_ >= hoist_limit_) {
                    COMPILER_LOG(DEBUG, LICM_OPT)
                        << "Limit is exceeded, Can't hoist instruction with id = " << inst->GetId();
                    continue;
                }
                if (inst->IsResolver()) {
                    // If it is not "do-while" or "while(true)" loops then the resolver
                    // can not be hoisted out as it might perform class loading/initialization.
                    if (block != loop->GetHeader() && loop->GetHeader()->IsMarked(marker_loop_exit_)) {
                        COMPILER_LOG(DEBUG, LICM_OPT)
                            << "Header is a loop exit, Can't hoist (resolver) instruction with id = " << inst->GetId();
                        continue;
                    }
                    // Don't increment hoisted instruction count until check
                    // if there is another suitable SaveState for the resolver.
                    hoistable_instructions_.push_back(inst);
                    inst->SetMarker(marker_hoist_inst_);
                } else {
                    COMPILER_LOG(DEBUG, LICM_OPT) << "Hoist instruction with id = " << inst->GetId();
                    hoistable_instructions_.push_back(inst);
                    inst->SetMarker(marker_hoist_inst_);
                    hoisted_inst_count_++;
                }
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
        Inst *target = nullptr;
        if (inst->IsResolver()) {
            auto ss = FindSaveStateForResolver(inst, pre_header);
            if (ss != nullptr) {
                ASSERT(ss->IsSaveState());
                COMPILER_LOG(DEBUG, LICM_OPT) << "Hoist (resolver) instruction with id = " << inst->GetId();
                inst->SetSaveState(ss);
                inst->SetPc(ss->GetPc());
                hoisted_inst_count_++;
            } else {
                continue;
            }
        } else if (inst->IsMovableObject()) {
            target = inst->GetPrev();
            if (target == nullptr) {
                target = inst->GetNext();
            }
            if (target == nullptr) {
                target = GetGraph()->CreateInstNOP();
                inst->GetBasicBlock()->InsertAfter(target, inst);
            }
        }
        inst->GetBasicBlock()->EraseInst(inst);
        if (last_inst == nullptr || last_inst->IsPhi()) {
            pre_header->AppendInst(inst);
            last_inst = inst;
        } else {
            ASSERT(last_inst->GetBasicBlock() == pre_header);
            Inst *ss = pre_header->FindSaveStateDeoptimize();
            if (ss != nullptr && AllInputsDominate(inst, ss)) {
                ss->InsertBefore(inst);
            } else {
                last_inst->InsertAfter(inst);
                last_inst = inst;
            }
        }
        GetGraph()->GetEventWriter().EventLicm(inst->GetId(), inst->GetPc(), loop->GetId(),
                                               pre_header->GetLoop()->GetId());
        if (inst->IsMovableObject()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), inst, target);
        }
    }
}

static bool FindUnsafeInstBetween(BasicBlock *dom_bb, BasicBlock *curr_bb, Marker visited, const Inst *resolver,
                                  Inst *start_inst = nullptr)
{
    ASSERT(resolver->IsResolver());

    if (dom_bb == curr_bb) {
        return false;
    }
    if (curr_bb->SetMarker(visited)) {
        return false;
    }
    // Do not cross a try block because the resolver
    // can throw NoSuch{Method|Field}Exception
    if (curr_bb->IsTry()) {
        return true;
    }
    // Field and Method resolvers may call GetField and InitializeClass methods
    // of the class linker resulting to class loading/initialization.
    // An order of class initialization must be preserved, so the resolver
    // must not cross other instructions performing class initialization
    // We have to be conservative with respect to calls.
    for (auto inst : InstSafeReverseIter(*curr_bb, start_inst)) {
        if (inst->IsClassInst() || inst->IsResolver() || inst->IsCall()) {
            return true;
        }
    }
    for (auto pred : curr_bb->GetPredsBlocks()) {
        if (FindUnsafeInstBetween(dom_bb, pred, visited, resolver)) {
            return true;
        }
    }
    return false;
}

Inst *Licm::FindSaveStateForResolver(Inst *resolver, const BasicBlock *pre_header)
{
    ASSERT(pre_header->GetSuccsBlocks().size() == 1);
    // Lookup for an appropriate SaveState in the preheader
    Inst *ss = nullptr;
    for (const auto &inst : pre_header->InstsSafeReverse()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            // Give up further checking if SaveState came from an inlined function.
            if (static_cast<SaveStateInst *>(inst)->GetCallerInst() != nullptr) {
                return nullptr;
            }
            for (auto i = pre_header->GetLastInst(); i != inst; i = i->GetPrev()) {
                if (i->IsMovableObject()) {
                    // In this case we have to avoid instructions, which may trigger GC and move objects,
                    // thus we can not move the resolver to the pre_header and link with SaveState.
                    return nullptr;
                }
            }
            ss = inst;
            break;
        }
    }
    if (ss == nullptr) {
        return nullptr;
    }
    // Check if the path from the resolver instruction to the SaveState block is safe
    ASSERT(ss->IsDominate(resolver));
    MarkerHolder visited(GetGraph());
    if (FindUnsafeInstBetween(ss->GetBasicBlock(), resolver->GetBasicBlock(), visited.GetMarker(), resolver,
                              resolver->GetPrev())) {
        return nullptr;
    }
    return ss;
}

/*
 * Instruction is not hoistable if one of the following conditions are true:
 *  - it has input which is defined in the same loop and not mark as hoistable
 *  - it has input which is not dominate instruction's loop preheader
 */
bool Licm::InstInputDominatesPreheader(Inst *inst)
{
    ASSERT(!inst->IsPhi());
    auto inst_loop = inst->GetBasicBlock()->GetLoop();
    for (auto input : inst->GetInputs()) {
        // Graph is in SSA form and the instructions not PHI,
        // so every 'input' must dominate 'inst'.
        ASSERT(input.GetInst()->IsDominate(inst));
        auto input_loop = input.GetInst()->GetBasicBlock()->GetLoop();
        if (inst_loop == input_loop) {
            if (inst->IsResolver() && input.GetInst()->IsSaveState()) {
                // Resolver is coupled with SaveState that is not hoistable.
                // We will try to find an appropriate SaveState in the preheader.
                continue;
            }
            if (!input.GetInst()->IsMarked(marker_hoist_inst_)) {
                return false;
            }
        } else if (inst_loop->GetOuterLoop() == input_loop->GetOuterLoop()) {
            if (!input.GetInst()->GetBasicBlock()->IsDominate(inst_loop->GetPreHeader())) {
                return false;
            }
        } else if (!inst_loop->IsInside(input_loop)) {
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
    if (inst->IsNotHoistable()) {
        return false;
    }
    // Do not hoist the resolver if it came from an inlined function.
    if (inst->IsResolver() && inst->GetSaveState()->GetCallerInst() != nullptr) {
        return false;
    }
    if (inst->GetOpcode() == Opcode::LenArray &&
        !BoundsAnalysis::IsInstNotNull(inst->GetDataFlowInput(0), inst->GetBasicBlock()->GetLoop()->GetHeader())) {
        return false;
    }
    return InstInputDominatesPreheader(inst) && InstDominatesLoopExits(inst) && !GetGraph()->IsInstThrowable(inst);
}
}  // namespace panda::compiler
