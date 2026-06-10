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

#include "hybridgref_napi.h"

#include "arkts_interop_js_api.h"
#include "hybridgref_registry.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::hygref {

bool HybridGrefCreateRefFromNapi(napi_env env, napi_value value, hybridgref *result)
{
    EtsExecutionContext *executionCtx {};
    interop::js::InteropCtx *ctx {};
    if (value == nullptr || result == nullptr || !ValidateNapiEnv(env, &executionCtx, &ctx)) {
        return false;
    }

    napi_ref napiRef {};
    if (napi_create_reference(env, value, 1, &napiRef) != napi_ok) {
        return false;
    }

    if (!RegisterHybridRefCleanupHook(executionCtx->GetPandaAniEnv(), env)) {
        [[maybe_unused]] auto status = napi_delete_reference(env, napiRef);
        return false;
    }

    if (InsertHybridRef(NapiHybridRef {env, napiRef}, result)) {
        return true;
    }

    [[maybe_unused]] auto status = napi_delete_reference(env, napiRef);
    return false;
}

bool HybridGrefDeleteRefFromNapi(napi_env env, hybridgref ref)
{
    EtsExecutionContext *executionCtx {};
    interop::js::InteropCtx *ctx {};
    if (ref == nullptr || !ValidateNapiEnv(env, &executionCtx, &ctx)) {
        return false;
    }

    HybridRefValue value;
    if (!DeleteHybridRef(ref, &value)) {
        return false;
    }

    if (auto *napiRef = std::get_if<NapiHybridRef>(&value); napiRef != nullptr) {
        if (!IsOwnedByEnv(*napiRef, env)) {
            [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
            return false;
        }
        if (napi_delete_reference(env, napiRef->ref) == napi_ok) {
            return true;
        }
        [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
        return false;
    }

    auto aniRef = std::get<AniHybridRef>(value);
    ani_env *aniEnv {};
    if (!arkts_ani_scope_open(&env, &aniEnv)) {
        [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
        return false;
    }
    if (!IsOwnedByEnv(aniRef, aniEnv)) {
        [[maybe_unused]] auto closed = arkts_ani_scope_close_n(aniEnv, 0, nullptr, nullptr);
        [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
        return false;
    }
    auto deleteStatus = aniEnv->GlobalReference_Delete(aniRef.ref);
    auto closeStatus = arkts_ani_scope_close_n(aniEnv, 0, nullptr, nullptr);
    if (deleteStatus == ANI_OK) {
        return closeStatus;
    }
    [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
    return false;
}

bool HybridGrefGetNapiValue(napi_env env, hybridgref ref, napi_value *result)
{
    EtsExecutionContext *executionCtx {};
    interop::js::InteropCtx *ctx {};
    if (ref == nullptr || result == nullptr || !ValidateNapiEnv(env, &executionCtx, &ctx)) {
        return false;
    }

    HybridRefValue value;
    if (!GetHybridRef(ref, &value)) {
        return false;
    }

    if (auto *napiRef = std::get_if<NapiHybridRef>(&value); napiRef != nullptr) {
        if (!IsOwnedByEnv(*napiRef, env)) {
            return false;
        }
        return napi_get_reference_value(env, napiRef->ref, result) == napi_ok;
    }

    auto aniRef = std::get<AniHybridRef>(value);
    ani_env *aniEnv {};
    if (!arkts_ani_scope_open(&env, &aniEnv)) {
        return false;
    }
    if (!IsOwnedByEnv(aniRef, aniEnv)) {
        [[maybe_unused]] auto closed = arkts_ani_scope_close_n(aniEnv, 0, nullptr, nullptr);
        return false;
    }

    ani_ref aniValue {};
    if (!UnwrapLocalRefFromAniRef(aniEnv, aniRef.ref, &aniValue)) {
        [[maybe_unused]] auto closed = arkts_ani_scope_close_n(aniEnv, 0, nullptr, nullptr);
        return false;
    }
    return arkts_ani_scope_close_n(aniEnv, 1, &aniValue, result);
}

}  // namespace ark::ets::hygref

// NOLINTBEGIN(readability-identifier-naming)
extern "C" bool hybridgref_create_from_napi(napi_env env, napi_value value, hybridgref *result)
{
    return ark::ets::hygref::HybridGrefCreateRefFromNapi(env, value, result);
}

extern "C" bool hybridgref_delete_from_napi(napi_env env, hybridgref ref)
{
    return ark::ets::hygref::HybridGrefDeleteRefFromNapi(env, ref);
}

extern "C" bool hybridgref_get_napi_value(napi_env env, hybridgref ref, napi_value *result)
{
    return ark::ets::hygref::HybridGrefGetNapiValue(env, ref, result);
}
// NOLINTEND(readability-identifier-naming)
