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

#ifndef PANDA_RUNTIME_INCLUDE_MUTATOR_MANAGER_H
#define PANDA_RUNTIME_INCLUDE_MUTATOR_MANAGER_H

#include <functional>

#include "libarkbase/clang.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mutator.h"

namespace ark {

/// @class MutatorManager manages mutators for GC work (STW, barriers set, etc.)
class MutatorManager final {
public:
    using MutatorCallback = std::function<bool(Mutator *)>;

    MutatorManager() = default;
    NO_COPY_SEMANTIC(MutatorManager);
    NO_MOVE_SEMANTIC(MutatorManager);
    ~MutatorManager();

    /**
     * @brief Register new mutator and suspend it if suspention process is in progress
     * @note For managed mutators should be called under related managed mutator list lock
     * @param mutator - a new mutator for registration in the MutatorManager
     * @param callback - a callback will be called after mutator registration under the MutatorManager lock
     */
    void RegisterMutator(Mutator *mutator, const MutatorCallback &callback);

    /**
     * @brief Unregister mutator from MutatorManager
     * @note For managed mutators should be called under related managed mutator list lock
     * @param mutator - a registered mutator in the MutatorManager
     */
    void UnregisterMutator(Mutator *mutator);

    /**
     * @param callback applies for each registered mutator
     * @returns true if all callback returns true, false - otherwise
     */
    bool ForEachMutator(const MutatorCallback &callback);

    /**
     * @brief Suspend each registered mutator
     * @see Mutator::SuspendImpl
     */
    void SuspendAllMutators();

    /**
     * @brief Resume each registered mutator
     * @see Mutator::ResumeImpl
     */
    void ResumeAllMutators();

    /**
     * @brief Notify the MutatorManager about daemon mutators suspention start
     * @note Should be called under related managed mutator list lock
     */
    void NotifyToSuspendDaemonMutators();

private:
    bool ForEachMutatorImpl(const MutatorCallback &callback) REQUIRES(mutatorsLock_);

    os::memory::Mutex mutatorsLock_;
    PandaUnorderedSet<Mutator *> mutatorsSet_ GUARDED_BY(mutatorsLock_);
    // CC-OFFNXT(G.FMT.03): false positive, GUARDED_BY is thread safety macro
    size_t suspendForNewMutatorsCounter_ GUARDED_BY(mutatorsLock_) {0U};
};  // class MutatorManager

}  // namespace ark

#endif  // PANDA_RUNTIME_INCLUDE_MUTATOR_MANAGER_H
