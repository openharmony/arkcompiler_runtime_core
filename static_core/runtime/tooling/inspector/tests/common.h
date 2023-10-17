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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_COMMON_H
#define PANDA_TOOLING_INSPECTOR_TEST_COMMON_H

#include "utils/json_builder.h"
#include "utils/json_parser.h"

namespace panda::tooling::inspector::test {
template <typename T>
static JsonObject ToJson(const T &object)
{
    JsonObjectBuilder json_builder;
    object.ToJson()(json_builder);
    return JsonObject(std::move(json_builder).Build());
}

static JsonObject ToObject(std::function<void(JsonObjectBuilder &)> &&params)
{
    JsonObjectBuilder json_builder;
    params(json_builder);
    return JsonObject(std::move(json_builder).Build());
}

}  // namespace panda::tooling::inspector::test

#endif  // PANDA_TOOLING_INSPECTOR_TEST_JSON_OBJECT_MATCHER_H