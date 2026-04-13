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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_EXECUTION_CONTEXT_H
#define PANDA_RUNTIME_EXECUTION_JOB_EXECUTION_CONTEXT_H

#include "runtime/include/managed_thread.h"

namespace ark {

class Job;
class JobWorkerThread;
class JobManager;

/**
 * @brief Execution context for a Job running on a worker thread.
 *
 * JobExecutionContext extends ManagedThread to provide the execution environment
 * for a Job. It manages the association between a Job and the worker thread
 * that executes it, as well as the JobManager that coordinates execution.
 *
 * Each job runs within an execution context that provides:
 * - Thread-local storage for interop context and external interfaces
 * - Cached objects for performance optimization
 * - Association with the worker thread and job manager
 *
 * @see Job
 * @see JobWorkerThread
 * @see JobManager
 */
class JobExecutionContext : public ManagedThread {
public:
    NO_COPY_SEMANTIC(JobExecutionContext);
    NO_MOVE_SEMANTIC(JobExecutionContext);
    ~JobExecutionContext() override;

    static JobExecutionContext *Create(Runtime *runtime, PandaVM *vm, Job *job);

    static JobExecutionContext *CastFromMutator(Mutator *mutator)
    {
        return static_cast<JobExecutionContext *>(mutator);
    }

    static JobExecutionContext *GetCurrent()
    {
        return CastFromMutator(Mutator::GetCurrent());
    }

    Job *GetJob()
    {
        return currentJob_;
    }

    const Job *GetJob() const
    {
        return currentJob_;
    }

    void SetJob(Job *job)
    {
        currentJob_ = job;
    }

    /// Assign this coroutine to a worker. Should not be used directly
    virtual void SetWorker(JobWorkerThread *worker)
    {
        worker_ = worker;
    }

    /// Get the currently assigned worker
    virtual JobWorkerThread *GetWorker() const
    {
        return worker_;
    }

    /// Get the currently assigned worker
    JobManager *GetManager() const
    {
        return manager_;
    }

    virtual void ExecuteJob(Job *job);

    virtual void UpdateCachedObjects() {}

    virtual void CacheBuiltinClasses() {}

    virtual void OnJobCompletion(Value result);

    /// @brief list unhandled language specific events on program exit
    virtual void ListUnhandledEventsOnProgramExit() {}

protected:
    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    JobExecutionContext(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                        ManagedThread::ThreadType threadType, panda_file::SourceLang threadLang, Job *job);

private:
    Job *currentJob_ = nullptr;
    JobManager *manager_ = nullptr;
    JobWorkerThread *worker_ = nullptr;

    // Allocator calls our protected ctor
    friend class mem::Allocator;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_JOB_EXECUTION_CONTEXT_H
