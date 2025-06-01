/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "compiler/optimizer/code_generator/codegen.h"

namespace ark::compiler {

inline void Codegen::PrepareForCallLaunchVirtual(CallInst *callInst, RuntimeInterface::MethodPtr method, Reg &thisReg,
                                                 Reg &param0)
{
    thisReg = ConvertRegister(callInst->GetLocation(1).GetValue(), DataType::REFERENCE);
    LoadClassFromObject(param0, thisReg);
    // Get index
    auto vtableIndex = GetRuntime()->GetVTableIndex(method);
    // Load from VTable, address = klass + ((index << shift) + vtable_offset)
    auto totalOffset = GetRuntime()->GetVTableOffset(GetArch()) + (vtableIndex << GetVtableShift());
    // Class ref was loaded to method_reg
    GetEncoder()->EncodeLdr(param0, false, MemRef(param0, totalOffset));
}

bool Codegen::LaunchCallCodegen(CallInst *callInst)
{
    SCOPED_DISASM_STR(this, "Create Launch Call");

    auto dstReg = ConvertRegister(callInst->GetDstReg(), callInst->GetType());

    Reg param0 = GetTarget().GetParamReg(0);
    ScopedTmpRegLazy tmp(GetEncoder());

    RegMask liveRegs {GetLiveRegisters(callInst).first};
    if (liveRegs.Test(param0.GetId())) {
        tmp.Acquire();
        param0 = tmp;
    }

    Reg objReg;
    Reg thisReg;
    if (callInst->GetOpcode() == Opcode::CallResolvedLaunchStatic ||
        callInst->GetOpcode() == Opcode::CallResolvedLaunchVirtual) {
        auto location = callInst->GetLocation(0);
        ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

        param0 = ConvertRegister(location.GetValue(), DataType::POINTER);
        auto location1 = callInst->GetLocation(1);
        ASSERT(location1.IsFixedRegister() && location1.IsRegisterValid());

        objReg = ConvertRegister(location1.GetValue(), DataType::REFERENCE);
        if (callInst->GetOpcode() == Opcode::CallResolvedLaunchVirtual) {
            thisReg = ConvertRegister(callInst->GetLocation(2U).GetValue(), DataType::REFERENCE);
        }
    } else {
        auto location = callInst->GetLocation(0);
        ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

        objReg = ConvertRegister(location.GetValue(), DataType::REFERENCE);

        auto method = callInst->GetCallMethod();
        if (callInst->GetOpcode() == Opcode::CallLaunchStatic) {
            ASSERT(!GetGraph()->IsAotMode());
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<size_t>(method)));
        } else {
            ASSERT(callInst->GetOpcode() == Opcode::CallLaunchVirtual);
            PrepareForCallLaunchVirtual(callInst, method, thisReg, param0);
        }
    }

    if (callInst->IsStaticLaunchCall()) {
        CallRuntime(callInst, EntrypointId::CREATE_LAUNCH_STATIC_COROUTINE, dstReg, RegMask::GetZeroMask(), param0,
                    objReg, SpReg());
    } else {
        CallRuntime(callInst, EntrypointId::CREATE_LAUNCH_VIRTUAL_COROUTINE, dstReg, RegMask::GetZeroMask(), param0,
                    objReg, SpReg(), thisReg);
    }
    if (callInst->GetFlag(inst_flags::MEM_BARRIER)) {
        GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
    return true;
}

void Codegen::EtsGetNativeMethod(IntrinsicInst *inst, Reg dst, [[maybe_unused]] SRCREGS &src)
{
    auto *encoder = GetEncoder();
    ScopedTmpReg methodReg(encoder);

    if (GetGraph()->IsJitOrOsrMode()) {
        auto *method = inst->GetCallMethod();
        encoder->EncodeMov(methodReg, Imm(bit_cast<uintptr_t>(method)));
    } else {
        ScopedTmpReg addrReg(encoder);
        auto *aotData = GetGraph()->GetAotData();
        auto methodId = inst->GetCallMethodId();
        intptr_t offset = aotData->GetCommonSlotOffset(encoder->GetCursorOffset(), methodId);
        encoder->MakeLoadAotTableAddr(offset, addrReg, methodReg);

        auto skipLabel = encoder->CreateLabel();
        encoder->EncodeJump(skipLabel, methodReg, Condition::NE);
        LoadMethod(methodReg);
        CallRuntime(inst, EntrypointId::GET_CALLEE_METHOD, methodReg, RegMask::GetZeroMask(), methodReg,
                    TypedImm(methodId));
        encoder->EncodeStr(methodReg, MemRef(addrReg));
        encoder->BindLabel(skipLabel);
    }

    auto successLabel = encoder->CreateLabel();
    auto failLabel = encoder->CreateLabel();

    // If native method is not registered, deoptimize
    ScopedTmpReg tmpReg(encoder);
    encoder->EncodeLdr(tmpReg, false, MemRef(methodReg, GetRuntime()->GetNativePointerOffset(GetArch())));
    encoder->EncodeJump(failLabel, tmpReg, Condition::EQ);

    // Don't support deprecated Native API and currently Function native mode in new API
    // NOTE: add Function mode support
    if (GetRuntime()->CanNativeMethodUseObjects(inst->GetCallMethod())) {
        tmpReg.ChangeType(INT32_TYPE);
        encoder->EncodeLdr(tmpReg, false, MemRef(methodReg, GetRuntime()->GetAccessFlagsOffset(GetArch())));
        auto unsupportedMask = GetRuntime()->GetDeprecatedNativeApiMask();
        encoder->EncodeAnd(tmpReg, tmpReg, Imm(unsupportedMask));
        encoder->EncodeJump(failLabel, tmpReg, Condition::NE);
    }
    encoder->EncodeMov(dst, methodReg);
    encoder->EncodeJump(successLabel);

    encoder->BindLabel(failLabel);
    encoder->EncodeMov(dst, Imm(0));

    encoder->BindLabel(successLabel);
}

void Codegen::EtsGetNativeMethodManagedClass(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    if (GetGraph()->IsJitOrOsrMode()) {
        auto *method = inst->GetCallMethod();
        auto runtimeClass = bit_cast<uintptr_t>(GetRuntime()->GetClass(method));
        auto managedClass = runtimeClass - GetRuntime()->GetRuntimeClassOffset(GetArch());
        GetEncoder()->EncodeMov(dst, Imm(managedClass));
    } else {
        auto methodReg = src[0U];
        GetEncoder()->EncodeLdr(dst, false, MemRef(methodReg, GetRuntime()->GetClassOffset(GetArch())));
        GetEncoder()->EncodeSub(dst, dst, Imm(GetRuntime()->GetRuntimeClassOffset(GetArch())));
    }
}

void Codegen::EtsGetMethodNativePointer(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    auto *method = inst->GetCallMethod();
    auto *nativePointer = GetRuntime()->GetMethodNativePointer(method);
    if (GetGraph()->IsJitOrOsrMode() && nativePointer != nullptr) {
        GetEncoder()->EncodeMov(dst, Imm(bit_cast<uintptr_t>(nativePointer)));
    } else {
        auto methodReg = src[0U];
        GetEncoder()->EncodeLdr(dst, false, MemRef(methodReg, GetRuntime()->GetNativePointerOffset(GetArch())));
    }
}

void Codegen::EtsGetNativeApiEnv([[maybe_unused]] IntrinsicInst *inst, Reg dst, [[maybe_unused]] SRCREGS &src)
{
    GetEncoder()->EncodeLdr(dst, false, MemRef(ThreadReg(), GetRuntime()->GetTlsNativeApiOffset(GetArch())));
}

void Codegen::EtsBeginNativeMethod(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS &src)
{
    ASSERT(!HasLiveCallerSavedRegs(inst));
    ASSERT(inst->GetSaveState()->GetRootsRegsMask().none());

    {
        SCOPED_DISASM_STR(this, "Prepare for begin native");
        GetEncoder()->EncodeStr(FpReg(), MemRef(ThreadReg(), GetRuntime()->GetTlsFrameOffset(GetArch())));

        CreateStackMap(inst);
        ScopedTmpReg pc(GetEncoder());
        GetEncoder()->EncodeGetCurrentPc(pc);
        GetEncoder()->EncodeStr(pc, MemRef(ThreadReg(), GetRuntime()->GetTlsNativePcOffset(GetArch())));
    }

    {
        SCOPED_DISASM_STR(this, "Call begin native intrinsic");
        bool switchesToNative = GetRuntime()->IsNecessarySwitchThreadState(inst->GetCallMethod());
        auto id =
            switchesToNative ? EntrypointId::BEGIN_GENERAL_NATIVE_METHOD : EntrypointId::BEGIN_QUICK_NATIVE_METHOD;
        MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), id));
        GetEncoder()->MakeCall(entry);
    }
}

void Codegen::EtsEndNativeMethodPrim(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    EtsEndNativeMethod(inst, dst, src, false);
}

void Codegen::EtsEndNativeMethodObj(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    EtsEndNativeMethod(inst, dst, src, true);
}

void Codegen::EtsEndNativeMethod(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS &src,
                                 bool isObject)
{
    ASSERT(!HasLiveCallerSavedRegs(inst));
    ASSERT(inst->GetSaveState()->GetRootsRegsMask().none());

    {
        SCOPED_DISASM_STR(this, "Call end native intrinsic");
        bool switchesToNative = GetRuntime()->IsNecessarySwitchThreadState(inst->GetCallMethod());
        EntrypointId id = EntrypointId::INVALID;
        if (switchesToNative) {
            id = isObject ? EntrypointId::END_GENERAL_NATIVE_METHOD_OBJ : EntrypointId::END_GENERAL_NATIVE_METHOD_PRIM;
        } else {
            id = isObject ? EntrypointId::END_QUICK_NATIVE_METHOD_OBJ : EntrypointId::END_QUICK_NATIVE_METHOD_PRIM;
        }
        MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), id));
        GetEncoder()->MakeCall(entry);
    }
}

void Codegen::EtsCheckNativeException(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS &src)
{
    ASSERT(inst->CanThrow());
    auto throwLabel = CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::THROW_NATIVE_EXCEPTION)->GetLabel();

    ScopedTmpReg tmpReg(GetEncoder());
    GetEncoder()->EncodeLdr(tmpReg, false, MemRef(ThreadReg(), GetRuntime()->GetExceptionOffset(GetArch())));
    GetEncoder()->EncodeJump(throwLabel, tmpReg, Condition::NE);
}

void Codegen::EtsWrapObjectNative(WrapObjectNativeInst *wrapObject)
{
    Inst *obj = wrapObject->GetDataFlowInput(0);
    ASSERT(obj != nullptr);
    CallInst *callNative = wrapObject->GetUsers().Front().GetInst()->CastToCallNative();
    ASSERT(callNative != nullptr);

    ASSERT(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    auto targetLifeNumber = la.GetInstLifeIntervals(callNative)->GetBegin();

    auto *interval = la.GetInstLifeIntervals(obj)->FindSiblingAt(targetLifeNumber);
    ASSERT(interval != nullptr);
    auto location = interval->GetLocation();
    ASSERT(location.GetKind() == LocationType::STACK_PARAMETER || location.GetKind() == LocationType::STACK);

    auto dstReg = ConvertRegister(wrapObject->GetDstReg(), wrapObject->GetType());
    GetEncoder()->EncodeAdd(dstReg, SpReg(), Imm(GetStackOffset(location)));
    // don't apply stack ref mask, since it is empty
    ASSERT(GetRuntime()->GetStackReferenceMask() == 0U);
}

bool Codegen::ResolveCallByNameCodegen(ResolveVirtualInst *resolver)
{
    SCOPED_DISASM_STR(this, "Create runtime call to resolve a call by name");
    ASSERT(resolver->GetOpcode() == Opcode::ResolveByName);
    ASSERT(resolver->GetCallMethod() != nullptr);

    auto methodReg = ConvertRegister(resolver->GetDstReg(), resolver->GetType());
    auto objectReg = ConvertRegister(resolver->GetSrcReg(0), DataType::REFERENCE);
    ScopedTmpReg tmpMethodReg(GetEncoder());
    LoadMethod(tmpMethodReg);

    CallRuntime(resolver, EntrypointId::RESOLVE_CALL_BY_NAME, tmpMethodReg, {}, tmpMethodReg, objectReg,
                TypedImm(resolver->GetCallMethodId()));
    GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    return true;
}

}  // namespace ark::compiler
