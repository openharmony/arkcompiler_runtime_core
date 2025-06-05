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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_UNHANDLED_OBJECT_MANAGER_H
#define PANDA_PLUGINS_ETS_RUNTIME_UNHANDLED_OBJECT_MANAGER_H

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

class PandaEtsVM;
class EtsJob;
class EtsPromise;

/// @brief The class manages all unhandled rejected objects
class UnhandledObjectManager final {
public:
    explicit UnhandledObjectManager(PandaEtsVM *vm) : vm_(vm) {}
    NO_COPY_SEMANTIC(UnhandledObjectManager);
    NO_MOVE_SEMANTIC(UnhandledObjectManager);
    ~UnhandledObjectManager() = default;

    /// @brief Adds failed job to an internal storage
    void AddFailedJob(EtsJob *job);

    /// @brief Removes failed job from internal storage
    void RemoveFailedJob(EtsJob *job);

    /// @brief Invokes managed method to apply custom handler on stored failed jobs
    void ListFailedJobs(EtsCoroutine *coro);

    /// @brief Adds rejected promise to an internal storage
    void AddRejectedPromise(EtsPromise *promise);

    /// @brief Removes rejected promise from internal storage
    void RemoveRejectedPromise(EtsPromise *promise);

    /// @brief Invokes managed method to apply custom handler on stored rejected promises
    void ListRejectedPromises(EtsCoroutine *coro);

    /// @brief Updates references to objects moved by GC
    void UpdateObjects(const GCRootUpdater &gcRootUpdater);

    /// @brief Visits objects as GC roots
    void VisitObjects(const GCRootVisitor &visitor);

private:
    PandaEtsVM *vm_;
    os::memory::Mutex mutex_;
    PandaUnorderedSet<EtsObject *> failedJobs_ GUARDED_BY(mutex_);
    PandaUnorderedSet<EtsObject *> rejectedPromises_ GUARDED_BY(mutex_);
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_UNHANDLED_OBJECT_MANAGER_H
