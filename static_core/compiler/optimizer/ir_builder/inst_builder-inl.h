/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::compiler {
// NOLINTNEXTLINE(misc-definitions-in-headers,readability-function-size)
template <Opcode OPCODE>
void InstBuilder::BuildCall(const BytecodeInstruction *bcInst, bool isRange, bool accRead, Inst *additionalInput)
{
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex());
    if (GetRuntime()->IsMethodIntrinsic(GetMethod(), methodId)) {
        BuildIntrinsic(bcInst, isRange, accRead);
        return;
    }
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto hasImplicitArg = !GetRuntime()->IsMethodStatic(GetMethod(), methodId);
    auto method = GetRuntime()->GetMethodById(GetMethod(), methodId);

    NullCheckInst *nullCheck = nullptr;
    uint32_t classId = 0;
    if (hasImplicitArg) {
        nullCheck =
            graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetArgDefinition(bcInst, 0, accRead), saveState);
    } else if (method != nullptr) {
        if (graph_->IsAotMode()) {
            classId = GetRuntime()->GetClassIdWithinFile(GetMethod(), GetRuntime()->GetClass(method));
        } else {
            classId = GetRuntime()->GetClassIdForMethod(GetMethod(), methodId);
        }
    }

    Inst *resolver = nullptr;
    // NOLINTNEXTLINE(readability-magic-numbers)
    CallInst *call = BuildCallInst<OPCODE>(method, methodId, pc, &resolver, classId);
    SetCallArgs(bcInst, isRange, accRead, resolver, call, nullCheck, saveState, hasImplicitArg, methodId,
                additionalInput);

    // Add SaveState
    AddInstruction(saveState);
    // Add NullCheck
    if (hasImplicitArg) {
        ASSERT(nullCheck != nullptr);
        AddInstruction(nullCheck);
    } else if (!call->IsUnresolved() && call->GetCallMethod() != nullptr) {
        // Initialize class as call is resolved
        BuildInitClassInstForCallStatic(method, classId, pc, saveState);
    }
    // Add resolver
    if (resolver != nullptr) {
        if (call->IsStaticCall()) {
            resolver->SetInput(0, saveState);
        } else {
            resolver->SetInput(0, nullCheck);
            resolver->SetInput(1, saveState);
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

template void InstBuilder::BuildCall<Opcode::CallLaunchStatic>(const BytecodeInstruction *bcInst, bool isRange,
                                                               bool accRead, Inst *additionalInput);
template void InstBuilder::BuildCall<Opcode::CallLaunchVirtual>(const BytecodeInstruction *bcInst, bool isRange,
                                                                bool accRead, Inst *additionalInput);

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <typename T>
void InstBuilder::SetCallArgs(const BytecodeInstruction *bcInst, bool isRange, bool accRead, Inst *resolver, T *call,
                              Inst *nullCheck, SaveStateInst *saveState, bool hasImplicitArg, uint32_t methodId,
                              Inst *additionalInput)
{
    size_t hiddenArgsCount = hasImplicitArg ? 1 : 0;                  // +1 for non-static call
    size_t additionalArgsCount = additionalInput == nullptr ? 0 : 1;  // +1 for launch call
    size_t argsCount = GetMethodArgumentsCount(methodId);
    size_t totalArgsCount = hiddenArgsCount + argsCount + additionalArgsCount;
    size_t inputsCount = totalArgsCount + (saveState == nullptr ? 0 : 1) + (resolver == nullptr ? 0 : 1);
    call->ReserveInputs(inputsCount);
    call->AllocateInputTypes(GetGraph()->GetAllocator(), inputsCount);
    if (resolver != nullptr) {
        call->AppendInput(resolver);
        call->AddInputType(DataType::POINTER);
    }
    if (additionalInput != nullptr) {
        call->AppendInput(additionalInput);
        call->AddInputType(DataType::REFERENCE);
        saveState->AppendBridge(additionalInput);
    }
    if (hasImplicitArg) {
        call->AppendInput(nullCheck);
        call->AddInputType(DataType::REFERENCE);
    }
    if (isRange) {
        auto startReg = bcInst->GetVReg(0);
        // start reg for Virtual call was added
        if (hasImplicitArg) {
            ++startReg;
        }
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            call->AppendInput(GetDefinition(startReg));
            call->AddInputType(GetMethodArgumentType(methodId, i));
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            call->AppendInput(GetArgDefinition(bcInst, i + hiddenArgsCount, accRead));
            call->AddInputType(GetMethodArgumentType(methodId, i));
        }
    }
    if (saveState != nullptr) {
        call->AppendInput(saveState);
        call->AddInputType(DataType::NO_TYPE);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitClassInstForCallStatic(RuntimeInterface::MethodPtr method, uint32_t classId, size_t pc,
                                                  Inst *saveState)
{
    if (GetClassId() != classId) {
        auto initClass = graph_->CreateInstInitClass(DataType::NO_TYPE, pc, saveState, classId, GetGraph()->GetMethod(),
                                                     GetRuntime()->GetClass(method));
        AddInstruction(initClass);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
CallInst *InstBuilder::BuildCallStaticInst(RuntimeInterface::MethodPtr method, uint32_t methodId, size_t pc,
                                           Inst **resolver, uint32_t classId)
{
    constexpr auto SLOT_KIND = UnresolvedTypesInterface::SlotKind::METHOD;
    CallInst *call = nullptr;
    if (method == nullptr || (runtime_->IsMethodStatic(GetMethod(), methodId) && classId == 0) || ForceUnresolved() ||
        (OPCODE == Opcode::CallLaunchStatic && graph_->IsAotMode())) {
        ResolveStaticInst *resolveStatic = graph_->CreateInstResolveStatic(DataType::POINTER, pc, methodId, nullptr);
        *resolver = resolveStatic;
        if constexpr (OPCODE == Opcode::CallStatic) {
            call = graph_->CreateInstCallResolvedStatic(GetMethodReturnType(methodId), pc, methodId);
        } else {
            call = graph_->CreateInstCallResolvedLaunchStatic(DataType::VOID, pc, methodId);
        }
        if (!graph_->IsAotMode() && !graph_->IsBytecodeOptimizer()) {
            runtime_->GetUnresolvedTypes()->AddTableSlot(GetMethod(), methodId, SLOT_KIND);
        }
    } else {
        if constexpr (OPCODE == Opcode::CallStatic) {
            call = graph_->CreateInstCallStatic(GetMethodReturnType(methodId), pc, methodId, method);
        } else {
            call = graph_->CreateInstCallLaunchStatic(DataType::VOID, pc, methodId, method);
        }
    }
    return call;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
CallInst *InstBuilder::BuildCallVirtualInst(RuntimeInterface::MethodPtr method, uint32_t methodId, size_t pc,
                                            Inst **resolver)
{
    constexpr auto SLOT_KIND = UnresolvedTypesInterface::SlotKind::VIRTUAL_METHOD;
    CallInst *call = nullptr;
    ASSERT(!runtime_->IsMethodStatic(GetMethod(), methodId));
    if (method != nullptr && (runtime_->IsInterfaceMethod(method) || graph_->IsAotNoChaMode())) {
        ResolveVirtualInst *resolveVirtual = graph_->CreateInstResolveVirtual(DataType::POINTER, pc, methodId, method);
        *resolver = resolveVirtual;
        if constexpr (OPCODE == Opcode::CallVirtual) {
            call = graph_->CreateInstCallResolvedVirtual(GetMethodReturnType(methodId), pc, methodId, method);
        } else {
            call = graph_->CreateInstCallResolvedLaunchVirtual(DataType::VOID, pc, methodId, method);
        }
    } else if (method == nullptr || ForceUnresolved()) {
        ResolveVirtualInst *resolveVirtual = graph_->CreateInstResolveVirtual(DataType::POINTER, pc, methodId, nullptr);
        *resolver = resolveVirtual;
        if constexpr (OPCODE == Opcode::CallVirtual) {
            call = graph_->CreateInstCallResolvedVirtual(GetMethodReturnType(methodId), pc, methodId);
        } else {
            call = graph_->CreateInstCallResolvedLaunchVirtual(DataType::VOID, pc, methodId);
        }
        if (!graph_->IsAotMode() && !graph_->IsBytecodeOptimizer()) {
            runtime_->GetUnresolvedTypes()->AddTableSlot(GetMethod(), methodId, SLOT_KIND);
        }
    } else {
        ASSERT(method != nullptr);
        if constexpr (OPCODE == Opcode::CallVirtual) {
            call = graph_->CreateInstCallVirtual(GetMethodReturnType(methodId), pc, methodId, method);
        } else {
            call = graph_->CreateInstCallLaunchVirtual(DataType::VOID, pc, methodId, method);
        }
    }
    return call;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
CallInst *InstBuilder::BuildCallInst(RuntimeInterface::MethodPtr method, uint32_t methodId, size_t pc, Inst **resolver,
                                     [[maybe_unused]] uint32_t classId)
{
    ASSERT(resolver != nullptr);
    CallInst *call = nullptr;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::CallStatic || OPCODE == Opcode::CallLaunchStatic) {
        call = BuildCallStaticInst<OPCODE>(method, methodId, pc, resolver, classId);
    }
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::CallVirtual || OPCODE == Opcode::CallLaunchVirtual) {
        call = BuildCallVirtualInst<OPCODE>(method, methodId, pc, resolver);
    }
    if (UNLIKELY(call == nullptr)) {
        UNREACHABLE();
    }
    call->SetCanNativeException(method == nullptr || runtime_->HasNativeException(method));
    return call;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMonitor(const BytecodeInstruction *bcInst, Inst *def, bool isEnter)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto inst = GetGraph()->CreateInstMonitor(DataType::VOID, GetPc(bcInst->GetAddress()));
    AddInstruction(saveState);
    if (!isEnter) {
        inst->CastToMonitor()->SetExit();
    } else {
        // Create NullCheck instruction
        auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bcInst->GetAddress()), def, saveState);
        def = nullCheck;
        AddInstruction(nullCheck);
    }
    inst->SetInput(0, def);
    inst->SetInput(1, saveState);

    AddInstruction(inst);
}

#include <intrinsics_ir_build.inl>

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildDefaultStaticIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead)
{
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex());
    auto method = GetRuntime()->GetMethodById(GetMethod(), methodId);
    auto intrinsicId = GetRuntime()->GetIntrinsicId(method);
    ASSERT(intrinsicId != RuntimeInterface::IntrinsicId::COUNT);
    auto retType = GetMethodReturnType(methodId);
    auto pc = GetPc(bcInst->GetAddress());
    IntrinsicInst *call = GetGraph()->CreateInstIntrinsic(retType, pc, intrinsicId);
    // If an intrinsic may call runtime then we need a SaveState
    SaveStateInst *saveState = call->RequireState() ? CreateSaveState(Opcode::SaveState, pc) : nullptr;
    SetCallArgs(bcInst, isRange, accRead, nullptr, call, nullptr, saveState, false, methodId);
    if (saveState != nullptr) {
        AddInstruction(saveState);
    }
    /* if there are reference type args to be checked for NULL ('need_nullcheck' intrinsic property) */
    AddArgNullcheckIfNeeded<false>(intrinsicId, call, saveState, pc);
    AddInstruction(call);
    if (call->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(call);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
    if (NeedSafePointAfterIntrinsic(intrinsicId)) {
        AddInstruction(CreateSafePoint(currentBb_));
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildAbsIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto inst = GetGraph()->CreateInstAbs(GetMethodReturnType(methodId), GetPc(bcInst->GetAddress()));
    ASSERT(GetMethodArgumentsCount(methodId) == 1);
    inst->SetInput(0, GetArgDefinition(bcInst, 0, accRead));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

template <Opcode OPCODE>
static BinaryOperation *CreateBinaryOperation(Graph *graph, DataType::Type returnType, size_t pc) = delete;

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Min>(Graph *graph, DataType::Type returnType, size_t pc)
{
    return graph->CreateInstMin(returnType, pc);
}

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Max>(Graph *graph, DataType::Type returnType, size_t pc)
{
    return graph->CreateInstMax(returnType, pc);
}

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Mod>(Graph *graph, DataType::Type returnType, size_t pc)
{
    return graph->CreateInstMod(returnType, pc);
}

template void InstBuilder::BuildBinaryOperationIntrinsic<Opcode::Mod>(const BytecodeInstruction *bcInst, bool accRead);

template <Opcode OPCODE>
void InstBuilder::BuildBinaryOperationIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    [[maybe_unused]] auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    ASSERT(GetMethodArgumentsCount(methodId) == 2U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto inst = CreateBinaryOperation<OPCODE>(GetGraph(), GetMethodReturnType(methodId), GetPc(bcInst->GetAddress()));
    inst->SetInput(0, GetArgDefinition(bcInst, 0, accRead));
    inst->SetInput(1, GetArgDefinition(bcInst, 1, accRead));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildSqrtIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    [[maybe_unused]] auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto inst = GetGraph()->CreateInstSqrt(GetMethodReturnType(methodId), GetPc(bcInst->GetAddress()));
    ASSERT(GetMethodArgumentsCount(methodId) == 1);
    Inst *def = GetArgDefinition(bcInst, 0, accRead);
    inst->SetInput(0, def);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIsNanIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    // No need to create specialized node for isNaN. Since NaN != NaN, simple float compare node is fine.
    // Also, ensure that float comparison node is implemented for specific architecture
    auto vreg = GetArgDefinition(bcInst, 0, accRead);
    auto inst = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), vreg, vreg,
                                              GetMethodArgumentType(methodId, 0), ConditionCode::CC_NE);
    inst->SetOperandsType(GetMethodArgumentType(methodId, 0));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStringLengthIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);

    auto nullCheck =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, GetArgDefinition(bcInst, 0, accRead), saveState);
    auto arrayLength = graph_->CreateInstLenArray(DataType::INT32, bcAddr, nullCheck, false);

    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(arrayLength);

    Inst *stringLength;
    if (graph_->GetRuntime()->IsCompressedStringsEnabled()) {
        auto constOneInst = graph_->FindOrCreateConstant(1);
        stringLength = graph_->CreateInstShr(DataType::INT32, bcAddr, arrayLength, constOneInst);
        AddInstruction(stringLength);
    } else {
        stringLength = arrayLength;
    }
    UpdateDefinitionAcc(stringLength);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStringIsEmptyIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    auto nullCheck =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, GetArgDefinition(bcInst, 0, accRead), saveState);
    auto length = graph_->CreateInstLenArray(DataType::INT32, bcAddr, nullCheck, false);
    auto zeroConst = graph_->FindOrCreateConstant(0);
    auto checkZeroLength =
        graph_->CreateInstCompare(DataType::BOOL, bcAddr, length, zeroConst, DataType::INT32, ConditionCode::CC_EQ);
    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(length);
    AddInstruction(checkZeroLength);
    UpdateDefinitionAcc(checkZeroLength);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharIsUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    // IsUpperCase(char) = (char - 'A') < ('Z' - 'A')

    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    // Adding InstCast here as the aternative compiler backend requires inputs of InstSub to be of the same type.
    // The InstCast instructon makes no harm as normally they are removed by the following compiler stages.
    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('A');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('Z' - 'A'), DataType::UINT16, ConditionCode::CC_BE);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    UpdateDefinitionAcc(inst2);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharToUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    // ToUpperCase(char) = ((char - 'a') < ('z' - 'a')) * ('Z' - 'z') + char

    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('a');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('z' - 'a'), DataType::UINT16, ConditionCode::CC_BE);
    auto inst3 = GetGraph()->CreateInstMul(DataType::UINT16, GetPc(bcInst->GetAddress()), inst2,
                                           graph_->FindOrCreateConstant('Z' - 'z'));
    auto inst4 = GetGraph()->CreateInstAdd(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, inst3);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    AddInstruction(inst3);
    AddInstruction(inst4);
    UpdateDefinitionAcc(inst4);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharIsLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('a');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('z' - 'a'), DataType::UINT16, ConditionCode::CC_BE);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    UpdateDefinitionAcc(inst2);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharToLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('A');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('Z' - 'A'), DataType::UINT16, ConditionCode::CC_BE);
    auto inst3 = GetGraph()->CreateInstMul(DataType::UINT16, GetPc(bcInst->GetAddress()), inst2,
                                           graph_->FindOrCreateConstant('z' - 'Z'));
    auto inst4 = GetGraph()->CreateInstAdd(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, inst3);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    AddInstruction(inst3);
    AddInstruction(inst4);
    UpdateDefinitionAcc(inst4);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::GetArgDefinition(const BytecodeInstruction *bcInst, size_t idx, bool accRead, bool isRange)
{
    if (isRange) {
        return GetArgDefinitionRange(bcInst, idx);
    }
    if (accRead) {
        auto accPos = static_cast<size_t>(bcInst->GetImm64());
        if (idx < accPos) {
            return GetDefinition(bcInst->GetVReg(idx));
        }
        if (accPos == idx) {
            return GetDefinitionAcc();
        }
        return GetDefinition(bcInst->GetVReg(idx - 1));
    }
    return GetDefinition(bcInst->GetVReg(idx));
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::GetArgDefinitionRange(const BytecodeInstruction *bcInst, size_t idx)
{
    auto startReg = bcInst->GetVReg(0);
    return GetDefinition(startReg + idx);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMonitorIntrinsic(const BytecodeInstruction *bcInst, bool isEnter, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    [[maybe_unused]] auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    ASSERT(GetMethodReturnType(methodId) == DataType::VOID);
    ASSERT(GetMethodArgumentsCount(methodId) == 1);
    BuildMonitor(bcInst, GetArgDefinition(bcInst, 0, accRead), isEnter);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto method = GetRuntime()->GetMethodById(GetMethod(), methodId);
    auto intrinsicId = GetRuntime()->GetIntrinsicId(method);
    auto isVirtual = IsVirtual(intrinsicId);
    if (GetGraph()->IsBytecodeOptimizer() || !g_options.IsCompilerEncodeIntrinsics()) {
        BuildDefaultIntrinsic(isVirtual, bcInst, isRange, accRead);
        return;
    }
    if (!isVirtual) {
        return BuildStaticCallIntrinsic(bcInst, isRange, accRead);
    }
    return BuildVirtualCallIntrinsic(bcInst, isRange, accRead);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildDefaultIntrinsic(bool isVirtual, const BytecodeInstruction *bcInst, bool isRange, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto method = GetRuntime()->GetMethodById(GetMethod(), methodId);
    auto intrinsicId = GetRuntime()->GetIntrinsicId(method);
    if (intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER ||
        intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT) {
        BuildMonitorIntrinsic(bcInst, intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER,
                              accRead);
        return;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if (!isVirtual) {
        BuildDefaultStaticIntrinsic(bcInst, isRange, accRead);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        BuildDefaultVirtualCallIntrinsic(bcInst, isRange, accRead);
    }
}

// do not specify reason for tidy suppression because comment does not fit single line
// NOLINTNEXTLINE
void InstBuilder::BuildStaticCallIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto method = GetRuntime()->GetMethodById(GetMethod(), methodId);
    auto intrinsicId = GetRuntime()->GetIntrinsicId(method);
    switch (intrinsicId) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER:
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT: {
            BuildMonitorIntrinsic(bcInst, intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER,
                                  accRead);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F64: {
            BuildAbsIntrinsic(bcInst, accRead);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F64: {
            BuildSqrtIntrinsic(bcInst, accRead);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F64: {
            BuildBinaryOperationIntrinsic<Opcode::Min>(bcInst, accRead);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F64: {
            BuildBinaryOperationIntrinsic<Opcode::Max>(bcInst, accRead);
            break;
        }
#include "intrinsics_ir_build_static_call.inl"
        default: {
            BuildDefaultStaticIntrinsic(bcInst, isRange, accRead);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildDefaultVirtualCallIntrinsic(const BytecodeInstruction *bcInst, bool isRange, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto method = GetRuntime()->GetMethodById(GetMethod(), methodId);
    auto intrinsicId = GetRuntime()->GetIntrinsicId(method);
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    auto nullCheck =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, GetArgDefinition(bcInst, 0, accRead), saveState);

    auto call = GetGraph()->CreateInstIntrinsic(GetMethodReturnType(methodId), bcAddr, intrinsicId);
    SetCallArgs(bcInst, isRange, accRead, nullptr, call, nullCheck, call->RequireState() ? saveState : nullptr, true,
                methodId);

    AddInstruction(saveState);
    AddInstruction(nullCheck);

    /* if there are reference type args to be checked for NULL */
    AddArgNullcheckIfNeeded<true>(intrinsicId, call, saveState, bcAddr);

    AddInstruction(call);
    if (call->GetType() != DataType::VOID) {
        UpdateDefinitionAcc(call);
    } else {
        UpdateDefinitionAcc(nullptr);
    }
    if (NeedSafePointAfterIntrinsic(intrinsicId)) {
        AddInstruction(CreateSafePoint(currentBb_));
    }
}

// NOLINTNEXTLINE(readability-function-size,misc-definitions-in-headers)
template <bool IS_ACC_WRITE>
void InstBuilder::BuildLoadObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto pc = GetPc(bcInst->GetAddress());
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    // Create NullCheck instruction
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, pc,
                                                 GetDefinition(bcInst->GetVReg(IS_ACC_WRITE ? 0 : 1)), saveState);
    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    auto field = runtime->ResolveField(GetMethod(), fieldId, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    // Create LoadObject instruction
    Inst *inst;
    AddInstruction(saveState, nullCheck);
    if (field == nullptr || ForceUnresolved()) {
        // 1. Create an instruction to resolve an object's field
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), fieldId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        auto *resolveField =
            graph_->CreateInstResolveObjectField(DataType::UINT32, pc, saveState, fieldId, GetGraph()->GetMethod());
        AddInstruction(resolveField);
        // 2. Create an instruction to load a value from the resolved field
        auto loadField = graph_->CreateInstLoadResolvedObjectField(type, pc, nullCheck, resolveField, fieldId,
                                                                   GetGraph()->GetMethod());
        inst = loadField;
    } else {
        auto loadField = graph_->CreateInstLoadObject(type, pc, nullCheck, fieldId, GetGraph()->GetMethod(), field,
                                                      runtime->IsFieldVolatile(field));
        // 'final' field can be reassigned e. g. with reflection, but 'readonly' cannot
        // `IsInConstructor` check should not be necessary, but need proper frontend support first
        if (runtime->IsFieldReadonly(field) && !IsInConstructor</* IS_STATIC= */ false>()) {
            loadField->ClearFlag(inst_flags::NO_CSE);
        }
        inst = loadField;
    }

    AddInstruction(inst);
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_ACC_WRITE) {
        UpdateDefinitionAcc(inst);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        UpdateDefinition(bcInst->GetVReg(0), inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreObjectInst(const BytecodeInstruction *bcInst, DataType::Type type,
                                        RuntimeInterface::FieldPtr field, size_t fieldId, Inst **resolveInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    if (field == nullptr || ForceUnresolved()) {
        // The field is unresolved, so we have to resolve it and then store
        // 1. Create an instruction to resolve an object's field
        auto resolveField =
            graph_->CreateInstResolveObjectField(DataType::UINT32, pc, nullptr, fieldId, GetGraph()->GetMethod());
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), fieldId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        // 2. Create an instruction to store a value to the resolved field
        auto storeField = graph_->CreateInstStoreResolvedObjectField(
            type, pc, nullptr, nullptr, nullptr, fieldId, GetGraph()->GetMethod(), false, type == DataType::REFERENCE);
        *resolveInst = resolveField;
        return storeField;
    }

    ASSERT(field != nullptr);
    auto storeField = graph_->CreateInstStoreObject(type, pc, nullptr, nullptr, fieldId, GetGraph()->GetMethod(), field,
                                                    GetRuntime()->IsFieldVolatile(field), type == DataType::REFERENCE);
    *resolveInst = nullptr;  // resolver is not needed in this case
    return storeField;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool IS_ACC_READ>
void InstBuilder::BuildStoreObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));

    // Create NullCheck instruction
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bcInst->GetAddress()),
                                                 GetDefinition(bcInst->GetVReg(IS_ACC_READ ? 0 : 1)), saveState);

    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    auto field = runtime->ResolveField(GetMethod(), fieldId, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    // Get a value to store
    Inst *storeVal = nullptr;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_ACC_READ) {
        storeVal = GetDefinitionAcc();
    } else {  // NOLINT(readability-misleading-indentation)
        storeVal = GetDefinition(bcInst->GetVReg(0));
    }

    // Create StoreObject instruction
    Inst *resolveField = nullptr;
    Inst *storeField = BuildStoreObjectInst(bcInst, type, field, fieldId, &resolveField);
    storeField->SetInput(0, nullCheck);
    storeField->SetInput(1, storeVal);

    AddInstruction(saveState);
    AddInstruction(nullCheck);
    if (resolveField != nullptr) {
        ASSERT(field == nullptr || ForceUnresolved());
        resolveField->SetInput(0, saveState);
        storeField->SetInput(2U, resolveField);
        AddInstruction(resolveField);
    }
    AddInstruction(storeField);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadStaticInst(size_t pc, DataType::Type type, size_t typeId, Inst *saveState)
{
    uint32_t classId;
    auto field = GetRuntime()->ResolveField(GetMethod(), typeId, !GetGraph()->IsAotMode(), &classId);

    if (field == nullptr || ForceUnresolved()) {
        // The static field is unresolved, so we have to resolve it and then load
        // 1. Create an instruction to resolve an object's static field.
        //    Its result is a static field memory address (not an offset as there is no object)
        auto resolveField = graph_->CreateInstResolveObjectFieldStatic(DataType::REFERENCE, pc, saveState, typeId,
                                                                       GetGraph()->GetMethod());
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), typeId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        AddInstruction(resolveField);
        // 2. Create an instruction to load a value from the resolved static field address
        auto loadField =
            graph_->CreateInstLoadResolvedObjectFieldStatic(type, pc, resolveField, typeId, GetGraph()->GetMethod());
        return loadField;
    }

    ASSERT(field != nullptr);
    auto initClass = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, saveState, classId,
                                                        GetGraph()->GetMethod(), GetRuntime()->GetClassForField(field));
    auto loadField = graph_->CreateInstLoadStatic(type, pc, initClass, typeId, GetGraph()->GetMethod(), field,
                                                  GetRuntime()->IsFieldVolatile(field));
    // 'final' field can be reassigned e. g. with reflection, but 'readonly' cannot
    // `IsInConstructor` check should not be necessary, but need proper frontend support first
    if (GetRuntime()->IsFieldReadonly(field) && !IsInConstructor</* IS_STATIC= */ true>()) {
        loadField->ClearFlag(inst_flags::NO_CSE);
    }
    AddInstruction(initClass);
    return loadField;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildAnyTypeCheckInst(size_t bcAddr, Inst *input, Inst *saveState, AnyBaseType type,
                                         bool typeWasProfiled, profiling::AnyInputType allowedInputType)
{
    auto anyCheck = graph_->CreateInstAnyTypeCheck(DataType::ANY, bcAddr, input, saveState, type);
    anyCheck->SetAllowedInputType(allowedInputType);
    anyCheck->SetIsTypeWasProfiled(typeWasProfiled);
    AddInstruction(anyCheck);

    return anyCheck;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = GetRuntime()->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = GetRuntime()->GetFieldTypeById(GetMethod(), fieldId);
    }
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    AddInstruction(saveState);
    Inst *inst = BuildLoadStaticInst(GetPc(bcInst->GetAddress()), type, fieldId, saveState);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(readability-function-size,misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreStaticInst(const BytecodeInstruction *bcInst, DataType::Type type, size_t typeId,
                                        Inst *storeInput, Inst *saveState)
{
    AddInstruction(saveState);

    uint32_t classId;
    auto field = GetRuntime()->ResolveField(GetMethod(), typeId, !GetGraph()->IsAotMode(), &classId);
    auto pc = GetPc(bcInst->GetAddress());

    if (field == nullptr || ForceUnresolved()) {
        if (type == DataType::REFERENCE) {
            // 1. Class initialization is needed.
            // 2. GC Pre/Post write barriers may be needed.
            // Just call runtime EntrypointId::UNRESOLVED_STORE_STATIC_BARRIERED,
            // which performs all the necessary steps (see codegen.cpp for the details).
            auto inst = graph_->CreateInstUnresolvedStoreStatic(type, pc, storeInput, saveState, typeId,
                                                                GetGraph()->GetMethod(), true);
            return inst;
        }
        ASSERT(type != DataType::REFERENCE);
        // 1. Create an instruction to resolve an object's static field.
        //    Its result is a static field memory address (REFERENCE)
        auto resolveField = graph_->CreateInstResolveObjectFieldStatic(DataType::REFERENCE, pc, saveState, typeId,
                                                                       GetGraph()->GetMethod());
        AddInstruction(resolveField);
        // 2. Create an instruction to store a value to the resolved static field address
        auto storeField = graph_->CreateInstStoreResolvedObjectFieldStatic(type, pc, resolveField, storeInput, typeId,
                                                                           GetGraph()->GetMethod());
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), typeId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        return storeField;
    }

    ASSERT(field != nullptr);
    auto initClass = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, saveState, classId,
                                                        GetGraph()->GetMethod(), GetRuntime()->GetClassForField(field));
    auto storeField =
        graph_->CreateInstStoreStatic(type, pc, initClass, storeInput, typeId, GetGraph()->GetMethod(), field,
                                      GetRuntime()->IsFieldVolatile(field), type == DataType::REFERENCE);
    AddInstruction(initClass);
    return storeField;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = GetRuntime()->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = GetRuntime()->GetFieldTypeById(GetMethod(), fieldId);
    }
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    Inst *storeInput = GetDefinitionAcc();
    Inst *inst = BuildStoreStaticInst(bcInst, type, fieldId, storeInput, saveState);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildChecksBeforeArray(size_t pc, Inst *arrayRef, Inst **ss, Inst **nc, Inst **al, Inst **bc,
                                         bool withNullcheck)
{
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    Inst *nullCheck = nullptr;
    if (withNullcheck) {
        nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, pc, arrayRef, saveState);
    } else {
        nullCheck = arrayRef;
    }

    // Create LenArray instruction
    auto arrayLength = graph_->CreateInstLenArray(DataType::INT32, pc, nullCheck);

    // Create BoundCheck instruction
    auto boundsCheck = graph_->CreateInstBoundsCheck(DataType::INT32, pc, arrayLength, nullptr, saveState);

    *ss = saveState;
    *nc = nullCheck;
    *al = arrayLength;
    *bc = boundsCheck;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    ASSERT(type != DataType::NO_TYPE);
    Inst *saveState = nullptr;
    Inst *nullCheck = nullptr;
    Inst *arrayLength = nullptr;
    Inst *boundsCheck = nullptr;
    auto pc = GetPc(bcInst->GetAddress());
    BuildChecksBeforeArray(pc, GetDefinition(bcInst->GetVReg(0)), &saveState, &nullCheck, &arrayLength, &boundsCheck);
    ASSERT(saveState != nullptr && nullCheck != nullptr && arrayLength != nullptr && boundsCheck != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstLoadArray(type, pc, nullCheck, boundsCheck);
    boundsCheck->SetInput(1, GetDefinitionAcc());
    AddInstruction(saveState, nullCheck, arrayLength, boundsCheck, inst);
    UpdateDefinitionAcc(inst);
}

template <typename T>
void InstBuilder::BuildUnfoldLoadConstStringArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                                  const pandasm::LiteralArray &litArray, NewArrayInst *arrayInst)
{
    auto method = GetGraph()->GetMethod();
    auto arraySize = litArray.literals.size();
    for (size_t i = 0; i < arraySize; i++) {
        auto indexInst = graph_->FindOrCreateConstant(i);
        auto save = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        auto loadStringInst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bcInst->GetAddress()), save,
                                                               std::get<T>(litArray.literals[i].value), method);
        AddInstruction(save);
        AddInstruction(loadStringInst);
        if (GetGraph()->IsDynamicMethod()) {
            BuildCastToAnyString(bcInst);
        }

        BuildStoreArrayInst<false>(bcInst, type, arrayInst, indexInst, loadStringInst);
    }
}

template <typename T>
void InstBuilder::BuildUnfoldLoadConstArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                            const pandasm::LiteralArray &litArray)
{
    auto method = GetGraph()->GetMethod();
    auto arraySize = litArray.literals.size();
    auto typeId = GetRuntime()->GetLiteralArrayClassIdWithinFile(method, litArray.literals[0].tag);

    // Create NewArray instruction
    auto sizeInst = graph_->FindOrCreateConstant(arraySize);
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto negCheck = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bcInst->GetAddress()), sizeInst, saveState);
    auto initClass = CreateLoadAndInitClassGeneric(typeId, GetPc(bcInst->GetAddress()));
    initClass->SetInput(0, saveState);
    auto arrayInst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bcInst->GetAddress()), initClass, negCheck,
                                                saveState, typeId, GetGraph()->GetMethod());
    AddInstruction(saveState);
    AddInstruction(initClass);
    AddInstruction(negCheck);
    AddInstruction(arrayInst);
    UpdateDefinition(bcInst->GetVReg(0), arrayInst);

    if (arraySize > g_options.GetCompilerUnfoldConstArrayMaxSize()) {
        // Create LoadConstArray instruction
        auto ss = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        auto inst = GetGraph()->CreateInstFillConstArray(type, GetPc(bcInst->GetAddress()), arrayInst, ss,
                                                         bcInst->GetId(0).AsFileId().GetOffset(), method, arraySize);
        AddInstruction(ss);
        AddInstruction(inst);
        return;
    }

    // Create instructions for array filling
    auto tag = litArray.literals[0].tag;
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        for (size_t i = 0; i < arraySize; i++) {
            auto indexInst = graph_->FindOrCreateConstant(i);
            ConstantInst *valueInst;
            if (tag == panda_file::LiteralTag::ARRAY_F32) {
                valueInst = FindOrCreateFloatConstant(static_cast<float>(std::get<T>(litArray.literals[i].value)));
            } else if (tag == panda_file::LiteralTag::ARRAY_F64) {
                valueInst = FindOrCreateDoubleConstant(static_cast<double>(std::get<T>(litArray.literals[i].value)));
            } else {
                valueInst = FindOrCreateConstant(std::get<T>(litArray.literals[i].value));
            }

            BuildStoreArrayInst<false>(bcInst, type, arrayInst, indexInst, valueInst);
        }

        return;
    }
    [[maybe_unused]] auto arrayClass = GetRuntime()->ResolveType(method, typeId);
    ASSERT(GetRuntime()->CheckStoreArray(arrayClass, GetRuntime()->GetStringClass(method)));

    // Special case for string array
    BuildUnfoldLoadConstStringArray<T>(bcInst, type, litArray, arrayInst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadConstStringArray(const BytecodeInstruction *bcInst)
{
    auto literalArrayIdx = bcInst->GetId(0).AsIndex();
    auto litArray = GetRuntime()->GetLiteralArray(GetMethod(), literalArrayIdx);
    auto arraySize = litArray.literals.size();
    ASSERT(arraySize > 0);
    if (arraySize > g_options.GetCompilerUnfoldConstArrayMaxSize()) {
        // Create LoadConstArray instruction for String array, because we calls runtime for the case.
        auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        auto method = GetGraph()->GetMethod();
        auto inst = GetGraph()->CreateInstLoadConstArray(DataType::REFERENCE, GetPc(bcInst->GetAddress()), saveState,
                                                         literalArrayIdx, method);
        AddInstruction(saveState);
        AddInstruction(inst);
        UpdateDefinition(bcInst->GetVReg(0), inst);
    } else {
        BuildUnfoldLoadConstArray<uint32_t>(bcInst, DataType::REFERENCE, litArray);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadConstArray(const BytecodeInstruction *bcInst)
{
    auto literalArrayIdx = bcInst->GetId(0).AsIndex();
    auto litArray = GetRuntime()->GetLiteralArray(GetMethod(), literalArrayIdx);
    // Unfold LoadConstArray instruction
    auto tag = litArray.literals[0].tag;
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1: {
            BuildUnfoldLoadConstArray<bool>(bcInst, DataType::INT8, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U8: {
            BuildUnfoldLoadConstArray<uint8_t>(bcInst, DataType::INT8, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U16: {
            BuildUnfoldLoadConstArray<uint16_t>(bcInst, DataType::INT16, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U32: {
            BuildUnfoldLoadConstArray<uint32_t>(bcInst, DataType::INT32, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_U64: {
            BuildUnfoldLoadConstArray<uint64_t>(bcInst, DataType::INT64, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F32: {
            BuildUnfoldLoadConstArray<float>(bcInst, DataType::FLOAT32, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F64: {
            BuildUnfoldLoadConstArray<double>(bcInst, DataType::FLOAT64, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_STRING: {
            BuildLoadConstStringArray(bcInst);
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildStoreArrayInst<true>(bcInst, type, GetDefinition(bcInst->GetVReg(0)), GetDefinition(bcInst->GetVReg(1)),
                              GetDefinitionAcc());
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool CREATE_REF_CHECK>
void InstBuilder::BuildStoreArrayInst(const BytecodeInstruction *bcInst, DataType::Type type, Inst *arrayRef,
                                      Inst *index, Inst *value)
{
    ASSERT(type != DataType::NO_TYPE);
    Inst *refCheck = nullptr;
    Inst *saveState = nullptr;
    Inst *nullCheck = nullptr;
    Inst *arrayLength = nullptr;
    Inst *boundsCheck = nullptr;
    auto pc = GetPc(bcInst->GetAddress());
    BuildChecksBeforeArray(pc, arrayRef, &saveState, &nullCheck, &arrayLength, &boundsCheck);
    ASSERT(saveState != nullptr && nullCheck != nullptr && arrayLength != nullptr && boundsCheck != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstStoreArray(type, pc);
    boundsCheck->SetInput(1, index);
    auto storeDef = value;
    if (type == DataType::REFERENCE) {
        // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
        if constexpr (CREATE_REF_CHECK) {
            refCheck = graph_->CreateInstRefTypeCheck(DataType::REFERENCE, pc, nullCheck, storeDef, saveState);
            storeDef = refCheck;
        }
        inst->CastToStoreArray()->SetNeedBarrier(true);
    }
    inst->SetInput(0, nullCheck);
    inst->SetInput(1, boundsCheck);
    inst->SetInput(2U, storeDef);
    if (refCheck != nullptr) {
        AddInstruction(saveState, nullCheck, arrayLength, boundsCheck, refCheck, inst);
    } else {
        AddInstruction(saveState, nullCheck, arrayLength, boundsCheck, inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLenArray(const BytecodeInstruction *bcInst)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
    nullCheck->SetInput(0, GetDefinition(bcInst->GetVReg(0)));
    nullCheck->SetInput(1, saveState);
    auto inst = graph_->CreateInstLenArray(DataType::INT32, GetPc(bcInst->GetAddress()), nullCheck);
    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewArray(const BytecodeInstruction *bcInst)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto negCheck = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bcInst->GetAddress()),
                                                    GetDefinition(bcInst->GetVReg(1)), saveState);

    auto typeIndex = bcInst->GetId(0).AsIndex();
    auto typeId = GetRuntime()->ResolveTypeIndex(GetMethod(), typeIndex);

    auto initClass = CreateLoadAndInitClassGeneric(typeId, GetPc(bcInst->GetAddress()));
    initClass->SetInput(0, saveState);

    auto inst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bcInst->GetAddress()), initClass, negCheck,
                                           saveState, typeId, GetGraph()->GetMethod());
    AddInstruction(saveState, initClass, negCheck, inst);
    UpdateDefinition(bcInst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewObject(const BytecodeInstruction *bcInst)
{
    auto classId = GetRuntime()->ResolveTypeIndex(GetMethod(), bcInst->GetId(0).AsIndex());
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto initClass = CreateLoadAndInitClassGeneric(classId, pc);
    auto inst = CreateNewObjectInst(pc, classId, saveState, initClass);
    initClass->SetInput(0, saveState);
    AddInstruction(saveState, initClass, inst);
    UpdateDefinition(bcInst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMultiDimensionalArrayObject(const BytecodeInstruction *bcInst, bool isRange)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto pc = GetPc(bcInst->GetAddress());
    auto classId = GetRuntime()->GetClassIdForMethod(GetMethod(), methodId);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto initClass = CreateLoadAndInitClassGeneric(classId, pc);
    size_t argsCount = GetMethodArgumentsCount(methodId);
    auto inst = GetGraph()->CreateInstMultiArray(DataType::REFERENCE, pc, methodId);

    initClass->SetInput(0, saveState);

    inst->ReserveInputs(ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AppendInput(initClass);
    inst->AddInputType(DataType::REFERENCE);

    AddInstruction(saveState, initClass);

    if (isRange) {
        auto startReg = bcInst->GetVReg(0);
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            auto negCheck = graph_->CreateInstNegativeCheck(DataType::INT32, pc, GetDefinition(startReg), saveState);
            AddInstruction(negCheck);
            inst->AppendInput(negCheck);
            inst->AddInputType(DataType::INT32);
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            auto negCheck =
                graph_->CreateInstNegativeCheck(DataType::INT32, pc, GetDefinition(bcInst->GetVReg(i)), saveState);
            AddInstruction(negCheck);
            inst->AppendInput(negCheck);
            inst->AddInputType(DataType::INT32);
        }
    }
    inst->AppendInput(saveState);
    inst->AddInputType(DataType::NO_TYPE);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitObjectMultiDimensionalArray(const BytecodeInstruction *bcInst, bool isRange)
{
    auto pc = GetPc(bcInst->GetAddress());
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto classId = GetRuntime()->GetClassIdForMethod(GetMethod(), methodId);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto initClass =
        graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, saveState, classId, GetGraph()->GetMethod(),
                                           GetRuntime()->ResolveType(GetGraph()->GetMethod(), classId));
    auto inst = GetGraph()->CreateInstInitObject(DataType::REFERENCE, pc, methodId);

    size_t argsCount = GetMethodArgumentsCount(methodId);

    inst->ReserveInputs(ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AppendInput(initClass);
    inst->AddInputType(DataType::REFERENCE);
    if (isRange) {
        auto startReg = bcInst->GetVReg(0);
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            inst->AppendInput(GetDefinition(startReg));
            inst->AddInputType(GetMethodArgumentType(methodId, i));
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            inst->AppendInput(GetDefinition(bcInst->GetVReg(i)));
            inst->AddInputType(GetMethodArgumentType(methodId, i));
        }
    }
    inst->AppendInput(saveState);
    inst->AddInputType(DataType::NO_TYPE);
    inst->SetCallMethod(GetRuntime()->GetMethodById(GetGraph()->GetMethod(), methodId));

    AddInstruction(saveState, initClass, inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
CallInst *InstBuilder::BuildCallStaticForInitObject(const BytecodeInstruction *bcInst, uint32_t methodId,
                                                    Inst **resolver)
{
    auto pc = GetPc(bcInst->GetAddress());
    size_t argsCount = GetMethodArgumentsCount(methodId);
    size_t inputsCount = ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE;
    auto method = GetRuntime()->GetMethodById(graph_->GetMethod(), methodId);
    CallInst *call = nullptr;
    if (method == nullptr || ForceUnresolved()) {
        ResolveStaticInst *resolveStatic = graph_->CreateInstResolveStatic(DataType::POINTER, pc, methodId, nullptr);
        *resolver = resolveStatic;
        call = graph_->CreateInstCallResolvedStatic(GetMethodReturnType(methodId), pc, methodId);
        if (!graph_->IsAotMode() && !graph_->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), methodId,
                                                             UnresolvedTypesInterface::SlotKind::METHOD);
        }
        inputsCount += 1;  // resolver
    } else {
        call = graph_->CreateInstCallStatic(GetMethodReturnType(methodId), pc, methodId, method);
    }
    call->ReserveInputs(inputsCount);
    call->AllocateInputTypes(graph_->GetAllocator(), inputsCount);
    return call;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitString(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    AddInstruction(saveState);

    auto ctorMethodIndex = bcInst->GetId(0).AsIndex();
    auto ctorMethodId = GetRuntime()->ResolveMethodIndex(GetMethod(), ctorMethodIndex);
    size_t argsCount = GetMethodArgumentsCount(ctorMethodId);

    Inst *inst = nullptr;
    if (argsCount == 0) {
        inst = GetGraph()->CreateInstInitEmptyString(DataType::REFERENCE, pc, saveState);
    } else {
        ASSERT(argsCount == 1);
        auto nullCheck =
            graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bcInst->GetVReg(0)), saveState);
        AddInstruction(nullCheck);

        auto ctorMethod = GetRuntime()->GetMethodById(GetMethod(), ctorMethodId);
        auto ctorType = GetRuntime()->GetStringCtorType(ctorMethod);
        inst = GetGraph()->CreateInstInitString(DataType::REFERENCE, pc, nullCheck, saveState, ctorType);
    }
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitObject(const BytecodeInstruction *bcInst, bool isRange)
{
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex());
    auto typeId = GetRuntime()->GetClassIdForMethod(GetMethod(), methodId);
    if (GetRuntime()->IsArrayClass(GetMethod(), typeId)) {
        if (GetGraph()->IsBytecodeOptimizer()) {
            BuildInitObjectMultiDimensionalArray(bcInst, isRange);
        } else {
            BuildMultiDimensionalArrayObject(bcInst, isRange);
        }
        return;
    }

    if (GetRuntime()->IsStringClass(GetMethod(), typeId) && !GetGraph()->IsBytecodeOptimizer()) {
        BuildInitString(bcInst);
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto initClass = CreateLoadAndInitClassGeneric(typeId, pc);
    initClass->SetInput(0, saveState);
    auto newObj = CreateNewObjectInst(pc, typeId, saveState, initClass);
    UpdateDefinitionAcc(newObj);
    Inst *resolver = nullptr;
    CallInst *call = BuildCallStaticForInitObject(bcInst, methodId, &resolver);
    if (resolver != nullptr) {
        call->AppendInput(resolver);
        call->AddInputType(DataType::POINTER);
    }
    call->AppendInput(newObj);
    call->AddInputType(DataType::REFERENCE);

    size_t argsCount = GetMethodArgumentsCount(methodId);
    if (isRange) {
        auto startReg = bcInst->GetVReg(0);
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            call->AppendInput(GetDefinition(startReg));
            call->AddInputType(GetMethodArgumentType(methodId, i));
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            call->AppendInput(GetDefinition(bcInst->GetVReg(i)));
            call->AddInputType(GetMethodArgumentType(methodId, i));
        }
    }
    auto saveStateForCall = CreateSaveState(Opcode::SaveState, pc);
    call->AppendInput(saveStateForCall);
    call->AddInputType(DataType::NO_TYPE);
    if (resolver != nullptr) {
        resolver->SetInput(0, saveStateForCall);
        AddInstruction(saveState, initClass, newObj, saveStateForCall, resolver, call);
    } else {
        AddInstruction(saveState, initClass, newObj, saveStateForCall, call);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCheckCast(const BytecodeInstruction *bcInst)
{
    auto typeIndex = bcInst->GetId(0).AsIndex();
    auto typeId = GetRuntime()->ResolveTypeIndex(GetMethod(), typeIndex);
    auto klassType = GetRuntime()->GetClassType(GetGraph()->GetMethod(), typeId);
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto loadClass = BuildLoadClass(typeId, pc, saveState);
    auto inst = GetGraph()->CreateInstCheckCast(DataType::NO_TYPE, pc, GetDefinitionAcc(), loadClass, saveState, typeId,
                                                GetGraph()->GetMethod(), klassType);
    AddInstruction(saveState, loadClass, inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIsInstance(const BytecodeInstruction *bcInst)
{
    auto typeIndex = bcInst->GetId(0).AsIndex();
    auto typeId = GetRuntime()->ResolveTypeIndex(GetMethod(), typeIndex);
    auto klassType = GetRuntime()->GetClassType(GetGraph()->GetMethod(), typeId);
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto loadClass = BuildLoadClass(typeId, pc, saveState);
    auto inst = GetGraph()->CreateInstIsInstance(DataType::BOOL, pc, GetDefinitionAcc(), loadClass, saveState, typeId,
                                                 GetGraph()->GetMethod(), klassType);
    AddInstruction(saveState, loadClass, inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadClass(RuntimeInterface::IdType typeId, size_t pc, Inst *saveState)
{
    auto inst =
        GetGraph()->CreateInstLoadClass(DataType::REFERENCE, pc, saveState, typeId, GetGraph()->GetMethod(), nullptr);
    auto klass = GetRuntime()->ResolveType(GetGraph()->GetMethod(), typeId);
    if (klass != nullptr) {
        inst->SetClass(klass);
    } else if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
        GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetGraph()->GetMethod(), typeId,
                                                         UnresolvedTypesInterface::SlotKind::CLASS);
    }
    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildThrow(const BytecodeInstruction *bcInst)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto inst = graph_->CreateInstThrow(DataType::NO_TYPE, GetPc(bcInst->GetAddress()),
                                        GetDefinition(bcInst->GetVReg(0)), saveState);
    inst->SetCallMethod(GetMethod());
    AddInstruction(saveState);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
void InstBuilder::BuildLoadFromPool(const BytecodeInstruction *bcInst)
{
    auto method = GetGraph()->GetMethod();
    uint32_t typeId;
    Inst *inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (OPCODE == Opcode::LoadType) {
        auto typeIndex = bcInst->GetId(0).AsIndex();
        typeId = GetRuntime()->ResolveTypeIndex(method, typeIndex);
        if (GetRuntime()->ResolveType(method, typeId) == nullptr) {
            inst = GetGraph()->CreateInstUnresolvedLoadType(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
            if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
                GetRuntime()->GetUnresolvedTypes()->AddTableSlot(method, typeId,
                                                                 UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
            }
        } else {
            inst = GetGraph()->CreateInstLoadType(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers)
        static_assert(OPCODE == Opcode::LoadString);
        typeId = bcInst->GetId(0).AsFileId().GetOffset();
        if (!GetGraph()->IsDynamicMethod() || GetGraph()->IsBytecodeOptimizer()) {
            inst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
        } else {
            inst = GetGraph()->CreateInstLoadFromConstantPool(DataType::ANY, GetPc(bcInst->GetAddress()));
            inst->CastToLoadFromConstantPool()->SetString(true);
        }
    }
    if (!GetGraph()->IsDynamicMethod() || GetGraph()->IsBytecodeOptimizer()) {
        // Create SaveState instruction
        auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        inst->SetInput(0, saveState);
        static_cast<LoadFromPool *>(inst)->SetTypeId(typeId);
        static_cast<LoadFromPool *>(inst)->SetMethod(method);
        AddInstruction(saveState);
    } else {
        inst->SetInput(0, GetEnvDefinition(CONST_POOL_IDX));
        inst->CastToLoadFromConstantPool()->SetTypeId(typeId);
        inst->CastToLoadFromConstantPool()->SetMethod(method);
    }

    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::LoadString) {
        if (GetGraph()->IsDynamicMethod() && GetGraph()->IsBytecodeOptimizer()) {
            BuildCastToAnyString(bcInst);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyString(const BytecodeInstruction *bcInst)
{
    auto input = GetDefinitionAcc();
    ASSERT(input->GetType() == DataType::REFERENCE);

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto anyType = GetAnyStringType(language);
    ASSERT(anyType != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bcInst->GetAddress()), anyType, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyNumber(const BytecodeInstruction *bcInst)
{
    auto input = GetDefinitionAcc();
    auto type = input->GetType();
    if (input->IsConst() && !DataType::IsFloatType(type)) {
        auto constInsn = input->CastToConstant();
        if (constInsn->GetType() == DataType::INT64) {
            auto value = input->CastToConstant()->GetInt64Value();
            if (value == static_cast<uint32_t>(value)) {
                type = DataType::INT32;
            }
        }
    }

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto anyType = NumericDataTypeToAnyType(type, language);
    ASSERT(anyType != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bcInst->GetAddress()), anyType, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
bool InstBuilder::TryBuildStringCharAtIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto compressionEnabled = graph_->GetRuntime()->IsCompressedStringsEnabled();
    auto canEncodeCompressedStrCharAt = graph_->GetEncoder()->CanEncodeCompressedStringCharAt();
    if (compressionEnabled && !canEncodeCompressedStrCharAt) {
        return false;
    }
    auto saveStateNullcheck = CreateSaveState(Opcode::SaveState, bcAddr);
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, GetArgDefinition(bcInst, 0, accRead),
                                                 saveStateNullcheck);
    auto arrayLength = graph_->CreateInstLenArray(DataType::INT32, bcAddr, nullCheck, false);

    AddInstruction(saveStateNullcheck);
    AddInstruction(nullCheck);
    AddInstruction(arrayLength);

    Inst *stringLength = nullptr;
    if (compressionEnabled) {
        auto constOneInst = graph_->FindOrCreateConstant(1);
        stringLength = graph_->CreateInstShr(DataType::INT32, bcAddr, arrayLength, constOneInst);
        AddInstruction(stringLength);
    } else {
        stringLength = arrayLength;
    }

    auto boundsCheck = graph_->CreateInstBoundsCheck(DataType::INT32, bcAddr, stringLength,
                                                     GetArgDefinition(bcInst, 1, accRead), saveStateNullcheck, false);
    AddInstruction(boundsCheck);

    Inst *inst = nullptr;
    if (compressionEnabled) {
        inst =
            graph_->CreateInstLoadCompressedStringChar(DataType::UINT16, bcAddr, nullCheck, boundsCheck, arrayLength);
    } else {
        inst = graph_->CreateInstLoadArray(DataType::UINT16, bcAddr, nullCheck, boundsCheck);
    }

    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    return true;
}

}  // namespace ark::compiler

#endif  // PANDA_INST_BUILDER_INL_H
