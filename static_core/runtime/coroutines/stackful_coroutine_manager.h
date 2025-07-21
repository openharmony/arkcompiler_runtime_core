/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
    bool Launch(CompletionEvent *completionEvent, Method *entrypoint, PandaVector<Value> &&arguments,
                CoroutineLaunchMode mode, CoroutinePriority priority, bool abortFlag) override;
    bool LaunchImmediately(CompletionEvent *completionEvent, Method *entrypoint, PandaVector<Value> &&arguments,
                           CoroutineLaunchMode mode, CoroutinePriority priority, bool abortFlag) override;
    bool LaunchNative(NativeEntrypointFunc epFunc, void *param, PandaString coroName, CoroutineLaunchMode mode,
                      CoroutinePriority priority, bool abortFlag) override;
    void Schedule() override;
    void Await(CoroutineEvent *awaitee) RELEASE(awaitee) override;
    void UnblockWaiters(CoroutineEvent *blocker) override;

    bool IsMainWorker(Coroutine *co) const override;
    Coroutine *CreateExclusiveWorkerForThread(Runtime *runtime, PandaVM *vm) override;
    bool DestroyExclusiveWorker() override;
    bool IsExclusiveWorkersLimitReached() const override;

    void CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) override;
    void FinalizeWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) override;

    void PreZygoteFork() override;
    void PostZygoteFork() override;

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
                                     PandaString name, Coroutine::Type type, CoroutinePriority priority);
    /// destroy the "native" coroutine created earlier
    void DestroyNativeCoroutine(Coroutine *co);
    void DestroyEntrypointfulCoroutine(Coroutine *co) override;

    /// get next free worker id
    size_t GetNextFreeWorkerId();

    /* events */
    /// called when a coroutine worker thread ends its execution
    void OnWorkerShutdown(StackfulCoroutineWorker *worker);
    /// called when a coroutine worker thread starts its execution
    void OnWorkerStartup(StackfulCoroutineWorker *worker);
    /// Should be called when a coro makes the non_active->active transition (see the state diagram in coroutine.h)
    void OnCoroBecameActive(Coroutine *co) override;
    /**
     * Should be called when a running coro is being blocked or terminated, i.e. makes
     * the active->non_active transition (see the state diagram in coroutine.h)
     */
    void OnCoroBecameNonActive(Coroutine *co) override;
    /// Should be called at the end of the VM native interface call
    void OnNativeCallExit(Coroutine *co) override;

    /* debugging tools */
    /**
     * For StackfulCoroutineManager implementation: a fatal error is issued if an attempt to switch coroutines on
     * current worker is detected when coroutine switch is disabled.
     */
    void DisableCoroutineSwitch() override;
    void EnableCoroutineSwitch() override;
    bool IsCoroutineSwitchDisabled() override;
    bool IsDrainQueueInterfaceEnabled()
    {
        return enableDrainQueueIface_;
    }
    bool IsMigrationEnabled()
    {
        return enableMigration_;
    }
    bool IsMigrateAwakenedCorosEnabled()
    {
        return migrateAwakenedCoros_;
    }

    /* profiling tools */
    CoroutineStats &GetPerfStats()
    {
        return stats_;
    }

    /// migrate coroutines from the 'from' worker to other workers
    bool MigrateCoroutinesOutward(StackfulCoroutineWorker *from);
    /// trigger the managerThread to migrate
    void TriggerMigration();
    /**
     * @brief migrate the awakened coroutine to the worker with the lowest load
     * @param co the awakened coroutine
     */
    void MigrateAwakenedCoro(Coroutine *co);

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
    void ReuseCoroutineInstance(Coroutine *co, EntrypointInfo &&epInfo, PandaString name, CoroutinePriority priority);

private:
    using WorkerId = uint32_t;

    StackfulCoroutineContext *CreateCoroutineContextImpl(bool needStack);
    StackfulCoroutineWorker *ChooseWorkerForCoroutine(Coroutine *co) REQUIRES(workersLock_);
    stackful_coroutines::AffinityMask CalcAffinityMaskFromLaunchMode(CoroutineLaunchMode mode);

    Coroutine *GetCoroutineInstanceForLaunch(EntrypointInfo &&epInfo, PandaString &&coroName,
                                             CoroutinePriority priority, stackful_coroutines::AffinityMask affinityMask,
                                             bool abortFlag);
    bool LaunchImpl(EntrypointInfo &&epInfo, PandaString &&coroName, CoroutineLaunchMode mode,
                    CoroutinePriority priority, bool abortFlag);
    bool LaunchImmediatelyImpl(EntrypointInfo &&epInfo, PandaString &&coroName, CoroutineLaunchMode mode,
                               CoroutinePriority priority, bool abortFlag);
    bool LaunchWithMode(EntrypointInfo &&epInfo, PandaString &&coroName, CoroutineLaunchMode mode,
                        CoroutinePriority priority, bool launchImmediately, bool abortFlag);
    /**
     * Tries to extract a coroutine instance from the pool for further reuse, returns nullptr in case when it is not
     * possible.
     */
    Coroutine *TryGetCoroutineFromPool();

    /* workers API */
    void CreateWorkersImpl(size_t howMany, Runtime *runtime, PandaVM *vm) REQUIRES(workersLock_);
    /**
     * This method creates main worker and coroutine + the number of common workers
     * @param howMany total number of common worker threads, NOT including MAIN
     */
    void CreateMainCoroAndWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) REQUIRES(workersLock_);
    void OnWorkerStartupImpl(StackfulCoroutineWorker *worker) REQUIRES(workersLock_);
    StackfulCoroutineWorker *CreateWorker(Runtime *runtime, PandaVM *vm,
                                          StackfulCoroutineWorker::ScheduleLoopType wType, PandaString workerName);

    /* coroutine registry management */
    void AddToRegistry(Coroutine *co) REQUIRES(coroListLock_);
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

    /// @return true if there is no active coroutines
    bool IsNoActiveMutatorsExceptCurrent();
    /// Increment/decrement active corotuines count
    void IncrementActiveCoroutines();
    void DecrementActiveCoroutines();

    /// list unhandled language specific events on program exit
    void ListUnhandledEventsOnProgramExit();

    StackfulCoroutineWorker *ChooseWorkerForFinalization();

    void InitializeWorkerIdAllocator();
    WorkerId AllocateWorkerId();
    void ReleaseWorkerId(WorkerId workerId);

    /**
     * The EP for manager thread. The manager thread can only be created when coroutine migration is supported.
     * This function cannot be called directly, and can only be called once.
     */
    void ManagerThreadProc();
    /// manage the lifecycle of the manager thread
    void StartManagerThread();
    void StopManagerThread();

    void CheckForBlockedWorkers();
    void MigrateCoroutinesInward(uint32_t &count);
    StackfulCoroutineWorker *ChooseWorkerImpl(WorkerSelectionPolicy policy, size_t maskValue) REQUIRES(workersLock_);

    /**
     * @brief Calculate worker limits based on configuration
     * @param config The coroutine manager configuration
     * @param exclusiveWorkersLimit Output parameter for exclusive workers limit
     * @param commonWorkersLimit Output parameter for common workers limit
     */
    void CalculateWorkerLimits(const CoroutineManagerConfig &config, size_t &exclusiveWorkersLimit,
                               size_t &commonWorkersLimit);

    // for thread safety with GC
    mutable os::memory::Mutex coroListLock_;
    // all registered coros
    PandaSet<Coroutine *> coroutines_ GUARDED_BY(coroListLock_);

    // worker threads-related members
    PandaList<StackfulCoroutineWorker *> workers_ GUARDED_BY(workersLock_);
    // allocator of worker ids
    os::memory::Mutex workerIdLock_;
    PandaList<WorkerId> freeWorkerIds_ GUARDED_BY(workerIdLock_);
    size_t activeWorkersCount_ GUARDED_BY(workersLock_) = 0;
    mutable os::memory::RecursiveMutex workersLock_;
    mutable os::memory::ConditionVariable workersCv_;

    // events that control program completion
    os::memory::Mutex programCompletionLock_;
    CoroutineEvent *programCompletionEvent_ = nullptr;

    // various counters
    std::atomic_uint32_t coroutineCount_ = 0;
    size_t coroutineCountLimit_ = 0;
    size_t exclusiveWorkersLimit_ = 0;
    size_t commonWorkersCount_ = 0;
    size_t coroStackSizeBytes_ = 0;

    // active coroutines are runnable + running coroutines
    std::atomic_uint32_t activeCoroutines_ = 0;
    // NOTE(konstanting): make it a map once number of the coroutine types gets bigger
    std::atomic_uint32_t systemCoroutineCount_ = 0;

    /**
     * @brief holds pointers to the cached coroutine instances in order to speedup coroutine creation and destruction.
     * linked coroutinecontext instances are cached too (we just keep the cached coroutines linked to their contexts).
     * used only in case when --use-coroutine-pool=true
     */
    PandaVector<Coroutine *> coroutinePool_ GUARDED_BY(coroPoolLock_);
    mutable os::memory::Mutex coroPoolLock_;

    // stats
    CoroutineStats stats_;
    PandaVector<CoroutineWorkerStats> finalizedWorkerStats_;

    os::memory::Mutex eWorkerCreationLock_;

    // Feature flags, eventually will be refactored into some structure.
    // Should we just store the initial CoroutineConfig?..
    bool enableDrainQueueIface_ = false;
    // coroutine migration feature
    bool enableMigration_ = false;
    bool migrateAwakenedCoros_ = false;

    // the number of migration triggers
    std::atomic_uint32_t migrateCount_ = 0;
    // manager thread infos
    std::atomic_bool managerRunning_ = false;
    std::thread managerThread_;
    os::memory::Mutex managerMutex_;
    os::memory::ConditionVariable managerCv_;

    // the time interval between detecting worker blocking
    static constexpr uint32_t DETECTION_INTERVAL_VALUE = 5000;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H */
