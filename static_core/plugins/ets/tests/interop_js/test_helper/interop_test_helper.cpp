/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <array>
#include <new>
#include <vector>
#include <iostream>
#include <node_api.h>
#include <uv.h>
#include "napi/native_node_hybrid_api.h"
#include "interop_test_helper.h"
#include "native_engine/native_engine.h"

namespace ark::ets::interop::js::helper {

static constexpr const char *MODULE_PREFIX = "[INTEROP_TEST_HELPER] ";
static constexpr const char *CONCATENATED_ARGV_ENV_VAR = "CONCATENATED_ARGV_ENV_VAR";

// Owns one JS callback that is intentionally dispatched in a later libuv event-loop turn.
struct EventLoopCallbackData {
    napi_env env;
    napi_ref callback;
    // The boundary active before the Promise is completed from a stackful coroutine.
    NapiStackInfo expectedStackInfo;
    uv_async_t async;
};

static std::vector<napi_value> GetArgs(napi_env env, napi_callback_info info);

[[noreturn]] static void AbortOnEventLoopCallbackError(const char *message, napi_status status)
{
    std::cerr << MODULE_PREFIX << message << ", status = " << status << std::endl;
    std::abort();
}

static void CloseEventLoopCallback(uv_handle_t *handle)
{
    auto *data = reinterpret_cast<EventLoopCallbackData *>(handle->data);
    if (napi_delete_reference(data->env, data->callback) != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to delete event-loop callback reference", napi_generic_failure);
    }
    delete data;
}

static void RunEventLoopCallback(uv_async_t *async)
{
    auto *data = reinterpret_cast<EventLoopCallbackData *>(async->data);
    napi_value callback = nullptr;
    auto status = napi_get_reference_value(data->env, data->callback, &callback);
    if (status != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to get event-loop callback", status);
    }

    napi_value undefined = nullptr;
    status = napi_get_undefined(data->env, &undefined);
    if (status != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to get undefined", status);
    }

    // Promise reactions can run while napi_resolve_deferred() is still on the coroutine stack.
    // This later libuv callback observes the stack info left after the ETS-to-JS scope has closed.
    NapiStackInfo currentStackInfo {};
    status = napi_get_stackinfo(data->env, &currentStackInfo);
    if (status != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to get current stack info", status);
    }
    bool stackInfoRestored = currentStackInfo.stackStart == data->expectedStackInfo.stackStart &&
                             currentStackInfo.stackSize == data->expectedStackInfo.stackSize;

    napi_value callbackArg = nullptr;
    status = napi_get_boolean(data->env, stackInfoRestored, &callbackArg);
    if (status != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to create stack-info callback argument", status);
    }

    status = napi_call_function(data->env, undefined, callback, 1U, &callbackArg, nullptr);
    if (status != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to call event-loop callback", status);
    }

    uv_close(reinterpret_cast<uv_handle_t *>(async), CloseEventLoopCallback);
}

static napi_value TriggerEventLoopCallback(napi_env env, napi_callback_info info)
{
    void *callbackData = nullptr;
    if (napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &callbackData) != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to get event-loop trigger data", napi_generic_failure);
    }

    auto *data = reinterpret_cast<EventLoopCallbackData *>(callbackData);
    auto status = uv_async_send(&data->async);
    if (status != 0) {
        std::cerr << MODULE_PREFIX << "Failed to trigger event-loop callback, status = " << status << std::endl;
        std::abort();
    }

    napi_value undefined = nullptr;
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        AbortOnEventLoopCallbackError("Failed to get trigger result", napi_generic_failure);
    }
    return undefined;
}

static napi_value CreateEventLoopCallbackTrigger(napi_env env, napi_callback_info info)
{
    auto args = GetArgs(env, info);
    napi_valuetype callbackType;
    if (args.size() != 1 || napi_typeof(env, args[0], &callbackType) != napi_ok || callbackType != napi_function) {
        napi_throw_type_error(env, nullptr, "A callback function is required");
        return nullptr;
    }

    auto *data = new (std::nothrow) EventLoopCallbackData {env, nullptr, {}, {}};
    if (data == nullptr) {
        napi_throw_error(env, nullptr, "Failed to allocate event-loop callback data");
        return nullptr;
    }

    auto status = napi_get_stackinfo(env, &data->expectedStackInfo);
    if (status != napi_ok) {
        delete data;
        AbortOnEventLoopCallbackError("Failed to get initial stack info", status);
    }

    status = napi_create_reference(env, args[0], 1, &data->callback);
    if (status != napi_ok) {
        delete data;
        AbortOnEventLoopCallbackError("Failed to create event-loop callback reference", status);
    }

    uv_loop_t *loop = nullptr;
    status = napi_get_uv_event_loop(env, &loop);
    if (status != napi_ok) {
        napi_delete_reference(env, data->callback);
        delete data;
        AbortOnEventLoopCallbackError("Failed to get event loop", status);
    }

    // An active uv_async_t keeps the loop alive until the Promise reaction triggers this callback.
    auto uvStatus = uv_async_init(loop, &data->async, RunEventLoopCallback);
    if (uvStatus != 0) {
        napi_delete_reference(env, data->callback);
        delete data;
        std::cerr << MODULE_PREFIX << "Failed to initialize event-loop callback, status = " << uvStatus << std::endl;
        std::abort();
    }
    data->async.data = data;

    napi_value trigger = nullptr;
    status = napi_create_function(env, "triggerEventLoopCallback", NAPI_AUTO_LENGTH, TriggerEventLoopCallback, data,
                                  &trigger);
    if (status != napi_ok) {
        uv_close(reinterpret_cast<uv_handle_t *>(&data->async), CloseEventLoopCallback);
        AbortOnEventLoopCallbackError("Failed to create event-loop trigger", status);
    }
    return trigger;
}

bool RunAbcFileOnArkJSVM(napi_env env, const std::string_view path, std::string_view testName)
{
    auto *engine = reinterpret_cast<NativeEngine *>(env);
    auto *vm = const_cast<EcmaVM *>(engine->GetEcmaVm());

    std::string normalizedPath = panda::JSNApi::NormalizePath(path.data());
    std::string entrypoint(testName.data());

    return panda::JSNApi::Execute(vm, normalizedPath, entrypoint);
}

void LinkWithLibraryTrick() {}

static std::string GetStringFromNapiValue(napi_env env, napi_value name)
{
    napi_status status;

    size_t stringLength = 0;
    if (status = napi_get_value_string_utf8(env, name, nullptr, 0, &stringLength); status != napi_ok) {
        std::cerr << MODULE_PREFIX << "Can not get name size" << std::endl;
        return {};
    }

    if (stringLength == 0) {
        std::cerr << MODULE_PREFIX << "Environment variable name is empty" << std::endl;
        return {};
    }

    std::string result;
    result.reserve(stringLength + 1);
    result.resize(stringLength);

    size_t numberOfCopiedBytes = 0;

    if (napi_get_value_string_utf8(env, name, result.data(), stringLength + 1, &numberOfCopiedBytes) != napi_ok) {
        std::cerr << MODULE_PREFIX << "Failed to get name from napi_value" << std::endl;
        return {};
    }

    if (numberOfCopiedBytes != stringLength) {
        std::cerr << MODULE_PREFIX << "Failed to copy " << stringLength << " bytes from napi_value to string, "
                  << " only " << numberOfCopiedBytes << " has been copied" << std::endl;
        return {};
    }

    return result;
}

static napi_value GetEnvironmentVarImpl(napi_env env, napi_value name)
{
    napi_value result;
    napi_status status;

    auto nameStr = GetStringFromNapiValue(env, name);
    if (nameStr.empty()) {
        std::cerr << MODULE_PREFIX << "Failed to convert napi_value name to string " << std::endl;
        napi_get_undefined(env, &result);
        return result;
    }

    auto envVar = std::getenv(nameStr.c_str());
    if (envVar == nullptr) {
        std::cerr << MODULE_PREFIX << "Environment variable " << nameStr << " should be set" << std::endl;
        napi_get_undefined(env, &result);
        return result;
    }

    if (status = napi_create_string_utf8(env, envVar, NAPI_AUTO_LENGTH, &result); status != napi_ok) {
        std::cerr << MODULE_PREFIX << "Failed to create napi string from `" << envVar << "`, status = " << status
                  << std::endl;
        std::abort();
    }

    return result;
}

static std::vector<napi_value> GetArgs(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);

    napi_value thisVar = nullptr;
    std::vector<napi_value> argv(argc);
    napi_get_cb_info(env, info, &argc, argv.data(), &thisVar, nullptr);
    return argv;
}

static napi_value GetEnvironmentVar(napi_env env, napi_callback_info info)
{
    return GetEnvironmentVarImpl(env, GetArgs(env, info)[0]);
}

// Return array of argv that was passed into main when process starts
static napi_value GetArgv(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value argvArray;
    napi_status status;

    // Since we have not access to `process` library in js this workaround helps to obtain argv
    // Before js will be called, all argv parameters must be concatenated using `|` separator
    // and set in `CONCATENATED_ARGV_ENV_VAR` environment variable (currently concatination doing in ark_js_napi_cli)
    char *envVar = std::getenv(CONCATENATED_ARGV_ENV_VAR);
    if (envVar == nullptr) {
        std::cerr << MODULE_PREFIX << "Environment variable " << CONCATENATED_ARGV_ENV_VAR << " should be set"
                  << std::endl;
        napi_get_undefined(env, &argvArray);
        return argvArray;
    }

    if (status = napi_create_array(env, &argvArray); status != napi_ok) {
        std::cerr << MODULE_PREFIX << "Can't create array, status = " << status << std::endl;
        napi_get_undefined(env, &argvArray);
        return argvArray;
    }

    std::string envString(envVar);
    char separator = '|';

    auto pos = envString.find(separator);
    for (size_t idx = 0; pos != std::string::npos; ++idx) {
        auto currArgv = envString.substr(0, pos);
        napi_value napiCurrArgv;

        if (status = napi_create_string_utf8(env, currArgv.c_str(), currArgv.size(), &napiCurrArgv);
            status != napi_ok) {
            std::cerr << MODULE_PREFIX << "Failed to create string `" << currArgv << "`, napi_status = " << status
                      << std::endl;
            std::abort();
        }

        if (status = napi_set_element(env, argvArray, idx, napiCurrArgv); status != napi_ok) {
            std::cerr << MODULE_PREFIX << "Failed to set element with string `" << currArgv
                      << "` in array, napi_status = " << status << std::endl;
            std::abort();
        }

        envString.erase(0, pos + 1);
        pos = envString.find(separator);
    }

    return argvArray;
}

static napi_value Init(napi_env env, napi_value exports)
{
    const std::array desc = {
        napi_property_descriptor {"getEnvironmentVar", 0, GetEnvironmentVar, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"getArgv", 0, GetArgv, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"createEventLoopCallbackTrigger", 0, CreateEventLoopCallbackTrigger, 0, 0, 0,
                                  napi_enumerable, 0},
    };

    if (napi_define_properties(env, exports, desc.size(), desc.data()) != napi_ok) {
        std::cerr << MODULE_PREFIX << "Failed to define properties" << std::endl;
        std::abort();
    }

    return exports;
}

NAPI_MODULE(INTEROP_TEST_HELPER, ark::ets::interop::js::helper::Init)

}  // namespace ark::ets::interop::js::helper
