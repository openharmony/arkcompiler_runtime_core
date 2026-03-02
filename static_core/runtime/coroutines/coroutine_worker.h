/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H

#include "include/external_callback_poster.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/coroutines/local_storage.h"
#include "runtime/coroutines/coroutine_worker_domain.h"

namespace ark {

/**
 * THE ORDER HAS MEANING
 * ASCEDING ORDER - HIGHER PRIORITY
 * DO NOT CHANGE INITIALIZATION VALUES
 */
enum class CoroutinePriority {
    LOW_PRIORITY,
    MEDIUM_PRIORITY,
    DEFAULT_PRIORITY = MEDIUM_PRIORITY,
    HIGH_PRIORITY,
    CRITICAL_PRIORITY,
    PRIORITY_COUNT
};

/// Represents a coroutine worker, which can host multiple coroutines and schedule them.
class CoroutineWorker : public Thread {
public:
    enum class DataIdx { INTEROP_CTX_PTR, EXTERNAL_IFACES, FLATTENED_STRING_CACHE, LAST_ID };
    using LocalStorage = StaticLocalStorage<DataIdx>;
    using Id = int32_t;

    struct LocalObjectData {
        CoroutineWorker::DataIdx type;
        mem::Reference *objectRef;
        LocalStorage::Finalizer finalizer;
    };

    static constexpr Id INVALID_ID = -1;

    NO_COPY_SEMANTIC(CoroutineWorker);
    NO_MOVE_SEMANTIC(CoroutineWorker);

    CoroutineWorker(Runtime *runtime, PandaVM *vm, PandaString name, Id id, bool isMainWorker)
        : Thread(vm), runtime_(runtime), name_(std::move(name)), id_(id), isMainWorker_(isMainWorker)
    {
#if defined(ARK_USE_COMMON_RUNTIME)
        mutator_ = common_vm::Mutator::CreateAndRegisterNewMutator(nullptr);
#endif  // ARK_USE_COMMON_RUNTIME
    }
    ~CoroutineWorker() override
    {
        DestroyCallbackPoster();
    }

    Runtime *GetRuntime()
    {
        return runtime_;
    }

    Id GetId() const
    {
        return id_;
    }

    PandaString GetName() const
    {
        return name_;
    }

    void SetName(PandaString name)
    {
        name_ = std::move(name);
    }

    bool IsMainWorker() const
    {
        return isMainWorker_;
    }

    virtual bool InExclusiveMode() const
    {
        return false;
    }

    LocalStorage &GetLocalStorage()
    {
        return localStorage_;
    }

    void SetCallbackPoster(PandaUniquePtr<CallbackPoster> poster)
    {
        ASSERT(!extSchedulingPoster_);
        extSchedulingPoster_ = std::move(poster);
    }

    void DestroyCallbackPoster();

    bool IsExternalSchedulingEnabled()
    {
        os::memory::LockHolder l(posterLock_);
        return extSchedulingPoster_ != nullptr;
    }

    void OnCoroBecameActive(Coroutine *co);
    virtual void OnNewCoroutineStartup([[maybe_unused]] Coroutine *co) {};

    void TriggerSchedulerExternally(Coroutine *requester, int64_t delayMs = 0);

    /// @brief Update worker local object in coroutines
    virtual void CacheLocalObjectsInCoroutines() {};

    /**
     * Initialize worker local objects
     * @param objectRefs created objects
     */
    void InitWorkerLocalObjects(PandaVector<LocalObjectData> &&objectRefs);

    /// should be called once the VM is ready to create managed objects in the managed heap
    void InitializeManagedStructures();

    static PandaVector<LocalObjectData> CreateWorkerLocalObjects();

    /// @brief should be called by CoroutineManager just before the worker become visible for others
    void OnBeforeWorkerStartup();

    /**
     * @brief Must be called by CoroutineManager after the worker is created,
     *        from the coroutine assigned to that worker.
     */
    void OnWorkerStartup();

    /**
     * This method perform ready asynchronous work of the coroutine worker.
     * If there is no ready async work but pending exists, the method blocks until it gets ready.
     * @return true if some async work still exists, false otherwise
     */
    virtual bool ProcessAsyncWork()
    {
        return false;
    }

#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::Mutator *GetMutator()
    {
        return mutator_;
    }
#endif  // ARK_USE_COMMON_RUNTIME

private:
    Runtime *runtime_ = nullptr;
    PandaString name_;
    Id id_ = INVALID_ID;
    bool isMainWorker_ = false;
    LocalStorage localStorage_;
    // event loop poster
    os::memory::Mutex posterLock_;
    PandaUniquePtr<CallbackPoster> extSchedulingPoster_;
#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::Mutator *mutator_ = nullptr;
#endif  // ARK_USE_COMMON_RUNTIME
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H */
