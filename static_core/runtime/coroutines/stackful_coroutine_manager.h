/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"

namespace panda {

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
    Coroutine *Launch(CompletionEvent *completion_event, Method *entrypoint, PandaVector<Value> &&arguments) override;
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
                                     Coroutine::NativeEntrypointInfo::NativeEntrypointFunc entry, void *param);
    /// destroy the "native" coroutine created earlier
    void DestroyNativeCoroutine(Coroutine *co);

    void DestroyEntrypointfulCoroutine(Coroutine *co) override;

    /// called when a coroutine worker thread ends its execution
    void OnWorkerShutdown();

    /* debugging tools */
    /**
     * For StackfulCoroutineManager implementation: a fatal error is issued if an attempt to switch coroutines on
     * current worker is detected when coroutine switch is disabled.
     */
    void DisableCoroutineSwitch() override;
    void EnableCoroutineSwitch() override;
    bool IsCoroutineSwitchDisabled() override;

protected:
    bool EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int inc_mask,
                              unsigned int xor_mask) const override;
    CoroutineContext *CreateCoroutineContext(bool coro_has_entrypoint) override;
    void DeleteCoroutineContext(CoroutineContext *ctx) override;

    size_t GetCoroutineCount() override;
    size_t GetCoroutineCountLimit() override;

    StackfulCoroutineContext *GetCurrentContext();
    StackfulCoroutineWorker *GetCurrentWorker();
    bool IsJsMode();

    /**
     * @brief reuse a cached coroutine instance in case when coroutine pool is enabled
     * see Coroutine::ReInitialize for details
     */
    void ReuseCoroutineInstance(Coroutine *co, CompletionEvent *completion_event, Method *entrypoint,
                                PandaVector<Value> &&arguments, PandaString name);

private:
    StackfulCoroutineContext *CreateCoroutineContextImpl(bool need_stack);

    Coroutine *LaunchImpl(CompletionEvent *completion_event, Method *entrypoint, PandaVector<Value> &&arguments);
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
    void CreateWorkers(uint32_t how_many, Runtime *runtime, PandaVM *vm) REQUIRES(workers_lock_);

    /* coroutine registry management */
    void AddToRegistry(Coroutine *co);
    void RemoveFromRegistry(Coroutine *co) REQUIRES(coro_list_lock_);

    /// call to check if we are done executing managed code and set appropriate member flags
    void CheckProgramCompletion();
    /// call when main coroutine is done executing its managed EP
    void MainCoroutineCompleted();
    /// @return number of running worker loop coroutines
    size_t GetActiveWorkersCount();

    /* resource management */
    uint8_t *AllocCoroutineStack();
    void FreeCoroutineStack(uint8_t *stack);

    // for thread safety with GC
    mutable os::memory::Mutex coro_list_lock_;
    // all registered coros
    PandaSet<Coroutine *> coroutines_ GUARDED_BY(coro_list_lock_);

    // worker threads-related members
    PandaVector<StackfulCoroutineWorker *> workers_ GUARDED_BY(workers_lock_);
    size_t active_workers_count_ GUARDED_BY(workers_lock_) = 0;
    mutable os::memory::RecursiveMutex workers_lock_;
    mutable os::memory::ConditionVariable workers_shutdown_cv_;

    // events that control program completion
    mutable os::memory::Mutex program_completion_lock_;
    CoroutineEvent *program_completion_event_ = nullptr;

    // various counters
    std::atomic_uint32_t coroutine_count_ = 0;
    size_t coroutine_count_limit_ = 0;
    size_t coro_stack_size_bytes_ = 0;
    bool js_mode_ = false;

    /**
     * @brief holds pointers to the cached coroutine instances in order to speedup coroutine creation and destruction.
     * linked coroutinecontext instances are cached too (we just keep the cached coroutines linked to their contexts).
     * used only in case when --use-coroutine-pool=true
     */
    PandaVector<Coroutine *> coroutine_pool_ GUARDED_BY(coro_pool_lock_);
    mutable os::memory::Mutex coro_pool_lock_;
};

}  // namespace panda

#endif /* PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H */
