/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_LOCATION_H
#define PANDA_TOOLING_INSPECTOR_TYPES_LOCATION_H

#include "tooling/inspector/json_serialization/serializable.h"

#include <cstddef>
#include <functional>
#include <string>

#include "utils/expected.h"

#include "types/numeric_id.h"

namespace ark {
class JsonObject;
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
class Location final : public JsonSerializable {
public:
    Location(ScriptId scriptId, size_t lineNumber) : scriptId_(scriptId), lineNumber_(lineNumber) {}

    static Expected<Location, std::string> FromJsonProperty(const JsonObject &object, const char *propertyName);

    ScriptId GetScriptId() const
    {
        return scriptId_;
    }

    size_t GetLineNumber() const
    {
        return lineNumber_;
    }

    void SetLineNumber(size_t lineNumber)
    {
        lineNumber_ = lineNumber;
    }

    void Serialize(JsonObjectBuilder &builder) const override;

private:
    ScriptId scriptId_;
    size_t lineNumber_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_LOCATION_H
