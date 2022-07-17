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

#include "include/class.h"
#include "runtime/include/class-inl.h"
#include "compiler_logger.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/try_catch_resolving.h"

namespace panda::compiler {
TryCatchResolving::TryCatchResolving(Graph *graph) : Optimization(graph) {}

bool TryCatchResolving::RunImpl()
{
    marker_ = GetGraph()->NewMarker();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (!block->IsTryBegin()) {
            continue;
        }
        COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING) << "Visit try-begin id = " << std::to_string(block->GetId());
        VisitTry(GetTryBeginInst(block));
    }
    GetGraph()->RemoveUnreachableBlocks();
    GetGraph()->ClearTryCatchInfo();
    GetGraph()->EraseMarker(marker_);
#ifndef NDEBUG
    InvalidateAnalyses();
    // Cleanup should be done inside pass, to satisfy GraphChecker
    GetGraph()->RunPass<Cleanup>();
#endif
    return true;
}

/**
 * Search throw instruction with known at compile-time `object_id`
 * and directly connect catch-handler for this `object_id` if it exists in the current graph
 */
void TryCatchResolving::VisitTry(TryInst *try_inst)
{
    auto try_begin = try_inst->GetBasicBlock();
    auto try_end = try_inst->GetTryEndBlock();
    ASSERT(try_begin->IsTryBegin());
    ASSERT(try_end->IsTryEnd());
    // First of all, try to find catch-handler that can be directly connected to the block with `throw`
    auto resolved_catch_handler = TryFindResolvedCatchHandler(try_begin, try_end);
    // Now, when catch-handler was searched - remove all edges from `try_begin` and `try_end` blocks
    DeleteTryCatchEdges(try_begin, try_end);
    // If resolved catch-handler exists, connect it
    if (resolved_catch_handler != nullptr) {
        ConnectCatchHandlerAfterThrow(try_end, resolved_catch_handler);
    }
    // Clean-up lables and `try_inst`
    try_begin->EraseInst(try_inst);
    try_begin->SetTryBegin(false);
    try_end->SetTryEnd(false);
}

BasicBlock *TryCatchResolving::TryFindResolvedCatchHandler(BasicBlock *try_begin, BasicBlock *try_end)
{
    // `try_end` could be removed after inlining
    if (try_end->GetPredsBlocks().size() != 1U) {
        return nullptr;
    }
    auto throw_inst = try_end->GetPredecessor(0)->GetLastInst();
    if (throw_inst == nullptr || throw_inst->GetOpcode() != Opcode::Throw) {
        return nullptr;
    }
    auto object_id = TryGetObjectId(throw_inst);
    if (!object_id) {
        return nullptr;
    }
    // There should be no side-exits from try
    auto holder = MarkerHolder(GetGraph());
    auto marker = holder.GetMarker();
    if (!DFS(try_begin->GetSuccessor(0), marker, try_begin->GetTryId())) {
        return nullptr;
    }
    // We've got `object_id` which is thrown form try-block
    // Let's find handler, which catches this `object_id`
    BasicBlock *resolved_handler = nullptr;
    try_begin->EnumerateCatchHandlers([this, object_id, &resolved_handler](BasicBlock *catch_handler, size_t type_id) {
        // We don't connect catch-handlers which are related to more than one try-block.
        // So that we skip blocks that:
        // - have more than 2 predecessors (one try-begin/end pair);
        // - were already visited from another try-block and were marked;
        if (catch_handler->GetPredsBlocks().size() > 2U || catch_handler->IsMarked(marker_)) {
            return true;
        }
        static constexpr size_t CATCH_ALL_ID = 0;
        if (type_id == CATCH_ALL_ID) {
            resolved_handler = catch_handler;
            return false;
        }
        auto runtime = GetGraph()->GetRuntime();
        auto *cls = runtime->ResolveType(GetGraph()->GetMethod(), object_id.value());
        auto *handler_cls = runtime->ResolveType(GetGraph()->GetMethod(), type_id);
        if (static_cast<Class *>(cls)->IsSubClassOf(static_cast<Class *>(handler_cls))) {
            resolved_handler = catch_handler;
            return false;
        }
        return true;
    });
    return resolved_handler;
}

/**
 * Disconnect auxiliary `try_begin` and `try_end`. That means all related catch-handlers become unreachable
 */
void TryCatchResolving::DeleteTryCatchEdges(BasicBlock *try_begin, BasicBlock *try_end)
{
    while (try_begin->GetSuccsBlocks().size() > 1U) {
        auto catch_succ = try_begin->GetSuccessor(1U);
        ASSERT(catch_succ->IsCatchBegin());
        try_begin->RemoveSucc(catch_succ);
        catch_succ->RemovePred(try_begin);
        if (try_end->GetGraph() != nullptr) {
            ASSERT(try_end->GetSuccessor(1) == catch_succ);
            try_end->RemoveSucc(catch_succ);
            catch_succ->RemovePred(try_end);
        }
        // Mark that catch-handler was visited
        catch_succ->SetMarker(marker_);
    }
}

/**
 * Add edge between catch-handler and `try_end` follows `throw` instruction
 */
void TryCatchResolving::ConnectCatchHandlerAfterThrow(BasicBlock *try_end, BasicBlock *catch_block)
{
    ASSERT(try_end != nullptr && try_end->IsTryEnd());
    ASSERT(catch_block != nullptr && catch_block->IsCatchBegin());
    ASSERT(try_end->GetPredsBlocks().size() == 1U);
    auto throw_inst = try_end->GetPredecessor(0)->GetLastInst();
    ASSERT(throw_inst != nullptr && throw_inst->GetOpcode() == Opcode::Throw);

    COMPILER_LOG(DEBUG, TRY_CATCH_RESOLVING)
        << "Connect blocks: " << std::to_string(try_end->GetId()) << " -> " << std::to_string(catch_block->GetId());

    auto succ = try_end->GetSuccessor(0);
    try_end->ReplaceSucc(succ, catch_block);
    succ->RemovePred(try_end);
    RemoveCatchPhis(catch_block, throw_inst);

    auto save_state = throw_inst->GetInput(1).GetInst();
    ASSERT(save_state->GetOpcode() == Opcode::SaveState);
    auto throw_block = throw_inst->GetBasicBlock();
    throw_block->RemoveInst(throw_inst);
    throw_block->RemoveInst(save_state);
}

/**
 * Replace all catch-phi instructions with their inputs
 * Replace accumulator's catch-phi with exception's object
 */
void TryCatchResolving::RemoveCatchPhis(BasicBlock *block, Inst *throw_inst)
{
    ASSERT(block->IsCatchBegin());
    for (auto inst : block->AllInstsSafe()) {
        if (!inst->IsCatchPhi()) {
            break;
        }
        auto catch_phi = inst->CastToCatchPhi();
        if (catch_phi->IsAcc()) {
            auto throw_obj = throw_inst->GetInput(0).GetInst();
            catch_phi->ReplaceUsers(throw_obj);
        } else {
            auto throw_insts = catch_phi->GetThrowableInsts();
            auto it = std::find(throw_insts->begin(), throw_insts->end(), throw_inst);
            if (it != throw_insts->end()) {
                auto input_index = std::distance(throw_insts->begin(), it);
                auto input_inst = catch_phi->GetInput(input_index).GetInst();
                catch_phi->ReplaceUsers(input_inst);
            } else {
                // Virtual register related to the catch-phi is undefined, so that there should be no real users
                auto users = catch_phi->GetUsers();
                while (!users.Empty()) {
                    auto &user = users.Front();
                    ASSERT(user.GetInst()->IsSaveState() || user.GetInst()->IsCatchPhi());
                    user.GetInst()->RemoveInput(user.GetIndex());
                }
            }
        }
        block->RemoveInst(catch_phi);
    }
    block->SetCatch(false);
    block->SetCatchBegin(false);
}

/**
 * Return object's id if `Throw` has `NewObject` input
 */
std::optional<uint32_t> TryCatchResolving::TryGetObjectId(const Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Throw);
    auto ref_input = inst->GetInput(0).GetInst();
    if (ref_input->GetOpcode() == Opcode::NewObject) {
        return ref_input->CastToNewObject()->GetTypeId();
    }
    return std::nullopt;
}

bool TryCatchResolving::DFS(BasicBlock *block, Marker marker, uint32_t try_id)
{
    if (block->GetTryId() != try_id) {
        return false;
    }
    if (block->IsTryEnd() && block->GetTryId() == try_id) {
        return true;
    }
    block->SetMarker(marker);
    for (auto succ : block->GetSuccsBlocks()) {
        if (succ->IsMarked(marker)) {
            continue;
        }
        if (!DFS(succ, marker, try_id)) {
            return false;
        }
    }
    return true;
}

void TryCatchResolving::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}
}  // namespace panda::compiler
