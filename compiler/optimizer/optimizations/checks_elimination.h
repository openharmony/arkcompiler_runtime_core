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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H_

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

#include "object_type_check_elimination.h"
#include "compiler_logger.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/ir/graph_visitor.h"
#include "checks_elimination.h"

namespace panda::compiler {
// parent_index->{Vector<bound_check>, max_val, min_val}
using GroupedBoundsChecks = ArenaUnorderedMap<Inst *, std::tuple<InstVector, int64_t, int64_t>>;
// loop->len_array->GroupedBoundsChecks
using NotFullyRedundantBoundsCheck = ArenaDoubleUnorderedMap<Loop *, Inst *, GroupedBoundsChecks>;

// {savestate; lower upper; upper value; loop index; cond code}
using LoopInfo = std::tuple<Inst *, Inst *, Inst *, Inst *, ConditionCode>;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ChecksElimination : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit ChecksElimination(Graph *graph)
        : Optimization(graph),
          bounds_checks_(graph->GetLocalAllocator()->Adapter()),
          null_checks_(graph->GetLocalAllocator()->Adapter()),
          any_type_checks_(graph->GetLocalAllocator()->Adapter()),
          checks_must_throw_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(ChecksElimination);
    NO_COPY_SEMANTIC(ChecksElimination);
    ~ChecksElimination() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "ChecksElimination";
    }

    bool IsApplied() const
    {
        return is_applied_;
    }
    void SetApplied()
    {
        is_applied_ = true;
    }

    bool IsEnable() const override
    {
        return options.IsCompilerChecksElimination();
    }

    void InvalidateAnalyses() override;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    static void VisitNullCheck(GraphVisitor *v, Inst *inst);
    static void VisitDeoptimizeIf(GraphVisitor *v, Inst *inst);
    static void VisitNegativeCheck(GraphVisitor *v, Inst *inst);
    static void VisitZeroCheck(GraphVisitor *v, Inst *inst);
    static void VisitRefTypeCheck(GraphVisitor *v, Inst *inst);
    static void VisitBoundsCheck(GraphVisitor *v, Inst *inst);
    static void VisitCheckCast(GraphVisitor *v, Inst *inst);
    static void VisitAnyTypeCheck(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"
private:
    void ReplaceUsersAndRemoveCheck(Inst *inst_del, Inst *inst_rep);
    void PushNewNullCheck(Inst *null_check)
    {
        null_checks_.push_back(null_check);
    }
    void PushNewAnyTypeCheck(Inst *any_type_check)
    {
        any_type_checks_.push_back(any_type_check);
    }

    void PushNewCheckMustThrow(Inst *inst)
    {
        checks_must_throw_.push_back(inst);
    }

    bool IsInstIncOrDec(Inst *inst);
    void InitItemForNewIndex(GroupedBoundsChecks *place, Inst *index, Inst *inst);
    void PushNewBoundsCheck(Loop *loop, Inst *len_array, Inst *index, Inst *inst);
    void TryRemoveDominatedNullChecks(Inst *inst, Inst *ref);
    template <Opcode opc>
    void TryRemoveDominatedChecks(Inst *inst);
    template <Opcode opc>
    void TryRemoveConsecutiveChecks(Inst *inst);
    template <Opcode opc>
    bool TryRemoveCheckByBounds(Inst *inst, Inst *input);
    template <Opcode opc>
    bool TryRemoveCheck(Inst *inst);

private:
    bool TryInsertDeoptimization(LoopInfo loop_info, Inst *len_array, int64_t max_add, int64_t max_sub);

    Inst *InsertNewLenArray(Inst *len_array, Inst *ss);
    void ProcessingLoop(Loop *loop, ArenaUnorderedMap<Inst *, GroupedBoundsChecks> *lenarr_index_checks);
    void ProcessingGroupBoundsCheck(GroupedBoundsChecks *index_boundschecks, LoopInfo loop_info, Inst *len_array);
    void ReplaceBoundsCheckToDeoptimizationBeforeLoop();
    Inst *FindSaveState(const InstVector &insts_to_delete);
    void ReplaceBoundsCheckToDeoptimizationInLoop();
    void ReplaceNullCheckToDeoptimizationBeforeLoop();
    void ReplaceCheckMustThrowByUnconditionalDeoptimize();
    void MoveAnyTypeCheckOutOfLoop();

    void InsertDeoptimization(ConditionCode cc, Inst *left, int64_t val, Inst *right, Inst *ss, Inst *insert_after);
    Inst *InsertDeoptimization(ConditionCode cc, Inst *left, Inst *right, Inst *ss, Inst *insert_after);

    std::optional<LoopInfo> FindLoopInfo(Loop *loop);
    Inst *FindSaveState(Loop *loop);

private:
    NotFullyRedundantBoundsCheck bounds_checks_;
    InstVector null_checks_;
    InstVector any_type_checks_;
    InstVector checks_must_throw_;

    bool is_applied_ {false};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H_
