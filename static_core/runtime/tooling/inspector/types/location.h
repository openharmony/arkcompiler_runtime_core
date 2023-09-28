/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "numeric_id.h"

#include "utils/expected.h"

#include <cstddef>
#include <functional>
#include <string>

namespace panda {
class JsonObject;
class JsonObjectBuilder;
}  // namespace panda

namespace panda::tooling::inspector {
class Location {
public:
    Location(ScriptId script_id, size_t line_number) : script_id_(script_id), line_number_(line_number) {}

    static Expected<Location, std::string> FromJsonProperty(const JsonObject &object, const char *property_name);

    ScriptId GetScriptId() const
    {
        return script_id_;
    }

    size_t GetLineNumber() const
    {
        return line_number_;
    }

    std::function<void(JsonObjectBuilder &)> ToJson() const;

private:
    ScriptId script_id_;
    size_t line_number_;
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_LOCATION_H
