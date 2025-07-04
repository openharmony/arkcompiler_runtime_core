/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <limits>
#include "coroutines/stackful_common.h"
#include "libpandabase/macros.h"
#include "libpandabase/os/time.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"

namespace ark {

uint8_t *StackfulCoroutineManager::AllocCoroutineStack()
{
    Pool stackPool = PoolManager::GetMmapMemPool()->AllocPool<OSPagesAllocPolicy::NO_POLICY>(
        coroStackSizeBytes_, SpaceType::SPACE_TYPE_NATIVE_STACKS, AllocatorType::NATIVE_STACKS_ALLOCATOR);
    return static_cast<uint8_t *>(stackPool.GetMem());
}

void StackfulCoroutineManager::FreeCoroutineStack(uint8_t *stack)
{
    if (stack != nullptr) {
        PoolManager::GetMmapMemPool()->FreePool(stack, coroStackSizeBytes_);
    }
}

void StackfulCoroutineManager::CreateMainCoroAndWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    auto *wMain = CreateWorker(runtime, vm, StackfulCoroutineWorker::ScheduleLoopType::FIBER, "[main] worker ");
    ASSERT(wMain->GetId() == stackful_coroutines::MAIN_WORKER_ID);

    auto *mainCo = CreateMainCoroutine(runtime, vm);
    wMain->AddRunningCoroutine(mainCo);
    OnWorkerStartupImpl(wMain);

    CreateWorkersImpl(howMany, runtime, vm);
}

void StackfulCoroutineManager::CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    os::memory::LockHolder lh(workersLock_);
    CreateWorkersImpl(howMany, runtime, vm);
}

void StackfulCoroutineManager::FinalizeWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    struct EntrypointParam {
        explicit EntrypointParam(size_t wCount, CoroutineManager *coroMan)
            : wCount_(wCount), workerFinalizationEvent(coroMan)
        {
        }

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        const size_t wCount_;
        GenericEvent workerFinalizationEvent;
        std::atomic<uint32_t> finalizedWorkersCount = 0;
        // NOLINTEND(misc-non-private-member-variables-in-classes)
    };

    auto wCountBeforeFinalization = GetActiveWorkersCount();
    ASSERT(wCountBeforeFinalization > howMany);

    auto entrypointParam = EntrypointParam(howMany, this);
    auto coroEntryPoint = [](void *param) {
        auto *finalizee = Coroutine::GetCurrent()->GetContext<StackfulCoroutineContext>()->GetWorker();
        finalizee->MigrateCoroutines();
        finalizee->CompleteAllAffinedCoroutines();
        finalizee->SetActive(false);
        auto *entryParams = reinterpret_cast<EntrypointParam *>(param);
        // Atomic with relaxed order reason: synchronization is not required
        if (entryParams->finalizedWorkersCount.fetch_add(1, std::memory_order_relaxed) == entryParams->wCount_ - 1) {
            entryParams->workerFinalizationEvent.Happen();
        }
    };

    for (auto i = 0U; i < howMany; i++) {
        auto *finWorker = ChooseWorkerForFinalization();
        auto *co = CreateNativeCoroutine(runtime, vm, coroEntryPoint, &entrypointParam, "[finalize coro] ",
                                         Coroutine::Type::FINALIZER, CoroutinePriority::CRITICAL_PRIORITY);
        finWorker->AddRunnableCoroutine(co);
    }
    entrypointParam.workerFinalizationEvent.Lock();

    Await(&entrypointParam.workerFinalizationEvent);

    os::memory::LockHolder lh(workersLock_);
    while (activeWorkersCount_ != wCountBeforeFinalization - howMany) {
        workersCv_.Wait(&workersLock_);
    }
    ASSERT(activeWorkersCount_ > 0);
}

StackfulCoroutineWorker *StackfulCoroutineManager::ChooseWorkerForFinalization()
{
    os::memory::LockHolder lh(workersLock_);
    auto finWorkerIt = std::find_if(workers_.begin(), workers_.end(), [](auto &&worker) {
        return !worker->IsMainWorker() && !worker->IsDisabledForCrossWorkersLaunch() && !worker->InExclusiveMode();
    });
    ASSERT(finWorkerIt != workers_.end());
    (*finWorkerIt)->DisableForCrossWorkersLaunch();
    return *finWorkerIt;
}

void StackfulCoroutineManager::CreateWorkersImpl(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    if (howMany == 0) {
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::CreateWorkersImpl():creation of zero workers requested,skipping...";
        return;
    }
    auto wCountBeforeCreation = activeWorkersCount_;
    for (uint32_t i = 0; i < howMany; ++i) {
        CreateWorker(runtime, vm, StackfulCoroutineWorker::ScheduleLoopType::THREAD, "worker ");
    }
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::CreateWorkers(): waiting for workers startup";
    while (activeWorkersCount_ != howMany + wCountBeforeCreation) {
        // NOTE(konstanting, #IAD5MH): need timed wait?..
        workersCv_.Wait(&workersLock_);
    }
}

StackfulCoroutineWorker *StackfulCoroutineManager::CreateWorker(Runtime *runtime, PandaVM *vm,
                                                                StackfulCoroutineWorker::ScheduleLoopType wType,
                                                                PandaString workerName)
{
    auto allocator = runtime->GetInternalAllocator();
    auto workerId = AllocateWorkerId();
    workerName += ToPandaString(workerId);
    auto *worker = allocator->New<StackfulCoroutineWorker>(runtime, vm, this, wType, std::move(workerName), workerId);
    ASSERT(worker != nullptr);
    if (stats_.IsEnabled()) {
        worker->GetPerfStats().Enable();
    }
    return worker;
}

void StackfulCoroutineManager::OnWorkerShutdown(StackfulCoroutineWorker *worker)
{
    os::memory::LockHolder lock(workersLock_);
    auto workerIt = std::find_if(workers_.begin(), workers_.end(), [worker](auto &&w) { return w == worker; });
    workers_.erase(workerIt);
    // We may have a problem related to the coroutine affinity mask aliasing (The ABA Problem) #23715:
    // 1. Finalizing worker was available for the coroutine (the coroutine had a bit set in the affinity mask)
    // 2. New specific worker (e.g. EAWorker) was created with the same Id
    // 3. This worker becomes available for the coroutine, but it was not initially expected
    ReleaseWorkerId(worker->GetId());
    --activeWorkersCount_;
    auto &workerStats = worker->GetPerfStats();
    workerStats.Disable();
    finalizedWorkerStats_.emplace_back(std::move(workerStats));
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(worker);
    workersCv_.Signal();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnWorkerShutdown(): COMPLETED, workers left = "
                           << activeWorkersCount_;
}

void StackfulCoroutineManager::OnWorkerStartup(StackfulCoroutineWorker *worker)
{
    os::memory::LockHolder lock(workersLock_);
    OnWorkerStartupImpl(worker);
}

void StackfulCoroutineManager::OnWorkerStartupImpl(StackfulCoroutineWorker *worker)
{
    workers_.push_back(worker);
    ++activeWorkersCount_;
    workersCv_.Signal();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnWorkerStartup(): COMPLETED, active workers = "
                           << activeWorkersCount_;
}

void StackfulCoroutineManager::InitializeWorkerIdAllocator()
{
    os::memory::LockHolder lh(workerIdLock_);
    for (auto id = stackful_coroutines::MAIN_WORKER_ID; id != stackful_coroutines::MAX_WORKERS_COUNT; ++id) {
        freeWorkerIds_.push_back(id);
    }
}

uint32_t StackfulCoroutineManager::AllocateWorkerId()
{
    os::memory::LockHolder lh(workerIdLock_);
    ASSERT(!freeWorkerIds_.empty());
    auto workerId = freeWorkerIds_.front();
    freeWorkerIds_.pop_front();
    return workerId;
}

void StackfulCoroutineManager::ReleaseWorkerId(uint32_t workerId)
{
    os::memory::LockHolder lh(workerIdLock_);
    freeWorkerIds_.push_back(workerId);
    ASSERT(freeWorkerIds_.size() <= stackful_coroutines::MAX_WORKERS_COUNT);
}

void StackfulCoroutineManager::DisableCoroutineSwitch()
{
    GetCurrentWorker()->DisableCoroutineSwitch();
}

void StackfulCoroutineManager::EnableCoroutineSwitch()
{
    GetCurrentWorker()->EnableCoroutineSwitch();
}

bool StackfulCoroutineManager::IsCoroutineSwitchDisabled()
{
    return GetCurrentWorker()->IsCoroutineSwitchDisabled();
}

void StackfulCoroutineManager::Initialize(CoroutineManagerConfig config, Runtime *runtime, PandaVM *vm)
{
    // enable stats collection if needed
    if (config.enablePerfStats) {
        stats_.Enable();
    }
    ScopedCoroutineStats s(&stats_, CoroutineTimeStats::INIT);
    // set feature flags
    enableDrainQueueIface_ = config.enableDrainQueueIface;
    enableMigration_ = config.enableMigration;
    migrateAwakenedCoros_ = config.migrateAwakenedCoros;

    // set limits
    coroStackSizeBytes_ = Runtime::GetCurrent()->GetOptions().GetCoroutineStackSizePages() * os::mem::GetPageSize();
    if (coroStackSizeBytes_ != AlignUp(coroStackSizeBytes_, PANDA_POOL_ALIGNMENT_IN_BYTES)) {
        size_t alignmentPages = PANDA_POOL_ALIGNMENT_IN_BYTES / os::mem::GetPageSize();
        LOG(FATAL, COROUTINES) << "Coroutine stack size should be >= " << alignmentPages
                               << " pages and should be aligned to " << alignmentPages << "-page boundary!";
    }
    size_t coroStackAreaSizeBytes = Runtime::GetCurrent()->GetOptions().GetCoroutinesStackMemLimit();
    coroutineCountLimit_ = coroStackAreaSizeBytes / coroStackSizeBytes_;

    CalculateWorkerLimits(config, exclusiveWorkersLimit_, commonWorkersCount_);
    ASSERT(commonWorkersCount_ + exclusiveWorkersLimit_ <= stackful_coroutines::MAX_WORKERS_COUNT);
    InitializeWorkerIdAllocator();
    {
        os::memory::LockHolder lock(workersLock_);
        CreateMainCoroAndWorkers(commonWorkersCount_ - 1, runtime, vm);  // 1 is for MAIN here
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): successfully created and activated " << workers_.size()
                               << " coroutine workers";
        programCompletionEvent_ = Runtime::GetCurrent()->GetInternalAllocator()->New<GenericEvent>(this);
    }
    if (enableMigration_) {
        StartManagerThread();
    }
}

void StackfulCoroutineManager::Finalize()
{
    os::memory::LockHolder lock(coroPoolLock_);

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(programCompletionEvent_);
    for (auto *co : coroutinePool_) {
        co->DestroyInternalResources();
        CoroutineManager::DestroyEntrypointfulCoroutine(co);
    }
    coroutinePool_.clear();
}

void StackfulCoroutineManager::AddToRegistry(Coroutine *co)
{
    co->GetVM()->GetGC()->OnThreadCreate(co);
    coroutines_.insert(co);
    coroutineCount_++;
    if (co->GetType() != Coroutine::Type::MUTATOR) {
        systemCoroutineCount_++;
    }
}

void StackfulCoroutineManager::RemoveFromRegistry(Coroutine *co)
{
    coroutines_.erase(co);
    coroutineCount_--;
    if (co->GetType() != Coroutine::Type::MUTATOR) {
        systemCoroutineCount_--;
    }
}

void StackfulCoroutineManager::RegisterCoroutine(Coroutine *co)
{
    os::memory::LockHolder lock(coroListLock_);
    AddToRegistry(co);
    // Propagate SUSPEND_REQUEST flag to the new coroutine to avoid the following situation:
    // * Main coro holds read lock of the MutatorLock.
    // * GC thread calls SuspendAll nad set SUSPEND_REQUEST flag to the main coro and
    //   tries to acquire write lock of the MutatorLock.
    // * Main coro creates a new coro and adds it to the coroutines_ list.
    // * SUSPEND_REQUEST is not set in the new coroutine
    // * New coro starts execution, acquires read lock of the MutatorLock and enters a long loop
    // * Main coro checks SUSPEND_REQUEST flag and blocks
    // * GC will not start becuase the new coro has no SUSPEND_REQUEST flag and it will never release the MutatorLock
    //
    // We need to propagate SUSPEND_REQUEST under the coroListLock_.
    // It guarantees that the flag is already set for the current coro and we need to propagate it
    // or GC will see the new coro in EnumerateAllThreads.
#ifndef ARK_HYBRID
    if (Thread::GetCurrent() != nullptr && Coroutine::GetCurrent() != nullptr &&
        Coroutine::GetCurrent()->IsSuspended() && !co->IsSuspended()) {
        co->SuspendImpl(true);
    }
#endif
}

bool StackfulCoroutineManager::TerminateCoroutine(Coroutine *co)
{
    if (co->HasManagedEntrypoint()) {
        // profiling: start interval here, end in ctxswitch after finalization request is done
        GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
    } else {
        // profiling: no need. MAIN and NATIVE EP coros are deleted from the SCHEDULER itself
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::TerminateCoroutine(): terminating "
                               << ((GetExistingWorkersCount() == 0) ? "MAIN..." : "NATIVE EP coro...");
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::TerminateCoroutine() started";
    co->NativeCodeEnd();
    co->UpdateStatus(ThreadStatus::TERMINATING);

    {
        os::memory::LockHolder lList(coroListLock_);
        RemoveFromRegistry(co);
#ifdef ARK_HYBRID
        co->GetThreadHolder()->UnregisterCoroutine(co);
#endif
        // We need collect TLAB metrics and clear TLAB before calling the manage thread destructor
        // because of the possibility heap use after free. This happening when GC starts execute ResetYoungAllocator
        // method which start iterate set of threads, collect TLAB metrics and clear TLAB. If thread was deleted from
        // set but we haven't destroyed the thread yet, GC won't collect metrics and can complete TLAB
        // deletion faster. And when we try to get the TLAB metrics in the destructor of managed thread, we will get
        // heap use after free
        co->CollectTLABMetrics();
        co->ClearTLAB();
        // DestroyInternalResources()/CleanupInternalResources() must be called in one critical section with
        // RemoveFromRegistry (under core_list_lock_). This functions transfer cards from coro's post_barrier buffer to
        // UpdateRemsetThread internally. Situation when cards still remain and UpdateRemsetThread cannot visit the
        // coro (because it is already removed) must be impossible.
        if (Runtime::GetOptions().IsUseCoroutinePool() && co->HasManagedEntrypoint()) {
            co->CleanupInternalResources();
        } else {
            co->DestroyInternalResources();
        }
        co->UpdateStatus(ThreadStatus::FINISHED);
    }
    Runtime::GetCurrent()->GetNotificationManager()->ThreadEndEvent(co);

    if (co->HasManagedEntrypoint()) {
        CheckProgramCompletion();
        GetCurrentWorker()->RequestFinalization(co);
    } else if (co->HasNativeEntrypoint()) {
        GetCurrentWorker()->RequestFinalization(co);
    } else {
        // entrypointless and NOT native: e.g. MAIN
        // (do nothing, as entrypointless coroutines should be destroyed manually)
    }

    return false;
}

size_t StackfulCoroutineManager::GetActiveWorkersCount() const
{
    os::memory::LockHolder lkWorkers(workersLock_);
    return activeWorkersCount_;
}

size_t StackfulCoroutineManager::GetExistingWorkersCount() const
{
    os::memory::LockHolder lkWorkers(workersLock_);
    return workers_.size();
}

void StackfulCoroutineManager::CheckProgramCompletion()
{
    os::memory::LockHolder lkCompletion(programCompletionLock_);

    size_t activeWorkerCoros = GetActiveWorkersCount();
    if (coroutineCount_ <= 1 + activeWorkerCoros) {  // 1 here is for MAIN
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::CheckProgramCompletion(): all coroutines finished execution!";
        // programCompletionEvent_ acts as a stackful-friendly cond var
        programCompletionEvent_->Happen();
    } else {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::CheckProgramCompletion(): still "
                               << coroutineCount_ - 1 - activeWorkerCoros << " coroutines exist...";
    }
}

CoroutineContext *StackfulCoroutineManager::CreateCoroutineContext(bool coroHasEntrypoint)
{
    return CreateCoroutineContextImpl(coroHasEntrypoint);
}

void StackfulCoroutineManager::DeleteCoroutineContext(CoroutineContext *ctx)
{
    FreeCoroutineStack(static_cast<StackfulCoroutineContext *>(ctx)->GetStackLoAddrPtr());
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(ctx);
}

size_t StackfulCoroutineManager::GetCoroutineCount()
{
    return coroutineCount_;
}

size_t StackfulCoroutineManager::GetCoroutineCountLimit()
{
    return coroutineCountLimit_;
}

bool StackfulCoroutineManager::Launch(CompletionEvent *completionEvent, Method *entrypoint,
                                      PandaVector<Value> &&arguments, CoroutineLaunchMode mode,
                                      CoroutinePriority priority, bool abortFlag)
{
    auto epInfo = Coroutine::ManagedEntrypointInfo {completionEvent, entrypoint, std::move(arguments)};
    return LaunchWithMode(std::move(epInfo), entrypoint->GetFullName(), mode, priority, false, abortFlag);
}

bool StackfulCoroutineManager::LaunchImmediately(CompletionEvent *completionEvent, Method *entrypoint,
                                                 PandaVector<Value> &&arguments, CoroutineLaunchMode mode,
                                                 CoroutinePriority priority, bool abortFlag)
{
    auto epInfo = Coroutine::ManagedEntrypointInfo {completionEvent, entrypoint, std::move(arguments)};
    return LaunchWithMode(std::move(epInfo), entrypoint->GetFullName(), mode, priority, true, abortFlag);
}

bool StackfulCoroutineManager::LaunchNative(NativeEntrypointFunc epFunc, void *param, PandaString coroName,
                                            CoroutineLaunchMode mode, CoroutinePriority priority, bool abortFlag)
{
    auto epInfo = Coroutine::NativeEntrypointInfo {epFunc, param};
    return LaunchWithMode(epInfo, std::move(coroName), mode, priority, false, abortFlag);
}

void StackfulCoroutineManager::Await(CoroutineEvent *awaitee)
{
    ASSERT_NATIVE_CODE();
    // profiling
    ScopedCoroutineStats s(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);

    ASSERT(awaitee != nullptr);
    [[maybe_unused]] auto *waiter = Coroutine::GetCurrent();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Await started by " + waiter->GetName();

    if (awaitee->Happened()) {
        awaitee->Unlock();
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Await finished (no await happened)";
        return;
    }

    GetCurrentWorker()->WaitForEvent(awaitee);
    // NB: at this point the awaitee is likely already deleted
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Await finished by " + waiter->GetName();
}

void StackfulCoroutineManager::UnblockWaiters(CoroutineEvent *blocker)
{
    // profiling: this function can be called either independently or as a path of some other SCH sequence,
    // hence using the "weak" stats collector
    ScopedCoroutineStats s(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL, true);

    os::memory::LockHolder lkWorkers(workersLock_);
    ASSERT(blocker != nullptr);
#ifndef NDEBUG
    {
        os::memory::LockHolder lkBlocker(*blocker);
        ASSERT(blocker->Happened());
    }
#endif

    for (auto *w : workers_) {
        w->UnblockWaiters(blocker);
    }
}

void StackfulCoroutineManager::Schedule()
{
    // profiling
    ScopedCoroutineStats s(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Schedule() request from "
                           << Coroutine::GetCurrent()->GetName();
    GetCurrentWorker()->RequestSchedule();
}

bool StackfulCoroutineManager::EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                                                    unsigned int xorMask) const
{
    os::memory::LockHolder lock(coroListLock_);
    for (auto *t : coroutines_) {
        if (!ApplyCallbackToThread(cb, t, incMask, xorMask)) {
            return false;
        }
    }
    return true;
}

void StackfulCoroutineManager::SuspendAllThreads()
{
    os::memory::LockHolder lock(coroListLock_);
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::SuspendAllThreads started";
    for (auto *t : coroutines_) {
        t->SuspendImpl(true);
    }
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::SuspendAllThreads finished";
}

void StackfulCoroutineManager::ResumeAllThreads()
{
    os::memory::LockHolder lock(coroListLock_);
    for (auto *t : coroutines_) {
        t->ResumeImpl(true);
    }
}

bool StackfulCoroutineManager::IsRunningThreadExist()
{
    UNREACHABLE();
    // NOTE(konstanting): correct implementation. Which coroutine do we consider running?
    return false;
}

void StackfulCoroutineManager::WaitForDeregistration()
{
    // profiling: start interval here, end in ctxswitch (if needed)
    GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
    //
    MainCoroutineCompleted();
}

void StackfulCoroutineManager::ReuseCoroutineInstance(Coroutine *co, EntrypointInfo &&epInfo, PandaString name,
                                                      CoroutinePriority priority)
{
    auto *ctx = co->GetContext<CoroutineContext>();
    co->ReInitialize(std::move(name), ctx, std::move(epInfo), priority);
}

Coroutine *StackfulCoroutineManager::TryGetCoroutineFromPool()
{
    os::memory::LockHolder lkPool(coroPoolLock_);
    if (coroutinePool_.empty()) {
        return nullptr;
    }
    Coroutine *co = coroutinePool_.back();
    coroutinePool_.pop_back();
    return co;
}

StackfulCoroutineWorker *StackfulCoroutineManager::ChooseWorkerForCoroutine(Coroutine *co)
{
    ASSERT(co != nullptr);
    auto maskValue = co->GetContext<StackfulCoroutineContext>()->GetAffinityMask();
    return ChooseWorkerImpl(WorkerSelectionPolicy::LEAST_LOADED, maskValue);
}

stackful_coroutines::AffinityMask StackfulCoroutineManager::CalcAffinityMaskFromLaunchMode(CoroutineLaunchMode mode)
{
    /**
     * launch mode \ policy      DEFAULT                         NON_MAIN
     *   DEFAULT                ->least busy, allow migration   ->least busy, allow migration, disallow <main>
     *   SAME_WORKER            ->same, forbid migration        ->same, forbid migration
     *   MAIN_WORKER            ->main, forbid migration        ->main, forbid migration
     *   EXCLUSIVE              ->least busy, forbid migration  ->least busy, forbid migration, disallow <main>
     */

    if (mode == CoroutineLaunchMode::SAME_WORKER) {
        std::bitset<stackful_coroutines::MAX_WORKERS_COUNT> mask(stackful_coroutines::AFFINITY_MASK_NONE);
        mask.set(GetCurrentWorker()->GetId());
        return mask.to_ullong();
    }

    if (mode == CoroutineLaunchMode::MAIN_WORKER) {
        std::bitset<stackful_coroutines::MAX_WORKERS_COUNT> mask(stackful_coroutines::AFFINITY_MASK_NONE);
        mask.set(stackful_coroutines::MAIN_WORKER_ID);
        return mask.to_ullong();
    }

    // CoroutineLaunchMode::EXCLUSIVE is not supported yet (but will be)
    ASSERT(mode == CoroutineLaunchMode::DEFAULT);

    std::bitset<stackful_coroutines::MAX_WORKERS_COUNT> mask(stackful_coroutines::AFFINITY_MASK_FULL);
    switch (GetSchedulingPolicy()) {
        case CoroutineSchedulingPolicy::NON_MAIN_WORKER: {
            mask.reset(stackful_coroutines::MAIN_WORKER_ID);
            break;
        }
        default:
        case CoroutineSchedulingPolicy::ANY_WORKER:
            break;
    }
    return mask.to_ullong();
}

Coroutine *StackfulCoroutineManager::GetCoroutineInstanceForLaunch(EntrypointInfo &&epInfo, PandaString &&coroName,
                                                                   CoroutinePriority priority,
                                                                   stackful_coroutines::AffinityMask affinityMask,
                                                                   bool abortFlag)
{
    Coroutine *co = nullptr;
    if (Runtime::GetOptions().IsUseCoroutinePool()) {
        co = TryGetCoroutineFromPool();
    }
    if (co != nullptr) {
        ReuseCoroutineInstance(co, std::move(epInfo), std::move(coroName), priority);
    } else {
        co = CreateCoroutineInstance(std::move(epInfo), std::move(coroName), Coroutine::Type::MUTATOR, priority);
    }
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::GetCoroutineInstanceForLaunch: failed to create a coroutine!";
        return co;
    }
    co->SetAbortFlag(abortFlag);
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(co);
    co->GetContext<StackfulCoroutineContext>()->SetAffinityMask(affinityMask);
    return co;
}

bool StackfulCoroutineManager::LaunchImpl(EntrypointInfo &&epInfo, PandaString &&coroName, CoroutineLaunchMode mode,
                                          CoroutinePriority priority, bool abortFlag)
{
#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl begin");
#endif
    Coroutine *co = nullptr;
    auto affinityMask = CalcAffinityMaskFromLaunchMode(mode);
    co = GetCoroutineInstanceForLaunch(std::move(epInfo), std::move(coroName), priority, affinityMask, abortFlag);
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchImpl: failed to create a coroutine!";
        return false;
    }
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        auto *w = ChooseWorkerForCoroutine(co);
        ASSERT(w != nullptr);
        w->AddRunnableCoroutine(co);
    }
#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl end");
#endif
    return true;
}

bool StackfulCoroutineManager::LaunchImmediatelyImpl(EntrypointInfo &&epInfo, PandaString &&coroName,
                                                     CoroutineLaunchMode mode, CoroutinePriority priority,
                                                     bool abortFlag)
{
    Coroutine *co = nullptr;
    auto affinityMask = CalcAffinityMaskFromLaunchMode(mode);

    ASSERT(affinityMask == CalcAffinityMaskFromLaunchMode(CoroutineLaunchMode::SAME_WORKER));

    co = GetCoroutineInstanceForLaunch(std::move(epInfo), std::move(coroName), priority, affinityMask, abortFlag);
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchImmediatelyImpl: failed to create a coroutine!";
        return false;
    }
    StackfulCoroutineWorker *w = nullptr;
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        w = ChooseWorkerForCoroutine(co);
    }
    ASSERT(w != nullptr);
    // since we are going to switch the context, we have to close the interval
    GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::LAUNCH);
    co->SetImmediateLauncher(Coroutine::GetCurrent());
    w->AddCreatedCoroutineAndSwitchToIt(co);
    // resume the interval once we schedule the original coro again
    GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::LAUNCH);

    return true;
}

bool StackfulCoroutineManager::LaunchWithMode(Coroutine::EntrypointInfo &&epInfo, PandaString &&coroName,
                                              CoroutineLaunchMode mode, CoroutinePriority priority,
                                              bool launchImmediately, bool abortFlag)
{
    // profiling: scheduler and launch time
    ScopedCoroutineStats sSch(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);
    ScopedCoroutineStats sLaunch(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::LAUNCH);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchWithMode started";

    auto *co = Coroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *w = co->GetContext<StackfulCoroutineContext>()->GetWorker();
    mode = (mode == CoroutineLaunchMode::DEFAULT && w->InExclusiveMode()) ? CoroutineLaunchMode::SAME_WORKER : mode;
    bool result = false;
    if (launchImmediately) {
        result = LaunchImmediatelyImpl(std::move(epInfo), std::move(coroName), mode, priority, abortFlag);
    } else {
        result = LaunchImpl(std::move(epInfo), std::move(coroName), mode, priority, abortFlag);
    }
    if (!result) {
        // let's count all launch failures as "limit exceeded" for now.
        // Later on we can think of throwing different errors for different reasons.
        ThrowCoroutinesLimitExceedError(
            "Unable to create a new coroutine: reached the limit for the number of existing coroutines.");
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchWithMode finished";
    return result;
}

void StackfulCoroutineManager::DumpCoroutineStats() const
{
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager: dumping performance statistics...";
    std::cout << "=== Coroutine statistics begin ===" << std::endl;
    std::cout << stats_.GetFullStatistics(finalizedWorkerStats_);
    std::cout << "=== Coroutine statistics end ===" << std::endl;
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager: performance statistics dumped successfully.";
}

void StackfulCoroutineManager::ListUnhandledEventsOnProgramExit()
{
    auto *coro = Coroutine::GetCurrent();
    ASSERT(coro != nullptr);
    coro->ListUnhandledEventsOnProgramExit();
}

void StackfulCoroutineManager::WaitForNonMainCoroutinesCompletion()
{
    os::memory::LockHolder lkCompletion(programCompletionLock_);
    // It's neccessary to read activeWorkersCount before coroutineCount to avoid deadlock
    do {
        while (GetActiveWorkersCount() + 1 < coroutineCount_) {  // 1 is for MAIN
            programCompletionEvent_->SetNotHappened();
            programCompletionEvent_->Lock();
            programCompletionLock_.Unlock();
            GetCurrentWorker()->WaitForEvent(programCompletionEvent_);
            LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::WaitForNonMainCoroutinesCompletion(): possibly "
                                      "spurious wakeup from wait...";
            // NOTE(konstanting, #IAD5MH): test for the spurious wakeup
            programCompletionLock_.Lock();
        }
        ListUnhandledEventsOnProgramExit();
    } while (GetActiveWorkersCount() + 1 < coroutineCount_);  // 1 is for MAIN
    // coroutineCount_ < 1 + GetActiveWorkersCount() in case of concurrent EWorker destroy
    // in this case coroutineCount_ >= 1 + GetActiveWorkersCount() - ExclusiveWorkersCount()
    ASSERT(!(GetActiveWorkersCount() + 1 < coroutineCount_));
}

void StackfulCoroutineManager::MainCoroutineCompleted()
{
    // precondition: MAIN is already in the native mode
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): STARTED";
    // block till only schedule loop coroutines are present
    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): waiting for other coroutines to complete";
    WaitForNonMainCoroutinesCompletion();
    if (enableMigration_) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): stop manager thread";
        StopManagerThread();
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping workers";
    {
        os::memory::LockHolder lock(workersLock_);
        for (auto *worker : workers_) {
            worker->SetActive(false);
        }
        while (activeWorkersCount_ > 1) {  // 1 is for MAIN
            // profiling: the SCH interval is expected to be started after the ctxswitch
            GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::SCH_ALL);
            // NOTE(konstanting, #IAD5MH): need timed wait?..
            workersCv_.Wait(&workersLock_);
            // profiling: we don't want to profile the sleeping state
            GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
        }
    }
    // Only system coroutines and current coro (MAIN) are left (1 is for MAIN)
    ASSERT(activeCoroutines_ == systemCoroutineCount_ + 1);

    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping await loop on the main worker";
    while (coroutineCount_ > 1) {
        GetCurrentWorker()->FinalizeFiberScheduleLoop();
    }
    // profiling: the SCH interval is expected to be started after the ctxswitch
    GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::SCH_ALL);

    OnWorkerShutdown(GetCurrentWorker());

#ifndef NDEBUG
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        ASSERT(workers_.empty());
        ASSERT(activeWorkersCount_ == 0);
    }
#endif

    if (stats_.IsEnabled()) {
        DumpCoroutineStats();
    }
    stats_.Disable();

    // We need to lock programCompletionLock_ here to call
    // programCompletionLock_.Unlock() in ExclusiveWorker before runtime destruction
    os::memory::LockHolder lkCompletion(programCompletionLock_);
    GetCurrentContext()->MainThreadFinished();
    // MAIN finished, all workers are deleted, no active coros remain
    ASSERT(activeCoroutines_ == 0);
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): DONE";
}

StackfulCoroutineContext *StackfulCoroutineManager::GetCurrentContext()
{
    auto *co = Coroutine::GetCurrent();
    ASSERT(co != nullptr);
    return co->GetContext<StackfulCoroutineContext>();
}

StackfulCoroutineWorker *StackfulCoroutineManager::GetCurrentWorker()
{
    return GetCurrentContext()->GetWorker();
}

bool StackfulCoroutineManager::IsMainWorker(Coroutine *co) const
{
    auto *worker = co->GetContext<StackfulCoroutineContext>()->GetWorker();
    return worker->IsMainWorker();
}

void StackfulCoroutineManager::DestroyEntrypointfulCoroutine(Coroutine *co)
{
    if (Runtime::GetOptions().IsUseCoroutinePool() && co->HasManagedEntrypoint()) {
        co->CleanUp();
        os::memory::LockHolder lock(coroPoolLock_);
        coroutinePool_.push_back(co);
    } else {
        CoroutineManager::DestroyEntrypointfulCoroutine(co);
    }
}

StackfulCoroutineContext *StackfulCoroutineManager::CreateCoroutineContextImpl(bool needStack)
{
    uint8_t *stack = nullptr;
    size_t stackSizeBytes = 0;
    if (needStack) {
        stack = AllocCoroutineStack();
        if (stack == nullptr) {
            return nullptr;
        }
        stackSizeBytes = coroStackSizeBytes_;
    }
    return Runtime::GetCurrent()->GetInternalAllocator()->New<StackfulCoroutineContext>(stack, stackSizeBytes);
}

Coroutine *StackfulCoroutineManager::CreateNativeCoroutine(Runtime *runtime, PandaVM *vm,
                                                           Coroutine::NativeEntrypointInfo::NativeEntrypointFunc entry,
                                                           void *param, PandaString name, Coroutine::Type type,
                                                           CoroutinePriority priority)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    StackfulCoroutineContext *ctx = CreateCoroutineContextImpl(true);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    auto *co = GetCoroutineFactory()(runtime, vm, std::move(name), ctx, Coroutine::NativeEntrypointInfo(entry, param),
                                     type, priority);
    ASSERT(co != nullptr);

    // Let's assume that even the "native" coroutine can eventually try to execute some managed code.
    // In that case pre/post barrier buffers are necessary.
    co->InitBuffers();
    return co;
}

void StackfulCoroutineManager::DestroyNativeCoroutine(Coroutine *co)
{
    DestroyEntrypointlessCoroutine(co);
}

void StackfulCoroutineManager::OnCoroBecameActive(Coroutine *co)
{
    ASSERT(co->IsActive());
    IncrementActiveCoroutines();
    co->GetWorker()->OnCoroBecameActive(co);
}

void StackfulCoroutineManager::OnCoroBecameNonActive([[maybe_unused]] Coroutine *co)
{
    ASSERT(!co->IsActive());
    DecrementActiveCoroutines();
}

void StackfulCoroutineManager::OnNativeCallExit(Coroutine *co)
{
    if (IsDrainQueueInterfaceEnabled()) {
        // A temporary hack for draining the coroutine queue on the current worker.
        // Will stay there until we have the proper design for the execution model and
        // the rules for interaction with the app framework.
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnNativeCallExit(): START DRAINING COROQUEUE";

        // precondition: the event is handled for the current coroutine
        ASSERT(co == Coroutine::GetCurrent());
        auto *worker = GetCurrentWorker();
        if (!worker->IsMainWorker() && !worker->InExclusiveMode()) {
            return;
        }
        ScopedNativeCodeThread nativeCode(co);
        while (worker->GetRunnablesCount(Coroutine::Type::MUTATOR) > 0) {
            GetCurrentWorker()->RequestSchedule();
        }

        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnNativeCallExit(): STOP DRAINING COROQUEUE";
    }
}

void StackfulCoroutineManager::IncrementActiveCoroutines()
{
    activeCoroutines_++;
}

void StackfulCoroutineManager::DecrementActiveCoroutines()
{
    [[maybe_unused]] uint32_t old = activeCoroutines_--;
    ASSERT(old > 0);
}

bool StackfulCoroutineManager::IsNoActiveMutatorsExceptCurrent()
{
    // activeCoroutines_ == 1 means that only current or main mutator is left
    // activeCoroutines_ == 0 means that main mutator is terminating
    // or all coroutines are blocked (a deadlock in managed code happened)

    // NOTE(konstanting): need to reevaluate the necessity of locks here as
    // atomics difference is somewhat confusing. Also, we may have concurrent access to them.
    return (activeCoroutines_ - systemCoroutineCount_) <= 1;
}

Coroutine *StackfulCoroutineManager::CreateExclusiveWorkerForThread(Runtime *runtime, PandaVM *vm)
{
    ASSERT(Thread::GetCurrent() == nullptr);

    // actually we need this lock due to worker limit
    os::memory::LockHolder eWorkerLock(eWorkerCreationLock_);

    if (IsExclusiveWorkersLimitReached()) {
        LOG(DEBUG, COROUTINES) << "The program reached the limit of exclusive workers";
        return nullptr;
    }

    auto *eWorker = CreateWorker(runtime, vm, StackfulCoroutineWorker::ScheduleLoopType::FIBER, "[e-worker] ");
    ASSERT(eWorker != nullptr);
    eWorker->SetExclusiveMode(true);
    eWorker->DisableForCrossWorkersLaunch();
    auto *eCoro = CreateEntrypointlessCoroutine(runtime, vm, true, "[ea_coro] " + eWorker->GetName(),
                                                Coroutine::Type::MUTATOR, CoroutinePriority::MEDIUM_PRIORITY);
    ASSERT(eCoro != nullptr);
    eWorker->AddRunningCoroutine(eCoro);
    OnWorkerStartup(eWorker);

    ASSERT(Coroutine::GetCurrent() == eCoro);
    return eCoro;
}

bool StackfulCoroutineManager::DestroyExclusiveWorker()
{
    auto *eWorker = GetCurrentWorker();
    if (!eWorker->InExclusiveMode()) {
        LOG(DEBUG, COROUTINES) << "Trying to destroy not exclusive worker";
        return false;
    }

    eWorker->CompleteAllAffinedCoroutines();

    eWorker->SetActive(false);
    eWorker->FinalizeFiberScheduleLoop();

    CheckProgramCompletion();

    auto *eaCoro = Coroutine::GetCurrent();
    programCompletionLock_.Lock();
    DestroyEntrypointlessCoroutine(eaCoro);
    Coroutine::SetCurrent(nullptr);

    OnWorkerShutdown(eWorker);
    programCompletionLock_.Unlock();
    return true;
}

bool StackfulCoroutineManager::IsExclusiveWorkersLimitReached() const
{
    bool limitIsReached = GetActiveWorkersCount() - commonWorkersCount_ >= exclusiveWorkersLimit_;
    LOG_IF(limitIsReached, DEBUG, COROUTINES) << "The programm reached the limit of exclusive workers";
    return limitIsReached;
}

void StackfulCoroutineManager::ManagerThreadProc()
{
    // calculate the time after 5 seconds.
    auto nextCheckTime = ark::os::time::GetClockTimeInMilli() + DETECTION_INTERVAL_VALUE;
    while (managerRunning_) {
        {
            os::memory::LockHolder lock(managerMutex_);
            managerCv_.TimedWait(&managerMutex_, DETECTION_INTERVAL_VALUE);  // wait 5 seconds.
            if (!managerRunning_) {
                LOG(DEBUG, COROUTINES) << "The manager thread stops running.";
                break;
            }
        }
        uint32_t count = migrateCount_.exchange(0);
        while (count > 0) {
            MigrateCoroutinesInward(count);
        }
        auto now = ark::os::time::GetClockTimeInMilli();
        if (now >= nextCheckTime) {
            CheckForBlockedWorkers();
            nextCheckTime = now + DETECTION_INTERVAL_VALUE;  // update to the next 5 second.
        }
    }
}

void StackfulCoroutineManager::CheckForBlockedWorkers()
{
    os::memory::LockHolder lock(workersLock_);
    ASSERT(!workers_.empty());
    for (auto *w : workers_) {
        // skip main worker and exclusive workers
        if (w->IsMainWorker() || w->InExclusiveMode()) {
            continue;
        }
        w->MigrateCorosOutwardsIfBlocked();
    }
}

void StackfulCoroutineManager::MigrateCoroutinesInward(uint32_t &count)
{
    auto affinityMask = CalcAffinityMaskFromLaunchMode(CoroutineLaunchMode::DEFAULT);
    os::memory::LockHolder lkWorkers(workersLock_);
    StackfulCoroutineWorker *from = ChooseWorkerImpl(WorkerSelectionPolicy::MOST_LOADED, affinityMask);
    if (from == nullptr || from->IsIdle()) {
        LOG(DEBUG, COROUTINES) << "no suitable worker.";
        count = 0;
        return;
    }

    for (auto *w : workers_) {
        // skip main worker and exclusive workers
        if (w->IsMainWorker() || w->InExclusiveMode()) {
            continue;
        }
        if (w->MigrateFrom(from)) {
            --count;
            return;
        }
    }
    count = 0;
}

bool StackfulCoroutineManager::MigrateCoroutinesOutward(StackfulCoroutineWorker *from)
{
    if (from->IsIdle()) {
        LOG(DEBUG, COROUTINES) << "The worker is idle, stop migration outward.";
        return false;
    }
    auto affinityMask = CalcAffinityMaskFromLaunchMode(CoroutineLaunchMode::DEFAULT);
    os::memory::LockHolder lkWorkers(workersLock_);
    StackfulCoroutineWorker *to = ChooseWorkerImpl(WorkerSelectionPolicy::LEAST_LOADED, affinityMask);
    if (to == nullptr || to == from) {
        LOG(DEBUG, COROUTINES) << "no suitable worker.";
        return false;
    }
    from->MigrateTo(to);
    return true;
}

StackfulCoroutineWorker *StackfulCoroutineManager::ChooseWorkerImpl(WorkerSelectionPolicy policy, size_t maskValue)
{
    std::bitset<stackful_coroutines::MAX_WORKERS_COUNT> affinityBits(maskValue);
    auto preferFirstOverSecond = [&affinityBits, &policy](const StackfulCoroutineWorker *first,
                                                          const StackfulCoroutineWorker *second) {
        if (!affinityBits.test(first->GetId())) {
            return false;
        }
        if (!affinityBits.test(second->GetId())) {
            return true;
        }
        // choosing the least loaded worker from the allowed worker set
        if (policy == WorkerSelectionPolicy::LEAST_LOADED) {
            return first->GetLoadFactor() < second->GetLoadFactor();
        }
        // choosing the most loaded worker from the allowed worker set
        return first->GetLoadFactor() > second->GetLoadFactor();
    };

    if (workers_.empty()) {
        LOG(DEBUG, COROUTINES) << "workers is empty.";
        return nullptr;
    }
#ifndef NDEBUG
    LOG(DEBUG, COROUTINES) << "Evaluating load factors:";
    for (auto w : workers_) {
        LOG(DEBUG, COROUTINES) << w->GetName() << ": LF = " << w->GetLoadFactor();
    }
#endif
    std::vector<StackfulCoroutineWorker *> suitableWorkers;
    // skip exclusive and finalizing workers
    std::copy_if(workers_.begin(), workers_.end(), std::back_inserter(suitableWorkers), [affinityBits](auto *w) {
        return !w->IsDisabledForCrossWorkersLaunch() || affinityBits.test(w->GetId());
    });
    ASSERT(!suitableWorkers.empty());

    auto wIt = std::min_element(suitableWorkers.begin(), suitableWorkers.end(), preferFirstOverSecond);
    LOG(DEBUG, COROUTINES) << "Chose worker: " << (*wIt)->GetName();

    return *wIt;
}

void StackfulCoroutineManager::TriggerMigration()
{
    auto *worker = GetCurrentWorker();
    if (worker->IsMainWorker() || worker->InExclusiveMode()) {
        return;
    }
    if (!IsMigrationEnabled()) {
        LOG(DEBUG, COROUTINES) << "Migration is not supported.";
        return;
    }
    ++migrateCount_;
    LOG(DEBUG, COROUTINES) << "trigger migration.";
    os::memory::LockHolder lock(managerMutex_);
    managerCv_.Signal();
}

void StackfulCoroutineManager::MigrateAwakenedCoro(Coroutine *co)
{
    os::memory::LockHolder lkWorkers(workersLock_);
    auto *w = ChooseWorkerForCoroutine(co);
    ASSERT(w != nullptr);
    w->AddRunnableCoroutine(co);
}

void StackfulCoroutineManager::StartManagerThread()
{
    // create a thread to detect worker blocking and perform coroutine migration
    managerRunning_ = true;
    managerThread_ = std::thread(&StackfulCoroutineManager::ManagerThreadProc, this);
    os::thread::SetThreadName(managerThread_.native_handle(), "managerThread");
}

void StackfulCoroutineManager::StopManagerThread()
{
    if (!managerRunning_) {
        return;
    }
    {
        os::memory::LockHolder lock(managerMutex_);
        managerRunning_ = false;
        managerCv_.SignalAll();
    }
    if (managerThread_.joinable()) {
        managerThread_.join();
    }
}

void StackfulCoroutineManager::PreZygoteFork()
{
    WaitForNonMainCoroutinesCompletion();
    if (enableMigration_) {
        StopManagerThread();
    }

    FinalizeWorkers(commonWorkersCount_ - 1, Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
}

void StackfulCoroutineManager::PostZygoteFork()
{
    os::memory::LockHolder lh(workersLock_);
    Runtime *runtime = Runtime::GetCurrent();
    CreateWorkers(commonWorkersCount_ - 1, runtime, runtime->GetPandaVM());
    if (enableMigration_) {
        StartManagerThread();
    }
}

void StackfulCoroutineManager::CalculateWorkerLimits(const CoroutineManagerConfig &config,
                                                     size_t &exclusiveWorkersLimit, size_t &commonWorkersLimit)
{
    // 1 is for MAIN
    size_t eWorkersLimit =
        std::min(stackful_coroutines::MAX_WORKERS_COUNT - 1, static_cast<size_t>(config.exclusiveWorkersLimit));

#ifdef PANDA_ETS_INTEROP_JS
    // 2 is for taskpool execution engine eaworker
    bool res = Runtime::GetOptions().IsTaskpoolSupportInterop(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));
    if (res) {
        eWorkersLimit += stackful_coroutines::TASKPOOL_EAWORKER_LIMIT;
    }
#endif
    // create and activate workers
    size_t numberOfAvailableCores = std::max(std::thread::hardware_concurrency() / 4ULL, 2ULL);

    // workaround for issue #21582
    const size_t maxCommonWorkers =
        std::max(stackful_coroutines::MAX_WORKERS_COUNT - eWorkersLimit, static_cast<size_t>(2ULL));

    commonWorkersLimit = (config.workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO)
                             ? std::min(numberOfAvailableCores, maxCommonWorkers)
                             : std::min(static_cast<size_t>(config.workersCount), maxCommonWorkers);
    if (config.workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): AUTO mode selected, will set number of coroutine "
                                  "common workers to number of CPUs / 4, but not less than 2 and no more than "
                               << maxCommonWorkers << " = " << commonWorkersLimit;
    }
    ASSERT(commonWorkersLimit > 0);

    exclusiveWorkersLimit = std::min(stackful_coroutines::MAX_WORKERS_COUNT - commonWorkersLimit, eWorkersLimit);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): EWorkers limit is set to " << exclusiveWorkersLimit
                           << ", when suggested " << eWorkersLimit;
}
}  // namespace ark
