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

#include "location.h"
#include "numeric_id.h"

#include "utils/expected.h"
#include "utils/json_builder.h"
#include "utils/json_parser.h"

#include <cfloat>
#include <cmath>
#include <string>

using namespace std::literals::string_literals;  // NOLINT(google-build-using-namespace)

namespace panda::tooling::inspector {
Expected<Location, std::string> Location::FromJsonProperty(const JsonObject &object, const char *property_name)
{
    auto property = object.GetValue<JsonObject::JsonObjPointer>(property_name);
    if (property == nullptr) {
        return Unexpected("No such property: "s + property_name);
    }

    auto script_id = ParseNumericId<ScriptId>(**property, "scriptId");
    if (!script_id) {
        return Unexpected(script_id.Error());
    }

    auto line_number = property->get()->GetValue<JsonObject::NumT>("lineNumber");
    if (line_number == nullptr) {
        return Unexpected("Invalid Location: No 'lineNumber' property"s);
    }

    auto line_number_trunc = std::trunc(*line_number);
    if (*line_number < 0 || *line_number - line_number_trunc > line_number_trunc * DBL_EPSILON) {
        return Unexpected("Invalid line number: " + std::to_string(*line_number));
    }

    return Location(*script_id, line_number_trunc + 1);
}

std::function<void(JsonObjectBuilder &)> Location::ToJson() const
{
    return [this](JsonObjectBuilder &json_builder) {
        json_builder.AddProperty("scriptId", std::to_string(script_id_));
        json_builder.AddProperty("lineNumber", line_number_ - 1);
    };
}
}  // namespace panda::tooling::inspector
