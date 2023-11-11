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

#ifndef PANDA_INST_BUILDER_INL_H
#define PANDA_INST_BUILDER_INL_H

#include "inst_builder.h"
#include "optimizer/code_generator/encode.h"

namespace panda::compiler {
// NOLINTNEXTLINE(misc-definitions-in-headers,readability-function-size)
template <Opcode OPCODE>
void InstBuilder::BuildCall(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read, Inst *additional_input)
{
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex());
    if (GetRuntime()->IsMethodIntrinsic(GetMethod(), method_id)) {
        BuildIntrinsic(bc_inst, is_range, acc_read);
        return;
    }
    auto pc = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    auto has_implicit_arg = !GetRuntime()->IsMethodStatic(GetMethod(), method_id);
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);

    NullCheckInst *null_check = nullptr;
    uint32_t class_id = 0;
    if (has_implicit_arg) {
        null_check =
            graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetArgDefinition(bc_inst, 0, acc_read), save_state);
    } else if (method != nullptr) {
        if (graph_->IsAotMode()) {
            class_id = GetRuntime()->GetClassIdWithinFile(GetMethod(), GetRuntime()->GetClass(method));
        } else {
            class_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);
        }
    }

    Inst *resolver = nullptr;
    // NOLINTNEXTLINE(readability-magic-numbers)
    CallInst *call = BuildCallInst<OPCODE>(method, method_id, pc, &resolver, class_id);
    SetCallArgs(bc_inst, is_range, acc_read, resolver, call, null_check, save_state, has_implicit_arg, method_id,
                additional_input);

    // Add SaveState
    AddInstruction(save_state);
    // Add NullCheck
    if (has_implicit_arg) {
        ASSERT(null_check != nullptr);
        AddInstruction(null_check);
    } else if (!call->IsUnresolved() && call->GetCallMethod() != nullptr) {
        // Initialize class as call is resolved
        BuildInitClassInstForCallStatic(method, class_id, pc, save_state);
    }
    // Add resolver
    if (resolver != nullptr) {
        if (call->IsStaticCall()) {
            resolver->SetInput(0, save_state);
        } else {
            resolver->SetInput(0, null_check);
            resolver->SetInput(1, save_state);
        }
        AddInstruction(resolver);
    }
    // Add Call
    AddInstruction(call);
    if (call->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(call);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
}

template void InstBuilder::BuildCall<Opcode::CallLaunchStatic>(const BytecodeInstruction *bc_inst, bool is_range,
                                                               bool acc_read, Inst *additional_input);
template void InstBuilder::BuildCall<Opcode::CallLaunchVirtual>(const BytecodeInstruction *bc_inst, bool is_range,
                                                                bool acc_read, Inst *additional_input);

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <typename T>
void InstBuilder::SetCallArgs(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read, Inst *resolver, T *call,
                              Inst *null_check, SaveStateInst *save_state, bool has_implicit_arg, uint32_t method_id,
                              Inst *additional_input)
{
    size_t hidden_args_count = has_implicit_arg ? 1 : 0;                 // +1 for non-static call
    size_t additional_args_count = additional_input == nullptr ? 0 : 1;  // +1 for launch call
    size_t args_count = GetMethodArgumentsCount(method_id);
    size_t total_args_count = hidden_args_count + args_count + additional_args_count;
    size_t inputs_count = total_args_count + (save_state == nullptr ? 0 : 1) + (resolver == nullptr ? 0 : 1);
    call->ReserveInputs(inputs_count);
    call->AllocateInputTypes(GetGraph()->GetAllocator(), inputs_count);
    if (resolver != nullptr) {
        call->AppendInput(resolver);
        call->AddInputType(DataType::POINTER);
    }
    if (additional_input != nullptr) {
        call->AppendInput(additional_input);
        call->AddInputType(DataType::REFERENCE);
        save_state->AppendBridge(additional_input);
    }
    if (has_implicit_arg) {
        call->AppendInput(null_check);
        call->AddInputType(DataType::REFERENCE);
    }
    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        // start reg for Virtual call was added
        if (has_implicit_arg) {
            ++start_reg;
        }
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            call->AppendInput(GetDefinition(start_reg));
            call->AddInputType(GetMethodArgumentType(method_id, i));
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            call->AppendInput(GetArgDefinition(bc_inst, i + hidden_args_count, acc_read));
            call->AddInputType(GetMethodArgumentType(method_id, i));
        }
    }
    if (save_state != nullptr) {
        call->AppendInput(save_state);
        call->AddInputType(DataType::NO_TYPE);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitClassInstForCallStatic(RuntimeInterface::MethodPtr method, uint32_t class_id, size_t pc,
                                                  Inst *save_state)
{
    if (GetClassId() != class_id) {
        auto init_class = graph_->CreateInstInitClass(DataType::NO_TYPE, pc, save_state, class_id,
                                                      GetGraph()->GetMethod(), GetRuntime()->GetClass(method));
        AddInstruction(init_class);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
CallInst *InstBuilder::BuildCallInst(RuntimeInterface::MethodPtr method, uint32_t method_id, size_t pc, Inst **resolver,
                                     uint32_t class_id)
{
    ASSERT(resolver != nullptr);
    constexpr bool IS_STATIC = (OPCODE == Opcode::CallStatic || OPCODE == Opcode::CallLaunchStatic);
    constexpr auto SLOT_KIND =
        IS_STATIC ? UnresolvedTypesInterface::SlotKind::METHOD : UnresolvedTypesInterface::SlotKind::VIRTUAL_METHOD;
    CallInst *call = nullptr;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::CallStatic || OPCODE == Opcode::CallLaunchStatic) {
        if (method == nullptr || (runtime_->IsMethodStatic(GetMethod(), method_id) && class_id == 0) ||
            ForceUnresolved() || (OPCODE == Opcode::CallLaunchStatic && graph_->IsAotMode())) {
            ResolveStaticInst *resolve_static =
                graph_->CreateInstResolveStatic(DataType::POINTER, pc, method_id, nullptr);
            *resolver = resolve_static;
            if constexpr (OPCODE == Opcode::CallStatic) {
                call = graph_->CreateInstCallResolvedStatic(GetMethodReturnType(method_id), pc, method_id);
            } else {
                call = graph_->CreateInstCallResolvedLaunchStatic(DataType::VOID, pc, method_id);
            }
            if (!graph_->IsAotMode() && !graph_->IsBytecodeOptimizer()) {
                runtime_->GetUnresolvedTypes()->AddTableSlot(GetMethod(), method_id, SLOT_KIND);
            }
        } else {
            if constexpr (OPCODE == Opcode::CallStatic) {
                call = graph_->CreateInstCallStatic(GetMethodReturnType(method_id), pc, method_id, method);
            } else {
                call = graph_->CreateInstCallLaunchStatic(DataType::VOID, pc, method_id, method);
            }
        }
    }
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::CallVirtual || OPCODE == Opcode::CallLaunchVirtual) {
        ASSERT(!runtime_->IsMethodStatic(GetMethod(), method_id));
        if (method != nullptr && (runtime_->IsInterfaceMethod(method) || graph_->IsAotNoChaMode())) {
            ResolveVirtualInst *resolve_virtual =
                graph_->CreateInstResolveVirtual(DataType::POINTER, pc, method_id, method);
            *resolver = resolve_virtual;
            if constexpr (OPCODE == Opcode::CallVirtual) {
                call = graph_->CreateInstCallResolvedVirtual(GetMethodReturnType(method_id), pc, method_id, method);
            } else {
                call = graph_->CreateInstCallResolvedLaunchVirtual(DataType::VOID, pc, method_id, method);
            }
        } else if (method == nullptr || ForceUnresolved()) {
            ResolveVirtualInst *resolve_virtual =
                graph_->CreateInstResolveVirtual(DataType::POINTER, pc, method_id, nullptr);
            *resolver = resolve_virtual;
            if constexpr (OPCODE == Opcode::CallVirtual) {
                call = graph_->CreateInstCallResolvedVirtual(GetMethodReturnType(method_id), pc, method_id);
            } else {
                call = graph_->CreateInstCallResolvedLaunchVirtual(DataType::VOID, pc, method_id);
            }
            if (!graph_->IsAotMode() && !graph_->IsBytecodeOptimizer()) {
                runtime_->GetUnresolvedTypes()->AddTableSlot(GetMethod(), method_id, SLOT_KIND);
            }
        } else {
            ASSERT(method != nullptr);
            if constexpr (OPCODE == Opcode::CallVirtual) {
                call = graph_->CreateInstCallVirtual(GetMethodReturnType(method_id), pc, method_id, method);
            } else {
                call = graph_->CreateInstCallLaunchVirtual(DataType::VOID, pc, method_id, method);
            }
        }
    }
    if (UNLIKELY(call == nullptr)) {
        UNREACHABLE();
    }
    call->SetCanNativeException(method == nullptr || runtime_->HasNativeException(method));
    return call;
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
        auto null_check =
            graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()), def, save_state);
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
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex());
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);
    auto intrinsic_id = GetRuntime()->GetIntrinsicId(method);
    ASSERT(intrinsic_id != RuntimeInterface::IntrinsicId::COUNT);
    auto ret_type = GetMethodReturnType(method_id);
    auto pc = GetPc(bc_inst->GetAddress());
    IntrinsicInst *call = GetGraph()->CreateInstIntrinsic(ret_type, pc, intrinsic_id);
    // If an intrinsic may call runtime then we need a SaveState
    SaveStateInst *save_state = call->RequireState() ? CreateSaveState(Opcode::SaveState, pc) : nullptr;
    SetCallArgs(bc_inst, is_range, acc_read, nullptr, call, nullptr, save_state, false, method_id);
    if (save_state != nullptr) {
        AddInstruction(save_state);
    }
    /* if there are reference type args to be checked for NULL ('need_nullcheck' intrinsic property) */
    AddArgNullcheckIfNeeded<false>(intrinsic_id, call, save_state, pc);
    AddInstruction(call);
    if (call->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(call);
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

template <Opcode OPCODE>
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

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Mod>(Graph *graph, DataType::Type return_type, size_t pc)
{
    return graph->CreateInstMod(return_type, pc);
}

template void InstBuilder::BuildBinaryOperationIntrinsic<Opcode::Mod>(const BytecodeInstruction *bc_inst,
                                                                      bool acc_read);

template <Opcode OPCODE>
void InstBuilder::BuildBinaryOperationIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    [[maybe_unused]] auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    ASSERT(GetMethodArgumentsCount(method_id) == 2U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto inst = CreateBinaryOperation<OPCODE>(GetGraph(), GetMethodReturnType(method_id), GetPc(bc_inst->GetAddress()));
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
    auto vreg = GetArgDefinition(bc_inst, 0, acc_read);
    auto inst = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), vreg, vreg,
                                              GetMethodArgumentType(method_id, 0), ConditionCode::CC_NE);
    inst->SetOperandsType(GetMethodArgumentType(method_id, 0));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStringLengthIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto bc_addr = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, bc_addr);

    auto null_check =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bc_addr, GetArgDefinition(bc_inst, 0, acc_read), save_state);
    auto array_length = graph_->CreateInstLenArray(DataType::INT32, bc_addr, null_check, false);

    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(array_length);

    Inst *string_length;
    if (graph_->GetRuntime()->IsCompressedStringsEnabled()) {
        auto const_one_inst = graph_->FindOrCreateConstant(1);
        string_length = graph_->CreateInstShr(DataType::INT32, bc_addr, array_length, const_one_inst);
        AddInstruction(string_length);
    } else {
        string_length = array_length;
    }
    UpdateDefinitionAcc(string_length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStringIsEmptyIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto bc_addr = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, bc_addr);
    auto null_check =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bc_addr, GetArgDefinition(bc_inst, 0, acc_read), save_state);
    auto length = graph_->CreateInstLenArray(DataType::INT32, bc_addr, null_check, false);
    auto zero_const = graph_->FindOrCreateConstant(0);
    auto check_zero_length =
        graph_->CreateInstCompare(DataType::BOOL, bc_addr, length, zero_const, DataType::INT32, ConditionCode::CC_EQ);
    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(length);
    AddInstruction(check_zero_length);
    UpdateDefinitionAcc(check_zero_length);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharIsUpperCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    // IsUpperCase(char) = (char - 'A') < ('Z' - 'A')

    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex())) == 1);

    // Adding InstCast here as the aternative compiler backend requires inputs of InstSub to be of the same type.
    // The InstCast instructon makes no harm as normally they are removed by the following compiler stages.
    auto arg_input = GetArgDefinition(bc_inst, 0, acc_read);
    auto const_input = graph_->FindOrCreateConstant('A');
    auto arg =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg_input, arg_input->GetType());
    auto const_a =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), const_input, const_input->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg, const_a);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('Z' - 'A'), DataType::UINT16, ConditionCode::CC_BE);

    AddInstruction(arg);
    AddInstruction(const_a);
    AddInstruction(inst1);
    AddInstruction(inst2);
    UpdateDefinitionAcc(inst2);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharToUpperCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    // ToUpperCase(char) = ((char - 'a') < ('z' - 'a')) * ('Z' - 'z') + char

    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex())) == 1);

    auto arg_input = GetArgDefinition(bc_inst, 0, acc_read);
    auto const_input = graph_->FindOrCreateConstant('a');
    auto arg =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg_input, arg_input->GetType());
    auto const_a =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), const_input, const_input->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg, const_a);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('z' - 'a'), DataType::UINT16, ConditionCode::CC_BE);
    auto inst3 = GetGraph()->CreateInstMul(DataType::UINT16, GetPc(bc_inst->GetAddress()), inst2,
                                           graph_->FindOrCreateConstant('Z' - 'z'));
    auto inst4 = GetGraph()->CreateInstAdd(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg, inst3);

    AddInstruction(arg);
    AddInstruction(const_a);
    AddInstruction(inst1);
    AddInstruction(inst2);
    AddInstruction(inst3);
    AddInstruction(inst4);
    UpdateDefinitionAcc(inst4);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharIsLowerCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex())) == 1);

    auto arg_input = GetArgDefinition(bc_inst, 0, acc_read);
    auto const_input = graph_->FindOrCreateConstant('a');
    auto arg =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg_input, arg_input->GetType());
    auto const_a =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), const_input, const_input->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg, const_a);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('z' - 'a'), DataType::UINT16, ConditionCode::CC_BE);

    AddInstruction(arg);
    AddInstruction(const_a);
    AddInstruction(inst1);
    AddInstruction(inst2);
    UpdateDefinitionAcc(inst2);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharToLowerCaseIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex())) == 1);

    auto arg_input = GetArgDefinition(bc_inst, 0, acc_read);
    auto const_input = graph_->FindOrCreateConstant('A');
    auto arg =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg_input, arg_input->GetType());
    auto const_a =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bc_inst->GetAddress()), const_input, const_input->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg, const_a);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('Z' - 'A'), DataType::UINT16, ConditionCode::CC_BE);
    auto inst3 = GetGraph()->CreateInstMul(DataType::UINT16, GetPc(bc_inst->GetAddress()), inst2,
                                           graph_->FindOrCreateConstant('z' - 'Z'));
    auto inst4 = GetGraph()->CreateInstAdd(DataType::UINT16, GetPc(bc_inst->GetAddress()), arg, inst3);

    AddInstruction(arg);
    AddInstruction(const_a);
    AddInstruction(inst1);
    AddInstruction(inst2);
    AddInstruction(inst3);
    AddInstruction(inst4);
    UpdateDefinitionAcc(inst4);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::GetArgDefinition(const BytecodeInstruction *bc_inst, size_t idx, bool acc_read, bool is_range)
{
    if (is_range) {
        return GetArgDefinitionRange(bc_inst, idx);
    }
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
Inst *InstBuilder::GetArgDefinitionRange(const BytecodeInstruction *bc_inst, size_t idx)
{
    auto start_reg = bc_inst->GetVReg(0);
    return GetDefinition(start_reg + idx);
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
    if (!OPTIONS.IsCompilerEncodeIntrinsics()) {
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
    auto null_check =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bc_addr, GetArgDefinition(bc_inst, 0, acc_read), save_state);

    auto call = GetGraph()->CreateInstIntrinsic(GetMethodReturnType(method_id), bc_addr, intrinsic_id);
    SetCallArgs(bc_inst, is_range, acc_read, nullptr, call, null_check, call->RequireState() ? save_state : nullptr,
                true, method_id);

    AddInstruction(save_state);
    AddInstruction(null_check);

    /* if there are reference type args to be checked for NULL */
    AddArgNullcheckIfNeeded<true>(intrinsic_id, call, save_state, bc_addr);

    AddInstruction(call);
    if (call->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(call);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
    if (NeedSafePointAfterIntrinsic(intrinsic_id)) {
        AddInstruction(CreateSafePoint(current_bb_));
    }
}

// NOLINTNEXTLINE(readability-function-size,misc-definitions-in-headers)
template <bool IS_ACC_WRITE>
void InstBuilder::BuildLoadObject(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    auto pc = GetPc(bc_inst->GetAddress());
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, pc,
                                                  GetDefinition(bc_inst->GetVReg(IS_ACC_WRITE ? 0 : 1)), save_state);
    auto runtime = GetRuntime();
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = runtime->ResolveFieldIndex(GetMethod(), field_index);
    auto field = runtime->ResolveField(GetMethod(), field_id, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), field_id);
    }

    // Create LoadObject instruction
    Inst *inst;
    ResolveObjectFieldInst *resolve_field = nullptr;
    if (field == nullptr || ForceUnresolved()) {
        // 1. Create an instruction to resolve an object's field
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), field_id,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        resolve_field =
            graph_->CreateInstResolveObjectField(DataType::UINT32, pc, save_state, field_id, GetGraph()->GetMethod());
        // 2. Create an instruction to load a value from the resolved field
        auto load_field = graph_->CreateInstLoadResolvedObjectField(type, pc, null_check, resolve_field, field_id,
                                                                    GetGraph()->GetMethod());
        inst = load_field;
    } else {
        auto load_field = graph_->CreateInstLoadObject(type, pc, null_check, field_id, GetGraph()->GetMethod(), field,
                                                       runtime->IsFieldVolatile(field));
        inst = load_field;
    }

    AddInstruction(save_state);
    AddInstruction(null_check);
    if (resolve_field != nullptr) {
        AddInstruction(resolve_field);
    }
    AddInstruction(inst);

    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_ACC_WRITE) {
        UpdateDefinitionAcc(inst);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        UpdateDefinition(bc_inst->GetVReg(0), inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreObjectInst(const BytecodeInstruction *bc_inst, DataType::Type type,
                                        RuntimeInterface::FieldPtr field, size_t field_id, Inst **resolve_inst)
{
    auto pc = GetPc(bc_inst->GetAddress());
    if (field == nullptr || ForceUnresolved()) {
        // The field is unresolved, so we have to resolve it and then store
        // 1. Create an instruction to resolve an object's field
        auto resolve_field =
            graph_->CreateInstResolveObjectField(DataType::UINT32, pc, nullptr, field_id, GetGraph()->GetMethod());
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), field_id,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        // 2. Create an instruction to store a value to the resolved field
        auto store_field = graph_->CreateInstStoreResolvedObjectField(
            type, pc, nullptr, nullptr, nullptr, field_id, GetGraph()->GetMethod(), false, type == DataType::REFERENCE);
        *resolve_inst = resolve_field;
        return store_field;
    }

    ASSERT(field != nullptr);
    auto store_field =
        graph_->CreateInstStoreObject(type, pc, nullptr, nullptr, field_id, GetGraph()->GetMethod(), field,
                                      GetRuntime()->IsFieldVolatile(field), type == DataType::REFERENCE);
    *resolve_inst = nullptr;  // resolver is not needed in this case
    return store_field;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool IS_ACC_READ>
void InstBuilder::BuildStoreObject(const BytecodeInstruction *bc_inst, DataType::Type type)
{
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));

    // Create NullCheck instruction
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()),
                                                  GetDefinition(bc_inst->GetVReg(IS_ACC_READ ? 0 : 1)), save_state);

    auto runtime = GetRuntime();
    auto field_index = bc_inst->GetId(0).AsIndex();
    auto field_id = runtime->ResolveFieldIndex(GetMethod(), field_index);
    auto field = runtime->ResolveField(GetMethod(), field_id, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), field_id);
    }

    // Get a value to store
    Inst *store_val = nullptr;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_ACC_READ) {
        store_val = GetDefinitionAcc();
    } else {  // NOLINT(readability-misleading-indentation)
        store_val = GetDefinition(bc_inst->GetVReg(0));
    }

    // Create StoreObject instruction
    Inst *resolve_field = nullptr;
    Inst *store_field = BuildStoreObjectInst(bc_inst, type, field, field_id, &resolve_field);
    store_field->SetInput(0, null_check);
    store_field->SetInput(1, store_val);

    AddInstruction(save_state);
    AddInstruction(null_check);
    if (resolve_field != nullptr) {
        ASSERT(field == nullptr || ForceUnresolved());
        resolve_field->SetInput(0, save_state);
        store_field->SetInput(2U, resolve_field);
        AddInstruction(resolve_field);
    }
    AddInstruction(store_field);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadStaticInst(const BytecodeInstruction *bc_inst, DataType::Type type, size_t type_id,
                                       Inst *save_state)
{
    AddInstruction(save_state);

    uint32_t class_id;
    auto field = GetRuntime()->ResolveField(GetMethod(), type_id, !GetGraph()->IsAotMode(), &class_id);
    auto pc = GetPc(bc_inst->GetAddress());

    if (field == nullptr || ForceUnresolved()) {
        // The static field is unresolved, so we have to resolve it and then load
        // 1. Create an instruction to resolve an object's static field.
        //    Its result is a static field memory address (not an offset as there is no object)
        auto resolve_field = graph_->CreateInstResolveObjectFieldStatic(DataType::REFERENCE, pc, save_state, type_id,
                                                                        GetGraph()->GetMethod());
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), type_id,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        AddInstruction(resolve_field);
        // 2. Create an instruction to load a value from the resolved static field address
        auto load_field =
            graph_->CreateInstLoadResolvedObjectFieldStatic(type, pc, resolve_field, type_id, GetGraph()->GetMethod());
        return load_field;
    }

    ASSERT(field != nullptr);
    auto init_class = graph_->CreateInstLoadAndInitClass(
        DataType::REFERENCE, pc, save_state, class_id, GetGraph()->GetMethod(), GetRuntime()->GetClassForField(field));
    auto load_field = graph_->CreateInstLoadStatic(type, pc, init_class, type_id, GetGraph()->GetMethod(), field,
                                                   GetRuntime()->IsFieldVolatile(field));
    AddInstruction(init_class);
    return load_field;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildAnyTypeCheckInst(size_t bc_addr, Inst *input, Inst *save_state, AnyBaseType type,
                                         bool type_was_profiled, profiling::AnyInputType allowed_input_type)
{
    auto any_check = graph_->CreateInstAnyTypeCheck(DataType::ANY, bc_addr, input, save_state, type);
    any_check->SetAllowedInputType(allowed_input_type);
    any_check->SetIsTypeWasProfiled(type_was_profiled);
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

// NOLINTNEXTLINE(readability-function-size,misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreStaticInst(const BytecodeInstruction *bc_inst, DataType::Type type, size_t type_id,
                                        Inst *store_input, Inst *save_state)
{
    AddInstruction(save_state);

    uint32_t class_id;
    auto field = GetRuntime()->ResolveField(GetMethod(), type_id, !GetGraph()->IsAotMode(), &class_id);
    auto pc = GetPc(bc_inst->GetAddress());

    if (field == nullptr || ForceUnresolved()) {
        if (type == DataType::REFERENCE) {
            // 1. Class initialization is needed.
            // 2. GC Pre/Post write barriers may be needed.
            // Just call runtime EntrypointId::UNRESOLVED_STORE_STATIC_BARRIERED,
            // which performs all the necessary steps (see codegen.cpp for the details).
            auto inst = graph_->CreateInstUnresolvedStoreStatic(type, pc, store_input, save_state, type_id,
                                                                GetGraph()->GetMethod(), true);
            return inst;
        }
        ASSERT(type != DataType::REFERENCE);
        // 1. Create an instruction to resolve an object's static field.
        //    Its result is a static field memory address (REFERENCE)
        auto resolve_field = graph_->CreateInstResolveObjectFieldStatic(DataType::REFERENCE, pc, save_state, type_id,
                                                                        GetGraph()->GetMethod());
        AddInstruction(resolve_field);
        // 2. Create an instruction to store a value to the resolved static field address
        auto store_field = graph_->CreateInstStoreResolvedObjectFieldStatic(type, pc, resolve_field, store_input,
                                                                            type_id, GetGraph()->GetMethod());
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), type_id,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        return store_field;
    }

    ASSERT(field != nullptr);
    auto init_class = graph_->CreateInstLoadAndInitClass(
        DataType::REFERENCE, pc, save_state, class_id, GetGraph()->GetMethod(), GetRuntime()->GetClassForField(field));
    auto store_field =
        graph_->CreateInstStoreStatic(type, pc, init_class, store_input, type_id, GetGraph()->GetMethod(), field,
                                      GetRuntime()->IsFieldVolatile(field), type == DataType::REFERENCE);
    AddInstruction(init_class);
    return store_field;
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
void InstBuilder::BuildChecksBeforeArray(size_t pc, Inst *array_ref, Inst **ss, Inst **nc, Inst **al, Inst **bc,
                                         bool with_nullcheck)
{
    // Create SaveState instruction
    auto save_state = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    Inst *null_check = nullptr;
    if (with_nullcheck) {
        null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, pc, array_ref, save_state);
    } else {
        null_check = array_ref;
    }

    // Create LenArray instruction
    auto array_length = graph_->CreateInstLenArray(DataType::INT32, pc, null_check);

    // Create BoundCheck instruction
    auto bounds_check = graph_->CreateInstBoundsCheck(DataType::INT32, pc, array_length, nullptr, save_state);

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
    auto pc = GetPc(bc_inst->GetAddress());
    BuildChecksBeforeArray(pc, GetDefinition(bc_inst->GetVReg(0)), &save_state, &null_check, &array_length,
                           &bounds_check);
    ASSERT(save_state != nullptr && null_check != nullptr && array_length != nullptr && bounds_check != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstLoadArray(type, pc, null_check, bounds_check);
    bounds_check->SetInput(1, GetDefinitionAcc());
    AddInstruction(save_state, null_check, array_length, bounds_check, inst);
    UpdateDefinitionAcc(inst);
}

template <typename T>
void InstBuilder::BuildUnfoldLoadConstArray(const BytecodeInstruction *bc_inst, DataType::Type type,
                                            const pandasm::LiteralArray &lit_array)
{
    auto method = GetGraph()->GetMethod();
    auto array_size = lit_array.literals.size();
    auto type_id = GetRuntime()->GetLiteralArrayClassIdWithinFile(method, lit_array.literals[0].tag);

    // Create NewArray instruction
    auto size_inst = graph_->FindOrCreateConstant(array_size);
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto neg_check =
        graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bc_inst->GetAddress()), size_inst, save_state);
    auto init_class = CreateLoadAndInitClassGeneric(type_id, GetPc(bc_inst->GetAddress()));
    init_class->SetInput(0, save_state);
    auto array_inst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bc_inst->GetAddress()), init_class,
                                                 neg_check, save_state, type_id, GetGraph()->GetMethod());
    AddInstruction(save_state);
    AddInstruction(init_class);
    AddInstruction(neg_check);
    AddInstruction(array_inst);
    UpdateDefinition(bc_inst->GetVReg(0), array_inst);

    if (array_size > OPTIONS.GetCompilerUnfoldConstArrayMaxSize()) {
        // Create LoadConstArray instruction
        auto ss = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
        auto inst = GetGraph()->CreateInstFillConstArray(type, GetPc(bc_inst->GetAddress()), array_inst, ss,
                                                         bc_inst->GetId(0).AsFileId().GetOffset(), method, array_size);
        AddInstruction(ss);
        AddInstruction(inst);
        return;
    }

    // Create instructions for array filling
    auto tag = lit_array.literals[0].tag;
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        for (size_t i = 0; i < array_size; i++) {
            auto index_inst = graph_->FindOrCreateConstant(i);
            ConstantInst *value_inst;
            if (tag == panda_file::LiteralTag::ARRAY_F32) {
                value_inst = FindOrCreateFloatConstant(static_cast<float>(std::get<T>(lit_array.literals[i].value)));
            } else if (tag == panda_file::LiteralTag::ARRAY_F64) {
                value_inst = FindOrCreateDoubleConstant(static_cast<double>(std::get<T>(lit_array.literals[i].value)));
            } else {
                value_inst = FindOrCreateConstant(std::get<T>(lit_array.literals[i].value));
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
        auto load_string_inst = GetGraph()->CreateInstLoadString(
            DataType::REFERENCE, GetPc(bc_inst->GetAddress()), save, std::get<T>(lit_array.literals[i].value), method);
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
    auto literal_array_idx = bc_inst->GetId(0).AsIndex();
    auto lit_array = GetRuntime()->GetLiteralArray(GetMethod(), literal_array_idx);

    auto array_size = lit_array.literals.size();
    ASSERT(array_size > 0);

    // Unfold LoadConstArray instruction
    auto tag = lit_array.literals[0].tag;
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
            if (array_size > OPTIONS.GetCompilerUnfoldConstArrayMaxSize()) {
                // Create LoadConstArray instruction for String array, because we calls runtime for the case.
                auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
                auto method = GetGraph()->GetMethod();
                auto inst = GetGraph()->CreateInstLoadConstArray(DataType::REFERENCE, GetPc(bc_inst->GetAddress()),
                                                                 save_state, literal_array_idx, method);
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
template <bool CREATE_REF_CHECK>
void InstBuilder::BuildStoreArrayInst(const BytecodeInstruction *bc_inst, DataType::Type type, Inst *array_ref,
                                      Inst *index, Inst *value)
{
    ASSERT(type != DataType::NO_TYPE);
    Inst *ref_check = nullptr;
    Inst *save_state = nullptr;
    Inst *null_check = nullptr;
    Inst *array_length = nullptr;
    Inst *bounds_check = nullptr;
    auto pc = GetPc(bc_inst->GetAddress());
    BuildChecksBeforeArray(pc, array_ref, &save_state, &null_check, &array_length, &bounds_check);
    ASSERT(save_state != nullptr && null_check != nullptr && array_length != nullptr && bounds_check != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstStoreArray(type, pc);
    bounds_check->SetInput(1, index);
    auto store_def = value;
    if (type == DataType::REFERENCE) {
        // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
        if constexpr (CREATE_REF_CHECK) {
            ref_check = graph_->CreateInstRefTypeCheck(DataType::REFERENCE, pc, null_check, store_def, save_state);
            store_def = ref_check;
        }
        inst->CastToStoreArray()->SetNeedBarrier(true);
    }
    inst->SetInput(0, null_check);
    inst->SetInput(1, bounds_check);
    inst->SetInput(2U, store_def);
    if (ref_check != nullptr) {
        AddInstruction(save_state, null_check, array_length, bounds_check, ref_check, inst);
    } else {
        AddInstruction(save_state, null_check, array_length, bounds_check, inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLenArray(const BytecodeInstruction *bc_inst)
{
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
    null_check->SetInput(0, GetDefinition(bc_inst->GetVReg(0)));
    null_check->SetInput(1, save_state);
    auto inst = graph_->CreateInstLenArray(DataType::INT32, GetPc(bc_inst->GetAddress()), null_check);
    AddInstruction(save_state);
    AddInstruction(null_check);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewArray(const BytecodeInstruction *bc_inst)
{
    auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
    auto neg_check = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bc_inst->GetAddress()),
                                                     GetDefinition(bc_inst->GetVReg(1)), save_state);

    auto type_index = bc_inst->GetId(0).AsIndex();
    auto type_id = GetRuntime()->ResolveTypeIndex(GetMethod(), type_index);

    auto init_class = CreateLoadAndInitClassGeneric(type_id, GetPc(bc_inst->GetAddress()));
    init_class->SetInput(0, save_state);

    auto inst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bc_inst->GetAddress()), init_class, neg_check,
                                           save_state, type_id, GetGraph()->GetMethod());
    AddInstruction(save_state, init_class, neg_check, inst);
    UpdateDefinition(bc_inst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewObject(const BytecodeInstruction *bc_inst)
{
    auto class_id = GetRuntime()->ResolveTypeIndex(GetMethod(), bc_inst->GetId(0).AsIndex());
    auto pc = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    auto init_class = CreateLoadAndInitClassGeneric(class_id, pc);
    auto inst = CreateNewObjectInst(pc, class_id, save_state, init_class);
    init_class->SetInput(0, save_state);
    AddInstruction(save_state, init_class, inst);
    UpdateDefinition(bc_inst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMultiDimensionalArrayObject(const BytecodeInstruction *bc_inst, bool is_range)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto pc = GetPc(bc_inst->GetAddress());
    auto class_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    auto init_class = CreateLoadAndInitClassGeneric(class_id, pc);
    size_t args_count = GetMethodArgumentsCount(method_id);
    auto inst = GetGraph()->CreateInstMultiArray(DataType::REFERENCE, pc, method_id);

    init_class->SetInput(0, save_state);

    inst->ReserveInputs(ONE_FOR_OBJECT + args_count + ONE_FOR_SSTATE);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), ONE_FOR_OBJECT + args_count + ONE_FOR_SSTATE);
    inst->AppendInput(init_class);
    inst->AddInputType(DataType::REFERENCE);

    AddInstruction(save_state, init_class);

    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            auto neg_check = graph_->CreateInstNegativeCheck(DataType::INT32, pc, GetDefinition(start_reg), save_state);
            AddInstruction(neg_check);
            inst->AppendInput(neg_check);
            inst->AddInputType(DataType::INT32);
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            auto neg_check =
                graph_->CreateInstNegativeCheck(DataType::INT32, pc, GetDefinition(bc_inst->GetVReg(i)), save_state);
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
    auto pc = GetPc(bc_inst->GetAddress());
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto class_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    auto init_class =
        graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, save_state, class_id, GetGraph()->GetMethod(),
                                           GetRuntime()->ResolveType(GetGraph()->GetMethod(), class_id));
    auto inst = GetGraph()->CreateInstInitObject(DataType::REFERENCE, pc, method_id);

    size_t args_count = GetMethodArgumentsCount(method_id);

    inst->ReserveInputs(ONE_FOR_OBJECT + args_count + ONE_FOR_SSTATE);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), ONE_FOR_OBJECT + args_count + ONE_FOR_SSTATE);
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

    AddInstruction(save_state, init_class, inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
CallInst *InstBuilder::BuildCallStaticForInitObject(const BytecodeInstruction *bc_inst, uint32_t method_id,
                                                    Inst **resolver)
{
    auto pc = GetPc(bc_inst->GetAddress());
    size_t args_count = GetMethodArgumentsCount(method_id);
    size_t inputs_count = ONE_FOR_OBJECT + args_count + ONE_FOR_SSTATE;
    auto method = GetRuntime()->GetMethodById(graph_->GetMethod(), method_id);
    CallInst *call = nullptr;
    if (method == nullptr || ForceUnresolved()) {
        ResolveStaticInst *resolve_static = graph_->CreateInstResolveStatic(DataType::POINTER, pc, method_id, nullptr);
        *resolver = resolve_static;
        call = graph_->CreateInstCallResolvedStatic(GetMethodReturnType(method_id), pc, method_id);
        if (!graph_->IsAotMode() && !graph_->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), method_id,
                                                             UnresolvedTypesInterface::SlotKind::METHOD);
        }
        inputs_count += 1;  // resolver
    } else {
        call = graph_->CreateInstCallStatic(GetMethodReturnType(method_id), pc, method_id, method);
    }
    call->ReserveInputs(inputs_count);
    call->AllocateInputTypes(graph_->GetAllocator(), inputs_count);
    return call;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitString(const BytecodeInstruction *bc_inst)
{
    auto pc = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    AddInstruction(save_state);

    auto ctor_method_index = bc_inst->GetId(0).AsIndex();
    auto ctor_method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), ctor_method_index);
    size_t args_count = GetMethodArgumentsCount(ctor_method_id);

    Inst *inst = nullptr;
    if (args_count == 0) {
        inst = GetGraph()->CreateInstInitEmptyString(DataType::REFERENCE, pc, save_state);
    } else {
        ASSERT(args_count == 1);
        auto null_check =
            graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bc_inst->GetVReg(0)), save_state);
        AddInstruction(null_check);

        auto ctor_method = GetRuntime()->GetMethodById(GetMethod(), ctor_method_id);
        auto ctor_type = GetRuntime()->GetStringCtorType(ctor_method);
        inst = GetGraph()->CreateInstInitString(DataType::REFERENCE, pc, null_check, save_state, ctor_type);
    }
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitObject(const BytecodeInstruction *bc_inst, bool is_range)
{
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex());
    auto type_id = GetRuntime()->GetClassIdForMethod(GetMethod(), method_id);
    if (GetRuntime()->IsArrayClass(GetMethod(), type_id)) {
        if (GetGraph()->IsBytecodeOptimizer()) {
            BuildInitObjectMultiDimensionalArray(bc_inst, is_range);
        } else {
            BuildMultiDimensionalArrayObject(bc_inst, is_range);
        }
        return;
    }

    if (GetRuntime()->IsStringClass(GetMethod(), type_id) && !GetGraph()->IsBytecodeOptimizer()) {
        BuildInitString(bc_inst);
        return;
    }

    auto pc = GetPc(bc_inst->GetAddress());
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    auto init_class = CreateLoadAndInitClassGeneric(type_id, pc);
    init_class->SetInput(0, save_state);
    auto new_obj = CreateNewObjectInst(pc, type_id, save_state, init_class);
    UpdateDefinitionAcc(new_obj);
    Inst *resolver = nullptr;
    CallInst *call = BuildCallStaticForInitObject(bc_inst, method_id, &resolver);
    if (resolver != nullptr) {
        call->AppendInput(resolver);
        call->AddInputType(DataType::POINTER);
    }
    call->AppendInput(new_obj);
    call->AddInputType(DataType::REFERENCE);

    size_t args_count = GetMethodArgumentsCount(method_id);
    if (is_range) {
        auto start_reg = bc_inst->GetVReg(0);
        for (size_t i = 0; i < args_count; start_reg++, i++) {
            call->AppendInput(GetDefinition(start_reg));
            call->AddInputType(GetMethodArgumentType(method_id, i));
        }
    } else {
        for (size_t i = 0; i < args_count; i++) {
            call->AppendInput(GetDefinition(bc_inst->GetVReg(i)));
            call->AddInputType(GetMethodArgumentType(method_id, i));
        }
    }
    auto save_state_for_call = CreateSaveState(Opcode::SaveState, pc);
    call->AppendInput(save_state_for_call);
    call->AddInputType(DataType::NO_TYPE);
    if (resolver != nullptr) {
        resolver->SetInput(0, save_state_for_call);
        AddInstruction(save_state, init_class, new_obj, save_state_for_call, resolver, call);
    } else {
        AddInstruction(save_state, init_class, new_obj, save_state_for_call, call);
    }
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
    auto inst = GetGraph()->CreateInstCheckCast(DataType::NO_TYPE, pc, GetDefinitionAcc(), load_class, save_state,
                                                type_id, GetGraph()->GetMethod(), klass_type);
    AddInstruction(save_state, load_class, inst);
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
    auto inst = GetGraph()->CreateInstIsInstance(DataType::BOOL, pc, GetDefinitionAcc(), load_class, save_state,
                                                 type_id, GetGraph()->GetMethod(), klass_type);
    AddInstruction(save_state, load_class, inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadClass(RuntimeInterface::IdType type_id, size_t pc, Inst *save_state)
{
    auto inst =
        GetGraph()->CreateInstLoadClass(DataType::REFERENCE, pc, save_state, type_id, GetGraph()->GetMethod(), nullptr);
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
    auto inst = graph_->CreateInstThrow(DataType::NO_TYPE, GetPc(bc_inst->GetAddress()),
                                        GetDefinition(bc_inst->GetVReg(0)), save_state);
    inst->SetCallMethod(GetMethod());
    AddInstruction(save_state);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
void InstBuilder::BuildLoadFromPool(const BytecodeInstruction *bc_inst)
{
    auto method = GetGraph()->GetMethod();
    uint32_t type_id;
    Inst *inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (OPCODE == Opcode::LoadType) {
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
        static_assert(OPCODE == Opcode::LoadString);
        type_id = bc_inst->GetId(0).AsFileId().GetOffset();
        if (!GetGraph()->IsDynamicMethod() || GetGraph()->IsBytecodeOptimizer()) {
            inst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bc_inst->GetAddress()));
        } else {
            inst = GetGraph()->CreateInstLoadFromConstantPool(DataType::ANY, GetPc(bc_inst->GetAddress()));
            inst->CastToLoadFromConstantPool()->SetString(true);
        }
    }
    if (!GetGraph()->IsDynamicMethod() || GetGraph()->IsBytecodeOptimizer()) {
        // Create SaveState instruction
        auto save_state = CreateSaveState(Opcode::SaveState, GetPc(bc_inst->GetAddress()));
        inst->SetInput(0, save_state);
        static_cast<LoadFromPool *>(inst)->SetTypeId(type_id);
        static_cast<LoadFromPool *>(inst)->SetMethod(method);
        AddInstruction(save_state);
    } else {
        inst->SetInput(0, GetEnvDefinition(CONST_POOL_IDX));
        inst->CastToLoadFromConstantPool()->SetTypeId(type_id);
        inst->CastToLoadFromConstantPool()->SetMethod(method);
    }

    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::LoadString) {
        if (GetGraph()->IsDynamicMethod() && GetGraph()->IsBytecodeOptimizer()) {
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

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()), any_type, input);
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

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bc_inst->GetAddress()), any_type, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
bool InstBuilder::TryBuildStringCharAtIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto bc_addr = GetPc(bc_inst->GetAddress());
    auto compression_enabled = graph_->GetRuntime()->IsCompressedStringsEnabled();
    auto can_encode_compressed_str_char_at = graph_->GetEncoder()->CanEncodeCompressedStringCharAt();
    if (compression_enabled && !can_encode_compressed_str_char_at) {
        return false;
    }
    auto save_state_nullcheck = CreateSaveState(Opcode::SaveState, bc_addr);
    auto null_check = graph_->CreateInstNullCheck(DataType::REFERENCE, bc_addr, GetArgDefinition(bc_inst, 0, acc_read),
                                                  save_state_nullcheck);
    auto array_length = graph_->CreateInstLenArray(DataType::INT32, bc_addr, null_check, false);

    AddInstruction(save_state_nullcheck);
    AddInstruction(null_check);
    AddInstruction(array_length);

    Inst *string_length = nullptr;
    if (compression_enabled) {
        auto const_one_inst = graph_->FindOrCreateConstant(1);
        string_length = graph_->CreateInstShr(DataType::INT32, bc_addr, array_length, const_one_inst);
        AddInstruction(string_length);
    } else {
        string_length = array_length;
    }

    auto bounds_check = graph_->CreateInstBoundsCheck(
        DataType::INT32, bc_addr, string_length, GetArgDefinition(bc_inst, 1, acc_read), save_state_nullcheck, false);
    AddInstruction(bounds_check);

    Inst *inst = nullptr;
    if (compression_enabled) {
        inst = graph_->CreateInstLoadCompressedStringChar(DataType::UINT16, bc_addr, null_check, bounds_check,
                                                          array_length);
    } else {
        inst = graph_->CreateInstLoadArray(DataType::UINT16, bc_addr, null_check, bounds_check);
    }

    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    return true;
}

}  // namespace panda::compiler

#endif  // PANDA_INST_BUILDER_INL_H
