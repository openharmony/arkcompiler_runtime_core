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

#include "object_type_propagation.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"

namespace panda::compiler {
bool ObjectTypePropagation::RunImpl()
{
    VisitGraph();
    visited_ = GetGraph()->NewMarker();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto phi : bb->PhiInsts()) {
            auto type_info = GetPhiTypeInfo(phi);
            for (auto visited_phi : visited_phis_) {
                ASSERT(visited_phi->GetObjectTypeInfo() == ObjectTypeInfo::UNKNOWN);
                visited_phi->SetObjectTypeInfo(type_info);
            }
            visited_phis_.clear();
        }
    }
    GetGraph()->EraseMarker(visited_);
    return true;
}

void ObjectTypePropagation::VisitNewObject(GraphVisitor *v, Inst *i)
{
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto inst = i->CastToNewObject();
    auto klass = self->GetGraph()->GetRuntime()->GetClass(inst->GetMethod(), inst->GetTypeId());
    if (klass != nullptr) {
        inst->SetObjectTypeInfo({klass, true});
    }
}

void ObjectTypePropagation::VisitNewArray(GraphVisitor *v, Inst *i)
{
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto inst = i->CastToNewArray();
    auto klass = self->GetGraph()->GetRuntime()->GetClass(inst->GetMethod(), inst->GetTypeId());
    if (klass != nullptr) {
        inst->SetObjectTypeInfo({klass, true});
    }
}

void ObjectTypePropagation::VisitLoadString(GraphVisitor *v, Inst *i)
{
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto inst = i->CastToLoadString();
    auto klass = self->GetGraph()->GetRuntime()->GetStringClass(inst->GetMethod());
    if (klass != nullptr) {
        inst->SetObjectTypeInfo({klass, true});
    }
}

void ObjectTypePropagation::VisitLoadArray([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *i)
{
    // LoadArray should be processed more carefully, because it may contain object of the derived class with own method
    // implementation. We need to check all array stores and method calls between NewArray and LoadArray.
    // NOTE(mshertennikov): Support it.
}

void ObjectTypePropagation::VisitLoadObject(GraphVisitor *v, Inst *i)
{
    if (i->GetType() != DataType::REFERENCE || i->CastToLoadObject()->GetObjectType() != ObjectType::MEM_OBJECT) {
        return;
    }
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto inst = i->CastToLoadObject();
    auto field_id = inst->GetTypeId();
    if (field_id == 0) {
        return;
    }
    auto runtime = self->GetGraph()->GetRuntime();
    auto method = inst->GetMethod();
    auto type_id = runtime->GetFieldValueTypeId(method, field_id);
    auto klass = runtime->GetClass(method, type_id);
    if (klass != nullptr) {
        auto is_exact = runtime->GetClassType(method, type_id) == ClassType::FINAL_CLASS;
        inst->SetObjectTypeInfo({klass, is_exact});
    }
}

void ObjectTypePropagation::VisitCallStatic(GraphVisitor *v, Inst *i)
{
    ProcessManagedCall(v, i->CastToCallStatic());
}

void ObjectTypePropagation::VisitCallVirtual(GraphVisitor *v, Inst *i)
{
    ProcessManagedCall(v, i->CastToCallVirtual());
}

void ObjectTypePropagation::VisitNullCheck([[maybe_unused]] GraphVisitor *v, Inst *i)
{
    auto inst = i->CastToNullCheck();
    inst->SetObjectTypeInfo(inst->GetInput(0).GetInst()->GetObjectTypeInfo());
}

void ObjectTypePropagation::VisitRefTypeCheck([[maybe_unused]] GraphVisitor *v, Inst *i)
{
    auto inst = i->CastToRefTypeCheck();
    inst->SetObjectTypeInfo(inst->GetInput(0).GetInst()->GetObjectTypeInfo());
}

void ObjectTypePropagation::ProcessManagedCall(GraphVisitor *v, CallInst *inst)
{
    if (inst->GetType() != DataType::REFERENCE) {
        return;
    }
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto runtime = self->GetGraph()->GetRuntime();
    auto method = inst->GetCallMethod();
    auto type_id = runtime->GetMethodReturnTypeId(method);
    auto klass = runtime->GetClass(method, type_id);
    if (klass != nullptr) {
        auto is_exact = runtime->GetClassType(method, type_id) == ClassType::FINAL_CLASS;
        inst->SetObjectTypeInfo({klass, is_exact});
    }
}

ObjectTypeInfo ObjectTypePropagation::GetPhiTypeInfo(Inst *inst)
{
    if (!inst->IsPhi() || inst->SetMarker(visited_)) {
        return inst->GetObjectTypeInfo();
    }
    auto type_info = ObjectTypeInfo::UNKNOWN;
    inst->SetObjectTypeInfo(type_info);
    bool need_update = false;
    for (auto input : inst->GetInputs()) {
        auto input_info = GetPhiTypeInfo(input.GetInst());
        if (input_info == ObjectTypeInfo::UNKNOWN) {
            ASSERT(input.GetInst()->IsPhi());
            need_update = true;
            continue;
        }
        if (input_info == ObjectTypeInfo::INVALID ||
            (type_info.IsValid() && type_info.GetClass() != input_info.GetClass())) {
            inst->SetObjectTypeInfo(ObjectTypeInfo::INVALID);
            return ObjectTypeInfo::INVALID;
        }
        if (type_info == ObjectTypeInfo::UNKNOWN) {
            type_info = input_info;
            continue;
        }
        type_info = {type_info.GetClass(), type_info.IsExact() && input_info.IsExact()};
    }
    if (need_update) {
        visited_phis_.push_back(inst);
    } else {
        inst->SetObjectTypeInfo(type_info);
    }
    return type_info;
}

}  // namespace panda::compiler
