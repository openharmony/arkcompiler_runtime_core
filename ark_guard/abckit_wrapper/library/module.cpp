/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "abckit_wrapper/module.h"
#include "object_visitor.h"

void abckit_wrapper::Module::InitForObject(Object *object)
{
    object->owningModule_ = this;
    object->parent_ = this;
}

bool abckit_wrapper::Module::Accept(ModuleVisitor &visitor)
{
    return visitor.Visit(this);
}

template <typename T>
bool abckit_wrapper::Module::TypedObjectsAccept(T &visitor)
{
    for (auto &[_, object] : this->typedObjectTable_) {
        if (!std::visit(ObjectVisitor(&visitor), object)) {
            return false;
        }
    }

    return true;
}

bool abckit_wrapper::Module::NamespacesAccept(NamespaceVisitor &visitor)
{
    return this->TypedObjectsAccept(visitor);
}

bool abckit_wrapper::Module::ClassesAccept(ClassVisitor &visitor)
{
    return this->TypedObjectsAccept(visitor);
}
