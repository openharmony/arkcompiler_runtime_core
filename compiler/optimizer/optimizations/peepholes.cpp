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

#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/optimizations/const_folding.h"
#include "optimizer/optimizations/object_type_check_elimination.h"
#include "peepholes.h"

namespace panda::compiler {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PEEPHOLE_IS_APPLIED(visitor, inst) visitor->SetIsApplied(inst, true, __FILE__, __LINE__)

void Peepholes::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool Peepholes::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    VisitGraph();
    return IsApplied();
}

void Peepholes::VisitSafePoint(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToSafePoint()->RemoveNumericInputs()) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    }
}

void Peepholes::VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingNeg(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    auto input_opc = input->GetOpcode();
    auto inst_type = inst->GetType();
    auto input_type = input->GetType();
    if (DataType::IsFloatType(inst_type)) {
        return;
    }
    // Repeated application of the Neg
    // 1. Some inst -> {v2}
    // 2.i64 neg v1 -> {v3, ...}
    // 3.i64 neg v2 -> {...}
    // ===>
    // 1. Some inst -> {vv3 users}
    // 2.i64 neg v1 -> {v3, ...}
    // 3.i64 neg v2
    if (input_opc == Opcode::Neg && inst_type == input_type) {
        inst->ReplaceUsers(input->GetInput(0).GetInst());
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    // Сhanging the parameters of subtraction with the use of Neg
    // 2.i64 sub v0, v1 -> {v3, ...}
    // 3.i64 neg v2 -> {...}
    // ===>
    // 2.i64 sub v0, v1 -> {v3, ...}
    // 3.i64 neg v2 -> {}
    // 4.i64 sub v1, v0 -> {users v3}
    if (input_opc == Opcode::Sub && inst_type == input_type) {
        CreateAndInsertInst(Opcode::Sub, inst, input->GetInput(1).GetInst(), input->GetInput(0).GetInst());
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
}

void Peepholes::VisitAbs([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!DataType::IsTypeSigned(inst->GetType())) {
        inst->ReplaceUsers(inst->GetInput(0).GetInst());
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    if (ConstFoldingAbs(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
}

void Peepholes::VisitNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingNot(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
}

/**
 * Case 1: Swap inputs if the first is a constant
 * 2. add const, v1 -> {...}
 * ===>
 * 2. add v1, const -> {...}
 * Case 2: Addition with zero
 * 1. Some inst -> v2
 * 2. add v1, 0 -> {...}
 * ===>
 * 1. Some inst ->{...}
 * 2. add v1, 0 -> {}
 * Case 3: Repeated arithmetic with constants
 * 2. add/sub v1, const1 -> {v3, ...}
 * 3. add v2, const2 -> {...}
 * ===>
 * 2. add/sub v1, const1 -> {...}
 * 3. add v1, const2 +/- const1 ->{...}
 * Case 4: Addition with Neg
 * 3. neg v1 -> {5}
 * 4. neg v2 -> {5}
 * 5. add v3, v4 -> {...}
 * ===>
 * 3. neg v1 -> {}
 * 4. neg v2 -> {}
 * 5. add v1, v2 -> {4}
 * 6. neg v5 -> {users v5}
 * Case 5:
 * 3. neg v2 -> {4, ...}
 * 4. add v1, v3 -> {...}
 * ===>
 * 3. neg v2 -> {...}
 * 4. sub v1, v2 -> {...}
 * Case 6:
 * Adding and subtracting the same value
 * 3. sub v2, v1 -> {4, ...}
 * 4. add v3, v1 -> {...}
 * ===>
 * 3. sub v2, v1 -> {4, ...}
 * 4. add v3, v1 -> {}
 * v2 -> {users v4}
 */
void Peepholes::VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingAdd(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Case 1
    visitor->TrySwapInputs(inst);
    if (visitor->TrySimplifyAddSubWithConstInput(inst)) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input0->GetOpcode() == Opcode::Neg && input1->GetOpcode() == Opcode::Neg) {
        // Case 4
        if (input0->HasSingleUser() && input1->HasSingleUser()) {
            inst->SetInput(0, input0->GetInput(0).GetInst());
            inst->SetInput(1, input1->GetInput(0).GetInst());
            CreateAndInsertInst(Opcode::Neg, inst, inst);
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    } else {
        // Case 5
        visitor->TrySimplifyNeg(inst);
    }
    // Case 6
    visitor->TrySimplifyAddSub<Opcode::Sub, 0>(inst, input0, input1);
    visitor->TrySimplifyAddSub<Opcode::Sub, 0>(inst, input1, input0);

    // Some of the previous transformations might change the opcode of inst.
    if (inst->GetOpcode() == Opcode::Add) {
        if (visitor->TryReassociateShlShlAddSub(inst)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }

        if (input0->GetOpcode() == Opcode::Add && input1->GetOpcode() == Opcode::Sub) {
            std::swap(input0, input1);
        }

        if (input0->GetOpcode() == Opcode::Sub) {
            if (input1->GetOpcode() == Opcode::Add) {
                if (visitor->TrySimplifyAddSubAdd(inst, input0, input1)) {
                    return;
                }
            }

            if (input1->GetOpcode() == Opcode::Sub) {
                if (visitor->TrySimplifyAddSubSub(inst, input0, input1)) {
                    return;
                }
            }
        }
    }
}

/**
 * Case 1
 * Subtraction of zero
 * 1. Some inst
 * 2. sub v1, 0 -> {...}
 * ===>
 * 1. Some inst
 * 2. sub v1, 0 -> {}
 * Case 2
 * Repeated arithmetic with constants
 * 2. add/sub v1, const1 -> {v3, ...}
 * 3. sub v2, const2 -> {...}
 * ===>
 * 2. add/sub v1, const1 -> {...}
 * 3. sub v1, const2 -/+ const1 ->{...}
 * Case 3
 * Subtraction of Neg
 * 3. neg v1 -> {5, ...}
 * 4. neg v2 -> {5, ...}
 * 5. sub v3, v4 -> {...}
 * ===>
 * 3. neg v1 -> {...}
 * 4. neg v2 -> {...}
 * 5. sub v2, v1 -> {...}
 * Case 4
 * Adding and subtracting the same value
 * 3. add v1, v2 -> {4, ...}
 * 4. sub v3, v2 -> {...}
 * ===>
 * 3. add v1, v2 -> {4, ...}
 * 4. sub v3, v1 -> {}
 * v1 -> {users v4}
 * Case 5
 * 3. sub v1, v2 -> {4, ...}
 * 4. sub v1, v3 -> {...}
 * ===>
 * 3. sub v1, v2 -> {4, ...}
 * 4. sub v1, v3 -> {}
 * v2 -> {users v4}
 * Case 6
 * 1. Some inst
 * 2. sub 0, v1 -> {...}
 * ===>
 * 1. Some inst
 * 2. neg v1 -> {...}
 */
void Peepholes::VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingSub(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Case 1, 2, 6
    if (visitor->TrySimplifyAddSubWithConstInput(inst)) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    // Case 3
    if (input0->GetOpcode() == Opcode::Neg && input1->GetOpcode() == Opcode::Neg) {
        auto input0_tmp = input0->GetInput(0).GetInst();
        inst->SetInput(0, input1->GetInput(0).GetInst());
        inst->SetInput(1, input0_tmp);
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (input1->GetOpcode() == Opcode::Neg) {
        inst->SetInput(1, input1->GetInput(0).GetInst());
        inst->SetOpcode(Opcode::Add);
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    // Case 4
    visitor->TrySimplifyAddSub<Opcode::Add, 0>(inst, input0, input1);
    visitor->TrySimplifyAddSub<Opcode::Add, 1>(inst, input0, input1);
    // Case 5
    if (input1->GetOpcode() == Opcode::Sub && input1->GetInput(0).GetInst() == input0) {
        auto prev_input = input1->GetInput(1).GetInst();
        if (inst->GetType() == prev_input->GetType()) {
            inst->ReplaceUsers(prev_input);
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    }

    // Some of the previous transformations might change the opcode of inst.
    if (inst->GetOpcode() == Opcode::Sub) {
        if (visitor->TryReassociateShlShlAddSub(inst)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }

        if (input0->GetOpcode() == Opcode::Add && input1->GetOpcode() == Opcode::Add) {
            if (visitor->TrySimplifySubAddAdd(inst, input0, input1)) {
                return;
            }
        }
    }
}

void Peepholes::VisitMulOneConst([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    if (!input1->IsConst()) {
        return;
    }
    auto const_inst = static_cast<ConstantInst *>(input1);
    if (const_inst->IsEqualConstAllTypes(1)) {
        // case 1:
        // 0. Const 1
        // 1. MUL v5, v0 -> {v2, ...}
        // 2. INS v1
        // ===>
        // 0. Const 1
        // 1. MUL v5, v0
        // 2. INS v5
        inst->ReplaceUsers(input0);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (const_inst->IsEqualConstAllTypes(-1)) {
        // case 2:
        // 0. Const -1
        // 1. MUL v5, v0 -> {v2, ...}
        // 2. INS v1
        // ===>
        // 0. Const -1
        // 1. MUL v5, v0
        // 3. NEG v5 -> {v2, ...}
        // 2. INS v3
        CreateAndInsertInst(Opcode::Neg, inst, input0);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (const_inst->IsEqualConstAllTypes(2U)) {
        // case 3:
        // 0. Const 2
        // 1. MUL v5, v0 -> {v2, ...}
        // 2. INS v1
        // ===>
        // 0. Const -1
        // 1. MUL v5, v0
        // 3. ADD v5 , V5 -> {v2, ...}
        // 2. INS v3
        CreateAndInsertInst(Opcode::Add, inst, input0, input0);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
        int64_t n = GetPowerOfTwo(const_inst->GetIntValue());
        if (n != -1) {
            // case 4:
            // 0. Const 2^N
            // 1. MUL v5, v0 -> {v2, ...}
            // 2. INS v1
            // ===>
            // 0. Const -1
            // 1. MUL v5, v0
            // 3. SHL v5 , N -> {v2, ...}
            // 2. INS v3
            ConstantInst *power = ConstFoldingCreateIntConst(inst, static_cast<uint64_t>(n));
            CreateAndInsertInst(Opcode::Shl, inst, input0, power);
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
        }
    }
}

void Peepholes::VisitMul(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingMul(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    visitor->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = input1->CastToConstant();
        if (input0->GetOpcode() == Opcode::Mul && TryCombineMulConst(inst, cnst)) {
            // 0. Const 3
            // 1. Const 7
            // 2. ...
            // 3. Mul v2, v0
            // 4. Mul v3, v1
            // ===>
            // 5. Const 21
            // 2. ...
            // 4. Mul v2, v5
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    }
    VisitMulOneConst(v, inst, input0, input1);
}

void Peepholes::VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingDiv(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto const_inst = static_cast<ConstantInst *>(input1);
        if (const_inst->IsEqualConstAllTypes(1)) {
            // case 1:
            // 0. Const 1
            // 1. DIV v5, v0 -> {v2, ...}
            // 2. INS v1
            // ===>
            // 0. Const 1
            // 1. DIV v5, v0
            // 2. INS v5
            inst->ReplaceUsers(input0);
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
        } else if (const_inst->IsEqualConstAllTypes(-1)) {
            // case 2:
            // 0. Const -1
            // 1. DIV v5, v0 -> {v2, ...}
            // 2. INS v1
            // ===>
            // 0. Const -1
            // 1. DIV v5, v0
            // 3. NEG v5 -> {v2, ...}
            // 2. INS v3
            CreateAndInsertInst(Opcode::Neg, inst, input0);
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
        } else if (DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
            static_cast<Peepholes *>(v)->TryReplaceDivByShift(inst);
        }
    }
}

void Peepholes::VisitMin([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingMin(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Swap inputs if the first is a constant
    // 2. Min const, v1 -> {...}
    // ===>
    // 2. Min v1, const -> {...}
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
}

void Peepholes::VisitMax([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingMax(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return;
    }
    // Swap inputs if the first is a constant
    // 2. Max const, v1 -> {...}
    // ===>
    // 2. Max v1, const -> {...}
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
}

void Peepholes::VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingMod(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
}

void Peepholes::VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingShl(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    static_cast<Peepholes *>(v)->TrySimplifyShifts(inst);
}

void Peepholes::VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingShr(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    static_cast<Peepholes *>(v)->TrySimplifyShifts(inst);
    // TrySimplifyShifts can replace inst by another.
    if (inst->GetUsers().Empty()) {
        return;
    }
    // case 1
    // 0.i64 Constant X
    // 1. ...
    // 2.Type Shl v1, v0
    // 3.Type Shr v2, v0
    // ====>
    // 0.i64 Constant X
    // 4.i64 Constant (1U << (Size(Type) - X)) - 1
    // 1. ...
    // 5.Type And v1, v4
    auto op1 = inst->GetInput(0).GetInst();
    auto op2 = inst->GetInput(1).GetInst();
    if (op1->GetOpcode() == Opcode::Shl && op2->IsConst() && op1->GetInput(1).GetInst() == op2) {
        uint64_t val = op2->CastToConstant()->GetIntValue();
        ASSERT(inst->GetType() == op1->GetType());
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto arch = graph->GetArch();
        ASSERT(DataType::GetTypeSize(inst->GetType(), arch) >= val);
        auto t = static_cast<uint8_t>(DataType::GetTypeSize(inst->GetType(), arch) - val);
        uint64_t mask = (1UL << t) - 1;
        auto new_cnst = graph->FindOrCreateConstant(mask);
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            t = static_cast<uint8_t>(DataType::GetTypeSize(inst->GetType(), arch) - static_cast<uint32_t>(val));
            mask = static_cast<uint32_t>((1U << t) - 1);
            new_cnst = graph->FindOrCreateConstant<uint32_t>(mask);
        }
        CreateAndInsertInst(Opcode::And, inst, op1->GetInput(0).GetInst(), new_cnst);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    }
}

void Peepholes::VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingAShr(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    /**
     * This VisitAShr() may new create some Cast instruction which will cause failure in the codegen stage.
     * Even though these Casts' resolving can be supported in bytecode_optimizer's CodeGen,
     * they will be translated back to 'shl && ashr', therefore, this part is skipped in bytecode_opt mode.
     */
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return;
    }
    static_cast<Peepholes *>(v)->TrySimplifyShifts(inst);
    // TrySimplifyShifts can replace inst by another.
    if (inst->GetUsers().Empty()) {
        return;
    }
    // case 1
    // 0.i64 Constant
    // 1. ...
    // 2.Type Shl  v1, v0
    // 3.Type AShr v2, v0
    // ====>
    // 0.i64 Constant
    // 1. ...
    // 5. Cast v1
    auto op1 = inst->GetInput(0).GetInst();
    auto op2 = inst->GetInput(1).GetInst();
    if (op1->GetOpcode() == Opcode::Shl && op2->IsConst() && op1->GetInput(1).GetInst() == op2) {
        ASSERT(inst->GetType() == op1->GetType());
        const uint64_t OFFSET_INT8 = 24;
        const uint64_t OFFSET_INT16 = 16;
        const uint64_t OFFSET_INT32 = 8;
        auto offset = op2->CastToConstant()->GetIntValue();
        if (offset == OFFSET_INT16 || offset == OFFSET_INT8 || offset == OFFSET_INT32) {
            auto cast = CreateAndInsertInst(Opcode::Cast, inst, op1->GetInput(0).GetInst());
            cast->SetType((offset == OFFSET_INT8) ? DataType::INT8
                                                  : (offset == OFFSET_INT16) ? DataType::INT16 : DataType::INT32);
            cast->CastToCast()->SetOperandsType(op1->GetInput(0).GetInst()->GetType());
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
        }
    }
}

static bool ApplyForCastU16(GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    return input1->IsConst() && input0->GetOpcode() == Opcode::Cast &&
           DataType::GetCommonType(input0->GetInput(0).GetInst()->GetType()) == DataType::INT64 &&
           IsInt32Bit(inst->GetType()) &&
           DataType::GetTypeSize(input0->GetType(), static_cast<Peepholes *>(v)->GetGraph()->GetArch()) > HALF_SIZE;
}

void Peepholes::VisitAnd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingAnd(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    // case 1:
    // 0.i64 Const ...
    // 1.i64 AND v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 AND v5, v0
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if (input1->IsConst() && static_cast<ConstantInst *>(input1)->GetIntValue() == static_cast<uint64_t>(-1)) {
        // case 2:
        // 0.i64 Const 0xFFF..FF
        // 1.i64 AND v5, v0
        // 2.i64 INS which use v1
        // ===>
        // 0.i64 Const 0xFFF..FF
        // 1.i64 AND v5, v0
        // 2.i64 INS which use v5
        inst->ReplaceUsers(input0);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (input0 == input1 && input0->GetType() == inst->GetType()) {
        // case 3:
        // 1.i64 AND v5, v5
        // 2.i64 INS which use v1
        // ===>
        // 1.i64 AND v5, v5
        // 2.i64 INS which use v5
        inst->ReplaceUsers(input0);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (input0->GetOpcode() == Opcode::Not && input1->GetOpcode() == Opcode::Not && input0->HasSingleUser() &&
               input1->HasSingleUser()) {
        // case 4: De Morgan rules
        // 2.i64 not v0 -> {4}
        // 3.i64 not v1 -> {4}
        // 4.i64 AND v2, v3
        // ===>
        // 5.i64 OR v0, v1
        // 6.i64 not v5
        auto not_input0 = input0->GetInput(0).GetInst();
        auto not_input1 = input1->GetInput(0).GetInst();
        auto or_inst = CreateAndInsertInst(Opcode::Or, inst, not_input0, not_input1);
        CreateAndInsertInst(Opcode::Not, or_inst, or_inst);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (ApplyForCastU16(v, inst, input0, input1)) {
        // case 5: IR for cast i64 to u16 is
        // 2.i32 cast i64 to i32
        // 3.i32 and v2, 0xFFFF
        // replace it with cast i64tou16
        auto val = input1->CastToConstant()->GetIntValue();
        constexpr auto mask = (1U << HALF_SIZE) - 1;
        if (val == mask) {
            auto cast = CreateAndInsertInst(Opcode::Cast, inst, input0->GetInput(0).GetInst());
            cast->SetType(DataType::UINT16);
            cast->CastToCast()->SetOperandsType(input0->GetInput(0).GetInst()->GetType());
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
        }
    }
}

void Peepholes::VisitOr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingOr(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    // case 1:
    // 0.i64 Const ...
    // 1.i64 Or v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 Or v5, v0
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if (input1->IsConst() && static_cast<ConstantInst *>(input1)->GetIntValue() == static_cast<uint64_t>(0)) {
        // case 2:
        // 0.i64 Const 0x000..00
        // 1.i64 OR v5, v0
        // 2.i64 INS which use v1
        // ===>
        // 0.i64 Const 0x000..00
        // 1.i64 OR v5, v0
        // 2.i64 INS which use v5
        inst->ReplaceUsers(input0);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (input0 == input1 && input0->GetType() == inst->GetType()) {
        // case 3:
        // 1.i64 OR v5, v5
        // 2.i64 INS which use v1
        // ===>
        // 1.i64 OR v5, v5
        // 2.i64 INS which use v5
        inst->ReplaceUsers(input0);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    } else if (input0->GetOpcode() == Opcode::Not && input1->GetOpcode() == Opcode::Not && input0->HasSingleUser() &&
               input1->HasSingleUser()) {
        // case 4: De Morgan rules
        // 2.i64 not v0 -> {4}
        // 3.i64 not v1 -> {4}
        // 4.i64 OR v2, v3
        // ===>
        // 5.i64 AND v0, v1
        // 6.i64 not v5
        auto not_input0 = input0->GetInput(0).GetInst();
        auto not_input1 = input1->GetInput(0).GetInst();
        auto and_inst = CreateAndInsertInst(Opcode::And, inst, not_input0, not_input1);
        CreateAndInsertInst(Opcode::Not, and_inst, and_inst);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    }
}

void Peepholes::VisitXor([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingXor(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    // Swap inputs if the first is a constant
    // 2. Xor const, v1 -> {...}
    // ===>
    // 2. Xor v1, const -> {...}
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        // replace xor with not:
        // 0.i64 Const -1
        // 1.i64 XOR v0, v1
        // ===>
        // 1.i64 NOT v1
        uint64_t val = input1->CastToConstant()->GetIntValue();
        if (static_cast<int64_t>(val) == -1) {
            CreateAndInsertInst(Opcode::Not, inst, input0);
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
            // A xor 0 = A
        } else if (input1->CastToConstant()->GetIntValue() == 0) {
            inst->ReplaceUsers(input0);
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
        }
    }
}

void Peepholes::VisitCmp(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingCmp(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }
}

void Peepholes::VisitCompare(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    if (ConstFoldingCompare(inst)) {
        visitor->SetIsApplied(inst);
        return;
    }
    if (visitor->TrySimplifyCompareWithBoolInput(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyCmpCompareWithZero(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyCompareAndZero(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (TrySimplifyCompareLenArrayWithZero(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyTestEqualInputs(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyCompareAnyType(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }
}

bool Peepholes::TrySimplifyCompareAnyType(Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return false;
    }
    auto cmp_inst = inst->CastToCompare();
    if (cmp_inst->GetOperandsType() != DataType::ANY) {
        return false;
    }
    auto input0 = cmp_inst->GetInput(0).GetInst();
    if (input0->GetOpcode() != Opcode::CastValueToAnyType) {
        return false;
    }
    if (AnyBaseTypeToDataType(input0->CastToCastValueToAnyType()->GetAnyType()) != DataType::BOOL) {
        return false;
    }

    auto input1 = cmp_inst->GetInput(1).GetInst();
    if (input1->GetOpcode() == Opcode::CastValueToAnyType) {
        // 2.any  CastValueToAnyType BOOLEAN_TYPE v0 -> (v4)
        // 3.any  CastValueToAnyType BOOLEAN_TYPE v1 -> (v4)
        // 4.     Compare EQ/NE any    v2, v3
        // =======>
        // 4.     Compare EQ/NE bool   v0, v1
        if (input0->CastToCastValueToAnyType()->GetAnyType() != input1->CastToCastValueToAnyType()->GetAnyType()) {
            return false;
        }
        cmp_inst->SetOperandsType(DataType::BOOL);
        cmp_inst->SetInput(0, input0->GetInput(0).GetInst());
        cmp_inst->SetInput(1, input1->GetInput(0).GetInst());
        return true;
    }
    if (!input1->IsConst()) {
        return false;
    }

    // 3.any  CastValueToAnyType BOOLEAN_TYPE v2 -> (v4)
    // 4.     Compare EQ/NE any    v3, DYNAMIC_TRUE/FALSE
    // =======>
    // 4.     Compare EQ/NE bool   v2, 0x1/0x0
    auto if_imm = input1->CastToConstant()->GetRawValue();
    auto runtime = graph->GetRuntime();
    uint64_t new_const;
    if (if_imm == runtime->GetDynamicPrimitiveFalse()) {
        new_const = 0;
    } else if (if_imm == runtime->GetDynamicPrimitiveTrue()) {
        new_const = 1;
    } else {
        // In this case, we are comparing the dynamic boolean type with not boolean constant.
        // So the Compare EQ/NE alwayes false/true.
        // In this case, we can change the Compare to Constant instruction.
        // NOTE! It is assumed that there is only one Boolean type for each dynamic language.
        // Support for multiple Boolean types must be maintained separately.
        auto cc = cmp_inst->GetCc();
        if (cc != CC_EQ && cc != CC_NE) {
            return false;
        }
        cmp_inst->ReplaceUsers(graph->FindOrCreateConstant<uint64_t>(cc == CC_NE ? 1 : 0));
        return true;
    }
    cmp_inst->SetOperandsType(DataType::BOOL);
    cmp_inst->SetInput(0, input0->GetInput(0).GetInst());
    cmp_inst->SetInput(1, graph->FindOrCreateConstant(new_const));
    return true;
}

void Peepholes::VisitIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // 2.     Constant           0x0
    // 3.i64  And                v0, v1
    // 4.     If EQ/NE i64       v3, v2
    // =======>
    // 4.     If TST_EQ/TST_NE   v1, v2
    auto visitor = static_cast<Peepholes *>(v);
    if (visitor->GetGraph()->IsBytecodeOptimizer()) {
        return;
    }
    auto if_imm = inst->CastToIf();
    if (if_imm->GetCc() != CC_EQ && if_imm->GetCc() != CC_NE) {
        return;
    }

    auto lhs = if_imm->GetInput(0).GetInst();
    auto rhs = if_imm->GetInput(1).GetInst();
    Inst *and_input {nullptr};
    if (lhs->GetOpcode() == Opcode::And && IsZeroConstantOrNullPtr(rhs)) {
        and_input = lhs;
    } else if (rhs->GetOpcode() == Opcode::And && IsZeroConstantOrNullPtr(lhs)) {
        and_input = rhs;
    } else {
        return;
    }
    if (!and_input->HasSingleUser()) {
        return;
    }

    if_imm->SetCc(if_imm->GetCc() == CC_EQ ? CC_TST_EQ : CC_TST_NE);
    if_imm->SetInput(0, and_input->GetInput(0).GetInst());
    if_imm->SetInput(1, and_input->GetInput(1).GetInst());
    visitor->SetIsApplied(inst);
}

void Peepholes::VisitCompareAnyType(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<Peepholes *>(v);
    auto input = inst->GetInput(0).GetInst();
    // from
    //  2.any CastValueToAnyType TYPE1 -> (v3)
    //  3.b CompareAnyType TYPE2 -> (...)
    // to
    //  4. Constant (TYPE1 == TYPE2)  ->(...)
    if (input->GetOpcode() == Opcode::CastValueToAnyType) {
        auto inst_type = inst->CastToCompareAnyType()->GetAnyType();
        auto input_type = input->CastToCastValueToAnyType()->GetAnyType();
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto language = graph->GetRuntime()->GetMethodSourceLanguage(graph->GetMethod());
        auto res = IsAnyTypeCanBeSubtypeOf(inst_type, input_type, language);
        if (!res) {
            // We cannot compare types in compile-time
            return;
        }
        auto const_value = res.value();
        auto cnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(const_value);
        inst->ReplaceUsers(cnst);
        visitor->SetIsApplied(inst);
    }
}

static bool IsInputTypeMismatch(Inst *inst, int32_t input_index, Arch arch)
{
    auto input_inst = inst->GetInput(input_index).GetInst();
    auto input_type_size = DataType::GetTypeSize(input_inst->GetType(), arch);
    auto inst_input_size = DataType::GetTypeSize(inst->GetInputType(input_index), arch);
    return (input_type_size > inst_input_size) ||
           (input_type_size == inst_input_size &&
            DataType::IsTypeSigned(input_inst->GetType()) != DataType::IsTypeSigned(inst->GetInputType(input_index)));
}

static bool ApplyForCastJoin(Inst *cast, Inst *input, Inst *orig_inst, Arch arch)
{
    auto input_type_mismatch = IsInputTypeMismatch(cast, 0, arch);
#ifndef NDEBUG
    ASSERT(!input_type_mismatch);
#else
    if (input_type_mismatch) {
        return false;
    }
#endif
    auto input_type = input->GetType();
    auto input_type_size = DataType::GetTypeSize(input_type, arch);
    auto curr_type = cast->GetType();
    auto orig_type = orig_inst->GetType();
    return DataType::GetCommonType(input_type) == DataType::INT64 &&
           DataType::GetCommonType(curr_type) == DataType::INT64 &&
           DataType::GetCommonType(orig_type) == DataType::INT64 &&
           input_type_size > DataType::GetTypeSize(curr_type, arch) &&
           DataType::GetTypeSize(orig_type, arch) > input_type_size;
}

void Peepholes::VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingCast(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    auto prev_type = input->GetType();
    auto curr_type = inst->GetType();
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();
    // case 1:
    // remove redundant cast, when source type is equal with target type
    if (prev_type == curr_type) {
        inst->ReplaceUsers(input);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
    if (!graph->IsBytecodeOptimizer()) {
        if (input->GetOpcode() == Opcode::Cast) {
            auto orig_inst = input->GetInput(0).GetInst();
            auto orig_type = orig_inst->GetType();
            // case 2:
            // remove redundant cast, when cast from source to target and back
            // for bytecode optimizer, this operation may cause mistake for float32-converter pass
            if (curr_type == orig_type &&
                DataType::GetTypeSize(prev_type, arch) > DataType::GetTypeSize(curr_type, arch)) {
                inst->ReplaceUsers(orig_inst);
                static_cast<Peepholes *>(v)->SetIsApplied(inst);
                return;
            }
            // case 4:
            // join two sequent narrowing integer casts, e.g:
            // replace
            // cast i64toi32
            // cast i32toi16
            // with
            // cast i64toi16
            if (ApplyForCastJoin(inst, input, orig_inst, arch)) {
                auto cast = inst->CastToCast();
                cast->SetOperandsType(orig_inst->GetType());
                cast->SetInput(0, orig_inst);
                static_cast<Peepholes *>(v)->SetIsApplied(inst);
                return;
            }
        }
    }
    // case 3:
    // i8.Cast(v1 & 0xff)        = i8.Cast(u8.Cast(v1))   = i8.Cast(v1)
    // i16.Cast(v1 & 0xffff)     = i16.Cast(u16.Cast(v1)) = i16.Cast(v1)
    // i32.Cast(v1 & 0xffffffff) = i32.Cast(u32.Cast(v1)) = i32.Cast(v1)
    if (input->GetOpcode() == Opcode::And && DataType::GetCommonType(curr_type) == DataType::INT64) {
        auto op0 = input->GetInput(0).GetInst();
        auto op1 = input->GetInput(1).GetInst();
        auto type_size = DataType::GetTypeSize(curr_type, arch);
        if (op1->IsConst() && type_size < DOUBLE_WORD_SIZE) {
            auto val = op1->CastToConstant()->GetIntValue();
            auto mask = (1ULL << type_size) - 1;
            if ((val & mask) == mask) {
                inst->SetInput(0, op0);
                inst->CastToCast()->SetOperandsType(op0->GetType());
                static_cast<Peepholes *>(v)->SetIsApplied(inst);
                return;
            }
        }
    }
}

void Peepholes::VisitLenArray(GraphVisitor *v, Inst *inst)
{
    auto graph = static_cast<Peepholes *>(v)->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    // 1. .... ->{v2}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2 ->{v4, v5...}
    // ===>
    // 1. .... ->{v2, v4, v5, ...}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2
    auto input = inst->GetDataFlowInput(inst->GetInput(0).GetInst());
    if (input->GetOpcode() == Opcode::NewArray) {
        auto array_size = input->GetDataFlowInput(input->GetInput(NewArrayInst::INDEX_SIZE).GetInst());
        // We can't restore array_size register if it was defined out of OSR loop
        if (array_size->GetOpcode() != Opcode::Constant && graph->IsOsrMode() && HasOsrEntryBetween(array_size, inst)) {
            return;
        }
        inst->ReplaceUsers(array_size);
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    }
}

void Peepholes::VisitPhi([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // 2.type  Phi v0, v1 -> v4, v5, ...
    // 3.type  Phi v0, v1 -> v5, v6, ...
    // ===>
    // 2.type  Phi v0, v1 -> v4, v5, v6, ...
    // 3.type  Phi v0, v1
    if (!inst->GetUsers().Empty()) {
        auto block = inst->GetBasicBlock();
        auto phi = inst->CastToPhi();
        for (auto other_phi : block->PhiInsts()) {
            if (IsPhiUnionPossible(phi, other_phi->CastToPhi())) {
                other_phi->ReplaceUsers(phi);
                static_cast<Peepholes *>(v)->SetIsApplied(inst);
            }
        }
    }
}

void Peepholes::VisitSqrt([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingSqrt(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
}

void Peepholes::VisitIsInstance(GraphVisitor *v, Inst *inst)
{
    static_cast<Peepholes *>(v)->GetGraph()->RunPass<ObjectTypePropagation>();
    if (ObjectTypeCheckElimination::TryEliminateIsInstance(inst)) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
        return;
    }
}

void Peepholes::VisitCastValueToAnyType(GraphVisitor *v, Inst *inst)
{
    // 1. .... -> v1
    // 2. CastValueToAnyType v1 -> v2
    // 3. CastAnyTypeValue v2 -> v3
    // 4. v3 -> ....
    // ===>
    // 1. .... -> v1
    // 4. v1 -> ....

    if (inst->GetUsers().Empty()) {
        // Peepholes can be run before cleanup phase.
        return;
    }

    auto *cva = inst->CastToCastValueToAnyType();
    Inst *cva_input = cva->GetInput(0).GetInst();
    bool is_applied = false;

    for (User &user : cva->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::CastAnyTypeValue) {
            // It has usefull user, than skip optimization.
            continue;
        }

        if (user.GetInst()->CastToCastAnyTypeValue()->GetAnyType() == cva->GetAnyType()) {
            user.GetInst()->ReplaceUsers(cva_input);
            is_applied = true;
        }
    }

    if (is_applied) {
        static_cast<Peepholes *>(v)->SetIsApplied(inst);
    }
}

template <typename T>
void Peepholes::EliminateInstPrecedingStore(GraphVisitor *v, Inst *inst)
{
    auto graph = static_cast<Peepholes *>(v)->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    auto arch = graph->GetArch();
    auto type_size = DataType::GetTypeSize(inst->GetType(), arch);
    if (DataType::GetCommonType(inst->GetType()) == DataType::INT64 && type_size < DOUBLE_WORD_SIZE) {
        auto store_value_inst = inst->GetInput(T::STORED_INPUT_INDEX).GetInst();
        auto mask = (1ULL << type_size) - 1;
        uint64_t imm;

        switch (store_value_inst->GetOpcode()) {
            case Opcode::And: {
                auto input_inst1 = store_value_inst->GetInput(1).GetInst();
                if (!input_inst1->IsConst()) {
                    return;
                }
                imm = input_inst1->CastToConstant()->GetIntValue();
                break;
            }
            case Opcode::Cast: {
                auto size = DataType::GetTypeSize(store_value_inst->GetType(), arch);
                if (size >= DOUBLE_WORD_SIZE) {
                    return;
                }
                auto input_type_mismatch = IsInputTypeMismatch(store_value_inst, 0, arch);
#ifndef NDEBUG
                ASSERT(!input_type_mismatch);
#else
                if (input_type_mismatch) {
                    return;
                }
#endif
                imm = (1ULL << size) - 1;
                break;
            }
            default:
                return;
        }

        auto input_inst = store_value_inst->GetInput(0).GetInst();
        if (DataType::GetCommonType(input_inst->GetType()) != DataType::INT64) {
            return;
        }

        if ((imm & mask) == mask) {
            inst->ReplaceInput(store_value_inst, input_inst);
            static_cast<Peepholes *>(v)->SetIsApplied(inst);
        }
    }
}

void Peepholes::VisitStore([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Store);
    EliminateInstPrecedingStore<StoreMemInst>(v, inst);
}

void Peepholes::VisitStoreObject([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::StoreObject);
    EliminateInstPrecedingStore<StoreObjectInst>(v, inst);
}

void Peepholes::VisitStoreStatic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::StoreStatic);
    EliminateInstPrecedingStore<StoreStaticInst>(v, inst);
}

// This function check that we can merge two Phi instructions in one basic block.
bool Peepholes::IsPhiUnionPossible(PhiInst *phi1, PhiInst *phi2)
{
    ASSERT(phi1->GetBasicBlock() == phi2->GetBasicBlock() && phi1->GetInputsCount() == phi2->GetInputsCount());
    if (phi1 == phi2 || phi1->GetType() != phi2->GetType()) {
        return false;
    }
    for (auto pred_block : phi1->GetBasicBlock()->GetPredsBlocks()) {
        if (phi1->GetPhiInput(pred_block) != phi2->GetPhiInput(pred_block)) {
            return false;
        }
    }
    return true;
}

// Get power of 2
// if n not power of 2 return -1;
int64_t Peepholes::GetPowerOfTwo(uint64_t n)
{
    int64_t result = -1;
    if (n != 0 && (n & (n - 1)) == 0) {
        for (; n != 0; n >>= 1U) {
            ++result;
        }
    }
    return result;
}

// Create new instruction instead of current inst
Inst *Peepholes::CreateAndInsertInst(Opcode new_opc, Inst *curr_inst, Inst *input0, Inst *input1)
{
    auto bb = curr_inst->GetBasicBlock();
    auto graph = bb->GetGraph();
    auto new_inst = graph->CreateInst(new_opc);
    new_inst->SetType(curr_inst->GetType());
    new_inst->SetPc(curr_inst->GetPc());
    curr_inst->ReplaceUsers(new_inst);
    new_inst->SetInput(0, input0);
    if (input1 != nullptr) {
        new_inst->SetInput(1, input1);
    }
    bb->InsertAfter(new_inst, curr_inst);
    return new_inst;
}

// Try put constant in second input
void Peepholes::TrySwapInputs(Inst *inst)
{
    [[maybe_unused]] auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Add || opc == Opcode::And || opc == Opcode::Or || opc == Opcode::Xor || opc == Opcode::Min ||
           opc == Opcode::Max || opc == Opcode::Mul);
    if (inst->GetInput(0).GetInst()->IsConst()) {
        inst->SwapInputs();
        SetIsApplied(inst);
    }
}

void Peepholes::TrySimplifyShifts(Inst *inst)
{
    auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Shl || opc == Opcode::Shr || opc == Opcode::AShr);
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input1);
        // Zero shift
        // 2. shl/shr/ashr v1, 0 -> {...}
        // 3. Some inst v2
        // ===>
        // 2. shl/shr/ashr v1, 0 -> {}
        // 3. Some inst v1
        if (cnst->IsEqualConst(0)) {
            inst->ReplaceUsers(input0);
            SetIsApplied(inst);
            return;
        }
        // Repeated arithmetic with constants
        // 2. shl/shr/ashr v1, const1 -> {v3, ...}
        // 3. shl/shr/ashr v2, const2 -> {...}
        // ===>
        // 2. shl/shr/ashr v1, const1 -> {...}
        // 3. shl/shr/ashr v1, const2 + const1 -> {...}
        if (opc == input0->GetOpcode() && TryCombineShiftConst(inst, cnst)) {
            SetIsApplied(inst);
            return;
        }
        uint64_t size_mask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        auto cnst_value = cnst->GetIntValue();
        // Shift by a constant greater than the type size
        // 2. shl/shr/ashr v1, big_const -> {...}
        // ===>
        // 2. shl/shr/ashr v1, size_mask & big_const -> {...}
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            size_mask = static_cast<uint32_t>(size_mask);
            cnst_value = static_cast<uint32_t>(cnst_value);
        }
        if (size_mask < cnst_value) {
            ConstantInst *shift = ConstFoldingCreateIntConst(inst, size_mask & cnst_value);
            inst->SetInput(1, shift);
            SetIsApplied(inst);
            return;
        }
    }
}

void Peepholes::TryReplaceDivByShrAndAshr(Inst *inst, uint64_t unsigned_val, Inst *input0)
{
    auto bb = inst->GetBasicBlock();
    auto graph = bb->GetGraph();
    auto signed_val = static_cast<int64_t>(unsigned_val);
    int64_t n = GetPowerOfTwo(signed_val < 0 ? -signed_val : signed_val);
    if (n != -1) {
        auto type_size = DataType::GetTypeSize(inst->GetType(), graph->GetArch());
        auto ashr = graph->CreateInstAShr();
        ashr->SetType(inst->GetType());
        ashr->SetInput(0, input0);
        ashr->SetInput(1, graph->FindOrCreateConstant(type_size - 1));
        bb->InsertAfter(ashr, inst);
        auto shr = graph->CreateInstShr();
        shr->SetType(inst->GetType());
        shr->SetInput(0, ashr);
        shr->SetInput(1, graph->FindOrCreateConstant(type_size - n));
        bb->InsertAfter(shr, ashr);
        auto add = graph->CreateInstAdd();
        add->SetType(inst->GetType());
        add->SetInput(0, shr);
        add->SetInput(1, input0);
        bb->InsertAfter(add, shr);
        Inst *result = graph->CreateInstAShr();
        result->SetType(inst->GetType());
        result->SetInput(0, add);
        result->SetInput(1, graph->FindOrCreateConstant(n));
        bb->InsertAfter(result, add);
        if (signed_val < 0) {
            auto div = result;
            result = graph->CreateInstNeg();
            result->SetType(inst->GetType());
            result->SetInput(0, div);
            bb->InsertAfter(result, div);
        }
        inst->ReplaceUsers(result);
        SetIsApplied(inst);
    }
}

bool Peepholes::TrySimplifyAddSubWithConstInput(Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = input1->CastToConstant();
        if (cnst->IsEqualConstAllTypes(0)) {
            inst->ReplaceUsers(input0);
            SetIsApplied(inst);
            return true;
        }
        if ((input0->GetOpcode() == Opcode::Add || input0->GetOpcode() == Opcode::Sub) &&
            TryCombineAddSubConst(inst, cnst)) {
            SetIsApplied(inst);
            return true;
        }
    } else if (input0->IsConst() && inst->GetOpcode() == Opcode::Sub && !IsFloatType(inst->GetType())) {
        // Fold `sub 0, v1` into `neg v1`.
        auto cnst = input0->CastToConstant();
        if (cnst->IsEqualConstAllTypes(0)) {
            CreateAndInsertInst(Opcode::Neg, inst, input1);
            SetIsApplied(inst);
            return true;
        }
    }
    return false;
}

template <Opcode opc, int idx>
void Peepholes::TrySimplifyAddSub(Inst *inst, Inst *input0, Inst *input1)
{
    if (input0->GetOpcode() == opc && input0->GetInput(1 - idx).GetInst() == input1) {
        auto prev_input = input0->GetInput(idx).GetInst();
        if (inst->GetType() == prev_input->GetType()) {
            inst->ReplaceUsers(prev_input);
            SetIsApplied(inst);
            return;
        }
    }
}

bool Peepholes::TrySimplifyAddSubAdd(Inst *inst, Inst *input0, Inst *input1)
{
    // (a - b) + (b + c) = a + c
    if (input0->GetInput(1).GetInst() == input1->GetInput(0).GetInst()) {
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, input1->GetInput(1).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a - b) + (c + b) = a + c
    if (input0->GetInput(1).GetInst() == input1->GetInput(1).GetInst()) {
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, input1->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

bool Peepholes::TrySimplifyAddSubSub(Inst *inst, Inst *input0, Inst *input1)
{
    // (a - b) + (b - c) = a - c
    if (input0->GetInput(1).GetInst() == input1->GetInput(0).GetInst()) {
        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, input1->GetInput(1).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a - b) + (c - a) = c - b
    if (input0->GetInput(0).GetInst() == input1->GetInput(1).GetInst()) {
        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(0, input1->GetInput(0).GetInst());
        inst->SetInput(1, input0->GetInput(1).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

bool Peepholes::TrySimplifySubAddAdd(Inst *inst, Inst *input0, Inst *input1)
{
    // (a + b) - (a + c) = b - c
    if (input0->GetInput(0).GetInst() == input1->GetInput(0).GetInst()) {
        inst->SetInput(0, input0->GetInput(1).GetInst());
        inst->SetInput(1, input1->GetInput(1).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (c + a) = b - c
    if (input0->GetInput(0).GetInst() == input1->GetInput(1).GetInst()) {
        inst->SetInput(0, input0->GetInput(1).GetInst());
        inst->SetInput(1, input1->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (b + c) = a - c
    if (input0->GetInput(1).GetInst() == input1->GetInput(0).GetInst()) {
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, input1->GetInput(1).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (c + b) = a - c
    if (input0->GetInput(1).GetInst() == input1->GetInput(1).GetInst()) {
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, input1->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

/**
 * The goal is to transform the following sequence:
 *
 * shl v1, v0, const1;
 * shl v2, v0, const2;
 * add/sub v3, v1, v2;
 * add/sub v5, v4, v3;
 *
 * so as to make it ready for the lowering with shifted operands.
 *
 * add v3, v1, v2;
 * add v5, v4, v3;
 * ===>
 * add v6, v4, v1;
 * add v5, v6, v2;
 *
 * add v3, v1, v2;
 * sub v5, v4, v3;
 * ===>
 * sub v6, v4, v1;
 * sub v5, v6, v2;
 *
 * sub v3, v1, v2;
 * add v5, v4, v3;
 * ===>
 * add v6, v4, v1;
 * sub v5, v6, v2;
 *
 * sub v3, v1, v2;
 * sub v5, v4, v3;
 * ===>
 * sub v6, v4, v1;
 * add v5, v6, v2;
 */
bool Peepholes::TryReassociateShlShlAddSub(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::Sub && inst->GetOpcode() != Opcode::Add) {
        return false;
    }
    if (IsFloatType(inst->GetType())) {
        return false;
    }
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->GetOpcode() != Opcode::Sub && input1->GetOpcode() != Opcode::Add) {
        return false;
    }
    // Potentially inst and input1 can have different types (e.g. UINT32 and UINT16).
    if (input1->GetType() != inst->GetType()) {
        return false;
    }
    if (!input1->HasSingleUser()) {
        return false;
    }
    auto shl0 = input1->GetInput(0).GetInst();
    if (shl0->GetOpcode() != Opcode::Shl || !shl0->HasSingleUser()) {
        return false;
    }
    auto shl1 = input1->GetInput(1).GetInst();
    if (shl1->GetOpcode() != Opcode::Shl || !shl1->HasSingleUser() ||
        shl1->GetInput(0).GetInst() != shl0->GetInput(0).GetInst()) {
        return false;
    }
    if (!shl0->GetInput(1).GetInst()->IsConst() || !shl1->GetInput(1).GetInst()->IsConst()) {
        return false;
    }
    auto input0 = inst->GetInput(0).GetInst();
    if (!input0->IsConst() && !input0->IsParameter()) {
        if (!input0->IsDominate(input1)) {
            return false;
        }
    }

    Opcode opInput1 = input1->GetOpcode();
    Opcode opInst = inst->GetOpcode();
    if (opInst == Opcode::Sub && opInput1 == Opcode::Add) {
        // input0 - (shl0 + shl1) -> (input0 - shl0) - shl1
        opInput1 = Opcode::Sub;
    } else if (opInst == Opcode::Add && opInput1 == Opcode::Sub) {
        // input0 + (shl0 - shl1) -> (input0 + shl0) - shl1
        opInput1 = Opcode::Add;
        opInst = Opcode::Sub;
    } else if (opInst == Opcode::Sub && opInput1 == Opcode::Sub) {
        // input0 - (shl0 - shl1) -> (input0 - shl0) + shl1
        opInst = Opcode::Add;
    } else if (opInst != Opcode::Add && opInput1 != Opcode::Add) {
        UNREACHABLE();
    }
    auto newInput0 = CreateAndInsertInst(opInput1, input1, input0, shl0);
    CreateAndInsertInst(opInst, inst, newInput0, shl1);
    return true;
}

void Peepholes::TrySimplifyNeg(Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input0->GetOpcode() == Opcode::Neg && !input1->IsConst()) {
        inst->SetInput(0, input1);
        inst->SetInput(1, input0);
        std::swap(input0, input1);
    }
    if (input1->GetOpcode() == Opcode::Neg) {
        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(1, input1->GetInput(0).GetInst());
        SetIsApplied(inst);
        return;
    }
}

void Peepholes::TryReplaceDivByShift(Inst *inst)
{
    auto bb = inst->GetBasicBlock();
    auto graph = bb->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    ASSERT(input1->IsConst());
    uint64_t unsigned_val = input1->CastToConstant()->GetIntValue();
    if (!DataType::IsTypeSigned(inst->GetType())) {
        // case 3:
        // 0.unsigned Parameter
        // 1.i64 Const 2^n -> {v2}
        // 2.un-signed DIV v0, v1 -> {v3}
        // 3.unsigned INST v2
        // ===>
        // 0.unsigned Parameter
        // 1.i64 Const n -> {v2}
        // 2.un-signed SHR v0, v1 -> {v3}
        // 3.unsigned INST v2
        int64_t n = GetPowerOfTwo(unsigned_val);
        if (n != -1) {
            auto power = graph->FindOrCreateConstant(n);
            CreateAndInsertInst(Opcode::Shr, inst, input0, power);
            SetIsApplied(inst);
        }
    } else {
        // case 3:
        // 0.signed Parameter
        // 1.i64 Const 2^n -> {v2}
        // 2.signed DIV v0, v1 -> {v3}
        // 3.signed INST v2
        // ===>
        // 0.signed Parameter
        // 1.i64 Const n -> {v2}
        // 2.un-signed SHR v0, v1 -> {v3}
        // 3.unsigned INST v2
        TryReplaceDivByShrAndAshr(inst, unsigned_val, input0);
    }
}

bool Peepholes::TrySimplifyCompareCaseInputInv(Inst *inst, Inst *input)
{
    for (auto &user : inst->GetUsers()) {
        auto opcode = user.GetInst()->GetOpcode();
        if (opcode != Opcode::If && opcode != Opcode::IfImm) {
            return false;
        }
    }
    for (auto &user : inst->GetUsers()) {
        auto opcode = user.GetInst()->GetOpcode();
        if (opcode == Opcode::If) {
            user.GetInst()->CastToIf()->InverseConditionCode();
        } else if (opcode == Opcode::IfImm) {
            user.GetInst()->CastToIfImm()->InverseConditionCode();
        } else {
            UNREACHABLE();
        }
    }
    inst->ReplaceUsers(input);
    return true;
}

// Eliminates double comparison if possible
// 4.i64  Constant                   0x0 -> (v6)
// 5.b    ### Some abstract expression that return boolean ###
// 6.b    Compare EQ i32             v5, v4 -> (v7)
// 7.     IfImm NE b                 v6, 0x0
// =======>
// 4.i64  Constant                   0x0 -> (v6)
// 5.b    ### Some abstract expression that return boolean ###
// 6.b    Compare EQ i32             v5, v4
// 7.     IfImm EQ b                 v5, 0x0
bool Peepholes::TrySimplifyCompareWithBoolInput(Inst *inst)
{
    auto compare = inst->CastToCompare();
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *constInput = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &constInput, &swap)) {
        return false;
    }
    if (input->GetType() != DataType::BOOL) {
        return false;
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();
    InputCode code = GetInputCode(constInput, cc);
    if (code == INPUT_TRUE || code == INPUT_FALSE) {
        compare->ReplaceUsers(ConstFoldingCreateIntConst(compare, code == INPUT_TRUE ? 1 : 0));
        return true;
    }
    if (code == INPUT_ORIG) {
        compare->ReplaceUsers(input);
        return true;
    }
    if (code == INPUT_INV) {
        return TrySimplifyCompareCaseInputInv(compare, input);
    }
    UNREACHABLE();
    return false;
}

// 6.i32  Cmp        v5, v1
// 7.b    Compare    v6, 0
// 9.     IfImm NE b v7, 0x0
// =======>
// 6.i32  Cmp        v5, v1
// 7.b    Compare    v5, v1
// 9.     IfImm NE b v7, 0x0
bool Peepholes::TrySimplifyCmpCompareWithZero(Inst *inst)
{
    auto compare = inst->CastToCompare();
    if (compare->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return false;
    }
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *constInput = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &constInput, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::Cmp || !input->HasSingleUser()) {
        return false;
    }
    if (!constInput->IsEqualConstAllTypes(0)) {
        return false;
    }
    auto cmpOpType = input->CastToCmp()->GetOperandsType();
    if (IsFloatType(cmpOpType)) {
        return false;
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();
    if (!IsTypeSigned(cmpOpType)) {
        ASSERT(cc == ConditionCode::CC_EQ || cc == ConditionCode::CC_NE || IsSignedConditionCode(cc));
        // If Cmp operands are unsigned then Compare.CC must be converted to unsigned.
        cc = InverseSignednessConditionCode(cc);
    }
    compare->SetInput(0, input->GetInput(0).GetInst());
    compare->SetInput(1, input->GetInput(1).GetInst());
    compare->SetOperandsType(cmpOpType);
    compare->SetCc(cc);
    return true;
}

bool Peepholes::TrySimplifyTestEqualInputs(Inst *inst)
{
    auto cmp_inst = inst->CastToCompare();
    if (cmp_inst->GetCc() != ConditionCode::CC_TST_EQ && cmp_inst->GetCc() != ConditionCode::CC_TST_NE) {
        return false;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input0 != input1) {
        return false;
    }
    if (cmp_inst->GetCc() == ConditionCode::CC_TST_EQ) {
        cmp_inst->SetCc(ConditionCode::CC_EQ);
    } else {
        cmp_inst->SetCc(ConditionCode::CC_NE);
    }
    cmp_inst->SetInput(1, ConstFoldingCreateIntConst(input1, 0));
    return true;
}

bool Peepholes::TrySimplifyCompareAndZero(Inst *inst)
{
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return false;
    }
    auto cmp_inst = inst->CastToCompare();
    auto cc = cmp_inst->GetCc();
    if (cc != CC_EQ && cc != CC_NE) {
        return false;
    }
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *const_input = nullptr;
    if (!GetInputsOfCompareWithConst(cmp_inst, &input, &const_input, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::And || !input->HasSingleUser() || !const_input->IsEqualConstAllTypes(0)) {
        return false;
    }
    // 2.i32 And                  v0, v1
    // 3.i32 Constant             0x0
    // 4.b   Compare CC_EQ/CC_NE  v2, v3
    // =======>
    // 4.b   Compare CC_TST_EQ/CC_TST_NE  v0, v1
    cmp_inst->SetCc(cc == CC_EQ ? CC_TST_EQ : CC_TST_NE);
    cmp_inst->SetInput(0, input->GetInput(0).GetInst());
    cmp_inst->SetInput(1, input->GetInput(1).GetInst());
    return true;
}

bool Peepholes::TrySimplifyCompareLenArrayWithZero(Inst *inst)
{
    auto compare = inst->CastToCompare();
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *constInput = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &constInput, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::LenArray || !constInput->IsEqualConstAllTypes(0)) {
        return false;
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();

    if (cc == CC_GE || cc == CC_LT) {
        compare->ReplaceUsers(ConstFoldingCreateIntConst(compare, cc == CC_GE ? 1 : 0));
        return true;
    }
    return false;
}

// Try to combine constants when arithmetic operations with constants are repeated
template <typename T>
bool Peepholes::TryCombineConst(Inst *inst, ConstantInst *cnst1, T combine)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto previnput1 = input0->GetInput(1).GetInst();
    if (previnput1->IsConst() && inst->GetType() == input0->GetType()) {
        auto cnst2 = static_cast<ConstantInst *>(previnput1);
        auto graph = inst->GetBasicBlock()->GetGraph();
        ConstantInst *new_cnst = nullptr;
        switch (DataType::GetCommonType(cnst1->GetType())) {
            case DataType::INT64:
                new_cnst = ConstFoldingCreateIntConst(inst, combine(cnst1->GetIntValue(), cnst2->GetIntValue()));
                break;
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(combine(cnst1->GetFloatValue(), cnst2->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(combine(cnst1->GetDoubleValue(), cnst2->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, new_cnst);
        return true;
    }
    return false;
}

bool Peepholes::TryCombineAddSubConst(Inst *inst, ConstantInst *cnst1)
{
    auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Add || opc == Opcode::Sub);
    auto input0 = inst->GetInput(0).GetInst();
    auto combine = [&opc, &input0](auto x, auto y) { return opc == input0->GetOpcode() ? x + y : x - y; };
    return TryCombineConst(inst, cnst1, combine);
}

bool Peepholes::TryCombineShiftConst(Inst *inst, ConstantInst *cnst1)
{
    [[maybe_unused]] auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Shl || opc == Opcode::Shr || opc == Opcode::AShr);
    auto combine = [](auto x, auto y) { return x + y; };
    return TryCombineConst(inst, cnst1, combine);
}

bool Peepholes::TryCombineMulConst(Inst *inst, ConstantInst *cnst1)
{
    ASSERT(inst->GetOpcode() == Opcode::Mul);
    auto combine = [](auto x, auto y) { return x * y; };
    return TryCombineConst(inst, cnst1, combine);
}

bool Peepholes::GetInputsOfCompareWithConst(const Inst *inst, Inst **input, ConstantInst **constInput,
                                            bool *inputsSwapped)
{
    if (inst->GetOpcode() == Opcode::Compare || inst->GetOpcode() == Opcode::Cmp) {
        if (inst->GetInput(1).GetInst()->IsConst()) {
            *input = inst->GetInput(0).GetInst();
            *constInput = inst->GetInput(1).GetInst()->CastToConstant();
            *inputsSwapped = false;
            return true;
        }
        if (inst->GetInput(0).GetInst()->IsConst()) {
            *input = inst->GetInput(1).GetInst();
            *constInput = inst->GetInput(0).GetInst()->CastToConstant();
            *inputsSwapped = true;
            return true;
        }
    }
    return false;
}
}  // namespace panda::compiler
