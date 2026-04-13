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
#ifndef PANDA_RUNTIME_EXECUTION_STACKLESS_STACKLESS_JOB_WORKER_THREAD_H
#define PANDA_RUNTIME_EXECUTION_STACKLESS_STACKLESS_JOB_WORKER_THREAD_H

#include "runtime/execution/job_worker_thread.h"
#include "runtime/execution/priority_queue.h"
#include "runtime/include/mem/panda_containers.h"

#include "libarkbase/clang.h"
#include "libarkbase/os/mutex.h"

namespace ark {

class Job;
class JobEvent;
class JobExecutionContext;
class StacklessJobManager;

class StacklessJobWorkerThread : public JobWorkerThread {
public:
    NO_COPY_SEMANTIC(StacklessJobWorkerThread);
    NO_MOVE_SEMANTIC(StacklessJobWorkerThread);

    StacklessJobWorkerThread(Runtime *runtime, PandaVM *vm, PandaString workerName, Id id, bool inExclusiveMode,
                             bool isMainWorker, JobExecutionContext *executionCtx = nullptr);
    ~StacklessJobWorkerThread() override = default;

    static StacklessJobWorkerThread *FromJobWorkerThread(JobWorkerThread *worker)
    {
        return static_cast<StacklessJobWorkerThread *>(worker);
    }

    void ExecuteJobs();

    void WaitForEvent(JobEvent *awaitee) RELEASE(awaitee);

    void UnblockWaiters(JobEvent *blocker);

    void AddRunnableJob(Job *job);

    void AddJobAndExecute(Job *job);

    void AddJobWithDependency(Job *job, JobEvent *dependency);

    void OnStartup() override;

    void Deactivate();

    bool IsActive() const
    {
        return isActive_;
    }
    /// @return the moving average number of runnable jobs in the queue
    double GetLoadFactor() const
    {
        return loadFactor_;
    }

    void CacheLocalObjectsInExecutionCtx() override;

private:
    void AttachExecutionContext(JobExecutionContext *executionCtx);

    void ThreadProc();

    void ScheduleLoopBody();

    bool ExecuteRunnableJob();

    void WaitForRunnables();

    void UpdateLoadFactor();

    void UpdateLoadFactorImpl() REQUIRES(runnablesLock_);

    void PushToRunnableQueue(Job *job, JobPriority priority);

    void AddJobInWaiters(JobEvent *blocker, Job *job) RELEASE(*blocker);

    void RegisterIncomingJob(Job *newJob);

    bool EventIsInPendingQueue(JobEvent *event);

    void ExecuteJob(Job *job);

    void ProcessTimerEvents();

    int64_t GetShortestTimerDelay();

    void UpdateMinExpirationTime(JobEvent *blocker);

private:
    static constexpr uint64_t MAX_EXPIRATION_TIME = std::numeric_limits<uint64_t>::max();

    StacklessJobManager *jobManager_;
    std::atomic<bool> isActive_ = true;

    os::memory::Mutex runnablesLock_;
    os::memory::ConditionVariable runnablesCv_;
    PriorityQueue<Job> runnables_ GUARDED_BY(runnablesLock_);

    os::memory::Mutex waitersLock_;
    PandaMap<JobEvent *, Job *> waiters_ GUARDED_BY(waitersLock_);

    /// the moving average number of coroutines in the runnable queue
    std::atomic<double> loadFactor_ = 0;

    /// the minimal expiration time of pending timer
    uint64_t minExpirationTime_ = MAX_EXPIRATION_TIME;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_STACKLESS_STACKLESS_JOB_WORKER_THREAD_H
