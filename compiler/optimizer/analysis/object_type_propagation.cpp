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

#include "object_type_propagation.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"

namespace panda::compiler {
bool ObjectTypePropagation::RunImpl()
{
    VisitGraph();
    return true;
}

void ObjectTypePropagation::VisitNewObject(GraphVisitor *v, Inst *i)
{
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto inst = i->CastToNewObject();
    auto klass = self->GetGraph()->GetRuntime()->GetClass(inst->GetMethod(), inst->GetTypeId());
    if (klass != nullptr) {
        inst->SetObjectTypeInfo(ObjectTypeInfo(static_cast<ObjectTypeInfo::ClassType>(klass)));
    }
}

void ObjectTypePropagation::VisitNewArray(GraphVisitor *v, Inst *i)
{
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto inst = i->CastToNewArray();
    auto klass = self->GetGraph()->GetRuntime()->GetClass(inst->GetMethod(), inst->GetTypeId());
    if (klass != nullptr) {
        inst->SetObjectTypeInfo(ObjectTypeInfo(static_cast<ObjectTypeInfo::ClassType>(klass)));
    }
}

void ObjectTypePropagation::VisitLoadString(GraphVisitor *v, Inst *i)
{
    auto self = static_cast<ObjectTypePropagation *>(v);
    auto inst = i->CastToLoadString();
    auto klass = self->GetGraph()->GetRuntime()->GetStringClass(inst->GetMethod());
    if (klass != nullptr) {
        inst->SetObjectTypeInfo(ObjectTypeInfo(static_cast<ObjectTypeInfo::ClassType>(klass)));
    }
}

void ObjectTypePropagation::VisitLoadArray([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *i)
{
    // LoadArray should be processed more carefully, because it may contain object of the derived class with own method
    // implementation. We need to check all array stores and method calls between NewArray and LoadArray.
    // TODO(mshertennikov): Support it.
}
}  // namespace panda::compiler
