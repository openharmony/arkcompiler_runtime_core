/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_MANAGER_H
#define PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_MANAGER_H

#include "libpandabase/os/mutex.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/include/value.h"

namespace panda {

/**
 * @brief Thread-based coroutine manager implementation.
 *
 * In this implementation coroutines are OS threads. The invariants (only N coroutines are running simultaniously, etc.)
 * are kept with the help of mutexes and condition variables.
 *
 * For interface function descriptions see CoroutineManager class declaration.
 */
class ThreadedCoroutineManager : public CoroutineManager {
public:
    NO_COPY_SEMANTIC(ThreadedCoroutineManager);
    NO_MOVE_SEMANTIC(ThreadedCoroutineManager);

    explicit ThreadedCoroutineManager(CoroutineFactory factory) : CoroutineManager(factory) {}
    ~ThreadedCoroutineManager() override = default;

    /* CoroutineManager interfaces, see CoroutineManager class for the details */
    void Initialize(CoroutineManagerConfig config, Runtime *runtime, PandaVM *vm) override;
    void Finalize() override;
    void RegisterCoroutine(Coroutine *co) override;
    bool TerminateCoroutine(Coroutine *co) override;
    Coroutine *Launch(CompletionEvent *completion_event, Method *entrypoint, PandaVector<Value> &&arguments) override;
    void Schedule() override;
    void Await(CoroutineEvent *awaitee) RELEASE(awaitee) override;
    void UnblockWaiters(CoroutineEvent *blocker) override;

    /* ThreadManager interfaces, see ThreadManager class for the details */
    void WaitForDeregistration() override;
    void SuspendAllThreads() override;
    void ResumeAllThreads() override;
    bool IsRunningThreadExist() override;

    /* debugging tools */
    /**
     * For ThreadedCoroutineManager implementation: no action performed if an attempt to switch coroutines on current
     * worker is detected when coroutine switch is disabled.
     */
    void DisableCoroutineSwitch() override {};
    void EnableCoroutineSwitch() override {};
    bool IsCoroutineSwitchDisabled() override
    {
        return false;
    };

protected:
    bool EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int inc_mask,
                              unsigned int xor_mask) const override;
    CoroutineContext *CreateCoroutineContext(bool coro_has_entrypoint) override;
    void DeleteCoroutineContext(CoroutineContext *ctx) override;

    size_t GetCoroutineCount() override;
    size_t GetCoroutineCountLimit() override;

    /// Sets number of coroutines that can execute simultaniously
    void SetWorkersCount(uint32_t n);
    /// Returns number of coroutines that can execute simultaniously
    uint32_t GetWorkersCount() const;

private:
    Coroutine *LaunchImpl(CompletionEvent *completion_event, Method *entrypoint, PandaVector<Value> &&arguments,
                          bool start_suspended = true);
    void ScheduleImpl();

    /* runnables queue management */
    void PushToRunnableQueue(Coroutine *co);
    Coroutine *PopFromRunnableQueue();
#ifndef NDEBUG
    void PrintRunnableQueue(const PandaString &requester);
#endif
    bool RunnableCoroutinesExist();

    /* coroutine registry management */
    void AddToRegistry(Coroutine *co);
    void RemoveFromRegistry(Coroutine *co) REQUIRES(coro_list_lock_);

    void DeleteCoroutineInstance(Coroutine *co);
    bool RegisterWaiter(Coroutine *waiter, CoroutineEvent *awaitee) RELEASE(awaitee);
    void ScheduleNextCoroutine();
    void MainCoroutineCompleted();

    os::memory::Mutex coro_switch_lock_;
    os::memory::Mutex waiters_lock_;
    mutable os::memory::Mutex coro_list_lock_;
    // all registered coros
    PandaSet<Coroutine *> coroutines_ GUARDED_BY(coro_list_lock_);
    // ready coros
    PandaDeque<Coroutine *> runnables_queue_;
    // blocked coros: Coroutine AWAITS CoroutineEvent
    PandaMap<CoroutineEvent *, Coroutine *> waiters_;

    os::memory::ConditionVariable cv_await_all_;
    os::memory::Mutex cv_mutex_;

    uint32_t workers_count_ = 1;
    // main is running from the very beginning
    std::atomic_uint32_t running_coros_count_ = 1;
    std::atomic_uint32_t coroutine_count_ = 0;
};

}  // namespace panda

#endif /* PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_MANAGER_H */
