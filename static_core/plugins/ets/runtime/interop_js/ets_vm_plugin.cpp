/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/event_loop_module.h"
#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/ts2ets_copy.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/timer_module.h"
#include "generated/base_options.h"
#include "compiler_options.h"
#include "compiler/compiler_logger.h"
#include "interop_js/napi_impl/napi_impl.h"

namespace ark::ets::interop::js {

static napi_value Version(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();
    constexpr std::string_view MSG = "0.1";

    napi_value result;
    [[maybe_unused]] napi_status status = napi_create_string_utf8(env, MSG.data(), MSG.size(), &result);
    ASSERT(status == napi_ok);

    return result;
}

static napi_value Fatal([[maybe_unused]] napi_env env, [[maybe_unused]] napi_callback_info info)
{
    [[maybe_unused]] JSNapiEnvScope napiScope(InteropCtx::Current(), env);
    InteropCtx::Fatal("etsVm.Fatal");
}

static napi_value GetEtsFunction(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));
    if (jsArgc != 2U && jsArgc != 1U) {
        InteropCtx::ThrowJSError(env, "GetEtsFunction: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }

    std::array<napi_value, 2U> jsArgv {};
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, jsArgv.data(), nullptr, nullptr));

    napi_value jsFunctionName = jsArgc == 1 ? jsArgv[0] : jsArgv[1];

    std::string packageName;
    if (jsArgc == 2U) {
        napi_value jsPackageName = jsArgv[0];
        if (GetValueType(env, jsPackageName) != napi_string) {
            InteropCtx::ThrowJSError(env, "GetEtsFunction: package name is not a string");
        }
        packageName = GetString(env, jsPackageName);
    }

    if (GetValueType(env, jsFunctionName) != napi_string) {
        InteropCtx::ThrowJSError(env, "GetEtsFunction: function name is not a string");
        return nullptr;
    }

    std::string methodName = GetString(env, jsFunctionName);

    return ets_proxy::GetETSFunction(env, packageName, methodName);
}

static napi_value GetEtsClass(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t jsArgc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, nullptr, nullptr, nullptr));

    if (jsArgc != 1) {
        InteropCtx::ThrowJSError(env, "GetEtsClass: bad args, actual args count: " + std::to_string(jsArgc));
        return nullptr;
    }

    napi_value jsClassDescriptor {};
    ASSERT(jsArgc == 1);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &jsArgc, &jsClassDescriptor, nullptr, nullptr));

    std::string classDescriptor = GetString(env, jsClassDescriptor);
    return ets_proxy::GetETSClass(env, classDescriptor);
}

static napi_value CallEtsFunctionImpl(EtsCoroutine *coro, InteropCtx *ctx, Span<napi_value> jsargv)
{
    auto env = ctx->GetJSEnv();

    if (UNLIKELY(jsargv.Empty())) {
        InteropCtx::ThrowJSError(env, "CallEtsFunction: method name required");
        return nullptr;
    }
    if (UNLIKELY(GetValueType(env, jsargv[0]) != napi_string)) {
        InteropCtx::ThrowJSError(env, "CallEtsFunction: method name is not a string");
        return nullptr;
    }

    auto callTarget = GetString(env, jsargv[0]);
    INTEROP_LOG(DEBUG) << "CallEtsFunction: method name: " << callTarget;

    auto methodRes = ResolveEntryPoint(ctx, callTarget);
    if (UNLIKELY(!methodRes)) {
        InteropCtx::ThrowJSError(env, "CallEtsFunction: " + callTarget + " " + methodRes.Error());
        return nullptr;
    }
    return CallETSStatic(coro, ctx, methodRes.Value(), jsargv.SubSpan(1));
}

static napi_value Call(napi_env env, napi_callback_info info)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro, env);

    size_t argc = 0;
    [[maybe_unused]] napi_status status = napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    ASSERT(status == napi_ok);

    auto argv = ctx->GetTempArgs<napi_value>(argc);
    napi_value thisArg {};
    void *data = nullptr;
    status = napi_get_cb_info(env, info, &argc, argv->data(), &thisArg, &data);
    ASSERT(status == napi_ok);

    return CallEtsFunctionImpl(coro, ctx, *argv);
}

static napi_value CallWithCopy(napi_env env, napi_callback_info info)
{
    ASSERT_SCOPED_NATIVE_CODE();

    size_t argc = 0;
    [[maybe_unused]] napi_status status = napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    ASSERT(status == napi_ok);

    auto coro = EtsCoroutine::GetCurrent();
    auto argv = InteropCtx::Current(coro)->GetTempArgs<napi_value>(argc);
    napi_value thisArg {};
    void *data = nullptr;
    status = napi_get_cb_info(env, info, &argc, argv->data(), &thisArg, &data);
    ASSERT(status == napi_ok);

    return InvokeEtsMethodImpl(env, argv->data(), argc, false);
}

static bool RegisterTimerModule(napi_env jsEnv)
{
    EtsVM *vm = nullptr;
    ets_size count = 0;
    ETS_GetCreatedVMs(&vm, 1, &count);
    if (count != 1) {
        LogError("No one VM is created");
        return false;
    }
    EtsEnv *etsEnv = nullptr;
    vm->GetEnv(&etsEnv, ETS_NAPI_VERSION_1_0);
    return TimerModule::Init(etsEnv, jsEnv);
}

static std::vector<napi_value> GetArgs(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr));

    std::vector<napi_value> argv(argc);
    napi_value thisArg {};
    void *data = nullptr;
    NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, argv.data(), &thisArg, &data));

    return argv;
}

static void RegisterEventLoopModule()
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(coro == coro->GetPandaVM()->GetCoroutineManager()->GetMainThread());
    coro->GetPandaVM()->CreateCallbackPosterFactory<EventLoopCallbackPosterFactoryImpl>();
}

static napi_value CreateEtsRuntime(napi_env env, napi_callback_info info)
{
    napi_value napiFalse;
    NAPI_ASSERT_OK(napi_get_boolean(env, false, &napiFalse));

    auto argv = GetArgs(env, info);
    if (argv.size() != 4U) {
        LogError("CreateEtsRuntime: exactly 4 arguments are required");
        return napiFalse;
    }

    napi_valuetype type;
    napi_typeof(env, argv[0], &type);
    if (type != napi_string) {
        LogError("CreateEtsRuntime: first argument is not a string");
        return napiFalse;
    }
    auto stdlibPath = GetString(env, argv[0]);

    napi_typeof(env, argv[1], &type);
    if (type != napi_string) {
        LogError("CreateEtsRuntime: second argument is not a string");
        return napiFalse;
    }
    auto indexPath = GetString(env, argv[1]);

    napi_typeof(env, argv[2U], &type);
    if (type != napi_boolean) {
        LogError("CreateEtsRuntime: third argument is not a boolean");
        return napiFalse;
    }
    bool useJit;
    napi_get_value_bool(env, argv[2U], &useJit);

    napi_typeof(env, argv[3U], &type);
    if (type != napi_boolean) {
        LogError("CreateEtsRuntime: fourth argument is not a boolean");
        return napiFalse;
    }
    bool useAot;
    napi_get_value_bool(env, argv[3U], &useAot);

    bool res = ark::ets::CreateRuntime(stdlibPath, indexPath, useJit, useAot);
    res &= RegisterTimerModule(env);
    RegisterEventLoopModule();
    if (res) {
        auto coro = EtsCoroutine::GetCurrent();
        ScopedManagedCodeThread scoped(coro);
        napi_add_env_cleanup_hook(
            env, [](void *) { DestroyRuntime(); }, nullptr);
        InteropCtx::Init(coro, env);
    }
    napi_value napiRes;
    NAPI_ASSERT_OK(napi_get_boolean(env, res, &napiRes));
    return napiRes;
}

static std::optional<std::vector<std::string>> GetArgStrings(napi_env env, napi_value options)
{
    uint32_t numOptions;
    std::vector<std::string> argStrings;
    if (napi_get_array_length(env, options, &numOptions) == napi_ok) {
        // options passed as an array
        argStrings.reserve(numOptions + 1);
        argStrings.emplace_back("argv[0] placeholder");

        for (uint32_t i = 0; i < numOptions; ++i) {
            napi_value option;
            NAPI_ASSERT_OK(napi_get_element(env, options, i, &option));
            if (napi_coerce_to_string(env, option, &option) != napi_ok) {
                LogError("Option values must be coercible to string");
                return std::nullopt;
            }
            argStrings.push_back(GetString(env, option));
        }
    } else {
        // options passed as a map
        napi_value propNames;
        NAPI_ASSERT_OK(napi_get_property_names(env, options, &propNames));
        NAPI_ASSERT_OK(napi_get_array_length(env, propNames, &numOptions));

        argStrings.reserve(numOptions + 1);
        argStrings.emplace_back("argv[0] placeholder");

        for (uint32_t i = 0; i < numOptions; ++i) {
            napi_value key;
            napi_value value;
            NAPI_ASSERT_OK(napi_get_element(env, propNames, i, &key));
            NAPI_ASSERT_OK(napi_get_property(env, options, key, &value));
            if (napi_coerce_to_string(env, value, &value) != napi_ok) {
                LogError("Option values must be coercible to string");
                return std::nullopt;
            }
            argStrings.push_back("--" + GetString(env, key) + "=" + GetString(env, value));
        }
    }

    return argStrings;
}

static bool AddOptions(base_options::Options *baseOptions, ark::RuntimeOptions *runtimeOptions,
                       const std::vector<std::string> &argStrings)
{
    ark::PandArgParser paParser;
    baseOptions->AddOptions(&paParser);
    runtimeOptions->AddOptions(&paParser);
    ark::compiler::g_options.AddOptions(&paParser);

    std::vector<const char *> fakeArgv;
    fakeArgv.reserve(argStrings.size());
    for (auto const &arg : argStrings) {
        fakeArgv.push_back(arg.c_str());  // Be careful, do not reallocate referenced strings
    }

    if (!paParser.Parse(fakeArgv.size(), fakeArgv.data())) {
        LogError("Parse options failed. Optional arguments:\n" + paParser.GetHelpString());
        return false;
    }

    auto runtimeOptionsErr = runtimeOptions->Validate();
    if (runtimeOptionsErr) {
        LogError("Parse options failed: " + runtimeOptionsErr.value().GetMessage());
        return false;
    }
    ark::compiler::CompilerLogger::SetComponents(ark::compiler::g_options.GetCompilerLog());
    return true;
}

static napi_value CreateRuntime(napi_env env, napi_callback_info info)
{
    size_t constexpr ARGC = 1;
    std::array<napi_value, ARGC> argv {};

    size_t argc = ARGC;
    NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, argv.data(), nullptr, nullptr));

    napi_value napiFalse;
    NAPI_ASSERT_OK(napi_get_boolean(env, false, &napiFalse));

    if (argc != ARGC) {
        LogError("CreateEtsRuntime: bad args number");
        return napiFalse;
    }

    napi_value options = argv[0];
    if (GetValueType(env, options) != napi_object) {
        LogError("CreateEtsRuntime: argument is not an object");
        return napiFalse;
    }

    auto argStrings = GetArgStrings(env, options);
    if (argStrings == std::nullopt) {
        return napiFalse;
    }

    auto addOpts = [&argStrings](base_options::Options *baseOptions, ark::RuntimeOptions *runtimeOptions) {
        return AddOptions(baseOptions, runtimeOptions, argStrings.value());
    };

    bool res = ets::CreateRuntime(addOpts);
    res &= RegisterTimerModule(env);
    RegisterEventLoopModule();
    if (res) {
        auto coro = EtsCoroutine::GetCurrent();
        ScopedManagedCodeThread scoped(coro);
        napi_add_env_cleanup_hook(
            env, [](void *) { DestroyRuntime(); }, nullptr);
        InteropCtx::Init(coro, env);
    }
    napi_value napiRes;
    NAPI_ASSERT_OK(napi_get_boolean(env, res, &napiRes));
    return napiRes;
}

static napi_value Init(napi_env env, napi_value exports)
{
    const std::array desc = {
        napi_property_descriptor {"version", 0, Version, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"fatal", 0, Fatal, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"call", 0, Call, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"callWithCopy", 0, CallWithCopy, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"createEtsRuntime", 0, CreateEtsRuntime, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"createRuntime", 0, CreateRuntime, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"getFunction", 0, GetEtsFunction, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"getClass", 0, GetEtsClass, 0, 0, 0, napi_enumerable, 0},
    };

    NAPI_CHECK_FATAL(napi_define_properties(env, exports, desc.size(), desc.data()));

    NapiImpl::InitNapi();

    return exports;
}

}  // namespace ark::ets::interop::js

NAPI_MODULE(ETS_INTEROP_JS_NAPI, ark::ets::interop::js::Init)
