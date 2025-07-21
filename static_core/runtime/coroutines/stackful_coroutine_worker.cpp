/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "libpandabase/os/time.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"

namespace ark {

StackfulCoroutineWorker::StackfulCoroutineWorker(Runtime *runtime, PandaVM *vm, StackfulCoroutineManager *coroManager,
                                                 ScheduleLoopType type, PandaString name, size_t id)
    : CoroutineWorker(runtime, vm),
      coroManager_(coroManager),
      threadId_(os::thread::GetCurrentThreadId()),
      workerCompletionEvent_(coroManager),
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
        scheduleLoopCtx_ = coroManager->CreateNativeCoroutine(GetRuntime(), GetPandaVM(), ScheduleLoopProxy, this,
                                                              "[fiber_sch] " + GetName(), Coroutine::Type::SCHEDULER,
                                                              CoroutinePriority::MEDIUM_PRIORITY);
        AddRunnableCoroutine(scheduleLoopCtx_);
    }
}

void StackfulCoroutineWorker::AddRunnableCoroutine(Coroutine *newCoro)
{
    ASSERT(newCoro != nullptr);
    os::memory::LockHolder lock(runnablesLock_);
    PushToRunnableQueue(newCoro, newCoro->GetPriority());
    RegisterIncomingActiveCoroutine(newCoro);
}

void StackfulCoroutineWorker::AddRunningCoroutine(Coroutine *newCoro)
{
    ASSERT(newCoro != nullptr);
    RegisterIncomingActiveCoroutine(newCoro);
}

void StackfulCoroutineWorker::AddCreatedCoroutineAndSwitchToIt(Coroutine *newCoro)
{
    // precondition: called within the current worker, no cross-worker calls allowed
    ASSERT(GetCurrentContext()->GetWorker() == this);

    auto *coro = Coroutine::GetCurrent();
    ScopedNativeCodeThread n(coro);
    coro->RequestSuspend(false);

    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = newCoro->GetContext<StackfulCoroutineContext>();
    nextCtx->RequestResume();
    Coroutine::SetCurrent(newCoro);
    RegisterIncomingActiveCoroutine(newCoro);

    SwitchCoroutineContext(currentCtx, nextCtx);

    // process finalization queue once this coro gets scheduled again
    FinalizeTerminatedCoros();
}

void StackfulCoroutineWorker::WaitForEvent(CoroutineEvent *awaitee)
{
    // precondition: this method is not called by the schedule loop coroutine

    Coroutine *waiter = Coroutine::GetCurrent();
    ASSERT(GetCurrentContext()->GetWorker() == this);
    ASSERT(awaitee != nullptr);
    ASSERT(!awaitee->Happened());
    ASSERT(waiter->IsInNativeCode());

    waitersLock_.Lock();
    awaitee->Unlock();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::AddWaitingCoroutine: " << waiter->GetName() << " AWAITS";
    [[maybe_unused]] auto [_, inserted] = waiters_.insert({awaitee, waiter});
    ASSERT(inserted);

    runnablesLock_.Lock();
    ASSERT(RunnableCoroutinesExist());
    // will unlock waiters_lock_ and switch ctx.
    // NB! If migration on await is enabled, current coro can migrate to another worker, so
    // IsCrossWorkerCall() will become true after resume!
    BlockCurrentCoroAndScheduleNext();
}

void StackfulCoroutineWorker::UnblockWaiters(CoroutineEvent *blocker)
{
    bool canMigrateAwait = coroManager_->IsMigrateAwakenedCorosEnabled() && !IsMainWorker() && !InExclusiveMode();
    Coroutine *unblockedCoro = nullptr;
    {
        os::memory::LockHolder lockW(waitersLock_);
        auto w = waiters_.find(blocker);
        if (w != waiters_.end()) {
            unblockedCoro = w->second;
            waiters_.erase(w);
            if (!canMigrateAwait) {
                os::memory::LockHolder lockR(runnablesLock_);
                unblockedCoro->RequestUnblock();
                PushToRunnableQueue(unblockedCoro, unblockedCoro->GetPriority());
            } else {
                // (wangyuzhong,#24880): in case of IsMigrateAwakenedCorosEnabled() we need to correctly issue the
                // external scheduler request from the correct worker. Here the coroutine becomes Active on one
                // worker(which causes the request to be sent) and then gets potentially transferred to another worker.
                unblockedCoro->RequestUnblock();
            }
        }
    }
    if (unblockedCoro == nullptr) {
        LOG(DEBUG, COROUTINES) << "The coroutine is nullptr.";
        return;
    }
    if (canMigrateAwait) {
        coroManager_->MigrateAwakenedCoro(unblockedCoro);
    }
    if (IsDisabledForCrossWorkersLaunch()) {
        workerCompletionEvent_.Happen();
    }
}

void StackfulCoroutineWorker::RequestFinalization(Coroutine *finalizee)
{
    // precondition: current coro and finalizee belong to the current worker
    ASSERT(finalizee->GetWorker() == this);
    ASSERT(GetCurrentContext()->GetWorker() == this);

    finalizationQueue_.push(finalizee);
    // finalizee will never be scheduled again
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
        ASSERT(runnables_.Size() == 1);
        SuspendCurrentCoroAndScheduleNext();
        ASSERT(!IsCrossWorkerCall());
    }
}

void StackfulCoroutineWorker::CompleteAllAffinedCoroutines()
{
    ASSERT_NATIVE_CODE();
    ASSERT(GetCurrentContext()->GetWorker() == this);
    ASSERT(IsDisabledForCrossWorkersLaunch());

    // CC-OFFNXT(G.FMT.04-CPP): project code style
    auto lock = [](auto &&...locks) { ([&]() NO_THREAD_SAFETY_ANALYSIS { locks.Lock(); }(), ...); };
    // CC-OFFNXT(G.FMT.04-CPP): project code style
    auto unlock = [](auto &&...locks) { ([&]() NO_THREAD_SAFETY_ANALYSIS { locks.Unlock(); }(), ...); };

    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        lock(waitersLock_, runnablesLock_);
        if (runnables_.Size() > 1) {
            unlock(waitersLock_, runnablesLock_);
            coroManager_->Schedule();
        } else if (!waiters_.empty()) {
            workerCompletionEvent_.SetNotHappened();
            workerCompletionEvent_.Lock();
            unlock(waitersLock_, runnablesLock_);
            coroManager_->Await(&workerCompletionEvent_);
        } else {
            unlock(waitersLock_, runnablesLock_);
            break;
        }
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
    runnables_.IterateOverCoroutines([](Coroutine *co) { LOG(DEBUG, COROUTINES) << co->GetName() << " <"; });
    LOG(DEBUG, COROUTINES) << "X";
}
#endif

size_t StackfulCoroutineWorker::GetRunnablesCount(Coroutine::Type type)
{
    os::memory::LockHolder lock(runnablesLock_);
    size_t runnablesCount = 0;
    runnables_.IterateOverCoroutines([type, &runnablesCount](Coroutine *coro) {
        if (coro->GetType() == type) {
            runnablesCount++;
        }
    });
    return runnablesCount;
}

void StackfulCoroutineWorker::ThreadProc()
{
    threadId_ = os::thread::GetCurrentThreadId();
    scheduleLoopCtx_ =
        coroManager_->CreateEntrypointlessCoroutine(GetRuntime(), GetPandaVM(), false, "[thr_sch] " + GetName(),
                                                    Coroutine::Type::SCHEDULER, CoroutinePriority::MEDIUM_PRIORITY);
    Coroutine::SetCurrent(scheduleLoopCtx_);
    scheduleLoopCtx_->RequestResume();
    AddRunningCoroutine(scheduleLoopCtx_);
    scheduleLoopCtx_->NativeCodeBegin();
    coroManager_->OnWorkerStartup(this);

    // profiling: start interval here, end in ctxswitch
    stats_.StartInterval(CoroutineTimeStats::SCH_ALL);
    ScheduleLoopBody();

    coroManager_->DestroyEntrypointlessCoroutine(scheduleLoopCtx_);
    ASSERT(threadId_ == os::thread::GetCurrentThreadId());
    coroManager_->OnWorkerShutdown(this);
}

void StackfulCoroutineWorker::ScheduleLoop()
{
    LOG(DEBUG, COROUTINES) << "[" << GetName() << "] Schedule loop called!";
    ScheduleLoopBody();
}

void StackfulCoroutineWorker::ScheduleLoopBody()
{
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

void StackfulCoroutineWorker::PushToRunnableQueue(Coroutine *co, CoroutinePriority priority)
{
    runnables_.Push(co, priority);
    UpdateLoadFactor();
    runnablesCv_.Signal();
}

Coroutine *StackfulCoroutineWorker::PopFromRunnableQueue()
{
    os::memory::LockHolder lock(runnablesLock_);
    ASSERT(!runnables_.Empty());
    auto [co, _] = runnables_.Pop();
    UpdateLoadFactor();
    return co;
}

bool StackfulCoroutineWorker::RunnableCoroutinesExist() const
{
    os::memory::LockHolder lock(runnablesLock_);
    return !runnables_.Empty();
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

void StackfulCoroutineWorker::RegisterIncomingActiveCoroutine(Coroutine *newCoro)
{
    ASSERT(newCoro != nullptr);
    newCoro->SetWorker(this);
    auto canMigrate = newCoro->GetContext<StackfulCoroutineContext>()->IsMigrationAllowed();
    newCoro->LinkToExternalHolder(IsMainWorker() && !canMigrate);
}

void StackfulCoroutineWorker::RequestScheduleImpl()
{
    // precondition: called within the current worker, no cross-worker calls allowed
    ASSERT(GetCurrentContext()->GetWorker() == this);
    ASSERT_NATIVE_CODE();
    runnablesLock_.Lock();

    // NOTE(konstanting): implement coro migration, work stealing, etc.
    if (RunnableCoroutinesExist()) {
        SuspendCurrentCoroAndScheduleNext();
        ASSERT(!IsCrossWorkerCall() || (Coroutine::GetCurrent()->GetType() == Coroutine::Type::MUTATOR));
    } else {
        coroManager_->TriggerMigration();
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::RequestSchedule: No runnables, starting to wait...";
        WaitForRunnables();
        runnablesLock_.Unlock();
    }
}

void StackfulCoroutineWorker::BlockCurrentCoroAndScheduleNext()
{
    // precondition: current coro is already added to the waiters_
    BlockCurrentCoro();
    // will transfer control to another coro... Can change current coroutine's host worker!
    ScheduleNextCoroUnlockRunnablesWaiters();
    // ...this coro has been scheduled again: process finalization queue
    if (!IsCrossWorkerCall()) {
        FinalizeTerminatedCoros();
    } else {
        // migration happened!
    }
}

void StackfulCoroutineWorker::SuspendCurrentCoroAndScheduleNext()
{
    // will transfer control to another coro... Can change current coroutine's host worker!
    SuspendCurrentCoro();
    // will transfer control to another coro...
    ScheduleNextCoroUnlockRunnables();
    // ...this coro has been scheduled again: process finalization queue
    if (!IsCrossWorkerCall()) {
        FinalizeTerminatedCoros();
    } else {
        // migration happened!
    }
}

template <bool SUSPEND_AS_BLOCKED>
void StackfulCoroutineWorker::SuspendCurrentCoroGeneric()
{
    auto *currentCoro = Coroutine::GetCurrent();
    ASSERT(currentCoro != nullptr);
    currentCoro->RequestSuspend(SUSPEND_AS_BLOCKED);
    if constexpr (!SUSPEND_AS_BLOCKED) {
        os::memory::LockHolder lock(runnablesLock_);
        PushToRunnableQueue(currentCoro, currentCoro->GetPriority());
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
    ASSERT(co != nullptr);
    return co->GetContext<StackfulCoroutineContext>();
}

StackfulCoroutineContext *StackfulCoroutineWorker::PrepareNextRunnableContextForSwitch()
{
    // precondition: runnable coros are present
    ASSERT(Coroutine::GetCurrent() != nullptr);
    auto *il = Coroutine::GetCurrent()->ReleaseImmediateLauncher();
    auto *nextCtx = il != nullptr ? il->GetContext<StackfulCoroutineContext>()
                                  : PopFromRunnableQueue()->GetContext<StackfulCoroutineContext>();
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
    OnContextSwitch();
    stats_.StartInterval(CoroutineTimeStats::CTX_SWITCH);
    from->SwitchTo(to);
    if (IsCrossWorkerCall()) {
        ASSERT(Coroutine::GetCurrent()->GetType() == Coroutine::Type::MUTATOR);
        // Here this != current coroutine's worker. The rest of this function will be executed CONCURRENTLY!
        // NOTE(konstanting): need to correctly handle stats_ update here
        return;
    }
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
    loadFactor_ = (loadFactor_ + runnables_.Size()) / 2U;
}

void StackfulCoroutineWorker::EnsureCoroutineSwitchEnabled()
{
    if (IsCoroutineSwitchDisabled()) {
        LOG(FATAL, COROUTINES) << "ERROR ERROR ERROR >>> Trying to switch coroutines on " << GetName()
                               << " when coroutine switch is DISABLED!!! <<< ERROR ERROR ERROR";
        UNREACHABLE();
    }
}

void StackfulCoroutineWorker::MigrateTo(StackfulCoroutineWorker *to)
{
    os::memory::LockHolder fromLock(runnablesLock_);
    size_t migrateCount = runnables_.Size() / 2;  // migrate up to half of runnable coroutines
    if (migrateCount == 0) {
        LOG(DEBUG, COROUTINES) << "The blocked worker does not have runnable coroutines.";
        return;
    }

    os::memory::LockHolder toLock(to->runnablesLock_);
    MigrateCoroutinesImpl(to, migrateCount);
}

bool StackfulCoroutineWorker::MigrateFrom(StackfulCoroutineWorker *from)
{
    os::memory::LockHolder toLock(runnablesLock_);
    if (!IsIdle()) {
        LOG(DEBUG, COROUTINES) << "The worker is not idle.";
        return false;
    }

    os::memory::LockHolder fromLock(from->runnablesLock_);
    size_t migrateCount = from->runnables_.Size() / 2;  // migrate up to half of runnable coroutines
    if (migrateCount == 0) {
        LOG(DEBUG, COROUTINES) << "The target worker does not have runnable coroutines.";
        return true;
    }

    from->MigrateCoroutinesImpl(this, migrateCount);
    return true;
}

void StackfulCoroutineWorker::MigrateCoroutinesImpl(StackfulCoroutineWorker *to, size_t migrateCount)
{
    using CIterator = PriorityQueue::CIterator;
    PandaVector<CIterator> migratedCoros;
    runnables_.VisitCoroutines([&migratedCoros, &migrateCount, this, to](auto begin, auto end) {
        for (; migrateCount > 0 && begin != end; ++begin) {
            // not migrate SCHEDULER coroutine and FINALIZER coroutine
            if ((*begin)->GetType() != Coroutine::Type::MUTATOR) {
                continue;
            }
            auto maskValue = (*begin)->template GetContext<StackfulCoroutineContext>()->GetAffinityMask();
            std::bitset<stackful_coroutines::MAX_WORKERS_COUNT> affinityBits(maskValue);
            if (affinityBits.test(to->GetId())) {
                LOG(DEBUG, COROUTINES) << "migrate coro " << (*begin)->GetCoroutineId() << " from " << GetId() << " to "
                                       << to->GetId();
                to->AddRunnableCoroutine(*(*begin));
                migratedCoros.push_back(begin);
                --migrateCount;
            }
        }
    });
    runnables_.RemoveCoroutines(migratedCoros);
}

bool StackfulCoroutineWorker::IsIdle()
{
    return GetRunnablesCount(Coroutine::Type::MUTATOR) == 0;
}

void StackfulCoroutineWorker::MigrateCorosOutwardsIfBlocked()
{
    if (!IsPotentiallyBlocked()) {
        return;
    }
    coroManager_->MigrateCoroutinesOutward(this);
}

bool StackfulCoroutineWorker::IsPotentiallyBlocked()
{
    os::memory::LockHolder lock(runnablesLock_);
    if (runnables_.Empty() || lastCtxSwitchTimeMillis_ == 0) {
        return false;
    }
    if ((ark::os::time::GetClockTimeInMilli() - lastCtxSwitchTimeMillis_) >= MAX_EXECUTION_DURATION) {
        LOG(DEBUG, COROUTINES) << "The current coroutine has been executed more than 6s.";
        return true;
    }
    return false;
}

void StackfulCoroutineWorker::OnContextSwitch()
{
    lastCtxSwitchTimeMillis_ = ark::os::time::GetClockTimeInMilli();
}

}  // namespace ark
