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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FINALREG_FINALIZATION_REGISTRY_H
#define PANDA_PLUGINS_ETS_RUNTIME_FINALREG_FINALIZATION_REGISTRY_H

#include "runtime/mem/gc/gc.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

class PandaEtsVM;

/// @brief The final class that manages all created FinalizationRegistry objects
class FinalizationRegistryManager final {
public:
    explicit FinalizationRegistryManager(PandaEtsVM *vm) : vm_(vm) {}
    NO_COPY_SEMANTIC(FinalizationRegistryManager);
    NO_MOVE_SEMANTIC(FinalizationRegistryManager);
    ~FinalizationRegistryManager() = default;

    /**
     * @brief Add a new created FinalizationRegistry instanse to array of FinalizationRegistry
     * @param instance Object that represents WeakRef<FinalizationRegistry>
     */
    void RegisterInstance(EtsObject *instance);

    /**
     * @brief Checks the number of called coroutines with cleanup and calls the next one
     * if the number does not exceed MAX_FINREG_CLEANUP_COROS.
     * @param coro Pointer to current coroutine
     */
    void StartCleanupCoroIfNeeded(EtsCoroutine *coro);

    /// @brief Decreases the counter of called cleanup coroutines
    void CleanupCoroFinished();

private:
    void SortInstancies();

    void EnsureCapacity(EtsCoroutine *coro);

    /// @brief Increase number of cleanup coroutines and check if not exceeds limit
    bool UpdateFinRegCoroCountAndCheckIfCleanupNeeded();

    // Limit of cleanup coroutines count
    static constexpr uint32_t MAX_FINREG_CLEANUP_COROS = 3;
    // Initial size of the created array
    static constexpr uint32_t INIT_SIZE = 10;

    PandaEtsVM *vm_ {nullptr};
    size_t finRegLastIndex_ {0};
    std::atomic<uint32_t> finRegCleanupCoroCount_ {0};
    mem::Reference *finRegInstancies_ {nullptr};
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FINALREG_FINALIZATION_REGISTRY_H