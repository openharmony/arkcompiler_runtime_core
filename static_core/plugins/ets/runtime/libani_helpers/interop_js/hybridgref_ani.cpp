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

#include "hybridgref_ani.h"

#include "arkts_interop_js_api.h"
#include "hybridgref_registry.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::hygref {

bool HybridGrefGetESValue(ani_env *env, hybridgref ref, ani_object *result)
{
    EtsExecutionContext *executionCtx {};
    interop::js::InteropCtx *ctx {};
    if (ref == nullptr || result == nullptr || !ValidateAniEnv(env, &executionCtx, &ctx)) {
        return false;
    }

    HybridRefValue value;
    if (!GetHybridRef(ref, &value)) {
        return false;
    }

    if (auto *aniRef = std::get_if<AniHybridRef>(&value); aniRef != nullptr) {
        if (!IsOwnedByEnv(*aniRef, env)) {
            return false;
        }
        ani_ref localRef {};
        if (!CreateLocalRefFromAniRef(env, aniRef->ref, &localRef)) {
            return false;
        }
        *result = static_cast<ani_object>(localRef);
        return true;
    }

    napi_value napiValue {};
    napi_env napiEnv {};
    if (!arkts_napi_scope_open(env, &napiEnv)) {
        return false;
    }
    auto napiRef = std::get<NapiHybridRef>(value);
    if (!IsOwnedByEnv(napiRef, napiEnv)) {
        [[maybe_unused]] auto closed = arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return false;
    }
    if (napi_get_reference_value(napiEnv, napiRef.ref, &napiValue) != napi_ok) {
        [[maybe_unused]] auto closed = arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return false;
    }

    ani_ref aniValue {};
    if (!arkts_napi_scope_close_n(napiEnv, 1, &napiValue, &aniValue)) {
        return false;
    }

    ani_ref localRef {};
    if (!CreateLocalRefFromAniRef(env, aniValue, &localRef)) {
        return false;
    }
    *result = static_cast<ani_object>(localRef);
    return true;
}

bool HybridGrefCreateRefFromAni(ani_env *env, ani_ref value, hybridgref *result)
{
    EtsExecutionContext *executionCtx {};
    interop::js::InteropCtx *ctx {};
    if (value == nullptr || result == nullptr || !ValidateAniEnv(env, &executionCtx, &ctx)) {
        return false;
    }

    ani_ref aniGlobalRef {};
    if (env->GlobalReference_Create(value, &aniGlobalRef) != ANI_OK) {
        return false;
    }

    if (!RegisterHybridRefCleanupHook(env, ctx->GetJSEnv())) {
        [[maybe_unused]] auto status = env->GlobalReference_Delete(aniGlobalRef);
        return false;
    }

    if (InsertHybridRef(AniHybridRef {env, aniGlobalRef}, result)) {
        return true;
    }

    [[maybe_unused]] auto status = env->GlobalReference_Delete(aniGlobalRef);
    return false;
}

bool HybridGrefDeleteRefFromAni(ani_env *env, hybridgref ref)
{
    EtsExecutionContext *executionCtx {};
    interop::js::InteropCtx *ctx {};
    if (ref == nullptr || !ValidateAniEnv(env, &executionCtx, &ctx)) {
        return false;
    }

    HybridRefValue value;
    if (!DeleteHybridRef(ref, &value)) {
        return false;
    }

    if (auto *aniRef = std::get_if<AniHybridRef>(&value); aniRef != nullptr) {
        if (!IsOwnedByEnv(*aniRef, env)) {
            [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
            return false;
        }
        if (env->GlobalReference_Delete(aniRef->ref) == ANI_OK) {
            return true;
        }
        [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
        return false;
    }

    napi_env napiEnv {};
    if (!arkts_napi_scope_open(env, &napiEnv)) {
        [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
        return false;
    }
    auto napiRef = std::get<NapiHybridRef>(value);
    if (!IsOwnedByEnv(napiRef, napiEnv)) {
        [[maybe_unused]] auto closed = arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
        return false;
    }
    auto deleteStatus = napi_delete_reference(napiEnv, napiRef.ref);
    auto closeStatus = arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
    if (deleteStatus == napi_ok) {
        return closeStatus;
    }
    [[maybe_unused]] auto restored = RestoreHybridRef(ref, value);
    return false;
}

}  // namespace ark::ets::hygref

// NOLINTBEGIN(readability-identifier-naming)
extern "C" bool hybridgref_get_esvalue(ani_env *env, hybridgref ref, ani_object *result)
{
    return ark::ets::hygref::HybridGrefGetESValue(env, ref, result);
}

extern "C" bool hybridgref_create_from_ani(ani_env *env, ani_ref value, hybridgref *result)
{
    return ark::ets::hygref::HybridGrefCreateRefFromAni(env, value, result);
}

extern "C" bool hybridgref_delete_from_ani(ani_env *env, hybridgref ref)
{
    return ark::ets::hygref::HybridGrefDeleteRefFromAni(env, ref);
}
// NOLINTEND(readability-identifier-naming)
