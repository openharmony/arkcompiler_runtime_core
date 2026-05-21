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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
#include "hybridgref_registry.h"

#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

#include <atomic>
#include <unordered_map>

namespace ark::ets::hygref {
namespace {

struct HybridRefCleanupHook {
    ani_env *aniEnv {};
    napi_env napiEnv {};
};

struct HybridRefRegistry {
    std::atomic<HybridRefId> nextId {1};
    os::memory::Mutex mutex;
    std::unordered_map<HybridRefId, HybridRefValue> registry;
    std::unordered_map<napi_env, HybridRefCleanupHook *> cleanupHooks;
    ani_env *esValueEnv {};
    ani_ref esValueClass {};
    ani_method esValueCtor {};
    ani_field esValueField {};
    bool isESValueInitialized {false};
};

std::atomic<HybridRefRegistry *> g_registryState {nullptr};

HybridRefRegistry &GetRegistry()
{
    // Atomic with acquire order reason: pairs with release publish and guarantees registry init visibility
    auto *registry = g_registryState.load(std::memory_order_acquire);
    if (registry != nullptr) {
        return *registry;
    }

    auto *newRegistry = new HybridRefRegistry();
    HybridRefRegistry *expected = nullptr;
    // Atomic with release order reason: publishes initialized registry to other threads
    // Atomic with acquire order reason: observes initialized registry from concurrent publisher
    if (g_registryState.compare_exchange_strong(expected, newRegistry, std::memory_order_release,
                                                std::memory_order_acquire)) {
        return *newRegistry;
    }
    delete newRegistry;
    return *expected;
}

bool ToHybridRef(HybridRefId id, hybridgref *result)
{
    if (id == 0 || result == nullptr) {
        return false;
    }
    *result = reinterpret_cast<hybridgref>(static_cast<uintptr_t>(id));
    return true;
}

bool FromHybridRef(hybridgref ref, HybridRefId *id)
{
    if (ref == nullptr || id == nullptr) {
        return false;
    }

    auto refId = static_cast<HybridRefId>(reinterpret_cast<uintptr_t>(ref));
    if (refId == 0) {
        return false;
    }
    *id = refId;
    return true;
}

HybridRefId AllocateHybridRefId()
{
    auto &registry = GetRegistry();
    HybridRefId id = 0;
    while (id == 0) {
        // Atomic with relaxed order reason: only unique id generation is required; map access is mutex-protected
        id = registry.nextId.fetch_add(1, std::memory_order_relaxed);
    }
    return id;
}

bool DeleteHybridRefValue(const HybridRefValue &value)
{
    if (auto *aniRef = std::get_if<AniHybridRef>(&value); aniRef != nullptr) {
        return aniRef->env != nullptr && aniRef->ref != nullptr &&
               aniRef->env->GlobalReference_Delete(aniRef->ref) == ANI_OK;
    }

    auto napiRef = std::get<NapiHybridRef>(value);
    return napiRef.env != nullptr && napiRef.ref != nullptr &&
           napi_delete_reference(napiRef.env, napiRef.ref) == napi_ok;
}

bool BelongsToEnv(const HybridRefValue &value, ani_env *aniEnv, napi_env napiEnv)
{
    if (auto *aniRef = std::get_if<AniHybridRef>(&value); aniRef != nullptr) {
        return aniEnv != nullptr && aniRef->env == aniEnv;
    }
    auto napiRef = std::get<NapiHybridRef>(value);
    return napiEnv != nullptr && napiRef.env == napiEnv;
}

bool CleanupESValueCache(HybridRefRegistry &registry, ani_env *aniEnv)
{
    if (!registry.isESValueInitialized || registry.esValueEnv != aniEnv) {
        return true;
    }

    auto status = ANI_OK;
    if (registry.esValueClass != nullptr) {
        status = aniEnv->GlobalReference_Delete(registry.esValueClass);
    }
    registry.esValueEnv = nullptr;
    registry.esValueClass = nullptr;
    registry.esValueCtor = nullptr;
    registry.esValueField = nullptr;
    registry.isESValueInitialized = false;
    return status == ANI_OK;
}

void HybridRefCleanupHookCallback(void *arg)
{
    auto *hook = static_cast<HybridRefCleanupHook *>(arg);
    if (hook == nullptr) {
        return;
    }
    [[maybe_unused]] auto cleaned = ShutdownHybridRefRegistry(hook->aniEnv, hook->napiEnv);
    delete hook;
}

bool InitializeESValue(ani_env *env)
{
    if (env == nullptr) {
        return false;
    }

    auto &registry = GetRegistry();
    os::memory::LockHolder lock(registry.mutex);
    if (registry.isESValueInitialized) {
        return registry.esValueEnv == env;
    }

    ani_class esValueClass {};
    if (UNLIKELY(env->FindClass("std.interop.ESValue", &esValueClass) != ANI_OK)) {
        return false;
    }
    if (UNLIKELY(env->GlobalReference_Create(esValueClass, &registry.esValueClass) != ANI_OK)) {
        return false;
    }
    if (UNLIKELY(env->Class_FindMethod(esValueClass, "<ctor>", "Y:", &registry.esValueCtor) != ANI_OK)) {
        [[maybe_unused]] auto status = env->GlobalReference_Delete(registry.esValueClass);
        registry.esValueClass = nullptr;
        return false;
    }
    if (UNLIKELY(env->Class_FindField(esValueClass, "ev", &registry.esValueField) != ANI_OK)) {
        [[maybe_unused]] auto status = env->GlobalReference_Delete(registry.esValueClass);
        registry.esValueClass = nullptr;
        registry.esValueCtor = nullptr;
        return false;
    }
    registry.esValueEnv = env;
    registry.isESValueInitialized = true;
    return true;
}

bool IsESValue(ani_env *env, ani_ref value)
{
    if (UNLIKELY(!InitializeESValue(env))) {
        return false;
    }

    ani_class esValueClass {};
    {
        auto &registry = GetRegistry();
        os::memory::LockHolder lock(registry.mutex);
        esValueClass = static_cast<ani_class>(registry.esValueClass);
    }
    ani_boolean isESValue = ANI_FALSE;
    auto status = env->Object_InstanceOf(static_cast<ani_object>(value), esValueClass, &isESValue);
    return status == ANI_OK && isESValue == ANI_TRUE;
}

bool IsNullishValue(ani_env *env, ani_ref value, ani_boolean *result)
{
    if (env == nullptr || value == nullptr || result == nullptr) {
        return false;
    }
    return env->Reference_IsNullishValue(value, result) == ANI_OK;
}

bool CreateESValue(ani_env *env, ani_ref value, ani_ref *result)
{
    if (env == nullptr || value == nullptr || result == nullptr || UNLIKELY(!InitializeESValue(env))) {
        return false;
    }

    ani_class esValueClass {};
    ani_method esValueCtor {};
    {
        auto &registry = GetRegistry();
        os::memory::LockHolder lock(registry.mutex);
        esValueClass = static_cast<ani_class>(registry.esValueClass);
        esValueCtor = registry.esValueCtor;
    }
    ani_object esValue {};
    auto status = env->Object_New(esValueClass, esValueCtor, &esValue, value);
    if (status != ANI_OK) {
        return false;
    }
    *result = static_cast<ani_ref>(esValue);
    return true;
}

bool UnwrapESValue(ani_env *env, ani_ref value, ani_ref *result)
{
    if (env == nullptr || value == nullptr || result == nullptr || UNLIKELY(!InitializeESValue(env))) {
        return false;
    }

    ani_boolean isNullish = ANI_FALSE;
    if (!IsNullishValue(env, value, &isNullish)) {
        return false;
    }
    if (isNullish == ANI_TRUE) {
        *result = value;
        return true;
    }

    if (!IsESValue(env, value)) {
        *result = value;
        return true;
    }

    ani_field esValueField {};
    {
        auto &registry = GetRegistry();
        os::memory::LockHolder lock(registry.mutex);
        esValueField = registry.esValueField;
    }
    auto status = env->Object_GetField_Ref(static_cast<ani_object>(value), esValueField, result);
    return status == ANI_OK;
}

bool CreateLocalRef(ani_env *env, ani_ref value, ani_ref *result)
{
    if (env == nullptr || value == nullptr || result == nullptr) {
        return false;
    }

    ani_boolean isUndefined = ANI_FALSE;
    if (env->Reference_IsUndefined(value, &isUndefined) != ANI_OK) {
        return false;
    }
    if (isUndefined == ANI_TRUE) {
        return ani::ManagedCodeAccessor::GetUndefinedRef(result) == ANI_OK;
    }

    ani_boolean isNullish = ANI_FALSE;
    if (!IsNullishValue(env, value, &isNullish)) {
        return false;
    }
    if (isNullish == ANI_TRUE) {
        return env->GetNull(result) == ANI_OK;
    }

    ani::ScopedManagedCodeFix s(env);
    return s.AddLocalRef(s.ToInternalType(value), result) == ANI_OK;
}

bool FindHybridRef(hybridgref ref, HybridRefValue *result, HybridRefId *idOut)
{
    HybridRefId id = 0;
    if (!FromHybridRef(ref, &id) || result == nullptr) {
        return false;
    }

    auto &registry = GetRegistry();
    auto it = registry.registry.find(id);
    if (it == registry.registry.end()) {
        return false;
    }

    *result = it->second;
    if (idOut != nullptr) {
        *idOut = id;
    }
    return true;
}

}  // namespace

bool InsertHybridRef(HybridRefValue value, hybridgref *result)
{
    if (result == nullptr) {
        return false;
    }

    while (true) {
        auto id = AllocateHybridRefId();
        auto &registry = GetRegistry();
        os::memory::LockHolder lock(registry.mutex);
        auto [it, inserted] = registry.registry.emplace(id, value);
        if (!inserted) {
            continue;
        }
        return ToHybridRef(it->first, result);
    }
}

bool GetHybridRef(hybridgref ref, HybridRefValue *result)
{
    auto &registry = GetRegistry();
    os::memory::LockHolder lock(registry.mutex);
    return FindHybridRef(ref, result, nullptr);
}

bool DeleteHybridRef(hybridgref ref, HybridRefValue *result)
{
    auto &registry = GetRegistry();
    os::memory::LockHolder lock(registry.mutex);
    HybridRefId id = 0;
    if (!FindHybridRef(ref, result, &id)) {
        return false;
    }
    registry.registry.erase(id);
    return true;
}

bool RestoreHybridRef(hybridgref ref, HybridRefValue value)
{
    auto &registry = GetRegistry();
    os::memory::LockHolder lock(registry.mutex);
    HybridRefId id = 0;
    if (!FromHybridRef(ref, &id)) {
        return false;
    }
    return registry.registry.emplace(id, value).second;
}

bool RegisterHybridRefCleanupHook(ani_env *aniEnv, napi_env napiEnv)
{
    if (aniEnv == nullptr || napiEnv == nullptr) {
        return false;
    }

    auto &registry = GetRegistry();
    os::memory::LockHolder lock(registry.mutex);
    if (registry.cleanupHooks.find(napiEnv) != registry.cleanupHooks.end()) {
        return true;
    }

    // N-API cleanup hooks run in reverse registration order, so first-use registration keeps refs alive during use
    // and releases them before the earlier interop runtime teardown hook.
    auto *hook = new HybridRefCleanupHook {aniEnv, napiEnv};
    auto status = napi_add_env_cleanup_hook(napiEnv, HybridRefCleanupHookCallback, hook);
    if (status != napi_ok) {
        delete hook;
        return false;
    }
    registry.cleanupHooks.emplace(napiEnv, hook);
    return true;
}

bool ShutdownHybridRefRegistry(ani_env *aniEnv, napi_env napiEnv)
{
    // Atomic with acquire order reason: pairs with release publish and guarantees registry init visibility
    auto *registry = g_registryState.load(std::memory_order_acquire);
    if (registry == nullptr) {
        return true;
    }

    bool success = true;
    bool registryEmpty = false;
    {
        os::memory::LockHolder lock(registry->mutex);
        for (auto it = registry->registry.begin(); it != registry->registry.end();) {
            if (!BelongsToEnv(it->second, aniEnv, napiEnv)) {
                ++it;
                continue;
            }
            success = DeleteHybridRefValue(it->second) && success;
            it = registry->registry.erase(it);
        }
        success = CleanupESValueCache(*registry, aniEnv) && success;
        registry->cleanupHooks.erase(napiEnv);
        registryEmpty = registry->registry.empty() && !registry->isESValueInitialized;
    }

    if (registryEmpty) {
        auto *expected = registry;
        // Atomic with release order reason: publishes registry shutdown to other threads
        // Atomic with acquire order reason: observes concurrent registry replacement
        if (g_registryState.compare_exchange_strong(expected, nullptr, std::memory_order_release,
                                                    std::memory_order_acquire)) {
            delete registry;
        }
    }
    return success;
}

bool ValidateAniEnv(ani_env *env, EtsExecutionContext **executionCtx, interop::js::InteropCtx **ctx)
{
    if (env == nullptr || executionCtx == nullptr || ctx == nullptr) {
        return false;
    }

    auto *currentExecutionCtx = EtsExecutionContext::GetCurrent();
    if (currentExecutionCtx == nullptr || PandaAniEnv::FromAniEnv(env)->GetExecutionContext() != currentExecutionCtx) {
        return false;
    }

    auto *currentInteropCtx = interop::js::InteropCtx::Current(currentExecutionCtx);
    if (currentInteropCtx == nullptr) {
        return false;
    }

    *executionCtx = currentExecutionCtx;
    *ctx = currentInteropCtx;
    return true;
}

bool ValidateNapiEnv(napi_env env, EtsExecutionContext **executionCtx, interop::js::InteropCtx **ctx)
{
    if (env == nullptr || executionCtx == nullptr || ctx == nullptr) {
        return false;
    }

    auto *currentExecutionCtx = EtsExecutionContext::GetCurrent();
    if (currentExecutionCtx == nullptr) {
        return false;
    }

    auto *currentInteropCtx = interop::js::InteropCtx::Current(currentExecutionCtx);
    if (currentInteropCtx == nullptr || currentInteropCtx->GetJSEnv() != env) {
        return false;
    }

    *executionCtx = currentExecutionCtx;
    *ctx = currentInteropCtx;
    return true;
}

bool CreateLocalRefFromAniRef(ani_env *env, ani_ref value, ani_ref *result)
{
    if (env == nullptr || value == nullptr || result == nullptr || UNLIKELY(!InitializeESValue(env))) {
        return false;
    }

    ani_ref localRef {};
    if (!CreateLocalRef(env, value, &localRef)) {
        return false;
    }
    ani_boolean isNullish = ANI_FALSE;
    if (!IsNullishValue(env, localRef, &isNullish)) {
        return false;
    }
    if (isNullish == ANI_TRUE) {
        return CreateESValue(env, localRef, result);
    }
    if (IsESValue(env, localRef)) {
        *result = localRef;
        return true;
    }
    return CreateESValue(env, localRef, result);
}

bool UnwrapLocalRefFromAniRef(ani_env *env, ani_ref value, ani_ref *result)
{
    if (env == nullptr || value == nullptr || result == nullptr || UNLIKELY(!InitializeESValue(env))) {
        return false;
    }
    ani_ref localRef {};
    if (!CreateLocalRef(env, value, &localRef)) {
        return false;
    }
    return UnwrapESValue(env, localRef, result);
}

}  // namespace ark::ets::hygref

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
