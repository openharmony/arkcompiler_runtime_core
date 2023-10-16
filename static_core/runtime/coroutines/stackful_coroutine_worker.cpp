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

#include "runtime/include/thread_scopes.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"

namespace panda {

StackfulCoroutineWorker::StackfulCoroutineWorker(Runtime *runtime, PandaVM *vm, StackfulCoroutineManager *coro_manager,
                                                 ScheduleLoopType type, PandaString name)
    : coro_manager_(coro_manager), id_(os::thread::GetCurrentThreadId()), name_(std::move(name))
{
    if (type == ScheduleLoopType::THREAD) {
        schedule_loop_ctx_ = coro_manager->CreateEntrypointlessCoroutine(runtime, vm, /*make_current*/ false);
        std::thread t(&StackfulCoroutineWorker::ThreadProc, this);
        os::thread::SetThreadName(t.native_handle(), name_.c_str());
        t.detach();
    } else {
        schedule_loop_ctx_ = coro_manager->CreateNativeCoroutine(runtime, vm, ScheduleLoopProxy, this);
        PushToRunnableQueue(schedule_loop_ctx_);
    }
}

void StackfulCoroutineWorker::AddRunnableCoroutine(Coroutine *new_coro, bool prioritize)
{
    PushToRunnableQueue(new_coro, prioritize);
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

    waiters_lock_.Lock();
    awaitee->Unlock();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::AddWaitingCoroutine: " << waiter->GetName() << " AWAITS";
    waiters_.insert({awaitee, waiter});

    runnables_lock_.Lock();
    ASSERT(RunnableCoroutinesExist());
    ScopedNativeCodeThread n(Coroutine::GetCurrent());
    // will unlock waiters_lock_ and switch ctx
    BlockCurrentCoroAndScheduleNext();

    return true;
}

void StackfulCoroutineWorker::UnblockWaiters(CoroutineEvent *blocker)
{
    os::memory::LockHolder lock(waiters_lock_);
    auto w = waiters_.find(blocker);
    if (w != waiters_.end()) {
        auto *coro = w->second;
        waiters_.erase(w);
        coro->RequestUnblock();
        PushToRunnableQueue(coro);
    }
}

void StackfulCoroutineWorker::RequestFinalization(Coroutine *finalizee)
{
    // precondition: current coro and finalizee belong to the current worker
    ASSERT(finalizee->GetContext<StackfulCoroutineContext>()->GetWorker() == this);
    ASSERT(GetCurrentContext()->GetWorker() == this);

    finalization_queue_.push(finalizee);
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
        ASSERT(schedule_loop_ctx_->HasNativeEntrypoint());
        runnables_lock_.Lock();
        // sch loop only
        ASSERT(runnables_.size() == 1);
        SuspendCurrentCoroAndScheduleNext();
    }
}

void StackfulCoroutineWorker::DisableCoroutineSwitch()
{
    ++disable_coro_switch_counter_;
    LOG(DEBUG, COROUTINES) << "Coroutine switch on " << GetName()
                           << " has been disabled! Recursive ctr = " << disable_coro_switch_counter_;
}

void StackfulCoroutineWorker::EnableCoroutineSwitch()
{
    ASSERT(IsCoroutineSwitchDisabled());
    --disable_coro_switch_counter_;
    LOG(DEBUG, COROUTINES) << "Coroutine switch on " << GetName()
                           << " has been enabled! Recursive ctr = " << disable_coro_switch_counter_;
}

bool StackfulCoroutineWorker::IsCoroutineSwitchDisabled()
{
    return disable_coro_switch_counter_ > 0;
}

#ifndef NDEBUG
void StackfulCoroutineWorker::PrintRunnables(const PandaString &requester)
{
    os::memory::LockHolder lock(runnables_lock_);
    LOG(DEBUG, COROUTINES) << "[" << requester << "] ";
    for (auto *co : runnables_) {
        LOG(DEBUG, COROUTINES) << co->GetName() << " <";
    }
    LOG(DEBUG, COROUTINES) << "X";
}
#endif

void StackfulCoroutineWorker::ThreadProc()
{
    id_ = os::thread::GetCurrentThreadId();
    schedule_loop_ctx_->GetContext<StackfulCoroutineContext>()->SetWorker(this);
    Coroutine::SetCurrent(schedule_loop_ctx_);
    schedule_loop_ctx_->RequestResume();
    schedule_loop_ctx_->NativeCodeBegin();
    ScheduleLoopBody();
    coro_manager_->DestroyEntrypointlessCoroutine(schedule_loop_ctx_);

    ASSERT(id_ == os::thread::GetCurrentThreadId());
    coro_manager_->OnWorkerShutdown();
}

void StackfulCoroutineWorker::ScheduleLoop()
{
    LOG(DEBUG, COROUTINES) << "[" << GetName() << "] Schedule loop called!";
    ScheduleLoopBody();
}

void StackfulCoroutineWorker::ScheduleLoopBody()
{
    ScopedManagedCodeThread s(schedule_loop_ctx_);
    while (IsActive()) {
        RequestScheduleImpl();
        os::memory::LockHolder lk_runnables(runnables_lock_);
        UpdateLoadFactor();
    }
}

void StackfulCoroutineWorker::ScheduleLoopProxy(void *worker)
{
    static_cast<StackfulCoroutineWorker *>(worker)->ScheduleLoop();
}

void StackfulCoroutineWorker::PushToRunnableQueue(Coroutine *co, bool push_front)
{
    os::memory::LockHolder lock(runnables_lock_);
    co->GetContext<StackfulCoroutineContext>()->SetWorker(this);

    if (push_front) {
        runnables_.push_front(co);
    } else {
        runnables_.push_back(co);
    }
    UpdateLoadFactor();

    runnables_cv_.Signal();
}

Coroutine *StackfulCoroutineWorker::PopFromRunnableQueue()
{
    os::memory::LockHolder lock(runnables_lock_);
    ASSERT(!runnables_.empty());
    auto *co = runnables_.front();
    runnables_.pop_front();
    UpdateLoadFactor();
    return co;
}

bool StackfulCoroutineWorker::RunnableCoroutinesExist() const
{
    os::memory::LockHolder lock(runnables_lock_);
    return !runnables_.empty();
}

void StackfulCoroutineWorker::WaitForRunnables()
{
    // TODO(konstanting): in case of work stealing, use timed wait and try periodically to steal some runnables
    while (!RunnableCoroutinesExist() && IsActive()) {
        runnables_cv_.Wait(
            &runnables_lock_);  // or timed wait? we may miss the signal in some cases (e.g. IsActive() change)...
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
    runnables_lock_.Lock();

    // TODO(konstanting): implement coro migration, work stealing, etc.
    ScopedNativeCodeThread n(Coroutine::GetCurrent());
    if (RunnableCoroutinesExist()) {
        SuspendCurrentCoroAndScheduleNext();
    } else {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::RequestSchedule: No runnables, starting to wait...";
        WaitForRunnables();
        runnables_lock_.Unlock();
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
    auto *current_coro = Coroutine::GetCurrent();
    current_coro->RequestSuspend(SUSPEND_AS_BLOCKED);
    if constexpr (!SUSPEND_AS_BLOCKED) {
        PushToRunnableQueue(current_coro, false);
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
    auto *current_ctx = GetCurrentContext();
    auto *next_ctx = PrepareNextRunnableContextForSwitch();

    runnables_lock_.Unlock();
    waiters_lock_.Unlock();

    SwitchCoroutineContext(current_ctx, next_ctx);
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockRunnables()
{
    // precondition: runnable coros are present
    auto *current_ctx = GetCurrentContext();
    auto *next_ctx = PrepareNextRunnableContextForSwitch();

    runnables_lock_.Unlock();

    SwitchCoroutineContext(current_ctx, next_ctx);
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockNone()
{
    // precondition: runnable coros are present
    auto *current_ctx = GetCurrentContext();
    auto *next_ctx = PrepareNextRunnableContextForSwitch();
    SwitchCoroutineContext(current_ctx, next_ctx);
}

StackfulCoroutineContext *StackfulCoroutineWorker::GetCurrentContext() const
{
    auto *co = Coroutine::GetCurrent();
    return co->GetContext<StackfulCoroutineContext>();
}

StackfulCoroutineContext *StackfulCoroutineWorker::PrepareNextRunnableContextForSwitch()
{
    // precondition: runnable coros are present
    auto *next_ctx = PopFromRunnableQueue()->GetContext<StackfulCoroutineContext>();
    next_ctx->RequestResume();
    Coroutine::SetCurrent(next_ctx->GetCoroutine());
    return next_ctx;
}

void StackfulCoroutineWorker::SwitchCoroutineContext(StackfulCoroutineContext *from, StackfulCoroutineContext *to)
{
    ASSERT(from != nullptr);
    ASSERT(to != nullptr);
    EnsureCoroutineSwitchEnabled();
    from->SwitchTo(to);
}

void StackfulCoroutineWorker::FinalizeTerminatedCoros()
{
    while (!finalization_queue_.empty()) {
        auto *f = finalization_queue_.front();
        finalization_queue_.pop();
        coro_manager_->DestroyEntrypointfulCoroutine(f);
    }
}

void StackfulCoroutineWorker::UpdateLoadFactor()
{
    load_factor_ = (load_factor_ + runnables_.size()) / 2;
}

void StackfulCoroutineWorker::EnsureCoroutineSwitchEnabled()
{
    if (IsCoroutineSwitchDisabled()) {
        LOG(FATAL, COROUTINES) << "ERROR ERROR ERROR >>> Trying to switch coroutines on " << GetName()
                               << " when coroutine switch is DISABLED!!! <<< ERROR ERROR ERROR";
        UNREACHABLE();
    }
}

}  // namespace panda