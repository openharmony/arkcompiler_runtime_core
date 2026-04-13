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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_H
#define PANDA_RUNTIME_EXECUTION_JOB_H

#include <variant>
#include "runtime/include/value.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/execution/job_priority.h"
#include "runtime/execution/affinity_mask.h"

namespace ark {

class CompletionEvent;
class JobExecutionContext;

/**
 * @brief Represents a unit of work that can be executed asynchronously.
 *
 * Job is the fundamental unit of work in the Job execution model, designed to
 * support 1xN and Nx1 execution patterns. Each Job represents a potentially
 * suspendable piece of work that can be scheduled on worker threads.
 *
 * A Job can have one of three types (Job::Type):
 * - MUTATOR: Regular application code execution
 * - SCHEDULER: Internal scheduler tasks
 * - FINALIZER: Finalization registry cleanup tasks
 *
 * Job lifecycle states (Job::Status):
 * - CREATED: Initial state after construction
 * - RUNNABLE: Ready to be scheduled on a worker
 * - RUNNING: Currently executing on a worker
 * - YIELDED: Suspended by the scheduler to execute another task
 * - BLOCKED: Waiting for an event (I/O, timer, etc.)
 * - TERMINATING: Execution completed, cleanup in progress
 *
 * Jobs support both managed (bytecode) and native entrypoints, with optional
 * abort flag for uncaught exception handling.
 *
 * @see JobExecutionContext
 * @see JobManager
 * @see JobWorkerThread
 * @see SuspendableJob
 */
class Job {
public:
    /**
     * Status transitions:
     *
     * +---------+              +----------+
     * | CREATED | -----------> |          | <-------------+
     * +---------+              | RUNNABLE |               |
     *                     +--- |          | <--+          |
     *                     |    +----------+    |          |
     *                     |                    |          |
     *                     |    +----------+    |     +----------+
     * +-------------+     +--> |          | ---+     |          |
     * | TERMINATING | <------- | RUNNING  | -------> | BLOCKED  |
     * +-------------+     +--- |          | ---+     |          |
     *                     |    +----------+    |     +----------+
     *                     |                    |
     *                     |    +----------+    |
     *                     +--> |          | <--+
     *                          | YIELDED  |
     *                          |          |
     *                          +----------+
     *
     */
    enum class Status { CREATED, RUNNABLE, RUNNING, YIELDED, BLOCKED, TERMINATING };

    /// the type of work that a job performs
    enum class Type { MUTATOR, SCHEDULER, FINALIZER };

    using Id = uint32_t;

    /**
     * Needed for object locking
     * Now 10,000 jobs is the maximum number of jobs that can exist at the moment.
     * So MAX_JOB_ID is the value of the smallest power of two that is greater than 10,000.
     */
    static constexpr Id MAX_JOB_ID = 1U << 14U;

    /// A helper struct that aggregates all EP related data for a job with a managed EP
    struct ManagedEntrypointInfo {
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        CompletionEvent *completionEvent = nullptr;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        Method *entrypoint = nullptr;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        PandaVector<Value> arguments;

        /**
         * @param event an instance of CompletionEvent to be used on job completion to pass the
         * return value to the point where it is needed. Also is used to unblock the jobs that are waiting for
         * the one being created to complete.
         *
         * @param entry managed method to execute in the context of job.
         *
         * @param args the array of EP method arguments
         */
        explicit ManagedEntrypointInfo(CompletionEvent *event, Method *entry, PandaVector<Value> &&args)
            : completionEvent(event), entrypoint(entry), arguments(std::move(args))
        {
            ASSERT(entry != nullptr);
        }

        NO_COPY_SEMANTIC(ManagedEntrypointInfo);
        ManagedEntrypointInfo(ManagedEntrypointInfo &&epInfo);
        ManagedEntrypointInfo &operator=(ManagedEntrypointInfo &&epInfo);
        ~ManagedEntrypointInfo();
    };

    /// A helper struct that aggregates all EP related data for a job with a native EP
    struct NativeEntrypointInfo {
        using NativeEntrypointFunc = void (*)(void *);
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        NativeEntrypointFunc entrypoint;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        void *param;

        /**
         * @param entry native function to execute in the context of job
         *
         * @param arguments a parameter which will be passed to the entrypoint (usually some object pointer)
         */
        explicit NativeEntrypointInfo(NativeEntrypointFunc entry, void *data) : entrypoint(entry), param(data)
        {
            ASSERT(entry != nullptr);
        }
    };

    using EntrypointInfo = std::variant<std::monostate, ManagedEntrypointInfo, NativeEntrypointInfo>;

    void CleanUp();
    void ReInitialize(PandaString name, EntrypointInfo &&epInfo, JobPriority priority, bool abortFlag);

    PANDA_PUBLIC_API static Job *GetCurrent();
    PANDA_PUBLIC_API static void SetCurrent(Job *job);

    static Job *Create(PandaString name, Id id, EntrypointInfo &&epInfo,
                       JobPriority priority = JobPriority::DEFAULT_PRIORITY, Type type = Type::MUTATOR,
                       bool abortFlag = false);
    static void Destroy(Job *job);

    /// @return job's name.
    inline const PandaString &GetName() const;
    /// @return job's unique ID
    inline Id GetId() const;
    /// Get job's entrypoint info
    inline bool HasEntrypoint() const;
    inline bool HasManagedEntrypoint() const;
    inline bool HasNativeEntrypoint() const;
    /// Get job's managed entrypoint method.
    inline Method *GetManagedEntrypoint() const;
    /// Get the CompletionEvent instance
    inline CompletionEvent *GetCompletionEvent() const;
    /// Get job's managed entrypoint args if any.
    inline PandaVector<Value> &GetManagedEntrypointArguments();
    /// Get job's managed entrypoint args if any.
    inline const PandaVector<Value> &GetManagedEntrypointArguments() const;
    /// Get job's native entry function (if this is a "native" job)
    inline NativeEntrypointInfo::NativeEntrypointFunc GetNativeEntrypoint() const;
    /// Get job's native entry function parameter (if this is a "native" job)
    inline void *GetNativeEntrypointParam() const;
    /// Replace job entrypoint
    inline void ReplaceEntrypoint(EntrypointInfo &&epInfo);
    /// @return job priority which is used in the runnable queue
    inline JobPriority GetPriority() const;
    /// @return type of work that the job performs
    inline Type GetType() const;
    /// @return job's abortFlag parameter is set
    inline bool HasAbortFlag() const;
    /// @return job's affinity bits
    inline AffinityMask GetAffinityMask() const;
    /// @param mask shows the group of available workers
    inline void SetAffinityMask(const AffinityMask &mask);
    /// @return true if migration from worker is allowed
    inline bool IsMigrationAllowed() const;
    /// @return job's current status
    inline Status GetStatus() const;
    /// @param newStatus updates job's status
    inline void SetStatus(Status newStatus);
    /// @return job's current execution context
    inline JobExecutionContext *GetExecutionContext() const;
    /// @param executionCtx updates job's execution context
    inline void SetExecutionContext(JobExecutionContext *executionCtx);
    /// invokes job's entrypoint within execution context
    virtual void InvokeEntrypoint();

    Job(PandaString name, Id id, EntrypointInfo &&epInfo, JobPriority priority, Type type, bool abortFlag)
        : name_(std::move(name)),
          id_(id),
          entrypoint_(std::move(epInfo)),
          priority_(priority),
          type_(type),
          abortFlag_(abortFlag)
    {
    }
    NO_COPY_SEMANTIC(Job);
    DEFAULT_MOVE_SEMANTIC(Job);
    virtual ~Job() = default;

protected:
    void InvokeEntrypointImpl(bool resumedFrame);

private:
    /// job's stuff
    PandaString name_;
    Id id_;
    EntrypointInfo entrypoint_;
    JobPriority priority_;
    Type type_;
    /// abort flag means that the job could abort the program in an uncaugh exeption occured
    bool abortFlag_;
    AffinityMask affinityMask_ = AffinityMask::Empty();
    Status status_ {Status::CREATED};

    /// current execution context
    JobExecutionContext *executionCtx_ = nullptr;
};

}  // namespace ark

#include "runtime/execution/job-inl.h"

#endif  // PANDA_RUNTIME_EXECUTION_JOB_H
