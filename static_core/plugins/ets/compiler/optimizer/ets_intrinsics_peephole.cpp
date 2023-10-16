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

#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/runtime_interface.h"

namespace panda::compiler {
bool Peepholes::PeepholeStringEquals([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] IntrinsicInst *intrinsic)
{
    return false;
}

template <bool IS_STORE>
bool TryInsertFieldInst(IntrinsicInst *intrinsic, RuntimeInterface::ClassPtr klass_ptr,
                        RuntimeInterface::FieldPtr raw_field, size_t field_id)
{
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto field = runtime->ResolveLookUpField(raw_field, klass_ptr);
    if (field == nullptr) {
        return false;
    }
    Inst *mem_obj;
    auto type = intrinsic->GetType();
    auto pc = intrinsic->GetPc();
    if constexpr (IS_STORE) {
        auto store_field = graph->CreateInstStoreObject(type, pc);
        store_field->SetTypeId(field_id);
        store_field->SetMethod(intrinsic->GetMethod());
        store_field->SetObjField(field);
        if (runtime->IsFieldVolatile(field)) {
            store_field->SetVolatile(true);
        }
        if (type == DataType::REFERENCE) {
            store_field->SetNeedBarrier(true);
        }
        store_field->SetInput(1, intrinsic->GetInput(1).GetInst());
        mem_obj = store_field;
    } else {
        auto load_field = graph->CreateInstLoadObject(type, pc);
        load_field->SetTypeId(field_id);
        load_field->SetMethod(intrinsic->GetMethod());
        load_field->SetObjField(field);
        if (runtime->IsFieldVolatile(field)) {
            load_field->SetVolatile(true);
        }
        mem_obj = load_field;
        intrinsic->ReplaceUsers(load_field);
    }
    mem_obj->SetInput(0, intrinsic->GetInput(0).GetInst());
    intrinsic->InsertAfter(mem_obj);

    intrinsic->ClearFlag(inst_flags::NO_DCE);
    return true;
}

template <bool IS_STORE>
bool TryInsertCallInst(IntrinsicInst *intrinsic, RuntimeInterface::ClassPtr klass_ptr,
                       RuntimeInterface::FieldPtr raw_field)
{
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto method = runtime->ResolveLookUpCall(raw_field, klass_ptr, IS_STORE);
    if (method == nullptr) {
        return false;
    }
    auto type = IS_STORE ? DataType::VOID : intrinsic->GetType();
    auto pc = intrinsic->GetPc();

    auto call = graph->CreateInstCallVirtual(type, pc, runtime->GetMethodId(method));
    call->SetCallMethod(method);
    size_t num_inputs = IS_STORE ? 3 : 2;
    call->ReserveInputs(num_inputs);
    call->AllocateInputTypes(graph->GetAllocator(), num_inputs);
    for (size_t i = 0; i < num_inputs; ++i) {
        call->AppendInput(intrinsic->GetInput(i));
        call->AddInputType(intrinsic->GetInputType(i));
    }
    intrinsic->InsertAfter(call);
    intrinsic->ReplaceUsers(call);
    intrinsic->ClearFlag(inst_flags::NO_DCE);
    return true;
}

bool Peepholes::PeepholeLdObjByName(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto klass_ptr = GetClassPtrForObject(intrinsic);
    if (klass_ptr == nullptr) {
        return false;
    }
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto method = intrinsic->GetMethod();
    auto runtime = graph->GetRuntime();
    auto field_id = intrinsic->GetImm(0);
    auto raw_field = runtime->ResolveField(method, field_id, !graph->IsAotMode(), nullptr);
    ASSERT(raw_field != nullptr);

    if (TryInsertFieldInst<false>(intrinsic, klass_ptr, raw_field, field_id)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    if (TryInsertCallInst<false>(intrinsic, klass_ptr, raw_field)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    return false;
}

bool Peepholes::PeepholeStObjByName(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto klass_ptr = GetClassPtrForObject(intrinsic);
    if (klass_ptr == nullptr) {
        return false;
    }
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto method = intrinsic->GetMethod();
    auto runtime = graph->GetRuntime();
    auto field_id = intrinsic->GetImm(0);
    auto raw_field = runtime->ResolveField(method, field_id, !graph->IsAotMode(), nullptr);
    ASSERT(raw_field != nullptr);

    if (TryInsertFieldInst<true>(intrinsic, klass_ptr, raw_field, field_id)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    if (TryInsertCallInst<true>(intrinsic, klass_ptr, raw_field)) {
        PEEPHOLE_IS_APPLIED(static_cast<Peepholes *>(v), intrinsic);
        return true;
    }
    return false;
}

}  // namespace panda::compiler