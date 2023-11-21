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

#include <thread>
#include "runtime/coroutines/coroutine.h"
#include "runtime/include/thread_scopes.h"
#include "libpandabase/os/mutex.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/panda_vm.h"
#include "runtime/coroutines/threaded_coroutine.h"
#include "runtime/coroutines/threaded_coroutine_manager.h"

namespace panda {

void ThreadedCoroutineManager::Initialize(CoroutineManagerConfig config, Runtime *runtime, PandaVM *vm)
{
    if (config.emulate_js) {
        LOG(FATAL, COROUTINES) << "ThreadedCoroutineManager(): JS emulation is not supported!";
        UNREACHABLE();
    }
    if (config.workers_count > 0) {
        SetWorkersCount(static_cast<uint32_t>(config.workers_count));
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager(): setting number of coroutine workers to "
                               << GetWorkersCount();
    } else {
        SetWorkersCount(std::thread::hardware_concurrency());
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager(): setting number of coroutine workers to CPU count = "
                               << GetWorkersCount();
    }

    auto *main_co = CreateMainCoroutine(runtime, vm);
    Coroutine::SetCurrent(main_co);
}

uint32_t ThreadedCoroutineManager::GetWorkersCount() const
{
    return workers_count_;
}

void ThreadedCoroutineManager::SetWorkersCount(uint32_t n)
{
    workers_count_ = n;
}

CoroutineContext *ThreadedCoroutineManager::CreateCoroutineContext([[maybe_unused]] bool coro_has_entrypoint)
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
    return coroutine_count_;
}

size_t ThreadedCoroutineManager::GetCoroutineCountLimit()
{
    return UINT_MAX;
}

void ThreadedCoroutineManager::AddToRegistry(Coroutine *co)
{
    os::memory::LockHolder lock(coro_list_lock_);
    auto *main_co = GetMainThread();
    if (main_co != nullptr) {
        // NOTE(konstanting, #I67QXC): we should get this callback from GC instead of copying from the main thread
        co->SetPreWrbEntrypoint(main_co->GetPreWrbEntrypoint());
    }
    coroutines_.insert(co);
    coroutine_count_++;
}

void ThreadedCoroutineManager::RemoveFromRegistry(Coroutine *co)
{
    coroutines_.erase(co);
    coroutine_count_--;
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

    os::memory::LockHolder l(coro_switch_lock_);
    if (co->HasManagedEntrypoint()) {
        // entrypointless coros should be destroyed manually
        UnblockWaiters(co->GetCompletionEvent());
        if (RunnableCoroutinesExist()) {
            ScheduleNextCoroutine();
        } else {
            --running_coros_count_;
        }
    }

    {
        os::memory::LockHolder l_list(coro_list_lock_);
        RemoveFromRegistry(co);
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
    cv_await_all_.Signal();
    return true;
    // NOTE(konstanting): issue debug notifications to runtime
}

Coroutine *ThreadedCoroutineManager::Launch(CompletionEvent *completion_event, Method *entrypoint,
                                            PandaVector<Value> &&arguments)
{
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Launch started";

    auto *result = LaunchImpl(completion_event, entrypoint, std::move(arguments));
    if (result == nullptr) {
        ThrowOutOfMemoryError("Launch failed");
    }

    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Launch finished";
    return result;
}

bool ThreadedCoroutineManager::RegisterWaiter(Coroutine *waiter, CoroutineEvent *awaitee)
{
    os::memory::LockHolder l(waiters_lock_);
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
    auto *waiter_ctx = waiter->GetContext<ThreadedCoroutineContext>();

    ScopedNativeCodeThread n(waiter);
    coro_switch_lock_.Lock();

    if (!RegisterWaiter(waiter, awaitee)) {
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Await finished (no await happened)";
        coro_switch_lock_.Unlock();
        return;
    }

    waiter->RequestSuspend(true);
    if (RunnableCoroutinesExist()) {
        ScheduleNextCoroutine();
    }
    coro_switch_lock_.Unlock();
    waiter_ctx->WaitUntilResumed();

    // NB: at this point the awaitee is already deleted
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Await finished";
}

void ThreadedCoroutineManager::UnblockWaiters(CoroutineEvent *blocker)
{
    os::memory::LockHolder l(waiters_lock_);
    ASSERT(blocker != nullptr);
#ifndef NDEBUG
    {
        os::memory::LockHolder lk_blocker(*blocker);
        ASSERT(blocker->Happened());
    }
#endif
    auto w = waiters_.find(blocker);
    if (w != waiters_.end()) {
        auto *coro = w->second;
        waiters_.erase(w);
        coro->RequestUnblock();
        PushToRunnableQueue(coro);
    }
}

void ThreadedCoroutineManager::Schedule()
{
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::Schedule() request from "
                           << Coroutine::GetCurrent()->GetName();
    ScheduleImpl();
}

bool ThreadedCoroutineManager::EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int inc_mask,
                                                    unsigned int xor_mask) const
{
    os::memory::LockHolder lock(coro_list_lock_);
    for (auto *t : coroutines_) {
        if (!ApplyCallbackToThread(cb, t, inc_mask, xor_mask)) {
            return false;
        }
    }
    return true;
}

void ThreadedCoroutineManager::SuspendAllThreads()
{
    os::memory::LockHolder l_list(coro_list_lock_);
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::SuspendAllThreads started";
    for (auto *t : coroutines_) {
        t->SuspendImpl(true);
    }
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::SuspendAllThreads finished";
}

void ThreadedCoroutineManager::ResumeAllThreads()
{
    os::memory::LockHolder lock(coro_list_lock_);
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
    for (auto *co : runnables_queue_) {
        LOG(DEBUG, COROUTINES) << co->GetName() << " <";
    }
    LOG(DEBUG, COROUTINES) << "X";
}
#endif

void ThreadedCoroutineManager::PushToRunnableQueue(Coroutine *co)
{
    runnables_queue_.push_back(co);
}

bool ThreadedCoroutineManager::RunnableCoroutinesExist()
{
    return !runnables_queue_.empty();
}

Coroutine *ThreadedCoroutineManager::PopFromRunnableQueue()
{
    auto *co = runnables_queue_.front();
    runnables_queue_.pop_front();
    return co;
}

void ThreadedCoroutineManager::ScheduleNextCoroutine()
{
    Coroutine *next_coroutine = PopFromRunnableQueue();
    next_coroutine->RequestResume();
}

void ThreadedCoroutineManager::ScheduleImpl()
{
    auto *current_co = Coroutine::GetCurrent();
    auto *current_ctx = current_co->GetContext<ThreadedCoroutineContext>();
    ScopedNativeCodeThread n(current_co);

    coro_switch_lock_.Lock();
    if (RunnableCoroutinesExist()) {
        current_co->RequestSuspend(false);
        PushToRunnableQueue(current_co);
        ScheduleNextCoroutine();

        coro_switch_lock_.Unlock();
        current_ctx->WaitUntilResumed();
    } else {
        coro_switch_lock_.Unlock();
    }
}

Coroutine *ThreadedCoroutineManager::LaunchImpl(CompletionEvent *completion_event, Method *entrypoint,
                                                PandaVector<Value> &&arguments, bool start_suspended)
{
    os::memory::LockHolder l(coro_switch_lock_);
#ifndef NDEBUG
    PrintRunnableQueue("LaunchImpl begin");
#endif
    auto coro_name = entrypoint->GetFullName();
    Coroutine *co = CreateCoroutineInstance(completion_event, entrypoint, std::move(arguments), std::move(coro_name));
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(co);
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::LaunchImpl: failed to create a coroutine!";
        return co;
    }
    auto *ctx = co->GetContext<ThreadedCoroutineContext>();
    if (start_suspended) {
        ctx->WaitUntilInitialized();
        if (running_coros_count_ >= GetWorkersCount()) {
            PushToRunnableQueue(co);
        } else {
            ++running_coros_count_;
            ctx->RequestResume();
        }
    } else {
        ++running_coros_count_;
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
        os::memory::LockHolder l(coro_switch_lock_);
        ctx->MainThreadFinished();
        if (RunnableCoroutinesExist()) {
            ScheduleNextCoroutine();
        }
    }
    // then start awaiting for other coroutines to complete
    os::memory::LockHolder lk(cv_mutex_);
    ctx->EnterAwaitLoop();
    while (coroutine_count_ > 1) {  // main coro runs till VM shutdown
        cv_await_all_.Wait(&cv_mutex_);
        LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::MainCoroutineCompleted(): await_all(): still "
                               << coroutine_count_ << " coroutines exist...";
    }
    LOG(DEBUG, COROUTINES) << "ThreadedCoroutineManager::MainCoroutineCompleted(): await_all() done";
}

void ThreadedCoroutineManager::Finalize() {}

}  // namespace panda
