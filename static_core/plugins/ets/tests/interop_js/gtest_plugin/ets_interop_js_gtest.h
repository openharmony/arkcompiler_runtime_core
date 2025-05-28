/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "libpandabase/macros.h"
#include "interop_test_helper.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "utils/span.h"

// NOLINTBEGIN(readability-identifier-naming)
// CC-OFFNXT(G.FMT.10) project code style
__attribute__((weak)) napi_status napi_load_module_with_module_request([[maybe_unused]] napi_env env,
                                                                       [[maybe_unused]] const char *request_name,
                                                                       [[maybe_unused]] napi_value *result);
// NOLINTEND(readability-identifier-naming)

namespace ark::ets::interop::js::testing {

class EtsInteropTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        if (std::getenv("ARK_ETS_INTEROP_JS_GTEST_SOURCES") == nullptr) {
            std::cerr << "ARK_ETS_INTEROP_JS_GTEST_SOURCES not set" << std::endl;
            std::abort();
        }

        if (std::getenv("JS_ABC_OUTPUT_PATH") == nullptr) {
            std::cerr << "JS_ABC_OUTPUT_PATH not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        bool hasPendingExc;
        [[maybe_unused]] napi_status status = napi_is_exception_pending(GetJsEnv(), &hasPendingExc);
        ASSERT(status == napi_ok && !hasPendingExc);

        interopJsTestPath_ = std::getenv("ARK_ETS_INTEROP_JS_GTEST_SOURCES");
        jsAbcFilePath_ = std::getenv("JS_ABC_OUTPUT_PATH");

        // This object is used to save global js names
        if (!SetGtestEnv()) {
            std::abort();
        }

        auto packageName = std::getenv("PACKAGE_NAME");
        if (packageName != nullptr) {
            packageName_ = packageName;
        } else {
            std::cerr << "PACKAGE_NAME is not set" << std::endl;
            std::abort();
        }
    }

    bool SetGtestEnv()
    {
        napi_value jsGlobalObject = GetGlobalObject(jsEnv_);

        napi_value jsObject;
        [[maybe_unused]] napi_status status = napi_create_object(jsEnv_, &jsObject);
        ASSERT(status == napi_ok);

        status = napi_set_named_property(jsEnv_, jsGlobalObject, "gtest_env", jsObject);

        return status == napi_ok;
    }

    template <typename R, typename... Args>
    [[nodiscard]] std::optional<R> CallJsMethod(std::string_view fnName, std::string fileName)
    {
        LoadModuleAs(fileName, fileName);
        return DoCallJsFunction<R>(fileName, fnName, jsEnv_);
    }

    // legacy API to work with unnamed packages
    template <typename R, typename... Args>
    [[nodiscard]] std::optional<R> CallEtsMethod(std::string_view fnName, Args &&...args)
    {
        return DoCallEtsFunction<R>("", fnName, jsEnv_, std::forward<Args>(args)...);
    }

    [[nodiscard]] static bool RunJsScript(const std::string &script)
    {
        return DoRunJsScript(jsEnv_, script);
    }

    [[nodiscard]] bool RunJsTestSuite(const std::string &path)
    {
        return DoRunJsTestSuite(jsEnv_, path);
    }

    template <typename R, typename... Args>
    [[nodiscard]] std::optional<R> CallEtsFunction(std::string_view pkg, std::string_view fnName, Args &&...args)
    {
        return DoCallEtsFunction<R>(pkg, fnName, jsEnv_, std::forward<Args>(args)...);
    }

    static napi_env GetJsEnv()
    {
        return jsEnv_;
    }

    void LoadModuleAs(const std::string &moduleName, const std::string &modulePath)
    {
        auto res = LoadModule(moduleName, modulePath);
        if (!res) {
            std::cerr << "Failed to load module: " << modulePath << std::endl;
            std::abort();
        }
    }

    const std::string &GetPackageName()
    {
        return packageName_;
    }

private:
    bool LoadModule(const std::string &moduleName, const std::string &modulePath)
    {
        napi_value jsModule;
        std::string pathToModule = jsAbcFilePath_ + "/" + modulePath;

        napi_status status = napi_load_module_with_module_request(jsEnv_, pathToModule.c_str(), &jsModule);
        if (status == napi_pending_exception) {
            napi_value exp;
            napi_get_and_clear_last_exception(jsEnv_, &exp);
            napi_fatal_exception(jsEnv_, exp);
            std::cerr << "Unable to load module due to exception" << std::endl;
            UNREACHABLE();
        }

        napi_value jsGtestEnvObject = GetJsGtestEnvObject(jsEnv_);
        status = napi_set_named_property(jsEnv_, jsGtestEnvObject, moduleName.c_str(), jsModule);

        return status == napi_ok;
    }

    [[nodiscard]] static bool DoRunJsScript(napi_env env, const std::string &script)
    {
        [[maybe_unused]] napi_status status;
        napi_value jsScript;
        status = napi_create_string_utf8(env, script.c_str(), script.length(), &jsScript);
        ASSERT(status == napi_ok);

        napi_value jsResult;
        status = napi_run_script(env, jsScript, &jsResult);
        if (status == napi_generic_failure) {
            std::cerr << "EtsInteropTest fired an exception" << std::endl;
            napi_value exc;
            status = napi_get_and_clear_last_exception(env, &exc);
            ASSERT(status == napi_ok);
            napi_value jsStr;
            status = napi_coerce_to_string(env, exc, &jsStr);
            ASSERT(status == napi_ok);
            std::cerr << "Exception string: " << GetString(env, jsStr) << std::endl;
            return false;
        }
        return status == napi_ok;
    }

    [[nodiscard]] bool DoRunJsTestSuite(napi_env env, const std::string &path)
    {
        std::string fileWithoutExtension = path.substr(0, path.find_last_of('.'));
        std::string outputPath = jsAbcFilePath_ + "/" + fileWithoutExtension + ".abc";

        // RunAbcFileOnArkJSVM call should be replaced with napi_execute_abc after it is implemented (#20536).
        return interop::js::helper::RunAbcFileOnArkJSVM(env, outputPath.c_str(), fileWithoutExtension.c_str());
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

    static std::string GetString(napi_env env, napi_value jsStr)
    {
        size_t length;
        [[maybe_unused]] napi_status status = napi_get_value_string_utf8(env, jsStr, nullptr, 0, &length);
        ASSERT(status == napi_ok);
        std::string v(length, '\0');
        size_t copied;
        status = napi_get_value_string_utf8(env, jsStr, v.data(), length + 1, &copied);
        ASSERT(status == napi_ok);
        ASSERT(length == copied);
        return v;
    }

    template <typename T>
    static T GetRetValue([[maybe_unused]] napi_env env, [[maybe_unused]] napi_value jsValue)
    {
        if constexpr (std::is_same_v<T, double>) {
            double v;
            [[maybe_unused]] napi_status status = napi_get_value_double(env, jsValue, &v);
            ASSERT(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            int32_t v;
            [[maybe_unused]] napi_status status = napi_get_value_int32(env, jsValue, &v);
            ASSERT(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            uint32_t v;
            [[maybe_unused]] napi_status status = napi_get_value_uint32(env, jsValue, &v);
            ASSERT(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            int64_t v;
            [[maybe_unused]] napi_status status = napi_get_value_int64(env, jsValue, &v);
            ASSERT(status == napi_ok);
            return v;
        } else if constexpr (std::is_same_v<T, bool>) {
            bool v;
            [[maybe_unused]] napi_status status = napi_get_value_bool(env, jsValue, &v);
            ASSERT(status == napi_ok);
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
        ASSERT(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, int32_t arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_int32(env, arg, &v);
        ASSERT(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, uint32_t arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_uint32(env, arg, &v);
        ASSERT(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, int64_t arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_int64(env, arg, &v);
        ASSERT(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg(napi_env env, std::string_view arg)
    {
        napi_value v;
        [[maybe_unused]] napi_status status = napi_create_string_utf8(env, arg.data(), arg.length(), &v);
        ASSERT(status == napi_ok);
        return v;
    }

    static napi_value MakeJsArg([[maybe_unused]] napi_env env, napi_value arg)
    {
        return arg;
    }

    static napi_value GetGlobalObject(napi_env env)
    {
        [[maybe_unused]] napi_status status;

        // Get globalThis
        napi_value jsGlobalObject;
        status = napi_get_global(env, &jsGlobalObject);
        ASSERT(status == napi_ok);

        return jsGlobalObject;
    }

    static napi_value GetJsGtestObject(napi_env env)
    {
        [[maybe_unused]] napi_status status;

        // Get globalThis
        napi_value jsGlobalObject;
        status = napi_get_global(env, &jsGlobalObject);
        ASSERT(status == napi_ok);

        // Get globalThis.gtest
        napi_value jsGtestObject;
        status = napi_get_named_property(env, jsGlobalObject, "gtest", &jsGtestObject);
        ASSERT(status == napi_ok);

        return jsGtestObject;
    }

    static napi_value GetJsGtestEnvObject(napi_env env)
    {
        [[maybe_unused]] napi_status status;

        napi_value jsGlobalObject = GetGlobalObject(jsEnv_);

        // Get globalThis.gtest_env
        napi_value jsObject;
        status = napi_get_named_property(env, jsGlobalObject, "gtest_env", &jsObject);
        ASSERT(status == napi_ok);

        return jsObject;
    }

    template <typename R, typename... Args>
    [[nodiscard]] static std::optional<R> DoCallEtsFunction(std::string_view package, std::string_view fnName,
                                                            napi_env env, Args &&...args)
    {
        std::string qualifiedName = std::string(package) + '.' + fnName.data();
        std::vector<napi_value> napiArgs = {MakeJsArg(env, args)...};
        Span<napi_value> jsargv(napiArgs);

        auto coro = EtsCoroutine::GetCurrent();
        auto ctx = InteropCtx::Current(coro);
        INTEROP_CODE_SCOPE_JS(coro);
        ScopedManagedCodeThread managedCode(coro);

        auto methodRes = ResolveEntryPoint(ctx, qualifiedName);
        if (UNLIKELY(!methodRes)) {
            InteropCtx::ThrowJSError(env, "CallEtsFunction: " + qualifiedName + " " + std::string(methodRes.Error()));
            return std::nullopt;
        }

        auto res = CallETSStatic(coro, ctx, methodRes.Value(), jsargv);
        if (!res) {
            return std::nullopt;
        }

        return GetRetValue<R>(env, res);
    }

    template <typename R, typename... Args>
    [[nodiscard]] static std::optional<R> DoCallJsFunction(std::string_view modName, std::string_view fnName,
                                                           napi_env env)
    {
        napi_value calledFn;
        napi_value arg;
        napi_value jsUndefined;
        napi_value jsGtestEnvObject = GetJsGtestEnvObject(jsEnv_);

        napi_value jsModule;
        [[maybe_unused]] napi_status status = napi_get_named_property(env, jsGtestEnvObject, modName.data(), &jsModule);
        ASSERT(status == napi_ok);
        status = napi_get_named_property(env, jsModule, fnName.data(), &calledFn);
        ASSERT(status == napi_ok);

        status = napi_get_undefined(env, &jsUndefined);
        ASSERT(status == napi_ok);
        napi_value *argv = &arg;
        size_t argc = 0;

        napi_value returnVal;
        status = napi_call_function(env, jsUndefined, calledFn, argc, argv, &returnVal);
        ASSERT(status == napi_ok);
        return GetRetValue<R>(env, returnVal);
    }

    friend class JsEnvAccessor;
    static napi_env jsEnv_;

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string interopJsTestPath_;
    std::string jsAbcFilePath_;
    std::string packageName_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::ets::interop::js::testing

#endif  // !PANDA_PLUGINS_ETS_INTEROP_JS_GTEST_H_
