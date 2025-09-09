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
#include "abckit_wrapper/namespace.h"

void abckit_wrapper::Namespace::InitForObject(Object *object)
{
    object->owningModule_ = this->owningModule_;
    object->owningNamespace_ = this;
    object->parent_ = this;
}

bool abckit_wrapper::Namespace::Accept(NamespaceVisitor &visitor)
{
    return visitor.Visit(this);
}

bool abckit_wrapper::Namespace::Accept(ChildVisitor &visitor)
{
    return visitor.VisitNamespace(this);
}
