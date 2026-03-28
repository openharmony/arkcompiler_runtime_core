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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_MANAGER_H
#define PANDA_RUNTIME_EXECUTION_JOB_MANAGER_H

#include "libarkbase/clang.h"
#include "runtime/thread_manager.h"
#include "runtime/execution/job.h"
#include "runtime/execution/job_worker_group.h"
#include "runtime/execution/job_launch.h"

namespace ark {

class JobExecutionContext;
class JobWorkerThread;
class JobEvent;

/**
 * @brief Common manager for job scheduling and worker thread coordination.
 *
 * JobManager is responsible for:
 * - Creating and destroying jobs
 * - Managing worker thread pool
 * - Scheduling jobs on available workers
 * - Handling job dependencies via events
 * - Managing worker group IDs for targeted scheduling
 *
 * It extends ThreadManager to integrate with the runtime's threading infrastructure.
 *
 * Key responsibilities:
 * - Job lifecycle management (create, launch, destroy)
 * - Worker thread creation and management (general workers, exclusive workers)
 * - Job scheduling and migration
 * - Event-based synchronization (JobEvent, CompletionEvent, TimerEvent)
 * - Statistics gathering via JobStats
 *
 * JobManager supports various execution modes:
 * - General workers: Shared pool for normal job execution
 * - Exclusive workers: Dedicated workers for specific launch behaviour
 * - Main worker: Primary worker for main thread execution
 *
 * @see Job
 * @see JobWorkerThread
 * @see JobExecutionContext
 * @see JobStats
 */
class JobManager : public ThreadManager {
public:
    JobManager() = default;
    NO_COPY_SEMANTIC(JobManager);
    NO_MOVE_SEMANTIC(JobManager);
    ~JobManager() override = default;

    using EnumerateWorkerCallback = std::function<bool(JobWorkerThread *)>;
    using EnumerateJobsCallback = std::function<bool(Job *)>;

    /**
     * @brief Should be called after JobManager creation and before any other method calls.
     * Initializes internal structures, creates the main execution context.
     *
     * @param config describes the JobManager operation mode
     */
    virtual void InitializeScheduler(Runtime *runtime, PandaVM *vm) = 0;
    /// should be called once the VM is ready to create managed objects in the managed heap
    virtual void InitializeManagedStructures(const JobWorkerThread::CreatePluginObjFunc &createEtsObj) = 0;
    /// Should be called after all execution is finished. Destroys the main execution context.
    virtual void Finalize() = 0;

    virtual void RegisterExecutionContext(JobExecutionContext *executionCtx) = 0;
    virtual bool TerminateExecutionContext(JobExecutionContext *executionCtx) = 0;

    Job *CreateJob(PandaString name, Job::EntrypointInfo &&epInfo, JobPriority priority = JobPriority::DEFAULT_PRIORITY,
                   Job::Type type = Job::Type::MUTATOR, bool abortFlag = false);
    void DestroyJob(Job *job);

    virtual LaunchResult Launch(Job *job, const LaunchParams &params) = 0;
    virtual void ExecuteJobs() = 0;
    virtual void Await(JobEvent *awaitee) RELEASE(awaitee) = 0;

    /**
     * @brief Notify the waiting jobs that an event has happened, so they can stop waiting and
     * become ready for execution
     * @param blocker the blocking event which transitioned from pending to happened state
     */
    virtual void UnblockWaiters(JobEvent *blocker) = 0;

    /// id management
    void InitializeWorkerIdAllocator();
    /// allocates id for created worker
    JobWorkerThread::Id AllocateWorkerId();
    /// releases id of the worker that is terminating
    void ReleaseWorkerId(JobWorkerThread::Id workerId);
    /**
     * @brief This function generates the worker group ID that can be further used to launch jobs within that
     * group.
     * @param domain the worker domain.
     * GENERAL domain includes all non-main and non-EA workers.
     * EXACT_ID domain includes exact worker id from hint.
     * MAIN domain includes only main worker.
     * EXACT_ID domain includes only workers from "hint" parameter.
     * @param hint optional worker ids, the usage is domain-specific.
     * For GENERAL domain non-main and non-EA worker IDs are ignored, all other IDs are used as is.
     * For MAIN domain hint is ignored.
     * For EXACT_ID domain hint is used as is, empty hint means ANY_ID.
     * For EA domain EA worker IDs are used as is, non-EA worker IDs are ignored
     */
    JobWorkerThreadGroup::Id GenerateWorkerGroupId(JobWorkerThreadDomain domain,
                                                   const PandaVector<JobWorkerThread::Id> &hint);
    /// @return unique job id
    uint32_t AllocateJobId();
    /// mark the specified @param id as free
    void FreeJobId(uint32_t id);

    /// limit the number of IDs for performance reasons
    static constexpr size_t MAX_JOB_ID = std::min(0xffffU, Job::MAX_JOB_ID);
    static constexpr size_t UNINITIALIZED_JOB_ID = 0x0U;

    /**
     * @brief enumerate workers and apply @param cb to them
     * @return true if @param cb call was successful (returned true) for every worekr and false otherwise
     */
    bool EnumerateWorkers(const EnumerateWorkerCallback &cb) const
    {
        return EnumerateWorkersImpl(cb);
    }

    bool EnumerateJobs(const EnumerateJobsCallback &cb) const
    {
        return EnumerateJobsImpl(cb);
    }

    virtual JobExecutionContext *AttachExclusiveWorker(Runtime *runtime, PandaVM *vm) = 0;

    virtual bool DetachExclusiveWorker() = 0;

    virtual bool IsExclusiveWorkersLimitReached() const = 0;

    /**
     * @brief This method creates the required number of worker threads
     * @param howMany - number of common workers to be created
     */
    virtual void CreateGeneralWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) = 0;
    /**
     * @brief This method finalizes the required number of common worker threads
     * @param howMany - number of common workers to be finalized
     * NOTE: Make sure that howMany is less than the number of active workers
     */
    virtual void FinalizeGeneralWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) = 0;

    /* ZygoteFork operations */
    /// Called before Zygote fork to clean up and stop all worker threads.
    virtual void PreZygoteFork() = 0;
    /// Called after Zygote fork to reinitialize and restart worker threads.
    virtual void PostZygoteFork() = 0;

    /// event handlers
    virtual void OnWorkerStartup(JobWorkerThread *worker);
    virtual void OnWorkerShutdown(JobWorkerThread *worker);
    /// Should be called at the beginning of the VM native interface call
    virtual void OnNativeCallEnter([[maybe_unused]] JobExecutionContext *executionCtx) {}
    /// Should be called at the end of the VM native interface call
    virtual void OnNativeCallExit([[maybe_unused]] JobExecutionContext *executionCtx) {}

    /* debugging tools */
    /**
     * Disable job switch for the current worker.
     * If an attempt to switch the active job is performed when job switch is disabled, the exact actions
     * are defined by the concrete JobManager implementation (they could include no-op, program halt or something
     * else).
     *
     * NOTE(konstanting): consider extending this interface to allow for disabling the individual job
     * operations, like JobOperation::LAUNCH, AWAIT, SCHEDULE, ALL
     */
    virtual void DisableJobSwitch() {}
    /// Enable job switch for the current worker.
    virtual void EnableJobSwitch() {}
    /// @return true if job switch for the current worker is disabled
    virtual bool IsJobSwitchDisabled()
    {
        return false;
    }

    uint64_t GetCurrentTime() const
    {
        return os::time::GetClockTimeInMicro();
    }

protected:
    /// Worker enumerator, returns true iff cb call succeeds for every worker
    virtual bool EnumerateWorkersImpl(const EnumerateWorkerCallback &cb) const = 0;
    /// Jobs enumerator - applies callback to every registered job
    virtual bool EnumerateJobsImpl(const EnumerateJobsCallback &cb) const = 0;

    static constexpr JobWorkerThread::Id MAIN_WORKER_ID = 0U;
    // maximum worker id is bound by the number of bits in the affinity mask
    static constexpr JobWorkerThread::Id MAX_WORKER_ID = AffinityMask::MAX_WORKERS_COUNT - 1U;
    static_assert(MAIN_WORKER_ID != JobWorkerThread::INVALID_ID);
    static_assert(MAX_WORKER_ID >= MAIN_WORKER_ID);

private:
    // allocator of worker ids
    os::memory::Mutex workerIdLock_;
    PandaList<JobWorkerThread::Id> freeWorkerIds_ GUARDED_BY(workerIdLock_);

    // worker's group stuff
    JobWorkerThreadGroup::Id generalWorkerGroup_ = JobWorkerThreadGroup::Empty();
    JobWorkerThreadGroup::Id eaWorkerGroup_ = JobWorkerThreadGroup::Empty();

    // job id management
    os::memory::Mutex idsLock_;
    std::bitset<MAX_JOB_ID> jobIds_ GUARDED_BY(idsLock_);
    uint32_t lastJobId_ GUARDED_BY(idsLock_) = UNINITIALIZED_JOB_ID;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_JOB_MANAGER_H
