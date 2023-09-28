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

#include <array>
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "lowering.h"
#include "optimizer/code_generator/encode.h"

namespace panda::compiler {

void Lowering::VisitAdd([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto new_inst = LowerBinaryOperationWithShiftedOperand<Opcode::Add>(inst);
    if (new_inst == nullptr && LowerAddSub(inst) != nullptr) {
        return;
    }
    LowerMultiplyAddSub(new_inst == nullptr ? inst : new_inst);
}

void Lowering::VisitSub([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto new_inst = LowerBinaryOperationWithShiftedOperand<Opcode::Sub, false>(inst);
    if (new_inst == nullptr && LowerAddSub(inst) != nullptr) {
        return;
    }
    LowerMultiplyAddSub(inst);
}

void Lowering::VisitCastValueToAnyType([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || graph->IsOsrMode()) {
        // Find way to enable it in OSR mode.
        return;
    }

    // from
    // 1.u64 Const N -> (v2)
    // 2.any CastValueToAnyType INT_TYPE v1 -> (...)
    //
    // to
    // 1.any Const Pack(N) -> (...)
    if (LowerCastValueToAnyTypeWithConst(inst)) {
        graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
        return;
    }
    auto any_type = inst->CastToCastValueToAnyType()->GetAnyType();
    auto base_type = AnyBaseTypeToDataType(any_type);
    // We can't propogate opject, because GC can move it
    if (base_type == DataType::REFERENCE) {
        return;
    }
    // from
    // 2.any CastValueToAnyType INT_TYPE v1 -> (v3)
    // 3     SaveState                   v2(acc)
    //
    // to
    // 3     SaveState                   v1(acc)
    auto input = inst->GetInput(0).GetInst();
    if (input->IsConst() && base_type == DataType::VOID) {
        input = graph->FindOrCreateConstant(DataType::Any(input->CastToConstant()->GetIntValue()));
    }
    for (auto it = inst->GetUsers().begin(); it != inst->GetUsers().end();) {
        auto user_inst = it->GetInst();
        if (user_inst->IsSaveState()) {
            user_inst->SetInput(it->GetIndex(), input);
            it = inst->GetUsers().begin();
        } else {
            ++it;
        }
    }
}

void Lowering::VisitCast([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    // unsigned Load in AARCH64 zerod all high bits
    // from
    //  1.u8(u16, u32) Load ->(v2)
    //  2.u64(u32) Cast u8(u16, u32) -> (v3 ..)
    // to
    //  1.u8(u16) Load ->(v3, ..)
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        return;
    }
    auto type = inst->GetType();
    if (DataType::IsTypeSigned(type)) {
        return;
    }
    auto input_type = inst->CastToCast()->GetOperandsType();
    if (DataType::IsTypeSigned(input_type) || DataType::Is64Bits(input_type, graph->GetArch())) {
        return;
    }
    auto input_inst = inst->GetInput(0).GetInst();
    if (!input_inst->IsLoad() || input_inst->GetType() != input_type) {
        return;
    }
    inst->ReplaceUsers(input_inst);
    input_inst->GetBasicBlock()->GetGraph()->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()),
                                                                            inst->GetId(), inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

template <Opcode OPC>
void Lowering::VisitBitwiseBinaryOperation([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto new_inst = LowerBinaryOperationWithShiftedOperand<OPC>(inst);  // NOLINT(readability-magic-numbers)
    if (new_inst == nullptr && LowerLogic(inst) != nullptr) {
        return;
    }
    LowerLogicWithInvertedOperand(new_inst == nullptr ? inst : new_inst);
}

void Lowering::VisitOr(GraphVisitor *v, Inst *inst)
{
    VisitBitwiseBinaryOperation<Opcode::Or>(v, inst);
}

void Lowering::VisitAnd(GraphVisitor *v, Inst *inst)
{
    VisitBitwiseBinaryOperation<Opcode::And>(v, inst);
}

void Lowering::VisitXor(GraphVisitor *v, Inst *inst)
{
    VisitBitwiseBinaryOperation<Opcode::Xor>(v, inst);
}

void Lowering::VisitAndNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerBinaryOperationWithShiftedOperand<Opcode::AndNot, false>(inst);
}

void Lowering::VisitXorNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerBinaryOperationWithShiftedOperand<Opcode::XorNot, false>(inst);
}

void Lowering::VisitOrNot([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerBinaryOperationWithShiftedOperand<Opcode::OrNot, false>(inst);
}

void Lowering::VisitSaveState([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SaveState);
    LowerStateInst(inst->CastToSaveState());
}

void Lowering::VisitSafePoint([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SafePoint);
    LowerStateInst(inst->CastToSafePoint());
}

void Lowering::VisitSaveStateOsr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SaveStateOsr);
    LowerStateInst(inst->CastToSaveStateOsr());
}

void Lowering::VisitSaveStateDeoptimize([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::SaveStateDeoptimize);
    LowerStateInst(inst->CastToSaveStateDeoptimize());
}

void Lowering::VisitBoundsCheck([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    LowerConstArrayIndex<BoundsCheckInstI>(inst, Opcode::BoundsCheckI);
}

void Lowering::VisitLoadArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::LoadArray);
    LowerConstArrayIndex<LoadInstI>(inst, Opcode::LoadArrayI);
}

void Lowering::VisitLoadCompressedStringChar([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::LoadCompressedStringChar);
    LowerConstArrayIndex<LoadCompressedStringCharInstI>(inst, Opcode::LoadCompressedStringCharI);
}

void Lowering::VisitStoreArray([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::StoreArray);
    LowerConstArrayIndex<StoreInstI>(inst, Opcode::StoreArrayI);
}

void Lowering::VisitLoad([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Load);
    LowerMemInstScale(inst);
}

void Lowering::VisitStore([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Store);
    LowerMemInstScale(inst);
}

void Lowering::VisitReturn([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Return);
    LowerReturnInst(inst->CastToReturn());
}

void Lowering::VisitShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerShift(inst);
}

void Lowering::VisitAShr([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerShift(inst);
}

void Lowering::VisitShl([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerShift(inst);
}

void Lowering::VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::IfImm);
    static_cast<Lowering *>(v)->LowerIf(inst->CastToIfImm());
}

void Lowering::VisitMul([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (inst->GetInput(1).GetInst()->GetOpcode() != Opcode::Constant) {
        LowerNegateMultiply(inst);
    } else {
        LowerMulDivMod<Opcode::Mul>(inst);
    }
}

void Lowering::VisitDiv([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerMulDivMod<Opcode::Div>(inst);
}

bool Lowering::TryReplaceModPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return false;
    }
    if (DataType::IsFloatType(inst->GetType())) {
        return false;
    }
    auto c = inst->GetInput(1).GetInst();
    if (!c->IsConst()) {
        return false;
    }
    auto value = c->CastToConstant()->GetInt64Value();
    if (value == 0) {
        return false;
    }
    auto abs_value = std::abs(static_cast<int64_t>(value));
    if (BitCount(abs_value) != 1) {
        return false;
    }
    auto input0 = inst->GetInput(0).GetInst();
    if (DataType::IsTypeSigned(input0->GetType())) {
        ReplaceSignedModPowerOfTwo(v, inst, abs_value);
    } else {
        ReplaceUnsignedModPowerOfTwo(v, inst, abs_value);
    }
    return true;
}

void Lowering::ReplaceSignedModPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst, uint64_t abs_value)
{
    // It is optimal for AARCH64, not for AMD64. But even for AMD64 significantly better than original Mod.
    // 1. ...
    // 2. Const 0x4
    // 3. Mod v1, v2
    // ====>
    // 1. ...
    // 4. Const 0x3
    // 7. Const 0xFFFFFFFFFFFFFFFC
    // 5. Add v1, v4
    // 6. SelectImm v5, v1, v1, 0, CC_LT
    // 8. And v6, v7
    // 9. Sub v1, v8
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto input0 = inst->GetInput(0).GetInst();
    auto value_minus_1 = abs_value - 1;
    uint32_t size = (inst->GetType() == DataType::UINT64 || inst->GetType() == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    Inst *add_inst;
    if (graph->GetEncoder()->CanEncodeImmAddSubCmp(value_minus_1, size, false)) {
        add_inst = graph->CreateInstAddI(inst, value_minus_1);
    } else {
        auto value_minus_1_cnst = graph->FindOrCreateConstant(value_minus_1);
        add_inst = graph->CreateInstAdd(inst);
        add_inst->SetInput(1U, value_minus_1_cnst);
    }
    add_inst->SetInput(0U, input0);
    Inst *select_inst;
    if (graph->GetEncoder()->CanEncodeImmAddSubCmp(0, size, true)) {
        auto select = graph->CreateInstSelectImm(inst, ConditionCode::CC_LT, 0);
        select->SetOperandsType(inst->GetType());
        select_inst = select;
    } else {
        auto zero_cnst = graph->FindOrCreateConstant(0);
        auto select = graph->CreateInstSelect(inst, ConditionCode::CC_LT);
        select->SetOperandsType(inst->GetType());
        select->SetInput(3U, zero_cnst);
        select_inst = select;
    }
    select_inst->SetInput(0U, add_inst);
    select_inst->SetInput(1U, input0);
    select_inst->SetInput(2U, input0);
    auto mask_value = ~static_cast<uint64_t>(value_minus_1);
    Inst *and_inst;
    if (graph->GetEncoder()->CanEncodeImmLogical(mask_value, size)) {
        and_inst = graph->CreateInstAndI(inst, mask_value);
    } else {
        auto mask = graph->FindOrCreateConstant(mask_value);
        and_inst = graph->CreateInstAnd(inst);
        and_inst->SetInput(1U, mask);
    }
    and_inst->SetInput(0U, select_inst);
    auto sub_inst = graph->CreateInstSub(inst);
    sub_inst->SetInput(0U, input0);
    sub_inst->SetInput(1U, and_inst);

    inst->InsertBefore(add_inst);
    inst->InsertBefore(select_inst);
    inst->InsertBefore(and_inst);
    InsertInstruction(inst, sub_inst);
}

void Lowering::ReplaceUnsignedModPowerOfTwo([[maybe_unused]] GraphVisitor *v, Inst *inst, uint64_t abs_value)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto value_minus_1 = abs_value - 1;
    uint32_t size = (inst->GetType() == DataType::UINT64 || inst->GetType() == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    Inst *and_inst;
    if (graph->GetEncoder()->CanEncodeImmLogical(value_minus_1, size)) {
        and_inst = graph->CreateInstAndI(inst, value_minus_1);
    } else {
        auto value_minus_1_cnst = graph->FindOrCreateConstant(value_minus_1);
        and_inst = graph->CreateInstAnd(inst);
        and_inst->SetInput(1U, value_minus_1_cnst);
    }
    auto input0 = inst->GetInput(0).GetInst();
    and_inst->SetInput(0U, input0);
    InsertInstruction(inst, and_inst);
}

void Lowering::VisitMod([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    if (TryReplaceModPowerOfTwo(v, inst)) {
        return;
    }
    LowerMulDivMod<Opcode::Mod>(inst);
}

void Lowering::VisitNeg([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto new_inst = LowerNegateMultiply(inst);
    LowerUnaryOperationWithShiftedOperand<Opcode::Neg>(new_inst == nullptr ? inst : new_inst);
}

void Lowering::VisitDeoptimizeIf([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    LowerToDeoptimizeCompare(inst);
}

void Lowering::VisitLoadFromConstantPool([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto new_inst = graph->CreateInstLoadArrayI(DataType::ANY, inst->GetPc());
    new_inst->SetInput(0, inst->GetInput(0).GetInst());
    new_inst->SetImm(inst->CastToLoadFromConstantPool()->GetTypeId());
    inst->ReplaceUsers(new_inst);
    inst->RemoveInputs();
    inst->GetBasicBlock()->ReplaceInst(inst, new_inst);
    graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

// Replacing Compare EQ with Xor
// 1.i64 Const 0
// 2.b   ...
// 3.b   Compare EQ b v2, v1
// ===>
// 1.i64 Const 1
// 2.b   ...
// 3.i32 Xor v1, v2
void Lowering::VisitCompare(GraphVisitor *v, Inst *inst)
{
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();

    if (inst->CastToCompare()->GetCc() != ConditionCode::CC_EQ) {
        return;
    }

    // Compare EQ b 0x0, v2
    if (input1->GetType() == DataType::BOOL && input0->IsConst() && input0->CastToConstant()->GetIntValue() == 0U) {
        std::swap(input0, input1);
    }

    // Compare EQ b v2, 0x0
    bool is_applicable =
        input0->GetType() == DataType::BOOL && input1->IsConst() && input1->CastToConstant()->GetIntValue() == 0U;
    if (!is_applicable) {
        return;
    }
    // Always there are more than one user of Compare, because previous pass is Cleanup
    bool only_ifimm = true;
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::IfImm) {
            only_ifimm = false;
            break;
        }
    }
    // Skip optimization, if all users is IfImm, optimization Compare+IfImm will be better
    if (only_ifimm) {
        return;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto cnst = graph->FindOrCreateConstant(1);
    auto xor_inst = graph->CreateInstXor();
    xor_inst->SetPc(inst->GetPc());
    xor_inst->SetType(DataType::BOOL);
    xor_inst->SetInput(0, input0);
    xor_inst->SetInput(1, cnst);

    InsertInstruction(inst, xor_inst);
    static_cast<Lowering *>(v)->VisitXor(v, xor_inst);
}

template <size_t MAX_OPERANDS>
void Lowering::SetInputsAndInsertInstruction(OperandsCapture<MAX_OPERANDS> &operands, Inst *inst, Inst *new_inst)
{
    for (size_t idx = 0; idx < MAX_OPERANDS; idx++) {
        new_inst->SetInput(idx, operands.Get(idx));
    }
    InsertInstruction(inst, new_inst);
}

void Lowering::LowerShift(Inst *inst)
{
    Opcode opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Shr || opc == Opcode::AShr || opc == Opcode::Shl);
    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return;
    }
    ASSERT(pred->GetOpcode() == Opcode::Constant);
    uint64_t val = (static_cast<const ConstantInst *>(pred))->GetIntValue();
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    if (val >= size) {
        return;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->GetEncoder()->CanEncodeShift(size)) {
        return;
    }

    Inst *new_inst;
    if (opc == Opcode::Shr) {
        new_inst = graph->CreateInstShrI(inst, val);
    } else if (opc == Opcode::AShr) {
        new_inst = graph->CreateInstAShrI(inst, val);
    } else {
        new_inst = graph->CreateInstShlI(inst, val);
    }

    static_cast<BinaryImmOperation *>(new_inst)->SetImm(val);

    new_inst->SetInput(0, inst->GetInput(0).GetInst());
    InsertInstruction(inst, new_inst);
}

constexpr Opcode Lowering::GetInstructionWithShiftedOperand(Opcode opcode)
{
    switch (opcode) {
        case Opcode::Add:
            return Opcode::AddSR;
        case Opcode::Sub:
            return Opcode::SubSR;
        case Opcode::And:
            return Opcode::AndSR;
        case Opcode::Or:
            return Opcode::OrSR;
        case Opcode::Xor:
            return Opcode::XorSR;
        case Opcode::AndNot:
            return Opcode::AndNotSR;
        case Opcode::OrNot:
            return Opcode::OrNotSR;
        case Opcode::XorNot:
            return Opcode::XorNotSR;
        case Opcode::Neg:
            return Opcode::NegSR;
        default:
            UNREACHABLE();
    }
}

constexpr Opcode Lowering::GetInstructionWithInvertedOperand(Opcode opcode)
{
    switch (opcode) {
        case Opcode::And:
            return Opcode::AndNot;
        case Opcode::Or:
            return Opcode::OrNot;
        case Opcode::Xor:
            return Opcode::XorNot;
        default:
            return Opcode::INVALID;
    }
}

ShiftType Lowering::GetShiftTypeByOpcode(Opcode opcode)
{
    switch (opcode) {
        case Opcode::Shl:
        case Opcode::ShlI:
            return ShiftType::LSL;
        case Opcode::Shr:
        case Opcode::ShrI:
            return ShiftType::LSR;
        case Opcode::AShr:
        case Opcode::AShrI:
            return ShiftType::ASR;
        default:
            UNREACHABLE();
    }
}

Inst *Lowering::GetCheckInstAndGetConstInput(Inst *inst)
{
    DataType::Type type = inst->GetType();
    if (type != DataType::INT64 && type != DataType::UINT64 && type != DataType::INT32 && type != DataType::UINT32 &&
        type != DataType::POINTER && type != DataType::BOOL) {
        return nullptr;
    }

    auto cnst = inst->GetInput(1).GetInst();
    if (!cnst->IsConst()) {
        if (!inst->IsCommutative() || !inst->GetInput(0).GetInst()->IsConst()) {
            return nullptr;
        }
        ASSERT(!DataType::IsFloatType(inst->GetType()));
        auto input = cnst;
        cnst = inst->GetInput(0).GetInst();
        inst->SetInput(0, input);
        inst->SetInput(1, cnst);
    }
    ASSERT(cnst->GetOpcode() == Opcode::Constant);
    return cnst;
}

ShiftOpcode Lowering::ConvertOpcode(Opcode new_opcode)
{
    switch (new_opcode) {
        case Opcode::NegSR:
            return ShiftOpcode::NEG_SR;
        case Opcode::AddSR:
            return ShiftOpcode::ADD_SR;
        case Opcode::SubSR:
            return ShiftOpcode::SUB_SR;
        case Opcode::AndSR:
            return ShiftOpcode::AND_SR;
        case Opcode::OrSR:
            return ShiftOpcode::OR_SR;
        case Opcode::XorSR:
            return ShiftOpcode::XOR_SR;
        case Opcode::AndNotSR:
            return ShiftOpcode::AND_NOT_SR;
        case Opcode::OrNotSR:
            return ShiftOpcode::OR_NOT_SR;
        case Opcode::XorNotSR:
            return ShiftOpcode::XOR_NOT_SR;
        default:
            return ShiftOpcode::INVALID_SR;
    }
}

// Ask encoder whether Constant can be an immediate for Compare
bool Lowering::ConstantFitsCompareImm(Inst *cst, uint32_t size, ConditionCode cc)
{
    ASSERT(cst->GetOpcode() == Opcode::Constant);
    if (DataType::IsFloatType(cst->GetType())) {
        return false;
    }
    auto *graph = cst->GetBasicBlock()->GetGraph();
    auto *encoder = graph->GetEncoder();
    int64_t val = cst->CastToConstant()->GetRawValue();
    if (graph->IsBytecodeOptimizer()) {
        return (size == HALF_SIZE) && (val == 0);
    }
    if (cc == ConditionCode::CC_TST_EQ || cc == ConditionCode::CC_TST_NE) {
        return encoder->CanEncodeImmLogical(val, size);
    }
    return encoder->CanEncodeImmAddSubCmp(val, size, IsSignedConditionCode(cc));
}

Inst *Lowering::LowerAddSub(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add || inst->GetOpcode() == Opcode::Sub);
    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return nullptr;
    }

    ASSERT(pred->GetOpcode() == Opcode::Constant);

    auto graph = pred->GetBasicBlock()->GetGraph();
    int64_t val = pred->CastToConstant()->GetIntValue();
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    if (!graph->GetEncoder()->CanEncodeImmAddSubCmp(val, size, false)) {
        return nullptr;
    }

    bool is_add = (inst->GetOpcode() == Opcode::Add);
    if (val < 0 && graph->GetEncoder()->CanEncodeImmAddSubCmp(-val, size, false)) {
        val = -val;
        is_add = !is_add;
    }

    Inst *new_inst;
    if (is_add) {
        new_inst = graph->CreateInstAddI(inst, static_cast<uint64_t>(val));
    } else {
        new_inst = graph->CreateInstSubI(inst, static_cast<uint64_t>(val));
    }
    new_inst->SetInput(0, inst->GetInput(0).GetInst());
    InsertInstruction(inst, new_inst);
    return new_inst;
}

template <Opcode OPCODE>
void Lowering::LowerMulDivMod(Inst *inst)
{
    ASSERT(inst->GetOpcode() == OPCODE);
    auto graph = inst->GetBasicBlock()->GetGraph();

    if (graph->IsInstThrowable(inst)) {
        return;
    }

    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return;
    }

    int64_t val = pred->CastToConstant()->GetIntValue();
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    if (!graph->GetEncoder()->CanEncodeImmMulDivMod(val, size)) {
        return;
    }

    Inst *new_inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements, bugprone-branch-clone)
    if constexpr (OPCODE == Opcode::Mul) {
        new_inst = graph->CreateInstMulI(inst, static_cast<uint64_t>(val));
        // NOLINTNEXTLINE(readability-misleading-indentation,readability-braces-around-statements)
    } else if constexpr (OPCODE == Opcode::Div) {
        new_inst = graph->CreateInstDivI(inst, static_cast<uint64_t>(val));
        if (graph->IsBytecodeOptimizer()) {
            inst->ClearFlag(compiler::inst_flags::NO_DCE);  // In Bytecode Optimizer Div may have NO_DCE flag
            if (val == 0) {
                new_inst->SetFlag(compiler::inst_flags::NO_DCE);
            }
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        new_inst = graph->CreateInstModI(inst, static_cast<uint64_t>(val));
        if (graph->IsBytecodeOptimizer()) {
            inst->ClearFlag(compiler::inst_flags::NO_DCE);  // In Bytecode Optimizer Div may have NO_DCE flag
            if (val == 0) {
                new_inst->SetFlag(compiler::inst_flags::NO_DCE);
            }
        }
    }
    new_inst->SetInput(0, inst->GetInput(0).GetInst());
    InsertInstruction(inst, new_inst);
}

Inst *Lowering::LowerMultiplyAddSub(Inst *inst)
{
    // Don't use MAdd/MSub for floating point inputs to avoid different results for interpreted and
    // compiled code due to better precision of target instructions implementing MAdd/MSub.
    if (DataType::GetCommonType(inst->GetType()) != DataType::INT64) {
        return nullptr;
    }

    OperandsCapture<3U> operands {};
    InstructionsCapture<2U> insts {};
    InstructionsCapture<3U> insts_sub3 {};
    bool is_sub = true;

    // clang-format off
    using MAddMatcher = ADD<MUL<SRC0, SRC1, Flags::S>, SRC2>;
    using MSubMatcher2Ops =
        AnyOf<SUB<SRC2, MUL<SRC0, SRC1, Flags::S>>,
              ADD<BinaryOp<Opcode::MNeg, SRC0, SRC1, Flags::S>, SRC2>>;
    using MSubMatcher3Ops =
        AnyOf<ADD<MUL<NEG<SRC0, Flags::S>, SRC1, Flags::S>, SRC2>,
              ADD<NEG<MUL<SRC0, SRC1, Flags::S>, Flags::S>, SRC2>>;
    // clang-format on

    if (MSubMatcher2Ops::Capture(inst, operands, insts)) {
        // Operands may have different types (but the same common type), but instructions
        // having different types can not be fused, because it will change semantics.
        if (!insts.HaveSameType()) {
            return nullptr;
        }
    } else if (MSubMatcher3Ops::Capture(inst, operands, insts_sub3)) {
        if (!insts_sub3.HaveSameType()) {
            return nullptr;
        }
    } else if (MAddMatcher::Capture(inst, operands, insts.ResetIndex())) {
        is_sub = false;
        if (!insts.HaveSameType()) {
            return nullptr;
        }
    } else {
        return nullptr;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();
    if ((is_sub && !encoder->CanEncodeMSub()) || (!is_sub && !encoder->CanEncodeMAdd())) {
        return nullptr;
    }

    Inst *new_inst = is_sub ? graph->CreateInstMSub(inst) : graph->CreateInstMAdd(inst);
    SetInputsAndInsertInstruction(operands, inst, new_inst);
    return new_inst;
}

Inst *Lowering::LowerNegateMultiply(Inst *inst)
{
    OperandsCapture<2U> operands {};
    InstructionsCapture<2U> insts {};
    using MNegMatcher = AnyOf<NEG<MUL<SRC0, SRC1, Flags::S>>, MUL<NEG<SRC0, Flags::S>, SRC1>>;
    if (!MNegMatcher::Capture(inst, operands, insts) || !operands.HaveCommonType() || !insts.HaveSameType()) {
        return nullptr;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->GetEncoder()->CanEncodeMNeg()) {
        return nullptr;
    }

    Inst *new_inst = graph->CreateInstMNeg(inst);
    SetInputsAndInsertInstruction(operands, inst, new_inst);
    return new_inst;
}

bool Lowering::LowerCastValueToAnyTypeWithConst(Inst *inst)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto any_type = inst->CastToCastValueToAnyType()->GetAnyType();
    auto base_type = AnyBaseTypeToDataType(any_type);
    if (!IsTypeNumeric(base_type) || base_type == DataType::POINTER) {
        return false;
    }
    auto input_inst = inst->GetInput(0).GetInst();
    if (!input_inst->IsConst()) {
        return false;
    }
    auto imm = input_inst->CastToConstant()->GetRawValue();
    auto pack_imm = graph->GetRuntime()->GetPackConstantByPrimitiveType(any_type, imm);
    auto any_const = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(DataType::Any(pack_imm));
    inst->ReplaceUsers(any_const);
    return true;
}

void Lowering::LowerLogicWithInvertedOperand(Inst *inst)
{
    OperandsCapture<2U> operands {};
    InstructionsCapture<2U> insts {};
    using Matcher = AnyOf<BinaryOp<Opcode::Or, SRC0, UnaryOp<Opcode::Not, SRC1, Flags::S>, Flags::C>,
                          BinaryOp<Opcode::And, SRC0, UnaryOp<Opcode::Not, SRC1, Flags::S>, Flags::C>,
                          BinaryOp<Opcode::Xor, SRC0, UnaryOp<Opcode::Not, SRC1, Flags::S>, Flags::C>>;
    if (!Matcher::Capture(inst, operands, insts) || !insts.HaveSameType()) {
        return;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();
    auto opcode = inst->GetOpcode();
    Inst *new_inst;
    if (opcode == Opcode::Or) {
        if (!encoder->CanEncodeOrNot()) {
            return;
        }
        new_inst = graph->CreateInstOrNot(inst);
    } else if (opcode == Opcode::And) {
        if (!encoder->CanEncodeAndNot()) {
            return;
        }
        new_inst = graph->CreateInstAndNot(inst);
    } else {
        if (!encoder->CanEncodeXorNot()) {
            return;
        }
        new_inst = graph->CreateInstXorNot(inst);
    }

    SetInputsAndInsertInstruction(operands, inst, new_inst);
}

template <typename T, size_t MAX_OPERANDS>
Inst *Lowering::LowerOperationWithShiftedOperand(Inst *inst, OperandsCapture<MAX_OPERANDS> &operands, Inst *shift_inst,
                                                 Opcode new_opcode)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto encoder = graph->GetEncoder();

    ShiftType shift_type = GetShiftTypeByOpcode(shift_inst->GetOpcode());
    if (!encoder->CanEncodeShiftedOperand(ConvertOpcode(new_opcode), shift_type)) {
        return nullptr;
    }
    uint64_t imm = static_cast<BinaryImmOperation *>(shift_inst)->GetImm();
    auto new_inst = static_cast<T *>(graph->CreateInst(new_opcode));
    new_inst->SetType(inst->GetType());
    new_inst->SetPc(inst->GetPc());
    new_inst->SetImm(imm);
    new_inst->SetShiftType(shift_type);
    SetInputsAndInsertInstruction(operands, inst, new_inst);
    return new_inst;
}

template <Opcode OPCODE, bool IS_COMMUTATIVE>
Inst *Lowering::LowerBinaryOperationWithShiftedOperand(Inst *inst)
{
    OperandsCapture<2U> operands {};
    InstructionsCapture<2U> insts {};
    InstructionsCapture<3U> inv_insts {};
    constexpr auto FLAGS = IS_COMMUTATIVE ? Flags::COMMUTATIVE : Flags::NONE;
    // constexpr auto OPCODE = OPCODE;  // NOLINT(readability-magic-numbers)

    // We're expecting that at this point all "shift by immediate" patterns were replaced with ShlI/ShrI/AShrI
    // clang-format off
    using Matcher = AnyOf<BinaryOp<OPCODE, SRC0, SHLI<SRC1>, FLAGS>,
                          BinaryOp<OPCODE, SRC0, SHRI<SRC1>, FLAGS>,
                          BinaryOp<OPCODE, SRC0, ASHRI<SRC1>, FLAGS>>;
    // Instead of replacing instruction having inverted operand with single inverted-operand instruction
    // and then applying the rules defined above we're applying explicitly defined rules for such patterns,
    // because after inverted-operand instruction insertion there will be several users for shift operation.
    // BinaryOp won't match the IR-tree with a pattern and either more complicated checks should be introduced there
    // or DCE pass followed by additional Lowering pass should be performed.
    // To keep things simple and avoid extra Lowering passes explicit rules were added.
    using InvertedOperandMatcher = MatchIf<GetInstructionWithInvertedOperand(OPCODE) != Opcode::INVALID,
                                         AnyOf<BinaryOp<OPCODE, SRC0, NOT<SHLI<SRC1>>>,
                                         BinaryOp<OPCODE, SRC0, NOT<SHRI<SRC1>>>,
                                         BinaryOp<OPCODE, SRC0, NOT<ASHRI<SRC1>>>>>;
    // clang-format on

    if (GetCommonType(inst->GetType()) != DataType::INT64) {
        return nullptr;
    }

    Inst *shift_inst;
    Opcode new_opc;

    if (InvertedOperandMatcher::Capture(inst, operands, inv_insts) && inv_insts.HaveSameType()) {
        auto right_operand =
            operands.Get(0) == inst->GetInput(0).GetInst() ? inst->GetInput(1).GetInst() : inst->GetInput(0).GetInst();
        shift_inst = right_operand->GetInput(0).GetInst();
        new_opc = GetInstructionWithShiftedOperand(GetInstructionWithInvertedOperand(OPCODE));
    } else if (Matcher::Capture(inst, operands, insts) && insts.HaveSameType()) {
        shift_inst =
            operands.Get(0) == inst->GetInput(0).GetInst() ? inst->GetInput(1).GetInst() : inst->GetInput(0).GetInst();
        new_opc = GetInstructionWithShiftedOperand(OPCODE);
    } else {
        return nullptr;
    }

    return LowerOperationWithShiftedOperand<BinaryShiftedRegisterOperation>(inst, operands, shift_inst, new_opc);
}

template <Opcode OPCODE>
void Lowering::LowerUnaryOperationWithShiftedOperand(Inst *inst)
{
    OperandsCapture<1> operands {};
    InstructionsCapture<2U> insts {};
    // constexpr auto OPCODE = OPCODE;  // NOLINT(readability-magic-numbers)
    // We're expecting that at this point all "shift by immediate" patterns were replaced with ShlI/ShrI/AShrI
    // clang-format off
    using Matcher = AnyOf<UnaryOp<OPCODE, SHLI<SRC0>>,
                          UnaryOp<OPCODE, SHRI<SRC0>>,
                          UnaryOp<OPCODE, ASHRI<SRC0>>>;
    // clang-format on
    if (!Matcher::Capture(inst, operands, insts) || GetCommonType(inst->GetType()) != DataType::INT64 ||
        !insts.HaveSameType()) {
        return;
    }
    LowerOperationWithShiftedOperand<UnaryShiftedRegisterOperation>(inst, operands, inst->GetInput(0).GetInst(),
                                                                    GetInstructionWithShiftedOperand(OPCODE));
}

Inst *Lowering::LowerLogic(Inst *inst)
{
    Opcode opc = inst->GetOpcode();
    ASSERT(opc == Opcode::Or || opc == Opcode::And || opc == Opcode::Xor);
    auto pred = GetCheckInstAndGetConstInput(inst);
    if (pred == nullptr) {
        return nullptr;
    }
    ASSERT(pred->GetOpcode() == Opcode::Constant);
    uint64_t val = pred->CastToConstant()->GetIntValue();
    DataType::Type type = inst->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!graph->GetEncoder()->CanEncodeImmLogical(val, size)) {
        return nullptr;
    }
    Inst *new_inst;
    if (opc == Opcode::Or) {
        new_inst = graph->CreateInstOrI(inst, val);
    } else if (opc == Opcode::And) {
        new_inst = graph->CreateInstAndI(inst, val);
    } else {
        new_inst = graph->CreateInstXorI(inst, val);
    }

    static_cast<BinaryImmOperation *>(new_inst)->SetImm(val);

    new_inst->SetInput(0, inst->GetInput(0).GetInst());
    InsertInstruction(inst, new_inst);
    return new_inst;
}

// From
//  2.u64 ShlI v1, 0x3 -> (v3)
//  3.u64 Load v0, v2  -> (...)
// To
//  3.u64 Load v0, v2, Scale 0x3 -> (...)
void Lowering::LowerMemInstScale(Inst *inst)
{
    auto opcode = inst->GetOpcode();
    ASSERT(opcode == Opcode::Load || opcode == Opcode::Store);
    auto input_inst = inst->GetInput(1).GetInst();
    if (input_inst->GetOpcode() != Opcode::ShlI) {
        return;
    }
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto input_type = input_inst->GetType();
    if (Is64BitsArch(graph->GetArch())) {
        if (input_type != DataType::UINT64 && input_type != DataType::INT64) {
            return;
        }
    } else {
        if (input_type != DataType::UINT32 && input_type != DataType::INT32) {
            return;
        }
    }
    auto type = inst->GetType();
    uint64_t val = input_inst->CastToShlI()->GetImm();
    uint32_t size = DataType::GetTypeSize(type, graph->GetArch());
    if (!graph->GetEncoder()->CanEncodeScale(val, size)) {
        return;
    }
    if (opcode == Opcode::Load) {
        ASSERT(inst->CastToLoad()->GetScale() == 0);
        inst->CastToLoad()->SetScale(val);
    } else {
        ASSERT(inst->CastToStore()->GetScale() == 0);
        inst->CastToStore()->SetScale(val);
    }
    inst->SetInput(1, input_inst->GetInput(0).GetInst());
    graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

template <typename LowLevelType>
void Lowering::LowerConstArrayIndex(Inst *inst, Opcode low_level_opcode)
{
    if (inst->GetBasicBlock()->GetGraph()->IsBytecodeOptimizer()) {
        return;
    }
    static constexpr size_t ARRAY_INDEX_INPUT = 1;
    auto input_inst = inst->GetInput(ARRAY_INDEX_INPUT).GetInst();
    ASSERT(input_inst->GetOpcode() != Opcode::BoundsCheckI);
    if (input_inst->IsConst()) {
        uint64_t value = input_inst->CastToConstant()->GetIntValue();

        auto graph = inst->GetBasicBlock()->GetGraph();
        auto new_inst = graph->CreateInst(low_level_opcode);
        new_inst->SetType(inst->GetType());
        new_inst->SetPc(inst->GetPc());
        static_cast<LowLevelType *>(new_inst)->SetImm(value);

        // StoreInst and BoundsCheckInst have 3 inputs, LoadInst - has 2 inputs
        new_inst->SetInput(0, inst->GetInput(0).GetInst());
        if (inst->GetInputsCount() == 3U) {
            new_inst->SetInput(1, inst->GetInput(2U).GetInst());
        } else {
            ASSERT(inst->GetInputsCount() == 2U);
        }
        if (inst->GetOpcode() == Opcode::StoreArray) {
            new_inst->CastToStoreArrayI()->SetNeedBarrier(inst->CastToStoreArray()->GetNeedBarrier());
        }

        if (inst->GetOpcode() == Opcode::LoadArray) {
            new_inst->CastToLoadArrayI()->SetNeedBarrier(inst->CastToLoadArray()->GetNeedBarrier());
            new_inst->CastToLoadArrayI()->SetIsArray(inst->CastToLoadArray()->IsArray());
        }
        if (inst->GetOpcode() == Opcode::BoundsCheck) {
            new_inst->CastToBoundsCheckI()->SetIsArray(inst->CastToBoundsCheck()->IsArray());
            if (inst->CanDeoptimize()) {
                new_inst->SetFlag(inst_flags::CAN_DEOPTIMIZE);
            }
        }

        // Replace instruction immediately because it's not removable by DCE
        if (inst->GetOpcode() != Opcode::BoundsCheck) {
            inst->ReplaceUsers(new_inst);
        } else {
            auto cnst = graph->FindOrCreateConstant(value);
            inst->ReplaceUsers(cnst);
        }
        inst->RemoveInputs();
        inst->GetBasicBlock()->ReplaceInst(inst, new_inst);
        graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
    }
}

void Lowering::LowerStateInst(SaveStateInst *save_state)
{
    size_t idx = 0;
    size_t inputs_count = save_state->GetInputsCount();
    auto graph = save_state->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    bool skip_floats = (graph->GetArch() == Arch::AARCH32);
    while (idx < inputs_count) {
        auto input_inst = save_state->GetInput(idx).GetInst();
        // In Aarch32 floats values stores in different format then integer
        if (input_inst->GetOpcode() == Opcode::NullPtr ||
            (input_inst->IsConst() && (!skip_floats || input_inst->GetType() == DataType::INT64))) {
            uint64_t raw_value =
                input_inst->GetOpcode() == Opcode::NullPtr ? 0 : input_inst->CastToConstant()->GetRawValue();
            auto vreg = save_state->GetVirtualRegister(idx);
            auto type = input_inst->GetType();
            // There are no INT64 in dynamic
            if (type == DataType::INT64 && graph->IsDynamicMethod()) {
                type = DataType::INT32;
            }
            save_state->AppendImmediate(raw_value, vreg.Value(), type, vreg.GetVRegType());
            save_state->RemoveInput(idx);
            inputs_count--;
            graph->GetEventWriter().EventLowering(GetOpcodeString(save_state->GetOpcode()), save_state->GetId(),
                                                  save_state->GetPc());
            COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(save_state->GetOpcode());
        } else {
            idx++;
        }
    }
}

void Lowering::LowerReturnInst(FixedInputsInst1 *ret)
{
    auto graph = ret->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer()) {
        return;
    }
    ASSERT(ret->GetOpcode() == Opcode::Return);
    auto input_inst = ret->GetInput(0).GetInst();
    if (input_inst->IsConst()) {
        uint64_t raw_value = input_inst->CastToConstant()->GetRawValue();
        auto ret_imm = graph->CreateInstReturnI(ret->GetType(), ret->GetPc(), raw_value);

        // Replace instruction immediately because it's not removable by DCE
        ret->RemoveInputs();
        ret->GetBasicBlock()->ReplaceInst(ret, ret_imm);
        graph->GetEventWriter().EventLowering(GetOpcodeString(ret->GetOpcode()), ret->GetId(), ret->GetPc());
        COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(ret->GetOpcode());
    }
}

// We'd like to swap only to make second operand immediate
bool Lowering::BetterToSwapCompareInputs(Inst *cmp)
{
    ASSERT(cmp->GetOpcode() == Opcode::Compare);
    auto in0 = cmp->GetInput(0).GetInst();
    auto in1 = cmp->GetInput(1).GetInst();
    if (DataType::IsFloatType(in0->GetType())) {
        return false;
    }
    if (in0->GetOpcode() == compiler::Opcode::NullPtr) {
        return true;
    }

    if (in0->IsConst()) {
        if (in1->IsConst()) {
            DataType::Type type = cmp->CastToCompare()->GetOperandsType();
            uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
            auto cc = cmp->CastToCompare()->GetCc();
            return ConstantFitsCompareImm(in0, size, cc) && !ConstantFitsCompareImm(in1, size, cc);
        }
        return true;
    }
    return false;
}

// Optimize order of input arguments for decreasing using accumulator (Bytecodeoptimizer only).
void Lowering::OptimizeIfInput(compiler::Inst *if_inst)
{
    ASSERT(if_inst->GetOpcode() == compiler::Opcode::If);
    compiler::Inst *input_0 = if_inst->GetInput(0).GetInst();
    compiler::Inst *input_1 = if_inst->GetInput(1).GetInst();

    if (input_0->IsDominate(input_1)) {
        if_inst->SetInput(0, input_1);
        if_inst->SetInput(1, input_0);
        // And change CC
        auto cc = if_inst->CastToIf()->GetCc();
        cc = SwapOperandsConditionCode(cc);
        if_inst->CastToIf()->SetCc(cc);
    }
}

void Lowering::JoinFcmpInst(IfImmInst *inst, CmpInst *input)
{
    auto cc = inst->GetCc();
    ASSERT(cc == ConditionCode::CC_EQ || cc == ConditionCode::CC_NE || IsSignedConditionCode(cc));
    if (input->IsFcmpg()) {
        /* Please look at the table of vector condition flags:
         * LT => Less than, or unordered
         * LE => Less than or equal, or unordered
         * GT => Greater than
         * GE => Greater than or equal
         *
         * LO => Less than
         * LS => Less than or equal
         * HI => Greater than, or unordered
         * HS => Greater than or equal, or unordered
         *
         * So we change condition to "unsigned" for Fcmpg (which should return "greater than" for unordered
         * comparisons).
         */
        cc = InverseSignednessConditionCode(cc);
    }

    // New instruction
    auto graph = input->GetBasicBlock()->GetGraph();
    auto replace = graph->CreateInstIf(DataType::NO_TYPE, inst->GetPc(), cc);
    replace->SetMethod(inst->GetMethod());
    replace->SetOperandsType(input->GetOperandsType());
    replace->SetInput(0, input->GetInput(0).GetInst());
    replace->SetInput(1, input->GetInput(1).GetInst());

    // Replace IfImm instruction immediately because it's not removable by DCE
    inst->RemoveInputs();
    inst->GetBasicBlock()->ReplaceInst(inst, replace);
    graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
    if (graph->IsBytecodeOptimizer()) {
        OptimizeIfInput(replace);
    }
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

void Lowering::LowerIf(IfImmInst *inst)
{
    ASSERT(inst->GetCc() == ConditionCode::CC_NE || inst->GetCc() == ConditionCode::CC_EQ);
    ASSERT(inst->GetImm() == 0);
    if (inst->GetOperandsType() != DataType::BOOL) {
        ASSERT(!GetGraph()->SupportManagedCode() || GetGraph()->IsDynamicMethod());
        return;
    }
    auto input = inst->GetInput(0).GetInst();
    if (input->GetOpcode() != Opcode::Compare && input->GetOpcode() != Opcode::And) {
        return;
    }
    // Check, that inst have only IfImm user
    for (auto &user : input->GetUsers()) {
        if (user.GetInst()->GetOpcode() != Opcode::IfImm) {
            return;
        }
    }
    // Try put constant in second input
    if (input->GetOpcode() == Opcode::Compare && BetterToSwapCompareInputs(input)) {
        // Swap inputs
        auto in0 = input->GetInput(0).GetInst();
        auto in1 = input->GetInput(1).GetInst();
        input->SetInput(0, in1);
        input->SetInput(1, in0);
        // And change CC
        auto cc = input->CastToCompare()->GetCc();
        cc = SwapOperandsConditionCode(cc);
        input->CastToCompare()->SetCc(cc);
    }
    if (!GetGraph()->IsBytecodeOptimizer()) {
        for (auto &new_input : input->GetInputs()) {
            auto real_new_input = input->GetDataFlowInput(new_input.GetInst());
            if (real_new_input->IsMovableObject()) {
                ssb_.SearchAndCreateMissingObjInSaveState(GetGraph(), real_new_input, inst);
            }
        }
    }
    auto cst = input->GetInput(1).GetInst();
    DataType::Type type =
        (input->GetOpcode() == Opcode::Compare) ? input->CastToCompare()->GetOperandsType() : input->GetType();
    uint32_t size = (type == DataType::UINT64 || type == DataType::INT64) ? WORD_SIZE : HALF_SIZE;
    auto cc = input->GetOpcode() == Opcode::Compare ? input->CastToCompare()->GetCc() : ConditionCode::CC_TST_NE;
    // IfImm can be inverted
    if (inst->GetCc() == ConditionCode::CC_EQ && inst->GetImm() == 0) {
        cc = GetInverseConditionCode(cc);
    }
    if (cst->GetOpcode() == compiler::Opcode::NullPtr || (cst->IsConst() && ConstantFitsCompareImm(cst, size, cc))) {
        // In-place change for IfImm
        InPlaceLowerIfImm(inst, input, cst, cc, type);
    } else {
        LowerIfImmToIf(inst, input, cc, type);
    }
}

void Lowering::InPlaceLowerIfImm(IfImmInst *inst, Inst *input, Inst *cst, ConditionCode cc, DataType::Type input_type)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    inst->SetOperandsType(input_type);
    auto new_input = input->GetInput(0).GetInst();
    // For compare(nullptr, 0) set `nullptr` as new input
    if (cst->GetOpcode() == Opcode::NullPtr && IsZeroConstant(new_input) &&
        DataType::IsReference(inst->GetOperandsType())) {
        new_input = cst;
    }
    inst->SetInput(0, new_input);

    uint64_t val = cst->GetOpcode() == Opcode::NullPtr ? 0 : cst->CastToConstant()->GetRawValue();
    inst->SetImm(val);
    inst->SetCc(cc);
    inst->GetBasicBlock()->GetGraph()->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(),
                                                                      inst->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());

    if (inst->GetImm() == 0 && new_input->GetOpcode() == Opcode::Cmp &&
        DataType::IsFloatType(new_input->CastToCmp()->GetOperandsType()) && !graph->IsBytecodeOptimizer()) {
        // Check inst and input are the only users of new_input
        bool join {true};
        for (auto &user : new_input->GetUsers()) {
            if (auto user_inst = user.GetInst(); user_inst != inst && user_inst != input) {
                join = false;
                break;
            }
        }
        if (join) {
            JoinFcmpInst(inst, new_input->CastToCmp());
        }
    }
}

void Lowering::LowerIfImmToIf(IfImmInst *inst, Inst *input, ConditionCode cc, DataType::Type input_type)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    // New instruction
    auto replace = graph->CreateInstIf(DataType::NO_TYPE, inst->GetPc(), cc);
    replace->SetMethod(inst->GetMethod());
    replace->SetOperandsType(input_type);
    replace->SetInput(0, input->GetInput(0).GetInst());
    replace->SetInput(1, input->GetInput(1).GetInst());
    // Replace IfImm instruction immediately because it's not removable by DCE
    inst->RemoveInputs();
    inst->GetBasicBlock()->ReplaceInst(inst, replace);
    graph->GetEventWriter().EventLowering(GetOpcodeString(inst->GetOpcode()), inst->GetId(), inst->GetPc());
    if (graph->IsBytecodeOptimizer()) {
        OptimizeIfInput(replace);
    }
    COMPILER_LOG(DEBUG, LOWERING) << "Lowering is applied for " << GetOpcodeString(inst->GetOpcode());
}

void Lowering::LowerToDeoptimizeCompare(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::DeoptimizeIf);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(!graph->IsBytecodeOptimizer());

    auto deopt_if = inst->CastToDeoptimizeIf();
    if (deopt_if->GetInput(0).GetInst()->GetOpcode() != Opcode::Compare) {
        return;
    }
    auto compare = deopt_if->GetInput(0).GetInst()->CastToCompare();
    if (!compare->HasSingleUser()) {
        return;
    }
    COMPILER_LOG(DEBUG, LOWERING) << __func__ << "\n" << *compare << "\n" << *deopt_if;
    auto cmp_inp1 = compare->GetInput(1).GetInst();
    DataType::Type type = compare->GetOperandsType();
    uint32_t size =
        (type == DataType::UINT64 || type == DataType::INT64 || type == DataType::ANY) ? WORD_SIZE : HALF_SIZE;
    Inst *deopt_cmp = nullptr;
    if ((cmp_inp1->IsConst() && ConstantFitsCompareImm(cmp_inp1, size, compare->GetCc())) || cmp_inp1->IsNullPtr()) {
        uint64_t imm = cmp_inp1->IsNullPtr() ? 0 : cmp_inp1->CastToConstant()->GetRawValue();
        deopt_cmp = graph->CreateInstDeoptimizeCompareImm(deopt_if, compare, imm);
    } else {
        deopt_cmp = graph->CreateInstDeoptimizeCompare(deopt_if, compare);
        deopt_cmp->SetInput(1, compare->GetInput(1).GetInst());
    }
    deopt_cmp->SetInput(0, compare->GetInput(0).GetInst());
    deopt_cmp->SetSaveState(deopt_if->GetSaveState());
    deopt_if->ReplaceUsers(deopt_cmp);
    deopt_if->GetBasicBlock()->InsertAfter(deopt_cmp, deopt_if);
    deopt_if->ClearFlag(compiler::inst_flags::NO_DCE);
    graph->GetEventWriter().EventLowering(GetOpcodeString(deopt_if->GetOpcode()), deopt_if->GetId(), deopt_if->GetPc());
    COMPILER_LOG(DEBUG, LOWERING) << "===>\n" << *deopt_cmp;
}

void Lowering::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool Lowering::RunImpl()
{
    VisitGraph();
    return true;
}
}  // namespace panda::compiler
