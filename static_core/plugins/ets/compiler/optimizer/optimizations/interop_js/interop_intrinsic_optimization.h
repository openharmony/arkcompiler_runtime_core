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

#ifndef PLUGINS_ETS_COMPILER_OPTIMIZER_INTEROP_JS_INTEROP_INTRINSIC_OPTIMIZATION_H_
#define PLUGINS_ETS_COMPILER_OPTIMIZER_INTEROP_JS_INTEROP_INTRINSIC_OPTIMIZATION_H_

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

#include "compiler_logger.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/graph_visitor.h"

namespace panda::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class InteropIntrinsicOptimization : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit InteropIntrinsicOptimization(Graph *graph, bool try_single_scope = false)
        : Optimization(graph),
          try_single_scope_(try_single_scope),
          scope_object_limit_(OPTIONS.GetCompilerInteropScopeObjectLimit()),
          forbidden_loops_(GetGraph()->GetLocalAllocator()->Adapter()),
          block_info_(GetGraph()->GetVectorBlocks().size(), GetGraph()->GetLocalAllocator()->Adapter()),
          components_(GetGraph()->GetLocalAllocator()->Adapter()),
          current_component_blocks_(GetGraph()->GetLocalAllocator()->Adapter()),
          scope_for_inst_(GetGraph()->GetLocalAllocator()->Adapter()),
          blocks_to_process_(GetGraph()->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(InteropIntrinsicOptimization);
    NO_COPY_SEMANTIC(InteropIntrinsicOptimization);
    ~InteropIntrinsicOptimization() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return OPTIONS.IsCompilerEnableFastInterop() && OPTIONS.IsCompilerInteropIntrinsicOptimization();
    }

    const char *GetPassName() const override
    {
        return "InteropIntrinsicOptimization";
    }

    bool IsApplied() const
    {
        return is_applied_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

#include "optimizer/ir/visitor.inc"

private:
    struct BlockInfo {
        Inst *last_scope_start {};
        int32_t dfs_num_in {DFS_NOT_VISITED};
        int32_t dfs_num_out {DFS_NOT_VISITED};
        int32_t dom_tree_in {DFS_NOT_VISITED};
        int32_t dom_tree_out {DFS_NOT_VISITED};
        int32_t subgraph_min_num {DFS_NOT_VISITED};
        int32_t subgraph_max_num {DFS_NOT_VISITED};
        std::array<int32_t, 2U> block_component {DFS_NOT_VISITED, DFS_NOT_VISITED};
        int32_t max_chain {};
        int32_t max_depth {};
    };

    struct Component {
        int32_t parent {-1};
        uint32_t object_count {};
        uint32_t last_used {};
        bool is_forbidden {};
    };

    // Mechanism is similar to markers
    uint32_t GetObjectCountIfUnused(Component &comp, uint32_t used_number)
    {
        if (comp.last_used == used_number + 1) {
            return 0;
        }
        comp.last_used = used_number + 1;
        return comp.object_count;
    }

    BlockInfo &GetInfo(BasicBlock *block);
    void MergeScopesInsideBlock(BasicBlock *block);
    bool TryCreateSingleScope(BasicBlock *bb);
    bool TryCreateSingleScope();
    std::optional<uint64_t> FindForbiddenLoops(Loop *loop);
    bool IsForbiddenLoopEntry(BasicBlock *block);
    int32_t GetParentComponent(int32_t component);
    void MergeComponents(int32_t first, int32_t second);
    void UpdateStatsForMerging(Inst *inst, int32_t other_end_component, uint32_t *scope_switches,
                               uint32_t *objects_in_block_after_merge);
    template <bool IS_END>
    void IterateBlockFromBoundary(BasicBlock *block);
    template <bool IS_END>
    void BlockBoundaryDfs(BasicBlock *block);
    void MergeCurrentComponentWithNeighbours();
    template <bool IS_END>
    void FindComponentAndTryMerge(BasicBlock *block);
    void MergeInterScopeRegions();

    void DfsNumbering(BasicBlock *block);
    void CalculateReachabilityRec(BasicBlock *block);
    template <void (InteropIntrinsicOptimization::*DFS)(BasicBlock *)>
    void DoDfs();
    bool CreateScopeStartInBlock(BasicBlock *block);
    void RemoveReachableScopeStarts(BasicBlock *block, BasicBlock *new_start_block);
    void HoistScopeStarts();

    void InvalidateScopesInSubgraph(BasicBlock *block);
    void CheckGraphRec(BasicBlock *block, Inst *scope_start);
    void CheckGraphAndFindScopes();

    void MarkRequireRegMap(Inst *inst);
    void TryRemoveUnwrapAndWrap(Inst *inst, Inst *input);
    void TryRemoveUnwrapToJSValue(Inst *inst);
    void TryRemoveIntrinsic(Inst *inst);
    void EliminateCastPairs();

    void DomTreeDfs(BasicBlock *block);
    void MarkPartiallyAnticipated(BasicBlock *block, BasicBlock *stop_block);
    void CalculateDownSafe(BasicBlock *block);
    void ReplaceInst(Inst *inst, Inst **new_inst, Inst *insert_after);
    bool CanHoistTo(BasicBlock *block);
    void HoistAndEliminateRec(BasicBlock *block, const BlockInfo &start_info, Inst **new_inst, Inst *insert_after);
    Inst *FindEliminationCandidate(BasicBlock *block);
    void HoistAndEliminate(BasicBlock *start_block, Inst *boundary_inst);
    void DoRedundancyElimination(Inst *input, Inst *scope_start, InstVector &insts);
    void SaveSiblingForElimination(Inst *sibling, ArenaMap<Inst *, InstVector> &current_insts,
                                   RuntimeInterface::IntrinsicId id, Marker processed);
    void RedundancyElimination();

private:
    static constexpr uint64_t MAX_LOOP_ITERATIONS = 10;
    static constexpr int32_t DFS_NOT_VISITED = -1;
    static constexpr int32_t DFS_INVALIDATED = -2;

    bool try_single_scope_;
    uint32_t scope_object_limit_;
    Marker start_dfs_ {};
    Marker can_hoist_to_ {};
    Marker visited_ {};
    Marker inst_anticipated_ {};
    Marker scope_start_invalidated_ {};
    Marker elimination_candidate_ {};
    Marker require_reg_map_ {};
    bool has_scopes_ {false};
    bool is_applied_ {false};
    int32_t dfs_num_ {};
    int32_t dom_tree_num_ {};
    int32_t current_component_ {};
    uint32_t objects_in_scope_after_merge_ {};
    bool can_merge_ {};
    bool object_limit_hit_ {false};
    ArenaUnorderedSet<Loop *> forbidden_loops_;
    ArenaVector<BlockInfo> block_info_;
    ArenaVector<Component> components_;
    ArenaVector<BasicBlock *> current_component_blocks_;
    ArenaUnorderedMap<Inst *, Inst *> scope_for_inst_;
    ArenaVector<BasicBlock *> blocks_to_process_;
    SaveStateBridgesBuilder ssb_;
};
}  // namespace panda::compiler

#endif  // PLUGINS_ETS_COMPILER_OPTIMIZER_INTEROP_JS_INTEROP_INTRINSIC_OPTIMIZATION_H_
