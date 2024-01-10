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
        bool hasPendingExc;
        [[maybe_unused]] napi_status status = napi_is_exception_pending(GetJsEnv(), &hasPendingExc);
        assert(status == napi_ok && !hasPendingExc);

        interopJsTestPath_ = std::getenv("ARK_ETS_INTEROP_JS_GTEST_SOURCES");
        // This object is used to save global js names
        if (!RunJsScript("var gtest_env = {};\n")) {
            std::abort();
        }
    }

    [[nodiscard]] static bool RunJsScript(const std::string &script)
    {
        return DoRunJsScript(jsEnv_, script);
    }

    template <typename R>
    [[nodiscard]] std::optional<R> RunJsScriptByPath(const std::string &path)
    {
        return DoRunJsScriptByPath<R>(jsEnv_, path);
    }

    [[nodiscard]] bool RunJsTestSuite(const std::string &path)
    {
        return DoRunJsTestSuite(jsEnv_, path);
    }

    template <typename R, typename... Args>
    [[nodiscard]] std::optional<R> CallEtsMethod(std::string_view fnName, Args &&...args)
    {
        return DoCallEtsMethod<R>(fnName, jsEnv_, std::forward<Args>(args)...);
    }

    static napi_env GetJsEnv()
    {
        return jsEnv_;
    }

    void LoadModuleAs(const std::string &moduleName, const std::string &modulePath)
    {
        auto res =
            RunJsScript("gtest_env." + moduleName + " = require(\"" + interopJsTestPath_ + "/" + modulePath + "\");\n");
        if (!res) {
            std::cerr << "Failed to load module: " << modulePath << std::endl;
            std::abort();
        }
    }

private:
    [[nodiscard]] bool DoRunJsTestSuite(napi_env env, const std::string &path)
    {
        std::string fullPath = interopJsTestPath_ + "/" + path;
        std::string testSouceCode = ReadFile(fullPath);

        // NOTE:
        //  We wrap the test source code to anonymous lambda function to avoid pollution the global namespace.
        //  We also set the 'use strict' mode, because right now we are checking interop only in the strict mode.
        return DoRunJsScript(env, "(() => { \n'use strict'\n" + testSouceCode + "\n})()");
    }

    [[nodiscard]] static bool DoRunJsScript(napi_env env, const std::string &script)
    {
        [[maybe_unused]] napi_status status;
        napi_value jsScript;
        status = napi_create_string_utf8(env, script.c_str(), script.length(), &jsScript);
        assert(status == napi_ok);

        napi_value jsResult;
        status = napi_run_script(env, jsScript, &jsResult);
        if (status == napi_generic_failure) {
            std::cerr << "EtsInteropTest fired an exception" << std::endl;
            napi_value exc;
            status = napi_get_and_clear_last_exception(env, &exc);
            assert(status == napi_ok);
            napi_value jsStr;
            status = napi_coerce_to_string(env, exc, &jsStr);
            assert(status == napi_ok);
            std::cerr << "Exception string: " << GetString(env, jsStr) << std::endl;
            return false;
        }
        return status == napi_ok;
    }

    static std::string ReadFile(const std::string &fullPath)
    {
        std::ifstream jsSourceFile(fullPath);
        if (!jsSourceFile.is_open()) {
            std::cerr << __func__ << ": Cannot open file: " << fullPath << std::endl;
            std::abort();
        }
        std::ostringstream stringStream;
        stringStream << jsSourceFile.rdbuf();
        return stringStream.str();
    }

    template <typename T>
    [[nodiscard]] std::optional<T> DoRunJsScriptByPath(napi_env env, const std::string &path)
    {
        std::string fullPath = interopJsTestPath_ + "/" + path;

        if (!DoRunJsScript(env, ReadFile(fullPath))) {
            return std::nullopt;
        }

        [[maybe_unused]] napi_status status;
        napi_value jsRetValue {};
        status = napi_get_named_property(env, GetJsGtestObject(env), "ret", &jsRetValue);
        assert(status == napi_ok);

        // Get globalThis.gtest.ret
        return GetRetValue<T>(env, jsRetValue);
    }

    static std::string GetString(napi_env env, napi_value jsStr)
    {
        size_t length;
        [[maybe_unused]] napi_status status = napi_get_value_string_utf8(env, jsStr, nullptr, 0, &length);
        assert(status == napi_ok);
        std::string v(length, '\0');
        size_t copied;
        status = napi_get_value_string_utf8(env, jsStr, v.data(), length + 1, &copied);
        assert(status == napi_ok);
        assert(length == copied);
        return v;
    }

    template <typename T>
    static T GetRetValue([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value jsValue)
    {
        if constexpr (std::is_same_v<T, double>) {
            double v;
            [[maybe_unused]] napi_status status = napi_get_value_double(env, jsValue, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            int32_t v;
            [[maybe_unused]] napi_status status = napi_get_value_int32(env, jsValue, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            uint32_t v;
            [[maybe_unused]] napi_status status = napi_get_value_uint32(env, jsValue, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            int64_t v;
            [[maybe_unused]] napi_status status = napi_get_value_int64(env, jsValue, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, bool>) {
            bool v;
            [[maybe_unused]] napi_status status = napi_get_value_bool(env, jsValue, &v);
            assert(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return GetString(env, jsValue);
        } else if constexpr (std::is_same_v<T, napi_value>) {
            return jsValue;
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
        napi_value jsGlobalObject;
        status = napi_get_global(env, &jsGlobalObject);
        assert(status == napi_ok);

        // Get globalThis.gtest
        napi_value jsGtestObject;
        status = napi_get_named_property(env, jsGlobalObject, "gtest", &jsGtestObject);
        assert(status == napi_ok);

        return jsGtestObject;
    }

    template <typename R, typename... Args>
    [[nodiscard]] static std::optional<R> DoCallEtsMethod(std::string_view fnName, napi_env env, Args &&...args)
    {
        [[maybe_unused]] napi_status status;

        // Get globalThis.gtest
        napi_value jsGtestObject = GetJsGtestObject(env);

        // Set globalThis.gtest.functionName
        napi_value jsFunctionName;
        status = napi_create_string_utf8(env, fnName.data(), fnName.length(), &jsFunctionName);
        assert(status == napi_ok);
        status = napi_set_named_property(env, jsGtestObject, "functionName", jsFunctionName);
        assert(status == napi_ok);

        // Set globalThis.gtest.args
        std::initializer_list<napi_value> napiArgs = {MakeJsArg(env, args)...};
        napi_value jsArgs;
        status = napi_create_array_with_length(env, napiArgs.size(), &jsArgs);
        assert(status == napi_ok);
        uint32_t i = 0;
        for (auto arg : napiArgs) {
            status = napi_set_element(env, jsArgs, i++, arg);
            assert(status == napi_ok);
        }
        status = napi_set_named_property(env, jsGtestObject, "args", jsArgs);
        assert(status == napi_ok);

        // Call ETS method via JS
        auto res = RunJsScript(R"(
            gtest.ret = gtest.etsVm.call(gtest.functionName, ...gtest.args);
        )");
        if (!res) {
            return std::nullopt;
        }

        // Get globalThis.gtest.ret
        napi_value jsRetValue {};
        status = napi_get_named_property(env, jsGtestObject, "ret", &jsRetValue);
        assert(status == napi_ok);
        return GetRetValue<R>(env, jsRetValue);
    }

    friend class JsEnvAccessor;
    static napi_env jsEnv_;

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::string interopJsTestPath_;
};

}  // namespace panda::ets::interop::js::testing

#endif  // !PANDA_PLUGINS_ETS_INTEROP_JS_GTEST_H_
