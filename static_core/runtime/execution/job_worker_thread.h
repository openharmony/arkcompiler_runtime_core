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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_WORKER_THREAD_H
#define PANDA_RUNTIME_EXECUTION_JOB_WORKER_THREAD_H

#include "libarkbase/macros.h"
#include "runtime/include/thread.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/external_callback_poster.h"
#include "runtime/execution/local_storage.h"

namespace ark {

class Runtime;
class PandaVM;
class JobExecutionContext;

/**
 * @brief Represents a worker thread that executes Jobs.
 *
 * JobWorkerThread is the abstraction for a thread that executes jobs within
 * the Job execution model. It provides:
 * - Runtime and VM references for job execution
 * - Local storage for thread-specific data (interop context, external interfaces)
 * - Support for exclusive mode execution (E-workers)
 * - Callback poster for external scheduling
 *
 * Worker threads can operate in two modes:
 * - Normal mode: jobs can be migrated between workers for load balancing
 * - Exclusive mode: child jobs are always scheduled on the same worker
 *
 * @see Job
 * @see JobExecutionContext
 * @see JobManager
 */
class JobWorkerThread : public Thread {
public:
    enum class DataIdx {
        INTEROP_CTX_PTR,
        EXTERNAL_IFACES,
        FLATTENED_STRING_CACHE,
        DOUBLE_TO_STRING_CACHE,
        FLOAT_TO_STRING_CACHE,
        LONG_TO_STRING_CACHE,
        LAST_ID
    };
    using LocalStorage = StaticLocalStorage<DataIdx>;
    using Id = int32_t;

    struct LocalObjectData {
        JobWorkerThread::DataIdx type;
        mem::Reference *objectRef;
        LocalStorage::Finalizer finalizer;
    };

    static constexpr Id INVALID_ID = -1;

    using CreatePluginObjFunc = std::function<void(PandaVector<LocalObjectData> &)>;

    NO_COPY_SEMANTIC(JobWorkerThread);
    NO_MOVE_SEMANTIC(JobWorkerThread);

    JobWorkerThread(Runtime *runtime, PandaVM *vm, PandaString name, Id id, bool inExclusiveMode, bool isMainWorker)
        : Thread(vm),
          runtime_(runtime),
          name_(std::move(name)),
          id_(id),
          inExclusiveMode_(inExclusiveMode),
          isMainWorker_(isMainWorker)
    {
    }

    ~JobWorkerThread() override;

    inline Runtime *GetRuntime() const;

    inline Id GetId() const;

    inline PandaString GetName() const;

    inline void SetName(PandaString name);

    /// @brief get exclusive status of worker
    inline bool InExclusiveMode() const;

    inline bool IsMainWorker() const;

    inline LocalStorage &GetLocalStorage();

    inline bool IsExternalSchedulingEnabled();

    inline void SetCallbackPoster(PandaUniquePtr<CallbackPoster> poster);

    inline void PostSchedulingTask(int64_t delayMs = 0);

    inline void DestroyCallbackPoster();

    inline JobExecutionContext *GetSchedulerExecutionCtx() const;

    inline void SetSchedulerExecutionCtx(JobExecutionContext *executionCtx);

    /// @brief should be called by JobManager just before the worker become visible for others
    virtual void OnBeforeWorkerStartup(const CreatePluginObjFunc &createEtsObj);

    /**
     * @brief Must be called by JobManager after the worker is created,
     *        from the coroutine assigned to that worker.
     */
    virtual void OnStartup();

    /**
     * Initialize worker local objects
     * @param objectRefs created objects
     */
    void InitWorkerLocalObjects(PandaVector<LocalObjectData> &&objectRefs);

    static PandaVector<LocalObjectData> CreateWorkerLocalObjects(const CreatePluginObjFunc &createEtsObj = nullptr);

    /// @brief Update worker local object in execution context
    virtual void CacheLocalObjectsInExecutionCtx() {}

private:
    /// should be called once the VM is ready to create managed objects in the managed heap
    void InitializeManagedStructures(const CreatePluginObjFunc &createEtsObj = nullptr);

private:
    Runtime *runtime_ = nullptr;
    PandaString name_;
    Id id_ = INVALID_ID;
    /**
     * If worker is in exclusive mode, it means that:
     * 1. launch/transition of job from other workers to e-worker is disabled
     * 2. child e-worker jobs will be scheduled on the same worker
     */
    const bool inExclusiveMode_ = false;
    const bool isMainWorker_ = false;
    LocalStorage localStorage_;
    // event loop poster
    os::memory::Mutex posterLock_;
    PandaUniquePtr<CallbackPoster> extSchedulingPoster_;

    // worker thread execution context
    JobExecutionContext *schedulerExecutionCtx_ = nullptr;
};

}  // namespace ark

#include "runtime/execution/job_worker_thread-inl.h"

#endif  // PANDA_RUNTIME_EXECUTION_JOB_WORKER_THREAD_H
