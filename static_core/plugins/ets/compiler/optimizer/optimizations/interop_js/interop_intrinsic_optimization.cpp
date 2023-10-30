/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "interop_intrinsic_optimization.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "plugins/ets/compiler/generated/interop_intrinsic_kinds.h"

namespace panda::compiler {

static bool IsForbiddenInst(Inst *inst)
{
    return inst->IsCall() && !static_cast<CallInst *>(inst)->IsInlined();
}

static bool IsScopeStart(Inst *inst)
{
    return inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                      RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CREATE_LOCAL_SCOPE;
}

static bool IsScopeEnd(Inst *inst)
{
    return inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                      RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_DESTROY_LOCAL_SCOPE;
}

static bool IsWrapIntrinsicId(RuntimeInterface::IntrinsicId id)
{
    return GetInteropIntrinsicKind(id) == InteropIntrinsicKind::WRAP;
}

static bool IsUnwrapIntrinsicId(RuntimeInterface::IntrinsicId id)
{
    return GetInteropIntrinsicKind(id) == InteropIntrinsicKind::UNWRAP;
}

static RuntimeInterface::IntrinsicId GetUnwrapIntrinsicId(RuntimeInterface::IntrinsicId id)
{
    switch (id) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_DOUBLE:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_F64;
        case RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_BOOLEAN:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_U1;
        case RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_STRING:
            return RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_STRING;
        default:
            return RuntimeInterface::IntrinsicId::INVALID;
    }
}

static bool IsConvertIntrinsic(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }
    auto id = inst->CastToIntrinsic()->GetIntrinsicId();
    return IsWrapIntrinsicId(id) || IsUnwrapIntrinsicId(id);
}

static bool IsInteropIntrinsic(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }
    auto id = inst->CastToIntrinsic()->GetIntrinsicId();
    return GetInteropIntrinsicKind(id) != InteropIntrinsicKind::NONE;
}

static bool CanCreateNewScopeObject(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }
    auto kind = GetInteropIntrinsicKind(inst->CastToIntrinsic()->GetIntrinsicId());
    return kind == InteropIntrinsicKind::WRAP || kind == InteropIntrinsicKind::CREATES_LOCAL;
}

InteropIntrinsicOptimization::BlockInfo &InteropIntrinsicOptimization::GetInfo(BasicBlock *block)
{
    return block_info_[block->GetId()];
}

void InteropIntrinsicOptimization::MergeScopesInsideBlock(BasicBlock *block)
{
    Inst *last_start = nullptr;  // Start of the current scope or nullptr if no scope is open
    Inst *last_end = nullptr;    // End of the last closed scope
    // Number of object creations in the last closed scope (valid value if we are in the next scope now)
    uint32_t last_object_count = 0;
    uint32_t current_object_count = 0;  // Number of object creations in the current scope or 0
    for (auto *inst : block->InstsSafe()) {
        if (IsScopeStart(inst)) {
            ASSERT(last_start == nullptr);
            ASSERT(current_object_count == 0);
            has_scopes_ = true;
            last_start = inst;
        } else if (IsScopeEnd(inst)) {
            ASSERT(last_start != nullptr);
            if (last_end != nullptr && last_object_count + current_object_count <= scope_object_limit_) {
                ASSERT(last_end->IsPrecedingInSameBlock(last_start));
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Remove scope end " << *last_end << "\nand scope start "
                                                           << *last_start << "\nfrom BB " << block->GetId();
                block->RemoveInst(last_end);
                block->RemoveInst(last_start);
                last_object_count += current_object_count;
                is_applied_ = true;
            } else {
                object_limit_hit_ |= (last_end != nullptr);
                last_object_count = current_object_count;
            }
            current_object_count = 0;

            last_end = inst;
            last_start = nullptr;
        } else if (IsForbiddenInst(inst)) {
            ASSERT(last_start == nullptr);
            last_end = nullptr;
            last_object_count = 0;
            current_object_count = 0;
        } else if (CanCreateNewScopeObject(inst)) {
            ASSERT(last_start != nullptr);
            ++current_object_count;
        }
    }
}

bool InteropIntrinsicOptimization::TryCreateSingleScope(BasicBlock *bb)
{
    bool is_start = bb == GetGraph()->GetStartBlock()->GetSuccessor(0);
    bool has_start = false;
    auto last_inst = bb->GetLastInst();
    // We do not need to close scope in compiler before deoptimize or throw inst
    bool is_end = last_inst != nullptr && last_inst->IsReturn();
    SaveStateInst *ss = nullptr;
    Inst *last_end = nullptr;
    for (auto *inst : bb->InstsSafe()) {
        if (is_start && !has_start && IsScopeStart(inst)) {
            has_start = true;
        } else if (is_end && IsScopeEnd(inst)) {
            if (last_end != nullptr) {
                bb->RemoveInst(last_end);
            }
            last_end = inst;
        } else if (IsScopeStart(inst) || IsScopeEnd(inst)) {
            bb->RemoveInst(inst);
        } else if (is_end && inst->GetOpcode() == Opcode::SaveState && ss == nullptr) {
            ss = inst->CastToSaveState();
        }
    }
    if (!is_end || last_end != nullptr) {
        return has_start;
    }
    if (ss == nullptr) {
        ss = GetGraph()->CreateInstSaveState(DataType::NO_TYPE, last_inst->GetPc());
        last_inst->InsertBefore(ss);
        if (last_inst->GetOpcode() == Opcode::Return && last_inst->GetInput(0).GetInst()->IsMovableObject()) {
            ss->AppendBridge(last_inst->GetInput(0).GetInst());
        }
    }
    auto scope_end = GetGraph()->CreateInstIntrinsic(
        DataType::VOID, ss->GetPc(), RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_DESTROY_LOCAL_SCOPE);
    scope_end->SetInputs(GetGraph()->GetAllocator(), {{ss, DataType::NO_TYPE}});
    ss->InsertAfter(scope_end);
    return has_start;
}

bool InteropIntrinsicOptimization::TryCreateSingleScope()
{
    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : bb->Insts()) {
            if (try_single_scope_ && IsForbiddenInst(inst)) {
                try_single_scope_ = false;
            }
            if (IsScopeStart(inst)) {
                has_scopes_ = true;
            }
        }
    }
    if (!has_scopes_ || !try_single_scope_) {
        return false;
    }
    bool has_start = false;
    for (auto *bb : GetGraph()->GetBlocksRPO()) {
        has_start |= TryCreateSingleScope(bb);
    }
    if (has_start) {
        return true;
    }
    auto *start_block = GetGraph()->GetStartBlock()->GetSuccessor(0);
    SaveStateInst *ss = nullptr;
    for (auto *inst : start_block->InstsReverse()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            ss = inst->CastToSaveState();
            break;
        }
    }
    if (ss == nullptr) {
        ss = GetGraph()->CreateInstSaveState(DataType::NO_TYPE, start_block->GetFirstInst()->GetPc());
        start_block->PrependInst(ss);
        for (auto *inst : GetGraph()->GetStartBlock()->Insts()) {
            if (inst->IsMovableObject()) {
                ss->AppendBridge(inst);
            }
        }
    }
    auto scope_start = GetGraph()->CreateInstIntrinsic(
        DataType::VOID, ss->GetPc(), RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CREATE_LOCAL_SCOPE);
    scope_start->SetInputs(GetGraph()->GetAllocator(), {{ss, DataType::NO_TYPE}});
    ss->InsertAfter(scope_start);
    return true;
}

// Returns estimated object creations in loop if it can be moved into scope or std::nullopt otherwise
std::optional<uint64_t> InteropIntrinsicOptimization::FindForbiddenLoops(Loop *loop)
{
    bool forbidden = false;
    uint64_t objects = 0;
    for (auto *inner_loop : loop->GetInnerLoops()) {
        auto inner_objects = FindForbiddenLoops(inner_loop);
        if (inner_objects.has_value()) {
            objects += *inner_objects;
        } else {
            forbidden = true;
        }
    }
    if (forbidden || loop->IsIrreducible()) {
        forbidden_loops_.insert(loop);
        return std::nullopt;
    }
    for (auto *block : loop->GetBlocks()) {
        for (auto *inst : block->Insts()) {
            if (IsForbiddenInst(inst)) {
                forbidden_loops_.insert(loop);
                return std::nullopt;
            }
            if (CanCreateNewScopeObject(inst)) {
                ++objects;
            }
        }
    }
    if (objects == 0) {
        return 0;
    }
    if (objects > scope_object_limit_) {
        forbidden_loops_.insert(loop);
        return std::nullopt;
    }
    if (auto loop_info = CountableLoopParser(*loop).Parse()) {
        auto opt_iterations = CountableLoopParser::GetLoopIterations(*loop_info);
        if (opt_iterations.has_value() &&
            *opt_iterations <= scope_object_limit_ / objects) {  // compare using division to avoid overflow
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Found small countable loop, id = " << loop->GetId() << ", iterations = " << *opt_iterations;
            return objects * *opt_iterations;
        }
    }
    forbidden_loops_.insert(loop);
    return std::nullopt;
}

bool InteropIntrinsicOptimization::IsForbiddenLoopEntry(BasicBlock *block)
{
    return block->GetLoop()->IsIrreducible() || (block->IsLoopHeader() && forbidden_loops_.count(block->GetLoop()) > 0);
}

template <bool REVERSE>
static auto GetInstsIter(BasicBlock *block)
{
    if constexpr (REVERSE) {
        return block->InstsReverse();
    } else {
        return block->Insts();
    }
}

// Implementation of disjoint set union with path compression
int32_t InteropIntrinsicOptimization::GetParentComponent(int32_t component)
{
    auto &parent = components_[component].parent;
    if (parent != component) {
        parent = GetParentComponent(parent);
    }
    ASSERT(parent != DFS_NOT_VISITED);
    return parent;
}

void InteropIntrinsicOptimization::MergeComponents(int32_t first, int32_t second)
{
    first = GetParentComponent(first);
    second = GetParentComponent(second);
    if (first != second) {
        components_[first].object_count += components_[second].object_count;
        components_[second].parent = first;
    }
}

void InteropIntrinsicOptimization::UpdateStatsForMerging(Inst *inst, int32_t other_end_component,
                                                         uint32_t *scope_switches,
                                                         uint32_t *objects_in_block_after_merge)
{
    if (IsScopeStart(inst) || IsScopeEnd(inst)) {
        ++*scope_switches;
    } else if (CanCreateNewScopeObject(inst)) {
        if (*scope_switches == 1) {
            ++*objects_in_block_after_merge;
        } else if (*scope_switches == 0 && other_end_component != current_component_) {
            // If both ends of the block belong to the same component, count instructions only from
            // the first visited end
            ++components_[current_component_].object_count;
        }
    } else if (IsForbiddenInst(inst)) {
        if (*scope_switches == 1) {
            can_merge_ = false;
        } else if (*scope_switches == 0) {
            COMPILER_LOG_IF(!components_[current_component_].is_forbidden, DEBUG, INTEROP_INTRINSIC_OPT)
                << "Connected component " << current_component_
                << " cannot be moved into scope because it contains forbidden inst " << *inst;
            components_[current_component_].is_forbidden = true;
        }
    }
}

template <bool IS_END>
void InteropIntrinsicOptimization::IterateBlockFromBoundary(BasicBlock *block)
{
    auto other_end_component = GetInfo(block).block_component[static_cast<int32_t>(!IS_END)];
    if (other_end_component != current_component_) {
        current_component_blocks_.push_back(block);
    }
    uint32_t scope_switches = 0;
    uint32_t objects_in_block_after_merge = 0;
    for (auto *inst : GetInstsIter<IS_END>(block)) {
        UpdateStatsForMerging(inst, other_end_component, &scope_switches, &objects_in_block_after_merge);
    }
    if (scope_switches == 0) {
        if (block->IsStartBlock() || block->IsEndBlock()) {
            COMPILER_LOG_IF(!components_[current_component_].is_forbidden, DEBUG, INTEROP_INTRINSIC_OPT)
                << "Connected component " << current_component_ << " cannot be moved into scope because it contains "
                << (block->IsStartBlock() ? "start" : "end") << " block " << block->GetId();
            components_[current_component_].is_forbidden = true;
        }
        BlockBoundaryDfs<!IS_END>(block);
    } else if (scope_switches == 1) {
        // Other end of the block was already moved into scope (otherwise scope switch count in block would be even)
        ASSERT(other_end_component != DFS_NOT_VISITED && other_end_component != current_component_);
        other_end_component = GetParentComponent(other_end_component);
        if (components_[other_end_component].is_forbidden) {
            can_merge_ = false;
        } else {
            objects_in_scope_after_merge_ +=
                GetObjectCountIfUnused(components_[other_end_component], current_component_);
        }
    } else if (scope_switches > 2U || other_end_component != current_component_) {
        objects_in_scope_after_merge_ += objects_in_block_after_merge;
    }
}

template <bool IS_END>
void InteropIntrinsicOptimization::BlockBoundaryDfs(BasicBlock *block)
{
    auto index = static_cast<int32_t>(IS_END);
    if (GetInfo(block).block_component[index] != DFS_NOT_VISITED) {
        return;
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Added " << (IS_END ? "end" : "start ") << " of block "
                                               << block->GetId() << " to connected component " << current_component_;
    GetInfo(block).block_component[index] = current_component_;
    if (!IS_END && IsForbiddenLoopEntry(block)) {
        COMPILER_LOG_IF(!components_[current_component_].is_forbidden, DEBUG, INTEROP_INTRINSIC_OPT)
            << "Connected component " << current_component_
            << " cannot be moved into scope because it contains entry to forbidden loop " << block->GetLoop()->GetId()
            << " via BB " << block->GetId();
        components_[current_component_].is_forbidden = true;
    }
    IterateBlockFromBoundary<IS_END>(block);

    auto &neighbours = IS_END ? block->GetSuccsBlocks() : block->GetPredsBlocks();
    for (auto *neighbour : neighbours) {
        BlockBoundaryDfs<!IS_END>(neighbour);
    }
}

static bool MoveBlockStartIntoScope(BasicBlock *block)
{
    bool removed = false;
    for (auto *inst : block->InstsSafe()) {
        if (IsScopeStart(inst)) {
            ASSERT(!removed);
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Remove first scope start " << *inst << "\nfrom BB " << block->GetId();
            block->RemoveInst(inst);
            removed = true;
        }
        if (IsScopeEnd(inst)) {
            ASSERT(removed);
            return false;
        }
    }
    return removed;
}

static bool MoveBlockEndIntoScope(BasicBlock *block)
{
    bool removed = false;
    for (auto *inst : block->InstsSafeReverse()) {
        if (IsScopeEnd(inst)) {
            ASSERT(!removed);
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Remove last scope end " << *inst << "\nfrom BB " << block->GetId();
            block->RemoveInst(inst);
            removed = true;
        }
        if (IsScopeStart(inst)) {
            ASSERT(removed);
            return false;
        }
    }
    return removed;
}

void InteropIntrinsicOptimization::MergeCurrentComponentWithNeighbours()
{
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "Merging connected component " << current_component_ << " with its neighbours";
    for (auto *block : current_component_blocks_) {
        if (GetInfo(block).block_component[0] == current_component_ && MoveBlockStartIntoScope(block)) {
            MergeComponents(current_component_, GetInfo(block).block_component[1]);
        }
        if (GetInfo(block).block_component[1] == current_component_ && MoveBlockEndIntoScope(block)) {
            MergeComponents(current_component_, GetInfo(block).block_component[0]);
        }
    }
    is_applied_ = true;
}

template <bool IS_END>
void InteropIntrinsicOptimization::FindComponentAndTryMerge(BasicBlock *block)
{
    auto index = static_cast<int32_t>(IS_END);
    if (GetInfo(block).block_component[index] != DFS_NOT_VISITED) {
        return;
    }
    components_.push_back({current_component_, 0, 0, false});
    current_component_blocks_.clear();
    objects_in_scope_after_merge_ = 0;
    can_merge_ = true;
    BlockBoundaryDfs<IS_END>(block);
    if (components_[current_component_].is_forbidden) {
        can_merge_ = false;
    }
    if (can_merge_) {
        objects_in_scope_after_merge_ += components_[current_component_].object_count;
        if (objects_in_scope_after_merge_ > scope_object_limit_) {
            object_limit_hit_ = true;
        } else {
            MergeCurrentComponentWithNeighbours();
        }
    }
    ++current_component_;
}

void InteropIntrinsicOptimization::MergeInterScopeRegions()
{
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        FindComponentAndTryMerge<false>(block);
        FindComponentAndTryMerge<true>(block);
    }
}

// Numbering is similar to pre-order, but we stop in blocks with scope starts
void InteropIntrinsicOptimization::DfsNumbering(BasicBlock *block)
{
    if (GetInfo(block).dfs_num_in != DFS_NOT_VISITED) {
        return;
    }
    GetInfo(block).dfs_num_in = dfs_num_++;
    for (auto inst : block->Insts()) {
        ASSERT(!IsScopeEnd(inst));
        if (IsScopeStart(inst)) {
            GetInfo(block).dfs_num_out = dfs_num_;
            return;
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        DfsNumbering(succ);
    }
    GetInfo(block).dfs_num_out = dfs_num_;
}

// Calculate minimal and maximal `dfs_num_in` for blocks that can be reached by walking some edges forward
// and, after that, maybe one edge backward
// Visit order will be the same as in DfsNumbering
// We walk the graph again because during the first DFS some numbers for predecessors may be invalid yet
void InteropIntrinsicOptimization::CalculateReachabilityRec(BasicBlock *block)
{
    if (block->SetMarker(visited_)) {
        return;
    }
    auto &info = GetInfo(block);
    info.subgraph_min_num = info.subgraph_max_num = info.dfs_num_in;

    bool is_scope_start = false;
    for (auto inst : block->Insts()) {
        ASSERT(!IsScopeEnd(inst));
        if (IsForbiddenInst(inst)) {
            info.subgraph_min_num = DFS_NOT_VISITED;
        }
        if (IsScopeStart(inst)) {
            is_scope_start = true;
            break;
        }
    }

    if (!is_scope_start) {
        for (auto *succ : block->GetSuccsBlocks()) {
            CalculateReachabilityRec(succ);
            info.subgraph_min_num = std::min(info.subgraph_min_num, GetInfo(succ).subgraph_min_num);
            info.subgraph_max_num = std::max(info.subgraph_max_num, GetInfo(succ).subgraph_max_num);
            if (IsForbiddenLoopEntry(succ)) {
                info.subgraph_min_num = DFS_NOT_VISITED;
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << "BB " << block->GetId() << " cannot be moved into scope because succ " << succ->GetId()
                    << " is entry to forbidden loop " << succ->GetLoop()->GetId();
                break;
            }
        }
        if (info.dfs_num_in <= info.subgraph_min_num && info.subgraph_max_num < info.dfs_num_out) {
            block->SetMarker(can_hoist_to_);
        }
    }
    for (auto *pred : block->GetPredsBlocks()) {
        if (pred->IsMarked(start_dfs_)) {
            info.subgraph_min_num = DFS_NOT_VISITED;
            break;
        }
        info.subgraph_min_num = std::min(info.subgraph_min_num, GetInfo(pred).dfs_num_in);
        info.subgraph_max_num = std::max(info.subgraph_max_num, GetInfo(pred).dfs_num_in);
    }
}

template <void (InteropIntrinsicOptimization::*DFS)(BasicBlock *)>
void InteropIntrinsicOptimization::DoDfs()
{
    // We launch DFS in a graph where vertices correspond to block starts not contained in scopes, and
    // vertices for bb1 and bb2 are connected by a (directed) edge if bb1 and bb2 are connected in CFG and
    // there are no scopes in bb1

    for (auto *block : GetGraph()->GetBlocksRPO()) {
        // We mark block with `start_dfs_` marker if its **end** is contained in a new inter-scope region
        // (i. e. block is the start block or last scope switch in block is scope end)
        // And since our graph contains **starts** of blocks, we launch DFS from succs, not from the block itself
        if (block->IsMarked(start_dfs_)) {
            for (auto *succ : block->GetSuccsBlocks()) {
                (this->*DFS)(succ);
            }
        }
    }
}

bool InteropIntrinsicOptimization::CreateScopeStartInBlock(BasicBlock *block)
{
    for (auto *inst : block->InstsSafeReverse()) {
        ASSERT(!IsForbiddenInst(inst) && !IsScopeStart(inst) && !IsScopeEnd(inst));
        if (inst->GetOpcode() == Opcode::SaveState) {
            auto scope_start = GetGraph()->CreateInstIntrinsic(
                DataType::VOID, inst->GetPc(), RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CREATE_LOCAL_SCOPE);
            scope_start->SetInputs(GetGraph()->GetAllocator(), {{inst, DataType::NO_TYPE}});
            inst->InsertAfter(scope_start);
            is_applied_ = true;
            return true;
        }
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Failed to create new scope start in BB " << block->GetId();
    return false;
}

void InteropIntrinsicOptimization::RemoveReachableScopeStarts(BasicBlock *block, BasicBlock *new_start_block)
{
    ASSERT(!block->IsEndBlock());
    if (block->SetMarker(visited_)) {
        return;
    }
    block->ResetMarker(can_hoist_to_);
    ASSERT(new_start_block->IsDominate(block));
    if (block != new_start_block) {
        ASSERT(!IsForbiddenLoopEntry(block));
        for (auto *inst : block->Insts()) {
            ASSERT(!IsForbiddenInst(inst) && !IsScopeEnd(inst));
            if (IsScopeStart(inst)) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << "Removed scope start " << *inst << "from BB " << block->GetId() << ", new scope start in "
                    << new_start_block->GetId();
                block->RemoveInst(inst);
                return;
            }
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        RemoveReachableScopeStarts(succ, new_start_block);
    }
}

void InteropIntrinsicOptimization::HoistScopeStarts()
{
    auto start_dfs_holder = MarkerHolder(GetGraph());
    start_dfs_ = start_dfs_holder.GetMarker();
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        bool end_in_scope = false;
        for (auto inst : block->InstsReverse()) {
            if (IsScopeEnd(inst)) {
                block->SetMarker(start_dfs_);
                break;
            }
            if (IsScopeStart(inst)) {
                end_in_scope = true;
                break;
            }
        }
        if (block->IsStartBlock() && !end_in_scope) {
            block->SetMarker(start_dfs_);
        }
    }
    DoDfs<&InteropIntrinsicOptimization::DfsNumbering>();
    auto can_hoist_to_holder = MarkerHolder(GetGraph());
    can_hoist_to_ = can_hoist_to_holder.GetMarker();
    {
        auto visited_holder = MarkerHolder(GetGraph());
        visited_ = visited_holder.GetMarker();
        DoDfs<&InteropIntrinsicOptimization::CalculateReachabilityRec>();
    }

    for (auto *block : GetGraph()->GetBlocksRPO()) {
        if (block->IsMarked(can_hoist_to_) && CreateScopeStartInBlock(block)) {
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Hoisted scope start to BB " << block->GetId() << ", removing scope starts reachable from it";
            auto visited_holder = MarkerHolder(GetGraph());
            visited_ = visited_holder.GetMarker();
            RemoveReachableScopeStarts(block, block);
        }
    }
}

void InteropIntrinsicOptimization::InvalidateScopesInSubgraph(BasicBlock *block)
{
    ASSERT(!block->IsEndBlock());
    if (block->SetMarker(scope_start_invalidated_)) {
        return;
    }
    for (auto *inst : block->Insts()) {
        ASSERT(!IsScopeStart(inst));
        if (IsScopeEnd(inst)) {
            return;
        }
        if (IsInteropIntrinsic(inst)) {
            scope_for_inst_[inst] = nullptr;
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        InvalidateScopesInSubgraph(succ);
    }
}

void InteropIntrinsicOptimization::CheckGraphRec(BasicBlock *block, Inst *scope_start)
{
    if (block->SetMarker(visited_)) {
        if (GetInfo(block).last_scope_start != scope_start) {
            // It's impossible to have scope start in one path to block and to have no scope in another path
            ASSERT(GetInfo(block).last_scope_start != nullptr);
            ASSERT(scope_start != nullptr);
            // Different scope starts for different execution paths are possible
            // NOTE(aefremov): find insts with always equal scopes somehow
            InvalidateScopesInSubgraph(block);
        }
        return;
    }
    GetInfo(block).last_scope_start = scope_start;
    for (auto *inst : block->Insts()) {
        if (IsScopeEnd(inst)) {
            ASSERT(scope_start != nullptr);
            scope_start = nullptr;
        } else if (IsScopeStart(inst)) {
            ASSERT(scope_start == nullptr);
            scope_start = inst;
        } else if (IsForbiddenInst(inst)) {
            ASSERT(scope_start == nullptr);
        } else if (IsInteropIntrinsic(inst)) {
            ASSERT(scope_start != nullptr);
            scope_for_inst_[inst] = scope_start;
        }
    }
    if (block->IsEndBlock()) {
        ASSERT(scope_start == nullptr);
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        CheckGraphRec(succ, scope_start);
    }
}

void InteropIntrinsicOptimization::CheckGraphAndFindScopes()
{
    auto visited_holder = MarkerHolder(GetGraph());
    visited_ = visited_holder.GetMarker();
    auto invalidated_holder = MarkerHolder(GetGraph());
    scope_start_invalidated_ = invalidated_holder.GetMarker();
    CheckGraphRec(GetGraph()->GetStartBlock(), nullptr);
}

void InteropIntrinsicOptimization::MarkRequireRegMap(Inst *inst)
{
    SaveStateInst *save_state = nullptr;
    if (inst->IsSaveState()) {
        save_state = static_cast<SaveStateInst *>(inst);
    } else if (inst->RequireState()) {
        save_state = inst->GetSaveState();
    }
    while (save_state != nullptr && save_state->SetMarker(require_reg_map_) && save_state->GetCallerInst() != nullptr) {
        save_state = save_state->GetCallerInst()->GetSaveState();
    }
}

void InteropIntrinsicOptimization::TryRemoveUnwrapAndWrap(Inst *inst, Inst *input)
{
    if (scope_for_inst_.at(inst) == nullptr || scope_for_inst_.at(inst) != scope_for_inst_.at(input)) {
        COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
            << "Scopes don't match, skip: wrap " << *inst << "\nwith unwrap input " << *input;
        return;
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "Remove wrap " << *inst << "\nwith unwrap input " << *input;
    auto *new_input = input->GetInput(0).GetInst();
    // We don't extend scopes out of basic blocks in OSR mode
    ASSERT(!GetGraph()->IsOsrMode() || inst->GetBasicBlock() == new_input->GetBasicBlock());
    ASSERT(new_input->GetType() == DataType::POINTER);
    ASSERT(inst->GetType() == DataType::POINTER);
    inst->ReplaceUsers(new_input);
    inst->GetBasicBlock()->RemoveInst(inst);
    // If `input` (unwrap from local to JSValue or ets primitve) has SaveState users which require regmap,
    // we will not delete the unwrap intrinsic
    // NOTE(aefremov): support unwrap (local => JSValue/primitive) during deoptimization
    is_applied_ = true;
}

void InteropIntrinsicOptimization::TryRemoveUnwrapToJSValue(Inst *inst)
{
    auto common_id = RuntimeInterface::IntrinsicId::INVALID;
    DataType::Type user_type;
    for (auto &user : inst->GetUsers()) {
        auto *user_inst = user.GetInst();
        if (user_inst->IsSaveState()) {
            // see the previous note
            if (user_inst->IsMarked(require_reg_map_)) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << "User " << *user_inst << "\nof inst " << *inst << " requires regmap, skip";
                return;
            }
            continue;
        }
        if (!user_inst->IsIntrinsic()) {
            return;
        }
        if (HasOsrEntryBetween(inst, user_inst)) {
            return;
        }
        auto current_id = user_inst->CastToIntrinsic()->GetIntrinsicId();
        if (common_id == RuntimeInterface::IntrinsicId::INVALID) {
            user_type = user_inst->GetType();
            common_id = current_id;
        } else if (current_id != common_id) {
            return;
        }
    }
    auto new_id = GetUnwrapIntrinsicId(common_id);
    if (new_id == RuntimeInterface::IntrinsicId::INVALID) {
        // includes the case when common_id is INVALID
        return;
    }
    inst->CastToIntrinsic()->SetIntrinsicId(new_id);
    inst->SetOpcode(Opcode::Intrinsic);  // reset flags to default for intrinsic inst
    AdjustFlags(new_id, inst);
    inst->SetType(user_type);
    for (auto user_it = inst->GetUsers().begin(); user_it != inst->GetUsers().end();) {
        auto user_inst = user_it->GetInst();
        if (user_inst->IsIntrinsic() && user_inst->CastToIntrinsic()->GetIntrinsicId() == common_id) {
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "Replace cast from JSValue " << *user_inst << "\nwith direct unwrap " << *inst;
            user_inst->ReplaceUsers(inst);
            user_inst->GetBasicBlock()->RemoveInst(user_inst);
            user_it = inst->GetUsers().begin();
        } else {
            ++user_it;
        }
    }
    is_applied_ = true;
}

void InteropIntrinsicOptimization::TryRemoveIntrinsic(Inst *inst)
{
    if (!inst->IsIntrinsic()) {
        return;
    }
    auto *input = inst->GetInput(0).GetInst();
    auto intrinsic_id = inst->CastToIntrinsic()->GetIntrinsicId();
    if (input->IsIntrinsic() && IsWrapIntrinsicId(intrinsic_id) &&
        IsUnwrapIntrinsicId(input->CastToIntrinsic()->GetIntrinsicId())) {
        TryRemoveUnwrapAndWrap(inst, input);
    } else if (intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_JS_VALUE) {
        TryRemoveUnwrapToJSValue(inst);
    } else if (intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_JS_CALL_FUNCTION &&
               !inst->HasUsers()) {
        // avoid creation of handle for return value in local scope if it is unused
        inst->SetType(DataType::VOID);
        inst->CastToIntrinsic()->SetIntrinsicId(
            RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_JS_CALL_VOID_FUNCTION);
        is_applied_ = true;
    }
}

void InteropIntrinsicOptimization::EliminateCastPairs()
{
    auto require_reg_map_holder = MarkerHolder(GetGraph());
    require_reg_map_ = require_reg_map_holder.GetMarker();
    auto &blocks_rpo = GetGraph()->GetBlocksRPO();
    for (auto it = blocks_rpo.rbegin(); it != blocks_rpo.rend(); ++it) {
        auto *block = *it;
        for (auto inst : block->InstsSafeReverse()) {
            if (inst->RequireRegMap()) {
                MarkRequireRegMap(inst);
            }
            TryRemoveIntrinsic(inst);
        }
    }
}

void InteropIntrinsicOptimization::DomTreeDfs(BasicBlock *block)
{
    // bb1->IsDominate(bb2) iff bb1.dom_tree_in <= bb2.dom_tree_in < bb1.dom_tree_out
    GetInfo(block).dom_tree_in = dom_tree_num_++;
    for (auto *dom : block->GetDominatedBlocks()) {
        DomTreeDfs(dom);
    }
    GetInfo(block).dom_tree_out = dom_tree_num_;
}

void InteropIntrinsicOptimization::MarkPartiallyAnticipated(BasicBlock *block, BasicBlock *stop_block)
{
    if (block->SetMarker(inst_anticipated_)) {
        return;
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "Mark block " << block->GetId() << " where current inst is partially anticipated";
    GetInfo(block).subgraph_min_num = DFS_NOT_VISITED;
    GetInfo(block).max_chain = 0;
    GetInfo(block).max_depth = -1L;
    if (block == stop_block) {
        return;
    }
    ASSERT(!block->IsStartBlock());
    for (auto *pred : block->GetPredsBlocks()) {
        MarkPartiallyAnticipated(pred, stop_block);
    }
}

void InteropIntrinsicOptimization::CalculateDownSafe(BasicBlock *block)
{
    auto &info = GetInfo(block);
    if (!block->IsMarked(inst_anticipated_)) {
        info.max_chain = 0;
        info.max_depth = -1L;
        info.subgraph_min_num = DFS_NOT_VISITED;
        return;
    }
    if (info.subgraph_min_num != DFS_NOT_VISITED) {
        return;
    }
    ASSERT(info.dom_tree_in >= 0);
    info.subgraph_min_num = info.subgraph_max_num = info.dom_tree_in;
    int32_t succ_max_chain = 0;
    for (auto *succ : block->GetSuccsBlocks()) {
        CalculateDownSafe(succ);
        succ_max_chain = std::max(succ_max_chain, GetInfo(succ).max_chain);
        if (!block->IsMarked(elimination_candidate_)) {
            info.subgraph_min_num = std::min(info.subgraph_min_num, GetInfo(succ).subgraph_min_num);
            info.subgraph_max_num = std::max(info.subgraph_max_num, GetInfo(succ).subgraph_max_num);
        }
    }
    for (auto *dom : block->GetSuccsBlocks()) {
        if (dom->IsMarked(inst_anticipated_)) {
            info.max_depth = std::max(info.max_depth, GetInfo(dom).max_depth);
        }
    }
    info.max_chain += succ_max_chain;
}

void InteropIntrinsicOptimization::ReplaceInst(Inst *inst, Inst **new_inst, Inst *insert_after)
{
    ASSERT(inst != nullptr);
    ASSERT(*new_inst != nullptr);
    ASSERT((*new_inst)->IsDominate(inst));
    if ((*new_inst)->GetOpcode() == Opcode::SaveState) {
        ASSERT(insert_after != nullptr);
        auto old_next_inst = inst->GetNext();
        auto *block = inst->GetBasicBlock();
        block->EraseInst(inst);
        insert_after->InsertAfter(inst);
        inst->SetSaveState(*new_inst);
        *new_inst = inst;
        if (inst->IsReferenceOrAny()) {
            // SSB is needed for conversion from local to JSValue or other ref type
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), *new_inst, old_next_inst, nullptr, block);
        }
    } else {
        ASSERT(inst->GetOpcode() == (*new_inst)->GetOpcode());
        inst->ReplaceUsers(*new_inst);
        if (inst->IsReferenceOrAny()) {
            ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), *new_inst, inst);
        }
    }
}

static bool HasSaveState(BasicBlock *block)
{
    for (auto inst : block->Insts()) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            return true;
        }
    }
    return false;
}

bool InteropIntrinsicOptimization::CanHoistTo(BasicBlock *block)
{
    ASSERT(!block->IsMarked(elimination_candidate_));
    auto &info = GetInfo(block);
    bool dominates_subgraph = info.dom_tree_in <= info.subgraph_min_num && info.subgraph_max_num < info.dom_tree_out;
    auto depth = block->GetLoop()->GetDepth();
    // Heuristics to estimate if hoisting to this blocks is profitable, and it is not better to hoist to some dominated
    // blocks instead
    if (dominates_subgraph && info.max_chain > 1) {
        for (auto *dom : block->GetDominatedBlocks()) {
            auto dom_depth = dom->GetLoop()->GetDepth();
            if (GetInfo(dom).max_chain > 0 &&
                (GetInfo(dom).max_chain < info.max_chain || dom_depth > depth || !HasSaveState(dom))) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << " Block " << block->GetId() << " is candidate for hoisting";
                return true;
            }
        }
    }
    if (info.max_depth > static_cast<int32_t>(depth)) {
        for (auto *dom : block->GetDominatedBlocks()) {
            auto dom_depth = dom->GetLoop()->GetDepth();
            if (GetInfo(dom).max_depth >= 0 && (dom_depth > depth || !HasSaveState(dom))) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                    << " Block " << block->GetId() << " is candidate for hoisting";
                return true;
            }
        }
    }
    return false;
}

// DFS blocks dominated by start block, and save blocks from its dominance frontier to process them later in
// HoistAndEliminate. If *new_inst is SaveState, we move the first dominated inst we want to replace after insert_after
// and set *new_inst to it. Otherwise just replace dominated inst with *new_inst
void InteropIntrinsicOptimization::HoistAndEliminateRec(BasicBlock *block, const BlockInfo &start_info, Inst **new_inst,
                                                        Inst *insert_after)
{
    if (block->ResetMarker(elimination_candidate_)) {
        for (auto *inst : block->InstsSafe()) {
            if (inst->IsMarked(elimination_candidate_) && inst != *new_inst) {
                ReplaceInst(inst, new_inst, insert_after);
                is_applied_ = true;
            }
        }
    }
    for (auto *succ : block->GetSuccsBlocks()) {
        if (!succ->ResetMarker(inst_anticipated_)) {
            continue;
        }
        // Fast IsDominate check
        if (start_info.dom_tree_in <= GetInfo(succ).dom_tree_in &&
            GetInfo(succ).dom_tree_in < start_info.dom_tree_out) {
            HoistAndEliminateRec(succ, start_info, new_inst, insert_after);
        } else {
            blocks_to_process_.push_back(succ);
        }
    }
}

// Returns SaveState and inst to insert after
static std::pair<Inst *, Inst *> GetHoistPosition(BasicBlock *block, Inst *boundary_inst)
{
    for (auto inst : block->InstsReverse()) {
        if (inst == boundary_inst) {
            auto prev = inst->GetPrev();
            if (prev != nullptr && prev->GetOpcode() == Opcode::SaveState && !inst->IsMovableObject()) {
                return {prev, inst};
            }
            return {nullptr, nullptr};
        }
        if (inst->GetOpcode() == Opcode::SaveState) {
            return {inst, inst};
        }
    }
    return {nullptr, nullptr};
}

Inst *InteropIntrinsicOptimization::FindEliminationCandidate(BasicBlock *block)
{
    for (auto inst : block->Insts()) {
        if (inst->IsMarked(elimination_candidate_)) {
            return inst;
        }
    }
    UNREACHABLE();
}

void InteropIntrinsicOptimization::HoistAndEliminate(BasicBlock *start_block, Inst *boundary_inst)
{
    ASSERT(boundary_inst->GetBasicBlock() == start_block);
    blocks_to_process_.clear();
    blocks_to_process_.push_back(start_block);
    ASSERT(start_block->ResetMarker(inst_anticipated_));
    while (!blocks_to_process_.empty()) {
        auto *block = blocks_to_process_.back();
        blocks_to_process_.pop_back();
        // We reset inst_anticipated_ marker while we traverse the subgraph where inst is anticipated
        ASSERT(!block->IsMarked(inst_anticipated_));
        auto &info = GetInfo(block);
        ASSERT(info.subgraph_min_num != DFS_INVALIDATED);
        Inst *new_inst = nullptr;
        Inst *insert_after = nullptr;
        if (block->IsMarked(elimination_candidate_)) {
            new_inst = FindEliminationCandidate(block);
        } else if (CanHoistTo(block)) {
            std::tie(new_inst, insert_after) = GetHoistPosition(block, boundary_inst);
            if (new_inst == nullptr) {
                info.subgraph_min_num = DFS_INVALIDATED;
                continue;
            }
        }
        info.subgraph_min_num = DFS_INVALIDATED;
        if (new_inst != nullptr) {
            HoistAndEliminateRec(block, GetInfo(block), &new_inst, insert_after);
            continue;
        }
        for (auto *succ : block->GetSuccsBlocks()) {
            if (succ->ResetMarker(inst_anticipated_)) {
                blocks_to_process_.push_back(succ);
            }
        }
    }
}

void InteropIntrinsicOptimization::DoRedundancyElimination(Inst *input, Inst *scope_start, InstVector &insts)
{
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "Process group of intrinsics with identical inputs and scope start: " << *scope_start
        << "\ninput: " << *input;
    auto *boundary_inst = input->IsDominate(scope_start) ? scope_start : input;
    auto *boundary = boundary_inst->GetBasicBlock();
    ASSERT(input->IsDominate(boundary_inst) && scope_start->IsDominate(boundary_inst));
    auto inst_anticipated_holder = MarkerHolder(GetGraph());
    inst_anticipated_ = inst_anticipated_holder.GetMarker();
    auto elimination_candidate_holder = MarkerHolder(GetGraph());
    elimination_candidate_ = elimination_candidate_holder.GetMarker();
    // Marking blocks where inst is partially anticipated is needed only to reduce number of processed blocks
    for (auto *inst : insts) {
        COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << " Inst in this group: " << *inst;
        inst->SetMarker(elimination_candidate_);
        auto *block = inst->GetBasicBlock();
        block->SetMarker(elimination_candidate_);
        MarkPartiallyAnticipated(block, boundary);
        GetInfo(block).max_chain++;
        GetInfo(block).max_depth = block->GetLoop()->GetDepth();
    }
    CalculateDownSafe(boundary);
    HoistAndEliminate(boundary, boundary_inst);
}

void InteropIntrinsicOptimization::SaveSiblingForElimination(Inst *sibling, ArenaMap<Inst *, InstVector> &current_insts,
                                                             RuntimeInterface::IntrinsicId id, Marker processed)
{
    if (!sibling->IsIntrinsic() || sibling->CastToIntrinsic()->GetIntrinsicId() != id) {
        return;
    }
    sibling->SetMarker(processed);
    auto scope_start = scope_for_inst_.at(sibling);
    if (scope_start != nullptr) {
        auto it = current_insts.try_emplace(scope_start, GetGraph()->GetLocalAllocator()->Adapter()).first;
        it->second.push_back(sibling);
    }
}

void InteropIntrinsicOptimization::RedundancyElimination()
{
    DomTreeDfs(GetGraph()->GetStartBlock());
    auto processed_holder = MarkerHolder(GetGraph());
    auto processed = processed_holder.GetMarker();
    ArenaMap<Inst *, InstVector> current_insts(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : block->InstsSafe()) {
            if (inst->IsMarked(processed) || !IsConvertIntrinsic(inst)) {
                continue;
            }
            auto id = inst->CastToIntrinsic()->GetIntrinsicId();
            current_insts.clear();
            auto *input = inst->GetInput(0).GetInst();
            for (auto &user : input->GetUsers()) {
                SaveSiblingForElimination(user.GetInst(), current_insts, id, processed);
            }
            for (auto &[scope, insts] : current_insts) {
                DoRedundancyElimination(input, scope, insts);
            }
        }
    }
}

bool InteropIntrinsicOptimization::RunImpl()
{
    bool one_scope = TryCreateSingleScope();
    if (!has_scopes_) {
        return false;
    }
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<DominatorsTree>();
    if (!one_scope) {
        COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase I: merge scopes inside basic blocks";
        for (auto *block : GetGraph()->GetBlocksRPO()) {
            MergeScopesInsideBlock(block);
        }
        if (!GetGraph()->IsOsrMode()) {
            for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
                FindForbiddenLoops(loop);
            }
            COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
                << "  Phase II: remove inter-scope regions to merge neighbouring scopes";
            MergeInterScopeRegions();
            if (!object_limit_hit_) {
                COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase III: hoist scope starts";
                HoistScopeStarts();
            }
        }
    }
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT)
        << "  Phase IV: Check graph and find scope starts for interop intrinsics";
    // NB: we assume that each scope is residing inside one block before the pass, and that there are no nested scopes
    CheckGraphAndFindScopes();
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase V: Peepholes for interop intrinsics";
    EliminateCastPairs();
    COMPILER_LOG(DEBUG, INTEROP_INTRINSIC_OPT) << "  Phase VI: Redundancy elimination for wrap intrinsics";
    RedundancyElimination();
    return IsApplied();
}

}  // namespace panda::compiler
