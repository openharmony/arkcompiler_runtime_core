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
#ifndef PANDA_RUNTIME_EXECUTION_STACKLESS_STACKLESS_JOB_MANAGER_H
#define PANDA_RUNTIME_EXECUTION_STACKLESS_STACKLESS_JOB_MANAGER_H

#include "libarkbase/os/mutex.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_events.h"
#include "runtime/execution/stackless/job_manager_config.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {

class StacklessJobWorkerThread;
class JobExecutionContext;

class StacklessJobManager : public JobManager {
public:
    NO_COPY_SEMANTIC(StacklessJobManager);
    NO_MOVE_SEMANTIC(StacklessJobManager);

    using JobExecCtxFactory = JobExecutionContext *(*)(Runtime *runtime, PandaVM *vm, Job *job);

    StacklessJobManager(const JobManagerConfig &config, JobExecCtxFactory factory);
    ~StacklessJobManager() override = default;

    void InitializeScheduler(Runtime *runtime, PandaVM *vm) override;
    void InitializeManagedStructures(const JobWorkerThread::CreatePluginObjFunc &createEtsObj) override;
    void WaitForDeregistration() override;
    void Finalize() override;

    void RegisterExecutionContext(JobExecutionContext *executionCtx) override;
    bool TerminateExecutionContext(JobExecutionContext *executionCtx) override;

    LaunchResult Launch(Job *job, const LaunchParams &params) override;
    void ExecuteJobs() override;
    void Await(JobEvent *awaitee) RELEASE(awaitee) override;
    void UnblockWaiters(JobEvent *blocker) override;

    JobExecutionContext *AttachExclusiveWorker(Runtime *runtime, PandaVM *vm) override;
    bool DetachExclusiveWorker() override;
    bool IsExclusiveWorkersLimitReached() const override;

    void CreateGeneralWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) override;
    void FinalizeGeneralWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) override;

    void PreZygoteFork() override;
    void PostZygoteFork() override;

    void OnWorkerStartup(JobWorkerThread *worker) override;
    /// @brief method eraces provided worker from manager and deletes pointer
    void OnWorkerShutdown(JobWorkerThread *worker) override;

    bool IsRunningThreadExist() override;

    size_t GetExistingWorkersCount() const;

    JobExecutionContext *CreateEntrypointlessExecCtx(Runtime *runtime, PandaVM *vm, bool makeCurrent, PandaString name,
                                                     Job::Type type, JobPriority priority);
    void DestroyEntrypointlessExecCtx(JobExecutionContext *executionCtx);

    StacklessJobWorkerThread *GetCurrentWorker();

    JobManagerConfig &GetConfig();

    void RegisterJob(Job *job);

    void RemoveJobFromRegistry(Job *job);

private:
    bool EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                              unsigned int xorMask) const override;
    bool EnumerateWorkersImpl(const EnumerateWorkerCallback &cb) const override;

    bool EnumerateJobsImpl(const EnumerateJobsCallback &cb) const override;

    void CreateMainExecutionContext(Runtime *runtime, PandaVM *vm);
    void DestroyMainExecutionContext();
    JobExecutionContext *CreateExclusiveExecutionContext(Runtime *runtime, PandaVM *vm, PandaString workerName);

    StacklessJobWorkerThread *CreateWorker(Runtime *runtime, PandaVM *vm, PandaString workerName,
                                           bool inExclusiveMode = false, bool isMainWorker = false,
                                           JobExecutionContext *executionCtx = nullptr);

    std::pair<StacklessJobWorkerThread *, AffinityMask> ChooseWorkerForJob(const LaunchParams &params)
        REQUIRES(workersLock_);

    void WaitForMutatorJobsCompletion();

    void CheckProgramCompletion();

    bool AllJobsAreExecuted() const;

    size_t GetActiveWorkersCount() const;

    size_t GetRegisteredJobsCount() const;

    // NOTE(panferovi): generalize limits
    static constexpr uint32_t COMMON_WORKERS_COUNT = 2;

    // job manager config
    JobManagerConfig config_;
    // job execution context factory
    JobExecCtxFactory execCtxFactory_;

    // for thread safety with GC
    mutable os::memory::Mutex jobCtxListLock_;
    // all registered job execution contexts
    PandaSet<JobExecutionContext *> jobExecCtxs_ GUARDED_BY(jobCtxListLock_);

    // for thread safety with GC
    mutable os::memory::Mutex jobRegistryLock_;
    // all registered jobs
    PandaSet<Job *> jobRegistry_ GUARDED_BY(jobRegistryLock_);

    // worker threads-related members
    mutable os::memory::Mutex workersLock_;
    mutable os::memory::ConditionVariable workersCv_;
    PandaVector<StacklessJobWorkerThread *> workers_ GUARDED_BY(workersLock_);
    size_t activeWorkersCount_ GUARDED_BY(workersLock_) = 0;

    os::memory::Mutex eWorkerCreationLock_;

    // events that control program completion
    os::memory::Mutex programCompletionLock_;
    GenericEvent programCompletionEvent_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_STACKLESS_STACKLESS_JOB_MANAGER_H
