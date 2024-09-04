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

#include <node_api.h>
#include <array>
#include <iostream>
#include <string>

#include "libpandabase/macros.h"

static napi_value GetSampleString(napi_env env, napi_callback_info info)
{
    size_t jsArgc = 0;
    [[maybe_unused]] napi_status status = napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr);
    ASSERT(status == napi_ok);

    std::string retString = "Test";
    napi_value jsString;
    status = napi_create_string_utf8(env, retString.data(), retString.length(), &jsString);
    ASSERT(status == napi_ok);

    return jsString;
}
static napi_value Init(napi_env env, napi_value exports)
{
    const std::array desc = {
        napi_property_descriptor {"getSampleString", 0, GetSampleString, 0, 0, 0, napi_enumerable, 0},
    };

    [[maybe_unused]] napi_status status = napi_define_properties(env, exports, desc.size(), desc.data());
    ASSERT(status == napi_ok);

    return exports;
}

NAPI_MODULE(ARK_JS_NAPI_TEST_ADDON, Init)
