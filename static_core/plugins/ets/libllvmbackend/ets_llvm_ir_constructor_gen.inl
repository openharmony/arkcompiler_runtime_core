/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

bool LLVMIrConstructor::EmitArrayCopyTo(Inst *inst)
{
    RuntimeInterface::EntrypointId eid;
    switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BOOL_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_1B;
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_2B;
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_4B;
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_8B;
            break;
        default:
            UNREACHABLE();
            break;
    }

    CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), GetInputValue(inst, 1),
                                   GetInputValue(inst, 2), GetInputValue(inst, 3),
                                   GetInputValue(inst, 4)});
    // Fastpath doesn't return anything, but result is in 'dst' arg, which is second
    ValueMapAdd(inst, GetInputValue(inst, 1));
    return true;
}

bool LLVMIrConstructor::EmitStdStringSubstring(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::SUB_STRING_FROM_STRING_TLAB_COMPRESSED, 3U);
}

bool LLVMIrConstructor::EmitStringBuilderAppendBool(Inst *inst)
{
    auto offset = GetGraph()->GetRuntime()->GetArrayU16ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_BOOL;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), GetInputValue(inst, 1), klass});
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendChar(Inst *inst)
{
    auto offset = GetGraph()->GetRuntime()->GetArrayU16ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_CHAR_COMPRESSED;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), GetInputValue(inst, 1), klass});
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendByte(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendShort(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendInt(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendLong(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

llvm::Value *LLVMIrConstructor::CreateStringBuilderAppendLong(Inst *inst)
{
    auto sb = GetInputValue(inst, 0);
    auto value = builder_.CreateSExt(GetInputValue(inst, 1), builder_.getInt64Ty());
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_LONG;
    auto offset = GetGraph()->GetRuntime()->GetArrayU16ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    return CreateFastPathCall(inst, eid, {sb, value, klass});
}
