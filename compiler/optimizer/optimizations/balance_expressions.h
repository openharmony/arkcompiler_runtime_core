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

#ifndef COMPILER_OPTIMIZER_BALANCE_EXPRESSIONS_H_
#define COMPILER_OPTIMIZER_BALANCE_EXPRESSIONS_H_

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
class BalanceExpressions : public Optimization {
public:
    // For current realization, any binary commutative and associative operator suits;

    static constexpr size_t DEFAULT_GRAPH_DEPTH = 3;

    explicit BalanceExpressions(Graph *graph)
        : Optimization(graph),
          sources_(graph->GetLocalAllocator()->Adapter()),
          operators_(graph->GetLocalAllocator()->Adapter())
    {
        sources_.reserve(1U << DEFAULT_GRAPH_DEPTH);
        operators_.reserve(1U << DEFAULT_GRAPH_DEPTH);
    }

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "BalanceExpressions";
    }

    bool IsEnable() const override
    {
        return options.IsCompilerBalanceExpressions();
    }

    void Dump(std::ostream *out) const;

    void InvalidateAnalyses() override;

private:
    void ProcessBB(BasicBlock *bb);
    bool SuitableInst(Inst *inst);

    Inst *ProccesExpressionChain(Inst *last_operator);
    void AnalyzeInputsRec(Inst *inst);
    void TryExtendChainRec(Inst *inst);

    bool NeedsOptimization();
    Inst *OptimizeExpression(Inst *inst_after_expr);
    template <bool is_first_call>
    Inst *AllocateSourcesRec(size_t first_idx, size_t last_idx);
    Inst *GetOperand(size_t first_idx, size_t last_idx);
    Inst *GenerateElementalOperator(Inst *lhs, Inst *rhs);
    void Reset();
    void Dump();

    void SetOpcode(Opcode opc)
    {
        opcode_ = opc;
    }
    Opcode GetOpcode() const
    {
        return opcode_;
    }
    void SetBB(BasicBlock *bb)
    {
        bb_ = bb;
    }
    BasicBlock *GetBB() const
    {
        return bb_;
    }
    void SetIsApplied(bool value)
    {
        is_applied_ = value;
    }

    // Calculates the lowest integral power of two that is greater than `val`
    // Same as std::bit_ceil() introduced in C++20
    template <typename T>
    T GetBitCeil(T val)
    {
        static_assert(std::is_unsigned<T>::value);
        if (val <= 1) {
            return 1;
        }

        size_t highest_bit_idx = 0;
        T val_copy = val;
        while (val_copy != 0) {
            val_copy = val_copy >> 1U;
            highest_bit_idx++;
        }

        T res = 1U;
        ASSERT(highest_bit_idx >= 1);
        if ((val << (std::numeric_limits<T>::digits - highest_bit_idx + 1)) == 0) {
            // (highest_bit_idx >= 1) as soon as (val > 1)
            // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
            return res << (highest_bit_idx - 1);
        }
        return res << (highest_bit_idx);
    }

    // If x is not 0 or 1, calculates the largest integral power of two that is less than `val`
    // Same as std::bit_floor() introduced in C++20, except that return strictly less than `val`
    template <typename T>
    T GetBitFloor(T val)
    {
        static_assert(std::is_unsigned<T>::value);
        if (val <= 1) {
            return 0;
        }

        size_t highest_bit_idx = 0;
        T val_copy = val;
        while (val_copy != 0) {
            val_copy = val_copy >> 1U;
            highest_bit_idx++;
        }
        T res = 1U;
        ASSERT(highest_bit_idx >= 2U);
        if ((val << (std::numeric_limits<T>::digits - highest_bit_idx + 1)) == 0) {
            // (highest_bit_idx >= 2) as soon as (val > 1)
            // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
            return res << (highest_bit_idx - 2U);
        }
        // (highest_bit_idx >= 2) as soon as (val > 1)
        // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
        return res << (highest_bit_idx - 1);
    }

private:
    Opcode opcode_ {Opcode::INVALID};
    Marker processed_inst_mrk_ = UNDEF_MARKER;

    // i.e. critical path
    size_t expr_cur_depth_ {0};
    size_t expr_max_depth_ {0};

    BasicBlock *bb_ {nullptr};

    // Expression's terms:
    InstVector sources_;

    // Expression's operators:
    InstVector operators_;
    size_t operators_alloc_idx_ = 0;

    bool is_applied_ {false};
};

inline std::ostream &operator<<(std::ostream &os, const BalanceExpressions &expr)
{
    expr.Dump(&os);
    return os;
}
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_BALANCE_EXPRESSIONS_H_
