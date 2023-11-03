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

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "compiler_logger.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/analysis.h"

namespace panda::compiler {
/**
 * Branch Elimination optimization finds `if-true` blocks with resolvable conditional instruction.
 * Condition can be resolved in the following ways:
 * - Condition is constant;
 * - Condition is dominated by the equal one condition with the same inputs and the only one successor of the dominant
 * reaches dominated condition
 */
BranchElimination::BranchElimination(Graph *graph)
    : Optimization(graph),
      same_input_compares_(graph->GetLocalAllocator()->Adapter()),
      same_input_compare_any_type_(graph->GetLocalAllocator()->Adapter())
{
}

bool BranchElimination::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    is_applied_ = false;
    same_input_compares_.clear();
    auto marker_holder = MarkerHolder(GetGraph());
    rm_block_marker_ = marker_holder.GetMarker();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsEmpty() || (block->IsTry() && GetGraph()->IsBytecodeOptimizer())) {
            continue;
        }
        if (block->GetLastInst()->GetOpcode() == Opcode::IfImm) {
            if (SkipForOsr(block)) {
                COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Skip for OSR, id = " << block->GetId();
                continue;
            }
            /* skip branch() elimination at the end of the
             * preheader until LoopUnrolling pass is done */
            if (!GetGraph()->IsBytecodeOptimizer() && OPTIONS.IsCompilerDeferPreheaderTransform() &&
                !GetGraph()->IsUnrollComplete() && block->IsLoopValid() && block->IsLoopPreHeader()) {
                continue;
            }
            VisitBlock(block);
        }
    }
    DisconnectBlocks();

    if (is_applied_ && GetGraph()->IsOsrMode()) {
        CleanupGraphSaveStateOSR(GetGraph());
    }
    COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Branch elimination complete";
    return is_applied_;
}

bool BranchElimination::SkipForOsr(const BasicBlock *block)
{
    if (!block->IsLoopValid()) {
        return false;
    }
    auto loop = block->GetLoop();
    if (loop->IsRoot() || !loop->GetHeader()->IsOsrEntry()) {
        return false;
    }
    if (loop->GetBackEdges().size() > 1) {
        return false;
    }
    return block->IsDominate(loop->GetBackEdges()[0]);
}

void BranchElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    // Before in "CleanupGraphSaveStateOSR" already run "LoopAnalyzer"
    // in case (is_applied_ && GetGraph()->IsOsrMode())
    if (!(is_applied_ && GetGraph()->IsOsrMode())) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

void BranchElimination::BranchEliminationConst(BasicBlock *if_block)
{
    auto if_imm = if_block->GetLastInst()->CastToIfImm();
    auto condition_inst = if_block->GetLastInst()->GetInput(0).GetInst();
    COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Block with constant if instruction input is visited, id = "
                                     << if_block->GetId();

    uint64_t const_value = condition_inst->CastToConstant()->GetIntValue();
    bool cond_result = (const_value == if_imm->GetImm());
    if (if_imm->GetCc() == CC_NE) {
        cond_result = !cond_result;
    } else {
        ASSERT(if_imm->GetCc() == CC_EQ);
    }
    auto eliminated_successor = if_block->GetFalseSuccessor();
    if (!cond_result) {
        eliminated_successor = if_block->GetTrueSuccessor();
    }
    EliminateBranch(if_block, eliminated_successor);
    GetGraph()->GetEventWriter().EventBranchElimination(
        if_block->GetId(), if_block->GetGuestPc(), condition_inst->GetId(), condition_inst->GetPc(), "const-condition",
        eliminated_successor == if_block->GetTrueSuccessor());
}

void BranchElimination::BranchEliminationCompare(BasicBlock *if_block)
{
    auto if_imm = if_block->GetLastInst()->CastToIfImm();
    auto condition_inst = if_block->GetLastInst()->GetInput(0).GetInst();
    if (auto result = GetConditionResult(condition_inst)) {
        COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Compare instruction result was resolved. Instruction id = "
                                         << condition_inst->GetId()
                                         << ", resolved result: " << (result.value() ? "true" : "false");
        auto eliminated_successor = if_imm->GetEdgeIfInputFalse();
        if (!result.value()) {
            eliminated_successor = if_imm->GetEdgeIfInputTrue();
        }
        EliminateBranch(if_block, eliminated_successor);
        GetGraph()->GetEventWriter().EventBranchElimination(
            if_block->GetId(), if_block->GetGuestPc(), condition_inst->GetId(), condition_inst->GetPc(), "dominant-if",
            eliminated_successor == if_block->GetTrueSuccessor());
    } else {
        ConditionOps ops {condition_inst->GetInput(0).GetInst(), condition_inst->GetInput(1).GetInst()};
        auto it = same_input_compares_.try_emplace(ops, GetGraph()->GetLocalAllocator()->Adapter());
        it.first->second.push_back(condition_inst);
    }
}

void BranchElimination::BranchEliminationCompareAnyType(BasicBlock *if_block)
{
    auto if_imm = if_block->GetLastInst()->CastToIfImm();
    auto compare_any = if_block->GetLastInst()->GetInput(0).GetInst()->CastToCompareAnyType();
    if (auto result = GetCompareAnyTypeResult(if_imm)) {
        COMPILER_LOG(DEBUG, BRANCH_ELIM) << "CompareAnyType instruction result was resolved. Instruction id = "
                                         << compare_any->GetId()
                                         << ", resolved result: " << (result.value() ? "true" : "false");
        auto eliminated_successor = if_imm->GetEdgeIfInputFalse();
        if (!result.value()) {
            eliminated_successor = if_imm->GetEdgeIfInputTrue();
        }
        EliminateBranch(if_block, eliminated_successor);
        GetGraph()->GetEventWriter().EventBranchElimination(if_block->GetId(), if_block->GetGuestPc(),
                                                            compare_any->GetId(), compare_any->GetPc(), "dominant-if",
                                                            eliminated_successor == if_block->GetTrueSuccessor());
        return;
    }

    Inst *input = compare_any->GetInput(0).GetInst();
    auto it = same_input_compare_any_type_.try_emplace(input, GetGraph()->GetLocalAllocator()->Adapter());
    it.first->second.push_back(compare_any);
}

/**
 * Select unreachable successor and run elimination process
 * @param blocks - list of blocks with constant `if` instruction input
 */
void BranchElimination::VisitBlock(BasicBlock *if_block)
{
    ASSERT(if_block != nullptr);
    ASSERT(if_block->GetGraph() == GetGraph());
    ASSERT(if_block->GetLastInst()->GetOpcode() == Opcode::IfImm);
    ASSERT(if_block->GetSuccsBlocks().size() == MAX_SUCCS_NUM);

    auto condition_inst = if_block->GetLastInst()->GetInput(0).GetInst();
    switch (condition_inst->GetOpcode()) {
        case Opcode::Constant:
            BranchEliminationConst(if_block);
            break;
        case Opcode::Compare:
            BranchEliminationCompare(if_block);
            break;
        case Opcode::CompareAnyType:
            BranchEliminationCompareAnyType(if_block);
            break;
        default:
            break;
    }
}

/**
 * If `eliminated_block` has no other predecessor than `if_block`, mark `eliminated_block` to be disconnected
 * If `eliminated_block` dominates all predecessors, exclude `if_block`, mark `eliminated_block` to be disconneted
 * Else remove edge between these blocks
 * @param if_block - block with constant 'if' instruction input
 * @param eliminated_block - unreachable form `if_block`
 */
void BranchElimination::EliminateBranch(BasicBlock *if_block, BasicBlock *eliminated_block)
{
    ASSERT(if_block != nullptr && if_block->GetGraph() == GetGraph());
    ASSERT(eliminated_block != nullptr && eliminated_block->GetGraph() == GetGraph());
    // find predecessor which is not dominated by `eliminated_block`
    auto preds = eliminated_block->GetPredsBlocks();
    auto it = std::find_if(preds.begin(), preds.end(), [if_block, eliminated_block](BasicBlock *pred) {
        return pred != if_block && !eliminated_block->IsDominate(pred);
    });
    bool dominates_all_preds = (it == preds.cend());
    if (preds.size() > 1 && !dominates_all_preds) {
        RemovePredecessorUpdateDF(eliminated_block, if_block);
        if_block->RemoveSucc(eliminated_block);
        if_block->RemoveInst(if_block->GetLastInst());
        GetGraph()->GetAnalysis<Rpo>().SetValid(true);
        // TODO (a.popov) DominatorsTree could be restored inplace
        GetGraph()->RunPass<DominatorsTree>();
    } else {
        eliminated_block->SetMarker(rm_block_marker_);
    }
    is_applied_ = true;
}

/**
 * Before disconnecting the `block` find and disconnect all its successors dominated by it.
 * Use DFS to disconnect blocks in the PRO
 * @param block - unreachable block to disconnect from the graph
 */
void BranchElimination::MarkUnreachableBlocks(BasicBlock *block)
{
    for (auto dom : block->GetDominatedBlocks()) {
        dom->SetMarker(rm_block_marker_);
        MarkUnreachableBlocks(dom);
    }
}

bool AllPredecessorsMarked(BasicBlock *block, Marker marker)
{
    if (block->GetPredsBlocks().empty()) {
        ASSERT(block->IsStartBlock());
        return false;
    }

    for (auto pred : block->GetPredsBlocks()) {
        if (!pred->IsMarked(marker)) {
            return false;
        }
    }
    block->SetMarker(marker);
    return true;
}

/// Disconnect selected blocks
void BranchElimination::DisconnectBlocks()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsMarked(rm_block_marker_) || AllPredecessorsMarked(block, rm_block_marker_)) {
            MarkUnreachableBlocks(block);
        }
    }

    const auto &rpo_blocks = GetGraph()->GetBlocksRPO();
    for (auto it = rpo_blocks.rbegin(); it != rpo_blocks.rend(); it++) {
        auto block = *it;
        if (block != nullptr && block->IsMarked(rm_block_marker_)) {
            GetGraph()->DisconnectBlock(block);
            COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Block was disconnected, id = " << block->GetId();
        }
    }
}

/**
 * Check if the `target_block` is reachable from one successor only of the `dominant_block`
 *
 * If `target_block` is dominated by one of the successors, need to check that `target_block`
 * is NOT reachable by the other successor
 */
bool BlockIsReachedFromOnlySuccessor(BasicBlock *target_block, BasicBlock *dominant_block)
{
    ASSERT(dominant_block->IsDominate(target_block));
    BasicBlock *other_succesor = nullptr;
    if (dominant_block->GetTrueSuccessor()->IsDominate(target_block)) {
        other_succesor = dominant_block->GetFalseSuccessor();
    } else if (dominant_block->GetFalseSuccessor()->IsDominate(target_block)) {
        other_succesor = dominant_block->GetTrueSuccessor();
    } else {
        return false;
    }

    auto marker_holder = MarkerHolder(target_block->GetGraph());
    if (BlocksPathDfsSearch(marker_holder.GetMarker(), other_succesor, target_block)) {
        return false;
    }
    ASSERT(!other_succesor->IsDominate(target_block));
    return true;
}

/**
 * TODO (a.popov) Here can be supported more complex case:
 * when `dom_compare` has 2 or more `if_imm` users and `target_compare` is reachable from the same successors of these
 * if_imms
 */
Inst *FindIfImmDominatesCondition(Inst *dom_compare, Inst *target_compare)
{
    for (auto &user : dom_compare->GetUsers()) {
        auto inst = user.GetInst();
        if (inst->GetOpcode() == Opcode::IfImm && inst->IsDominate(target_compare)) {
            return inst;
        }
    }
    return nullptr;
}

/// Resolve condition result if there is a dominant equal condition
std::optional<bool> BranchElimination::GetConditionResult(Inst *condition)
{
    ConditionOps ops {condition->GetInput(0).GetInst(), condition->GetInput(1).GetInst()};
    if (same_input_compares_.count(ops) > 0) {
        auto instructions = same_input_compares_.at(ops);
        ASSERT(!instructions.empty());
        for (auto dom_cond : instructions) {
            // Find dom_cond's if_imm, that dominates target condition
            auto if_imm = FindIfImmDominatesCondition(dom_cond, condition);
            if (if_imm == nullptr) {
                continue;
            }
            if (BlockIsReachedFromOnlySuccessor(condition->GetBasicBlock(), if_imm->GetBasicBlock())) {
                if (auto result = TryResolveResult(condition, dom_cond, if_imm->CastToIfImm())) {
                    COMPILER_LOG(DEBUG, BRANCH_ELIM)
                        << "Equal compare instructions were found. Dominant id = " << dom_cond->GetId()
                        << ", dominated id = " << condition->GetId();
                    return result;
                }
            }
        }
    }
    return std::nullopt;
}

/// Resolve condition result if there is a dominant IfImmInst after CompareAnyTypeInst.
std::optional<bool> BranchElimination::GetCompareAnyTypeResult(IfImmInst *if_imm)
{
    auto compare_any = if_imm->GetInput(0).GetInst()->CastToCompareAnyType();
    Inst *input = compare_any->GetInput(0).GetInst();
    const auto it = same_input_compare_any_type_.find(input);
    if (it == same_input_compare_any_type_.end()) {
        return std::nullopt;
    }

    const ArenaVector<CompareAnyTypeInst *> &instructions = it->second;
    ASSERT(!instructions.empty());
    for (const auto dom_compare_any : instructions) {
        // Find dom_cond's if_imm, that dominates target condition.
        auto if_imm_dom_block = FindIfImmDominatesCondition(dom_compare_any, if_imm);
        if (if_imm_dom_block == nullptr) {
            continue;
        }

        if (!BlockIsReachedFromOnlySuccessor(if_imm->GetBasicBlock(), if_imm_dom_block->GetBasicBlock())) {
            continue;
        }

        auto result = TryResolveCompareAnyTypeResult(compare_any, dom_compare_any, if_imm_dom_block->CastToIfImm());
        if (!result) {
            continue;
        }

        COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Equal CompareAnyType instructions were found. Dominant id = "
                                         << dom_compare_any->GetId() << ", dominated id = " << compare_any->GetId();
        return result;
    }
    return std::nullopt;
}

/**
 * Try to resolve CompareAnyTypeInst result with the information
 * about dominant CompareAnyTypeInst with the same inputs.
 */
std::optional<bool> BranchElimination::TryResolveCompareAnyTypeResult(CompareAnyTypeInst *compare_any,
                                                                      CompareAnyTypeInst *dom_compare_any,
                                                                      IfImmInst *if_imm_dom_block)
{
    auto compare_any_bb = compare_any->GetBasicBlock();
    bool is_true_dom_branch = if_imm_dom_block->GetEdgeIfInputTrue()->IsDominate(compare_any_bb);

    auto graph = compare_any_bb->GetGraph();
    auto language = graph->GetRuntime()->GetMethodSourceLanguage(graph->GetMethod());
    auto res = IsAnyTypeCanBeSubtypeOf(language, compare_any->GetAnyType(), dom_compare_any->GetAnyType());
    if (!res) {
        // We cannot compare types in compile-time
        return std::nullopt;
    }

    if (res.value()) {
        // If CompareAnyTypeInst has the same types for any, then it can be optimized in any case.
        return is_true_dom_branch;
    }

    // If CompareAnyTypeInst has the different types for any, then it can be optimized only in true-branch.
    if (!is_true_dom_branch) {
        return std::nullopt;
    }

    return false;
}

/**
 * Try to resolve condition result with the information about dominant condition with the same inputs
 *
 * - The result of the dominant condition is counted using dom-tree by checking which successor of if_imm_block
 * (true/false) dominates the current block;
 *
 * - Then this result is applied to the current condition, if it is possible, using the table of condition codes
 * relation
 */
std::optional<bool> BranchElimination::TryResolveResult(Inst *condition, Inst *dominant_condition, IfImmInst *if_imm)
{
    using std::nullopt;

    // Table keeps the result of the condition in the row if the condition in the column is true
    // Order must be same as in "ir/inst.h"
    static constexpr std::array<std::array<std::optional<bool>, ConditionCode::CC_LAST + 1>, ConditionCode::CC_LAST + 1>
        // clang-format off
        COND_RELATION = {{
        //  CC_EQ     CC_NE    CC_LT    CC_LE    CC_GT    CC_GE    CC_B     CC_BE    CC_A     CC_AE
            {true,    false,   false,   nullopt, false,   nullopt, false,   nullopt, false,   nullopt}, // CC_EQ
            {false,   true,    true,    nullopt, true,    nullopt, true,    nullopt, true,    nullopt}, // CC_NE
            {false,   nullopt, true,    nullopt, false,   false,   nullopt, nullopt, nullopt, nullopt}, // CC_LT
            {true,    nullopt, true,    true,    false,   nullopt, nullopt, nullopt, nullopt, nullopt}, // CC_LE
            {false,   nullopt, false,   false,   true,    nullopt, nullopt, nullopt, nullopt, nullopt}, // CC_GT
            {true,    nullopt, false,   nullopt, true,    true,    nullopt, nullopt, nullopt, nullopt}, // CC_GE
            {false,   nullopt, nullopt, nullopt, nullopt, nullopt, true,    nullopt, false,   false},   // CC_B
            {true,    nullopt, nullopt, nullopt, nullopt, nullopt, true,    true,    false,   nullopt}, // CC_BE
            {false,   nullopt, nullopt, nullopt, nullopt, nullopt, false,   false,   true,    nullopt}, // CC_A
            {true,    nullopt, nullopt, nullopt, nullopt, nullopt, false,   nullopt, true,    true},    // CC_AE
        }};
    // clang-format on

    auto dominant_cc = dominant_condition->CastToCompare()->GetCc();
    // Swap the dominant condition code, if inputs are reversed: 'if (a < b)' -> 'if (b > a)'
    if (condition->GetInput(0) != dominant_condition->GetInput(0)) {
        ASSERT(condition->GetInput(0) == dominant_condition->GetInput(1));
        dominant_cc = SwapOperandsConditionCode(dominant_cc);
    }

    // Reverse the `dominant_cc` if the `condition` is reached after branching the false succesor of the
    // if_imm's basic block
    auto condition_bb = condition->GetBasicBlock();
    if (if_imm->GetEdgeIfInputFalse()->IsDominate(condition_bb)) {
        dominant_cc = GetInverseConditionCode(dominant_cc);
    } else {
        ASSERT(if_imm->GetEdgeIfInputTrue()->IsDominate(condition_bb));
    }
    // After these transformations dominant condition with current `dominant_cc` is equal to `true`
    // So `condition` result is resolved via table
    return COND_RELATION[condition->CastToCompare()->GetCc()][dominant_cc];
}
}  // namespace panda::compiler
