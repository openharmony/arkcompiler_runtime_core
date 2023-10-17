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

#include <node_api.h>
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_vm_api.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_proxy.h"
#include "plugins/ets/runtime/interop_js/js_value_call.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/ts2ets_copy.h"
#include "generated/base_options.h"
#include "compiler_options.h"
#include "compiler/compiler_logger.h"

namespace panda::ets::interop::js {

static napi_value Version(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    constexpr std::string_view MSG = "0.1";

    napi_value result;
    [[maybe_unused]] napi_status status = napi_create_string_utf8(env, MSG.data(), MSG.size(), &result);
    assert(status == napi_ok);

    return result;
}

static napi_value Fatal([[maybe_unused]] napi_env env, [[maybe_unused]] napi_callback_info info)
{
    InteropCtx::Fatal("etsVm.Fatal");
}

static napi_value GetEtsFunction(napi_env env, napi_callback_info info)
{
    size_t js_argc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &js_argc, nullptr, nullptr, nullptr));
    if (js_argc != 2) {
        InteropCtx::ThrowJSError(env, "GetEtsFunction: bad args, actual args count: " + std::to_string(js_argc));
        return nullptr;
    }

    std::array<napi_value, 2> js_argv {};
    ASSERT(js_argc == js_argv.size());
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &js_argc, js_argv.data(), nullptr, nullptr));

    napi_value js_class_descriptor = js_argv[0];
    napi_value js_function_name = js_argv[1];

    if (GetValueType(env, js_class_descriptor) != napi_string) {
        InteropCtx::ThrowJSError(env, "GetEtsFunction: class descriptor is not a string");
    }

    if (GetValueType(env, js_function_name) != napi_string) {
        InteropCtx::ThrowJSError(env, "GetEtsFunction: function name is not a string");
        return nullptr;
    }

    std::string class_descriptor = GetString(env, js_class_descriptor);
    std::string method_name = GetString(env, js_function_name);

    return ets_proxy::GetETSFunction(env, class_descriptor, method_name);
}

static napi_value GetEtsClass(napi_env env, napi_callback_info info)
{
    size_t js_argc = 0;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &js_argc, nullptr, nullptr, nullptr));

    if (js_argc != 1) {
        InteropCtx::ThrowJSError(env, "GetEtsClass: bad args, actual args count: " + std::to_string(js_argc));
        return nullptr;
    }

    napi_value js_class_descriptor {};
    ASSERT(js_argc == 1);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, info, &js_argc, &js_class_descriptor, nullptr, nullptr));

    std::string class_descriptor = GetString(env, js_class_descriptor);
    return ets_proxy::GetETSClass(env, class_descriptor);
}

static napi_value Call(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    [[maybe_unused]] napi_status status;
    status = napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    assert(status == napi_ok);

    auto coro = EtsCoroutine::GetCurrent();
    auto argv = InteropCtx::Current(coro)->GetTempArgs<napi_value>(argc);
    napi_value this_arg {};
    void *data = nullptr;
    status = napi_get_cb_info(env, info, &argc, argv->data(), &this_arg, &data);
    assert(status == napi_ok);

    return CallEtsFunctionImpl(env, *argv);
}

static napi_value CallWithCopy(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    [[maybe_unused]] napi_status status;
    status = napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    assert(status == napi_ok);

    auto coro = EtsCoroutine::GetCurrent();
    auto argv = InteropCtx::Current(coro)->GetTempArgs<napi_value>(argc);
    napi_value this_arg {};
    void *data = nullptr;
    status = napi_get_cb_info(env, info, &argc, argv->data(), &this_arg, &data);
    assert(status == napi_ok);

    return InvokeEtsMethodImpl(env, argv->data(), argc, false);
}

static napi_value CreateEtsRuntime(napi_env env, napi_callback_info info)
{
    [[maybe_unused]] napi_status status;
    napi_value napi_false;
    status = napi_get_boolean(env, false, &napi_false);
    assert(status == napi_ok);

    size_t argc = 0;
    status = napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    assert(status == napi_ok);

    std::vector<napi_value> argv(argc);
    napi_value this_arg {};
    void *data = nullptr;
    status = napi_get_cb_info(env, info, &argc, argv.data(), &this_arg, &data);
    assert(status == napi_ok);

    if (argc != 4) {
        LogError("CreateEtsRuntime: exactly 4 arguments are required");
        return napi_false;
    }

    napi_valuetype type;
    napi_typeof(env, argv[0], &type);
    if (type != napi_string) {
        LogError("CreateEtsRuntime: first argument is not a string");
        return napi_false;
    }
    auto stdlib_path = GetString(env, argv[0]);

    napi_typeof(env, argv[1], &type);
    if (type != napi_string) {
        LogError("CreateEtsRuntime: second argument is not a string");
        return napi_false;
    }
    auto index_path = GetString(env, argv[1]);

    napi_typeof(env, argv[2], &type);
    if (type != napi_boolean) {
        LogError("CreateEtsRuntime: third argument is not a boolean");
        return napi_false;
    }
    bool use_jit;
    napi_get_value_bool(env, argv[2], &use_jit);

    napi_typeof(env, argv[3], &type);
    if (type != napi_boolean) {
        LogError("CreateEtsRuntime: fourth argument is not a boolean");
        return napi_false;
    }
    bool use_aot;
    napi_get_value_bool(env, argv[3], &use_aot);

    bool res = panda::ets::CreateRuntime(stdlib_path, index_path, use_jit, use_aot);
    if (res) {
        auto coro = EtsCoroutine::GetCurrent();
        ScopedManagedCodeThread scoped(coro);
        InteropCtx::Init(coro, env);
    }
    napi_value napi_res;
    status = napi_get_boolean(env, res, &napi_res);
    assert(status == napi_ok);
    return napi_res;
}

static napi_value CreateRuntime(napi_env env, napi_callback_info info)
{
    [[maybe_unused]] napi_status status;

    size_t constexpr ARGC = 1;
    std::array<napi_value, ARGC> argv {};

    size_t argc = ARGC;
    NAPI_ASSERT_OK(napi_get_cb_info(env, info, &argc, argv.data(), nullptr, nullptr));

    napi_value napi_false;
    NAPI_ASSERT_OK(napi_get_boolean(env, false, &napi_false));

    if (argc != ARGC) {
        LogError("CreateEtsRuntime: bad args number");
        return napi_false;
    }

    napi_value options = argv[0];
    if (GetValueType(env, options) != napi_object) {
        LogError("CreateEtsRuntime: argument is not an object");
        return napi_false;
    }

    uint32_t num_options;
    std::vector<std::string> arg_strings;

    if (napi_get_array_length(env, options, &num_options) == napi_ok) {
        // options passed as an array
        arg_strings.reserve(num_options + 1);
        arg_strings.emplace_back("argv[0] placeholder");

        for (uint32_t i = 0; i < num_options; ++i) {
            napi_value option;
            NAPI_ASSERT_OK(napi_get_element(env, options, i, &option));
            if (napi_coerce_to_string(env, option, &option) != napi_ok) {
                LogError("Option values must be coercible to string");
                return napi_false;
            }
            arg_strings.push_back(GetString(env, option));
        }
    } else {
        // options passed as a map
        napi_value prop_names;
        NAPI_ASSERT_OK(napi_get_property_names(env, options, &prop_names));
        NAPI_ASSERT_OK(napi_get_array_length(env, prop_names, &num_options));

        arg_strings.reserve(num_options + 1);
        arg_strings.emplace_back("argv[0] placeholder");

        for (uint32_t i = 0; i < num_options; ++i) {
            napi_value key;
            napi_value value;
            NAPI_ASSERT_OK(napi_get_element(env, prop_names, i, &key));
            NAPI_ASSERT_OK(napi_get_property(env, options, key, &value));
            if (napi_coerce_to_string(env, value, &value) != napi_ok) {
                LogError("Option values must be coercible to string");
                return napi_false;
            }
            arg_strings.push_back("--" + GetString(env, key) + "=" + GetString(env, value));
        }
    }

    auto add_opts = [&](base_options::Options *base_options, panda::RuntimeOptions *runtime_options) {
        panda::PandArgParser pa_parser;
        base_options->AddOptions(&pa_parser);
        runtime_options->AddOptions(&pa_parser);
        panda::compiler::OPTIONS.AddOptions(&pa_parser);

        std::vector<const char *> fake_argv;
        fake_argv.reserve(arg_strings.size());
        for (auto const &arg : arg_strings) {
            fake_argv.push_back(arg.c_str());  // Be careful, do not reallocate referenced strings
        }

        if (!pa_parser.Parse(fake_argv.size(), fake_argv.data())) {
            LogError("Parse options failed. Optional arguments:\n" + pa_parser.GetHelpString());
            return false;
        }

        auto runtime_options_err = runtime_options->Validate();
        if (runtime_options_err) {
            LogError("Parse options failed: " + runtime_options_err.value().GetMessage());
            return false;
        }
        panda::compiler::CompilerLogger::SetComponents(panda::compiler::OPTIONS.GetCompilerLog());
        return true;
    };

    bool res = ets::CreateRuntime(add_opts);
    if (res) {
        auto coro = EtsCoroutine::GetCurrent();
        ScopedManagedCodeThread scoped(coro);
        InteropCtx::Init(coro, env);
    }
    napi_value napi_res;
    NAPI_ASSERT_OK(napi_get_boolean(env, res, &napi_res));
    return napi_res;
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
    return exports;
}

}  // namespace panda::ets::interop::js

NAPI_MODULE(ETS_INTEROP_JS_NAPI, panda::ets::interop::js::Init)
