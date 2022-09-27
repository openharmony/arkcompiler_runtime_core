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

#ifndef PANDA_INST_BUILDER_INL_H
#define PANDA_INST_BUILDER_INL_H

#include "inst_builder.h"
#include "optimizer/code_generator/encode.h"

namespace panda::compiler {
// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode opcode>
void InstBuilder::BuildCall(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);

    if (GetRuntime()->IsMethodIntrinsic(GetMethod(), method_id)) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        BuildIntrinsic(bc_inst, is_range, acc_read);
        return;
    }
    auto pc = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, pc);

    auto has_implicit_arg = !GetRuntime()->IsMethodStatic(GetMethod(), method_id);

    decltype(graph_->CreateInstNullCheck()) null_check = nullptr;
    uint32_t class_id = 0;
    if (has_implicit_arg) {
        auto ref_ptr = GetArgDefinition(bc_inst, 0, acc_read);
        null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, pc);
        null_check->SetInput(0, ref_ptr);
        null_check->SetInput(1, save_state);
    } else if (method != nullptr) {
        if (graph_->IsAotMode()) {
            class_id = GetRuntime()->GetClassIdWithinFile(GetMethod(), GetRuntime()->GetClass(method));
        } else {
            class_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);
        }
    }

    decltype(graph_->CreateInstCallStatic()) inst;
    if (method == nullptr || (!has_implicit_arg && class_id == 0)) {
        // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
        inst = BuildUnresolvedCallInst<opcode>(method_id, pc);
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
        inst = BuildCallInst<opcode>(method, method_id, pc);
    }
    inst->SetCanNativeException(method == nullptr || GetRuntime()->HasNativeException(method));

    SetInputsForCallInst(bc_inst, is_range, acc_read, inst, null_check, method_id, has_implicit_arg, true);
    inst->AppendInput(save_state);
    inst->AddInputType(DataType::NO_TYPE);
    AddInstruction(save_state);
    if (has_implicit_arg) {
        AddInstruction(null_check);
    } else if (!inst->IsUnresolved()) {  // Unresolved does not know which class should be initialized
        BuildInitClassInstForCallStatic(method, class_id, pc, save_state);
    }
    AddInstruction(inst);
    if (inst->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(inst);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <typename T>
void InstBuilder::SetInputsForCallInst(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read, T *inst,
                                       Inst *null_check, uint32_t method_id, bool has_implicit_arg, bool need_savestate)
{
    // Take into account this argument for virtual call
    size_t hidden_args_count = has_implicit_arg ? 1 : 0;
    size_t args_count = GetMethodArgumentsCount(method_id);
    size_t total_args_count = hidden_args_count + args_count;

    inst->ReserveInputs(total_args_count + (need_savestate ? 1 : 0));
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), total_args_count + (need_savestate ? 1 : 0));
    if (has_implicit_arg) {
        inst->AppendInput(null_check);
        inst->AddInputType(DataType::REFERENCE);
    }

    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        // start reg for Virtual call was added
        if (has_implicit_arg) {
            ++start_reg;
        }
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            inst->AppendInput(GetDefinition(start_reg));
            inst->AddInputType(GetMethodArgumentType(method_id, i));
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            inst->AppendInput(GetArgDefinition(bc_inst, i + hidden_args_count, acc_read));
            inst->AddInputType(GetMethodArgumentType(method_id, i));
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitClassInstForCallStatic(RuntimeInterface::MethodPtr method, uint32_t class_id, size_t pc,
                                                  Inst *save_state)
{
    if (GetClassId() == class_id) {
        return;
    }

    auto init_class = graph_->CreateInstInitClass(DataType::NO_TYPE, pc);
    init_class->SetTypeId(class_id);
    init_class->SetMethod(GetGraph()->GetMethod());
    init_class->SetClass(GetRuntime()->GetClass(method));
    init_class->SetInput(0, save_state);
    AddInstruction(init_class);
}
// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode opcode>
CallInst *InstBuilder::BuildUnresolvedCallInst(uint32_t method_id, size_t pc)
{
    decltype(GetGraph()->CreateInstCallVirtual()) inst;
    UnresolvedTypesInterface::SlotKind slot_kind;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (opcode == Opcode::CallStatic) {
        inst = GetGraph()->CreateInstUnresolvedCallStatic(GetMethodReturnType(method_id), pc, method_id);
        inst->SetCallMethod(GetGraph()->GetMethod());
        slot_kind = UnresolvedTypesInterface::SlotKind::METHOD;
        // NOLINTNEXTLINE(readability-magic-numbers,readability-misleading-indentation)
    } else if constexpr (opcode == Opcode::CallVirtual) {
        inst = GetGraph()->CreateInstUnresolvedCallVirtual(GetMethodReturnType(method_id), pc, method_id);
        inst->SetCallMethod(GetGraph()->GetMethod());
        slot_kind = UnresolvedTypesInterface::SlotKind::VIRTUAL_METHOD;
    }
    if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
        GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), method_id, slot_kind);
    }
    return inst;
}
// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode opcode>
CallInst *InstBuilder::BuildCallInst(RuntimeInterface::MethodPtr method, uint32_t method_id, size_t pc)
{
    ASSERT(method != nullptr);
    decltype(GetGraph()->CreateInstCallVirtual()) inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (opcode == Opcode::CallStatic) {
        inst = GetGraph()->CreateInstCallStatic(GetMethodReturnType(method_id), pc, method_id);
        inst->SetCallMethod(method);
        // NOLINTNEXTLINE(readability-magic-numbers,readability-misleading-indentation)
    } else if constexpr (opcode == Opcode::CallVirtual) {
        inst = GetGraph()->CreateInstCallVirtual(GetMethodReturnType(method_id), pc, method_id);
        inst->SetCallMethod(method);
        if (GetRuntime()->IsInterfaceMethod(method)) {
            inst->SetFlag(inst_flags::IMPLICIT_RUNTIME_CALL);
        }
    }
    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildDynamicCall(const BytecodeInstruction *bc_inst, bool is_range)
{
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));

    auto null_check = graph_->CreateInstNullCheck(DataType::ANY, GetPc(bc_inst->GetAddress()));
    null_check->SetInput(0, GetDefinition(bc_inst->GetVReg(0)));
    null_check->SetInput(1, save_state);

    decltype(GetGraph()->CreateInstCallDynamic()) inst =
        GetGraph()->CreateInstCallDynamic(DataType::ANY, GetPc(bc_inst->GetAddress()));

    inst->SetCanNativeException(true);

    // Take into account this argument for virtual call
    size_t args_count = bc_inst->GetImm64() + 1;  // '+1' for function object

    inst->ReserveInputs(args_count + 1);  // '+1' for SaveState input
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), args_count + 1);
    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        // start reg for Virtual call was added
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            inst->AppendInput(GetDefinition(start_reg));
            inst->AddInputType(DataType::ANY);
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            inst->AppendInput(GetDefinition(bc_inst->GetVReg(i)));
            inst->AddInputType(DataType::ANY);
        }
    }
    inst->AppendInput(save_state);
    inst->AddInputType(DataType::NO_TYPE);
    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMonitor(const BytecodeInstruction *bc_inst, Inst *def, bool is_enter)
{
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto inst = GetGraph()->CreateInstMonitor(DataType::VOID, GetPc(bc_inst->GetAddress()));
    AddInstruction(save_state);
    if (!is_enter) {
        inst->CastToMonitor()->SetExit();
    } else {
        // Create NullCheck instruction
        auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
        null_check->SetInput(0, def);
        null_check->SetInput(1, save_state);
        def = null_check;
        AddInstruction(null_check);
    }
    inst->SetInput(0, def);
    inst->SetInput(1, save_state);

    AddInstruction(inst);
}

#include <intrinsics_ir_build.inl>

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildDefaultStaticIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);
    auto intrinsic_id = GetRuntime()->GetIntrinsicId(method);
    ASSERT(intrinsic_id != RuntimeInterface::IntrinsicId::COUNT);
    IntrinsicInst *inst;
    auto ret_type = GetMethodReturnType(method_id);
    inst = GetGraph()->CreateInstIntrinsic(ret_type, GetPc(bc_inst->GetAddress()), intrinsic_id);

    SetInputsForCallInst(bc_inst, is_range, acc_read, inst, nullptr, method_id, false, inst->RequireState());
    // Add SaveState if intrinsic may call runtime
    if (inst->RequireState()) {
        auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
        inst->AppendInput(save_state);
        inst->AddInputType(DataType::NO_TYPE);
        AddInstruction(save_state);
    }
    AddInstruction(inst);
    if (inst->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(inst);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
    if (NeedSafePointAfterIntrinsic(intrinsic_id)) {
        AddInstruction(CreateSafePoint(current_bb_));
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildAbsIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto inst = GetGraph()->CreateInstAbs(GetMethodReturnType(method_id), GetPc(bc_inst->GetAddress()));
    ASSERT(GetMethodArgumentsCount(method_id) == 1);
    inst->SetInput(0, GetArgDefinition(bc_inst, 0, acc_read));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

template <Opcode opcode>
static BinaryOperation *CreateBinaryOperation(Graph *graph, DataType::Type return_type, size_t pc) = delete;

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Min>(Graph *graph, DataType::Type return_type, size_t pc)
{
    return graph->CreateInstMin(return_type, pc);
}

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Max>(Graph *graph, DataType::Type return_type, size_t pc)
{
    return graph->CreateInstMax(return_type, pc);
}

template <Opcode opcode>
void InstBuilder::BuildBinaryOperationIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    [[maybe_unused]] auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    ASSERT(GetMethodArgumentsCount(method_id) == 2U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto inst = CreateBinaryOperation<opcode>(GetGraph(), GetMethodReturnType(method_id), GetPc(bc_inst->GetAddress()));
    inst->SetInput(0, GetArgDefinition(bc_inst, 0, acc_read));
    inst->SetInput(1, GetArgDefinition(bc_inst, 1, acc_read));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildSqrtIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    [[maybe_unused]] auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto inst = GetGraph()->CreateInstSqrt(GetMethodReturnType(method_id), GetPc(bc_inst->GetAddress()));
    ASSERT(GetMethodArgumentsCount(method_id) == 1);
    Inst *def = GetArgDefinition(bc_inst, 0, acc_read);
    inst->SetInput(0, def);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIsNanIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    // No need to create specialized node for isNaN. Since NaN != NaN, simple float compare node is fine.
    // Also, ensure that float comparison node is implemented for specific architecture
    auto inst = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), ConditionCode::CC_NE);
    auto vreg = GetArgDefinition(bc_inst, 0, acc_read);
    inst->SetInput(0, vreg);
    inst->SetInput(1, vreg);
    inst->SetOperandsType(GetMethodArgumentType(method_id, 0));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::GetArgDefinition(const BytecodeInstruction *bc_inst, size_t idx, bool acc_read)
{
    if (acc_read) {
        size_t acc_pos = bc_inst->GetImm64();
        if (idx < acc_pos) {
            return GetDefinition(bc_inst->GetVReg(idx));
        }
        if (acc_pos == idx) {
            return GetDefinitionAcc();
        }
        return GetDefinition(bc_inst->GetVReg(idx - 1));
    }
    return GetDefinition(bc_inst->GetVReg(idx));
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMonitorIntrinsic(const BytecodeInstruction *bc_inst, bool is_enter, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    [[maybe_unused]] auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    ASSERT(GetMethodReturnType(method_id) == DataType::VOID);
    ASSERT(GetMethodArgumentsCount(method_id) == 1);
    BuildMonitor(bc_inst, GetArgDefinition(bc_inst, 0, acc_read), is_enter);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);
    auto intrinsic_id = GetRuntime()->GetIntrinsicId(method);
    auto is_virtual = IsVirtual(intrinsic_id);
    if (!options.IsCompilerEncodeIntrinsics()) {
        BuildDefaultIntrinsic(is_virtual, bc_inst, is_range, acc_read);
        return;
    }
    if (!is_virtual) {
        return BuildStaticCallIntrinsic(bc_inst, is_range, acc_read);
    }
    return BuildVirtualCallIntrinsic(bc_inst, is_range, acc_read);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildDefaultIntrinsic(bool is_virtual, const BytecodeInstruction *bc_inst, bool is_range,
                                        bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);
    auto intrinsic_id = GetRuntime()->GetIntrinsicId(method);
    if (intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER ||
        intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT) {
        BuildMonitorIntrinsic(bc_inst, intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER,
                              acc_read);
        return;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if (!is_virtual) {
        BuildDefaultStaticIntrinsic(bc_inst, is_range, acc_read);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        BuildDefaultVirtualCallIntrinsic(bc_inst, is_range, acc_read);
    }
}

// do not specify reason for tidy suppression because comment does not fit single line
// NOLINTNEXTLINE
void InstBuilder::BuildStaticCallIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);
    auto intrinsic_id = GetRuntime()->GetIntrinsicId(method);
    switch (intrinsic_id) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER:
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT: {
            BuildMonitorIntrinsic(
                bc_inst, intrinsic_id == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER, acc_read);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F64: {
            BuildAbsIntrinsic(bc_inst, acc_read);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F64: {
            BuildSqrtIntrinsic(bc_inst, acc_read);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F64: {
            BuildBinaryOperationIntrinsic<Opcode::Min>(bc_inst, acc_read);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F64: {
            BuildBinaryOperationIntrinsic<Opcode::Max>(bc_inst, acc_read);
            break;
        }
#include "intrinsics_ir_build_static_call.inl"
        default: {
            BuildDefaultStaticIntrinsic(bc_inst, is_range, acc_read);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildDefaultVirtualCallIntrinsic(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);
    auto intrinsic_id = GetRuntime()->GetIntrinsicId(method);
    auto bc_addr = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, bc_addr);
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, bc_addr);
    null_check->SetInput(0, GetArgDefinition(bc_inst, 0, acc_read));
    null_check->SetInput(1, save_state);

    auto inst =
        GetGraph()->CreateInstIntrinsic(GetMethodReturnType(method_id), GetPc(bc_inst->GetAddress()), intrinsic_id);

    SetInputsForCallInst(bc_inst, is_range, acc_read, inst, null_check, method_id, true, true);

    inst->AppendInput(save_state);
    inst->AddInputType(DataType::NO_TYPE);
    AddInstruction(save_state);
    AddInstruction(null_check);

    /* if there are reference type args to be checked for NULL */
    AddArgNullcheckIfNeeded<true>(intrinsic_id, inst, save_state, bc_addr);

    AddInstruction(inst);
    if (inst->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(inst);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
    if (NeedSafePointAfterIntrinsic(intrinsic_id)) {
        AddInstruction(CreateSafePoint(current_bb_));
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool is_acc_write>
void InstBuilder::BuildLoadObject(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));

    // Create NullCheck instruction
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    null_check->SetInput(0, GetDefinition(bc_inst->GetVReg(is_acc_write ? 0 : 1)));
    null_check->SetInput(1, save_state);

    auto runtime = GetRuntime();
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = runtime->ResolveFieldIndex(GetMethod(), field_index);
    auto field = runtime->ResolveField(GetMethod(), field_id, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), field_id);
    }

    // Create LoadObject instruction
    Inst *inst;
    if (field == nullptr) {
        auto uload_object = graph_->CreateInstUnresolvedLoadObject(type, GetPc(bc_inst->GetAddress()));
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), field_id,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        uload_object->SetInput(0, null_check);
        uload_object->SetInput(1, save_state);
        uload_object->SetTypeId(field_id);
        uload_object->SetMethod(GetGraph()->GetMethod());
        inst = uload_object;
    } else {
        auto load_object = graph_->CreateInstLoadObject(type, GetPc(bc_inst->GetAddress()));
        load_object->SetInput(0, null_check);
        load_object->SetTypeId(field_id);
        load_object->SetMethod(GetGraph()->GetMethod());
        load_object->SetObjField(field);
        if (runtime->IsFieldVolatile(field)) {
            load_object->SetVolatile(true);
        }
        inst = load_object;
    }

    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(inst);

    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (is_acc_write) {
        UpdateDefinitionAcc(inst);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        UpdateDefinition(bc_inst->GetVReg(0), inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreObjectInst(const BytecodeInstruction *bc_inst, DataType::Type type,
                                        RuntimeInterface::FieldPtr field, size_t type_id)
{
    Inst *inst;
    if (field == nullptr) {
        auto ustore_object = graph_->CreateInstUnresolvedStoreObject(type, GetPc(bc_inst->GetAddress()));
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), type_id,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        ustore_object->SetTypeId(type_id);
        ustore_object->SetMethod(GetGraph()->GetMethod());
        if (type == DataType::REFERENCE) {
            ustore_object->SetNeedBarrier(true);
        }
        inst = ustore_object;
    } else {
        auto store_object = graph_->CreateInstStoreObject(type, GetPc(bc_inst->GetAddress()));
        store_object->SetTypeId(type_id);
        store_object->SetMethod(GetGraph()->GetMethod());
        store_object->SetObjField(field);
        if (GetRuntime()->IsFieldVolatile(field)) {
            store_object->SetVolatile(true);
        }
        if (type == DataType::REFERENCE) {
            store_object->SetNeedBarrier(true);
        }
        inst = store_object;
    }
    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool is_acc_read>
void InstBuilder::BuildStoreObject(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));

    // Create NullCheck instruction
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    null_check->SetInput(0, GetDefinition(bc_inst->GetVReg(is_acc_read ? 0 : 1)));
    null_check->SetInput(1, save_state);

    auto runtime = GetRuntime();
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = runtime->ResolveFieldIndex(GetMethod(), field_index);
    auto field = runtime->ResolveField(GetMethod(), field_id, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), field_id);
    }

    // Create StoreObject instruction
    Inst *inst = BuildStoreObjectInst(bc_inst, type, field, field_id);
    inst->SetInput(0, null_check);
    if (field == nullptr) {
        inst->SetInput(INPUT_2, save_state);
    }
    AddInstruction(save_state);
    AddInstruction(null_check);

    Inst *store_val = nullptr;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (is_acc_read) {
        store_val = GetDefinitionAcc();
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        store_val = GetDefinition(bc_inst->GetVReg(0));
    }

    inst->SetInput(1, store_val);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadStaticInst(const BytecodeInstruction *bc_inst, DataType::Type type, size_t type_id,
                                       Inst *save_state)
{
    uint32_t class_id;

    AddInstruction(save_state);

    auto field = GetRuntime()->ResolveField(GetMethod(), type_id, !GetGraph()->IsAotMode(), &class_id);
    if (field == nullptr) {
        // Class initialization is handled in UnresolvedLoadStatic
        auto inst = graph_->CreateInstUnresolvedLoadStatic(type, GetPc(bc_inst->GetAddress()));
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), type_id,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        inst->SetTypeId(type_id);
        inst->SetMethod(GetGraph()->GetMethod());
        inst->SetInput(0, save_state);

        return inst;
    }

    auto init_class = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    init_class->SetTypeId(class_id);
    init_class->SetClass(GetRuntime()->GetClassForField(field));
    init_class->SetInput(0, save_state);
    init_class->SetMethod(GetGraph()->GetMethod());

    auto inst = graph_->CreateInstLoadStatic(type, GetPc(bc_inst->GetAddress()));
    inst->SetInput(0, init_class);
    inst->SetTypeId(type_id);
    inst->SetMethod(GetGraph()->GetMethod());
    inst->SetObjField(field);
    if (GetRuntime()->IsFieldVolatile(field)) {
        inst->SetVolatile(true);
    }

    AddInstruction(init_class);

    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildAnyTypeCheckInst(size_t bc_addr, Inst *input, Inst *save_state, AnyBaseType type)
{
    auto any_check = graph_->CreateInstAnyTypeCheck(DataType::ANY, bc_addr);
    any_check->SetInput(0, input);
    any_check->SetInput(1, save_state);
    any_check->SetAnyType(type);
    AddInstruction(any_check);

    return any_check;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadStatic(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = GetRuntime()->ResolveFieldIndex(GetMethod(), field_index);
    if (type != DataType::REFERENCE) {
        type = GetRuntime()->GetFieldTypeById(GetMethod(), field_id);
    }
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    Inst *inst = BuildLoadStaticInst(bc_inst, type, field_id, save_state);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreStaticInst(const BytecodeInstruction *bc_inst, DataType::Type type, size_t type_id,
                                        Inst *store_input, Inst *save_state)
{
    uint32_t class_id;

    AddInstruction(save_state);

    auto field = GetRuntime()->ResolveField(GetMethod(), type_id, !GetGraph()->IsAotMode(), &class_id);
    if (field == nullptr) {
        // Class initialization is handled in UnresolvedLoadStatic
        auto inst = graph_->CreateInstUnresolvedStoreStatic(type, GetPc(bc_inst->GetAddress()));
        auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
        inst->SetTypeId(type_id);
        inst->SetMethod(GetGraph()->GetMethod());
        inst->SetInput(0, store_input);
        inst->SetInput(1, save_state);
        if (type == DataType::REFERENCE) {
            inst->SetNeedBarrier(true);
        }
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), type_id, skind);
            if (type == DataType::REFERENCE) {
                skind = UnresolvedTypesInterface::SlotKind::STATIC_FIELD_PTR;
                GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), type_id, skind);
            }
        }

        return inst;
    }

    auto init_class = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    init_class->SetTypeId(class_id);
    init_class->SetClass(GetRuntime()->GetClassForField(field));
    init_class->SetInput(0, save_state);
    init_class->SetMethod(GetGraph()->GetMethod());

    auto inst = graph_->CreateInstStoreStatic(type, GetPc(bc_inst->GetAddress()));
    inst->SetInput(0, init_class);
    inst->SetInput(1, store_input);
    inst->SetTypeId(type_id);
    inst->SetMethod(GetGraph()->GetMethod());
    inst->SetObjField(field);
    if (GetRuntime()->IsFieldVolatile(field)) {
        inst->SetVolatile(true);
    }
    if (type == DataType::REFERENCE) {
        inst->SetNeedBarrier(true);
    }

    AddInstruction(init_class);

    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreStatic(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = GetRuntime()->ResolveFieldIndex(GetMethod(), field_index);
    if (type != DataType::REFERENCE) {
        type = GetRuntime()->GetFieldTypeById(GetMethod(), field_id);
    }
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    Inst *store_input = GetDefinitionAcc();
    Inst *inst = BuildStoreStaticInst(bc_inst, type, field_id, store_input, save_state);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildChecksBeforeArray(const BytecodeInstruction *bc_inst, Inst *array_ref, Inst **ss, Inst **nc,
                                         Inst **al, Inst **bc)
{
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));

    // Create NullCheck instruction
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    null_check->SetInput(0, array_ref);
    null_check->SetInput(1, save_state);

    // Create LenArray instruction
    auto array_length = graph_->CreateInstLenArray(DataType::INT32, GetPc(bc_inst->GetAddress()));
    array_length->SetInput(0, null_check);

    // Create BoundCheck instruction
    auto bounds_check = graph_->CreateInstBoundsCheck(DataType::INT32, GetPc(bc_inst->GetAddress()));
    bounds_check->SetInput(0, array_length);
    bounds_check->SetInput(INPUT_2, save_state);

    *ss = save_state;
    *nc = null_check;
    *al = array_length;
    *bc = bounds_check;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadArray(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    ASSERT(type != DataType::NO_TYPE);
    Inst *save_state = nullptr;
    Inst *null_check = nullptr;
    Inst *array_length = nullptr;
    Inst *bounds_check = nullptr;
    BuildChecksBeforeArray(bc_inst, GetDefinition(bc_inst->GetVReg(0)), &save_state, &null_check, &array_length,
                           &bounds_check);
    ASSERT(save_state != nullptr && null_check != nullptr && array_length != nullptr && bounds_check != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstLoadArray(type, GetPc(bc_inst->GetAddress()));
    bounds_check->SetInput(1, GetDefinitionAcc());
    inst->SetInput(0, null_check);
    inst->SetInput(1, bounds_check);
    AddInstructions(save_state, null_check, array_length, bounds_check);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

template <typename T>
void InstBuilder::BuildUnfoldLoadConstArray(const BytecodeInstruction *bc_inst, DataType::Type type,
                                            const pandasm::LiteralArray &lit_array)
{
    auto method = GetGraph()->GetMethod();
    auto array_size = lit_array.literals_.size();
    auto type_id = GetRuntime()->GetLiteralArrayClassIdWithinFile(method, lit_array.literals_[0].tag_);

    // Create NewArray instruction
    auto size_inst = graph_->FindOrCreateConstant(array_size);
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto neg_check = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bc_inst->GetAddress()));
    auto init_class = CreateLoadAndInitClassGeneric(type_id, GetPc(bc_inst->GetAddress()));
    init_class->SetInput(0, save_state);
    neg_check->SetInput(0, size_inst);
    neg_check->SetInput(1, save_state);
    auto array_inst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    array_inst->SetTypeId(type_id);
    array_inst->SetMethod(GetGraph()->GetMethod());
    array_inst->SetInput(NewArrayInst::INDEX_CLASS, init_class);
    array_inst->SetInput(NewArrayInst::INDEX_SIZE, neg_check);
    array_inst->SetInput(NewArrayInst::INDEX_SAVE_STATE, save_state);
    AddInstruction(save_state);
    AddInstruction(init_class);
    AddInstruction(neg_check);
    AddInstruction(array_inst);
    UpdateDefinition(bc_inst->GetVReg(0), array_inst);

    if (array_size > options.GetCompilerUnfoldConstArrayMaxSize()) {
        // Create LoadConstArray instruction
        auto ss = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
        auto inst = GetGraph()->CreateInstFillConstArray(type, GetPc(bc_inst->GetAddress()));
        inst->SetTypeId(bc_inst->GetId(0).AsFileId().GetOffset());
        inst->SetMethod(method);
        inst->SetImm(array_size);
        inst->SetInput(0, array_inst);
        inst->SetInput(1, ss);
        AddInstruction(ss);
        AddInstruction(inst);
        return;
    }

    // Create instructions for array filling
    auto tag = lit_array.literals_[0].tag_;
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        for (size_t i = 0; i < array_size; i++) {
            auto index_inst = graph_->FindOrCreateConstant(i);
            ConstantInst *value_inst;
            if (tag == panda_file::LiteralTag::ARRAY_F32) {
                value_inst = FindOrCreateFloatConstant(static_cast<float>(std::get<T>(lit_array.literals_[i].value_)));
            } else if (tag == panda_file::LiteralTag::ARRAY_F64) {
                value_inst =
                    FindOrCreateDoubleConstant(static_cast<double>(std::get<T>(lit_array.literals_[i].value_)));
            } else {
                value_inst = FindOrCreateConstant(std::get<T>(lit_array.literals_[i].value_));
            }

            BuildStoreArrayInst<false>(bc_inst, type, array_inst, index_inst, value_inst);
        }

        return;
    }
    [[maybe_unused]] auto array_class = GetRuntime()->ResolveType(method, type_id);
    ASSERT(GetRuntime()->CheckStoreArray(array_class, GetRuntime()->GetStringClass(method)));

    // Special case for string array
    for (size_t i = 0; i < array_size; i++) {
        auto index_inst = graph_->FindOrCreateConstant(i);
        auto save = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
        auto load_string_inst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
        load_string_inst->SetTypeId(std::get<T>(lit_array.literals_[i].value_));
        load_string_inst->SetMethod(method);
        load_string_inst->SetInput(0, save);
        AddInstruction(save);
        AddInstruction(load_string_inst);
        if (GetGraph()->IsDynamicMethod()) {
            BuildCastToAnyString(bc_inst);
        }

        BuildStoreArrayInst<false>(bc_inst, type, array_inst, index_inst, load_string_inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadConstArray(const BytecodeInstruction *bc_inst)
{
    auto literal_array_id = bc_inst->GetId(0).AsFileId().GetOffset();
    auto lit_array = GetRuntime()->GetLiteralArray(GetMethod(), literal_array_id);

    auto array_size = lit_array.literals_.size();
    ASSERT(array_size > 0);

    // Unfold LoadConstArray instruction
    auto tag = lit_array.literals_[0].tag_;
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1: {
            BuildUnfoldLoadConstArray<bool>(bc_inst, DataType::INT8, lit_array);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U8: {
            BuildUnfoldLoadConstArray<uint8_t>(bc_inst, DataType::INT8, lit_array);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U16: {
            BuildUnfoldLoadConstArray<uint16_t>(bc_inst, DataType::INT16, lit_array);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U32: {
            BuildUnfoldLoadConstArray<uint32_t>(bc_inst, DataType::INT32, lit_array);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_U64: {
            BuildUnfoldLoadConstArray<uint64_t>(bc_inst, DataType::INT64, lit_array);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F32: {
            BuildUnfoldLoadConstArray<float>(bc_inst, DataType::FLOAT32, lit_array);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F64: {
            BuildUnfoldLoadConstArray<double>(bc_inst, DataType::FLOAT64, lit_array);
            break;
        }
        case panda_file::LiteralTag::ARRAY_STRING: {
            if (array_size > options.GetCompilerUnfoldConstArrayMaxSize()) {
                // Create LoadConstArray instruction for String array, because we calls runtime for the case.
                auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
                auto method = GetGraph()->GetMethod();
                auto inst = GetGraph()->CreateInstLoadConstArray(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
                inst->SetTypeId(literal_array_id);
                inst->SetMethod(method);
                inst->SetInput(0, save_state);
                AddInstruction(save_state);
                AddInstruction(inst);
                UpdateDefinition(bc_inst->GetVReg(0), inst);
            } else {
                BuildUnfoldLoadConstArray<uint32_t>(bc_inst, DataType::REFERENCE, lit_array);
            }
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreArray(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    BuildStoreArrayInst<true>(bc_inst, type, GetDefinition(bc_inst->GetVReg(0)), GetDefinition(bc_inst->GetVReg(1)),
                              GetDefinitionAcc());
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool create_ref_check>
void InstBuilder::BuildStoreArrayInst(const BytecodeInstruction *bc_inst, DataType::Type type, Inst *array_ref,
                                      Inst *index, Inst *value)
{
    ASSERT(type != DataType::NO_TYPE);
    Inst *ref_check = nullptr;
    Inst *save_state = nullptr;
    Inst *null_check = nullptr;
    Inst *array_length = nullptr;
    Inst *bounds_check = nullptr;
    BuildChecksBeforeArray(bc_inst, array_ref, &save_state, &null_check, &array_length, &bounds_check);
    ASSERT(save_state != nullptr && null_check != nullptr && array_length != nullptr && bounds_check != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstStoreArray(type, GetPc(bc_inst->GetAddress()));
    bounds_check->SetInput(1, index);
    auto store_def = value;
    if (type == DataType::REFERENCE) {
        // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
        if constexpr (create_ref_check) {
            ref_check = graph_->CreateInstRefTypeCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
            ref_check->SetInput(0, null_check);
            ref_check->SetInput(1, store_def);
            ref_check->SetInput(INPUT_2, save_state);
            store_def = ref_check;
        }
        inst->CastToStoreArray()->SetNeedBarrier(true);
    }
    inst->SetInput(INPUT_2, store_def);
    inst->SetInput(0, null_check);
    inst->SetInput(1, bounds_check);
    AddInstructions(save_state, null_check, array_length, bounds_check);
    if (ref_check != nullptr) {
        AddInstruction(ref_check);
    }
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLenArray(const BytecodeInstruction *bc_inst)
{
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    null_check->SetInput(0, GetDefinition(bc_inst->GetVReg(0)));
    null_check->SetInput(1, save_state);
    auto inst = graph_->CreateInstLenArray(DataType::INT32, GetPc(bc_inst->GetAddress()));
    inst->SetInput(0, null_check);
    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewArray(const BytecodeInstruction *bc_inst)
{
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto neg_check = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bc_inst->GetAddress()));
    neg_check->SetInput(0, GetDefinition(bc_inst->GetVReg(1)));
    neg_check->SetInput(1, save_state);
    auto inst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));

    auto type_index = bc_inst->GetId(0).AsIndex();
    auto type_id = GetRuntime()->ResolveTypeIndex(GetMethod(), type_index);

    auto init_class = CreateLoadAndInitClassGeneric(type_id, GetPc(bc_inst->GetAddress()));
    init_class->SetInput(0, save_state);

    inst->SetTypeId(type_id);
    inst->SetMethod(GetGraph()->GetMethod());
    inst->SetInput(NewArrayInst::INDEX_CLASS, init_class);
    inst->SetInput(NewArrayInst::INDEX_SIZE, neg_check);
    inst->SetInput(NewArrayInst::INDEX_SAVE_STATE, save_state);
    AddInstruction(save_state);
    AddInstruction(init_class);
    AddInstruction(neg_check);
    AddInstruction(inst);
    UpdateDefinition(bc_inst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewObject(const BytecodeInstruction *bc_inst)
{
    auto class_index = bc_inst->GetId(0).AsIndex();
    auto class_id = GetRuntime()->ResolveTypeIndex(GetMethod(), class_index);

    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto init_class = CreateLoadAndInitClassGeneric(class_id, GetPc(bc_inst->GetAddress()));
    auto inst = graph_->CreateInstNewObject(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    inst->SetTypeId(class_id);
    inst->SetMethod(GetGraph()->GetMethod());
    inst->SetInput(0, init_class);
    inst->SetInput(1, save_state);
    init_class->SetInput(0, save_state);

    AddInstruction(save_state);
    AddInstruction(init_class);
    AddInstruction(inst);
    UpdateDefinition(bc_inst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMultiDimensionalArrayObject(const BytecodeInstruction *bc_inst, bool is_range)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);

    auto class_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto init_class = CreateLoadAndInitClassGeneric(class_id, GetPc(bc_inst->GetAddress()));
    size_t args_count = GetMethodArgumentsCount(method_id);
    auto inst = GetGraph()->CreateInstMultiArray(DataType::REFERENCE, GetPc(bc_inst->GetAddress()), method_id);

    init_class->SetInput(0, save_state);

    inst->ReserveInputs(args_count + TWO_INPUTS);  // first input is pointer to object, last input is SaveState
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), args_count + TWO_INPUTS);
    inst->AppendInput(init_class);
    inst->AddInputType(DataType::REFERENCE);

    AddInstruction(save_state);
    AddInstruction(init_class);

    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            auto neg_check = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bc_inst->GetAddress()));
            neg_check->SetInput(0, GetDefinition(start_reg));
            neg_check->SetInput(1, save_state);
            AddInstruction(neg_check);
            inst->AppendInput(neg_check);
            inst->AddInputType(DataType::INT32);
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            auto neg_check = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bc_inst->GetAddress()));
            neg_check->SetInput(0, GetDefinition(bc_inst->GetVReg(i)));
            neg_check->SetInput(1, save_state);
            AddInstruction(neg_check);
            inst->AppendInput(neg_check);
            inst->AddInputType(DataType::INT32);
        }
    }
    inst->AppendInput(save_state);
    inst->AddInputType(DataType::NO_TYPE);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitObjectMultiDimensionalArray(const BytecodeInstruction *bc_inst, bool is_range)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto class_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto init_class = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    auto inst = GetGraph()->CreateInstInitObject(DataType::REFERENCE, GetPc(bc_inst->GetAddress()), method_id);

    size_t args_count = GetMethodArgumentsCount(method_id);

    init_class->SetInput(0, save_state);
    init_class->SetTypeId(class_id);
    init_class->SetMethod(GetGraph()->GetMethod());
    init_class->SetClass(GetRuntime()->ResolveType(GetGraph()->GetMethod(), class_id));
    inst->ReserveInputs(args_count + TWO_INPUTS);  // first input is pointer to object, last input is SaveState
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), args_count + TWO_INPUTS);
    inst->AppendInput(init_class);
    inst->AddInputType(DataType::REFERENCE);
    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            inst->AppendInput(GetDefinition(start_reg));
            inst->AddInputType(GetMethodArgumentType(method_id, i));
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            inst->AppendInput(GetDefinition(bc_inst->GetVReg(i)));
            inst->AddInputType(GetMethodArgumentType(method_id, i));
        }
    }
    inst->AppendInput(save_state);
    inst->AddInputType(DataType::NO_TYPE);
    inst->SetCallMethod(GetRuntime()->GetMethodById(GetGraph()->GetMethod(), method_id));

    AddInstruction(save_state);
    AddInstruction(init_class);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
CallInst *InstBuilder::BuildCallStaticForInitObject(const BytecodeInstruction *bc_inst, uint32_t method_id)
{
    auto method = GetRuntime()->GetMethodById(GetGraph()->GetMethod(), method_id);
    CallInst *inst;
    if (method == nullptr) {
        inst = graph_->CreateInstUnresolvedCallStatic(GetMethodReturnType(method_id), GetPc(bc_inst->GetAddress()),
                                                      method_id);
        inst->SetCallMethod(GetGraph()->GetMethod());
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), method_id,
                                                             UnresolvedTypesInterface::SlotKind::METHOD);
        }
    } else {
        inst = graph_->CreateInstCallStatic(GetMethodReturnType(method_id), GetPc(bc_inst->GetAddress()), method_id);
        inst->SetCallMethod(method);
    }
    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitObject(const BytecodeInstruction *bc_inst, bool is_range)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto class_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);

    if (GetRuntime()->IsArrayClass(GetMethod(), class_id)) {
        if (GetGraph()->IsBytecodeOptimizer()) {
            BuildInitObjectMultiDimensionalArray(bc_inst, is_range);
            return;
        }
        BuildMultiDimensionalArrayObject(bc_inst, is_range);
        return;
    }

    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto init_class = CreateLoadAndInitClassGeneric(class_id, GetPc(bc_inst->GetAddress()));
    auto new_obj = graph_->CreateInstNewObject(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    CallInst *inst = BuildCallStaticForInitObject(bc_inst, method_id);
    size_t args_count = GetMethodArgumentsCount(method_id);

    new_obj->SetInput(0, init_class);
    new_obj->SetInput(1, save_state);
    new_obj->SetTypeId(class_id);
    new_obj->SetMethod(GetGraph()->GetMethod());
    init_class->SetInput(0, save_state);

    inst->ReserveInputs(args_count + TWO_INPUTS);  // first input is pointer to object, last input is SaveState
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), args_count + TWO_INPUTS);
    inst->AppendInput(new_obj);
    inst->AddInputType(DataType::REFERENCE);
    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            inst->AppendInput(GetDefinition(start_reg));
            inst->AddInputType(GetMethodArgumentType(method_id, i));
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            inst->AppendInput(GetDefinition(bc_inst->GetVReg(i)));
            inst->AddInputType(GetMethodArgumentType(method_id, i));
        }
    }
    UpdateDefinitionAcc(new_obj);
    auto save_state_for_call = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    inst->AppendInput(save_state_for_call);
    inst->AddInputType(DataType::NO_TYPE);
    AddInstructions(save_state, init_class, new_obj, save_state_for_call, inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCheckCast(const BytecodeInstruction *bc_inst)
{
    auto type_index = bc_inst->GetId(0).AsIndex();
    auto type_id = GetRuntime()->ResolveTypeIndex(GetMethod(), type_index);
    auto klass_type = GetRuntime()->GetClassType(GetGraph()->GetMethod(), type_id);
    auto pc = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, pc);

    auto load_class = BuildLoadClass(type_id, pc, save_state);

    auto inst = GetGraph()->CreateInstCheckCast(DataType::NO_TYPE, pc);
    inst->SetClassType(klass_type);
    inst->SetTypeId(type_id);
    inst->SetMethod(GetGraph()->GetMethod());
    inst->SetInput(0, GetDefinitionAcc());
    inst->SetInput(1, load_class);
    inst->SetInput(INPUT_2, save_state);

    AddInstruction(save_state);
    AddInstruction(load_class);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIsInstance(const BytecodeInstruction *bc_inst)
{
    auto type_index = bc_inst->GetId(0).AsIndex();
    auto type_id = GetRuntime()->ResolveTypeIndex(GetMethod(), type_index);
    auto klass_type = GetRuntime()->GetClassType(GetGraph()->GetMethod(), type_id);
    auto pc = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, pc);

    auto load_class = BuildLoadClass(type_id, pc, save_state);
    auto inst = GetGraph()->CreateInstIsInstance(DataType::BOOL, pc);
    inst->SetClassType(klass_type);
    inst->SetTypeId(type_id);
    inst->SetMethod(GetGraph()->GetMethod());
    inst->SetInput(0, GetDefinitionAcc());
    inst->SetInput(1, load_class);
    inst->SetInput(INPUT_2, save_state);

    AddInstruction(save_state);
    AddInstruction(load_class);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadClass(RuntimeInterface::IdType type_id, size_t pc, Inst *save_state)
{
    auto inst = GetGraph()->CreateInstLoadClass(DataType::REFERENCE, pc);
    inst->SetTypeId(type_id);
    inst->SetMethod(GetGraph()->GetMethod());
    inst->SetInput(0, save_state);
    auto klass = GetRuntime()->ResolveType(GetGraph()->GetMethod(), type_id);
    if (klass != nullptr) {
        inst->SetClass(klass);
    } else if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
        GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetGraph()->GetMethod(), type_id,
                                                         UnresolvedTypesInterface::SlotKind::CLASS);
    }
    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildThrow(const BytecodeInstruction *bc_inst)
{
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto inst = graph_->CreateInstThrow(DataType::NO_TYPE, GetPc(bc_inst->GetAddress()));
    inst->SetInput(0, GetDefinition(bc_inst->GetVReg(0)));
    inst->SetInput(1, save_state);
    AddInstruction(save_state);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode opcode>
void InstBuilder::BuildLoadFromPool(const BytecodeInstruction *bc_inst)
{
    auto method = GetGraph()->GetMethod();
    uint32_t type_id;
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    LoadFromPool *inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (opcode == Opcode::LoadType) {
        auto type_index = bc_inst->GetId(0).AsIndex();
        type_id = GetRuntime()->ResolveTypeIndex(method, type_index);
        if (GetRuntime()->ResolveType(method, type_id) == nullptr) {
            inst = GetGraph()->CreateInstUnresolvedLoadType(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
            if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
                GetRuntime()->GetUnresolvedTypes()->AddTableSlot(method, type_id,
                                                                 UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
            }
        } else {
            inst = GetGraph()->CreateInstLoadType(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers)
        static_assert(opcode == Opcode::LoadString);
        type_id = GetRuntime()->ResolveOffsetByIndex(GetGraph()->GetMethod(), bc_inst->GetId(0).AsIndex());
        inst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    }
    inst->SetTypeId(type_id);
    inst->SetMethod(method);
    inst->SetInput(0, save_state);

    AddInstruction(save_state);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (opcode == Opcode::LoadString) {
        if (GetGraph()->IsDynamicMethod()) {
            BuildCastToAnyString(bc_inst);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyString(const BytecodeInstruction *bc_inst)
{
    auto input = GetDefinitionAcc();
    ASSERT(input->GetType() == DataType::REFERENCE);

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto any_type = GetAnyStringType(language);
    ASSERT(any_type != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()));
    box->SetAnyType(any_type);
    box->SetInput(0, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyNumber(const BytecodeInstruction *bc_inst)
{
    auto input = GetDefinitionAcc();
    auto type = input->GetType();

    if (input->IsConst() && !DataType::IsFloatType(type)) {
        auto const_insn = input->CastToConstant();
        if (const_insn->GetType() == DataType::INT64) {
            auto value = input->CastToConstant()->GetInt64Value();
            if (value == static_cast<uint32_t>(value)) {
                type = DataType::INT32;
            }
        }
    }

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto any_type = NumericDataTypeToAnyType(type, language);
    ASSERT(any_type != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()));
    box->SetAnyType(any_type);
    box->SetInput(0, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

}  // namespace panda::compiler

#endif  // PANDA_INST_BUILDER_INL_H
