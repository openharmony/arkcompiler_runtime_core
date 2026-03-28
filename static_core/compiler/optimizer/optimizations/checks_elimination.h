/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

#include "compiler_logger.h"
#include "object_type_check_elimination.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
using FlagPair = std::pair<bool, bool>;
using InstPair = std::pair<Inst *, Inst *>;
using InstTriple = std::tuple<Inst *, Inst *, Inst *>;

// {CountableLoopInfo; savestate; lower value; upper value; cond code for Deoptimize; head is loop exit; has pre-header
// compare}
using LoopInfo = std::tuple<CountableLoopInfo, Inst *, Inst *, Inst *, ConditionCode, bool, bool>;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ChecksElimination : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit ChecksElimination(Graph *graph)
        : Optimization(graph),
          boundsChecks_(graph->GetLocalAllocator()->Adapter()),
          checksForMoveOutOfLoop_(graph->GetLocalAllocator()->Adapter()),
          checksMustThrow_(graph->GetLocalAllocator()->Adapter())
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
        return isApplied_;
    }
    void SetApplied()
    {
        isApplied_ = true;
    }

    bool IsLoopDeleted() const
    {
        return isLoopDeleted_;
    }
    void SetLoopDeleted()
    {
        isLoopDeleted_ = true;
    }

    bool IsEnable() const override
    {
        return g_options.IsCompilerChecksElimination();
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
    // CC-OFF(G.NAM.03-CPP) project code style
    static constexpr size_t MIN_BOUNDSCHECKS_FOR_DEOPT_MERGE = 2;

    // Guard info for one inserted deopt: {has_max_add, has_min_add, max_add, min_add, insert_after}.
    using DeoptGuardInfo = std::tuple<bool, bool, int64_t, int64_t, Inst *>;

    // {check_inst, check_upper, check_lower, cached_offset}
    using BoundsCheckInfo = std::tuple<Inst *, bool, bool, int64_t>;
    using NotDominateInstInfoVector = ArenaVector<BoundsCheckInfo>;
    // {check_block, NotDominateInstInfoVector}, kept in first-seen RPO block order.
    using NotDominateBoundsChecksByBlock = ArenaVector<std::pair<BasicBlock *, NotDominateInstInfoVector>>;

    // {parent_index, Vector<bound_check>, NotDominateBoundsChecksByBlock, max_val, min_val}
    using ParentIndexUsedBoundsChecks =
        std::tuple<Inst *, InstVector, NotDominateBoundsChecksByBlock, int64_t, int64_t>;
    using GroupedBoundsChecks = ArenaVector<ParentIndexUsedBoundsChecks>;
    // loop->len_array->GroupedBoundsChecks
    using LoopNotFullyRedundantBoundsCheck = ArenaVector<std::pair<Inst *, GroupedBoundsChecks>>;
    using NotFullyRedundantBoundsCheck = ArenaVector<std::pair<Loop *, LoopNotFullyRedundantBoundsCheck>>;
    bool TryToEliminateAnyTypeCheck(Inst *inst, Inst *instToReplace, AnyBaseType type, AnyBaseType prevType);
    void UpdateHclassChecks(Inst *inst);
    std::optional<Inst *> GetHclassCheckFromLoads(Inst *loadClass);
    void ReplaceUsersAndRemoveCheck(Inst *instDel, Inst *instRep);
    void PushNewCheckForMoveOutOfLoop(Inst *anyTypeCheck)
    {
        checksForMoveOutOfLoop_.push_back(anyTypeCheck);
    }

    void PushNewCheckMustThrow(Inst *inst)
    {
        checksMustThrow_.push_back(inst);
    }

    static bool IsInstIncOrDecConst(Inst *inst);
    static int64_t GetInc(Inst *inst);
    static Loop *GetLoopForBoundsCheck(BasicBlock *block, Inst *lenArray, Inst *index);
    // CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters)
    // NOLINTNEXTLINE(readability-function-size)
    void InitItemForNewIndex(GroupedBoundsChecks *place, Inst *index, Inst *inst, bool checkUpper, bool checkLower,
                             bool dominateLoopBackEdgeOrGraphEnd);
    void AppendNotDominateBoundsCheck(NotDominateBoundsChecksByBlock *notDominateInfos, Inst *inst, bool checkUpper,
                                      bool checkLower, int64_t offset);
    void PushNewBoundsCheck(Loop *loop, Inst *inst, InstPair helpers, bool checkUpper, bool checkLower);
    // CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters)
    // NOLINTNEXTLINE(readability-function-size)
    void PushNewBoundsCheckAtExistingIndexes(GroupedBoundsChecks *indexes, Inst *index, Inst *inst, bool checkUpper,
                                             bool checkLower, bool dominateLoopBackEdgeOrGraphEnd);
    void TryRemoveDominatedNullChecks(Inst *inst, Inst *ref);
    void TryRemoveDominatedHclassCheck(Inst *inst);
    template <Opcode OPC, bool CHECK_FULL_DOM, typename CheckInputs>
    void TryRemoveDominatedCheck(Inst *inst, Inst *userInst, CheckInputs checkInputs);
    template <Opcode OPC, bool CHECK_FULL_DOM = false, typename CheckInputs = bool (*)(Inst *)>
    void TryRemoveDominatedChecks(
        Inst *inst, CheckInputs checkInputs = [](Inst * /*unused*/) { return true; });
    template <Opcode OPC>
    void TryRemoveConsecutiveChecks(Inst *inst);
    template <Opcode OPC>
    bool TryRemoveCheckByBounds(Inst *inst, Inst *input);
    template <Opcode OPC, bool CHECK_FULL_DOM = false>
    bool TryRemoveCheck(Inst *inst);
    template <Opcode OPC>
    void TryOptimizeOverflowCheck(Inst *inst);

    bool TryInsertDeoptimizationForLargeStep(ConditionCode cc, InstPair bounds, InstTriple helpers, int64_t maxAdd,
                                             uint64_t constStep);
    bool TryInsertDeoptimization(LoopInfo loopInfo, Inst *lenArray, int64_t maxAdd, int64_t minAdd,
                                 bool hasCheckInHeader);
    bool TryInsertUpperDeoptimization(LoopInfo loopInfo, Inst *lenArray, BoundsRange lowerRange, int64_t maxAdd,
                                      bool insertNewLenArray);

    Inst *InsertNewLenArray(Inst *lenArray, Inst *ss);
    bool NeedUpperDeoptimization(BasicBlock *header, InstPair insts, FlagPair flags, int64_t maxAdd,
                                 bool *insertNewLenArray);
    void InsertDeoptimizationForIndexOverflow(CountableLoopInfo *countableLoopInfo, BoundsRange indexUpperRange,
                                              Inst *ss);
    void ProcessingLoop(Loop *loop, LoopNotFullyRedundantBoundsCheck *lenarrIndexChecks);
    void ProcessingGroupBoundsCheck(GroupedBoundsChecks *indexBoundschecks, LoopInfo loopInfo, Inst *lenArray);
    void ReplaceBoundsCheckToDeoptimizationBeforeLoop();
    void HoistLoopInvariantBoundsChecks(Inst *lenArray, GroupedBoundsChecks *indexBoundschecks, Loop *loop);
    Inst *FindSaveState(const InstVector &instsToDelete);
    void ReplaceBoundsCheckToDeoptimizationInLoop();
    // CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) keep explicit inputs for bounds-check coverage.
    // NOLINTNEXTLINE(readability-function-size)
    bool NotDominateInstFitsInDeopt(bool hasMaxAdd, bool hasMinAdd, int64_t maxAdd, int64_t minAdd, int64_t val,
                                    bool checkUpper, bool checkLower);
    // CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) keep explicit inputs for non-dominated BCE
    // NOLINTNEXTLINE(readability-function-size)
    void TryEliminateDeoptCoveredBoundsChecks(NotDominateBoundsChecksByBlock &notDominateBoundsChecks, bool hasMaxAdd,
                                              bool hasMinAdd, int64_t maxAdd, int64_t minAdd, Inst *domGate = nullptr);
    Inst *SelectInsertAfterForBoundsDeopt(Inst *lenArray, Inst *saveState, Inst *parentIndex) const;
    Inst *FindSaveStateBlockLocal(const NotDominateInstInfoVector &blockInfos, BasicBlock *block) const;
    // Merge non-dominated checks inside one block and insert local deopt in the same block only.
    std::optional<DeoptGuardInfo> TryReplaceBoundsCheckByDeoptInOneBlock(Inst *lenArray, Inst *parentIndex,
                                                                         BasicBlock *block,
                                                                         NotDominateInstInfoVector &blockInfos);
    // For each block (RPO order), do local merge first, then eliminate covered checks in dominated blocks.
    void TryReplaceBoundsCheckByDeoptInBlock(Inst *lenArray, Inst *parentIndex,
                                             NotDominateBoundsChecksByBlock &notDominateInstInfos);
    void ReplaceBoundsChecksByDeoptInOneLoop(std::pair<Loop *, LoopNotFullyRedundantBoundsCheck> &item);

    void ReplaceCheckMustThrowByUnconditionalDeoptimize();
    void MoveCheckOutOfLoop();

    void InsertInstAfter(Inst *inst, Inst *after, BasicBlock *block);
    void InsertBoundsCheckDeoptimization(ConditionCode cc, InstPair args, int64_t val, InstPair helpers,
                                         Opcode newLeftOpcode = Opcode::Add);

    std::optional<LoopInfo> FindLoopInfo(Loop *loop);
    Inst *FindSaveState(Loop *loop);
    Inst *FindOptimalSaveStateForHoist(Inst *inst, Inst **optimalInsertAfter);
    static bool IsInlinedCallLoadMethod(Inst *inst);

private:
    NotFullyRedundantBoundsCheck boundsChecks_;
    InstVector checksForMoveOutOfLoop_;
    InstVector checksMustThrow_;

    bool isApplied_ {false};
    bool isLoopDeleted_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_CHECKSELIMINATION_H
