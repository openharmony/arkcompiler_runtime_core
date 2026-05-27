/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "runtime/execution/stackless/stackless_job_worker_thread.h"
#include "runtime/execution/stackless/stackless_job_manager.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_worker_thread.h"
#include "runtime/execution/job_events.h"
#include "runtime/execution/job_priority.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"

#include "libarkbase/os/mutex.h"

#include <thread>

namespace ark {

StacklessJobWorkerThread::StacklessJobWorkerThread(Runtime *runtime, PandaVM *vm, PandaString workerName, Id id,
                                                   bool inExclusiveMode, bool isMainWorker,
                                                   JobExecutionContext *executionCtx)
    : JobWorkerThread(runtime, vm, std::move(workerName), id, inExclusiveMode, isMainWorker),
      jobManager_(static_cast<StacklessJobManager *>(vm->GetThreadManager())),
      allJobsAreExecutedEvt_(jobManager_)
{
    bool attachToExecCtx = executionCtx != nullptr;
    if (attachToExecCtx) {
        AttachExecutionContext(executionCtx);
    } else {
        std::thread t(&StacklessJobWorkerThread::ThreadProc, this);
        os::thread::SetThreadName(t.native_handle(), GetName().c_str());
        t.detach();
    }
    LOG(DEBUG, EXECUTION) << "Created a job ectx worker instance: id=" << GetId() << " name=" << GetName();
}

void StacklessJobWorkerThread::ExecuteJobs()
{
    bool hasRunnables = true;
    while (hasRunnables) {
        ProcessTimerEvents();
        hasRunnables = ExecuteRunnableJob();
    }
}

void StacklessJobWorkerThread::ExecuteJobsUntilHappened(JobEvent *awaitee)
{
    WaitForEvent(awaitee, true, false);
}

void StacklessJobWorkerThread::WaitForEvent(JobEvent *awaitee)
{
    Job::GetCurrent()->SetStatus(Job::Status::BLOCKED);
    WaitForEvent(awaitee, false, false);
    Job::GetCurrent()->SetStatus(Job::Status::RUNNING);
}

void StacklessJobWorkerThread::WaitForEvent(JobEvent *awaitee, bool executeJobs, bool signalFinalization)
{
    ASSERT_NATIVE_CODE();
    ASSERT(awaitee->GetType() == JobEvent::Type::BLOCKING);

    if (awaitee->Happened()) {
        awaitee->Unlock();
        LOG(DEBUG, COROUTINES) << "StacklessJobWorkerThread::WaitForEvent finished (no await happened)";
        return;
    }

    AddJobInWaiters(awaitee, Job::GetCurrent());

    while (true) {
        if (executeJobs) {
            ExecuteJobs();
        }

        if (signalFinalization) {
            CheckJobsExecution();
        }

        waitersLock_.Lock();
        auto evtIt = waiters_.find(awaitee);
        // NOTE(panferovi): we need to fix ABA problem here
        if (evtIt == waiters_.end()) {
            waitersLock_.Unlock();
            return;
        }
        {
            os::memory::LockHolder lh(runnablesLock_);
            waitersLock_.Unlock();
            if (runnables_.Empty() || !executeJobs) {
                WaitForRunnablesImpl();
            }
        }
        ProcessTimerEvents();
    }
}

void StacklessJobWorkerThread::AddJobInWaiters(JobEvent *blocker, Job *job)
{
    UpdateMinExpirationTime(blocker);
    os::memory::LockHolder lh(waitersLock_);
    blocker->Unlock();
    waiters_.emplace(blocker, job);
}

void StacklessJobWorkerThread::UnblockWaiters(JobEvent *blocker)
{
    os::memory::LockHolder lh(waitersLock_);
    if (auto waiter = waiters_.find(blocker); waiter != waiters_.end()) {
        auto *job = waiter->second;
        waiters_.erase(waiter);
        if (blocker->GetType() == JobEvent::Type::BLOCKING) {
            // we shouldn't add job to the runnables in this case
            runnablesCv_.Signal();
            return;
        }
        PushToRunnableQueue(job, job->GetPriority());
    }
}

void StacklessJobWorkerThread::AddRunnableJob(Job *job)
{
    ASSERT(job != nullptr);
    RegisterIncomingJob(job);
    PushToRunnableQueue(job, job->GetPriority());
}

void StacklessJobWorkerThread::AddJobAndExecute(Job *job)
{
    ASSERT(job != nullptr);
    RegisterIncomingJob(job);
    ExecuteJob(job);
}

void StacklessJobWorkerThread::AddJobWithDependency(Job *job, JobEvent *dependency)
{
    ASSERT(job != nullptr);
    ASSERT(dependency != nullptr);
    dependency->Lock();
    if (dependency->Happened()) {
        dependency->Unlock();
        AddRunnableJob(job);
        return;
    }
    RegisterIncomingJob(job);
    job->SetStatus(Job::Status::BLOCKED);
    AddJobInWaiters(dependency, job);
}

void StacklessJobWorkerThread::OnStartup()
{
    JobWorkerThread::OnStartup();
    auto execCtx = GetSchedulerExecutionCtx();
    ASSERT(execCtx != nullptr);
    GetRuntime()->GetNotificationManager()->ThreadStartEvent(execCtx);
    execCtx->GetJob()->SetStatus(Job::Status::RUNNING);
}

void StacklessJobWorkerThread::Deactivate()
{
    os::memory::LockHolder lh(runnablesLock_);
    isActive_ = false;
    runnablesCv_.Signal();
}

void StacklessJobWorkerThread::AttachExecutionContext(JobExecutionContext *executionCtx)
{
    ASSERT(executionCtx != nullptr);
    SetSchedulerExecutionCtx(executionCtx);
    executionCtx->SetWorker(this);
}

void StacklessJobWorkerThread::ThreadProc()
{
    auto *job = jobManager_->CreateJob("sch_ctx", Job::NoEntrypointInfo {}, JobPriority::DEFAULT_PRIORITY,
                                       Job::Type::SCHEDULER);
    auto *schLoopCtx = jobManager_->CreateExecutionContext(GetRuntime(), GetPandaVM(), job);
    AttachExecutionContext(schLoopCtx);

    jobManager_->OnWorkerStartup(this);

    ScheduleLoopBody();

    SetSchedulerExecutionCtx(nullptr);
    jobManager_->DestroyExecutionContext(schLoopCtx);
    jobManager_->OnWorkerShutdown(this);
}

void StacklessJobWorkerThread::ScheduleLoopBody()
{
    while (IsActive()) {
        ExecuteJobs();
        WaitForRunnables();
        UpdateLoadFactor();
    }
}

void StacklessJobWorkerThread::PushToRunnableQueue(Job *job, JobPriority priority)
{
    job->SetStatus(Job::Status::RUNNABLE);
    {
        os::memory::LockHolder lock(runnablesLock_);
        runnables_.Push(job, priority);
        UpdateLoadFactorImpl();
        runnablesCv_.Signal();
    }
    PostSchedulingTask();
}

void StacklessJobWorkerThread::RegisterIncomingJob(Job *newJob)
{
    jobManager_->RegisterJob(newJob);
}

bool StacklessJobWorkerThread::ExecuteRunnableJob()
{
    Job *nextJob = nullptr;
    {
        os::memory::LockHolder lh(runnablesLock_);
        if (nextJob = TakeNextJob(); nextJob == nullptr) {
            return false;
        }
    }
    ExecuteJob(nextJob);
    return true;
}

Job *StacklessJobWorkerThread::TakeNextJob()
{
    if (runnables_.Empty()) {
        return nullptr;
    }

    auto fromSchJob = Job::GetCurrent()->GetType() == Job::Type::SCHEDULER;

    if UNLIKELY (finalizationJob_ != nullptr && fromSchJob) {
        return std::exchange(finalizationJob_, nullptr);
    }

    auto [nextJob, priority] = runnables_.Pop();

    if LIKELY (fromSchJob || nextJob->GetType() != Job::Type::FINALIZER) {
        return nextJob;
    }

    ASSERT(finalizationJob_ == nullptr);
    // skip finalization job if current job is not a scheduler
    finalizationJob_ = nextJob;
    return TakeNextJob();
}

void StacklessJobWorkerThread::ExecuteJob(Job *job)
{
    GetSchedulerExecutionCtx()->ExecuteJob(job);
    if (job->GetStatus() == Job::Status::RUNNING) {
        job->SetStatus(Job::Status::TERMINATING);
        jobManager_->RemoveJobFromRegistry(job);
        jobManager_->DestroyJob(job);
    }
}

void StacklessJobWorkerThread::WaitForRunnables()
{
    ASSERT_NATIVE_CODE();

    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        {
            os::memory::LockHolder lh(runnablesLock_);
            if (!runnables_.Empty() || !IsActive()) {
                break;
            }
            WaitForRunnablesImpl();
        }
        ProcessTimerEvents();
    }
    LOG(DEBUG, EXECUTION) << "StacklessJobWorkerThread::WaitForRunnables: wakeup!";
}

void StacklessJobWorkerThread::WaitForRunnablesImpl()
{
    auto waitingTime = GetShortestTimerDelay();
    if (waitingTime == 0) {
        runnablesCv_.Wait(&runnablesLock_);
    } else if (waitingTime > 0) {
        runnablesCv_.TimedWait(&runnablesLock_, waitingTime);
    }
}

void StacklessJobWorkerThread::UpdateLoadFactor()
{
    os::memory::LockHolder lh(runnablesLock_);
    UpdateLoadFactorImpl();
}

void StacklessJobWorkerThread::UpdateLoadFactorImpl()
{
    loadFactor_ = (loadFactor_ + runnables_.Size()) / 2U;
}

void StacklessJobWorkerThread::CacheLocalObjectsInExecutionCtx()
{
    GetSchedulerExecutionCtx()->CacheBuiltinClasses();
    GetSchedulerExecutionCtx()->UpdateCachedObjects();
}

void StacklessJobWorkerThread::ExecuteJobsUntilIdle()
{
    ASSERT(!IsCrossWorkerCall());
    ASSERT(Job::GetCurrent()->GetType() == Job::Type::SCHEDULER ||
           Job::GetCurrent()->GetType() == Job::Type::FINALIZER ||
           ((Job::GetCurrent()->GetType() == Job::Type::MUTATOR) && !Job::GetCurrent()->HasEntrypoint()));

    allJobsAreExecutedEvt_.Lock();
    allJobsAreExecutedEvt_.SetNotHappened();
    WaitForEvent(&allJobsAreExecutedEvt_, true, true);
}

void StacklessJobWorkerThread::CompleteAllAffinedJobs()
{
    ASSERT(IsDisabledForCrossWorkersLaunch());

    bool hasAsyncWork = true;
    while (hasAsyncWork) {
        ExecuteJobsUntilIdle();
        hasAsyncWork = GetPandaVM()->RunEventLoop(EventLoopRunMode::RUN_NOWAIT);
    }
}

void StacklessJobWorkerThread::ProcessTimerEvents()
{
    auto currTime = jobManager_->GetCurrentTime();
    if (currTime < minExpirationTime_) {
        return;
    }

    PandaVector<TimerEvent *> expiredTimers;
    minExpirationTime_ = MAX_EXPIRATION_TIME;
    {
        os::memory::LockHolder lh(waitersLock_);
        currTime = jobManager_->GetCurrentTime();
        for (auto &[evt, _] : waiters_) {
            if (evt->GetType() == JobEvent::Type::TIMER) {
                auto *timerEvent = static_cast<TimerEvent *>(evt);
                timerEvent->SetCurrentTime(currTime);
                if (timerEvent->IsExpired()) {
                    expiredTimers.push_back(timerEvent);
                } else {
                    minExpirationTime_ = std::min(minExpirationTime_, timerEvent->GetExpirationTime());
                }
            }
        }
    }

    std::sort(expiredTimers.begin(), expiredTimers.end(),
              [](const TimerEvent *evt1, const TimerEvent *evt2) { return evt1->GetId() < evt2->GetId(); });
    for (auto *evt : expiredTimers) {
        evt->Happen();
    }
}

int64_t StacklessJobWorkerThread::GetShortestTimerDelay()
{
    if (minExpirationTime_ == MAX_EXPIRATION_TIME) {
        return 0;
    }
    auto currTime = jobManager_->GetCurrentTime();
    if (minExpirationTime_ <= currTime) {
        return -1;
    }
    return (minExpirationTime_ - currTime) / 1000U + 1;  // NOLINT(readability-magic-numbers)
}

void StacklessJobWorkerThread::UpdateMinExpirationTime(JobEvent *blocker)
{
    if (blocker->GetType() == JobEvent::Type::TIMER) {
        auto *timer = static_cast<TimerEvent *>(blocker);
        minExpirationTime_ = std::min(minExpirationTime_, timer->GetExpirationTime());
    }
}

void StacklessJobWorkerThread::CheckJobsExecution()
{
    auto allJobsAreExecuted = false;
    {
        os::memory::LockHolder wl(waitersLock_);
        os::memory::LockHolder rl(runnablesLock_);
        allJobsAreExecuted = (waiters_.size() == 1) && runnables_.Empty();
        ASSERT(!allJobsAreExecuted || waiters_.begin()->second == Job::GetCurrent());
        ASSERT(!waiters_.empty());
    }

    if (allJobsAreExecuted) {
        allJobsAreExecutedEvt_.Happen();
    }
}

}  // namespace ark
