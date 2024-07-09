/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "runtime/include/thread_scopes.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"

namespace ark {

StackfulCoroutineWorker::StackfulCoroutineWorker(Runtime *runtime, PandaVM *vm, StackfulCoroutineManager *coroManager,
                                                 ScheduleLoopType type, PandaString name, size_t id)
    : runtime_(runtime),
      vm_(vm),
      coroManager_(coroManager),
      threadId_(os::thread::GetCurrentThreadId()),
      stats_(name),
      name_(std::move(name)),
      id_(id)
{
    ASSERT(id <= stackful_coroutines::MAX_WORKER_ID);
    LOG(DEBUG, COROUTINES) << "Created a coroutine worker instance: id=" << id_ << " name=" << name_;
    if (type == ScheduleLoopType::THREAD) {
        std::thread t(&StackfulCoroutineWorker::ThreadProc, this);
        os::thread::SetThreadName(t.native_handle(), name_.c_str());
        t.detach();
        // will create the schedule loop coroutine in the thread proc in order to set the stack protector correctly
    } else {
        scheduleLoopCtx_ =
            coroManager->CreateNativeCoroutine(runtime, vm, ScheduleLoopProxy, this, "[fiber_sch] " + GetName());
        PushToRunnableQueue(scheduleLoopCtx_);
    }
}

void StackfulCoroutineWorker::AddRunnableCoroutine(Coroutine *newCoro, bool prioritize)
{
    PushToRunnableQueue(newCoro, prioritize);
}

bool StackfulCoroutineWorker::WaitForEvent(CoroutineEvent *awaitee)
{
    // precondition: this method is not called by the schedule loop coroutine

    Coroutine *waiter = Coroutine::GetCurrent();
    ASSERT(GetCurrentContext()->GetWorker() == this);
    ASSERT(awaitee != nullptr);

    if (awaitee->Happened()) {
        awaitee->Unlock();
        return false;
    }

    waitersLock_.Lock();
    awaitee->Unlock();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::AddWaitingCoroutine: " << waiter->GetName() << " AWAITS";
    waiters_.insert({awaitee, waiter});

    runnablesLock_.Lock();
    ASSERT(RunnableCoroutinesExist());
    ScopedNativeCodeThread n(Coroutine::GetCurrent());
    // will unlock waiters_lock_ and switch ctx
    BlockCurrentCoroAndScheduleNext();

    return true;
}

void StackfulCoroutineWorker::UnblockWaiters(CoroutineEvent *blocker)
{
    os::memory::LockHolder lock(waitersLock_);
    auto w = waiters_.find(blocker);
    while (w != waiters_.end()) {
        auto *coro = w->second;
        waiters_.erase(w);
        coro->RequestUnblock();
        PushToRunnableQueue(coro);
        w = waiters_.find(blocker);
    }
}

void StackfulCoroutineWorker::RequestFinalization(Coroutine *finalizee)
{
    // precondition: current coro and finalizee belong to the current worker
    ASSERT(finalizee->GetContext<StackfulCoroutineContext>()->GetWorker() == this);
    ASSERT(GetCurrentContext()->GetWorker() == this);

    finalizationQueue_.push(finalizee);
    ScheduleNextCoroUnlockNone();
}

void StackfulCoroutineWorker::RequestSchedule()
{
    RequestScheduleImpl();
}

void StackfulCoroutineWorker::FinalizeFiberScheduleLoop()
{
    ASSERT(GetCurrentContext()->GetWorker() == this);

    // part of MAIN finalization sequence
    if (RunnableCoroutinesExist()) {
        // the schedule loop is still runnable
        ASSERT(scheduleLoopCtx_->HasNativeEntrypoint());
        runnablesLock_.Lock();
        // sch loop only
        ASSERT(runnables_.size() == 1);
        SuspendCurrentCoroAndScheduleNext();
    }
}

void StackfulCoroutineWorker::DisableCoroutineSwitch()
{
    ++disableCoroSwitchCounter_;
    LOG(DEBUG, COROUTINES) << "Coroutine switch on " << GetName()
                           << " has been disabled! Recursive ctr = " << disableCoroSwitchCounter_;
}

void StackfulCoroutineWorker::EnableCoroutineSwitch()
{
    ASSERT(IsCoroutineSwitchDisabled());
    --disableCoroSwitchCounter_;
    LOG(DEBUG, COROUTINES) << "Coroutine switch on " << GetName()
                           << " has been enabled! Recursive ctr = " << disableCoroSwitchCounter_;
}

bool StackfulCoroutineWorker::IsCoroutineSwitchDisabled()
{
    return disableCoroSwitchCounter_ > 0;
}

#ifndef NDEBUG
void StackfulCoroutineWorker::PrintRunnables(const PandaString &requester)
{
    os::memory::LockHolder lock(runnablesLock_);
    LOG(DEBUG, COROUTINES) << "[" << requester << "] ";
    for (auto *co : runnables_) {
        LOG(DEBUG, COROUTINES) << co->GetName() << " <";
    }
    LOG(DEBUG, COROUTINES) << "X";
}
#endif

void StackfulCoroutineWorker::ThreadProc()
{
    threadId_ = os::thread::GetCurrentThreadId();
    scheduleLoopCtx_ = coroManager_->CreateEntrypointlessCoroutine(runtime_, vm_, false, "[thr_sch] " + GetName());
    scheduleLoopCtx_->GetContext<StackfulCoroutineContext>()->SetWorker(this);
    Coroutine::SetCurrent(scheduleLoopCtx_);
    scheduleLoopCtx_->RequestResume();
    scheduleLoopCtx_->NativeCodeBegin();
    coroManager_->OnWorkerStartup();

    // profiling: start interval here, end in ctxswitch
    stats_.StartInterval(CoroutineTimeStats::SCH_ALL);
    ScheduleLoopBody();

    coroManager_->DestroyEntrypointlessCoroutine(scheduleLoopCtx_);
    ASSERT(threadId_ == os::thread::GetCurrentThreadId());
    coroManager_->OnWorkerShutdown();
}

void StackfulCoroutineWorker::ScheduleLoop()
{
    LOG(DEBUG, COROUTINES) << "[" << GetName() << "] Schedule loop called!";
    ScheduleLoopBody();
}

void StackfulCoroutineWorker::ScheduleLoopBody()
{
    ScopedManagedCodeThread s(scheduleLoopCtx_);
    while (IsActive()) {
        RequestScheduleImpl();
        os::memory::LockHolder lkRunnables(runnablesLock_);
        UpdateLoadFactor();
    }
}

void StackfulCoroutineWorker::ScheduleLoopProxy(void *worker)
{
    static_cast<StackfulCoroutineWorker *>(worker)->ScheduleLoop();
}

void StackfulCoroutineWorker::PushToRunnableQueue(Coroutine *co, bool pushFront)
{
    os::memory::LockHolder lock(runnablesLock_);
    co->GetContext<StackfulCoroutineContext>()->SetWorker(this);

    if (pushFront) {
        runnables_.push_front(co);
    } else {
        runnables_.push_back(co);
    }
    UpdateLoadFactor();

    runnablesCv_.Signal();
}

Coroutine *StackfulCoroutineWorker::PopFromRunnableQueue()
{
    os::memory::LockHolder lock(runnablesLock_);
    ASSERT(!runnables_.empty());
    auto *co = runnables_.front();
    runnables_.pop_front();
    UpdateLoadFactor();
    return co;
}

bool StackfulCoroutineWorker::RunnableCoroutinesExist() const
{
    os::memory::LockHolder lock(runnablesLock_);
    return !runnables_.empty();
}

void StackfulCoroutineWorker::WaitForRunnables()
{
    // NOTE(konstanting): in case of work stealing, use timed wait and try periodically to steal some runnables
    while (!RunnableCoroutinesExist() && IsActive()) {
        // profiling: no need to profile the SLEEPING state, closing the interval
        stats_.FinishInterval(CoroutineTimeStats::SCH_ALL);
        runnablesCv_.Wait(
            &runnablesLock_);  // or timed wait? we may miss the signal in some cases (e.g. IsActive() change)...
        // profiling: reopening the interval after the sleep
        stats_.StartInterval(CoroutineTimeStats::SCH_ALL);
        if (!RunnableCoroutinesExist() && IsActive()) {
            LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::WaitForRunnables: spurious wakeup!";
        } else {
            LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::WaitForRunnables: wakeup!";
        }
    }
}

void StackfulCoroutineWorker::RequestScheduleImpl()
{
    // precondition: called within the current worker, no cross-worker calls allowed
    ASSERT(GetCurrentContext()->GetWorker() == this);
    runnablesLock_.Lock();

    // NOTE(konstanting): implement coro migration, work stealing, etc.
    ScopedNativeCodeThread n(Coroutine::GetCurrent());
    if (RunnableCoroutinesExist()) {
        SuspendCurrentCoroAndScheduleNext();
    } else {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::RequestSchedule: No runnables, starting to wait...";
        WaitForRunnables();
        runnablesLock_.Unlock();
    }
}

void StackfulCoroutineWorker::BlockCurrentCoroAndScheduleNext()
{
    // precondition: current coro is already added to the waiters_
    BlockCurrentCoro();
    // will transfer control to another coro...
    ScheduleNextCoroUnlockRunnablesWaiters();
    // ...this coro has been scheduled again: process finalization queue
    FinalizeTerminatedCoros();
}

void StackfulCoroutineWorker::SuspendCurrentCoroAndScheduleNext()
{
    SuspendCurrentCoro();
    // will transfer control to another coro...
    ScheduleNextCoroUnlockRunnables();
    // ...this coro has been scheduled again: process finalization queue
    FinalizeTerminatedCoros();
}

template <bool SUSPEND_AS_BLOCKED>
void StackfulCoroutineWorker::SuspendCurrentCoroGeneric()
{
    auto *currentCoro = Coroutine::GetCurrent();
    currentCoro->RequestSuspend(SUSPEND_AS_BLOCKED);
    if constexpr (!SUSPEND_AS_BLOCKED) {
        PushToRunnableQueue(currentCoro, false);
    }
}

void StackfulCoroutineWorker::BlockCurrentCoro()
{
    SuspendCurrentCoroGeneric<true>();
}

void StackfulCoroutineWorker::SuspendCurrentCoro()
{
    SuspendCurrentCoroGeneric<false>();
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockRunnablesWaiters()
{
    // precondition: runnable coros are present
    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = PrepareNextRunnableContextForSwitch();

    runnablesLock_.Unlock();
    waitersLock_.Unlock();

    SwitchCoroutineContext(currentCtx, nextCtx);
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockRunnables()
{
    // precondition: runnable coros are present
    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = PrepareNextRunnableContextForSwitch();

    runnablesLock_.Unlock();

    SwitchCoroutineContext(currentCtx, nextCtx);
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockNone()
{
    // precondition: runnable coros are present
    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = PrepareNextRunnableContextForSwitch();
    SwitchCoroutineContext(currentCtx, nextCtx);
}

StackfulCoroutineContext *StackfulCoroutineWorker::GetCurrentContext() const
{
    auto *co = Coroutine::GetCurrent();
    return co->GetContext<StackfulCoroutineContext>();
}

StackfulCoroutineContext *StackfulCoroutineWorker::PrepareNextRunnableContextForSwitch()
{
    // precondition: runnable coros are present
    auto *nextCtx = PopFromRunnableQueue()->GetContext<StackfulCoroutineContext>();
    nextCtx->RequestResume();
    Coroutine::SetCurrent(nextCtx->GetCoroutine());
    return nextCtx;
}

void StackfulCoroutineWorker::SwitchCoroutineContext(StackfulCoroutineContext *from, StackfulCoroutineContext *to)
{
    ASSERT(from != nullptr);
    ASSERT(to != nullptr);
    EnsureCoroutineSwitchEnabled();
    LOG(DEBUG, COROUTINES) << "Ctx switch: " << from->GetCoroutine()->GetName() << " --> "
                           << to->GetCoroutine()->GetName();
    stats_.FinishInterval(CoroutineTimeStats::SCH_ALL);
    stats_.StartInterval(CoroutineTimeStats::CTX_SWITCH);
    from->SwitchTo(to);
    stats_.FinishInterval(CoroutineTimeStats::CTX_SWITCH);
    stats_.StartInterval(CoroutineTimeStats::SCH_ALL);
}

void StackfulCoroutineWorker::FinalizeTerminatedCoros()
{
    while (!finalizationQueue_.empty()) {
        auto *f = finalizationQueue_.front();
        finalizationQueue_.pop();
        coroManager_->DestroyEntrypointfulCoroutine(f);
    }
}

void StackfulCoroutineWorker::UpdateLoadFactor()
{
    loadFactor_ = (loadFactor_ + runnables_.size()) / 2U;
}

void StackfulCoroutineWorker::EnsureCoroutineSwitchEnabled()
{
    if (IsCoroutineSwitchDisabled()) {
        LOG(FATAL, COROUTINES) << "ERROR ERROR ERROR >>> Trying to switch coroutines on " << GetName()
                               << " when coroutine switch is DISABLED!!! <<< ERROR ERROR ERROR";
        UNREACHABLE();
    }
}

}  // namespace ark
