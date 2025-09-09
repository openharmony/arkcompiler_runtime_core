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
#include "abckit_wrapper/class.h"
#include "abckit_wrapper/module.h"
#include "logger.h"

AbckitWrapperErrorCode abckit_wrapper::Class::Init()
{
    this->InitAccessFlags();

    return ERR_SUCCESS;
}

std::string abckit_wrapper::Class::GetName() const
{
    return this->GetObjectName();
}

void abckit_wrapper::Class::InitAccessFlags()
{
    if (const auto clazz = std::get_if<abckit::core::Class>(&this->impl_)) {
        this->accessFlags_ |= clazz->IsFinal() ? AccessFlags::FINAL : AccessFlags::NONE;
        this->accessFlags_ |= clazz->IsAbstract() ? AccessFlags::ABSTRACT : AccessFlags::NONE;
    }
}

bool abckit_wrapper::Class::IsExternal() const
{
    return std::visit([](const auto &object) { return object.IsExternal(); }, this->impl_);
}

bool abckit_wrapper::Class::Accept(ClassVisitor &visitor)
{
    return visitor.Visit(this);
}

bool abckit_wrapper::Class::Accept(ChildVisitor &visitor)
{
    return visitor.VisitClass(this);
}

bool abckit_wrapper::Class::HierarchyAccept(ClassVisitor &visitor, const bool visitThis, const bool visitSuper,
                                            const bool visitInterfaces, const bool visitSubclasses)
{
    // 1. visit this
    if (visitThis) {
        if (!this->Accept(visitor)) {
            return false;
        }
    }

    // ignore external class hierarchy visit
    if (this->IsExternal()) {
        return true;
    }

    // 2. visit super class recursively
    if (visitSuper && this->superClass_.has_value()) {
        if (!this->superClass_.value()->HierarchyAccept(visitor, true, true, visitInterfaces, false)) {
            return false;
        }
    }

    // 3. visit interfaces recursively
    if (visitInterfaces) {
        // if visit super class is false, add visit super class interfaces
        if (!visitSuper && this->superClass_.has_value()) {
            if (!this->superClass_.value()->HierarchyAccept(visitor, false, false, true, false)) {
                return false;
            }
        }

        // visit interfaces
        for (const auto &[_, interface] : this->interfaces_) {
            if (!interface->HierarchyAccept(visitor, true, false, true, false)) {
                return false;
            }
        }
    }

    // 4. visit subclasses recursively
    if (visitSubclasses) {
        for (const auto &[_, clazz] : this->subClasses_) {
            if (!clazz->HierarchyAccept(visitor, true, false, false, true)) {
                return false;
            }
        }
    }

    return true;
}
