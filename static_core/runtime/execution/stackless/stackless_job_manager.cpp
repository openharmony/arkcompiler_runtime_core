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

#include "runtime/execution/stackless/stackless_job_manager.h"
#include "runtime/execution/stackless/stackless_job_worker_thread.h"
#include "runtime/execution/affinity_mask.h"
#include "runtime/execution/job_worker_group.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_launch.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_priority.h"
#include "runtime/execution/job_events.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"

#include "libarkbase/os/mutex.h"

namespace ark {

StacklessJobManager::StacklessJobManager(const JobManagerConfig &config, JobExecCtxFactory factory)
    : JobManager(), config_(config), execCtxFactory_(factory), programCompletionEvent_(this)
{
}

void StacklessJobManager::InitializeScheduler(Runtime *runtime, PandaVM *vm)
{
    InitializeWorkerIdAllocator();
    CreateMainExecutionContext(runtime, vm);
    CreateGeneralWorkers(COMMON_WORKERS_COUNT, runtime, vm);
    LOG(DEBUG, EXECUTION) << "StacklessJobManager(): successfully created and activated " << GetExistingWorkersCount()
                          << " job execution contexts workers";
}

void StacklessJobManager::InitializeManagedStructures(const JobWorkerThread::CreatePluginObjFunc &createEtsObj)
{
    ASSERT_NATIVE_CODE();
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx == GetMainThread());

    ScopedManagedCodeThread msc(executionCtx);
    PandaVector<PandaVector<JobWorkerThread::LocalObjectData>> eachWorkerLocalObjects {};

    os::memory::LockHolder l {workersLock_};

    auto numWorkers = workers_.size();
    workersLock_.Unlock();

    eachWorkerLocalObjects.reserve(numWorkers);
    for (size_t i = 0; i < numWorkers; i++) {
        eachWorkerLocalObjects.push_back(JobWorkerThread::CreateWorkerLocalObjects(createEtsObj));
    }
    workersLock_.Lock();
    ASSERT(numWorkers == workers_.size());

    int i = 0;
    for (auto *w : workers_) {
        w->InitWorkerLocalObjects(std::move(eachWorkerLocalObjects[i++]));
        w->CacheLocalObjectsInExecutionCtx();
    }
    executionCtx->UpdateCachedObjects();
}

void StacklessJobManager::WaitForDeregistration()
{
    // precondition: MAIN is already in the native mode
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::WaitForDeregistation(): STARTED";
    GetCurrentWorker()->DestroyCallbackPoster();
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::WaitForDeregistation(): waiting for other jobs to complete";
    // block till only schedule loop jobs are present
    WaitForMutatorJobsCompletion();

    LOG(DEBUG, EXECUTION) << "StacklessJobManager::WaitForDeregistation(): stopping workers";
    {
        os::memory::LockHolder lock(workersLock_);
        for (auto *worker : workers_) {
            worker->Deactivate();
        }
        while (activeWorkersCount_ > 1) {  // 1 is for MAIN
            workersCv_.Wait(&workersLock_);
        }
    }

    LOG(DEBUG, EXECUTION) << "StacklessJobManager::WaitForDeregistation(): stopping await loop on the main worker";

    // MAIN finished, all workers are deleted, no active jobs remain
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::WaitForDeregistation(): DONE";
}

void StacklessJobManager::Finalize()
{
    DestroyMainExecutionContext();
}

void StacklessJobManager::RegisterExecutionContext(JobExecutionContext *executionCtx)
{
    os::memory::LockHolder lock(jobCtxListLock_);
    jobExecCtxs_.insert(executionCtx);
    RegisterJob(executionCtx->GetJob());
    executionCtx->GetVM()->GetGC()->OnMutatorCreate(executionCtx);
}

bool StacklessJobManager::TerminateExecutionContext(JobExecutionContext *executionCtx)
{
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::TerminateExecutionContext() started";
    executionCtx->NativeCodeEnd();
    executionCtx->UpdateStatus(MutatorStatus::TERMINATING);
    {
        os::memory::LockHolder lList(jobCtxListLock_);
        jobExecCtxs_.erase(executionCtx);

        executionCtx->CollectTLABMetrics();
        executionCtx->ClearTLAB();

        executionCtx->DestroyInternalResources(mem::MutatorUnregistrationMode::UNREGISTER);
        executionCtx->UpdateStatus(MutatorStatus::FINISHED);
    }
    Runtime::GetCurrent()->GetNotificationManager()->ThreadEndEvent(executionCtx);

    RemoveJobFromRegistry(executionCtx->GetJob());
    return false;
}

LaunchResult StacklessJobManager::Launch(Job *job, const LaunchParams &params)
{
    os::memory::LockHolder lh(workersLock_);
    auto [w, affinityMask] = ChooseWorkerForJob(params);
    if UNLIKELY (w == nullptr) {
        DestroyJob(job);
        return LaunchResult::NO_SUITABLE_WORKER;
    }
    job->SetAffinityMask(affinityMask);

    if (params.launchImmediately) {
        workersLock_.Unlock();
        w->AddJobAndExecute(job);
        workersLock_.Lock();
        return LaunchResult::OK;
    }
    if (params.startEvent != nullptr) {
        w->AddJobWithDependency(job, params.startEvent);
        return LaunchResult::OK;
    }
    w->AddRunnableJob(job);
    return LaunchResult::OK;
}

void StacklessJobManager::ExecuteJobs()
{
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::ExecuteJobs() request from " << Job::GetCurrent()->GetName();
    GetCurrentWorker()->ExecuteJobs();
}

void StacklessJobManager::Await(JobEvent *awaitee) RELEASE(awaitee)
{
    ASSERT(awaitee != nullptr);

    [[maybe_unused]] auto *waiter = Job::GetCurrent();
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::Await started by " + waiter->GetName();

    GetCurrentWorker()->WaitForEvent(awaitee);
    // NB: at this point the awaitee should not be deleted
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::Await finished by " + waiter->GetName();
}

void StacklessJobManager::UnblockWaiters(JobEvent *blocker)
{
#ifndef NDEBUG
    {
        os::memory::LockHolder lkBlocker(*blocker);
        ASSERT(blocker->Happened());
    }
#endif

    os::memory::LockHolder lkWorkers(workersLock_);
    for (auto *w : workers_) {
        w->UnblockWaiters(blocker);
    }
}

void StacklessJobManager::AwaitAsynchronous(JobEvent *awaitee)
{
    ASSERT(awaitee != nullptr);
    GetCurrentWorker()->AddJobWithDependency(Job::GetCurrent(), awaitee);
}

JobExecutionContext *StacklessJobManager::AttachExclusiveWorker(Runtime *runtime, PandaVM *vm)
{
    // actually we need this lock due to worker limit
    os::memory::LockHolder eWorkerLock(eWorkerCreationLock_);

    if (IsExclusiveWorkersLimitReached()) {
        LOG(DEBUG, EXECUTION) << "The program reached the limit of exclusive workers";
        return nullptr;
    }

    return CreateExclusiveExecutionContext(runtime, vm, "[e-worker] ");
}

bool StacklessJobManager::DetachExclusiveWorker()
{
    auto *eWorker = GetCurrentWorker();
    if (!eWorker->InExclusiveMode()) {
        LOG(DEBUG, EXECUTION) << "Trying to destroy not exclusive worker";
        return false;
    }
    LOG(FATAL, EXECUTION) << "This API is not yet supported in the stackless mode";
    return true;
}

bool StacklessJobManager::IsExclusiveWorkersLimitReached() const
{
    LOG(ERROR, EXECUTION) << "This API is not yet supported in the stackless mode";
    return false;
}

void StacklessJobManager::CreateGeneralWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    if (howMany == 0) {
        LOG(DEBUG, EXECUTION)
            << "StacklessJobManager::CreateGeneralWorkers(): creation of zero workers requested, skipping...";
        return;
    }
    os::memory::LockHolder lh(workersLock_);
    auto wCountBeforeCreation = activeWorkersCount_;
    for (uint32_t i = 0; i < howMany; ++i) {
        CreateWorker(runtime, vm, "worker ");
    }
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::CreateGeneralWorkers(): waiting for workers startup";
    while (activeWorkersCount_ != howMany + wCountBeforeCreation) {
        workersCv_.Wait(&workersLock_);
    }
}

void StacklessJobManager::FinalizeGeneralWorkers([[maybe_unused]] size_t howMany, [[maybe_unused]] Runtime *runtime,
                                                 [[maybe_unused]] PandaVM *vm)
{
    LOG(FATAL, EXECUTION) << "This API is not yet supported in the stackless mode";
}

void StacklessJobManager::PreZygoteFork()
{
    WaitForMutatorJobsCompletion();
    FinalizeGeneralWorkers(COMMON_WORKERS_COUNT - 1, Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
}

void StacklessJobManager::PostZygoteFork()
{
    Runtime *runtime = Runtime::GetCurrent();
    CreateGeneralWorkers(COMMON_WORKERS_COUNT - 1, runtime, runtime->GetPandaVM());
}

void StacklessJobManager::OnWorkerStartup(JobWorkerThread *worker)
{
    os::memory::LockHolder lock(workersLock_);
    workers_.push_back(StacklessJobWorkerThread::FromJobWorkerThread(worker));
    ++activeWorkersCount_;
    worker->OnStartup();
    workersCv_.Signal();
    JobManager::OnWorkerStartup(worker);
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::OnWorkerStartup(): COMPLETED, active workers = "
                          << activeWorkersCount_;
}

void StacklessJobManager::OnWorkerShutdown(JobWorkerThread *worker)
{
    os::memory::LockHolder lock(workersLock_);
    auto workerIt = std::find_if(workers_.begin(), workers_.end(), [worker](auto &&w) { return w == worker; });
    workers_.erase(workerIt);
    ReleaseWorkerId(worker->GetId());
    --activeWorkersCount_;
    JobManager::OnWorkerShutdown(worker);
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(worker);
    workersCv_.Signal();
    LOG(DEBUG, EXECUTION) << "StacklessJobManager::OnWorkerShutdown(): COMPLETED, workers left = "
                          << activeWorkersCount_;
}

bool StacklessJobManager::IsRunningThreadExist()
{
    UNREACHABLE();
    return false;
}

size_t StacklessJobManager::GetExistingWorkersCount() const
{
    os::memory::LockHolder lh(workersLock_);
    return workers_.size();
}

JobExecutionContext *StacklessJobManager::CreateEntrypointlessExecCtx(Runtime *runtime, PandaVM *vm, bool makeCurrent,
                                                                      PandaString name, Job::Type type,
                                                                      JobPriority priority)
{
    auto *job = CreateJob(std::move(name), std::monostate(), priority, type);
    ASSERT(job != nullptr);
    auto *execCtx = execCtxFactory_(runtime, vm, job);
    ASSERT(execCtx != nullptr);
    execCtx->InitBuffers();
    if (makeCurrent) {
        JobExecutionContext::SetCurrent(execCtx);
        execCtx->NativeCodeBegin();
    }
    return execCtx;
}

void StacklessJobManager::DestroyEntrypointlessExecCtx(JobExecutionContext *executionCtx)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(!executionCtx->GetJob()->HasEntrypoint());

    bool unsetCurrent = JobExecutionContext::GetCurrent() == executionCtx;

    executionCtx->GetJob()->SetStatus(Job::Status::TERMINATING);
    TerminateExecutionContext(executionCtx);

    Runtime::GetCurrent()->GetInternalAllocator()->Delete(executionCtx);

    if (unsetCurrent) {
        JobExecutionContext::SetCurrent(nullptr);
    }
}

StacklessJobWorkerThread *StacklessJobManager::GetCurrentWorker()
{
    auto *w = JobExecutionContext::GetCurrent()->GetWorker();
    return StacklessJobWorkerThread::FromJobWorkerThread(w);
}

JobManagerConfig &StacklessJobManager::GetConfig()
{
    return config_;
}

bool StacklessJobManager::EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                                               unsigned int xorMask) const
{
    os::memory::LockHolder lock(jobCtxListLock_);
    for (auto *t : jobExecCtxs_) {
        if (!ApplyCallbackToThread(cb, t, incMask, xorMask)) {
            return false;
        }
    }
    return true;
}

bool StacklessJobManager::EnumerateWorkersImpl([[maybe_unused]] const EnumerateWorkerCallback &cb) const
{
    os::memory::LockHolder lock(workersLock_);
    for (auto *w : workers_) {
        if (!cb(w)) {
            return false;
        }
    }
    return true;
}

void StacklessJobManager::CreateMainExecutionContext(Runtime *runtime, PandaVM *vm)
{
    ASSERT(GetMainThread() == nullptr);
    auto *mainCtx =
        CreateEntrypointlessExecCtx(runtime, vm, true, "_main_", Job::Type::MUTATOR, JobPriority::DEFAULT_PRIORITY);
    auto *wMain = CreateWorker(runtime, vm, "[main] worker ", false, true, mainCtx);
    OnWorkerStartup(wMain);
    ASSERT(wMain->GetId() == MAIN_WORKER_ID);
    ASSERT(wMain->IsMainWorker());
    ASSERT(wMain->GetSchedulerExecutionCtx() == mainCtx);
    ASSERT(JobExecutionContext::GetCurrent() == mainCtx);
    SetMainThread(mainCtx);
}

void StacklessJobManager::DestroyMainExecutionContext()
{
    ASSERT(GetCurrentWorker()->IsMainWorker());
    OnWorkerShutdown(GetCurrentWorker());
    auto *mainCtx = JobExecutionContext::CastFromMutator(GetMainThread());
    DestroyEntrypointlessExecCtx(mainCtx);
    SetMainThread(nullptr);
}

JobExecutionContext *StacklessJobManager::CreateExclusiveExecutionContext(Runtime *runtime, PandaVM *vm,
                                                                          PandaString workerName)
{
    auto *eaCtx = CreateEntrypointlessExecCtx(runtime, vm, true, "[ea_ctx] " + workerName, Job::Type::MUTATOR,
                                              JobPriority::MEDIUM_PRIORITY);
    auto *eaWorker = CreateWorker(runtime, vm, std::move(workerName), true, false, eaCtx);
    OnWorkerStartup(eaWorker);
    ASSERT(eaWorker->InExclusiveMode());
    ASSERT(eaWorker->GetSchedulerExecutionCtx() == eaCtx);
    ASSERT(JobExecutionContext::GetCurrent() == eaCtx);
    return eaCtx;
}

StacklessJobWorkerThread *StacklessJobManager::CreateWorker(Runtime *runtime, PandaVM *vm, PandaString workerName,
                                                            bool inExclusiveMode, bool isMainWorker,
                                                            JobExecutionContext *executionCtx)
{
    auto workerId = AllocateWorkerId();
    workerName += ToPandaString(workerId);
    auto allocator = runtime->GetInternalAllocator();
    auto *worker = allocator->New<StacklessJobWorkerThread>(runtime, vm, std::move(workerName), workerId,
                                                            inExclusiveMode, isMainWorker, executionCtx);
    ASSERT(worker != nullptr);
    return worker;
}

std::pair<StacklessJobWorkerThread *, AffinityMask> StacklessJobManager::ChooseWorkerForJob(const LaunchParams &params)
{
    auto *currentW = GetCurrentWorker();
    if (params.launchImmediately || (params.groupId == JobWorkerThreadGroup::AnyId() && currentW->InExclusiveMode())) {
        return {currentW, AffinityMask::FromGroupId(JobWorkerThreadGroup::GenerateExactWorkerId(currentW->GetId()))};
    }

    auto mask = AffinityMask::FromGroupId(params.groupId);
    std::vector<StacklessJobWorkerThread *> suitableWorkers;

#ifndef NDEBUG
    LOG(DEBUG, EXECUTION) << "Evaluating load factors:";
    for (auto w : workers_) {
        LOG(DEBUG, EXECUTION) << w->GetName() << ": LF = " << w->GetLoadFactor();
    }
#endif
    std::copy_if(workers_.begin(), workers_.end(), std::back_inserter(suitableWorkers), [mask](auto *w) {
        // NOTE(panferovi): shouldn't we restrict exclusive workers? + check if worker is disabled for launch?
        return mask.IsWorkerAllowed(w->GetId());
    });
    if (suitableWorkers.empty()) {
        return {nullptr, AffinityMask::Empty()};
    }
    auto wIt =
        std::min_element(suitableWorkers.begin(), suitableWorkers.end(), [](const auto *first, const auto *second) {
            // choosing the least loaded worker from the allowed worker set
            return first->GetLoadFactor() < second->GetLoadFactor();
        });
    LOG(DEBUG, EXECUTION) << "Chose worker: " << (*wIt)->GetName();
    return {*wIt, mask};
}

void StacklessJobManager::RegisterJob(Job *job)
{
    ASSERT(job != nullptr);
    os::memory::LockHolder lh(jobRegistryLock_);
    jobRegistry_.insert(job);
}

void StacklessJobManager::RemoveJobFromRegistry(Job *job)
{
    {
        os::memory::LockHolder lh(jobRegistryLock_);
        [[maybe_unused]] auto removed = jobRegistry_.erase(job);
        ASSERT(removed != 0);
    }
    CheckProgramCompletion();
}

bool StacklessJobManager::EnumerateJobsImpl(const EnumerateJobsCallback &cb) const
{
    os::memory::LockHolder lh(jobRegistryLock_);
    for (auto *job : jobRegistry_) {
        if (!cb(job)) {
            return false;
        }
    }
    return true;
}

void StacklessJobManager::WaitForMutatorJobsCompletion()
{
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        GetCurrentWorker()->ExecuteJobs();
        GetCurrentWorker()->GetSchedulerExecutionCtx()->ListUnhandledEventsOnProgramExit();
        {
            os::memory::LockHolder lkCompletion(programCompletionLock_);
            if (AllJobsAreExecuted()) {
                return;
            }
            programCompletionEvent_.SetNotHappened();
            programCompletionEvent_.Lock();
        }
        GetCurrentWorker()->WaitForEvent(&programCompletionEvent_);
        LOG(DEBUG, COROUTINES) << "StacklessJobManager::WaitForMutatorJobsCompletion(): possibly "
                                  "spurious wakeup from wait...";
    }
}

void StacklessJobManager::CheckProgramCompletion()
{
    os::memory::LockHolder lkCompletion(programCompletionLock_);

    if (AllJobsAreExecuted()) {
        LOG(DEBUG, EXECUTION) << "StacklessJobManager::CheckProgramCompletion(): all jobs finished execution!";
        programCompletionEvent_.Happen();
    } else {
        LOG(DEBUG, EXECUTION) << "StacklessJobManager::CheckProgramCompletion(): still "
                              << GetRegisteredJobsCount() - GetActiveWorkersCount() << " jobs exist...";
    }
}

bool StacklessJobManager::AllJobsAreExecuted() const
{
    return GetRegisteredJobsCount() <= GetActiveWorkersCount();
}

size_t StacklessJobManager::GetActiveWorkersCount() const
{
    os::memory::LockHolder lkWorkers(workersLock_);
    return activeWorkersCount_;
}

size_t StacklessJobManager::GetRegisteredJobsCount() const
{
    os::memory::LockHolder lkWorkers(jobRegistryLock_);
    return jobRegistry_.size();
}

}  // namespace ark
