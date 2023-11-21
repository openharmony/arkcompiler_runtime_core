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

namespace panda::compiler {
TryCatchResolving::TryCatchResolving(Graph *graph)
    : Optimization(graph),
      try_blocks_(graph->GetLocalAllocator()->Adapter()),
      throw_insts_(graph->GetLocalAllocator()->Adapter()),
      catch_blocks_(graph->GetLocalAllocator()->Adapter()),
      phi_insts_(graph->GetLocalAllocator()->Adapter()),
      cphi2phi_(graph->GetLocalAllocator()->Adapter()),
      catch2cphis_(graph->GetLocalAllocator()->Adapter())
{
}

bool TryCatchResolving::RunImpl()
{
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Running try-catch-resolving";
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsTryBegin()) {
            try_blocks_.emplace_back(bb);
        }
    }
    if (!OPTIONS.IsCompilerNonOptimizing()) {
        CollectCandidates();
        if (!catch_blocks_.empty() && !throw_insts_.empty()) {
            ConnectThrowCatch();
        }
    }
    for (auto bb : try_blocks_) {
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
            catch_blocks_.emplace(bb->GetGuestPc(), bb);
            BasicBlock *cbl_pred = FindCatchBeginBlock(bb);
            if (cbl_pred != nullptr) {
                cbl_pred->RemoveSucc(bb);
                bb->RemovePred(cbl_pred);
                catch2cphis_.emplace(bb, cbl_pred);
            }
        } else if (bb->IsTry() && GetGraph()->GetThrowCounter(bb) > 0) {
            auto throw_inst = bb->GetLastInst();
            ASSERT(throw_inst != nullptr && throw_inst->GetOpcode() == Opcode::Throw);
            throw_insts_.emplace_back(throw_inst);
        }
    }
}

void TryCatchResolving::ConnectThrowCatchImpl(BasicBlock *catch_block, BasicBlock *throw_block, uint32_t catch_pc,
                                              Inst *new_obj, Inst *thr0w)
{
    auto throw_block_succ = throw_block->GetSuccessor(0);
    throw_block->RemoveSucc(throw_block_succ);
    throw_block_succ->RemovePred(throw_block);
    throw_block->AddSucc(catch_block);
    PhiInst *phi_inst = nullptr;
    auto pit = phi_insts_.find(catch_pc);
    if (pit == phi_insts_.end()) {
        phi_inst = GetGraph()->CreateInstPhi(new_obj->GetType(), catch_pc);
        catch_block->AppendPhi(phi_inst);
        phi_insts_.emplace(catch_pc, phi_inst);
    } else {
        phi_inst = pit->second;
    }
    phi_inst->AppendInput(new_obj);
    auto cpit = catch2cphis_.find(catch_block);
    ASSERT(cpit != catch2cphis_.end());
    auto cphis_block = cpit->second;
    RemoveCatchPhis(cphis_block, catch_block, thr0w, phi_inst);
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING)
        << "throw I " << thr0w->GetId() << " BB " << throw_block->GetId() << " is connected with catch BB "
        << catch_block->GetId() << " and removed";
    throw_block->RemoveInst(thr0w);
}

void TryCatchResolving::ConnectThrowCatch()
{
    auto *graph = GetGraph();
    auto *runtime = graph->GetRuntime();
    auto *method = graph->GetMethod();
    for (auto thr0w : throw_insts_) {
        auto throw_block = thr0w->GetBasicBlock();
        auto throw_inst = thr0w->CastToThrow();
        // Inlined throws generate the problem with matching calls and returns now. NOTE Should be fixed.
        if (GetGraph()->GetThrowCounter(throw_block) == 0 || throw_inst->IsInlined()) {
            continue;
        }
        auto new_obj = thr0w->GetInput(0).GetInst();
        RuntimeInterface::ClassPtr cls = nullptr;
        if (new_obj->GetOpcode() != Opcode::NewObject) {
            continue;
        }
        auto init_class = new_obj->GetInput(0).GetInst();
        if (init_class->GetOpcode() == Opcode::LoadAndInitClass) {
            cls = init_class->CastToLoadAndInitClass()->GetClass();
        } else {
            ASSERT(init_class->GetOpcode() == Opcode::LoadImmediate);
            cls = init_class->CastToLoadImmediate()->GetClass();
        }
        if (cls == nullptr) {
            continue;
        }
        auto catch_pc = runtime->FindCatchBlock(method, cls, thr0w->GetPc());
        if (catch_pc == panda_file::INVALID_OFFSET) {
            continue;
        }
        auto cit = catch_blocks_.find(catch_pc);
        if (cit == catch_blocks_.end()) {
            continue;
        }
        ConnectThrowCatchImpl(cit->second, throw_block, catch_pc, new_obj, thr0w);
    }
}

/**
 * Search throw instruction with known at compile-time `object_id`
 * and directly connect catch-handler for this `object_id` if it exists in the current graph
 */
void TryCatchResolving::VisitTryInst(TryInst *try_inst)
{
    auto try_begin = try_inst->GetBasicBlock();
    auto try_end = try_inst->GetTryEndBlock();
    ASSERT(try_begin != nullptr && try_begin->IsTryBegin());
    ASSERT(try_end != nullptr && try_end->IsTryEnd());

    // Now, when catch-handler was searched - remove all edges from `try_begin` and `try_end` blocks
    DeleteTryCatchEdges(try_begin, try_end);
    // Clean-up labels and `try_inst`
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Erase try-inst I " << try_inst->GetId();
    try_begin->EraseInst(try_inst);
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Unset try-begin BB " << try_begin->GetId();
    try_begin->SetTryBegin(false);
    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Unset try-end BB " << try_end->GetId();
    try_end->SetTryEnd(false);
}

/// Disconnect auxiliary `try_begin` and `try_end`. That means all related catch-handlers become unreachable
void TryCatchResolving::DeleteTryCatchEdges(BasicBlock *try_begin, BasicBlock *try_end)
{
    while (try_begin->GetSuccsBlocks().size() > 1U) {
        auto catch_succ = try_begin->GetSuccessor(1U);
        ASSERT(catch_succ->IsCatchBegin());
        try_begin->RemoveSucc(catch_succ);
        catch_succ->RemovePred(try_begin);
        COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Remove edge between try_begin BB " << try_begin->GetId()
                                                 << " and catch-begin BB " << catch_succ->GetId();
        if (try_end->GetGraph() != nullptr) {
            ASSERT(try_end->GetSuccessor(1) == catch_succ);
            try_end->RemoveSucc(catch_succ);
            catch_succ->RemovePred(try_end);
            COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Remove edge between try_end BB " << try_end->GetId()
                                                     << " and catch-begin BB " << catch_succ->GetId();
        }
    }
}

void TryCatchResolving::RemoveCatchPhisImpl(CatchPhiInst *catch_phi, BasicBlock *catch_block, Inst *throw_inst)
{
    auto throw_insts = catch_phi->GetThrowableInsts();
    auto it = std::find(throw_insts->begin(), throw_insts->end(), throw_inst);
    if (it != throw_insts->end()) {
        auto input_index = std::distance(throw_insts->begin(), it);
        auto input_inst = catch_phi->GetInput(input_index).GetInst();
        PhiInst *phi = nullptr;
        auto cit = cphi2phi_.find(catch_phi);
        if (cit == cphi2phi_.end()) {
            phi = GetGraph()->CreateInstPhi(catch_phi->GetType(), catch_block->GetGuestPc())->CastToPhi();
            catch_block->AppendPhi(phi);
            catch_phi->ReplaceUsers(phi);
            cphi2phi_.emplace(catch_phi, phi);
        } else {
            phi = cit->second;
        }
        phi->AppendInput(input_inst);
    } else {
        while (!catch_phi->GetUsers().Empty()) {
            auto &user = catch_phi->GetUsers().Front();
            auto user_inst = user.GetInst();
            if (user_inst->IsSaveState() || user_inst->IsCatchPhi()) {
                user_inst->RemoveInput(user.GetIndex());
            } else {
                auto input_inst = catch_phi->GetInput(0).GetInst();
                user_inst->ReplaceInput(catch_phi, input_inst);
            }
        }
    }
}

/**
 * Replace all catch-phi instructions with their inputs
 * Replace accumulator's catch-phi with exception's object
 */
void TryCatchResolving::RemoveCatchPhis(BasicBlock *cphis_block, BasicBlock *catch_block, Inst *throw_inst,
                                        Inst *phi_inst)
{
    ASSERT(cphis_block->IsCatchBegin());
    for (auto inst : cphis_block->AllInstsSafe()) {
        if (!inst->IsCatchPhi()) {
            break;
        }
        auto catch_phi = inst->CastToCatchPhi();
        if (catch_phi->IsAcc()) {
            catch_phi->ReplaceUsers(phi_inst);
        } else {
            RemoveCatchPhisImpl(catch_phi, catch_block, throw_inst);
        }
    }
}

void TryCatchResolving::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}
}  // namespace panda::compiler
