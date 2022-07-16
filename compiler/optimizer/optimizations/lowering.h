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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H_

#include "compiler_logger.h"
#include "compiler_options.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"

namespace panda::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Lowering : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit Lowering(Graph *graph) : Optimization(graph) {}

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return options.IsCompilerLowering();
    }

    const char *GetPassName() const override
    {
        return "Lowering";
    }

    void InvalidateAnalyses() override;

    static constexpr uint8_t HALF_SIZE = 32;
    static constexpr uint8_t WORD_SIZE = 64;

private:
    /**
     * Utility template classes aimed to simplify pattern matching over IR-graph.
     * Patterns are trees declared as a type using Any, UnaryOp, BinaryOp and Operand composition.
     * Then IR-subtree could be tested for matching by calling static Capture method.
     * To capture operands from matched subtree Operand<Index> should be used, where
     * Index is an operand's index within OperandsCapture.
     *
     * For example, suppose that we want to test if IR-subtree rooted by some Inst matches add or sub instruction
     * pattern:
     *
     * Inst* inst = ...;
     * using Predicate = Any<BinaryOp<Opcode::Add, Operand<0>, Operand<1>,
     *                       BinaryOp<Opcode::Sub, Operand<0>, Operand<1>>;
     * OperandsCapture<2> capture{};
     * bool is_add_or_sub = Predicate::Capture(inst, capture);
     *
     * If inst is a binary instruction with opcode Add or Sub then Capture will return `true`,
     * capture.Get(0) will return left inst's input and capture.Get(1) will return right inst's input.
     */

    // Flags altering matching behavior.
    enum Flags {
        NONE = 0,
        COMMUTATIVE = 1,  // binary operation is commutative
        C = COMMUTATIVE,
        SINGLE_USER = 2,  // operation must have only single user
        S = SINGLE_USER
    };

    static bool IsSet(uint64_t flags, Flags flag)
    {
        return (flags & flag) != 0;
    }

    static bool IsNotSet(uint64_t flags, Flags flag)
    {
        return (flags & flag) == 0;
    }

    template <size_t max_operands>
    class OperandsCapture {
    public:
        Inst *Get(size_t i)
        {
            ASSERT(i < max_operands);
            return operands_[i];
        }

        void Set(size_t i, Inst *inst)
        {
            ASSERT(i < max_operands);
            operands_[i] = inst;
        }

        // Returns true if all non-constant operands have the same common type (obtained using GetCommonType) as all
        // other operands.
        bool HaveCommonType() const
        {
            auto non_const_type = DataType::LAST;
            for (size_t i = 0; i < max_operands; i++) {
                if (operands_[i]->GetOpcode() != Opcode::Constant) {
                    non_const_type = GetCommonType(operands_[i]->GetType());
                    break;
                }
            }
            // all operands are constants
            if (non_const_type == DataType::LAST) {
                non_const_type = operands_[0]->GetType();
            }
            for (size_t i = 0; i < max_operands; i++) {
                if (operands_[i]->GetOpcode() == Opcode::Constant) {
                    if (GetCommonType(operands_[i]->GetType()) != GetCommonType(non_const_type)) {
                        return false;
                    }
                } else if (non_const_type != GetCommonType(operands_[i]->GetType())) {
                    return false;
                }
            }
            return true;
        }

    private:
        std::array<Inst *, max_operands> operands_;
    };

    template <size_t max_insts>
    class InstructionsCapture {
    public:
        void Add(Inst *inst)
        {
            ASSERT(current_idx_ < max_insts);
            insts_[current_idx_++] = inst;
        }

        size_t GetCurrentIndex() const
        {
            return current_idx_;
        }

        void SetCurrentIndex(size_t idx)
        {
            ASSERT(idx < max_insts);
            current_idx_ = idx;
        }

        // Returns true if all non-constant operands have exactly the same type and all
        // constant arguments have the same common type (obtained using GetCommonType) as all other operands.
        bool HaveSameType() const
        {
            ASSERT(current_idx_ == max_insts);
            auto non_const_type = DataType::LAST;
            for (size_t i = 0; i < max_insts; i++) {
                if (insts_[i]->GetOpcode() != Opcode::Constant) {
                    non_const_type = insts_[i]->GetType();
                    break;
                }
            }
            // all operands are constants
            if (non_const_type == DataType::LAST) {
                non_const_type = insts_[0]->GetType();
            }
            for (size_t i = 0; i < max_insts; i++) {
                if (insts_[i]->GetOpcode() == Opcode::Constant) {
                    if (GetCommonType(insts_[i]->GetType()) != GetCommonType(non_const_type)) {
                        return false;
                    }
                } else if (non_const_type != insts_[i]->GetType()) {
                    return false;
                }
            }
            return true;
        }

        InstructionsCapture &ResetIndex()
        {
            current_idx_ = 0;
            return *this;
        }

    private:
        std::array<Inst *, max_insts> insts_;
        size_t current_idx_;
    };

    template <Opcode opcode, typename L, typename R, uint64_t flags = Flags::NONE>
    struct BinaryOp {
        template <size_t max_operands, size_t max_insts>
        static bool Capture(Inst *inst, OperandsCapture<max_operands> &args, InstructionsCapture<max_insts> &insts)
        {
            constexpr auto INPUTS_NUM = 2;
            // NOLINTNEXTLINE(readability-magic-numbers)
            if (inst->GetOpcode() != opcode || inst->GetInputsCount() != INPUTS_NUM ||
                (IsSet(flags, Flags::SINGLE_USER) && !inst->HasSingleUser())) {
                return false;
            }
            if (L::Capture(inst->GetInput(0).GetInst(), args, insts) &&
                R::Capture(inst->GetInput(1).GetInst(), args, insts)) {
                insts.Add(inst);
                return true;
            }
            if (IsSet(flags, Flags::COMMUTATIVE) && L::Capture(inst->GetInput(1).GetInst(), args, insts) &&
                R::Capture(inst->GetInput(0).GetInst(), args, insts)) {
                insts.Add(inst);
                return true;
            }
            return false;
        }
    };

    template <Opcode opcode, typename T, uint64_t flags = Flags::NONE>
    struct UnaryOp {
        template <size_t max_operands, size_t max_insts>
        static bool Capture(Inst *inst, OperandsCapture<max_operands> &args, InstructionsCapture<max_insts> &insts)
        {
            // NOLINTNEXTLINE(readability-magic-numbers)
            bool matched = inst->GetOpcode() == opcode && inst->GetInputsCount() == 1 &&
                           (IsNotSet(flags, Flags::SINGLE_USER) || inst->HasSingleUser()) &&
                           T::Capture(inst->GetInput(0).GetInst(), args, insts);
            if (matched) {
                insts.Add(inst);
            }
            return matched;
        }
    };

    template <size_t idx>
    struct Operand {
        template <size_t max_operands, size_t max_insts>
        static bool Capture(Inst *inst, OperandsCapture<max_operands> &args,
                            [[maybe_unused]] InstructionsCapture<max_insts> &insts)
        {
            static_assert(idx < max_operands, "Operand's index should not exceed OperandsCapture size");

            args.Set(idx, inst);
            return true;
        }
    };

    template <typename T, typename... Args>
    struct AnyOf {
        template <size_t max_operands, size_t max_insts>
        static bool Capture(Inst *inst, OperandsCapture<max_operands> &args, InstructionsCapture<max_insts> &insts)
        {
            size_t inst_idx = insts.GetCurrentIndex();
            if (T::Capture(inst, args, insts)) {
                return true;
            }
            insts.SetCurrentIndex(inst_idx);
            return AnyOf<Args...>::Capture(inst, args, insts);
        }
    };

    template <typename T>
    struct AnyOf<T> {
        template <size_t max_operands, size_t max_insts>
        static bool Capture(Inst *inst, OperandsCapture<max_operands> &args, InstructionsCapture<max_insts> &insts)
        {
            return T::Capture(inst, args, insts);
        }
    };

    using SRC0 = Operand<0>;
    using SRC1 = Operand<1>;
    using SRC2 = Operand<2U>;

    template <typename L, typename R, uint64_t F = Flags::NONE>
    using ADD = BinaryOp<Opcode::Add, L, R, F | Flags::COMMUTATIVE>;
    template <typename L, typename R, uint64_t F = Flags::NONE>
    using SUB = BinaryOp<Opcode::Sub, L, R, F>;
    template <typename L, typename R, uint64_t F = Flags::NONE>
    using MUL = BinaryOp<Opcode::Mul, L, R, F | Flags::COMMUTATIVE>;
    template <typename T, uint64_t F = Flags::NONE>
    using NEG = UnaryOp<Opcode::Neg, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using NOT = UnaryOp<Opcode::Not, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using SHRI = UnaryOp<Opcode::ShrI, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using ASHRI = UnaryOp<Opcode::AShrI, T, F>;
    template <typename T, uint64_t F = Flags::SINGLE_USER>
    using SHLI = UnaryOp<Opcode::ShlI, T, F>;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    static void VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, Inst *inst);

    template <Opcode opc>
    static void VisitBitwiseBinaryOperation([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOr(GraphVisitor *v, Inst *inst);
    static void VisitAnd(GraphVisitor *v, Inst *inst);
    static void VisitXor(GraphVisitor *v, Inst *inst);

    static void VisitAndNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitXorNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitOrNot([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSaveState([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSafePoint([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSaveStateOsr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitSaveStateDeoptimize([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitBoundsCheck([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadArray([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoadCompressedStringChar([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStoreArray([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitLoad([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitStore([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMul([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst);
    static void VisitDeoptimizeIf(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

    static void InsertInstruction(Inst *inst, Inst *new_inst)
    {
        inst->InsertBefore(new_inst);
        inst->ReplaceUsers(new_inst);
        new_inst->GetBasicBlock()->GetGraph()->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()),
                                                                              inst->GetId(), inst->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
    }

    template <size_t max_operands>
    static void SetInputsAndInsertInstruction(OperandsCapture<max_operands> &operands, Inst *inst, Inst *new_inst);

    static constexpr Opcode GetInstructionWithShiftedOperand(Opcode opcode);
    static constexpr Opcode GetInstructionWithInvertedOperand(Opcode opcode);
    static ShiftType GetShiftTypeByOpcode(Opcode opcode);
    static Inst *GetCheckInstAndGetConstInput(Inst *inst);
    static ShiftOpcode ConvertOpcode(Opcode new_opcode);

    static void LowerMemInstScale(Inst *inst);
    static void LowerShift(Inst *inst);
    static bool ConstantFitsCompareImm(Inst *cst, uint32_t size, ConditionCode cc);
    static Inst *LowerAddSub(Inst *inst);
    template <Opcode opcode>
    static void LowerMulDivMod(Inst *inst);
    static Inst *LowerMultiplyAddSub(Inst *inst);
    static Inst *LowerNegateMultiply(Inst *inst);
    static void LowerLogicWithInvertedOperand(Inst *inst);
    static bool LowerCastValueToAnyTypeWithConst(Inst *inst);
    template <typename T, size_t max_operands>
    static Inst *LowerOperationWithShiftedOperand(Inst *inst, OperandsCapture<max_operands> &operands, Inst *shift_inst,
                                                  Opcode new_opcode);
    template <Opcode opcode, bool is_commutative = true>
    static Inst *LowerBinaryOperationWithShiftedOperand(Inst *inst);
    template <Opcode opcode>
    static void LowerUnaryOperationWithShiftedOperand(Inst *inst);
    static Inst *LowerLogic(Inst *inst);
    template <typename LowLevelType>
    static void LowerConstArrayIndex(Inst *inst, Opcode low_level_opcode);
    static void LowerStateInst(SaveStateInst *save_state);
    static void LowerReturnInst(FixedInputsInst1 *ret);
    // We'd like to swap only to make second operand immediate
    static bool BetterToSwapCompareInputs(Inst *cmp);
    // Optimize order of input arguments for decreasing using accumulator (Bytecodeoptimizer only).
    static void OptimizeIfInput(compiler::Inst *if_inst);
    static void JoinFcmpInst(IfImmInst *inst, CmpInst *input);
    static void LowerIf(IfImmInst *inst);
    static void InPlaceLowerIfImm(IfImmInst *inst, Inst *input, Inst *cst, ConditionCode cc);
    static void LowerToDeoptimizeCompare(Inst *inst);
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H_
