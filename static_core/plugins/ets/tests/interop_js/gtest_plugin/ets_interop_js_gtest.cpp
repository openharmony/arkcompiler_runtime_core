/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ets_interop_js_gtest.h"
#include <iostream>
#include <memory>

#ifdef PANDA_TARGET_OHOS
// NOTE:
//  napi_fatal_exception() is not implemented it libace_napi.z.so,
//  so let's implement it ourselves
extern "C" napi_status napi_fatal_exception([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value err)
{
    std::cerr << "ETS_INTEROP_GTEST_PLUGIN: " << __func__ << " is not implemented" << std::endl;
    std::abort();
    return napi_ok;
}
#endif  // PANDA_TARGET_OHOS

namespace panda::ets::interop::js::testing {

napi_env EtsInteropTest::js_env_ = {};

class JsEnvAccessor {
public:
    static void SetJsEnv(napi_env env)
    {
        EtsInteropTest::js_env_ = env;
    }

    static void ResetJsEnv()
    {
        EtsInteropTest::js_env_ = {};
    }
};

static napi_value Main(napi_env env, napi_callback_info info)
{
    size_t js_argc = 0;
    [[maybe_unused]] napi_status status = napi_get_cb_info(env, info, &js_argc, nullptr, nullptr, nullptr);
    assert(status == napi_ok);

    if (js_argc != 1) {
        std::cerr << "ETS_INTEROP_GTEST_PLUGIN: Incorrect argc, argc=" << js_argc << std::endl;
        std::abort();
    }

    std::vector<napi_value> js_argv(js_argc);

    napi_value this_arg = nullptr;
    void *data = nullptr;
    status = napi_get_cb_info(env, info, &js_argc, js_argv.data(), &this_arg, &data);
    assert(status == napi_ok);

    napi_value js_array = js_argv.front();
    uint32_t js_array_length = 0;
    status = napi_get_array_length(env, js_array, &js_array_length);
    assert(status == napi_ok);

    std::vector<std::string> argv;
    for (uint32_t i = 0; i < js_array_length; ++i) {
        napi_value js_array_element;
        status = napi_get_element(env, js_array, i, &js_array_element);
        assert(status == napi_ok);

        size_t str_length = 0;
        status = napi_get_value_string_utf8(env, js_array_element, nullptr, 0, &str_length);
        assert(status == napi_ok);

        std::string utf8_str(++str_length, '\0');
        size_t copied = 0;
        status = napi_get_value_string_utf8(env, js_array_element, utf8_str.data(), utf8_str.size(), &copied);
        assert(status == napi_ok);

        argv.emplace_back(std::move(utf8_str));
    }

    int argc = js_array_length;
    std::vector<char *> argv_char;
    argv_char.reserve(argv.size());

    for (auto &arg : argv) {
        argv_char.emplace_back(arg.data());
    }

    assert(size_t(argc) == argv_char.size());
    ::testing::InitGoogleTest(&argc, argv_char.data());

    // Run tests
    JsEnvAccessor::SetJsEnv(env);
    int ret_value = RUN_ALL_TESTS();
    JsEnvAccessor::ResetJsEnv();

    napi_value js_ret_value;
    status = napi_create_int32(env, ret_value, &js_ret_value);
    assert(status == napi_ok);
    return js_ret_value;
}

static napi_value Init(napi_env env, napi_value exports)
{
    const std::array desc = {
        napi_property_descriptor {"main", 0, Main, 0, 0, 0, napi_enumerable, 0},
    };

    [[maybe_unused]] napi_status status;
    status = napi_define_properties(env, exports, desc.size(), desc.data());
    assert(status == napi_ok);

    return exports;
}

}  // namespace panda::ets::interop::js::testing

NAPI_MODULE(GTEST_ADDON, panda::ets::interop::js::testing::Init)
