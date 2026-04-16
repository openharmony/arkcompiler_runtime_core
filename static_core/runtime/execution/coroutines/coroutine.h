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
#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_H

#include <atomic>
#include <optional>
#include <variant>

#include "runtime/execution/job.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_events.h"
#include "runtime/execution/coroutines/coroutine_worker.h"
#include "runtime/execution/coroutines/coroutine_priority.h"
#include "runtime/include/runtime.h"
#include "runtime/include/managed_thread.h"
#if defined(ARK_USE_COMMON_RUNTIME)
#include "common_interfaces/thread/mutator.h"
#endif  // ARK_USE_COMMON_RUNTIME

namespace ark {
#if defined(ARK_USE_COMMON_RUNTIME)
using CommonRootVisitor = common_vm::CommonRootVisitor;

extern "C" void VisitCoroutine(void *coroutine, CommonRootVisitor visitor);
#endif  // ARK_USE_COMMON_RUNTIME

class CoroutineManager;
class CoroutineContext;
class CompletionEvent;
/**
 * @brief The base class for all coroutines. Holds the managed part of the coroutine context.
 *
 * The coroutine context is splitted into managed and native parts.
 * The managed part is store in this class and its descendants. For the native part see the
 * CoroutineContext class and its descendants.
 */
class Coroutine : public JobExecutionContext {
public:
    NO_COPY_SEMANTIC(Coroutine);
    NO_MOVE_SEMANTIC(Coroutine);

    /**
     * ACTIVE = (RUNNABLE | RUNNING) & WORKER_ASSIGNED
     * NOT_ACTIVE = (CREATED | BLOCKED | TERMINATING) | NO_WORKER_ASSIGNED
     */
    using Status = Job::Status;
    using Id = Job::Id;
    using Type = Job::Type;
    using EntrypointInfo = Job::EntrypointInfo;

    /**
     * The coroutine factory: creates and initializes a coroutine instance. The preferred way to create a
     * coroutine. For details see CoroutineManager::CoroutineFactory
     */
    static Coroutine *Create(Runtime *runtime, PandaVM *vm, Job *job, CoroutineContext *context);
    ~Coroutine() override = default;

    /// Should be called after creation in order to create native context and do other things
    virtual void Initialize();
    /**
     * Coroutine reinitialization: semantically equivalent to Initialize(),
     * but is called to prepare a cached Coroutine instance for reuse when it is needed.
     * Implies that the CleanUp() method was called before caching.
     */
    virtual void ReInitialize(Job *job, CoroutineContext *context);
    /**
     * Manual destruction, applicable only to the main coro. Other ones get deleted by the coroutine manager once they
     * finish execution of their entrypoint method.
     */
    void Destroy();

    void CleanUp() override;

    bool RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize) override;

    static bool MutatorIsCoroutine(Mutator *mutator)
    {
        ASSERT(mutator != nullptr);
        return (mutator->GetMutatorType() == MutatorType::MANAGED) &&
               (static_cast<ManagedThread *>(mutator)->GetManagedThreadType() ==
                ManagedThread::ThreadType::THREAD_TYPE_COROUTINE);
    }

    static Coroutine *CastFromMutator(Mutator *mutator)
    {
        ASSERT(MutatorIsCoroutine(mutator));
        return static_cast<Coroutine *>(mutator);
    }

    static Coroutine *GetCurrent()
    {
        auto *mutator = Mutator::GetCurrent();
        ASSERT(mutator != nullptr);
        if (MutatorIsCoroutine(mutator)) {
            return CastFromMutator(mutator);
        }
        return nullptr;
    }

    /// Get coroutine status. It is independent from MutatorStatus.
    Status GetCoroutineStatus() const;

    /**
     * Suspend a coroutine, so its status becomes either Status::RUNNABLE or Status::BLOCKED, depending on the suspend
     * reason.
     */
    virtual void RequestSuspend(bool getsBlocked);
    /// Resume the suspended coroutine, so its status becomes Status::RUNNING.
    virtual void RequestResume();
    /// Unblock the blocked coroutine, setting its status to Status::RUNNABLE
    virtual void RequestUnblock();
    /**
     * @brief Indicate that coroutine entrypoint execution is finished. Propagates the coroutine
     * return value to language level objects.
     */
    virtual void RequestCompletion(Value returnValue);

    template <class T>
    T *GetContext() const
    {
        return static_cast<T *>(context_);
    }

    CoroutineWorker *GetWorker() const override;

    PANDA_PUBLIC_API CoroutineManager *GetCoroutineManager() const;

    /**
     * Assign this coroutine to a worker. Please do not call this method directly unless it is absolutely
     * necessary (e.g. in the threaded backend). Use worker's interfaces like AddRunnableCoroutine(),
     * AddRunningCoroutine().
     */
    void SetWorker(JobWorkerThread *w) override;

    /// @return true if the coroutine is ACTIVE (see the status transition diagram and the notes below)
    bool IsActive();

    bool IsSuspendOnStartup() const
    {
        return startSuspended_;
    }

    /// @return immediate launcher and reset it to nullptr
    Coroutine *ReleaseImmediateLauncher()
    {
        return std::exchange(immediateLauncher_, nullptr);
    }

    /// Set immediate launcher
    void SetImmediateLauncher(Coroutine *il)
    {
        ASSERT(immediateLauncher_ == nullptr);
        immediateLauncher_ = il;
    }

    /// Possibly noreturn. Call this if the coroutine got an unexpected exception.
    virtual void HandleUncaughtException() {};

    /* event handlers */
    virtual void OnHostWorkerChanged() {};
    virtual void OnStatusChanged(Status oldStatus, Status newStatus);
    virtual void OnContextSwitchedTo();
    virtual void OnChildCoroutineCreated([[maybe_unused]] Coroutine *child) {};

    void ExecuteJob(Job *job) override;

#if defined(ARK_USE_COMMON_RUNTIME)
    void Visit(CommonRootVisitor visitor)
    {
        visitor(nullptr);
    }
#endif  // ARK_USE_COMMON_RUNTIME

    /// Get coroutine name.
    const PandaString &GetName() const
    {
        return GetJob()->GetName();
    }

    /// Get unique coroutine ID
    Id GetCoroutineId() const
    {
        return GetJob()->GetId();
    }

    /// @return type of work that the coroutine performs
    Type GetType() const
    {
        return GetJob()->GetType();
    }

    /// @return coroutine priority which is used in the runnable queue
    JobPriority GetPriority() const
    {
        return GetJob()->GetPriority();
    }

    void UpdateCachedObjects() override {}

    /// The method returns true if context switching at this point in the managed code is risky
    virtual bool IsContextSwitchRisky() const
    {
        return false;
    }

    /// The method prints managed call stack of coroutine
    virtual void PrintCallStack() const;

    /**
     * a converter function that stores the data from EntrypointInfo in the member variables
     * NOTE: should be made private after #29944
     */
    void ReplaceEntrypoint(EntrypointInfo &&epInfo);

protected:
    // We would like everyone to use the factory to create a Coroutine, thus ctor is protected
    explicit Coroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                       ark::panda_file::SourceLang threadLang, Job *job, CoroutineContext *context);

    void SetCoroutineStatus(Status newStatus);

private:
    /// Method starts or ends tracing base on statuses
    void IssueTracingEvents(Status oldStatus, Status newStatus);

    CoroutineContext *context_ = nullptr;
    // NOTE(konstanting, #IAD5MH): check if we still need this functionality
    bool startSuspended_ = false;
    /**
     * Immediate launcher is a caller coroutine that will be back edge for the callee coroutine.
     * This means that the next context to switch from callee is the context of immediateLauncher_.
     */
    Coroutine *immediateLauncher_ = nullptr;

    // Allocator calls our protected ctor
    friend class mem::Allocator;
    friend class CoroutineContext;
};

std::ostream &operator<<(std::ostream &os, Coroutine::Status status);
std::ostream &operator<<(std::ostream &os, Coroutine::Type type);

}  // namespace ark

#endif  // PANDA_RUNTIME_COROUTINES_COROUTINE_H
