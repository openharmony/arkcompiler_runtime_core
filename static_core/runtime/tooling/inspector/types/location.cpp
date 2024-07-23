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

#include "location.h"
#include "numeric_id.h"

#include "utils/expected.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"

#include <cfloat>
#include <cmath>
#include <string>

using namespace std::literals::string_literals;  // NOLINT(google-build-using-namespace)

namespace ark::tooling::inspector {
Expected<Location, std::string> Location::FromJsonProperty(const JsonObject &object, const char *propertyName)
{
    auto property = object.GetValue<JsonObject::JsonObjPointer>(propertyName);
    if (property == nullptr) {
        return Unexpected("No such property: "s + propertyName);
    }

    auto scriptId = ParseNumericId<ScriptId>(**property, "scriptId");
    if (!scriptId) {
        return Unexpected(scriptId.Error());
    }

    auto lineNumber = property->get()->GetValue<JsonObject::NumT>("lineNumber");
    if (lineNumber == nullptr) {
        return Unexpected("Invalid Location: No 'lineNumber' property"s);
    }

    auto lineNumberTrunc = std::trunc(*lineNumber);
    if (*lineNumber < 0 || *lineNumber - lineNumberTrunc > lineNumberTrunc * DBL_EPSILON) {
        return Unexpected("Invalid line number: " + std::to_string(*lineNumber));
    }

    return Location(*scriptId, lineNumberTrunc + 1);
}

std::function<void(JsonObjectBuilder &)> Location::ToJson() const
{
    return [this](JsonObjectBuilder &jsonBuilder) {
        jsonBuilder.AddProperty("scriptId", std::to_string(scriptId_));
        jsonBuilder.AddProperty("lineNumber", lineNumber_ - 1);
    };
}
}  // namespace ark::tooling::inspector
