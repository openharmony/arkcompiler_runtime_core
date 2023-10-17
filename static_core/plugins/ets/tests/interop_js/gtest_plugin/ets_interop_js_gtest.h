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

#ifndef PANDA_PLUGINS_ETS_INTEROP_JS_GTEST_H_
#define PANDA_PLUGINS_ETS_INTEROP_JS_GTEST_H_

#include <fstream>
#include <sstream>
#include <gtest/gtest.h>
#include <node_api.h>

namespace panda::ets::interop::js::testing {

class EtsInteropTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        if (std::getenv("ARK_ETS_INTEROP_JS_GTEST_SOURCES") == nullptr) {
            std::cerr << "ARK_ETS_INTEROP_JS_GTEST_SOURCES not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        bool has_pending_exc;
        [[maybe_unused]] napi_status status = napi_is_exception_pending(GetJsEnv(), &has_pending_exc);
        assert(status == napi_ok && !has_pending_exc);

        interop_js_test_path_ = std::getenv("ARK_ETS_INTEROP_JS_GTEST_SOURCES");
        // This object is used to save global js names
        if (!RunJsScript("var gtest_env = {};\n")) {
            std::abort();
        }
    }

    [[nodiscard]] static bool RunJsScript(const std::string &script)
    {
        return DoRunJsScript(js_env_, script);
    }

    template <typename R>
    [[nodiscard]] std::optional<R> RunJsScriptByPath(const std::string &path)
    {
        return DoRunJsScriptByPath<R>(js_env_, path);
    }

    [[nodiscard]] bool RunJsTestSute(const std::string &path)
    {
        return DoRunJsTestSute(js_env_, path);
    }

    template <typename R, typename... Args>
    [[nodiscard]] std::optional<R> CallEtsMethod(std::string_view fn_name, Args &&...args)
    {
        return DoCallEtsMethod<R>(fn_name, js_env_, std::forward<Args>(args)...);
    }

    static napi_env GetJsEnv()
    {
        return js_env_;
    }

    void LoadModuleAs(const std::string &module_name, const std::string &module_path)
    {
        auto res = RunJsScript("gtest_env." + module_name + " = require(\"" + interop_js_test_path_ + "/" +
                               module_path + "\");\n");
        if (!res) {
            std::cerr << "Failed to load module: " << module_path << std::endl;
            std::abort();
        }
    }

private:
    [[nodiscard]] bool DoRunJsTestSute(napi_env env, const std::string &path)
    {
        std::string full_path = interop_js_test_path_ + "/" + path;
        std::string test_souce_code = ReadFile(full_path);

        // NOTE:
        //  We wrap the test source code to anonymous lambda function to avoid pollution the global namespace.
        //  We also set the 'use strict' mode, because right now we are checking interop only in the strict mode.
        return DoRunJsScript(env, "(() => { \n'use strict'\n" + test_souce_code + "\n})()");
    }

    [[nodiscard]] static bool DoRunJsScript(napi_env env, const std::string &script)
    {
        [[maybe_unused]] napi_status status;
        napi_value js_script;
        status = napi_create_string_utf8(env, script.c_str(), script.length(), &js_script);
        assert(status == napi_ok);

        napi_value js_result;
        status = napi_run_script(env, js_script, &js_result);
        if (status == napi_generic_failure) {
            std::cerr << "EtsInteropTest fired an exception" << std::endl;
            napi_value exc;
            status = napi_get_and_clear_last_exception(env, &exc);
            assert(status == napi_ok);
            napi_value js_str;
            status = napi_coerce_to_string(env, exc, &js_str);
            assert(status == napi_ok);
            std::cerr << "Exception string: " << GetString(env, js_str) << std::endl;
            return false;
        }
        return status == napi_ok;
    }

    static std::string ReadFile(const std::string &full_path)
    {
        std::ifstream js_source_file(full_path);
        if (!js_source_file.is_open()) {
            std::cerr << __func__ << ": Cannot open file: " << full_path << std::endl;
            std::abort();
        }
        std::ostringstream string_stream;
        string_stream << js_source_file.rdbuf();
        return string_stream.str();
    }

    template <typename T>
    [[nodiscard]] std::optional<T> DoRunJsScriptByPath(napi_env env, const std::string &path)
    {
        std::string full_path = interop_js_test_path_ + "/" + path;

        if (!DoRunJsScript(env, ReadFile(full_path))) {
            return std::nullopt;
        }

        [[maybe_unused]] napi_status status;
        napi_value js_ret_value {};
        status = napi_get_named_property(env, GetJsGtestObject(env), "ret", &js_ret_value);
        assert(status == napi_ok);

        // Get globalThis.gtest.ret
        return GetRetValue<T>(env, js_ret_value);
    }

    static std::string GetString(napi_env env, napi_value js_str)
    {
        size_t length;
        [[maybe_unused]] napi_status status;
        status = napi_get_value_string_utf8(env, js_str, nullptr, 0, &length);
        assert(status == napi_ok);
        std::string v(length, '\0');
        size_t copied;
        status = napi_get_value_string_utf8(env, js_str, v.data(), length + 1, &copied);
        assert(status == napi_ok);
        assert(length == copied);
        return v;
    }

    template <typename T>
    static T GetRetValue([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value js_value)
    {
        if constexpr (std::is_same_v<T, double>) {
            double v;
            [[maybe_unused]] napi_status status = napi_get_value_double(env, js_value, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            int32_t v;
            [[maybe_unused]] napi_status status = napi_get_value_int32(env, js_value, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            uint32_t v;
            [[maybe_unused]] napi_status status = napi_get_value_uint32(env, js_value, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            int64_t v;
            [[maybe_unused]] napi_status status = napi_get_value_int64(env, js_value, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, bool>) {
            bool v;
            [[maybe_unused]] napi_status status = napi_get_value_bool(env, js_value, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return GetString(env, js_value);
        } else if constexpr (std::is_same_v<T, napi_value>) {
            return js_value;
        } else if constexpr (std::is_same_v<T, void>) {
            // do nothing
        } else {
            enum { INCORRECT_TEMPLATE_TYPE = false };
            static_assert(INCORRECT_TEMPLATE_TYPE, "Incorrect template type");
        }
    }

    static napi_value MakeJsArg(napi_env env, double arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_double(env, arg, &v);
        assert(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, int32_t arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_int32(env, arg, &v);
        assert(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, uint32_t arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_uint32(env, arg, &v);
        assert(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, int64_t arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_int64(env, arg, &v);
        assert(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, std::string_view arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_string_utf8(env, arg.data(), arg.length(), &v);
        assert(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg([[maybe_unused]] napi_env env, napi_value arg)
    {
        return arg;
    }

    static napi_value GetJsGtestObject(napi_env env)
    {
        [[maybe_unused]] napi_status status;

        // Get globalThis
        napi_value js_global_object;
        status = napi_get_global(env, &js_global_object);
        assert(status == napi_ok);

        // Get globalThis.gtest
        napi_value js_gtest_object;
        status = napi_get_named_property(env, js_global_object, "gtest", &js_gtest_object);
        assert(status == napi_ok);

        return js_gtest_object;
    }

    template <typename R, typename... Args>
    [[nodiscard]] static std::optional<R> DoCallEtsMethod(std::string_view fn_name, napi_env env, Args &&...args)
    {
        [[maybe_unused]] napi_status status;

        // Get globalThis.gtest
        napi_value js_gtest_object = GetJsGtestObject(env);

        // Set globalThis.gtest.functionName
        napi_value js_function_name;
        status = napi_create_string_utf8(env, fn_name.data(), fn_name.length(), &js_function_name);
        assert(status == napi_ok);
        status = napi_set_named_property(env, js_gtest_object, "functionName", js_function_name);
        assert(status == napi_ok);

        // Set globalThis.gtest.args
        std::initializer_list<napi_value> napi_args = {MakeJsArg(env, args)...};
        napi_value js_args;
        status = napi_create_array_with_length(env, napi_args.size(), &js_args);
        assert(status == napi_ok);
        uint32_t i = 0;
        for (auto arg : napi_args) {
            status = napi_set_element(env, js_args, i++, arg);
            assert(status == napi_ok);
        }
        status = napi_set_named_property(env, js_gtest_object, "args", js_args);
        assert(status == napi_ok);

        // Call ETS method via JS
        auto res = RunJsScript(R"(
            gtest.ret = gtest.etsVm.call(gtest.functionName, ...gtest.args);
        )");
        if (!res) {
            return std::nullopt;
        }

        // Get globalThis.gtest.ret
        napi_value js_ret_value {};
        status = napi_get_named_property(env, js_gtest_object, "ret", &js_ret_value);
        assert(status == napi_ok);
        return GetRetValue<R>(env, js_ret_value);
    }

    friend class JsEnvAccessor;
    static napi_env js_env_;

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string interop_js_test_path_;
};

}  // namespace panda::ets::interop::js::testing

#endif  // !PANDA_PLUGINS_ETS_INTEROP_JS_GTEST_H_
