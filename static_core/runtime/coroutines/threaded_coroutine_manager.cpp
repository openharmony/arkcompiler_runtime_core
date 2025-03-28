/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <thread>
#include "runtime/coroutines/coroutine.h"
#include "runtime/include/thread_scopes.h"
#include "libpandabase/os/mutex.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/panda_vm.h"
#include "runtime/coroutines/threaded_coroutine.h"
#include "runtime/coroutines/threaded_coroutine_manager.h"

namespace ark {

void ThreadedCoroutineManager::Initialize(CoroutineManagerConfig config, Runtime *runtime, PandaVM *vm)
{
    // create and activate workers
    size_t numberOfAvailableCores = std::max(std::thread::hardware_concurrency() / 4ULL, 2ULL);
    size_t targetNumberOfWorkers = (config.workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO)
                                       ? numberOfAvailableCores
                                       : config.workersCount;
    if (config.workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO) {
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager(): AUTO mode selected, will set number of coroutine "
                                  "workers to number of CPUs / 4, but not less than 2 = "
                               << targetNumberOfWorkers;
    }
    os::memory::LockHolder lock(workersLock_);
    CreateWorkers(targetNumberOfWorkers, runtime, vm);
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager(): successfully created and activated " << workers_.size()
                           << " coroutine workers";
}

uint32_t ThreadedCoroutineManager::GetActiveWorkersCount() const
{
    os::memory::LockHolder lkWorkers(workersLock_);
    return activeWorkersCount_;
}

void ThreadedCoroutineManager::CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    auto allocator = runtime->GetInternalAllocator();
    for (size_t id = 0; id < howMany; ++id) {
        auto *w = allocator->New<CoroutineWorker>(runtime, vm);
        workers_.push_back(w);
        ASSERT(workers_[id] == w);
    }
    activeWorkersCount_ = howMany;

    auto *mainCo = CreateMainCoroutine(runtime, vm);
    mainCo->SetWorker(workers_[0]);
    Coroutine::SetCurrent(mainCo);
}

CoroutineContext *ThreadedCoroutineManager::CreateCoroutineContext([[maybe_unused]] bool coroHasEntrypoint)
{
    auto alloc = Runtime::GetCurrent()->GetInternalAllocator();
    return alloc->New<ThreadedCoroutineContext>();
}

void ThreadedCoroutineManager::DeleteCoroutineContext(CoroutineContext *ctx)
{
    auto alloc = Runtime::GetCurrent()->GetInternalAllocator();
    alloc->Delete(ctx);
}

size_t ThreadedCoroutineManager::GetCoroutineCount()
{
    return coroutineCount_;
}

size_t ThreadedCoroutineManager::GetCoroutineCountLimit()
{
    return UINT_MAX;
}

void ThreadedCoroutineManager::AddToRegistry(Coroutine *co)
{
    co->GetVM()->GetGC()->OnThreadCreate(co);
    coroutines_.insert(co);
    coroutineCount_++;
}

void ThreadedCoroutineManager::RemoveFromRegistry(Coroutine *co)
{
    coroutines_.erase(co);
    coroutineCount_--;
}

void ThreadedCoroutineManager::DeleteCoroutineInstance(Coroutine *co)
{
    auto *context = co->GetContext<CoroutineContext>();
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(co);
    DeleteCoroutineContext(context);
}

void ThreadedCoroutineManager::RegisterCoroutine(Coroutine *co)
{
    os::memory::LockHolder lock(coroListLock_);
    AddToRegistry(co);
    // Propagate SUSPEND_REQUEST flag to the new coroutine to avoid the following situation:
    // * Main coro holds read lock of the MutatorLock.
    // * GC thread calls SuspendAll nad set SUSPEND_REQUEST flag to the main coro and
    //   tries to acquire write lock of the MutatorLock.
    // * Main coro creates a new coro and adds it to the coroutines_ list.
    // * SUSPEND_REQUEST is not set in the new coroutine
    // * New coro starts execution, acquires read lock of the MutatorLock and enters a long loop
    // * Main coro checks SUSPEND_REQUEST flag and blocks
    // * GC will not start becuase the new coro has no SUSPEND_REQUEST flag and it will never release the MutatorLock
    //
    // We need to propagate SUSPEND_REQUEST under the coroListLock_.
    // It guarantees that the flag is already set for the current coro and we need to propagate it
    // or GC will see the new coro in EnumerateAllThreads.
    if (Thread::GetCurrent() != nullptr && Coroutine::GetCurrent() != nullptr &&
        Coroutine::GetCurrent()->IsSuspended() && !co->IsSuspended()) {
        co->SuspendImpl(true);
    }
}

bool ThreadedCoroutineManager::TerminateCoroutine(Coroutine *co)
{
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::TerminateCoroutine() started";
    co->NativeCodeEnd();
    co->UpdateStatus(ThreadStatus::TERMINATING);

    os::memory::LockHolder l(coroSwitchLock_);
    if (co->HasManagedEntrypoint()) {
        // entrypointless coros should be destroyed manually
        if (RunnableCoroutinesExist()) {
            ScheduleNextCoroutine();
        } else {
            --runningCorosCount_;
        }
    }

    {
        os::memory::LockHolder lList(coroListLock_);
        RemoveFromRegistry(co);
        // We need collect TLAB metrics and clear TLAB before calling the manage thread destructor
        // because of the possibility heap use after free. This happening when GC starts execute ResetYoungAllocator
        // method which start iterate set of threads, collect TLAB metrics and clear TLAB. If thread was deleted from
        // set but we haven't destroyed the thread yet, GC won't collect metrics and can complete TLAB
        // deletion faster. And when we try to get the TLAB metrics in the destructor of managed thread, we will get
        // heap use after free
        co->CollectTLABMetrics();
        co->ClearTLAB();
        // DestroyInternalResources() must be called in one critical section with
        // RemoveFromRegistry (under core_list_lock_). This function transfers cards from coro's post_barrier buffer to
        // UpdateRemsetThread internally. Situation when cards still remain and UpdateRemsetThread cannot visit the
        // coro (because it is already removed) must be impossible.
        co->DestroyInternalResources();
    }
    co->UpdateStatus(ThreadStatus::FINISHED);
    Runtime::GetCurrent()->GetNotificationManager()->ThreadEndEvent(co);

    if (!co->HasManagedEntrypoint()) {
        // entrypointless coros should be destroyed manually
        return false;
    }

    DeleteCoroutineInstance(co);

    os::memory::LockHolder lk(cvAwaitAllMutex_);
    cvAwaitAll_.Signal();
    return true;
    // NOTE(konstanting): issue debug notifications to runtime
}

bool ThreadedCoroutineManager::Launch(CompletionEvent *completionEvent, Method *entrypoint,
                                      PandaVector<Value> &&arguments, [[maybe_unused]] CoroutineLaunchMode mode)
{
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Launch started";

    bool result = LaunchImpl(completionEvent, entrypoint, std::move(arguments));
    if (!result) {
        ThrowOutOfMemoryError("Launch failed");
    }

    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Launch finished";
    return true;
}

bool ThreadedCoroutineManager::LaunchImmediately([[maybe_unused]] CompletionEvent *completionEvent,
                                                 [[maybe_unused]] Method *entrypoint,
                                                 [[maybe_unused]] PandaVector<Value> &&arguments,
                                                 [[maybe_unused]] CoroutineLaunchMode mode)
{
    LOG(FATAL, COROUTINES) << "ThreadedCoroutineManager::LaunchImmediately not supported";
    return false;
}

bool ThreadedCoroutineManager::RegisterWaiter(Coroutine *waiter, CoroutineEvent *awaitee)
{
    os::memory::LockHolder l(waitersLock_);
    if (awaitee->Happened()) {
        awaitee->Unlock();
        return false;
    }

    awaitee->Unlock();
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::RegisterAsAwaitee: " << waiter->GetName() << " AWAITS";
    waiters_.insert({awaitee, waiter});
    return true;
}

void ThreadedCoroutineManager::Await(CoroutineEvent *awaitee)
{
    ASSERT(awaitee != nullptr);
    ASSERT_NATIVE_CODE();
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Await started";

    auto *waiter = Coroutine::GetCurrent();
    auto *waiterCtx = waiter->GetContext<ThreadedCoroutineContext>();

    coroSwitchLock_.Lock();

    if (!RegisterWaiter(waiter, awaitee)) {
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Await finished (no await happened)";
        coroSwitchLock_.Unlock();
        return;
    }

    waiter->RequestSuspend(true);
    if (RunnableCoroutinesExist()) {
        ScheduleNextCoroutine();
    }
    coroSwitchLock_.Unlock();
    waiterCtx->WaitUntilResumed();

    // NB: at this point the awaitee is already deleted
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Await finished";
}

void ThreadedCoroutineManager::UnblockWaiters(CoroutineEvent *blocker)
{
    os::memory::LockHolder lh(coroSwitchLock_);
    UnblockWaitersImpl(blocker);
}

void ThreadedCoroutineManager::UnblockWaitersImpl(CoroutineEvent *blocker)
{
    os::memory::LockHolder l(waitersLock_);
    ASSERT(blocker != nullptr);
#ifndef NDEBUG
    {
        os::memory::LockHolder lkBlocker(*blocker);
        ASSERT(blocker->Happened());
    }
#endif
    auto w = waiters_.find(blocker);
    while (w != waiters_.end()) {
        auto *coro = w->second;
        waiters_.erase(w);
        coro->RequestUnblock();
        PushToRunnableQueue(coro);
        w = waiters_.find(blocker);
    }
}

void ThreadedCoroutineManager::Schedule()
{
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Schedule() request from "
                           << Coroutine::GetCurrent()->GetName();
    ScheduleImpl();
}

bool ThreadedCoroutineManager::EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                                                    unsigned int xorMask) const
{
    os::memory::LockHolder lock(coroListLock_);
    for (auto *t : coroutines_) {
        if (!ApplyCallbackToThread(cb, t, incMask, xorMask)) {
            return false;
        }
    }
    return true;
}

void ThreadedCoroutineManager::SuspendAllThreads()
{
    os::memory::LockHolder lList(coroListLock_);
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::SuspendAllThreads started";
    for (auto *t : coroutines_) {
        t->SuspendImpl(true);
    }
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::SuspendAllThreads finished";
}

void ThreadedCoroutineManager::ResumeAllThreads()
{
    os::memory::LockHolder lock(coroListLock_);
    for (auto *t : coroutines_) {
        t->ResumeImpl(true);
    }
}

bool ThreadedCoroutineManager::IsRunningThreadExist()
{
    UNREACHABLE();
    // NOTE(konstanting): correct implementation. Which coroutine do we consider running?
    return false;
}

void ThreadedCoroutineManager::WaitForDeregistration()
{
    MainCoroutineCompleted();
}

#ifndef NDEBUG
void ThreadedCoroutineManager::PrintRunnableQueue(const PandaString &requester)
{
    LOG(DEBUG, COROUTINES) << "[" << requester << "] ";
    for (auto *co : runnablesQueue_) {
        LOG(DEBUG, COROUTINES) << co->GetName() << " <";
    }
    LOG(DEBUG, COROUTINES) << "X";
}
#endif

void ThreadedCoroutineManager::PushToRunnableQueue(Coroutine *co)
{
    runnablesQueue_.push_back(co);
}

bool ThreadedCoroutineManager::RunnableCoroutinesExist()
{
    return !runnablesQueue_.empty();
}

Coroutine *ThreadedCoroutineManager::PopFromRunnableQueue()
{
    auto *co = runnablesQueue_.front();
    runnablesQueue_.pop_front();
    return co;
}

void ThreadedCoroutineManager::ScheduleNextCoroutine()
{
    Coroutine *nextCoroutine = PopFromRunnableQueue();
    nextCoroutine->RequestResume();
}

void ThreadedCoroutineManager::ScheduleImpl()
{
    ASSERT_NATIVE_CODE();
    auto *currentCo = Coroutine::GetCurrent();
    auto *currentCtx = currentCo->GetContext<ThreadedCoroutineContext>();

    coroSwitchLock_.Lock();
    if (RunnableCoroutinesExist()) {
        currentCo->RequestSuspend(false);
        PushToRunnableQueue(currentCo);
        ScheduleNextCoroutine();

        coroSwitchLock_.Unlock();
        currentCtx->WaitUntilResumed();
    } else {
        coroSwitchLock_.Unlock();
    }
}

CoroutineWorker *ThreadedCoroutineManager::ChooseWorkerForCoroutine([[maybe_unused]] Coroutine *co)
{
    ASSERT(co != nullptr);
    // Currently this function is a stub: we assign everything to the main worker and hope for the best.
    // Later on, we will emulate the correct worker selection.
    os::memory::LockHolder lkWorkers(workersLock_);
    return workers_[0];
}

bool ThreadedCoroutineManager::LaunchImpl(CompletionEvent *completionEvent, Method *entrypoint,
                                          PandaVector<Value> &&arguments, bool startSuspended)
{
    os::memory::LockHolder l(coroSwitchLock_);
#ifndef NDEBUG
    PrintRunnableQueue("LaunchImpl begin");
#endif
    auto coroName = entrypoint->GetFullName();
    Coroutine *co = CreateCoroutineInstance(completionEvent, entrypoint, std::move(arguments), std::move(coroName),
                                            Coroutine::Type::MUTATOR);
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(co);
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::LaunchImpl: failed to create a coroutine!";
        return false;
    }
    // assign a worker
    auto *w = ChooseWorkerForCoroutine(co);
    co->SetWorker(w);
    // run
    auto *ctx = co->GetContext<ThreadedCoroutineContext>();
    if (startSuspended) {
        ctx->WaitUntilInitialized();
        if (runningCorosCount_ >= GetActiveWorkersCount()) {
            PushToRunnableQueue(co);
        } else {
            ++runningCorosCount_;
            ctx->RequestResume();
        }
    } else {
        ++runningCorosCount_;
    }
#ifndef NDEBUG
    PrintRunnableQueue("LaunchImpl end");
#endif
    return true;
}

void ThreadedCoroutineManager::MainCoroutineCompleted()
{
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::MainCoroutineCompleted() started";
    auto *ctx = Coroutine::GetCurrent()->GetContext<ThreadedCoroutineContext>();
    //  firstly yield
    {
        os::memory::LockHolder l(coroSwitchLock_);
        ctx->MainThreadFinished();
        if (RunnableCoroutinesExist()) {
            ScheduleNextCoroutine();
        }
    }
    // then start awaiting for other coroutines to complete
    os::memory::LockHolder lk(cvAwaitAllMutex_);
    ctx->EnterAwaitLoop();
    while (coroutineCount_ > 1) {  // main coro runs till VM shutdown
        cvAwaitAll_.Wait(&cvAwaitAllMutex_);
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::MainCoroutineCompleted(): await_all(): still "
                               << coroutineCount_ << " coroutines exist...";
    }
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::MainCoroutineCompleted(): await_all() done";

    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::MainCoroutineCompleted(): deleting workers";
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        for (auto *worker : workers_) {
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(worker);
        }
        workers_.clear();
    }
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::MainCoroutineCompleted(): DONE";
}

void ThreadedCoroutineManager::Finalize() {}

bool ThreadedCoroutineManager::IsMainWorker(Coroutine *co) const
{
    return co == GetMainThread();
}

}  // namespace ark
