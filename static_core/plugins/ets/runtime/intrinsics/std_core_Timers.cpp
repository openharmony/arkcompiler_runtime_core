/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_launch.h"
#include "intrinsics.h"
#include "libarkbase/os/mutex.h"
#include "runtime/execution/coroutines/coroutine.h"
#include "runtime/execution/job_events.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::intrinsics {

using TimerId = int32_t;

class TimerInfo final {
public:
    TimerInfo(mem::Reference *callback, int delay, bool isPeriodic, TimerId id)
        : callback_(callback), delay_(delay), isPeriodic_(isPeriodic), id_(id)
    {
    }

    NO_COPY_SEMANTIC(TimerInfo);
    NO_MOVE_SEMANTIC(TimerInfo);

    ~TimerInfo() = default;

    mem::Reference *GetCallback() const
    {
        return callback_;
    }

    int GetDelay() const
    {
        return delay_;
    }

    bool IsPeriodic() const
    {
        return isPeriodic_;
    }

    TimerId GetId() const
    {
        return id_;
    }

private:
    mem::Reference *callback_;
    const int delay_;
    const bool isPeriodic_;
    const TimerId id_;
};

class TimerTable {
public:
    bool RegisterTimer(TimerId timerId, TimerEvent *event);

    void RetireTimer(TimerId timerId);

    void DisarmTimer(TimerId timerId);

    void RemoveTimer(TimerId timerId);

    bool IsTimerDisarmed(TimerId timerId);

private:
    static constexpr TimerEvent *DISARMED_TIMER = nullptr;
    static inline TimerEvent *retiredTimer_ = reinterpret_cast<TimerEvent *>(std::numeric_limits<uintptr_t>::max());

    os::memory::Mutex lock_;
    std::unordered_map<TimerId, TimerEvent *> timers_;
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static TimerTable g_timerTable = {};
static std::atomic<TimerId> g_nextTimerId = 1;
static std::atomic<EventId> g_internalEventId = 0;

static void InvokeCallback(mem::Reference *callback)
{
    auto *mThread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread managedScope(mThread);
    auto *lambda = mThread->GetVM()->GetGlobalObjectStorage()->Get(callback);
    LambdaUtils::InvokeVoid(mThread, EtsObject::FromCoreType(lambda));
}

static bool CheckCoroutineSwitchEnabled(JobExecutionContext *executionCtx)
{
    ASSERT(executionCtx != nullptr);
    if (executionCtx->GetManager()->IsJobSwitchDisabled()) {
        ThrowEtsException(EtsExecutionContext::FromMT(executionCtx), PlatformTypes()->coreInvalidJobOperationError,
                          "setTimeout/setInterval is not allowed in global scope or during static initialization "
                          "(<cctor>). Move the call into a function invoked after initialization.");
        return false;
    }
    return true;
}

struct EntrypointParams {
    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    EntrypointParams(JobManager *jobMan, EventId eId, mem::Reference *callback, int delay, bool isPeriodic, TimerId tId)
        : event(jobMan, eId), timer(callback, delay, isPeriodic, tId)
    {
    }

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    static EntrypointParams *Create(JobManager *jobMan, EventId eId, mem::Reference *callback, int delay,
                                    bool isPeriodic, TimerId tId)
    {
        auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
        return allocator->New<EntrypointParams>(jobMan, eId, callback, delay, isPeriodic, tId);
    }

    static void Destroy(EntrypointParams *params)
    {
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(params);
    }

    TimerEvent event;  // NOLINT(misc-non-private-member-variables-in-classes)
    TimerInfo timer;   // NOLINT(misc-non-private-member-variables-in-classes)
};

void LaunchTimerCallback(EntrypointParams *epParams, TimerEvent *timerEvent, Job *job)
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    auto *jobMan = executionCtx->GetManager();

    auto deleteParams = [jobMan, job](EntrypointParams *params) {
        g_timerTable.RemoveTimer(params->timer.GetId());
        PandaVM::GetCurrent()->GetGlobalObjectStorage()->Remove(params->timer.GetCallback());
        EntrypointParams::Destroy(params);
        jobMan->DestroyJob(job);
    };

    if (!g_timerTable.RegisterTimer(epParams->timer.GetId(), timerEvent)) {
        deleteParams(epParams);
        return;
    }

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto usDelay = epParams->timer.GetDelay() * 1000U;
    timerEvent->SetExpirationTime(jobMan->GetCurrentTime() + usDelay);
    executionCtx->GetWorker()->PostSchedulingTask(epParams->timer.GetDelay());

    auto groupId = JobWorkerThreadGroup::GenerateExactWorkerId(executionCtx->GetWorker()->GetId());
    auto launchRes = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId, timerEvent});
    if (launchRes != LaunchResult::OK) {
        deleteParams(epParams);
    }
}

void TimerEntrypoint(void *param)
{
    auto *epParams = static_cast<EntrypointParams *>(param);

    auto deleteParams = [](EntrypointParams *params) {
        g_timerTable.RemoveTimer(params->timer.GetId());
        PandaVM::GetCurrent()->GetGlobalObjectStorage()->Remove(params->timer.GetCallback());
        EntrypointParams::Destroy(params);
    };

    if (g_timerTable.IsTimerDisarmed(epParams->timer.GetId())) {
        deleteParams(epParams);
        return;
    }

    InvokeCallback(epParams->timer.GetCallback());

    if (!epParams->timer.IsPeriodic()) {
        deleteParams(epParams);
        return;
    }

    g_timerTable.RetireTimer(epParams->timer.GetId());

    epParams->event.~TimerEvent();

    auto *executionCtx = JobExecutionContext::GetCurrent();
    auto *jobMan = executionCtx->GetManager();
    // Atomic with relaxed order reason: sync is not needed here
    auto eventId = g_internalEventId.fetch_add(1, std::memory_order_relaxed);
    auto *timerEvent = std::launder(new (&epParams->event) TimerEvent(jobMan, eventId));

    auto *job = executionCtx->GetJob();
    // clone job
    auto epInfo = Job::NativeEntrypointInfo {TimerEntrypoint, epParams};
    // NOLINTNEXTLINE(performance-move-const-arg)
    job = jobMan->CreateJob(job->GetName(), std::move(epInfo), job->GetPriority());
    ASSERT(job != nullptr);
    LaunchTimerCallback(epParams, timerEvent, job);
}

TimerId StdCoreRegisterTimer(EtsObject *callback, int32_t delay, uint8_t periodic)
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    if (!CheckCoroutineSwitchEnabled(executionCtx)) {
        return 0;
    }

    // Atomic with relaxed order reason: sync is not needed here
    auto eventId = g_internalEventId.fetch_add(1, std::memory_order_relaxed);
    // Atomic with relaxed order reason: sync is not needed here
    auto timerId = g_nextTimerId.fetch_add(1, std::memory_order_relaxed);
    auto isPeriodic = periodic != 0U;
    auto *jobMan = executionCtx->GetManager();
    auto *refStorage = executionCtx->GetVM()->GetGlobalObjectStorage();
    auto *callbackRef = refStorage->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    auto *epParams = EntrypointParams::Create(jobMan, eventId, callbackRef, delay, isPeriodic, timerId);
    ASSERT(epParams != nullptr);

    auto jobName = "Timer callback: " + ark::ToPandaString(timerId);
    auto epInfo = Job::NativeEntrypointInfo {TimerEntrypoint, epParams};
    // NOLINTNEXTLINE(performance-move-const-arg)
    auto *job = jobMan->CreateJob(std::move(jobName), std::move(epInfo), EtsCoroutine::TIMER_CALLBACK,
                                  Job::Type::MUTATOR, true);
    ASSERT(job != nullptr);

    LaunchTimerCallback(epParams, &epParams->event, job);
    return timerId;
}

void StdCoreClearTimer(TimerId timerId)
{
    g_timerTable.DisarmTimer(timerId);
}

bool TimerTable::RegisterTimer(TimerId timerId, TimerEvent *event)
{
    ASSERT(event != DISARMED_TIMER);
    ASSERT(event != retiredTimer_);
    os::memory::LockHolder lh(lock_);
    auto [timerIter, inserted] = timers_.insert({timerId, event});
    if (inserted) {
        return true;
    }
    if (timerIter->second == retiredTimer_) {
        timerIter->second = event;
        return true;
    }
    return false;
}

void TimerTable::RetireTimer(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    ASSERT(timerIter != timers_.end());
    if (timerIter->second != DISARMED_TIMER) {
        timerIter->second = retiredTimer_;
    }
}

void TimerTable::DisarmTimer(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    if (timerIter != timers_.end()) {
        auto &timerEvent = timerIter->second;
        if (timerEvent != DISARMED_TIMER && timerEvent != retiredTimer_) {
            timerEvent->SetExpired();
            timerEvent->Happen();
        }
        timerEvent = DISARMED_TIMER;
    }
}

void TimerTable::RemoveTimer(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    if (timerIter != timers_.end()) {
        timers_.erase(timerIter);
    }
}

bool TimerTable::IsTimerDisarmed(TimerId timerId)
{
    os::memory::LockHolder lh(lock_);
    auto timerIter = timers_.find(timerId);
    ASSERT(timerIter != timers_.end());
    return timerIter->second == DISARMED_TIMER;
}

}  // namespace ark::ets::intrinsics
