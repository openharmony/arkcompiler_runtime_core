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
static hybridgref g_ref = nullptr;
static napi_value NativeGetRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result {};
    if (!hybridgref_get_napi_value(env, g_ref, &result)) {
        napi_get_undefined(env, &result);
    }
    return result;
}

static void NativeSaveHybridGref(ani_env *env, ani_object ref)
{
    ASSERT_TRUE(hybridgref_create_from_ani(env, static_cast<ani_ref>(ref), &g_ref));
}

class HybridGrefPrimitiveEtsToTsTest : public EtsInteropTest {
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

    static bool RegisterNativeGetRef(napi_env env)
    {
        napi_value global;
        if (napi_get_global(env, &global) != napi_ok) {
            return false;
        }
        napi_value fn;
        if (napi_create_function(env, "nativeGetRef", NAPI_AUTO_LENGTH, NativeGetRef, nullptr, &fn) != napi_ok) {
            return false;
        }

        if (napi_set_named_property(env, global, "nativeGetRef", fn) != napi_ok) {
            return false;
        }

        return true;
    }

    static bool RegisterNativeSaveRef(ani_env *env)
    {
        ani_module etsMod {};
        auto status = env->FindModule("Lets_functions;", &etsMod);
        if (status != ANI_OK) {
            return false;
        }
        std::array methods = {
            ani_native_function {"saveHybridGref", nullptr, reinterpret_cast<void *>(NativeSaveHybridGref)},
        };
        status = env->Module_BindNativeFunctions(etsMod, methods.data(), methods.size());
        return status == ANI_OK;
    }
};

TEST_F(HybridGrefPrimitiveEtsToTsTest, check_ets_hybridgref)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    napi_env napiEnv = GetJsEnv();

    ASSERT_TRUE(RegisterNativeGetRef(napiEnv));
    ASSERT_TRUE(RegisterNativeSaveRef(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("ets_hybridgref.ts"));
}

}  // namespace ark::ets::interop::js::testing