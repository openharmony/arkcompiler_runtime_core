/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H

#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/stackful_common.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"
#include "runtime/coroutines/coroutine_stats.h"

namespace ark {

/**
 * @brief Stackful ("fiber"-based) coroutine manager implementation.
 *
 * In this implementation coroutines are user-level threads ("fibers") with manually allocated stacks.
 *
 * For interface function descriptions see CoroutineManager class declaration.
 */
class StackfulCoroutineManager : public CoroutineManager {
public:
    NO_COPY_SEMANTIC(StackfulCoroutineManager);
    NO_MOVE_SEMANTIC(StackfulCoroutineManager);

    explicit StackfulCoroutineManager(CoroutineFactory factory) : CoroutineManager(factory) {}
    ~StackfulCoroutineManager() override = default;

    /* CoroutineManager interfaces, see CoroutineManager class for the details */
    void Initialize(CoroutineManagerConfig config, Runtime *runtime, PandaVM *vm) override;
    void Finalize() override;
    void RegisterCoroutine(Coroutine *co) override;
    bool TerminateCoroutine(Coroutine *co) override;
    Coroutine *Launch(CompletionEvent *completionEvent, Method *entrypoint, PandaVector<Value> &&arguments,
                      CoroutineLaunchMode mode) override;
    void Schedule() override;
    void Await(CoroutineEvent *awaitee) RELEASE(awaitee) override;
    void UnblockWaiters(CoroutineEvent *blocker) override;

    /* ThreadManager interfaces, see ThreadManager class for the details */
    void WaitForDeregistration() override;
    void SuspendAllThreads() override;
    void ResumeAllThreads() override;
    bool IsRunningThreadExist() override;

    /**
     * @brief Creates a coroutine instance with a native function as an entry point
     * @param entry native function to execute
     * @param param param to pass to the EP
     */
    Coroutine *CreateNativeCoroutine(Runtime *runtime, PandaVM *vm,
                                     Coroutine::NativeEntrypointInfo::NativeEntrypointFunc entry, void *param,
                                     PandaString name);
    /// destroy the "native" coroutine created earlier
    void DestroyNativeCoroutine(Coroutine *co);

    void DestroyEntrypointfulCoroutine(Coroutine *co) override;

    /// called when a coroutine worker thread ends its execution
    void OnWorkerShutdown();
    /// called when a coroutine worker thread starts its execution
    void OnWorkerStartup();

    /* debugging tools */
    /**
     * For StackfulCoroutineManager implementation: a fatal error is issued if an attempt to switch coroutines on
     * current worker is detected when coroutine switch is disabled.
     */
    void DisableCoroutineSwitch() override;
    void EnableCoroutineSwitch() override;
    bool IsCoroutineSwitchDisabled() override;
    bool IsJsMode() override;

    /* profiling tools */
    CoroutineStats &GetPerfStats()
    {
        return stats_;
    }

protected:
    bool EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                              unsigned int xorMask) const override;
    CoroutineContext *CreateCoroutineContext(bool coroHasEntrypoint) override;
    void DeleteCoroutineContext(CoroutineContext *ctx) override;

    size_t GetCoroutineCount() override;
    size_t GetCoroutineCountLimit() override;

    StackfulCoroutineContext *GetCurrentContext();
    StackfulCoroutineWorker *GetCurrentWorker();

    /**
     * @brief reuse a cached coroutine instance in case when coroutine pool is enabled
     * see Coroutine::ReInitialize for details
     */
    void ReuseCoroutineInstance(Coroutine *co, CompletionEvent *completionEvent, Method *entrypoint,
                                PandaVector<Value> &&arguments, PandaString name);

private:
    StackfulCoroutineContext *CreateCoroutineContextImpl(bool needStack);
    StackfulCoroutineWorker *ChooseWorkerForCoroutine(Coroutine *co);
    stackful_coroutines::AffinityMask CalcAffinityMaskFromLaunchMode(CoroutineLaunchMode mode);

    Coroutine *LaunchImpl(CompletionEvent *completionEvent, Method *entrypoint, PandaVector<Value> &&arguments,
                          CoroutineLaunchMode mode);
    /**
     * Tries to extract a coroutine instance from the pool for further reuse, returns nullptr in case when it is not
     * possible.
     */
    Coroutine *TryGetCoroutineFromPool();

    /* workers API */
    /**
     * @brief create the arbitrary number of worker threads
     * @param how_many total number of worker threads, including MAIN
     */
    void CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) REQUIRES(workersLock_);

    /* coroutine registry management */
    void AddToRegistry(Coroutine *co);
    void RemoveFromRegistry(Coroutine *co) REQUIRES(coroListLock_);

    /// call to check if we are done executing managed code and set appropriate member flags
    void CheckProgramCompletion();
    /// call when main coroutine is done executing its managed EP
    void MainCoroutineCompleted();
    /// wait till all the non-main coroutines with managed EP finish execution
    void WaitForNonMainCoroutinesCompletion();
    /// @return number of running worker loop coroutines
    size_t GetActiveWorkersCount() const;
    /// @return number of existing worker instances
    size_t GetExistingWorkersCount() const;
    /// dump coroutine stats to stdout
    void DumpCoroutineStats() const;

    /* resource management */
    uint8_t *AllocCoroutineStack();
    void FreeCoroutineStack(uint8_t *stack);

    // for thread safety with GC
    mutable os::memory::Mutex coroListLock_;
    // all registered coros
    PandaSet<Coroutine *> coroutines_ GUARDED_BY(coroListLock_);

    // worker threads-related members
    PandaVector<StackfulCoroutineWorker *> workers_ GUARDED_BY(workersLock_);
    size_t activeWorkersCount_ GUARDED_BY(workersLock_) = 0;
    mutable os::memory::RecursiveMutex workersLock_;
    mutable os::memory::ConditionVariable workersCv_;

    // events that control program completion
    mutable os::memory::Mutex programCompletionLock_;
    CoroutineEvent *programCompletionEvent_ = nullptr;

    // various counters
    std::atomic_uint32_t coroutineCount_ = 0;
    size_t coroutineCountLimit_ = 0;
    size_t coroStackSizeBytes_ = 0;
    bool jsMode_ = false;

    /**
     * @brief holds pointers to the cached coroutine instances in order to speedup coroutine creation and destruction.
     * linked coroutinecontext instances are cached too (we just keep the cached coroutines linked to their contexts).
     * used only in case when --use-coroutine-pool=true
     */
    PandaVector<Coroutine *> coroutinePool_ GUARDED_BY(coroPoolLock_);
    mutable os::memory::Mutex coroPoolLock_;

    // stats
    CoroutineStats stats_;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H */
