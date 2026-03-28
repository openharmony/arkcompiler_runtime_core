/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_EXECUTION_COROUTINES_COROUTINE_WORKER_H
#define PANDA_RUNTIME_EXECUTION_COROUTINES_COROUTINE_WORKER_H

#include "runtime/execution/job_worker_thread.h"

namespace ark {

class Coroutine;

/// Represents a coroutine worker, which can host multiple coroutines and schedule them.
class CoroutineWorker : public JobWorkerThread {
public:
    CoroutineWorker(Runtime *runtime, PandaVM *vm, PandaString name, Id id, bool inExclusiveMode, bool isMainWorker)
        : JobWorkerThread(runtime, vm, std::move(name), id, inExclusiveMode, isMainWorker)
    {
    }

    void OnCoroBecameActive(Coroutine *co);
    virtual void OnNewCoroutineStartup([[maybe_unused]] Coroutine *co) {};

    void TriggerSchedulerExternally(Coroutine *requester, int64_t delayMs = 0);

    /**
     * This method perform ready asynchronous work of the coroutine worker.
     * If there is no ready async work but pending exists, the method blocks until it gets ready.
     * @return true if some async work still exists, false otherwise
     */
    virtual bool ProcessAsyncWork()
    {
        return false;
    }

#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::Mutator *GetMutator()
    {
        return mutator_;
    }
#endif  // ARK_USE_COMMON_RUNTIME

private:
#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::Mutator *mutator_ = nullptr;
#endif  // ARK_USE_COMMON_RUNTIME
};

}  // namespace ark

#endif /* PANDA_RUNTIME_EXECUTION_COROUTINES_COROUTINE_WORKER_H */
