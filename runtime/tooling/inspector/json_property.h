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

#ifndef PANDA_TOOLING_INSPECTOR_JSON_PROPERTY_H
#define PANDA_TOOLING_INSPECTOR_JSON_PROPERTY_H

#include "utils/json_parser.h"
#include "utils/logger.h"

#include <string>
#include <type_traits>

namespace panda::tooling::inspector {
template <typename PropertyType, typename Result, typename... Key>
bool GetProperty(Result &result, const JsonObject &object, const JsonObject::Key &key, const Key &... keys)
{
    if constexpr (sizeof...(keys) > 0) {  // NOLINT(bugprone-suspicious-semicolon,readability-braces-around-statements)
        auto *ptr = object.GetValue<JsonObject::JsonObjPointer>(key);
        return ptr && GetProperty<PropertyType>(result, **ptr, keys...);
    }

    auto *ptr = object.GetValue<PropertyType>(key);
    if (!ptr) {
        return false;
    }

    if constexpr (std::is_scalar_v<PropertyType>) {  // NOLINT(readability-braces-around-statements)
        result = *ptr;
        // NOLINTNEXTLINE(readability-braces-around-statements,readability-misleading-indentation)
    } else if constexpr (std::is_same_v<PropertyType, JsonObject::JsonObjPointer>) {
        result = ptr->get();
    } else {  // NOLINT(readability-misleading-indentation)
        result = ptr;
    }
    return true;
}

template <typename PropertyType, typename Result, typename... Key>
bool GetPropertyOrLog(Result &result, const JsonObject &object, const JsonObject::Key &key, const Key &... keys)
{
    using namespace std::literals::string_literals;

    if (GetProperty<PropertyType>(result, object, key, keys...)) {
        return true;
    }

    LOG(INFO, DEBUGGER) << "No '" << (key + ... + ("."s + keys)) << "' property";
    return false;
}
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_JSON_PROPERTY_H
