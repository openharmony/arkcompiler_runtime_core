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

#include "libarkbase/os/mutex.h"

#include <thread>

namespace ark {

StacklessJobWorkerThread::StacklessJobWorkerThread(Runtime *runtime, PandaVM *vm, PandaString workerName, Id id,
                                                   bool inExclusiveMode, bool isMainWorker,
                                                   JobExecutionContext *executionCtx)
    : JobWorkerThread(runtime, vm, std::move(workerName), id, inExclusiveMode, isMainWorker),
      jobManager_(static_cast<StacklessJobManager *>(vm->GetThreadManager()))
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
    while (ExecuteRunnableJob()) {
    }
}

void StacklessJobWorkerThread::WaitForEvent(JobEvent *awaitee)
{
    ASSERT(awaitee->GetType() == JobEvent::Type::BLOCKING);

    if (awaitee->Happened()) {
        awaitee->Unlock();
        LOG(DEBUG, COROUTINES) << "StacklessJobWorkerThread::WaitForEvent finished (no await happened)";
        return;
    }

    {
        os::memory::LockHolder lh(waitersLock_);
        awaitee->Unlock();
        waiters_.emplace(awaitee, Job::GetCurrent());
    }

    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        waitersLock_.Lock();
        auto evtIt = waiters_.find(awaitee);
        // NOTE(panferovi): we need to fix ABA problem here
        if (evtIt == waiters_.end()) {
            waitersLock_.Unlock();
            return;
        }
        os::memory::LockHolder lh(runnablesLock_);
        waitersLock_.Unlock();
        runnablesCv_.Wait(&runnablesLock_);
    }
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

bool StacklessJobWorkerThread::EventIsInPendingQueue(JobEvent *event)
{
    os::memory::LockHolder lh(waitersLock_);
    auto evtIt = waiters_.find(event);
    // NOTE(panferovi): we need to fix ABA problem here
    if (evtIt != waiters_.end()) {
        return true;
    }
    return false;
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
    LOG_IF(dependency->GetType() != JobEvent::Type::GENERIC, FATAL, EXECUTION)
        << "Currently only GenericEvent is supported";
    dependency->Lock();
    if (dependency->Happened()) {
        dependency->Unlock();
        AddRunnableJob(job);
        return;
    }
    os::memory::LockHolder lh(waitersLock_);
    dependency->Unlock();
    RegisterIncomingJob(job);
    job->SetStatus(Job::Status::BLOCKED);
    waiters_.emplace(dependency, job);
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
    auto *schLoopCtx = jobManager_->CreateEntrypointlessExecCtx(GetRuntime(), GetPandaVM(), true, "sch_ctx",
                                                                Job::Type::SCHEDULER, JobPriority::MEDIUM_PRIORITY);
    AttachExecutionContext(schLoopCtx);

    jobManager_->OnWorkerStartup(this);

    ScheduleLoopBody();

    SetSchedulerExecutionCtx(nullptr);
    jobManager_->DestroyEntrypointlessExecCtx(schLoopCtx);
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
    os::memory::LockHolder lock(runnablesLock_);
    runnables_.Push(job, priority);
    UpdateLoadFactorImpl();
    runnablesCv_.Signal();
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
        if (runnables_.Empty()) {
            return false;
        }
        nextJob = runnables_.Pop().first;
    }
    ExecuteJob(nextJob);
    return true;
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
    os::memory::LockHolder lh(runnablesLock_);
    while (runnables_.Empty() && IsActive()) {
        runnablesCv_.Wait(&runnablesLock_);
    }
    LOG(DEBUG, EXECUTION) << "StacklessJobWorkerThread::WaitForRunnables: wakeup!";
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

}  // namespace ark
