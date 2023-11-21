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

#include "compiler_logger.h"
#include "peepholes.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/optimizations/const_folding.h"
#include "optimizer/optimizations/object_type_check_elimination.h"

namespace panda::compiler {

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
    if (OPTIONS.IsCompilerSafePointsRequireRegMap()) {
        return;
    }
    if (inst->CastToSafePoint()->RemoveNumericInputs()) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingNeg(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        if (SkipThisPeepholeInOSR(inst, input->GetInput(0).GetInst())) {
            return;
        }
        inst->ReplaceUsers(input->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        if (SkipThisPeepholeInOSR(inst, input->GetInput(0).GetInst()) ||
            SkipThisPeepholeInOSR(inst, input->GetInput(1).GetInst())) {
            return;
        }
        CreateAndInsertInst(Opcode::Sub, inst, input->GetInput(1).GetInst(), input->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitAbs([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (!DataType::IsTypeSigned(inst->GetType())) {
        if (SkipThisPeepholeInOSR(inst, inst->GetInput(0).GetInst())) {
            return;
        }
        inst->ReplaceUsers(inst->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    if (ConstFoldingAbs(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingNot(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitAddFinalize([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    auto visitor = static_cast<Peepholes *>(v);
    // Some of the previous transformations might change the opcode of inst.
    if (inst->GetOpcode() == Opcode::Add) {
        if (visitor->TryReassociateShlShlAddSub(inst)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }

        if (input0->GetOpcode() == Opcode::Add && input1->GetOpcode() == Opcode::Sub) {
            std::swap(input0, input1);
        }

        if (input0->GetOpcode() == Opcode::Sub && input1->GetOpcode() == Opcode::Add) {
            if (visitor->TrySimplifyAddSubAdd(inst, input0, input1)) {
                return;
            }
        }

        if (input0->GetOpcode() == Opcode::Sub && input1->GetOpcode() == Opcode::Sub) {
            if (visitor->TrySimplifyAddSubSub(inst, input0, input1)) {
                return;
            }
        }
    }
}

/**
 * Case 1: Swap inputs if the first is a constant
 * 2. add const, v1 -> {...}
 * ===>
 * 2. add v1, const -> {...}
 *
 * Case 2: Addition with zero
 * 1. Some inst -> v2
 * 2. add v1, 0 -> {...}
 * ===>
 * 1. Some inst ->{...}
 * 2. add v1, 0 -> {}
 *
 * Case 3: Repeated arithmetic with constants
 * 2. add/sub v1, const1 -> {v3, ...}
 * 3. add v2, const2 -> {...}
 * ===>
 * 2. add/sub v1, const1 -> {...}
 * 3. add v1, const2 +/- const1 ->{...}
 *
 * Case 4: Addition with Neg
 * 3. neg v1 -> {5}
 * 4. neg v2 -> {5}
 * 5. add v3, v4 -> {...}
 * ===>
 * 3. neg v1 -> {}
 * 4. neg v2 -> {}
 * 5. add v1, v2 -> {4}
 * 6. neg v5 -> {users v5}
 *
 * Case 5:
 * 3. neg v2 -> {4, ...}
 * 4. add v1, v3 -> {...}
 * ===>
 * 3. neg v2 -> {...}
 * 4. sub v1, v2 -> {...}
 *
 * Case 6:
 * Adding and subtracting the same value
 * 3. sub v2, v1 -> {4, ...}
 * 4. add v3, v1 -> {...}
 * ===>
 * 3. sub v2, v1 -> {4, ...}
 * 4. add v3, v1 -> {}
 * v2 -> {users v4}
 *
 * Case 7:
 * Replacing Negation pattern (neg + add) with Compare
 * 1.i64 Constant 0x1 -> {4, ...}
 * 2.b   ... -> {3}
 * 3.i32 Neg v2 -> {4, ...}
 * 4.i32 Add v3, v1 -> {...}
 * ===>
 * 1.i64 Constant 0x1 -> {...}
 * 5.i64 Constant 0x0 -> {v6, ...}
 * 2.b   ... -> {v3, v6}
 * 3.i32 Neg v2 -> {...}
 * 4.i32 Add v3, v1
 * 6.b   Compare EQ b v2, v5 -> {USERS v4}
 *
 * Case 8:
 * Delete negation of Compare
 * 1.i64 Constant 0x1 -> {v4}
 * 2.b   Compare GT ... -> {v3}
 * 3.i32 Neg v2 -> {v4}
 * 4.i32 Add v3, v1 -> {...}
 * ===>
 * 1.i64 Constant 0x1 -> {v4}
 * 2.b   Compare LE ... -> {v3, USERS v4}
 * 3.i32 Neg v2 -> {v4}
 * 4.i32 Add v3, v1
 *
 * Case 9
 * One of the inputs of Compare is Negation pattern
 * 1.i64 Constant 0x1 -> {v4, ...}
 * 2.b   ... -> {v3}
 * 3.i32 Neg v2 -> {v4, ...}
 * 4.i32 Add v3, v1 -> {v5, ...}
 * 5.b   Compare EQ/NE b v4, ...
 * =======>
 * 1.i64 Constant 0x1 -> {v4, ...}
 * 2.b   ... -> {v3, v5}
 * 3.i32 Neg v2 -> {v4, ...}
 * 4.i32 Add v3, v1 -> {...}
 * 5.b   Compare NE/EQ b v2, ...
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
            if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst()) ||
                SkipThisPeepholeInOSR(inst, input1->GetInput(0).GetInst())) {
                return;
            }
            inst->SetInput(0, input0->GetInput(0).GetInst());
            inst->SetInput(1, input1->GetInput(0).GetInst());
            CreateAndInsertInst(Opcode::Neg, inst, inst);
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    } else {
        // Case 7, 8, 9
        if (visitor->TrySimplifyNegationPattern(inst)) {
            return;
        }
        // Case 5
        visitor->TrySimplifyNeg(inst);
    }
    // Case 6
    visitor->TrySimplifyAddSub<Opcode::Sub, 0>(inst, input0, input1);
    visitor->TrySimplifyAddSub<Opcode::Sub, 0>(inst, input1, input0);

    VisitAddFinalize(v, inst, input0, input1);
}

void Peepholes::VisitSubFinalize([[maybe_unused]] GraphVisitor *v, Inst *inst, Inst *input0, Inst *input1)
{
    auto visitor = static_cast<Peepholes *>(v);
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
        auto new_input1 = input0->GetInput(0).GetInst();
        auto new_input0 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return;
        }
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    if (input1->GetOpcode() == Opcode::Neg) {
        auto new_input = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input)) {
            return;
        }
        inst->SetInput(1, new_input);
        inst->SetOpcode(Opcode::Add);
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }
    // Case 4
    visitor->TrySimplifyAddSub<Opcode::Add, 0>(inst, input0, input1);
    visitor->TrySimplifyAddSub<Opcode::Add, 1>(inst, input0, input1);
    // Case 5
    if (input1->GetOpcode() == Opcode::Sub && input1->GetInput(0) == input0) {
        auto prev_input = input1->GetInput(1).GetInst();
        if (inst->GetType() == prev_input->GetType()) {
            if (SkipThisPeepholeInOSR(inst, prev_input)) {
                return;
            }
            inst->ReplaceUsers(prev_input);
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    }
    VisitSubFinalize(v, inst, input0, input1);
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
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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

        // "inst"(mul) and **INST WITHOUT NAME**(neg) are one by one, so we don't should check SaveStateOSR between them
        CreateAndInsertInst(Opcode::Neg, inst, input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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

        // "inst"(mul) and **INST WITHOUT NAME**(add) are one by one, so we don't should check SaveStateOSR between them
        CreateAndInsertInst(Opcode::Add, inst, input0, input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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

            // "inst"(mul) and **INST WITHOUT NAME**(add) are one by one, so we don't should
            // check SaveStateOSR between them
            ConstantInst *power = ConstFoldingCreateIntConst(inst, static_cast<uint64_t>(n));
            CreateAndInsertInst(Opcode::Shl, inst, input0, power);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        bool osr_blocked_peephole = false;
        if (input0->GetOpcode() == Opcode::Mul && TryCombineMulConst(inst, cnst, &osr_blocked_peephole)) {
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
        if (osr_blocked_peephole) {
            return;
        }
    }
    VisitMulOneConst(v, inst, input0, input1);
}

void Peepholes::VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingDiv(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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

            // "inst"(div) and **INST WITHOUT NAME**(neg) are one by one, we should check SaveStateOSR between
            // **INST WITHOUT NAME**(neg) and "input0", but may check only between "inst"(div) and "input0"
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            CreateAndInsertInst(Opcode::Neg, inst, input0);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        } else if (DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
            static_cast<Peepholes *>(v)->TryReplaceDivByShift(inst);
        }
    }
}

void Peepholes::VisitMin([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingMin(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingShl(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    static_cast<Peepholes *>(v)->TrySimplifyShifts(inst);
}

void Peepholes::VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingShr(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
    if (op1->GetOpcode() == Opcode::Shl && op2->IsConst() && op1->GetInput(1) == op2) {
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
        // "inst"(shr) and **INST WITHOUT NAME**(and) one by one, so we may check SaveStateOSR only between
        // "inst"(shr) and "op1->GetInput(0).GetInst()"
        if (SkipThisPeepholeInOSR(op1->GetInput(0).GetInst(), inst)) {
            return;
        }
        CreateAndInsertInst(Opcode::And, inst, op1->GetInput(0).GetInst(), new_cnst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingAShr(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
    if (op1->GetOpcode() == Opcode::Shl && op2->IsConst() && op1->GetInput(1) == op2) {
        ASSERT(inst->GetType() == op1->GetType());
        const uint64_t offset_in_t8 = 24;
        const uint64_t offset_in_t16 = 16;
        const uint64_t offset_in_t32 = 8;
        auto offset = op2->CastToConstant()->GetIntValue();
        if (offset == offset_in_t16 || offset == offset_in_t8 || offset == offset_in_t32) {
            // "inst"(shr) and "cast"(cast) one by one, so we may check SaveStateOSR only between "inst"(shr)
            // and "op1->GetInput(0).GetInst()"(some inst)
            if (SkipThisPeepholeInOSR(op1->GetInput(0).GetInst(), inst)) {
                return;
            }
            auto cast = CreateAndInsertInst(Opcode::Cast, inst, op1->GetInput(0).GetInst());
            cast->SetType((offset == offset_in_t8)    ? DataType::INT8
                          : (offset == offset_in_t16) ? DataType::INT16
                                                      : DataType::INT32);
            cast->CastToCast()->SetOperandsType(op1->GetInput(0).GetInst()->GetType());
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (input0 == input1 && input0->GetType() == inst->GetType()) {
        // case 3:
        // 1.i64 AND v5, v5
        // 2.i64 INS which use v1
        // ===>
        // 1.i64 AND v5, v5
        // 2.i64 INS which use v5
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        // "inst"(and), "or_inst"(or) and **INST WITHOUT NAME**(not) one by one, so we may check SaveStateOSR only
        // between "inst"(and) and inputs: "not_input0"(some inst), "not_input0"(some inst)
        // and "op1->GetInput(0).GetInst()"
        if (SkipThisPeepholeInOSR(inst, not_input0) || SkipThisPeepholeInOSR(inst, not_input1)) {
            return;
        }
        auto or_inst = CreateAndInsertInst(Opcode::Or, inst, not_input0, not_input1);
        CreateAndInsertInst(Opcode::Not, or_inst, or_inst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (ApplyForCastU16(v, inst, input0, input1)) {
        // case 5: IR for cast i64 to u16 is
        // 2.i32 cast i64 to i32
        // 3.i32 and v2, 0xFFFF
        // replace it with cast i64tou16
        auto val = input1->CastToConstant()->GetIntValue();
        constexpr auto MASK = (1U << HALF_SIZE) - 1;
        if (val == MASK) {
            // "inst"(and) and "cast"(cast) one by one, so we may check SaveStateOSR only between
            // "inst"(shr) and "input0->GetInput(0).GetInst()"
            if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) {
                return;
            }
            auto cast = CreateAndInsertInst(Opcode::Cast, inst, input0->GetInput(0).GetInst());
            cast->SetType(DataType::UINT16);
            cast->CastToCast()->SetOperandsType(input0->GetInput(0).GetInst()->GetType());
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        }
    }
}

void Peepholes::VisitOr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingOr(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    } else if (input0 == input1 && input0->GetType() == inst->GetType()) {
        // case 3:
        // 1.i64 OR v5, v5
        // 2.i64 INS which use v1
        // ===>
        // 1.i64 OR v5, v5
        // 2.i64 INS which use v5
        if (SkipThisPeepholeInOSR(inst, input0)) {
            return;
        }
        inst->ReplaceUsers(input0);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
        // "inst"(or), "and_inst"(and) and **INST WITHOUT NAME**(not) one by one, so we may check SaveStateOSR only
        // between "inst"(or) and inputs: "not_input0"(some inst), "not_input0"(some inst)
        // and "op1->GetInput(0).GetInst()"
        if (SkipThisPeepholeInOSR(inst, not_input0) || SkipThisPeepholeInOSR(inst, not_input1)) {
            return;
        }
        auto and_inst = CreateAndInsertInst(Opcode::And, inst, not_input0, not_input1);
        CreateAndInsertInst(Opcode::Not, and_inst, and_inst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitXor([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingXor(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    auto visitor = static_cast<Peepholes *>(v);
    // Swap inputs if the first is a constant
    // 2. Xor const, v1 -> {...}
    // ===>
    // 2. Xor v1, const -> {...}
    static_cast<Peepholes *>(v)->TrySwapInputs(inst);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        uint64_t val = input1->CastToConstant()->GetIntValue();
        if (static_cast<int64_t>(val) == -1) {
            // Replace xor with not:
            // 0.i64 Const -1
            // 1. ...
            // 2.i64 XOR v0, v1
            // ===>
            // 3.i64 NOT v1
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            CreateAndInsertInst(Opcode::Not, inst, input0);
            PEEPHOLE_IS_APPLIED(visitor, inst);
        } else if (input1->CastToConstant()->GetIntValue() == 0) {
            // Result A xor 0 equal A:
            // 0.i64 Const 0
            // 1. ...
            // 2.i64 XOR v0, v1 -> v3
            // 3. INS v2
            // ===>
            // 0.i64 Const 0
            // 1. ...
            // 2.i64 XOR v0, v1
            // 3. INS v1
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(visitor, inst);
        } else if (val == 1 && input0->GetType() == DataType::BOOL) {
            // Replacing Xor with Compare for more case optimizations
            // If this compare is not optimized during pipeline, we will revert it back to xor in Lowering
            // 1.i64 Const 1
            // 2.b   ...
            // 3.i32 Xor v1, v2
            // ===>
            // 1.i64 Const 0
            // 2.b   ...
            // 3.b   Compare EQ b v2, v1
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            CreateCompareInsteadOfXorAdd(inst);
            PEEPHOLE_IS_APPLIED(visitor, inst);
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
    // Try to replace Compare with any type to bool type
    if (visitor->TrySimplifyCompareAnyType(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }

    /* skip preheader Compare processing until unrolling is done */
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsBytecodeOptimizer() && OPTIONS.IsCompilerDeferPreheaderTransform() && !graph->IsUnrollComplete()) {
        auto bb = inst->GetBasicBlock();
        if (bb->IsLoopPreHeader() && inst->HasSingleUser() && inst->GetFirstUser()->GetInst() == bb->GetLastInst() &&
            bb->GetLastInst()->GetOpcode() == Opcode::IfImm) {
            return;
        }
    }

    if (ConstFoldingCompare(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
        return;
    }

    bool osr_blocked_peephole = false;
    if (visitor->TrySimplifyCompareWithBoolInput(inst, &osr_blocked_peephole)) {
        if (osr_blocked_peephole) {
            return;
        }
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyCmpCompareWithZero(inst, &osr_blocked_peephole)) {
        if (osr_blocked_peephole) {
            return;
        }
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyCompareAndZero(inst, &osr_blocked_peephole)) {
        if (osr_blocked_peephole) {
            return;
        }
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (TrySimplifyCompareLenArrayWithZero(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    } else if (visitor->TrySimplifyTestEqualInputs(inst)) {
        PEEPHOLE_IS_APPLIED(visitor, inst);
    }
}

bool Peepholes::TrySimplifyCompareAnyTypeCase1(Inst *inst, Inst *input0, Inst *input1)
{
    auto cmp_inst = inst->CastToCompare();

    // 2.any  CastValueToAnyType BOOLEAN_TYPE v0 -> (v4)
    // 3.any  CastValueToAnyType BOOLEAN_TYPE v1 -> (v4)
    // 4.     Compare EQ/NE any    v2, v3
    // =======>
    // 4.     Compare EQ/NE bool   v0, v1
    if (input0->CastToCastValueToAnyType()->GetAnyType() != input1->CastToCastValueToAnyType()->GetAnyType()) {
        return false;
    }
    if (SkipThisPeepholeInOSR(cmp_inst, input0->GetInput(0).GetInst()) ||
        SkipThisPeepholeInOSR(cmp_inst, input1->GetInput(0).GetInst())) {
        return false;
    }
    cmp_inst->SetOperandsType(DataType::BOOL);
    cmp_inst->SetInput(0, input0->GetInput(0).GetInst());
    cmp_inst->SetInput(1, input1->GetInput(0).GetInst());
    return true;
}

bool Peepholes::TrySimplifyCompareAnyTypeCase2(Inst *inst, Inst *input0, Inst *input1)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto cmp_inst = inst->CastToCompare();
    auto cc = cmp_inst->GetCc();
    auto if_imm = input1->CastToConstant()->GetRawValue();
    auto runtime = graph->GetRuntime();
    uint64_t new_const;

    // 3.any  CastValueToAnyType BOOLEAN_TYPE v2 -> (v4)
    // 4.     Compare EQ/NE any    v3, DYNAMIC_TRUE/FALSE
    // =======>
    // 4.     Compare EQ/NE bool   v2, 0x1/0x0
    if (SkipThisPeepholeInOSR(cmp_inst, input0->GetInput(0).GetInst())) {
        return false;
    }
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
        if (cc != CC_EQ && cc != CC_NE) {
            return false;
        }
        // We create constant, so we don't need to check SaveStateOSR between insts
        cmp_inst->ReplaceUsers(graph->FindOrCreateConstant<uint64_t>(cc == CC_NE ? 1 : 0));
        return true;
    }
    cmp_inst->SetOperandsType(DataType::BOOL);
    cmp_inst->SetInput(0, input0->GetInput(0).GetInst());
    cmp_inst->SetInput(1, graph->FindOrCreateConstant(new_const));
    return true;
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
    auto input1 = cmp_inst->GetInput(1).GetInst();
    auto cc = cmp_inst->GetCc();
    if (input0 == input1 && (cc == CC_EQ || cc == CC_NE)) {
        // We create constant, so we don't need to check SaveStateOSR between insts
        cmp_inst->ReplaceUsers(graph->FindOrCreateConstant<uint64_t>(cc == CC_EQ ? 1 : 0));
        return true;
    }

    if (input0->GetOpcode() != Opcode::CastValueToAnyType) {
        return false;
    }
    if (AnyBaseTypeToDataType(input0->CastToCastValueToAnyType()->GetAnyType()) != DataType::BOOL) {
        return false;
    }

    if (input1->GetOpcode() == Opcode::CastValueToAnyType) {
        return TrySimplifyCompareAnyTypeCase1(inst, input0, input1);
    }
    if (!input1->IsConst()) {
        return false;
    }

    return TrySimplifyCompareAnyTypeCase2(inst, input0, input1);
}

// This VisitIf is using only for compile IRTOC
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

    auto new_input0 = and_input->GetInput(0).GetInst();
    auto new_input1 = and_input->GetInput(1).GetInst();
    if (SkipThisPeepholeInOSR(if_imm, new_input0) || SkipThisPeepholeInOSR(if_imm, new_input1)) {
        return;
    }
    if_imm->SetCc(if_imm->GetCc() == CC_EQ ? CC_TST_EQ : CC_TST_NE);
    if_imm->SetInput(0, new_input0);
    if_imm->SetInput(1, new_input1);
    PEEPHOLE_IS_APPLIED(visitor, inst);
}

static bool TryReplaceCompareAnyType(Inst *inst, Inst *dominate_inst)
{
    auto inst_type = inst->CastToCompareAnyType()->GetAnyType();
    auto dominate_type = dominate_inst->GetAnyType();
    profiling::AnyInputType dominate_allowed_type {};
    if (dominate_inst->GetOpcode() == Opcode::AnyTypeCheck) {
        dominate_allowed_type = dominate_inst->CastToAnyTypeCheck()->GetAllowedInputType();
    } else {
        ASSERT(dominate_inst->GetOpcode() == Opcode::CastValueToAnyType);
    }
    ASSERT(inst->CastToCompareAnyType()->GetAllowedInputType() == profiling::AnyInputType::DEFAULT);
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto language = graph->GetRuntime()->GetMethodSourceLanguage(graph->GetMethod());
    auto res = IsAnyTypeCanBeSubtypeOf(language, inst_type, dominate_type, profiling::AnyInputType::DEFAULT,
                                       dominate_allowed_type);
    if (!res) {
        // We cannot compare types in compile-time
        return false;
    }
    auto const_value = res.value();
    auto cnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(const_value);
    // We replace constant, so we don't need to check SaveStateOSR between insts
    inst->ReplaceUsers(cnst);
    return true;
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
    if (input->GetOpcode() == Opcode::CastValueToAnyType || input->GetOpcode() == Opcode::AnyTypeCheck) {
        if (TryReplaceCompareAnyType(inst, input)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
    }
    // from
    //  2.any AnyTypeCheck v1 TYPE1 -> (...)
    //  3.b CompareAnyType v1 TYPE2 -> (...)
    // to
    //  4. Constant (TYPE1 == TYPE2)  ->(...)
    for (auto &user : input->GetUsers()) {
        auto user_inst = user.GetInst();
        if (user_inst != inst && user_inst->GetOpcode() == Opcode::AnyTypeCheck && user_inst->IsDominate(inst) &&
            TryReplaceCompareAnyType(inst, user_inst)) {
            PEEPHOLE_IS_APPLIED(visitor, inst);
            return;
        }
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

static inline bool IsCastAllowedInBytecode(const Inst *inst)
{
    auto type = inst->GetType();
    switch (type) {
        case DataType::Type::INT32:
        case DataType::Type::INT64:
        case DataType::Type::FLOAT32:
        case DataType::Type::FLOAT64:
            return true;
        default:
            return false;
    }
}

void Peepholes::VisitCastCase1([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // case 1:
    // remove redundant cast, when source type is equal with target type
    auto input = inst->GetInput(0).GetInst();
    if (SkipThisPeepholeInOSR(inst, input)) {
        return;
    }
    inst->ReplaceUsers(input);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitCastCase2([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // case 2:
    // remove redundant cast, when cast from source to target and back
    // for bytecode optimizer, this operation may cause mistake for float32-converter pass
    auto input = inst->GetInput(0).GetInst();
    auto prev_type = input->GetType();
    auto curr_type = inst->GetType();
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();
    auto orig_inst = input->GetInput(0).GetInst();
    auto orig_type = orig_inst->GetType();
    if (curr_type == orig_type && DataType::GetTypeSize(prev_type, arch) > DataType::GetTypeSize(curr_type, arch)) {
        if (SkipThisPeepholeInOSR(inst, orig_inst)) {
            return;
        }
        inst->ReplaceUsers(orig_inst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    // case 2.1:
    // join two sequent narrowing integer casts, e.g:
    // replace
    // cast i64toi32
    // cast i32toi16
    // with
    // cast i64toi16
    if (ApplyForCastJoin(inst, input, orig_inst, arch)) {
        auto cast = inst->CastToCast();
        if (SkipThisPeepholeInOSR(cast, orig_inst)) {
            return;
        }
        cast->SetOperandsType(orig_inst->GetType());
        cast->SetInput(0, orig_inst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitCastCase3([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto input = inst->GetInput(0).GetInst();
    auto curr_type = inst->GetType();
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();

    // case 3:
    // i8.Cast(v1 & 0xff)        = i8.Cast(u8.Cast(v1))   = i8.Cast(v1)
    // i16.Cast(v1 & 0xffff)     = i16.Cast(u16.Cast(v1)) = i16.Cast(v1)
    // i32.Cast(v1 & 0xffffffff) = i32.Cast(u32.Cast(v1)) = i32.Cast(v1)
    auto op0 = input->GetInput(0).GetInst();
    if (graph->IsBytecodeOptimizer() && !IsCastAllowedInBytecode(op0)) {
        return;
    }
    auto op1 = input->GetInput(1).GetInst();
    auto type_size = DataType::GetTypeSize(curr_type, arch);
    if (op1->IsConst() && type_size < DOUBLE_WORD_SIZE) {
        auto val = op1->CastToConstant()->GetIntValue();
        auto mask = (1ULL << type_size) - 1;
        if ((val & mask) == mask) {
            if (SkipThisPeepholeInOSR(inst, op0)) {
                return;
            }
            inst->SetInput(0, op0);
            inst->CastToCast()->SetOperandsType(op0->GetType());
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            return;
        }
    }
}

void Peepholes::VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingCast(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    auto prev_type = input->GetType();
    auto curr_type = inst->GetType();
    auto graph = inst->GetBasicBlock()->GetGraph();

    if (prev_type == curr_type) {
        VisitCastCase1(v, inst);
        return;
    }

    if (!graph->IsBytecodeOptimizer() && input->GetOpcode() == Opcode::Cast) {
        VisitCastCase2(v, inst);
        return;
    }

    if (input->GetOpcode() == Opcode::And && DataType::GetCommonType(curr_type) == DataType::INT64) {
        VisitCastCase3(v, inst);
        return;
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
        if (SkipThisPeepholeInOSR(inst, array_size)) {
            return;
        }
        inst->ReplaceUsers(array_size);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
    if (static_cast<Peepholes *>(v)->OptimizeLenArrayForMultiArray(inst, input, 0)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
                // In check above checked, that phi insts in same BB, so no problem with SaveStateOSR
                other_phi->ReplaceUsers(phi);
                PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            }
        }
    }

    if (TryEliminatePhi(inst->CastToPhi())) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}

void Peepholes::VisitSqrt([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (ConstFoldingSqrt(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitIsInstance(GraphVisitor *v, Inst *inst)
{
    static_cast<Peepholes *>(v)->GetGraph()->RunPass<ObjectTypePropagation>();
    if (ObjectTypeCheckElimination::TryEliminateIsInstance(inst)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        return;
    }
}

void Peepholes::VisitCastAnyTypeValue(GraphVisitor *v, Inst *inst)
{
    if (inst->GetUsers().Empty()) {
        // Peepholes can be run before cleanup phase.
        return;
    }

    auto *cav = inst->CastToCastAnyTypeValue();
    auto *cav_input = cav->GetDataFlowInput(0);
    if (cav_input->GetOpcode() == Opcode::CastValueToAnyType) {
        auto cva = cav_input->CastToCastValueToAnyType();
        auto value_inst = cva->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(cav, value_inst)) {
            return;
        }
        if (cva->GetAnyType() == cav->GetAnyType()) {
            // 1. .... -> v2
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3 -> v5
            // 5. ...
            // ===>
            // 1. .... -> v2, v5
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3
            // 5. ...
            cav->ReplaceUsers(value_inst);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            return;
        }
        auto input_type = value_inst->GetType();
        auto dst_type = cav->GetType();
        auto is_double_to_int = dst_type == DataType::INT32 && input_type == DataType::FLOAT64;
        if (IsTypeNumeric(input_type) && IsTypeNumeric(dst_type) && (!is_double_to_int || cav->IsIntegerWasSeen())) {
            // 1. .... -> v2
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3 -> v5
            // 5. ...
            // ===>
            // 1. .... -> v2, v6
            // 2. CastValueToAnyType v1 -> v3
            // 3. AnyTypeCheck v2 -> v3
            // 4. CastAnyTypeValue v3
            // 6. Cast v1 -> v5
            // 5. ...
            auto cast = CreateAndInsertInst(Opcode::Cast, cav, value_inst);
            cast->SetType(dst_type);
            cast->CastToCast()->SetOperandsType(input_type);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
        }
    }
}

void Peepholes::VisitCastValueToAnyType(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto input = inst->GetInput(0).GetInst();
    auto any_type = inst->CastToCastValueToAnyType()->GetAnyType();

    if (input->GetOpcode() == Opcode::CastAnyTypeValue) {
        auto cav = input->CastToCastAnyTypeValue();
        auto any_input = cav->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, any_input)) {
            return;
        }
        if (any_type == cav->GetAnyType() && cav->GetAllowedInputType() == profiling::AnyInputType::DEFAULT) {
            // 1. ... -> v2
            // 2. CastAnyTypeValue v1 -> v3
            // 3. CastValueToAnyType v2 -> v4
            // 4. ...
            // ===>
            // 1. ... -> v2, v4
            // 2. CastAnyTypeValue v1 -> v3
            // 3. CastValueToAnyType v2
            // 4. ...
            inst->ReplaceUsers(any_input);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
            return;
        }
    }

    if (graph->IsBytecodeOptimizer() || graph->IsOsrMode()) {
        // Find way to enable it in OSR mode.
        return;
    }

    auto base_type = AnyBaseTypeToDataType(any_type);
    // We propogate not INT types in Lowering
    if (base_type != DataType::INT32) {
        return;
    }
    // from
    // 2.any CastValueToAnyType INT_TYPE v1 -> (v3)
    // 3     SaveState                   v2(acc)
    //
    // to
    // 3     SaveState                   v1(acc)
    bool is_applied = false;
    for (auto it = inst->GetUsers().begin(); it != inst->GetUsers().end();) {
        auto user_inst = it->GetInst();
        if (user_inst->IsSaveState()) {
            user_inst->SetInput(it->GetIndex(), input);
            it = inst->GetUsers().begin();
            is_applied = true;
        } else {
            ++it;
        }
    }
    if (is_applied) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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
            if (SkipThisPeepholeInOSR(inst, input_inst)) {
                return;
            }
            inst->ReplaceInput(store_value_inst, input_inst);
            PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
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

// OverflowCheck can be removed if all its indirect users truncate input to int32
static bool CanRemoveOverflowCheck(Inst *inst, Marker marker)
{
    if (inst->SetMarker(marker)) {
        return true;
    }
    if (inst->GetOpcode() == Opcode::AnyTypeCheck) {
        auto any_type_check = inst->CastToAnyTypeCheck();
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto language = graph->GetRuntime()->GetMethodSourceLanguage(graph->GetMethod());
        auto int_any_type = NumericDataTypeToAnyType(DataType::INT32, language);
        // Bail out if this AnyTypeCheck can be triggered
        if (IsAnyTypeCanBeSubtypeOf(language, any_type_check->GetAnyType(), int_any_type,
                                    any_type_check->GetAllowedInputType()) != true) {
            return false;
        }
    }
    switch (inst->GetOpcode()) {
        case Opcode::Shl:
        case Opcode::Shr:
        case Opcode::AShr:
        case Opcode::And:
        case Opcode::Or:
        case Opcode::Xor:
            return true;
        // Next 3 opcodes should be removed in Peepholes and ChecksElimination, but Cast(f64).i32 or Or(x, 0)
        // may be optimized out by that moment
        case Opcode::CastAnyTypeValue:
        case Opcode::CastValueToAnyType:
        case Opcode::AnyTypeCheck:

        case Opcode::AddOverflowCheck:
        case Opcode::SubOverflowCheck:
        case Opcode::NegOverflowAndZeroCheck:
        case Opcode::Phi:
        // We check SaveState because if some of its users cannot be removed, result of OverflowCheck
        // may be used in interpreter after deoptimization
        case Opcode::SaveState:
            for (auto &user : inst->GetUsers()) {
                auto user_inst = user.GetInst();
                bool can_remove;
                if (DataType::IsFloatType(inst->GetType())) {
                    can_remove = user_inst->GetOpcode() == Opcode::Cast && user_inst->CastToCast()->IsDynamicCast();
                } else {
                    can_remove = CanRemoveOverflowCheck(user_inst, marker);
                }
                if (!can_remove) {
                    return false;
                }
            }
            return true;
        default:
            return false;
    }
}

void Peepholes::TryRemoveOverflowCheck(Inst *inst)
{
    auto block = inst->GetBasicBlock();
    auto marker_holder = MarkerHolder(block->GetGraph());
    if (CanRemoveOverflowCheck(inst, marker_holder.GetMarker())) {
        PEEPHOLE_IS_APPLIED(this, inst);
        block->RemoveOverflowCheck(inst);
        return;
    }
}

void Peepholes::VisitAddOverflowCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::AddOverflowCheck);
    auto visitor = static_cast<Peepholes *>(v);
    visitor->TrySwapInputs(inst);
    if (visitor->TrySimplifyAddSubWithZeroInput(inst)) {
        inst->ClearFlag(inst_flags::NO_DCE);
    } else {
        visitor->TryRemoveOverflowCheck(inst);
    }
}

void Peepholes::VisitSubOverflowCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SubOverflowCheck);
    auto visitor = static_cast<Peepholes *>(v);
    if (visitor->TrySimplifyAddSubWithZeroInput(inst)) {
        inst->ClearFlag(inst_flags::NO_DCE);
    } else {
        visitor->TryRemoveOverflowCheck(inst);
    }
}

void Peepholes::VisitNegOverflowAndZeroCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::NegOverflowAndZeroCheck);
    static_cast<Peepholes *>(v)->TryRemoveOverflowCheck(inst);
}

// This function check that we can merge two Phi instructions in one basic block.
bool Peepholes::IsPhiUnionPossible(PhiInst *phi1, PhiInst *phi2)
{
    ASSERT(phi1->GetBasicBlock() == phi2->GetBasicBlock() && phi1->GetInputsCount() == phi2->GetInputsCount());
    if (phi1 == phi2 || phi1->GetType() != phi2->GetType()) {
        return false;
    }
    for (auto pred_block : phi1->GetBasicBlock()->GetPredsBlocks()) {
        if (phi1->GetPhiDataflowInput(pred_block) != phi2->GetPhiDataflowInput(pred_block)) {
            return false;
        }
    }
    if (phi1->GetBasicBlock()->GetGraph()->IsOsrMode()) {
        // Values in vregs for phi1 and phi2 may be different in interpreter mode,
        // so we don't merge phis if they have SaveStateOsr users
        for (auto phi : {phi1, phi2}) {
            for (auto &user : phi->GetUsers()) {
                if (user.GetInst()->GetOpcode() == Opcode::SaveStateOsr) {
                    return false;
                }
            }
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
#ifdef PANDA_COMPILER_DEBUG_INFO
    new_inst->SetCurrentMethod(curr_inst->GetCurrentMethod());
#endif
    // It is a wrapper, so we don't do logic check for SaveStateOSR
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
           opc == Opcode::Max || opc == Opcode::Mul || opc == Opcode::AddOverflowCheck);
    if (inst->GetInput(0).GetInst()->IsConst()) {
        inst->SwapInputs();
        PEEPHOLE_IS_APPLIED(this, inst);
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
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(this, inst);
            return;
        }
        // Repeated arithmetic with constants
        // 2. shl/shr/ashr v1, const1 -> {v3, ...}
        // 3. shl/shr/ashr v2, const2 -> {...}
        // ===>
        // 2. shl/shr/ashr v1, const1 -> {...}
        // 3. shl/shr/ashr v1, const2 + const1 -> {...}
        bool osr_blocked_peephole = false;
        if (opc == input0->GetOpcode() && TryCombineShiftConst(inst, cnst, &osr_blocked_peephole)) {
            PEEPHOLE_IS_APPLIED(this, inst);
            return;
        }
        if (osr_blocked_peephole) {
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
            PEEPHOLE_IS_APPLIED(this, inst);
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
    // "inst", "ashr", "shr", "add", "result" one by one, so we need check SaveStateOSR only between "inst" and
    // "input0". "input0" is input of "inst", so we don't need check SaveStateOsr in this case
    if (n != -1) {
        auto type_size = DataType::GetTypeSize(inst->GetType(), graph->GetArch());
        auto ashr =
            graph->CreateInstAShr(inst->GetType(), INVALID_PC, input0, graph->FindOrCreateConstant(type_size - 1));
        bb->InsertAfter(ashr, inst);
        auto shr = graph->CreateInstShr(inst->GetType(), INVALID_PC, ashr, graph->FindOrCreateConstant(type_size - n));
        bb->InsertAfter(shr, ashr);
        auto add = graph->CreateInstAdd(inst->GetType(), INVALID_PC, shr, input0);
        bb->InsertAfter(add, shr);
        Inst *result = graph->CreateInstAShr(inst->GetType(), INVALID_PC, add, graph->FindOrCreateConstant(n));
        bb->InsertAfter(result, add);
        if (signed_val < 0) {
            auto div = result;
            result = graph->CreateInstNeg(inst->GetType(), INVALID_PC, div);
            bb->InsertAfter(result, div);
        }
        inst->ReplaceUsers(result);
        PEEPHOLE_IS_APPLIED(this, inst);
    }
}

bool Peepholes::TrySimplifyAddSubWithZeroInput(Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();

    if (input1->IsConst()) {
        auto cnst = input1->CastToConstant();
        if (cnst->IsEqualConstAllTypes(0)) {
            if (SkipThisPeepholeInOSR(inst, input0)) {
                return false;
            }
            inst->ReplaceUsers(input0);
            PEEPHOLE_IS_APPLIED(this, inst);
            return true;
        }
    }
    return false;
}

bool Peepholes::TrySimplifyAddSubWithConstInput(Inst *inst)
{
    if (TrySimplifyAddSubWithZeroInput(inst)) {
        return true;
    }
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input1->IsConst()) {
        auto cnst = input1->CastToConstant();
        if ((input0->GetOpcode() == Opcode::Add || input0->GetOpcode() == Opcode::Sub)) {
            bool osr_blocked_peephole = false;
            if (TryCombineAddSubConst(inst, cnst, &osr_blocked_peephole)) {
                PEEPHOLE_IS_APPLIED(this, inst);
                return true;
            }
            if (osr_blocked_peephole) {
                return true;
            }
        }
    } else if (input0->IsConst() && inst->GetOpcode() == Opcode::Sub && !IsFloatType(inst->GetType())) {
        // Fold `sub 0, v1` into `neg v1`.
        auto cnst = input0->CastToConstant();
        if (cnst->IsEqualConstAllTypes(0)) {
            // There can't be a SaveStateOSR between inst(sub) and new inst(neg), so we don't have a check
            CreateAndInsertInst(Opcode::Neg, inst, input1);
            PEEPHOLE_IS_APPLIED(this, inst);
            return true;
        }
    }
    return false;
}

template <Opcode OPC, int IDX>
void Peepholes::TrySimplifyAddSub(Inst *inst, Inst *input0, Inst *input1)
{
    if (input0->GetOpcode() == OPC && input0->GetInput(1 - IDX).GetInst() == input1) {
        auto prev_input = input0->GetInput(IDX).GetInst();
        if (inst->GetType() == prev_input->GetType()) {
            if (SkipThisPeepholeInOSR(inst, prev_input)) {
                return;
            }
            inst->ReplaceUsers(prev_input);
            PEEPHOLE_IS_APPLIED(this, inst);
            return;
        }
    }
}

bool Peepholes::TrySimplifyAddSubAdd(Inst *inst, Inst *input0, Inst *input1)
{
    // (a - b) + (b + c) = a + c
    if (input0->GetInput(1) == input1->GetInput(0)) {
        auto new_input0 = input0->GetInput(0).GetInst();
        auto new_input1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a - b) + (c + b) = a + c
    if (input0->GetInput(1) == input1->GetInput(1)) {
        auto new_input0 = input0->GetInput(0).GetInst();
        auto new_input1 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

bool Peepholes::TrySimplifyAddSubSub(Inst *inst, Inst *input0, Inst *input1)
{
    // (a - b) + (b - c) = a - c
    if (input0->GetInput(1) == input1->GetInput(0)) {
        auto new_input0 = input0->GetInput(0).GetInst();
        auto new_input1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }

        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a - b) + (c - a) = c - b
    if (input0->GetInput(0) == input1->GetInput(1)) {
        auto new_input0 = input1->GetInput(0).GetInst();
        auto new_input1 = input0->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }
        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    return false;
}

bool Peepholes::TrySimplifySubAddAdd(Inst *inst, Inst *input0, Inst *input1)
{
    // (a + b) - (a + c) = b - c
    if (input0->GetInput(0) == input1->GetInput(0)) {
        auto new_input0 = input0->GetInput(1).GetInst();
        auto new_input1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (c + a) = b - c
    if (input0->GetInput(0) == input1->GetInput(1)) {
        auto new_input0 = input0->GetInput(1).GetInst();
        auto new_input1 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (b + c) = a - c
    if (input0->GetInput(1) == input1->GetInput(0)) {
        auto new_input0 = input0->GetInput(0).GetInst();
        auto new_input1 = input1->GetInput(1).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }

    // (a + b) - (c + b) = a - c
    if (input0->GetInput(1) == input1->GetInput(1)) {
        auto new_input0 = input0->GetInput(0).GetInst();
        auto new_input1 = input1->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, new_input0) || SkipThisPeepholeInOSR(inst, new_input1)) {
            return true;
        }
        inst->SetInput(0, new_input0);
        inst->SetInput(1, new_input1);
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
    if (shl1->GetOpcode() != Opcode::Shl || !shl1->HasSingleUser() || shl1->GetInput(0) != shl0->GetInput(0)) {
        return false;
    }
    if (!shl0->GetInput(1).GetInst()->IsConst() || !shl1->GetInput(1).GetInst()->IsConst()) {
        return false;
    }
    auto input0 = inst->GetInput(0).GetInst();
    if (!input0->IsConst() && !input0->IsParameter() && !input0->IsDominate(input1)) {
        return false;
    }

    Opcode op_input1 = input1->GetOpcode();
    Opcode op_inst = inst->GetOpcode();
    if (op_inst == Opcode::Sub && op_input1 == Opcode::Add) {
        // input0 - (shl0 + shl1) -> (input0 - shl0) - shl1
        op_input1 = Opcode::Sub;
    } else if (op_inst == Opcode::Add && op_input1 == Opcode::Sub) {
        // input0 + (shl0 - shl1) -> (input0 + shl0) - shl1
        op_input1 = Opcode::Add;
        op_inst = Opcode::Sub;
    } else if (op_inst == Opcode::Sub && op_input1 == Opcode::Sub) {
        // input0 - (shl0 - shl1) -> (input0 - shl0) + shl1
        op_inst = Opcode::Add;
    } else if (op_inst != Opcode::Add && op_input1 != Opcode::Add) {
        UNREACHABLE();
    }
    // "input1" and "new_input0" one by one, so we don't should to check "SaveStateOSR" between this insts,
    // same for: "inst" and **INST WITHOUT NAME**. We should check only between "inst" and "input1"
    if (SkipThisPeepholeInOSR(inst, input1)) {
        return true;
    }
    auto new_input0 = CreateAndInsertInst(op_input1, input1, input0, shl0);
    CreateAndInsertInst(op_inst, inst, new_input0, shl1);
    return true;
}

void Peepholes::TrySimplifyNeg(Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if ((input0->GetOpcode() == Opcode::Neg && SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) ||
        (input1->GetOpcode() == Opcode::Neg && SkipThisPeepholeInOSR(inst, input1->GetInput(0).GetInst()))) {
        return;
    }

    // Case 5
    if (input0->GetOpcode() == Opcode::Neg && !input1->IsConst()) {
        inst->SetInput(0, input1);
        inst->SetInput(1, input0);
        std::swap(input0, input1);
    }
    if (input1->GetOpcode() == Opcode::Neg) {
        inst->SetOpcode(Opcode::Sub);
        inst->SetInput(1, input1->GetInput(0).GetInst());
        PEEPHOLE_IS_APPLIED(this, inst);
        return;
    }
}

bool Peepholes::TrySimplifyCompareNegation(Inst *inst)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->GetOpcode() == Opcode::Add);

    // Case 9: Neg -> Add -> Compare
    bool optimized = false;
    for (auto &user_add : inst->GetUsers()) {
        auto suspect_inst = user_add.GetInst();
        if (suspect_inst->GetOpcode() != Opcode::Compare) {
            continue;
        }
        auto compare_inst = suspect_inst->CastToCompare();
        if (compare_inst->GetOperandsType() != DataType::BOOL ||
            (compare_inst->GetCc() != ConditionCode::CC_EQ && compare_inst->GetCc() != ConditionCode::CC_NE)) {
            continue;
        }

        unsigned index_cast = compare_inst->GetInput(0).GetInst() == inst ? 0 : 1;
        auto bool_value = inst->GetInput(0).GetInst()->GetInput(0).GetInst();
        if (SkipThisPeepholeInOSR(inst, bool_value)) {
            continue;
        }
        compare_inst->SetInput(index_cast, bool_value);
        compare_inst->SetCc(GetInverseConditionCode(compare_inst->GetCc()));
        PEEPHOLE_IS_APPLIED(this, compare_inst);
        optimized = true;
    }
    return optimized;
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
            PEEPHOLE_IS_APPLIED(this, inst);
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
        // 2.signed ASHR v0, type_size - 1 -> {v4} // 0 or -1
        // 4.signed SHR v2, type_size - n -> {v5} //  0 or 2^n - 1
        // 5.signed ADD v4, v0 -> {v6}
        // 6.signed ASHR v5, n -> {v3 or v7}
        // if n < 0 7.signed NEG v6 ->{v3}
        // 3.signed INST v6 or v7
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
    if (SkipThisPeepholeInOSR(inst, input)) {
        return true;
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
bool Peepholes::TrySimplifyCompareWithBoolInput(Inst *inst, bool *is_osr_blocked)
{
    ASSERT(is_osr_blocked != nullptr);
    auto compare = inst->CastToCompare();
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *const_input = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &const_input, &swap)) {
        return false;
    }
    if (input->GetType() != DataType::BOOL) {
        return false;
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();
    InputCode code = GetInputCode(const_input, cc);
    if (code == INPUT_TRUE || code == INPUT_FALSE) {
        // We create constant, so we don't need to check SaveStateOSR between insts
        compare->ReplaceUsers(ConstFoldingCreateIntConst(compare, code == INPUT_TRUE ? 1 : 0));
        return true;
    }
    if (code == INPUT_ORIG) {
        if (SkipThisPeepholeInOSR(compare, input)) {
            *is_osr_blocked = true;
            return true;
        }
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
bool Peepholes::TrySimplifyCmpCompareWithZero(Inst *inst, bool *is_osr_blocked)
{
    ASSERT(is_osr_blocked != nullptr);
    auto compare = inst->CastToCompare();
    if (compare->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return false;
    }
    bool swap = false;
    Inst *input = nullptr;
    ConstantInst *const_input = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &const_input, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::Cmp) {
        return false;
    }
    if (!const_input->IsEqualConstAllTypes(0)) {
        return false;
    }
    auto cmp_op_type = input->CastToCmp()->GetOperandsType();
    if (IsFloatType(cmp_op_type)) {
        return false;
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();
    if (!IsTypeSigned(cmp_op_type)) {
        ASSERT(cc == ConditionCode::CC_EQ || cc == ConditionCode::CC_NE || IsSignedConditionCode(cc));
        // If Cmp operands are unsigned then Compare.CC must be converted to unsigned.
        cc = InverseSignednessConditionCode(cc);
    }
    if (SkipThisPeepholeInOSR(compare, input->GetInput(0).GetInst()) ||
        SkipThisPeepholeInOSR(compare, input->GetInput(1).GetInst())) {
        *is_osr_blocked = true;
        return true;
    }
    compare->SetInput(0, input->GetInput(0).GetInst());
    compare->SetInput(1, input->GetInput(1).GetInst());
    compare->SetOperandsType(cmp_op_type);
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
    // We create constant, so we don't need to check SaveStateOSR between insts
    cmp_inst->SetInput(1, ConstFoldingCreateIntConst(input1, 0));
    return true;
}

bool Peepholes::TrySimplifyCompareAndZero(Inst *inst, bool *is_osr_blocked)
{
    ASSERT(is_osr_blocked != nullptr);
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

    if (SkipThisPeepholeInOSR(cmp_inst, input->GetInput(0).GetInst())) {
        *is_osr_blocked = true;
        return true;
    }
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
    ConstantInst *const_input = nullptr;
    if (!GetInputsOfCompareWithConst(compare, &input, &const_input, &swap)) {
        return false;
    }
    if (input->GetOpcode() != Opcode::LenArray || !const_input->IsEqualConstAllTypes(0)) {
        return false;
    }
    ConditionCode cc = swap ? SwapOperandsConditionCode(compare->GetCc()) : compare->GetCc();

    if (cc == CC_GE || cc == CC_LT) {
        // We create constant, so we don't need to check SaveStateOSR between insts
        compare->ReplaceUsers(ConstFoldingCreateIntConst(compare, cc == CC_GE ? 1 : 0));
        return true;
    }
    return false;
}

// Try to combine constants when arithmetic operations with constants are repeated
template <typename T>
bool Peepholes::TryCombineConst(Inst *inst, ConstantInst *cnst1, T combine, bool *is_osr_blocked)
{
    ASSERT(is_osr_blocked != nullptr);
    auto input0 = inst->GetInput(0).GetInst();
    auto previnput1 = input0->GetInput(1).GetInst();
    if (previnput1->IsConst() && inst->GetType() == input0->GetType()) {
        if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) {
            *is_osr_blocked = true;
            return false;
        }
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

bool Peepholes::TryCombineAddSubConst(Inst *inst, ConstantInst *cnst1, bool *is_osr_blocked)
{
    ASSERT(is_osr_blocked != nullptr);
    auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Add || opc == Opcode::Sub);
    auto input0 = inst->GetInput(0).GetInst();
    auto combine = [&opc, &input0](auto x, auto y) { return opc == input0->GetOpcode() ? x + y : x - y; };
    return TryCombineConst(inst, cnst1, combine, is_osr_blocked);
}

bool Peepholes::TryCombineShiftConst(Inst *inst, ConstantInst *cnst1, bool *is_osr_blocked)
{
    ASSERT(is_osr_blocked != nullptr);
    auto opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Shl || opc == Opcode::Shr || opc == Opcode::AShr);

    auto input0 = inst->GetInput(0).GetInst();
    auto previnput1 = input0->GetInput(1).GetInst();
    if (!(previnput1->IsConst() && inst->GetType() == input0->GetType())) {
        return false;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    uint64_t size_mask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
    auto cnst2 = static_cast<ConstantInst *>(previnput1);
    auto new_value = (cnst1->GetIntValue() & size_mask) + (cnst2->GetIntValue() & size_mask);
    // If new_value > size_mask, result is always 0 for Shr and Shl,
    // and 0 or -1 (depending on highest bit of input) for AShr
    if (new_value <= size_mask || opc == Opcode::AShr) {
        if (SkipThisPeepholeInOSR(inst, input0->GetInput(0).GetInst())) {
            *is_osr_blocked = true;
            return false;
        }
        auto new_cnst = ConstFoldingCreateIntConst(inst, std::min(new_value, size_mask));
        inst->SetInput(0, input0->GetInput(0).GetInst());
        inst->SetInput(1, new_cnst);
        return true;
    }
    auto new_cnst = ConstFoldingCreateIntConst(inst, 0);
    inst->ReplaceUsers(new_cnst);
    return true;
}

bool Peepholes::TryCombineMulConst(Inst *inst, ConstantInst *cnst1, bool *is_osr_blocked)
{
    ASSERT(is_osr_blocked != nullptr);
    ASSERT(inst->GetOpcode() == Opcode::Mul);
    auto combine = [](auto x, auto y) { return x * y; };
    return TryCombineConst(inst, cnst1, combine, is_osr_blocked);
}

bool Peepholes::GetInputsOfCompareWithConst(const Inst *inst, Inst **input, ConstantInst **const_input,
                                            bool *inputs_swapped)
{
    if (inst->GetOpcode() == Opcode::Compare || inst->GetOpcode() == Opcode::Cmp) {
        if (inst->GetInput(1).GetInst()->IsConst()) {
            *input = inst->GetInput(0).GetInst();
            *const_input = inst->GetInput(1).GetInst()->CastToConstant();
            *inputs_swapped = false;
            return true;
        }
        if (inst->GetInput(0).GetInst()->IsConst()) {
            *input = inst->GetInput(1).GetInst();
            *const_input = inst->GetInput(0).GetInst()->CastToConstant();
            *inputs_swapped = true;
            return true;
        }
    }
    return false;
}

Inst *GenerateXorWithOne(BasicBlock *block, Inst *if_imm_input)
{
    auto graph = block->GetGraph();
    auto xor_inst = graph->CreateInstXor(DataType::BOOL, block->GetGuestPc());
    xor_inst->SetInput(0, if_imm_input);
    Inst *one_const = nullptr;
    if (graph->IsBytecodeOptimizer()) {
        one_const = graph->FindOrCreateConstant<uint32_t>(1);
    } else {
        one_const = graph->FindOrCreateConstant<uint64_t>(1);
    }
    xor_inst->SetInput(1, one_const);
    // We can add inst "xor" before SaveStateOSR in BasicBlock
    block->PrependInst(xor_inst);
    return xor_inst;
}

std::optional<bool> IsBoolPhiInverted(PhiInst *phi, IfImmInst *if_imm)
{
    auto phi_input0 = phi->GetInput(0).GetInst();
    auto phi_input1 = phi->GetInput(1).GetInst();
    if (!phi_input0->IsBoolConst() || !phi_input1->IsBoolConst()) {
        return std::nullopt;
    }
    auto constant0 = phi_input0->CastToConstant()->GetRawValue();
    auto constant1 = phi_input1->CastToConstant()->GetRawValue();
    if (constant0 == constant1) {
        return std::nullopt;
    }
    // Here constant0 and constant1 are 0 and 1 in some order

    auto inverted_if = IsIfInverted(phi->GetBasicBlock(), if_imm);
    if (inverted_if == std::nullopt) {
        return std::nullopt;
    }
    // constant0 is also index of phi input equal to 0
    if (phi->GetPhiInputBbNum(constant0) == 0) {
        return !*inverted_if;
    }
    return inverted_if;
}

bool Peepholes::TryEliminatePhi(PhiInst *phi)
{
    if (phi->GetInputsCount() != MAX_SUCCS_NUM) {
        return false;
    }

    auto bb = phi->GetBasicBlock();
    auto dom = bb->GetDominator();
    if (dom->IsEmpty()) {
        return false;
    }
    auto last = dom->GetLastInst();
    if (last->GetOpcode() != Opcode::IfImm) {
        return false;
    }

    auto graph = dom->GetGraph();
    auto if_imm = last->CastToIfImm();
    auto input = if_imm->GetInput(0).GetInst();
    // In case of the bytecode optimizer we can not generate Compare therefore we check that Peepholes has eliminated
    // Compare
    if (graph->IsBytecodeOptimizer() && input->GetOpcode() == Opcode::Compare) {
        return false;
    }
    if (input->GetType() != DataType::BOOL ||
        GetTypeSize(phi->GetType(), graph->GetArch()) != GetTypeSize(input->GetType(), graph->GetArch())) {
        return false;
    }
    auto inverted = IsBoolPhiInverted(phi, if_imm);
    if (!inverted) {
        return false;
    }
    if (*inverted) {
        // 0. Const 0
        // 1. Const 1
        // 2. v2
        // 3. IfImm NE b v2, 0x0
        // 4. Phi v0, v1 -> v5, ...
        // ===>
        // 0. Const 0
        // 1. Const 1
        // 2. v2
        // 3. IfImm NE b v2, 0x0
        // 6. Xor v2, v1 -> v5, ...
        // 4. Phi v0, v1

        // "xori"(xor) will insert as first inst in BB, so enough check between first inst and input
        if (SkipThisPeepholeInOSR(phi, input)) {
            return false;
        }
        auto xori = GenerateXorWithOne(phi->GetBasicBlock(), input);
        phi->ReplaceUsers(xori);
    } else {
        // 0. Const 0
        // 1. Const 1
        // 2. v2
        // 3. IfImm NE b v2, 0x0
        // 4. Phi v1, v0 -> v5, ...
        // ===>
        // 0. Const 0
        // 1. Const 1
        // 2. v2 -> v5, ...
        // 3. IfImm NE b v2, 0x0
        // 4. Phi v1, v0

        if (SkipThisPeepholeInOSR(phi, input)) {
            return false;
        }
        phi->ReplaceUsers(input);
    }
    return true;
}

bool Peepholes::SkipThisPeepholeInOSR(Inst *inst, Inst *new_input)
{
    auto osr = inst->GetBasicBlock()->GetGraph()->IsOsrMode();
    return osr && new_input->GetOpcode() != Opcode::Constant && IsInstInDifferentBlocks(inst, new_input);
}

void Peepholes::VisitGetInstanceClass(GraphVisitor *v, Inst *inst)
{
    auto type_info = inst->GetDataFlowInput(0)->GetObjectTypeInfo();
    if (type_info && type_info.IsExact()) {
        auto klass = type_info.GetClass();
        auto bb = inst->GetBasicBlock();
        auto graph = bb->GetGraph();
        auto class_imm = graph->CreateInstLoadImmediate(DataType::REFERENCE, inst->GetPc(), klass);
        inst->ReplaceUsers(class_imm);
        bb->InsertAfter(class_imm, inst);
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
    }
}
void Peepholes::VisitLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto klass = inst->CastToLoadAndInitClass()->GetClass();
    if (klass == nullptr || !graph->GetRuntime()->IsClassInitialized(reinterpret_cast<uintptr_t>(klass))) {
        return;
    }
    auto class_imm = graph->CreateInstLoadImmediate(DataType::REFERENCE, inst->GetPc(), klass);
    inst->ReplaceUsers(class_imm);
    inst->GetBasicBlock()->InsertAfter(class_imm, inst);

    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitInitClass(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto klass = inst->CastToInitClass()->GetClass();
    if (klass == nullptr || !graph->GetRuntime()->IsClassInitialized(reinterpret_cast<uintptr_t>(klass))) {
        return;
    }
    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitIntrinsic([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto intrinsic = inst->CastToIntrinsic();
    switch (intrinsic->GetIntrinsicId()) {
#include "intrinsics_peephole.inl"
        default: {
            return;
        }
    }
}

void Peepholes::VisitLoadClass(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto klass = inst->CastToLoadClass()->GetClass();
    if (klass == nullptr) {
        return;
    }
    auto class_imm = graph->CreateInstLoadImmediate(DataType::REFERENCE, inst->GetPc(), klass);
    inst->ReplaceUsers(class_imm);
    inst->GetBasicBlock()->InsertAfter(class_imm, inst);

    inst->ClearFlag(compiler::inst_flags::NO_DCE);
    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitLoadConstantPool(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->IsJitOrOsrMode()) {
        return;
    }
    auto func = inst->GetInput(0).GetInst();
    void *constant_pool = nullptr;
    if (func->IsParameter() && func->CastToParameter()->GetArgNumber() == 0) {
        constant_pool = graph->GetRuntime()->GetConstantPool(graph->GetMethod());
    } else {
        CallInst *caller_inst = nullptr;
        for (auto &user : inst->GetUsers()) {
            auto user_inst = user.GetInst();
            if (user_inst->GetOpcode() == Opcode::SaveState &&
                user.GetVirtualRegister().GetVRegType() == VRegType::CONST_POOL) {
                caller_inst = user_inst->CastToSaveState()->GetCallerInst();
                ASSERT(caller_inst != nullptr);
                break;
            }
        }
        if (caller_inst == nullptr) {
            return;
        }
        if (auto func_object = caller_inst->GetFunctionObject(); func_object != 0) {
            constant_pool = graph->GetRuntime()->GetConstantPool(func_object);
        } else {
            constant_pool = graph->GetRuntime()->GetConstantPool(caller_inst->GetCallMethod());
        }
    }
    if (constant_pool == nullptr) {
        return;
    }
    auto constant_pool_imm = graph->CreateInstLoadImmediate(DataType::ANY, inst->GetPc(), constant_pool,
                                                            LoadImmediateInst::ObjectType::CONSTANT_POOL);
    inst->InsertAfter(constant_pool_imm);
    inst->ReplaceUsers(constant_pool_imm);

    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

void Peepholes::VisitLoadFromConstantPool(GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    // LoadFromConstantPool with string flag may be used for intrinsics inlining, do not optimize too early
    if (!graph->IsUnrollComplete()) {
        return;
    }
    auto constant_pool = inst->GetInput(0).GetInst();
    if (constant_pool->GetOpcode() != Opcode::LoadImmediate) {
        return;
    }
    auto offset = inst->CastToLoadFromConstantPool()->GetTypeId();
    auto shift = DataType::ShiftByType(DataType::ANY, graph->GetArch());
    uintptr_t mem = constant_pool->CastToLoadImmediate()->GetConstantPool() +
                    graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()) + (offset << shift);
    auto load = graph->CreateInstLoadObjFromConst(DataType::ANY, inst->GetPc(), mem);
    inst->InsertAfter(load);
    inst->ReplaceUsers(load);

    PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), inst);
}

bool Peepholes::CreateCompareInsteadOfXorAdd(Inst *old_inst)
{
    ASSERT(old_inst->GetOpcode() == Opcode::Xor || old_inst->GetOpcode() == Opcode::Add);
    auto input0 = old_inst->GetInput(0).GetInst();
    [[maybe_unused]] auto input1 = old_inst->GetInput(1).GetInst();

    if (old_inst->GetOpcode() == Opcode::Add) {
        ASSERT(input0->GetOpcode() == Opcode::Neg);
        input0 = input0->GetInput(0).GetInst();
        for (auto &user_add : old_inst->GetUsers()) {
            if (SkipThisPeepholeInOSR(user_add.GetInst(), input0)) {
                return false;
            }
        }
    }

    // We shouldn't check on OSR with Xor, because old_inst and cmp_inst is placed one by one
    ASSERT(input0->GetType() == DataType::BOOL && input1->IsConst() && input1->CastToConstant()->GetIntValue() == 1U);
    auto cnst = old_inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(0);
    auto cmp_inst = CreateAndInsertInst(Opcode::Compare, old_inst, input0, cnst);
    cmp_inst->SetType(DataType::BOOL);
    cmp_inst->CastToCompare()->SetCc(ConditionCode::CC_EQ);
    cmp_inst->CastToCompare()->SetOperandsType(DataType::BOOL);
    auto type = old_inst->GetType();
    if (type == DataType::UINT64 || type == DataType::INT64) {
        auto cast = cmp_inst->GetBasicBlock()->GetGraph()->CreateInstCast();
        cast->SetType(type);
        cmp_inst->InsertAfter(cast);
        cmp_inst->ReplaceUsers(cast);
        cast->SetInput(0, cmp_inst);
        cast->SetOperandsType(DataType::BOOL);
    }
    return true;
}

// Move users from LenArray to constant which used in MultiArray
// Case 1
// 1.s64 ... ->{v3}
// 2.s64 ... ->{v3}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.s32 LenArray v3 ->{v5, ...}
// 5.    USE      v5
// ===>
// 1.s64 ... ->{v3, v5, ...}
// 2.s64 ... ->{v3}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.s32 LenArray v3
// 5.    USE      v1

// Case 2
// 1.s64 ... ->{v3}
// 2.s64 ... ->{v3}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.ref LoadArray v3, ...
// 5.ref NullCheck v4, ...
// 6.s32 LenArray v5 ->{v7, ...}
// 7.    USE      v6
// ===>
// 1.s64 ... ->{v3}
// 2.s64 ... ->{v3, v7, ...}
// 3.ref MultiArray ... , v1, v2, ... ->{v4,..}
// 4.ref LoadArray v3, ...
// 5.ref NullCheck v4, ...
// 6.s32 LenArray v5
// 7.    USE      v2
bool Peepholes::OptimizeLenArrayForMultiArray(Inst *len_array, Inst *inst, size_t index_size)
{
    if (inst->GetOpcode() == Opcode::MultiArray) {
        // Arguments of MultiArray look like : class, size_0, size_1, ..., size_N, SaveState
        // When element type of multyarray is array-like object (LenArray can be applyed to it), we can get case when
        // number sequential LoadArray with LenArray more than dimension of MultiArrays. So limiting the index_size.
        // Example in unittest PeepholesTest.MultiArrayWithLenArrayOfString

        auto multiarr_dimension = inst->GetInputsCount() - 2;
        if (!(index_size < multiarr_dimension)) {
            return false;
        }
        // MultiArray's sizes starts from index "1", so need add "1" to get absolute index
        auto value = inst->GetDataFlowInput(index_size + 1);
        for (auto &it : len_array->GetUsers()) {
            if (SkipThisPeepholeInOSR(it.GetInst(), value)) {
                return false;
            }
        }
        len_array->ReplaceUsers(value);
        return true;
    }
    if (inst->GetOpcode() == Opcode::LoadArray) {
        auto input = inst->GetDataFlowInput(0);
        return OptimizeLenArrayForMultiArray(len_array, input, index_size + 1);
    }
    return false;
}

bool Peepholes::IsNegationPattern(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add);
    // Negetion pattern is:
    // 1.   Constant 0x1
    // 2.b  ...
    // 3.i32 Neg v2 -> v4
    // 4.i32 Add v3, v1
    auto input0 = inst->GetInput(0).GetInst();
    if (input0->GetOpcode() != Opcode::Neg || input0->GetInput(0).GetInst()->GetType() != DataType::BOOL ||
        !input0->HasSingleUser()) {
        return false;
    }
    // We sure, that constant may be only in input1
    auto input1 = inst->GetInput(1).GetInst();
    return input1->GetOpcode() == Opcode::Constant && input1->CastToConstant()->GetIntValue() == 1;
}

bool Peepholes::TrySimplifyNegationPattern(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add);
    if (!IsNegationPattern(inst)) {
        return false;
    }
    auto suspect_inst = inst->GetInput(0).GetInst()->GetInput(0).GetInst();
    // Case 8
    // We sure, that type of Neg's input is Bool. We shue, that Neg has one user
    if (suspect_inst->GetOpcode() == Opcode::Compare && suspect_inst->HasSingleUser()) {
        auto compare_inst = suspect_inst->CastToCompare();
        bool is_possible = true;
        for (auto &i : inst->GetUsers()) {
            if (SkipThisPeepholeInOSR(i.GetInst(), compare_inst)) {
                is_possible = false;
                break;
            }
        }
        if (is_possible) {
            inst->ReplaceUsers(compare_inst);
            compare_inst->SetCc(GetInverseConditionCode(compare_inst->GetCc()));
            PEEPHOLE_IS_APPLIED(this, compare_inst);
            return true;
        }
    }

    // Case 9
    if (TrySimplifyCompareNegation(inst)) {
        return true;
    }

    // Case 7
    // This is used last of all if no case has worked!
    if (CreateCompareInsteadOfXorAdd(inst)) {
        PEEPHOLE_IS_APPLIED(this, inst);
        return true;
    }
    return false;
}

}  // namespace panda::compiler
