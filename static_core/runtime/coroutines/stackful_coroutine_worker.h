/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_WORKER_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_WORKER_H

#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_events.h"

namespace panda {

class StackfulCoroutineContext;
class StackfulCoroutineManager;

/**
 * Represents a worker thread for stackful coroutines.
 * Contains local part of the scheduling machinery (local coroutine queues, methods)
 */
class StackfulCoroutineWorker {
public:
    enum class ScheduleLoopType { THREAD, FIBER };

    NO_COPY_SEMANTIC(StackfulCoroutineWorker);
    NO_MOVE_SEMANTIC(StackfulCoroutineWorker);

    /**
     * @brief The worker constructor. Create the worker and its schedule loop.
     *
     * Notable parameters:
     * @param type defines the schedule loop type for this worker: a separate thread or a coroutine ("FIBER")
     */
    StackfulCoroutineWorker(Runtime *runtime, PandaVM *vm, StackfulCoroutineManager *coro_manager,
                            ScheduleLoopType type, PandaString name);
    ~StackfulCoroutineWorker() = default;

    /// @return false if the worker is stopped and does not schedule anything, otherwise true
    bool IsActive() const
    {
        os::memory::LockHolder lock(runnables_lock_);
        return active_;
    }

    /// enable or disable the worker
    void SetActive(bool val)
    {
        os::memory::LockHolder lock(runnables_lock_);
        active_ = val;
        runnables_cv_.Signal();
    }

    /// @return the moving average number of runnable coroutines in the queue
    double GetLoadFactor() const
    {
        return load_factor_;
    }

    PandaString GetName() const
    {
        return name_;
    }

    void SetName(PandaString name)
    {
        name_ = std::move(name);
    }

    /**
     * @brief Adds a coroutine to the runnables queue
     * @param new_coro coroutine to add
     * @param prioritize if true, add to the beginning of the queue (otherwise to the end)
     */
    void AddRunnableCoroutine(Coroutine *new_coro, bool prioritize = false);

    /**
     * @brief Block current coroutine till an event happens and switch context to the next ready one
     * @param awaitee the event to wait
     * @return false if the event is apready happened, true after the waiting is completed
     */
    bool WaitForEvent(CoroutineEvent *awaitee) RELEASE(awaitee);

    /**
     * @brief Signal that an event has happened and unblock all the coroutines in the current worker that are waiting
     * for this event
     * @param blocker the event that has happened
     */
    void UnblockWaiters(CoroutineEvent *blocker);

    /**
     * @brief Add a coroutine to the finalization queue for future destruction and schedule the next ready one for
     * execution. Used by a coroutine being terminated for safe self-destruction.
     * @param finalizee coroutine to finalize (the caller)
     */
    void RequestFinalization(Coroutine *finalizee);

    /// @brief schedule the next ready coroutine from the runnables queue for execution
    void RequestSchedule();

    /// @brief call to delete the fake "schedule loop" coroutine
    void FinalizeFiberScheduleLoop();

#ifndef NDEBUG
    void PrintRunnables(const PandaString &requester);
#endif

private:
    /* schedule loop management */
    /// the EP for threaded schedule loops
    void ThreadProc();
    /// the EP for fiber schedule loops
    void ScheduleLoop();
    /// the schedule loop itself
    void ScheduleLoopBody();
    /// the helper proxy function for the fiber schedule loop
    static void ScheduleLoopProxy(void *worker);

    /* runnables queue management */
    void PushToRunnableQueue(Coroutine *co, bool push_front = false);
    Coroutine *PopFromRunnableQueue();
    bool RunnableCoroutinesExist() const;
    void WaitForRunnables() REQUIRES(runnables_lock_);

    /* scheduling machinery from high level functions to elementary helpers */
    /**
     * Schedule the next ready coroutine from the runnables queue for execution if present, otherwise wait till those
     * appear, then pick the best suitable and schedule it. After that eventually the current coroutine will be
     * scheduled for execution too. Upon that, the control flow will get back to this function and it will return.
     */
    void RequestScheduleImpl();
    void BlockCurrentCoroAndScheduleNext() RELEASE(runnables_lock_) RELEASE(waiters_lock_);
    void SuspendCurrentCoroAndScheduleNext() RELEASE(runnables_lock_);
    template <bool SUSPEND_AS_BLOCKED>
    void SuspendCurrentCoroGeneric();
    void BlockCurrentCoro();
    void SuspendCurrentCoro();
    /* "the lesser evil": keep thread safety annotations but duplicate the function body */
    void ScheduleNextCoroUnlockRunnablesWaiters() RELEASE(runnables_lock_) RELEASE(waiters_lock_);
    void ScheduleNextCoroUnlockRunnables() RELEASE(runnables_lock_);
    void ScheduleNextCoroUnlockNone();
    StackfulCoroutineContext *GetCurrentContext() const;
    StackfulCoroutineContext *PrepareNextRunnableContextForSwitch();
    void SwitchCoroutineContext(StackfulCoroutineContext *from, StackfulCoroutineContext *to);

    /* various helper functions */
    /// parse the finalization queue and destroy all coroutines from it
    void FinalizeTerminatedCoros();
    /// recalculate the load factor
    void UpdateLoadFactor() REQUIRES(runnables_lock_);

    StackfulCoroutineManager *coro_manager_;
    Coroutine *schedule_loop_ctx_ = nullptr;
    bool active_ GUARDED_BY(runnables_lock_) = true;
    os::thread::ThreadId id_;

    // runnable coroutines-related members
    mutable os::memory::RecursiveMutex runnables_lock_;
    os::memory::ConditionVariable runnables_cv_;
    PandaDeque<Coroutine *> runnables_ GUARDED_BY(runnables_lock_);
    // blocked coros-related members: Coroutine AWAITS CoroutineEvent
    mutable os::memory::Mutex waiters_lock_;
    PandaMap<CoroutineEvent *, Coroutine *> waiters_ GUARDED_BY(waiters_lock_);
    // terminated coros (waiting for deletion)
    PandaQueue<Coroutine *> finalization_queue_;

    /// the moving average number of coroutines in the runnable queue
    std::atomic<double> load_factor_ = 0;

    PandaString name_;
};

}  // namespace panda

#endif /* PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_WORKER_H */