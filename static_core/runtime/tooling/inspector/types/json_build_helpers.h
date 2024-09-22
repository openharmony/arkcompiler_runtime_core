/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_HELPERS_H
#define PANDA_TOOLING_INSPECTOR_TYPES_HELPERS_H

#include <functional>

#include "utils/json_builder.h"

namespace ark::tooling::inspector {

template <typename T>
void AddElement(std::function<void(JsonArrayBuilder &)> &func, T value)
{
    func = [func = std::move(func), value = std::move(value)](JsonArrayBuilder &builder) {
        func(builder);
        builder.Add(value);
    };
}

template <typename T>
void AddProperty(std::function<void(JsonObjectBuilder &)> &func, const char *key, T value)
{
    func = [func = std::move(func), key, value = std::move(value)](JsonObjectBuilder &builder) {
        func(builder);
        builder.AddProperty(key, value);
    };
}

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_HELPERS_H
