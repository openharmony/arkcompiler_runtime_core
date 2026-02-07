/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "abckit_wrapper/object.h"
#include "object_visitor.h"
#include "logger.h"

namespace {
constexpr std::string_view OBJECT_FULL_NAME_SEP = ".";
constexpr size_t TYPICAL_FQN_DEPTH = 8;
}  // namespace

AbckitWrapperErrorCode abckit_wrapper::Object::Init()
{
    return AbckitWrapperErrorCode::ERR_SUCCESS;
}

void abckit_wrapper::Object::InvalidateFullyQualifiedNameCache()
{
    cachedFullyQualifiedName_.reset();
    for (auto &[_, child] : children_) {
        std::visit(
            [](auto *obj) {
                if (obj != nullptr) {
                    obj->InvalidateFullyQualifiedNameCache();
                }
            },
            child);
    }
}

std::string abckit_wrapper::Object::GetFullyQualifiedName() const
{
    if (cachedFullyQualifiedName_.has_value()) {
        return *cachedFullyQualifiedName_;
    }

    thread_local std::vector<std::string> parts;
    parts.clear();
    parts.reserve(TYPICAL_FQN_DEPTH);
    const Object *obj = this;
    while (obj != nullptr) {
        parts.push_back(obj->GetName());
        obj = obj->parent_.has_value() ? obj->parent_.value() : nullptr;
    }
    if (parts.empty()) {
        return "";
    }
    if (parts.size() == 1) {
        cachedFullyQualifiedName_ = std::move(parts[0]);
        return *cachedFullyQualifiedName_;
    }
    size_t total = 0;
    for (const auto &p : parts) {
        total += p.size() + 1;
    }
    --total;

    std::string result;
    result.reserve(total);
    for (size_t i = parts.size(); i-- > 0;) {
        if (i != parts.size() - 1) {
            result += OBJECT_FULL_NAME_SEP;
        }
        result += parts[i];
    }
    cachedFullyQualifiedName_ = std::move(result);
    return *cachedFullyQualifiedName_;
}

std::string abckit_wrapper::Object::GetPackageName() const
{
    if (this->parent_.has_value()) {
        return this->parent_.value()->GetFullyQualifiedName();
    }

    return "";
}

bool abckit_wrapper::Object::ChildrenAccept(ChildVisitor &visitor)
{
    for (auto &[_, object] : this->children_) {
        if (!std::visit(ObjectVisitor(&visitor), object)) {
            return false;
        }
    }

    return true;
}

void abckit_wrapper::ObjectPool::Add(std::unique_ptr<Object> &&object)
{
    std::string objectName = object->GetFullyQualifiedName();
    if (this->objects_.find(objectName) != this->objects_.end()) {
        LOG_F << "Duplicate object:" << object->GetFullyQualifiedName();
        abort();
    }

    this->objects_.emplace(objectName, std::move(object));
}

std::optional<abckit_wrapper::Object *> abckit_wrapper::ObjectPool::Get(const std::string &objectName) const
{
    const auto it = this->objects_.find(objectName);
    if (it == this->objects_.end()) {
        return std::nullopt;
    }

    return it->second.get();
}

void abckit_wrapper::ObjectPool::ClearObjectsData()
{
    for (auto &[_, object] : this->objects_) {
        object->data_.reset();
    }
}
