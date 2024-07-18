/**
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

#include "runtime/tooling/inspector/types/scope.h"

#include "libpandabase/utils/json_builder.h"

namespace ark::tooling::inspector {
static const char *GetTypeString(Scope::Type type)
{
    switch (type) {
        case Scope::Type::GLOBAL:
            return "global";
        case Scope::Type::LOCAL:
            return "local";
        default:
            UNREACHABLE();
            return nullptr;
    }
}

Scope::Scope(Scope::Type type, RemoteObject object, std::optional<std::string> name)
    : type_(type), object_(std::move(object)), name_(std::move(name))
{
}

std::function<void(JsonObjectBuilder &)> Scope::ToJson() const
{
    return [this](JsonObjectBuilder &jsonBuilder) {
        jsonBuilder.AddProperty("type", GetTypeString(type_));
        jsonBuilder.AddProperty("object", object_.ToJson());
        if (name_) {
            jsonBuilder.AddProperty("name", *name_);
        }
    };
}
}  // namespace ark::tooling::inspector
