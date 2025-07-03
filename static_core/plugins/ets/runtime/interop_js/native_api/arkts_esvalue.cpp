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

#include "arkts_esvalue.h"

#include <node_api.h>

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"

namespace ark::ets::interop::js {

static ani_class g_esvalueClass {};
static ani_method g_isEcmaObjectMethod {};
static ani_field g_evField {};

static bool CacheEsValueClass(ani_env *env)
{
    ani_class esvalueClass {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    [[maybe_unused]] auto status = env->FindClass("std.interop.ESValue", &esvalueClass);
    ASSERT(status == ANI_OK);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (UNLIKELY(env->GlobalReference_Create(esvalueClass, reinterpret_cast<ani_ref *>(&g_esvalueClass)) != ANI_OK)) {
        return false;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Class_FindMethod(esvalueClass, "isECMAObject", ":z", &g_isEcmaObjectMethod);
    ASSERT(status == ANI_OK);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Class_FindField(esvalueClass, "ev", &g_evField);
    ASSERT(status == ANI_OK);
    return true;
}

static bool InitializeGlobalOnce(ani_env *env)
{
    // Guaranteed to be called once
    static bool isInitialized = CacheEsValueClass(env);
    return isInitialized;
}

PANDA_PUBLIC_API bool UnwrapESValue(ani_env *env, ani_object esvalue, void **result)
{
    if (UNLIKELY(!InitializeGlobalOnce(env))) {
        return false;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (UNLIKELY(PandaEtsNapiEnv::FromAniEnv(env)->GetEtsCoroutine() != coro)) {
        return false;
    }
    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        return false;
    }

    ani_boolean isEsValue = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    [[maybe_unused]] auto status = env->Object_InstanceOf(esvalue, g_esvalueClass, &isEsValue);
    if (status != ANI_OK || isEsValue == ANI_FALSE) {
        return false;
    }
    ani_boolean isEcmaObject = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_CallMethod_Boolean(esvalue, g_isEcmaObjectMethod, &isEcmaObject);
    if (UNLIKELY(status != ANI_OK || isEcmaObject == ANI_FALSE)) {
        return false;
    }
    ani_ref jsValue {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    status = env->Object_GetField_Ref(esvalue, g_evField, &jsValue);
    ASSERT(status == ANI_OK);

    ani::ScopedManagedCodeFix s(env);
    auto *jsValueObject = JSValue::FromEtsObject(s.ToInternalType(jsValue));

    INTEROP_CODE_SCOPE_ETS(coro);
    auto jsenv = ctx->GetJSEnv();
    NapiScope jsHandleScope(jsenv);

    auto jsObj = JSConvertJSValue::WrapWithNullCheck(jsenv, jsValueObject);
    void *res = nullptr;
    if (napi_unwrap(jsenv, jsObj, &res) != napi_ok) {
        return false;
    }
    *result = res;
    return true;
}
}  // namespace ark::ets::interop::js