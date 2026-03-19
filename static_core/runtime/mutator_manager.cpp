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

#include "runtime/include/mutator_manager.h"

namespace ark {

MutatorManager::~MutatorManager()
{
    os::memory::LockHolder destructorLock(mutatorsLock_);
    mutatorsSet_.clear();
}

void MutatorManager::RegisterMutator(Mutator *mutator, const MutatorCallback &callback)
{
    os::memory::LockHolder registerMutatorLock(mutatorsLock_);
    ASSERT(mutatorsSet_.find(mutator) == mutatorsSet_.cend());
    LOG(DEBUG, RUNTIME) << "Register mutator " << mutator << ", type: " << mutator->GetMutatorType();
    mutatorsSet_.insert(mutator);
    // Propagate SUSPEND_REQUEST flag to the new mutator to avoid the following situation:
    // * A mutator (A) holds read lock of the MutatorLock.
    // * GC thread calls SuspendAll and set SUSPEND_REQUEST flag to the mutator (A) and
    //   tries to acquire write lock of the MutatorLock.
    // * The mutator (A) creates a new mutator (B) and adds it to the mutatorsSet_.
    // * SUSPEND_REQUEST is not set in the new mutator (B)
    // * The new mutator (B) starts execution, acquires read lock of the MutatorLock and enters a long loop
    // * The first mutator (A) checks SUSPEND_REQUEST flag and blocks
    // * GC will not start becuase the new mutator (B) has no SUSPEND_REQUEST flag and it will never release the
    //   MutatorLock
    //
    //     GC Thread                Mutator A
    //         |                        |
    //  SuspendAllMutators              |
    //         +                        |
    //         + - SuspendImpl -> SUSPEND_REQUEST
    //         +                        |
    //         |                        + - CreateNewMutator -> Mutator B
    //         |                        |                           |
    //         |                        |                  NO SUSPEND_REQUEST !!
    //         |                        |                           |
    // We need to propagate SUSPEND_REQUEST under the mutatorsLock_.
    // It guarantees that the flag is already set for the current mutator and we need to propagate it
    // or GC will see the new mutator in ForEachMutator.
#if !defined(ARK_USE_COMMON_RUNTIME)
    for (size_t i = suspendForNewMutatorsCounter_; i > 0U; --i) {
        mutator->SuspendImpl(true);
    }
#endif  // !ARK_USE_COMMON_RUNTIME
    if (callback != nullptr) {
        callback(mutator);
    }
}

void MutatorManager::UnregisterMutator(Mutator *mutator)
{
    os::memory::LockHolder unregisterMutatorLock(mutatorsLock_);
    LOG(DEBUG, RUNTIME) << "Unregister mutator " << mutator << ", type: " << mutator->GetMutatorType();
    mutatorsSet_.erase(mutator);
}

void MutatorManager::NotifyToSuspendDaemonMutators()
{
    os::memory::LockHolder lock(mutatorsLock_);
    suspendForNewMutatorsCounter_++;
}

void MutatorManager::SuspendAllMutators()
{
    os::memory::LockHolder suspendMutatorsLock(mutatorsLock_);
    LOG(DEBUG, RUNTIME) << "Suspend all mutators: " << mutatorsSet_.size();
    for (auto *mutator : mutatorsSet_) {
        mutator->SuspendImpl(true);
        LOG(DEBUG, RUNTIME) << "Suspend mutator " << mutator << ", type: " << mutator->GetMutatorType();
    }
    suspendForNewMutatorsCounter_++;
}

void MutatorManager::ResumeAllMutators()
{
    os::memory::LockHolder resumeMutatorsLock(mutatorsLock_);
    LOG(DEBUG, RUNTIME) << "Resume all mutators: " << mutatorsSet_.size();
    if (suspendForNewMutatorsCounter_ > 0U) {
        suspendForNewMutatorsCounter_--;
    }
    for (auto *mutator : mutatorsSet_) {
        mutator->ResumeImpl(true);
        LOG(DEBUG, RUNTIME) << "Resume mutator " << mutator << ", type: " << mutator->GetMutatorType();
    }
}

bool MutatorManager::ForEachMutator(const MutatorCallback &callback)
{
    os::memory::LockHolder lock(mutatorsLock_);
    return ForEachMutatorImpl(callback);
}

bool MutatorManager::ForEachMutatorImpl(const MutatorCallback &callback) REQUIRES(mutatorsLock_)
{
    for (auto *mutator : mutatorsSet_) {
        if (!callback(mutator)) {
            return false;
        }
    }
    return true;
}

}  // namespace ark
