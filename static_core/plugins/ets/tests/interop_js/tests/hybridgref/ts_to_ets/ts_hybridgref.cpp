/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/libani_helpers/interop_js/hybridgref_ani.h"
#include "plugins/ets/runtime/libani_helpers/interop_js/hybridgref_napi.h"
#include <ani.h>

namespace ark::ets::interop::js::testing {
static hybridgref g_jsToEtsRef = nullptr;

static napi_value NativeSaveRef(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];  // NOLINT(modernize-avoid-c-arrays)
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        return undefined;
    }

    if (g_jsToEtsRef != nullptr) {
        hybridgref_delete_from_napi(env, g_jsToEtsRef);
        g_jsToEtsRef = nullptr;
    }

    bool ok = hybridgref_create_from_napi(env, argv[0], &g_jsToEtsRef);
    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}
void NoopFinalize(napi_env env, void *finalizeData, void *finalizeHint)
{
    (void)env;
    (void)finalizeData;
    (void)finalizeHint;
}

static napi_value NativeWrapRef(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];  // NOLINT(modernize-avoid-c-arrays)
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return nullptr;
    }
    napi_valuetype valuetype;
    status = napi_typeof(env, argv[0], &valuetype);
    if (status != napi_ok) {
        return nullptr;
    }

    bool isObject = (valuetype == napi_object);

    if (!isObject) {
        return nullptr;
    }
    constexpr uintptr_t DUMMY_NATIVE_POINTER = 0x12345678;
    void *nativePtr = reinterpret_cast<void *>(DUMMY_NATIVE_POINTER);
    status = napi_wrap(env, argv[0], nativePtr, NoopFinalize, nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static ani_object NativeGetRef(ani_env *env)
{
    ani_object result {};
    hybridgref_get_esvalue(env, g_jsToEtsRef, &result);
    return result;
}

class NativeGrefTsToEtsTest : public EtsInteropTest {
public:
    static bool GetAniEnv(ani_env **env)
    {
        ani_vm *aniVm;
        ani_size res;

        auto status = ANI_GetCreatedVMs(&aniVm, 1U, &res);
        if (status != ANI_OK || res == 0) {
            return false;
        }

        status = aniVm->GetEnv(ANI_VERSION_1, env);
        return status == ANI_OK && *env != nullptr;
    }

    static bool RegisterNativeSaveRef(napi_env env)
    {
        napi_value global;
        if (napi_get_global(env, &global) != napi_ok) {
            return false;
        }

        napi_value fn;
        if (napi_create_function(env, "nativeSaveRef", NAPI_AUTO_LENGTH, NativeSaveRef, nullptr, &fn) != napi_ok) {
            return false;
        }

        return napi_set_named_property(env, global, "nativeSaveRef", fn) == napi_ok;
    }

    static bool RegisterNativeWrapRef(napi_env env)
    {
        napi_value global;
        if (napi_get_global(env, &global) != napi_ok) {
            return false;
        }

        napi_value fn;
        if (napi_create_function(env, "nativeWrapRef", NAPI_AUTO_LENGTH, NativeWrapRef, nullptr, &fn) != napi_ok) {
            return false;
        }

        return napi_set_named_property(env, global, "nativeWrapRef", fn) == napi_ok;
    }

    static bool RegisterETSGetter(ani_env *env)
    {
        ani_module mod {};
        if (env->FindModule("Lets_functions;", &mod) != ANI_OK) {
            return false;
        }

        std::array methods = {
            ani_native_function {"nativeGetRef", nullptr, reinterpret_cast<void *>(NativeGetRef)},
        };
        return env->Module_BindNativeFunctions(mod, methods.data(), methods.size()) == ANI_OK;
    }
};

TEST_F(NativeGrefTsToEtsTest, check_js_to_ets_hybridgref)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    napi_env napiEnv = GetJsEnv();

    ASSERT_TRUE(RegisterNativeSaveRef(napiEnv));
    ASSERT_TRUE(RegisterNativeWrapRef(napiEnv));
    ASSERT_TRUE(RegisterETSGetter(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("ts_hybridgref.ts"));
}
}  // namespace ark::ets::interop::js::testing