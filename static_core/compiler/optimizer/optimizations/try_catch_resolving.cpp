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

#include "include/class.h"
#include "runtime/include/class-inl.h"
#include "compiler_logger.h"
#include "compiler/optimizer/ir/runtime_interface.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/try_catch_resolving.h"

namespace ark::compiler {
TryCatchResolving::TryCatchResolving(Graph *graph)
    : Optimization(graph),
      tryBlocks_(graph->GetLocalAllocator()->Adapter()),
      throwInsts_(graph->GetLocalAllocator()->Adapter()),
      catchBlocks_(graph->GetLocalAllocator()->Adapter()),
      phiInsts_(graph->GetLocalAllocator()->Adapter()),
      cphi2phi_(graph->GetLocalAllocator()->Adapter()),
      catch2cphis_(graph->GetLocalAllocator()->Adapter())
{
}

bool TryCatchResolving::RunImpl()
{
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Running try-catch-resolving";
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsTryBegin()) {
            tryBlocks_.emplace_back(bb);
        }
    }
    if (!g_options.IsCompilerNonOptimizing()) {
        CollectCandidates();
        if (!catchBlocks_.empty() && !throwInsts_.empty()) {
            ConnectThrowCatch();
        }
    }
    for (auto bb : tryBlocks_) {
        COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Visit try-begin BB " << bb->GetId();
        VisitTryInst(GetTryBeginInst(bb));
    }
    GetGraph()->RemoveUnreachableBlocks();
    GetGraph()->ClearTryCatchInfo();
    GetGraph()->EraseMarker(marker_);
    InvalidateAnalyses();
    // Cleanup should be done inside pass, to satisfy GraphChecker
    GetGraph()->RunPass<Cleanup>();
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Finishing try-catch-resolving";
    return true;
}

BasicBlock *TryCatchResolving::FindCatchBeginBlock(BasicBlock *bb)
{
    for (auto pred : bb->GetPredsBlocks()) {
        if (pred->IsCatchBegin()) {
            return pred;
        }
    }
    return nullptr;
}

void TryCatchResolving::CollectCandidates()
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsCatch() && !(bb->IsCatchBegin() || bb->IsCatchEnd() || bb->IsTryBegin() || bb->IsTryEnd())) {
            catchBlocks_.emplace(bb->GetGuestPc(), bb);
            BasicBlock *cblPred = FindCatchBeginBlock(bb);
            if (cblPred != nullptr) {
                cblPred->RemoveSucc(bb);
                bb->RemovePred(cblPred);
                catch2cphis_.emplace(bb, cblPred);
            }
        } else if (bb->IsTry() && GetGraph()->GetThrowCounter(bb) > 0) {
            auto throwInst = bb->GetLastInst();
            ASSERT(throwInst != nullptr && throwInst->GetOpcode() == Opcode::Throw);
            throwInsts_.emplace_back(throwInst);
        }
    }
}

void TryCatchResolving::ConnectThrowCatchImpl(BasicBlock *catchBlock, BasicBlock *throwBlock, uint32_t catchPc,
                                              Inst *newObj, Inst *thr0w)
{
    auto throwBlockSucc = throwBlock->GetSuccessor(0);
    throwBlock->RemoveSucc(throwBlockSucc);
    throwBlockSucc->RemovePred(throwBlock);
    throwBlock->AddSucc(catchBlock);
    PhiInst *phiInst = nullptr;
    auto pit = phiInsts_.find(catchPc);
    if (pit == phiInsts_.end()) {
        phiInst = GetGraph()->CreateInstPhi(newObj->GetType(), catchPc);
        catchBlock->AppendPhi(phiInst);
        phiInsts_.emplace(catchPc, phiInst);
    } else {
        phiInst = pit->second;
    }
    phiInst->AppendInput(newObj);
    auto cpit = catch2cphis_.find(catchBlock);
    ASSERT(cpit != catch2cphis_.end());
    auto cphisBlock = cpit->second;
    RemoveCatchPhis(cphisBlock, catchBlock, thr0w, phiInst);
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "throw I " << thr0w->GetId() << " BB " << throwBlock->GetId()
                                             << " is connected with catch BB " << catchBlock->GetId() << " and removed";
    throwBlock->RemoveInst(thr0w);
}

void TryCatchResolving::ConnectThrowCatch()
{
    auto *graph = GetGraph();
    auto *runtime = graph->GetRuntime();
    auto *method = graph->GetMethod();
    for (auto thr0w : throwInsts_) {
        auto throwBlock = thr0w->GetBasicBlock();
        auto throwInst = thr0w->CastToThrow();
        // Inlined throws generate the problem with matching calls and returns now. NOTE Should be fixed.
        if (GetGraph()->GetThrowCounter(throwBlock) == 0 || throwInst->IsInlined()) {
            continue;
        }
        auto newObj = thr0w->GetInput(0).GetInst();
        RuntimeInterface::ClassPtr cls = nullptr;
        if (newObj->GetOpcode() != Opcode::NewObject) {
            continue;
        }
        auto initClass = newObj->GetInput(0).GetInst();
        if (initClass->GetOpcode() == Opcode::LoadAndInitClass) {
            cls = initClass->CastToLoadAndInitClass()->GetClass();
        } else {
            ASSERT(initClass->GetOpcode() == Opcode::LoadImmediate);
            cls = initClass->CastToLoadImmediate()->GetClass();
        }
        if (cls == nullptr) {
            continue;
        }
        auto catchPc = runtime->FindCatchBlock(method, cls, thr0w->GetPc());
        if (catchPc == panda_file::INVALID_OFFSET) {
            continue;
        }
        auto cit = catchBlocks_.find(catchPc);
        if (cit == catchBlocks_.end()) {
            continue;
        }
        ConnectThrowCatchImpl(cit->second, throwBlock, catchPc, newObj, thr0w);
    }
}

/**
 * Search throw instruction with known at compile-time `object_id`
 * and directly connect catch-handler for this `object_id` if it exists in the current graph
 */
void TryCatchResolving::VisitTryInst(TryInst *tryInst)
{
    auto tryBegin = tryInst->GetBasicBlock();
    auto tryEnd = tryInst->GetTryEndBlock();
    ASSERT(tryBegin != nullptr && tryBegin->IsTryBegin());
    ASSERT(tryEnd != nullptr && tryEnd->IsTryEnd());

    // Now, when catch-handler was searched - remove all edges from `try_begin` and `try_end` blocks
    DeleteTryCatchEdges(tryBegin, tryEnd);
    // Clean-up labels and `try_inst`
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Erase try-inst I " << tryInst->GetId();
    tryBegin->EraseInst(tryInst);
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Unset try-begin BB " << tryBegin->GetId();
    tryBegin->SetTryBegin(false);
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Unset try-end BB " << tryEnd->GetId();
    tryEnd->SetTryEnd(false);
}

/// Disconnect auxiliary `try_begin` and `try_end`. That means all related catch-handlers become unreachable
void TryCatchResolving::DeleteTryCatchEdges(BasicBlock *tryBegin, BasicBlock *tryEnd)
{
    while (tryBegin->GetSuccsBlocks().size() > 1U) {
        auto catchSucc = tryBegin->GetSuccessor(1U);
        ASSERT(catchSucc->IsCatchBegin());
        tryBegin->RemoveSucc(catchSucc);
        catchSucc->RemovePred(tryBegin);
        COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING)
            << "Remove edge between try_begin BB " << tryBegin->GetId() << " and catch-begin BB " << catchSucc->GetId();
        if (tryEnd->GetGraph() != nullptr) {
            ASSERT(tryEnd->GetSuccessor(1) == catchSucc);
            tryEnd->RemoveSucc(catchSucc);
            catchSucc->RemovePred(tryEnd);
            COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING)
                << "Remove edge between try_end BB " << tryEnd->GetId() << " and catch-begin BB " << catchSucc->GetId();
        }
    }
}

void TryCatchResolving::RemoveCatchPhisImpl(CatchPhiInst *catchPhi, BasicBlock *catchBlock, Inst *throwInst)
{
    auto throwInsts = catchPhi->GetThrowableInsts();
    auto it = std::find(throwInsts->begin(), throwInsts->end(), throwInst);
    if (it != throwInsts->end()) {
        auto inputIndex = std::distance(throwInsts->begin(), it);
        auto inputInst = catchPhi->GetInput(inputIndex).GetInst();
        PhiInst *phi = nullptr;
        auto cit = cphi2phi_.find(catchPhi);
        if (cit == cphi2phi_.end()) {
            phi = GetGraph()->CreateInstPhi(catchPhi->GetType(), catchBlock->GetGuestPc())->CastToPhi();
            catchBlock->AppendPhi(phi);
            catchPhi->ReplaceUsers(phi);
            cphi2phi_.emplace(catchPhi, phi);
        } else {
            phi = cit->second;
        }
        phi->AppendInput(inputInst);
    } else {
        while (!catchPhi->GetUsers().Empty()) {
            auto &user = catchPhi->GetUsers().Front();
            auto userInst = user.GetInst();
            if (userInst->IsSaveState() || userInst->IsCatchPhi()) {
                userInst->RemoveInput(user.GetIndex());
            } else {
                auto inputInst = catchPhi->GetInput(0).GetInst();
                userInst->ReplaceInput(catchPhi, inputInst);
            }
        }
    }
}

/**
 * Replace all catch-phi instructions with their inputs
 * Replace accumulator's catch-phi with exception's object
 */
void TryCatchResolving::RemoveCatchPhis(BasicBlock *cphisBlock, BasicBlock *catchBlock, Inst *throwInst, Inst *phiInst)
{
    ASSERT(cphisBlock->IsCatchBegin());
    for (auto inst : cphisBlock->AllInstsSafe()) {
        if (!inst->IsCatchPhi()) {
            break;
        }
        auto catchPhi = inst->CastToCatchPhi();
        if (catchPhi->IsAcc()) {
            catchPhi->ReplaceUsers(phiInst);
        } else {
            RemoveCatchPhisImpl(catchPhi, catchBlock, throwInst);
        }
    }
}

void TryCatchResolving::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}
}  // namespace ark::compiler
