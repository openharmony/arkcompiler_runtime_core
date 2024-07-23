/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_RENDEZVOUS_H
#define PANDA_RUNTIME_MEM_RENDEZVOUS_H

#include <atomic>

#include "libpandabase/os/mutex.h"
#include "macros.h"
#include "runtime/include/thread.h"
#include "include/locks.h"

namespace ark {

class PandaVM;

/**
 * Meeting point for all managed threads.
 * High level plan:
 *      * Check if there's already a main thread (running_)
 *      * If there is, wait until we get woken up
 *      * Otherwise, acquire write global Mutator lock and set field running_
 */
class Rendezvous {
public:
    explicit Rendezvous(PandaVM *pandaVm);
    virtual ~Rendezvous() = default;

    // Wait until all threads release Mutator lock and acquires it for write;
    virtual void SafepointBegin() ACQUIRE(*mutatorLock_);
    // Ends safepoint (wakes up waiting threads, releases Mutator lock);
    virtual void SafepointEnd() RELEASE(*mutatorLock_);

    MutatorLock *GetMutatorLock()
    {
        return mutatorLock_;
    }

private:
    NO_MOVE_SEMANTIC(Rendezvous);
    NO_COPY_SEMANTIC(Rendezvous);

    MutatorLock *mutatorLock_;
};

class ScopedSuspendAllThreads {
public:
    explicit ScopedSuspendAllThreads(Rendezvous *rendezvous) ACQUIRE(*rendezvous_->GetMutatorLock())
        : rendezvous_(rendezvous)
    {
        ASSERT(rendezvous_ != nullptr);
        rendezvous_->SafepointBegin();
    }
    ~ScopedSuspendAllThreads() RELEASE(*rendezvous_->GetMutatorLock())
    {
        ASSERT(rendezvous_ != nullptr);
        rendezvous_->SafepointEnd();
    }

    NO_COPY_SEMANTIC(ScopedSuspendAllThreads);
    NO_MOVE_SEMANTIC(ScopedSuspendAllThreads);

private:
    Rendezvous *rendezvous_;
};

class ScopedSuspendAllThreadsRunning {
public:
    explicit ScopedSuspendAllThreadsRunning(Rendezvous *rendezvous) ACQUIRE(*rendezvous_->GetMutatorLock());
    ~ScopedSuspendAllThreadsRunning() RELEASE(*rendezvous_->GetMutatorLock());

    NO_COPY_SEMANTIC(ScopedSuspendAllThreadsRunning);
    NO_MOVE_SEMANTIC(ScopedSuspendAllThreadsRunning);

private:
    Rendezvous *rendezvous_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_MEM_RENDEZVOUS_H
