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

#include "arkts_interop_js_api.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/interop_js/native_api/arkts_interop_js_api_impl.h"

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" bool arkts_napi_scope_open(ani_env *env, napi_env *result)
{
    if (UNLIKELY(result == nullptr)) {
        return false;
    }
    auto *coro = ark::ets::EtsCoroutine::GetCurrent();
    if (UNLIKELY(env == nullptr || ark::ets::PandaEtsNapiEnv::FromAniEnv(env)->GetEtsCoroutine() != coro)) {
        return false;
    }
    napi_env localJsEnv {};
    if (UNLIKELY(!ark::ets::interop::js::GetCurrentNapiEnv(env, &localJsEnv))) {
        return false;
    }
    if (UNLIKELY(!ark::ets::interop::js::OpenETSToJSScope(coro, nullptr))) {
        return false;
    }
    *result = localJsEnv;
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" bool arkts_napi_scope_close_n(napi_env env, size_t nValues, napi_value *values, ani_ref *result)
{
    return ark::ets::interop::js::CloseETSToJSScope(env, nValues, values, result);
}
