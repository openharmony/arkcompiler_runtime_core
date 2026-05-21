/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_HYBRIDGREF_REGISTRY_H
#define PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_HYBRIDGREF_REGISTRY_H

#include "hybridgref.h"

#include <ani.h>
#include <node_api.h>

#include <cstdint>
#include <variant>

namespace ark::ets {
class EtsExecutionContext;
}  // namespace ark::ets

namespace ark::ets::interop::js {
class InteropCtx;
}  // namespace ark::ets::interop::js

namespace ark::ets::hygref {

using HybridRefId = uint64_t;

struct AniHybridRef {
    ani_env *env {};
    ani_ref ref {};
};

struct NapiHybridRef {
    napi_env env {};
    napi_ref ref {};
};

using HybridRefValue = std::variant<AniHybridRef, NapiHybridRef>;

inline bool IsOwnedByEnv(const AniHybridRef &ref, ani_env *env)
{
    return ref.env == env;
}

inline bool IsOwnedByEnv(const NapiHybridRef &ref, napi_env env)
{
    return ref.env == env;
}

bool InsertHybridRef(HybridRefValue value, hybridgref *result);
bool GetHybridRef(hybridgref ref, HybridRefValue *result);
bool DeleteHybridRef(hybridgref ref, HybridRefValue *result);
bool RestoreHybridRef(hybridgref ref, HybridRefValue value);
bool RegisterHybridRefCleanupHook(ani_env *aniEnv, napi_env napiEnv);
bool ShutdownHybridRefRegistry(ani_env *aniEnv, napi_env napiEnv);

bool ValidateAniEnv(ani_env *env, EtsExecutionContext **executionCtx, interop::js::InteropCtx **ctx);
bool ValidateNapiEnv(napi_env env, EtsExecutionContext **executionCtx, interop::js::InteropCtx **ctx);

bool CreateLocalRefFromAniRef(ani_env *env, ani_ref value, ani_ref *result);
bool UnwrapLocalRefFromAniRef(ani_env *env, ani_ref value, ani_ref *result);

}  // namespace ark::ets::hygref

#endif  // PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_HYBRIDGREF_REGISTRY_H
