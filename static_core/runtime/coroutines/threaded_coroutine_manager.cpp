/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
    if (config.emulateJs) {
        LOG(FATAL, COROUTINES) << "ThreadedCoroutineManager(): JS emulation is not supported!";
        UNREACHABLE();
    }
    if (config.workersCount > 0) {
        SetWorkersCount(static_cast<uint32_t>(config.workersCount));
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager(): setting number of coroutine workers to "
                               << GetWorkersCount();
    } else {
        SetWorkersCount(std::thread::hardware_concurrency());
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager(): setting number of coroutine workers to CPU count = "
                               << GetWorkersCount();
    }

    auto *mainCo = CreateMainCoroutine(runtime, vm);
    Coroutine::SetCurrent(mainCo);
}

uint32_t ThreadedCoroutineManager::GetWorkersCount() const
{
    return workersCount_;
}

void ThreadedCoroutineManager::SetWorkersCount(uint32_t n)
{
    workersCount_ = n;
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
    os::memory::LockHolder lock(coroListLock_);
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
    AddToRegistry(co);
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

Coroutine *ThreadedCoroutineManager::Launch(CompletionEvent *completionEvent, Method *entrypoint,
                                            PandaVector<Value> &&arguments, [[maybe_unused]] CoroutineLaunchMode mode)
{
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Launch started";

    auto *result = LaunchImpl(completionEvent, entrypoint, std::move(arguments));
    if (result == nullptr) {
        ThrowOutOfMemoryError("Launch failed");
    }

    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Launch finished";
    return result;
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
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Await started";

    auto *waiter = Coroutine::GetCurrent();
    auto *waiterCtx = waiter->GetContext<ThreadedCoroutineContext>();

    ScopedNativeCodeThread n(waiter);
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
    auto *currentCo = Coroutine::GetCurrent();
    auto *currentCtx = currentCo->GetContext<ThreadedCoroutineContext>();
    ScopedNativeCodeThread n(currentCo);

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

Coroutine *ThreadedCoroutineManager::LaunchImpl(CompletionEvent *completionEvent, Method *entrypoint,
                                                PandaVector<Value> &&arguments, bool startSuspended)
{
    os::memory::LockHolder l(coroSwitchLock_);
#ifndef NDEBUG
    PrintRunnableQueue("LaunchImpl begin");
#endif
    auto coroName = entrypoint->GetFullName();
    Coroutine *co = CreateCoroutineInstance(completionEvent, entrypoint, std::move(arguments), std::move(coroName));
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(co);
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::LaunchImpl: failed to create a coroutine!";
        return co;
    }
    auto *ctx = co->GetContext<ThreadedCoroutineContext>();
    if (startSuspended) {
        ctx->WaitUntilInitialized();
        if (runningCorosCount_ >= GetWorkersCount()) {
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
    return co;
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
}

void ThreadedCoroutineManager::Finalize() {}

bool ThreadedCoroutineManager::IsMainWorker(Coroutine *co) const
{
    return co == GetMainThread();
}

}  // namespace ark
