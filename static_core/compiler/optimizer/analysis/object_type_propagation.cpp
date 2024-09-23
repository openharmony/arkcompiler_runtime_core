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

#include "object_type_propagation.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {
bool ObjectTypePropagation::RunImpl()
{
    VisitGraph();
    InstVector visitedPhis(GetGraph()->GetLocalAllocator()->Adapter());
    visitedPhis_ = &visitedPhis;
    visited_ = GetGraph()->NewMarker();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto phi : bb->PhiInsts()) {
            auto typeInfo = GetPhiTypeInfo(phi);
            for (auto visitedPhi : visitedPhis) {
                ASSERT(visitedPhi->GetObjectTypeInfo() == ObjectTypeInfo::UNKNOWN);
                visitedPhi->SetObjectTypeInfo(typeInfo);
            }
            visitedPhis.clear();
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
    auto klass = self->GetGraph()->GetRuntime()->GetStringClass(inst->GetMethod(), nullptr);
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
    auto fieldId = inst->GetTypeId();
    if (fieldId == 0) {
        return;
    }
    auto runtime = self->GetGraph()->GetRuntime();
    auto method = inst->GetMethod();
    auto typeId = runtime->GetFieldValueTypeId(method, fieldId);
    auto klass = runtime->GetClass(method, typeId);
    if (klass != nullptr) {
        auto isExact = runtime->GetClassType(method, typeId) == ClassType::FINAL_CLASS;
        inst->SetObjectTypeInfo({klass, isExact});
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

void ObjectTypePropagation::VisitParameter([[maybe_unused]] GraphVisitor *v, Inst *i)
{
    auto inst = i->CastToParameter();
    auto graph = i->GetBasicBlock()->GetGraph();
    if (inst->GetType() != DataType::REFERENCE || graph->IsBytecodeOptimizer() || inst->HasObjectTypeInfo()) {
        return;
    }
    auto refNum = inst->GetArgRefNumber();
    auto runtime = graph->GetRuntime();
    auto method = graph->GetMethod();
    RuntimeInterface::ClassPtr klass;
    if (refNum == ParameterInst::INVALID_ARG_REF_NUM) {
        // This parametr doesn't have ArgRefNumber
        if (inst->GetArgNumber() != 0 || runtime->IsMethodStatic(method)) {
            return;
        }
        klass = runtime->GetClass(method);
    } else {
        auto typeId = runtime->GetMethodArgReferenceTypeId(method, refNum);
        klass = runtime->GetClass(method, typeId);
    }
    if (klass != nullptr) {
        auto isExact = runtime->GetClassType(klass) == ClassType::FINAL_CLASS;
        inst->SetObjectTypeInfo({klass, isExact});
    }
}

void ObjectTypePropagation::ProcessManagedCall(GraphVisitor *v, CallInst *inst)
{
    if (inst->GetType() != DataType::REFERENCE) {
        return;
    }
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto runtime = self->GetGraph()->GetRuntime();
    auto method = inst->GetCallMethod();
    auto typeId = runtime->GetMethodReturnTypeId(method);
    auto klass = runtime->GetClass(method, typeId);
    if (klass != nullptr) {
        auto isExact = runtime->GetClassType(method, typeId) == ClassType::FINAL_CLASS;
        inst->SetObjectTypeInfo({klass, isExact});
    }
}

ObjectTypeInfo ObjectTypePropagation::GetPhiTypeInfo(Inst *inst)
{
    if (!inst->IsPhi() || inst->SetMarker(visited_)) {
        return inst->GetObjectTypeInfo();
    }
    auto typeInfo = ObjectTypeInfo::UNKNOWN;
    inst->SetObjectTypeInfo(typeInfo);
    bool needUpdate = false;
    for (auto input : inst->GetInputs()) {
        auto inputInfo = GetPhiTypeInfo(input.GetInst());
        if (inputInfo == ObjectTypeInfo::UNKNOWN) {
            ASSERT(input.GetInst()->IsPhi());
            needUpdate = true;
            continue;
        }
        if (inputInfo == ObjectTypeInfo::INVALID ||
            (typeInfo.IsValid() && typeInfo.GetClass() != inputInfo.GetClass())) {
            inst->SetObjectTypeInfo(ObjectTypeInfo::INVALID);
            return ObjectTypeInfo::INVALID;
        }
        if (typeInfo == ObjectTypeInfo::UNKNOWN) {
            typeInfo = inputInfo;
            continue;
        }
        typeInfo = {typeInfo.GetClass(), typeInfo.IsExact() && inputInfo.IsExact()};
    }
    if (needUpdate) {
        visitedPhis_->push_back(inst);
    } else {
        inst->SetObjectTypeInfo(typeInfo);
    }
    return typeInfo;
}

}  // namespace ark::compiler
