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

#include "optimizer/ir_builder/inst_builder.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/ir/inst.h"
#include "bytecode_instruction.h"
#include "bytecode_instruction-inl.h"

namespace panda::compiler {

template <RuntimeInterface::IntrinsicId ID, DataType::Type RET_TYPE, DataType::Type... PARAM_TYPES>
struct IntrinsicBuilder {
    static constexpr size_t N = sizeof...(PARAM_TYPES);
    template <typename... ARGS>
    static IntrinsicInst *Build(InstBuilder *ib, size_t pc, const ARGS &...inputs)
    {
        static_assert(sizeof...(inputs) == N);
        return ib->BuildInteropIntrinsic<N>(pc, ID, RET_TYPE, {PARAM_TYPES...}, {inputs...});
    }
};

using IntrinsicCompilerConvertJSValueToLocal =
    IntrinsicBuilder<RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_JS_VALUE_TO_LOCAL, DataType::POINTER,
                     DataType::REFERENCE>;
using IntrinsicCompilerResolveQualifiedJSCall =
    IntrinsicBuilder<RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_RESOLVE_QUALIFIED_JS_CALL, DataType::POINTER,
                     DataType::POINTER, DataType::REFERENCE>;
using IntrinsicCompilerGetJSNamedProperty =
    IntrinsicBuilder<RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_JS_NAMED_PROPERTY, DataType::POINTER,
                     DataType::POINTER, DataType::POINTER>;
using IntrinsicCompilerConvertLocalToJSValue =
    IntrinsicBuilder<RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_JS_VALUE, DataType::REFERENCE,
                     DataType::POINTER>;
using IntrinsicCompilerCreateLocalScope =
    IntrinsicBuilder<RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CREATE_LOCAL_SCOPE, DataType::VOID>;
using IntrinsicCompilerDestroyLocalScope =
    IntrinsicBuilder<RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_DESTROY_LOCAL_SCOPE, DataType::VOID>;
using IntrinsicCompilerJSCallCheck = IntrinsicBuilder<RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_JS_CALL_CHECK,
                                                      DataType::POINTER, DataType::POINTER>;

template <size_t N>
IntrinsicInst *InstBuilder::BuildInteropIntrinsic(size_t pc, RuntimeInterface::IntrinsicId id, DataType::Type ret_type,
                                                  const std::array<DataType::Type, N> &types,
                                                  const std::array<Inst *, N> &inputs)
{
    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    AddInstruction(save_state);
    auto intrinsic = GetGraph()->CreateInstIntrinsic(ret_type, pc, id);
    intrinsic->AllocateInputTypes(GetGraph()->GetAllocator(), N + 1);
    for (size_t i = 0; i < N; ++i) {
        intrinsic->AppendInput(inputs[i]);
        intrinsic->AddInputType(types[i]);
    }
    intrinsic->AppendInput(save_state);
    intrinsic->AddInputType(DataType::NO_TYPE);
    AddInstruction(intrinsic);
    return intrinsic;
}

std::pair<Inst *, Inst *> InstBuilder::BuildResolveInteropCallIntrinsic(RuntimeInterface::InteropCallKind call_kind,
                                                                        size_t pc, RuntimeInterface::MethodPtr method,
                                                                        Inst *arg0, Inst *arg1)
{
    IntrinsicInst *js_this = nullptr;
    IntrinsicInst *js_fn = nullptr;
    if (call_kind == RuntimeInterface::InteropCallKind::CALL_BY_VALUE) {
        js_fn = IntrinsicCompilerConvertJSValueToLocal::Build(this, pc, arg0);
        js_this = IntrinsicCompilerConvertJSValueToLocal::Build(this, pc, arg1);
    } else {
        auto js_val = IntrinsicCompilerConvertJSValueToLocal::Build(this, pc, arg0);

        auto str_id = arg1->CastToLoadString()->GetTypeId();
        js_this = IntrinsicCompilerResolveQualifiedJSCall::Build(this, pc, js_val, arg1);

        Inst *prop_name = nullptr;
        if (!GetGraph()->IsAotMode()) {
            auto prop_name_str = GetGraph()->GetRuntime()->GetFuncPropName(method, str_id);
            prop_name = GetGraph()->CreateInstLoadImmediate(DataType::POINTER, pc, prop_name_str,
                                                            LoadImmediateInst::ObjectType::STRING);
        } else {
            auto prop_name_str_offset = GetGraph()->GetRuntime()->GetFuncPropNameOffset(method, str_id);
            prop_name = GetGraph()->CreateInstLoadImmediate(DataType::POINTER, pc, prop_name_str_offset,
                                                            LoadImmediateInst::ObjectType::PANDA_FILE_OFFSET);
        }
        AddInstruction(prop_name);

        js_fn = IntrinsicCompilerGetJSNamedProperty::Build(this, pc, js_this, prop_name);
    }
    return {js_this, js_fn};
}

void InstBuilder::BuildReturnValueConvertInteropIntrinsic(RuntimeInterface::InteropCallKind call_kind, size_t pc,
                                                          RuntimeInterface::MethodPtr method, Inst *js_call)
{
    if (call_kind == RuntimeInterface::InteropCallKind::NEW_INSTANCE) {
        auto ret = IntrinsicCompilerConvertLocalToJSValue::Build(this, pc, js_call);
        UpdateDefinitionAcc(ret);
    } else {
        auto ret_intrinsic_id = GetGraph()->GetRuntime()->GetInfoForInteropCallRetValueConversion(method);
        if (ret_intrinsic_id.has_value()) {
            Inst *ret = nullptr;
            auto [id, ret_type] = ret_intrinsic_id.value();
            if (id == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CONVERT_LOCAL_TO_REF_TYPE) {
                auto save_state = CreateSaveState(Opcode::SaveState, pc);
                AddInstruction(save_state);
                auto load_class = BuildLoadClass(GetRuntime()->GetMethodReturnTypeId(method), pc, save_state);
                AddInstruction(load_class);
                ret = BuildInteropIntrinsic<2>(pc, id, ret_type, {DataType::REFERENCE, DataType::POINTER},
                                               {load_class, js_call});
            } else {
                ret = BuildInteropIntrinsic<1>(pc, id, ret_type, {DataType::POINTER}, {js_call});
            }
            UpdateDefinitionAcc(ret);
        }
    }
}

void InstBuilder::BuildInteropCall(const BytecodeInstruction *bc_inst, RuntimeInterface::InteropCallKind call_kind,
                                   RuntimeInterface::MethodPtr method, bool is_range, bool acc_read)
{
    auto pc = GetPc(bc_inst->GetAddress());

    // Create LOCAL scope
    IntrinsicCompilerCreateLocalScope::Build(this, pc);
    // Resolve call target
    auto [js_this, js_fn] =
        BuildResolveInteropCallIntrinsic(call_kind, pc, method, GetArgDefinition(bc_inst, 0, acc_read, is_range),
                                         GetArgDefinition(bc_inst, 1, acc_read, is_range));
    // js call check
    auto js_call_check = IntrinsicCompilerJSCallCheck::Build(this, pc, js_fn);

    // js call
    RuntimeInterface::IntrinsicId call_id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_JS_CALL_FUNCTION;
    if (call_kind == RuntimeInterface::InteropCallKind::NEW_INSTANCE) {
        call_id = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_JS_NEW_INSTANCE;
    }
    auto js_call = GetGraph()->CreateInstIntrinsic(DataType::POINTER, pc, call_id);
    ArenaVector<std::pair<RuntimeInterface::IntrinsicId, DataType::Type>> intrinsics_ids(
        GetGraph()->GetLocalAllocator()->Adapter());
    GetGraph()->GetRuntime()->GetInfoForInteropCallArgsConversion(method, &intrinsics_ids);
    if (call_kind != RuntimeInterface::InteropCallKind::NEW_INSTANCE) {
        js_call->AllocateInputTypes(GetGraph()->GetAllocator(), intrinsics_ids.size() + 4);
        js_call->AppendInput(js_this);
        js_call->AddInputType(DataType::POINTER);
        js_call->AppendInput(js_call_check);
        js_call->AddInputType(DataType::POINTER);
        js_call->AppendInput(GetGraph()->FindOrCreateConstant(intrinsics_ids.size()));
        js_call->AddInputType(DataType::UINT32);
    } else {
        js_call->AllocateInputTypes(GetGraph()->GetAllocator(), intrinsics_ids.size() + 3);
        js_call->AppendInput(js_call_check);
        js_call->AddInputType(DataType::POINTER);
        js_call->AppendInput(GetGraph()->FindOrCreateConstant(intrinsics_ids.size()));
        js_call->AddInputType(DataType::UINT32);
    }

    // Convert args
    size_t arg_idx = 0;
    for (auto [intrinsic_id, type] : intrinsics_ids) {
        Inst *arg = nullptr;
        if (type != DataType::NO_TYPE) {
            arg = BuildInteropIntrinsic<1>(pc, intrinsic_id, DataType::POINTER, {type},
                                           {GetArgDefinition(bc_inst, arg_idx + 2, acc_read, is_range)});
        } else {
            arg = BuildInteropIntrinsic<0>(pc, intrinsic_id, DataType::POINTER, {}, {});
        }
        js_call->AppendInput(arg);
        js_call->AddInputType(DataType::POINTER);
        arg_idx++;
    }

    auto save_state = CreateSaveState(Opcode::SaveState, pc);
    AddInstruction(save_state);
    js_call->AppendInput(save_state);
    js_call->AddInputType(DataType::NO_TYPE);
    AddInstruction(js_call);

    // Convert ret value
    BuildReturnValueConvertInteropIntrinsic(call_kind, pc, method, js_call);
    // Destroy handle scope
    IntrinsicCompilerDestroyLocalScope::Build(this, pc);
}

bool InstBuilder::TryBuildInteropCall(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read)
{
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), bc_inst->GetId(0).AsIndex());
    auto method = GetRuntime()->GetMethodById(GetMethod(), method_id);
    if (OPTIONS.IsCompilerEnableFastInterop()) {
        auto interop_call_kind = GetRuntime()->GetInteropCallKind(method);
        if (interop_call_kind != RuntimeInterface::InteropCallKind::UNKNOWN) {
            BuildInteropCall(bc_inst, interop_call_kind, method, is_range, acc_read);
            return true;
        }
    }
    return false;
}
}  // namespace panda::compiler