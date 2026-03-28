/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_MANAGER_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_MANAGER_H

#include "runtime/execution/coroutines/coroutine_context.h"
#include "runtime/include/runtime.h"
#include "runtime/execution/coroutines/stackful/stackful_common.h"
#include "runtime/execution/coroutines/coroutine.h"
#include "runtime/execution/job_events.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_worker_group.h"

namespace ark {

// For now, it is much more readable to store the config in a struct with constants, instead of having
// a class with numerous accessors. Can be reconsidered in future if the config becomes something more than POD.
// The member naming is directly related: THIS_MEMBER_NAME_CASE_STYLE has poor redability in this case.
// NOLINTBEGIN(readability-identifier-naming, misc-non-private-member-variables-in-classes)

/// @brief describes the set of adjustable parameters for CoroutineManager and its descendants initialization
struct CoroutineManagerConfig {
    static constexpr uint32_t WORKERS_COUNT_AUTO = 0;

    /// enable the experimental task execution interface
    const bool enableDrainQueueIface = false;
    /// enable migration
    const bool enableMigration = false;
    /// migrate coroutines that resumed from wait
    const bool migrateAwakenedCoros = false;
    /// Number of coroutine workers for the N:M mode
    const uint32_t workersCount = WORKERS_COUNT_AUTO;
    /// Limit on the number of exclusive coroutines workers
    const uint32_t exclusiveWorkersLimit = 0;
    /// Collection of performance statistics
    const bool enablePerfStats = false;
    /// Enable external timer implementation
    const bool enableExternalTimer = false;
    /// Number of exclusive workers created for runtime needs
    const uint32_t preallocatedExclusiveWorkersCount = 0;

    CoroutineManagerConfig() = delete;
};
// NOLINTEND(readability-identifier-naming, misc-non-private-member-variables-in-classes)

/// @brief defines the scheduling policy for a coroutine. Maybe in future we would like to add more types.
enum class CoroutineSchedulingPolicy {
    /// choose the least busy worker
    ANY_WORKER,
    /**
     * same as any_worker but exclude the main worker from available hosts on launch and
     * disallow non_main -> main transitions on migration
     */
    NON_MAIN_WORKER
};

/// @brief defines the selection policy for a worker.
enum class WorkerSelectionPolicy {
    /// choose the least busy worker
    LEAST_LOADED,
    /// choose the busiest worker
    MOST_LOADED
};

/**
 * @brief The interface of all coroutine manager implementations.
 *
 * Manages (registers, unregisters, enumerates) and schedules coroutines for execution using the worker threads.
 * Creates and destroys the main coroutine.
 * Provides interfaces for coroutine synchronization.
 */
class CoroutineManager : public JobManager {
public:
    /**
     * @brief The coroutine factory interface.
     *
     * @param name the coroutine name (for debugging and logging purposes)
     *
     * @param ctx the instance of implementation-dependend native coroutine context. Usually is provided by the
     * concrete CoroutineManager descendant.
     *
     * @param ep_info if provided (that means, ep_info != std::nullopt), defines the coroutine entry point to execute.
     * It can be either (the bytecode method, its arguments and the completion event instance to hold the method return
     * value) or (a native function and its parameter). See Coroutine::EntrypointInfo for details. If this parameter is
     * std::nullopt (i.e. no entrypoint present) then the following rules apply:
     * - the factory should only create the coroutine instance and expect that the bytecode for the coroutine will be
     * invoked elsewhere within the context of the newly created coroutine
     * - the coroutine should be destroyed manually by the runtime by calling the Destroy() method
     * - the coroutine does not use the method/arguments passing interface and the completion event interface
     * - no other coroutine will await this one (although it can await others).
     * The "main" coroutine (the EP of the application) is the specific example of such "no entrypoint" coroutine
     * If ep_info is provided then the newly created coroutine will execute the specified method and do all the
     * initialization/finalization steps, including completion_event management and notification of waiters.
     *
     * @param type type of work, which the coroutine performs: whether it is a mutator, a schedule loop or some other
     * thing
     */
    using CoroutineFactory = Coroutine *(*)(Runtime *runtime, PandaVM *vm, Job *job, CoroutineContext *ctx);
    using NativeEntrypointFunc = Job::NativeEntrypointInfo::NativeEntrypointFunc;

    NO_COPY_SEMANTIC(CoroutineManager);
    NO_MOVE_SEMANTIC(CoroutineManager);

    /// Factory is used to create coroutines when needed. See CoroutineFactory for details.
    explicit CoroutineManager(const CoroutineManagerConfig &config, CoroutineFactory factory);
    ~CoroutineManager() override = default;

    void RegisterExecutionContext(JobExecutionContext *executionCtx) override
    {
        RegisterCoroutine(Coroutine::CastFromMutator(executionCtx));
    }

    bool TerminateExecutionContext(JobExecutionContext *executionCtx) override
    {
        return TerminateCoroutine(Coroutine::CastFromMutator(executionCtx));
    }

    /// Add coroutine to registry (used for enumeration and tracking) and perform all the required actions
    virtual void RegisterCoroutine(Coroutine *co) = 0;
    /**
     * @brief Remove coroutine from all internal structures, notify waiters about its completion, correctly
     * delete coroutine and free its resources
     * @return returnes true if coroutine has been deleted, false otherwise (e.g. in case of terminating main)
     */
    virtual bool TerminateCoroutine(Coroutine *co) = 0;

    void ExecuteJobs() override
    {
        Schedule();
    }

    /**
     * The designated interface for creating the main coroutine instance.
     * The program EP will be invoked within its context.
     */
    Coroutine *CreateMainCoroutine(Runtime *runtime, PandaVM *vm);
    /// Delete the main coroutine instance and free its resources
    void DestroyMainCoroutine();

    /**
     * Create a coroutine instance (including the context) for internal purposes (e.g. verifier) and
     * set it as the current one.
     * The created coroutine instance will not have any method to execute. All the control flow must be managed
     * by the caller.
     *
     * @return nullptr if resource limit reached or something went wrong; ptr to the coroutine otherwise
     */
    Coroutine *CreateEntrypointlessCoroutine(Runtime *runtime, PandaVM *vm, bool makeCurrent, PandaString name,
                                             Coroutine::Type type, CoroutinePriority priority);
    void DestroyEntrypointlessCoroutine(Coroutine *co);

    /// Destroy a coroutine with an entrypoint
    virtual void DestroyEntrypointfulCoroutine(Coroutine *co);

    void SetSchedulingPolicy(CoroutineSchedulingPolicy policy);
    CoroutineSchedulingPolicy GetSchedulingPolicy() const;

    virtual bool IsUserCoroutineLimitReached() const
    {
        return false;
    }

    /*
     * @brief Checks if a coroutine is a system, i.e. coro creation fails is fatal
     * Coroutine is system if it is:
     *   SCHEDULER (represents the CoroutineWorker's scheduler loop) ──┬── UTILITY CORO
     *   FINALIZER (used to shut down coroutine workers)             ──╯
     *   EP-less MUTATOR (represents the defult manged execution context for MAIN and Exclusive workers)
     */
    static bool IsSystemCoroutine(Coroutine::Type type, bool hasEntryPoint)
    {
        return type == Coroutine::Type::SCHEDULER || (type == Coroutine::Type::FINALIZER) ||
               (type == Coroutine::Type::MUTATOR && !hasEntryPoint);
    }

    /*
     * @brief Checks if a coroutine is a system, i.e. coro creation fails is fatal
     * Coroutine is system if it is:
     *   SCHEDULER (represents the CoroutineWorker's scheduler loop) ──┬── UTILITY CORO
     *   FINALIZER (used to shut down coroutine workers)             ──╯
     *   EP-less MUTATOR (represents the defult manged execution context for MAIN and Exclusive workers)
     */
    static bool IsSystemCoroutine(Coroutine *co)
    {
        return CoroutineManager::IsSystemCoroutine(co->GetType(), co->GetJob()->HasNativeEntrypoint() ||
                                                                      co->GetJob()->HasManagedEntrypoint());
    }

    /* events */
    /// Should be called when a coro makes the non_active->active transition (see the state diagram in coroutine.h)
    virtual void OnCoroBecameActive([[maybe_unused]] Coroutine *co) {};
    /**
     * Should be called when a running coro is being blocked or terminated, i.e. makes
     * the active->non_active transition (see the state diagram in coroutine.h)
     */
    virtual void OnCoroBecameNonActive([[maybe_unused]] Coroutine *co) {};

    /// read only (implying no const_casts!) version of the initial config
    const CoroutineManagerConfig &GetConfig() const
    {
        return config_;
    }

    /// Returns number of existing coroutines
    virtual size_t GetCoroutineCount() = 0;

protected:
    /// Suspend the current coroutine and schedule the next ready one for execution
    virtual void Schedule() = 0;

    using EntrypointInfo = Coroutine::EntrypointInfo;
    /// Create native coroutine context instance (implementation dependent)
    virtual CoroutineContext *CreateCoroutineContext(bool coroHasEntrypoint) = 0;
    /// Delete native coroutine context instance (implementation dependent)
    virtual void DeleteCoroutineContext(CoroutineContext *ctx) = 0;
    /**
     * Create coroutine instance including the native context and link the context and the coroutine.
     * The coroutine is created using the factory provided in the CoroutineManager constructor and the virtual
     * context creation function, which should be defined in concrete CoroutineManager implementations.
     *
     * @return nullptr if resource limit reached or something went wrong; ptr to the coroutine otherwise
     */
    Coroutine *CreateCoroutineInstance(Job *job);
    /**
     * @brief returns number of coroutines that could be created till the resource limit is reached.
     * The resource limit definition is specific to the exact coroutines/coroutine manager implementation.
     */
    virtual size_t GetCoroutineCountLimit() = 0;

    /// Can be used in descendants to create custom coroutines manually
    CoroutineFactory GetCoroutineFactory();

private:
    CoroutineFactory coFactory_ = nullptr;

    mutable os::memory::Mutex policyLock_;
    CoroutineSchedulingPolicy schedulingPolicy_ GUARDED_BY(policyLock_) = CoroutineSchedulingPolicy::NON_MAIN_WORKER;

    // the coroutine manager config
    CoroutineManagerConfig config_;
};

/// Disables coroutine switch on the current worker for some scope. Can be used recursively.
class ScopedDisableJobSwitch {
public:
    explicit ScopedDisableJobSwitch(JobManager *jobManager) : jobMan_(jobManager)
    {
        ASSERT(jobMan_ != nullptr);
        jobMan_->DisableJobSwitch();
    }

    ~ScopedDisableJobSwitch()
    {
        jobMan_->EnableJobSwitch();
    }

private:
    JobManager *jobMan_;

    NO_COPY_SEMANTIC(ScopedDisableJobSwitch);
    NO_MOVE_SEMANTIC(ScopedDisableJobSwitch);
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_MANAGER_H */
