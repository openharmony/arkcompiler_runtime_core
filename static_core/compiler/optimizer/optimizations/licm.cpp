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
Licm::Licm(Graph *graph, uint32_t hoistLimit)
    : Optimization(graph), hoistLimit_(hoistLimit), hoistableInstructions_(graph->GetLocalAllocator()->Adapter())
{
}

// NOTE (a.popov) Use `LoopTransform` base class similarly `LoopUnroll`, `LoopPeeling`
bool Licm::RunImpl()
{
    if (!GetGraph()->GetAnalysis<LoopAnalyzer>().IsValid()) {
        GetGraph()->GetAnalysis<LoopAnalyzer>().Run();
    }

    ASSERT(GetGraph()->GetRootLoop() != nullptr);
    if (!GetGraph()->GetRootLoop()->GetInnerLoops().empty()) {
        markerLoopExit_ = GetGraph()->NewMarker();
        MarkLoopExits(GetGraph(), markerLoopExit_);
        for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
            LoopSearchDFS(loop);
        }
        GetGraph()->EraseMarker(markerLoopExit_);
    }
    return (hoistedInstCount_ != 0);
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
    return block->IsMarked(markerLoopExit_) && !block->IsLoopHeader();
}

/*
 * Search loops in DFS order to visit inner loops firstly
 */
void Licm::LoopSearchDFS(Loop *loop)
{
    ASSERT(loop != nullptr);
    for (auto innerLoop : loop->GetInnerLoops()) {
        LoopSearchDFS(innerLoop);
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
    if (hoistedInstCount_ >= hoistLimit_) {
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

void Licm::TryAppendHoistableInst(Inst *inst, BasicBlock *block, Loop *loop)
{
    if (hoistedInstCount_ >= hoistLimit_) {
        COMPILER_LOG(DEBUG, LICM_OPT) << "Limit is exceeded, Can't hoist instruction with id = " << inst->GetId();
        return;
    }
    if (inst->IsResolver()) {
        // If it is not "do-while" or "while(true)" loops then the resolver
        // can not be hoisted out as it might perform class loading/initialization.
        if (block != loop->GetHeader() && loop->GetHeader()->IsMarked(markerLoopExit_)) {
            COMPILER_LOG(DEBUG, LICM_OPT)
                << "Header is a loop exit, Can't hoist (resolver) instruction with id = " << inst->GetId();
            return;
        }
        // Don't increment hoisted instruction count until check
        // if there is another suitable SaveState for the resolver.
        hoistableInstructions_.push_back(inst);
        inst->SetMarker(markerHoistInst_);
    } else {
        COMPILER_LOG(DEBUG, LICM_OPT) << "Hoist instruction with id = " << inst->GetId();
        hoistableInstructions_.push_back(inst);
        inst->SetMarker(markerHoistInst_);
        hoistedInstCount_++;
    }
}

void Licm::MoveInstructions(BasicBlock *preHeader, Loop *loop)
{
    // Find position to move: either add first instruction to the empty block or after the last instruction
    Inst *lastInst = preHeader->GetLastInst();

    // Move instructions
    for (auto inst : hoistableInstructions_) {
        Inst *target = nullptr;
        if (inst->IsResolver()) {
            auto ss = FindSaveStateForResolver(inst, preHeader);
            if (ss != nullptr) {
                ASSERT(ss->IsSaveState());
                COMPILER_LOG(DEBUG, LICM_OPT) << "Hoist (resolver) instruction with id = " << inst->GetId();
                inst->SetSaveState(ss);
                inst->SetPc(ss->GetPc());
                hoistedInstCount_++;
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
        if (lastInst == nullptr || lastInst->IsPhi()) {
            preHeader->AppendInst(inst);
            lastInst = inst;
        } else {
            ASSERT(lastInst->GetBasicBlock() == preHeader);
            Inst *ss = preHeader->FindSaveStateDeoptimize();
            if (ss != nullptr && AllInputsDominate(inst, ss)) {
                ss->InsertBefore(inst);
            } else {
                lastInst->InsertAfter(inst);
                lastInst = inst;
            }
        }
        GetGraph()->GetEventWriter().EventLicm(inst->GetId(), inst->GetPc(), loop->GetId(),
                                               preHeader->GetLoop()->GetId());
        if (inst->IsMovableObject()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), inst, target);
        }
    }
}

/*
 * Iterate over all instructions in the loop and move hoistable instructions to the loop preheader
 */
void Licm::VisitLoop(Loop *loop)
{
    auto markerHolder = MarkerHolder(GetGraph());
    markerHoistInst_ = markerHolder.GetMarker();
    hoistableInstructions_.clear();
    // Collect instruction, which can be hoisted
    for (auto block : loop->GetBlocks()) {
        ASSERT(block->GetLoop() == loop);
        for (auto inst : block->InstsSafe()) {
            if (!IsInstHoistable(inst)) {
                continue;
            }
            TryAppendHoistableInst(inst, block, loop);
        }
    }
    auto preHeader = loop->GetPreHeader();
    ASSERT(preHeader != nullptr);
    auto header = loop->GetHeader();
    if (preHeader->GetSuccsBlocks().size() > 1) {
        ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
        // Create new pre-header with single successor: loop header
        auto newPreHeader = preHeader->InsertNewBlockToSuccEdge(header);
        preHeader->GetLoop()->AppendBlock(newPreHeader);
        // Fix Dominators info
        auto &domTree = GetGraph()->GetAnalysis<DominatorsTree>();
        domTree.SetValid(true);
        ASSERT(header->GetDominator() == preHeader);
        preHeader->RemoveDominatedBlock(header);
        domTree.SetDomPair(newPreHeader, header);
        domTree.SetDomPair(preHeader, newPreHeader);
        // Change pre-header
        loop->SetPreHeader(newPreHeader);
        preHeader = newPreHeader;
    }
    MoveInstructions(preHeader, loop);
}

static bool FindUnsafeInstBetween(BasicBlock *domBb, BasicBlock *currBb, Marker visited, const Inst *resolver,
                                  Inst *startInst = nullptr)
{
    ASSERT(resolver->IsResolver());

    if (domBb == currBb) {
        return false;
    }
    if (currBb->SetMarker(visited)) {
        return false;
    }
    // Do not cross a try block because the resolver
    // can throw NoSuch{Method|Field}Exception
    if (currBb->IsTry()) {
        return true;
    }
    // Field and Method resolvers may call GetField and InitializeClass methods
    // of the class linker resulting to class loading/initialization.
    // An order of class initialization must be preserved, so the resolver
    // must not cross other instructions performing class initialization
    // We have to be conservative with respect to calls.
    for (auto inst : InstSafeReverseIter(*currBb, startInst)) {
        if (inst->IsClassInst() || inst->IsResolver() || inst->IsCall()) {
            return true;
        }
    }
    for (auto pred : currBb->GetPredsBlocks()) {
        if (FindUnsafeInstBetween(domBb, pred, visited, resolver)) {
            return true;
        }
    }
    return false;
}

Inst *Licm::FindSaveStateForResolver(Inst *resolver, const BasicBlock *preHeader)
{
    ASSERT(preHeader->GetSuccsBlocks().size() == 1);
    // Lookup for an appropriate SaveState in the preheader
    Inst *ss = nullptr;
    for (const auto &inst : preHeader->InstsSafeReverse()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            // Give up further checking if SaveState came from an inlined function.
            if (static_cast<SaveStateInst *>(inst)->GetCallerInst() != nullptr) {
                return nullptr;
            }
            for (auto i = preHeader->GetLastInst(); i != inst; i = i->GetPrev()) {
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
    auto instLoop = inst->GetBasicBlock()->GetLoop();
    for (auto input : inst->GetInputs()) {
        // Graph is in SSA form and the instructions not PHI,
        // so every 'input' must dominate 'inst'.
        ASSERT(input.GetInst()->IsDominate(inst));
        auto inputLoop = input.GetInst()->GetBasicBlock()->GetLoop();
        if (instLoop == inputLoop) {
            if (inst->IsResolver() && input.GetInst()->IsSaveState()) {
                // Resolver is coupled with SaveState that is not hoistable.
                // We will try to find an appropriate SaveState in the preheader.
                continue;
            }
            if (!input.GetInst()->IsMarked(markerHoistInst_)) {
                return false;
            }
        } else if (instLoop->GetOuterLoop() == inputLoop->GetOuterLoop()) {
            if (!input.GetInst()->GetBasicBlock()->IsDominate(instLoop->GetPreHeader())) {
                return false;
            }
        } else if (!instLoop->IsInside(inputLoop)) {
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
    auto instLoop = inst->GetBasicBlock()->GetLoop();
    for (auto block : instLoop->GetBlocks()) {
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
