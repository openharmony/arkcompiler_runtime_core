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

#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "compiler_logger.h"
#include "optimizer/optimizations/balance_expressions.h"

namespace panda::compiler {
bool BalanceExpressions::RunImpl()
{
    processed_inst_mrk_ = GetGraph()->NewMarker();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        ProcessBB(bb);
    }
    GetGraph()->EraseMarker(processed_inst_mrk_);
    return is_applied_;
}

void BalanceExpressions::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
}

/**
 *  Iterate over instructions in reverse order, find every expression-chain by detecting
 *  the final operator of each chain, analyze the chain and optimize it if necessary.
 */
void BalanceExpressions::ProcessBB(BasicBlock *bb)
{
    ASSERT(bb != nullptr);
    SetBB(bb);

    auto it = bb->InstsReverse().begin();
    for (; it != it.end(); ++it) {
        ASSERT(*it != nullptr);
        if ((*it)->SetMarker(processed_inst_mrk_)) {
            // The instruction is already processed;
            continue;
        }
        if (SuitableInst(*it)) {
            // The final operator of the chain is found, start analyzing:
            auto inst_to_continue_cycle = ProccesExpressionChain(*it);
            it.SetCurrent(inst_to_continue_cycle);
        }
    }
}

bool BalanceExpressions::SuitableInst(Inst *inst)
{
    // Floating point operations are not associative:
    if (inst->IsCommutative() && !IsFloatType(inst->GetType())) {
        SetOpcode(inst->GetOpcode());
        return true;
    }
    return false;
}

Inst *BalanceExpressions::ProccesExpressionChain(Inst *last_operator)
{
    ASSERT(last_operator != nullptr);
    AnalyzeInputsRec(last_operator);

    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "\nConsidering expression:";
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << *this;

    auto inst_to_continue = NeedsOptimization() ? OptimizeExpression(last_operator->GetNext()) : last_operator;

    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Expression considered.";

    Reset();
    return inst_to_continue;
}

/**
 * Optimizes expression.
 *
 * By the end of the algorithm, `operators_.front()` points to the first instruction in expression and
 * `operators_.back()` points to the last (`operators_.front()` dominates `operators_.back()`).
 */
Inst *BalanceExpressions::OptimizeExpression(Inst *inst_after_expr)
{
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Optimizing expression:";
    AllocateSourcesRec<true>(0, sources_.size() - 1);

    size_t size = operators_.size();
    operators_.front()->SetNext(operators_[1]);
    constexpr auto IMM_2 = 2;
    operators_.back()->SetPrev(operators_[size - IMM_2]);
    for (size_t i = 1; i < size - 1; i++) {
        operators_[i]->SetNext(operators_[i + 1]);
        operators_[i]->SetPrev(operators_[i - 1]);
    }
    if (inst_after_expr == nullptr) {
        GetBB()->AppendRangeInst(operators_.front(), operators_.back());
    } else {
        GetBB()->InsertRangeBefore(operators_.front(), operators_.back(), inst_after_expr);
    }

    SetIsApplied(true);
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "\nOptimized expression:";
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << *this;

    // Need to return pointer to the next instruction in order to correctly continue the cycle:
    Inst *inst_to_continue_cycle = operators_.front();
    return inst_to_continue_cycle;
}

/**
 * Generates expression for sources_ in range from @param first_idx to @param last_idx by splitting them on
 * two parts, calling itself for each part and binding them to an instruction from operators_.
 *
 * By the end of the algorithm, `operators_` are sorted in execution order
 * (`operators_.front()` is the first, `operators_.back()` is the last).
 */
template <bool is_first_call>
Inst *BalanceExpressions::AllocateSourcesRec(size_t first_idx, size_t last_idx)
{
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Allocating operators for sources_[" << first_idx << " to " << last_idx << "]";
    size_t split_idx = first_idx + GetBitFloor(last_idx - first_idx + 1) - 1;

    Inst *lhs = GetOperand(first_idx, split_idx);
    Inst *rhs = LIKELY((split_idx + 1) != last_idx) ? GetOperand(split_idx + 1, last_idx) : sources_[split_idx + 1];
    // `(split_idx + 1) == last_idx` means an odd number of `sources_` and we are considering
    // the last (unpaired) source. This situation may occur only with `rhs`.
    ASSERT(first_idx != split_idx);

    // Operator allocation:
    ASSERT(operators_alloc_idx_ < operators_.size());
    Inst *allocated_operator = operators_[operators_alloc_idx_];
    operators_alloc_idx_++;

    // Operator initialization:
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (is_first_call) {
        // The first call allocates and generates at the same time the last operator of expression and
        // its users should be saved.
        allocated_operator->RemoveInputs();
        allocated_operator->GetBasicBlock()->EraseInst(allocated_operator);
    } else {  // NOLINT(readability-misleading-indentation)
        allocated_operator->GetBasicBlock()->RemoveInst(allocated_operator);
    }
    allocated_operator->SetBasicBlock(GetBB());
    allocated_operator->SetInput(0, lhs);
    allocated_operator->SetInput(1, rhs);

    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "sources_[" << first_idx << " to " << last_idx << "] allocated";
    return allocated_operator;
}

Inst *BalanceExpressions::GetOperand(size_t first_idx, size_t last_idx)
{
    ASSERT(last_idx > first_idx);
    return (last_idx - first_idx == 1) ? GenerateElementalOperator(sources_[first_idx], sources_[last_idx])
                                       : AllocateSourcesRec<false>(first_idx, last_idx);
}

/**
 * Create an operator with direct sources
 * (i.e. `lhs` and `rhs` are from sources_)
 */
Inst *BalanceExpressions::GenerateElementalOperator(Inst *lhs, Inst *rhs)
{
    ASSERT(lhs && rhs);
    ASSERT(operators_alloc_idx_ < operators_.size());
    Inst *allocated_operator = operators_[operators_alloc_idx_];
    operators_alloc_idx_++;
    allocated_operator->GetBasicBlock()->RemoveInst(allocated_operator);

    allocated_operator->SetBasicBlock(GetBB());

    // There is no need to clean users of lhs and rhs because it is cleaned during RemoveInst()
    // (as soon as every of operator_insts_ is removed before further usage)

    allocated_operator->SetInput(0, lhs);
    allocated_operator->SetInput(1, rhs);
    COMPILER_LOG(DEBUG, BALANCE_EXPR) << "Created an elemental operator:\n" << *allocated_operator;
    return allocated_operator;
}

/**
 * Recursively checks inputs.
 * Fills arrays of source-insts and opreation-insts.
 */
void BalanceExpressions::AnalyzeInputsRec(Inst *inst)
{
    expr_cur_depth_++;
    ASSERT(inst != nullptr);

    auto lhs_input = inst->GetInput(0).GetInst();
    auto rhs_input = inst->GetInput(1).GetInst();

    TryExtendChainRec(lhs_input);
    TryExtendChainRec(rhs_input);
    operators_.push_back(inst);

    if (expr_max_depth_ < expr_cur_depth_) {
        expr_max_depth_ = expr_cur_depth_;
    }
    expr_cur_depth_--;
}

/**
 * Recursively checks if the instruction should be added in the current expression chain.
 * If not, the considered instruction is the expression's term (source) and we save it for a later step.
 */
void BalanceExpressions::TryExtendChainRec(Inst *inst)
{
    ASSERT(inst);
    if (inst->GetOpcode() == GetOpcode()) {
        if (inst->HasSingleUser()) {
            inst->SetMarker(processed_inst_mrk_);

            AnalyzeInputsRec(inst);

            return;
        }
    }
    sources_.push_back(inst);
}

/**
 * Finds optimal depth and compares to the current.
 * Both of the numbers are represented as pow(x, 2).
 */
bool BalanceExpressions::NeedsOptimization()
{
    if (sources_.size() <= 3U) {
        return false;
    }
    // Avoid large shift exponent for size_t
    if (expr_max_depth_ >= std::numeric_limits<size_t>::digits) {
        return false;
    }
    size_t current = 1UL << (expr_max_depth_);
    size_t optimal = GetBitCeil(sources_.size());
    return current > optimal;
}

void BalanceExpressions::Reset()
{
    sources_.clear();
    operators_.clear();
    expr_cur_depth_ = 0;
    expr_max_depth_ = 0;
    operators_alloc_idx_ = 0;
    SetOpcode(Opcode::INVALID);
}

void BalanceExpressions::Dump(std::ostream *out) const
{
    (*out) << "Sources:\n";
    for (auto i : sources_) {
        (*out) << *i << '\n';
    }

    (*out) << "Operators:\n";
    for (auto i : operators_) {
        (*out) << *i << '\n';
    }
}

template size_t BalanceExpressions::GetBitFloor<size_t>(size_t val);
template size_t BalanceExpressions::GetBitCeil<size_t>(size_t val);
}  // namespace panda::compiler
