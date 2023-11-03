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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H_

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

#include "compiler_logger.h"
#include "checks_elimination.h"
#include "object_type_check_elimination.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/ir/graph_visitor.h"

namespace panda::compiler {
// parent_index->{Vector<bound_check>, max_val, min_val}
using GroupedBoundsChecks = ArenaUnorderedMap<Inst *, std::tuple<InstVector, int64_t, int64_t>>;
// loop->len_array->GroupedBoundsChecks
using NotFullyRedundantBoundsCheck = ArenaDoubleUnorderedMap<Loop *, Inst *, GroupedBoundsChecks>;

// {CountableLoopInfo; savestate; lower value; upper value; cond code for Deoptimize; head is loop exit; has pre-header
// compare}
using LoopInfo = std::tuple<CountableLoopInfo, Inst *, Inst *, Inst *, ConditionCode, bool, bool>;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ChecksElimination : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit ChecksElimination(Graph *graph)
        : Optimization(graph),
          bounds_checks_(graph->GetLocalAllocator()->Adapter()),
          checks_for_move_out_of_loop_(graph->GetLocalAllocator()->Adapter()),
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

    bool IsLoopDeleted() const
    {
        return is_loop_deleted_;
    }
    void SetLoopDeleted()
    {
        is_loop_deleted_ = true;
    }

    bool IsEnable() const override
    {
        return OPTIONS.IsCompilerChecksElimination();
    }

    void InvalidateAnalyses() override;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    template <bool CHECK_FULL_DOM = false>
    static void VisitNullCheck(GraphVisitor *v, Inst *inst);
    static void VisitNegativeCheck(GraphVisitor *v, Inst *inst);
    static void VisitNotPositiveCheck(GraphVisitor *v, Inst *inst);
    static void VisitZeroCheck(GraphVisitor *v, Inst *inst);
    static void VisitRefTypeCheck(GraphVisitor *v, Inst *inst);
    static void VisitBoundsCheck(GraphVisitor *v, Inst *inst);
    static void VisitCheckCast(GraphVisitor *v, Inst *inst);
    static void VisitIsInstance(GraphVisitor *v, Inst *inst);
    static void VisitAnyTypeCheck(GraphVisitor *v, Inst *inst);
    static void VisitHclassCheck(GraphVisitor *v, Inst *inst);
    static void VisitAddOverflowCheck(GraphVisitor *v, Inst *inst);
    static void VisitSubOverflowCheck(GraphVisitor *v, Inst *inst);
    static void VisitNegOverflowAndZeroCheck(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"
private:
    bool TryToEliminateAnyTypeCheck(Inst *inst, Inst *inst_to_replace, AnyBaseType type, AnyBaseType prev_type);
    void UpdateHclassChecks(Inst *inst);
    std::optional<Inst *> GetHclassCheckFromLoads(Inst *load_class);
    void ReplaceUsersAndRemoveCheck(Inst *inst_del, Inst *inst_rep);
    void PushNewCheckForMoveOutOfLoop(Inst *any_type_check)
    {
        checks_for_move_out_of_loop_.push_back(any_type_check);
    }

    void PushNewCheckMustThrow(Inst *inst)
    {
        checks_must_throw_.push_back(inst);
    }

    static bool IsInstIncOrDec(Inst *inst);
    static int64_t GetInc(Inst *inst);
    static Loop *GetLoopForBoundsCheck(BasicBlock *block, Inst *len_array, Inst *index);
    void InitItemForNewIndex(GroupedBoundsChecks *place, Inst *index, Inst *inst, bool check_upper, bool check_lower);
    void PushNewBoundsCheck(Loop *loop, Inst *len_array, Inst *index, Inst *inst, bool check_upper, bool check_lower);
    void TryRemoveDominatedNullChecks(Inst *inst, Inst *ref);
    void TryRemoveDominatedHclassCheck(Inst *inst);
    template <Opcode OPC, bool CHECK_FULL_DOM = false, typename CheckInputs = bool (*)(Inst *)>
    void TryRemoveDominatedChecks(
        Inst *inst, CheckInputs check_inputs = [](Inst * /*unused*/) { return true; });
    template <Opcode OPC>
    void TryRemoveConsecutiveChecks(Inst *inst);
    template <Opcode OPC>
    bool TryRemoveCheckByBounds(Inst *inst, Inst *input);
    template <Opcode OPC, bool CHECK_FULL_DOM = false>
    bool TryRemoveCheck(Inst *inst);
    template <Opcode OPC>
    void TryOptimizeOverflowCheck(Inst *inst);

    bool TryInsertDeoptimizationForLargeStep(ConditionCode cc, Inst *result_len_array, Inst *lower, Inst *upper,
                                             int64_t max_add, Inst *insert_deopt_after, Inst *ss, uint64_t const_step);
    bool TryInsertDeoptimization(LoopInfo loop_info, Inst *len_array, int64_t max_add, int64_t min_add,
                                 bool has_check_in_header);

    Inst *InsertNewLenArray(Inst *len_array, Inst *ss);
    bool NeedUpperDeoptimization(BasicBlock *header, Inst *len_array, BoundsRange len_array_range, Inst *upper,
                                 BoundsRange upper_range, int64_t max_add, ConditionCode cc,
                                 bool *insert_new_len_array);
    void InsertDeoptimizationForIndexOverflow(CountableLoopInfo *countable_loop_info, BoundsRange index_upper_range,
                                              Inst *ss);
    void ProcessingLoop(Loop *loop, ArenaUnorderedMap<Inst *, GroupedBoundsChecks> *lenarr_index_checks);
    void ProcessingGroupBoundsCheck(GroupedBoundsChecks *index_boundschecks, LoopInfo loop_info, Inst *len_array);
    void ReplaceBoundsCheckToDeoptimizationBeforeLoop();
    void HoistLoopInvariantBoundsChecks(Inst *len_array, GroupedBoundsChecks *index_boundschecks, Loop *loop);
    Inst *FindSaveState(const InstVector &insts_to_delete);
    void ReplaceBoundsCheckToDeoptimizationInLoop();
    void ReplaceCheckMustThrowByUnconditionalDeoptimize();
    void MoveCheckOutOfLoop();

    void InsertInstAfter(Inst *inst, Inst *after, BasicBlock *block);
    void InsertBoundsCheckDeoptimization(ConditionCode cc, Inst *left, int64_t val, Inst *right, Inst *ss,
                                         Inst *insert_after, Opcode new_left_opcode = Opcode::Add);
    Inst *InsertDeoptimization(ConditionCode cc, Inst *left, Inst *right, Inst *ss, Inst *insert_after,
                               DeoptimizeType deopt_type);

    std::optional<LoopInfo> FindLoopInfo(Loop *loop);
    Inst *FindSaveState(Loop *loop);
    Inst *FindOptimalSaveStateForHoist(Inst *inst, Inst **optimal_insert_after);
    static bool IsInlinedCallLoadMethod(Inst *inst);

private:
    NotFullyRedundantBoundsCheck bounds_checks_;
    InstVector checks_for_move_out_of_loop_;
    InstVector checks_must_throw_;

    bool is_applied_ {false};
    bool is_loop_deleted_ {false};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H_
