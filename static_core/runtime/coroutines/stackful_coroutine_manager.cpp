/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/include/thread_scopes.h"
#include "libpandabase/macros.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/panda_vm.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"

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

void StackfulCoroutineManager::CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    bool isStatsEnabled = stats_.IsEnabled();

    auto *wMain =
        allocator->New<StackfulCoroutineWorker>(runtime, vm, this, StackfulCoroutineWorker::ScheduleLoopType::FIBER,
                                                "[main] worker 0", stackful_coroutines::MAIN_WORKER_ID);
    if (isStatsEnabled) {
        wMain->GetPerfStats().Enable();
    }
    workers_.push_back(wMain);
    ASSERT(workers_[stackful_coroutines::MAIN_WORKER_ID] == wMain);
    ASSERT(wMain->GetId() == stackful_coroutines::MAIN_WORKER_ID);

    for (uint32_t i = 1; i < howMany; ++i) {
        auto *w = allocator->New<StackfulCoroutineWorker>(
            runtime, vm, this, StackfulCoroutineWorker::ScheduleLoopType::THREAD, "worker " + ToPandaString(i), i);
        if (isStatsEnabled) {
            w->GetPerfStats().Enable();
        }
        workers_.push_back(w);
        ASSERT(workers_[i] == w);
        ASSERT(w->GetId() == i);
    }

    auto *mainCo = CreateMainCoroutine(runtime, vm);
    mainCo->GetContext<StackfulCoroutineContext>()->SetWorker(wMain);
    Coroutine::SetCurrent(mainCo);
    activeWorkersCount_ = 1;  // 1 is for MAIN

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::CreateWorkers(): waiting for workers startup";
    while (activeWorkersCount_ < howMany) {
        // NOTE(konstanting, #I67QXC): need timed wait?..
        workersCv_.Wait(&workersLock_);
    }
}

void StackfulCoroutineManager::OnWorkerShutdown()
{
    os::memory::LockHolder lock(workersLock_);
    --activeWorkersCount_;
    workersCv_.Signal();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnWorkerShutdown(): COMPLETED, workers left = "
                           << activeWorkersCount_;
}

void StackfulCoroutineManager::OnWorkerStartup()
{
    os::memory::LockHolder lock(workersLock_);
    ++activeWorkersCount_;
    workersCv_.Signal();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnWorkerStartup(): COMPLETED, active workers = "
                           << activeWorkersCount_;
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
    // set limits
    coroStackSizeBytes_ = Runtime::GetCurrent()->GetOptions().GetCoroutineStackSizePages() * os::mem::GetPageSize();
    if (coroStackSizeBytes_ != AlignUp(coroStackSizeBytes_, PANDA_POOL_ALIGNMENT_IN_BYTES)) {
        size_t alignmentPages = PANDA_POOL_ALIGNMENT_IN_BYTES / os::mem::GetPageSize();
        LOG(FATAL, COROUTINES) << "Coroutine stack size should be >= " << alignmentPages
                               << " pages and should be aligned to " << alignmentPages << "-page boundary!";
    }
    size_t coroStackAreaSizeBytes = Runtime::GetCurrent()->GetOptions().GetCoroutinesStackMemLimit();
    coroutineCountLimit_ = coroStackAreaSizeBytes / coroStackSizeBytes_;
    jsMode_ = config.emulateJs;

    // create and activate workers
    size_t numberOfAvailableCores = std::max(std::thread::hardware_concurrency() / 4ULL, 2ULL);
    size_t targetNumberOfWorkers =
        (config.workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO)
            ? std::min(numberOfAvailableCores, stackful_coroutines::MAX_WORKERS_COUNT)
            : std::min(static_cast<size_t>(config.workersCount), stackful_coroutines::MAX_WORKERS_COUNT);
    if (config.workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): AUTO mode selected, will set number of coroutine "
                                  "workers to number of CPUs / 4, but not less than 2 = "
                               << targetNumberOfWorkers;
    }
    os::memory::LockHolder lock(workersLock_);
    CreateWorkers(targetNumberOfWorkers, runtime, vm);
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): successfully created and activated " << workers_.size()
                           << " coroutine workers";
    programCompletionEvent_ = Runtime::GetCurrent()->GetInternalAllocator()->New<GenericEvent>(this);
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
    os::memory::LockHolder lock(coroListLock_);
    co->GetVM()->GetGC()->OnThreadCreate(co);
    coroutines_.insert(co);
    coroutineCount_++;
}

void StackfulCoroutineManager::RemoveFromRegistry(Coroutine *co)
{
    coroutines_.erase(co);
    coroutineCount_--;
}

void StackfulCoroutineManager::RegisterCoroutine(Coroutine *co)
{
    AddToRegistry(co);
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
    if (coroutineCount_ == 1 + activeWorkerCoros) {  // 1 here is for MAIN
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

Coroutine *StackfulCoroutineManager::Launch(CompletionEvent *completionEvent, Method *entrypoint,
                                            PandaVector<Value> &&arguments, CoroutineLaunchMode mode)
{
    // profiling: scheduler and launch time
    ScopedCoroutineStats sSch(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);
    ScopedCoroutineStats sLaunch(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::LAUNCH);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Launch started";

    auto *result = LaunchImpl(completionEvent, entrypoint, std::move(arguments), mode);
    if (result == nullptr) {
        ThrowOutOfMemoryError("Launch failed");
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Launch finished";
    return result;
}

void StackfulCoroutineManager::Await(CoroutineEvent *awaitee)
{
    // profiling
    ScopedCoroutineStats s(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);

    ASSERT(awaitee != nullptr);
    [[maybe_unused]] auto *waiter = Coroutine::GetCurrent();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Await started by " + waiter->GetName();
    if (!GetCurrentWorker()->WaitForEvent(awaitee)) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Await finished (no await happened)";
        return;
    }
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

void StackfulCoroutineManager::ReuseCoroutineInstance(Coroutine *co, CompletionEvent *completionEvent,
                                                      Method *entrypoint, PandaVector<Value> &&arguments,
                                                      PandaString name)
{
    auto *ctx = co->GetContext<CoroutineContext>();
    co->ReInitialize(std::move(name), ctx,
                     Coroutine::ManagedEntrypointInfo {completionEvent, entrypoint, std::move(arguments)});
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
    // currently this function does only the initial worker appointment
    // but eventually it will support coroutine migration too

    auto maskValue = co->GetContext<StackfulCoroutineContext>()->GetAffinityMask();
    std::bitset<stackful_coroutines::MAX_WORKERS_COUNT> affinityBits(maskValue);
    LOG(DEBUG, COROUTINES) << "Choosing worker for coro " << co->GetName() << " with affinity mask = " << affinityBits;

    // choosing the least loaded worker from the allowed worker set
    auto preferFirstOverSecond = [&affinityBits](const StackfulCoroutineWorker *first,
                                                 const StackfulCoroutineWorker *second) {
        if (!affinityBits.test(first->GetId())) {
            return false;
        }
        if (!affinityBits.test(second->GetId())) {
            return true;
        }
        return first->GetLoadFactor() < second->GetLoadFactor();
    };

    os::memory::LockHolder lkWorkers(workersLock_);
#ifndef NDEBUG
    LOG(DEBUG, COROUTINES) << "Evaluating load factors:";
    for (auto w : workers_) {
        LOG(DEBUG, COROUTINES) << w->GetName() << ": LF = " << w->GetLoadFactor();
    }
#endif
    auto wIt = std::min_element(workers_.begin(), workers_.end(), preferFirstOverSecond);
    LOG(DEBUG, COROUTINES) << "Chose worker: " << (*wIt)->GetName();
    return *wIt;
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
        case CoroutineSchedulingPolicy::DEFAULT:
            break;
    }
    return mask.to_ullong();
}

Coroutine *StackfulCoroutineManager::LaunchImpl(CompletionEvent *completionEvent, Method *entrypoint,
                                                PandaVector<Value> &&arguments, CoroutineLaunchMode mode)
{
#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl begin");
#endif
    auto coroName = entrypoint->GetFullName();

    Coroutine *co = nullptr;
    if (Runtime::GetOptions().IsUseCoroutinePool()) {
        co = TryGetCoroutineFromPool();
    }
    if (co != nullptr) {
        ReuseCoroutineInstance(co, completionEvent, entrypoint, std::move(arguments), std::move(coroName));
    } else {
        co = CreateCoroutineInstance(completionEvent, entrypoint, std::move(arguments), std::move(coroName));
    }
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchImpl: failed to create a coroutine!";
        return co;
    }
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(co);

    auto affinityMask = CalcAffinityMaskFromLaunchMode(mode);
    co->GetContext<StackfulCoroutineContext>()->SetAffinityMask(affinityMask);
    auto *w = ChooseWorkerForCoroutine(co);
    w->AddRunnableCoroutine(co, IsJsMode());

#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl end");
#endif
    return co;
}

void StackfulCoroutineManager::DumpCoroutineStats() const
{
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager: dumping performance statistics...";
    os::memory::LockHolder lock(workersLock_);
    PandaVector<CoroutineWorkerStats *> wstats;
    for (auto *worker : workers_) {
        worker->GetPerfStats().Disable();
        wstats.push_back(&worker->GetPerfStats());
    }
    std::cout << "=== Coroutine statistics begin ===" << std::endl;
    std::cout << stats_.GetFullStatistics(std::move(wstats));
    std::cout << "=== Coroutine statistics end ===" << std::endl;
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager: performance statistics dumped successfully.";
}

void StackfulCoroutineManager::WaitForNonMainCoroutinesCompletion()
{
    os::memory::LockHolder lkCompletion(programCompletionLock_);
    auto *main = Coroutine::GetCurrent();
    while (coroutineCount_ > 1 + GetActiveWorkersCount()) {  // 1 is for MAIN
        programCompletionEvent_->SetNotHappened();
        programCompletionEvent_->Lock();
        programCompletionLock_.Unlock();
        ScopedManagedCodeThread s(main);  // perf?
        GetCurrentWorker()->WaitForEvent(programCompletionEvent_);
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::WaitForNonMainCoroutinesCompletion(): possibly spurious wakeup from wait...";
        // NOTE(konstanting, #I67QXC): test for the spurious wakeup
        programCompletionLock_.Lock();
    }
    ASSERT(coroutineCount_ == (1 + GetActiveWorkersCount()));
}

void StackfulCoroutineManager::MainCoroutineCompleted()
{
    // precondition: MAIN is already in the native mode
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): STARTED";
    // block till only schedule loop coroutines are present
    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): waiting for other coroutines to complete";
    WaitForNonMainCoroutinesCompletion();
    // NOTE(konstanting, #I67QXC): correct state transitions for MAIN
    GetCurrentContext()->MainThreadFinished();
    GetCurrentContext()->EnterAwaitLoop();

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping workers";
    {
        os::memory::LockHolder lock(workersLock_);
        for (auto *worker : workers_) {
            worker->SetActive(false);
        }
        while (activeWorkersCount_ > 1) {  // 1 is for MAIN
            // profiling: the SCH interval is expected to be started after the ctxswitch
            GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::SCH_ALL);
            // NOTE(konstanting, #I67QXC): need timed wait?..
            workersCv_.Wait(&workersLock_);
            // profiling: we don't want to profile the sleeping state
            GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
        }
    }

    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping await loop on the main worker";
    while (coroutineCount_ > 1) {
        GetCurrentWorker()->FinalizeFiberScheduleLoop();
    }
    // profiling: the SCH interval is expected to be started after the ctxswitch
    GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::SCH_ALL);

    if (stats_.IsEnabled()) {
        DumpCoroutineStats();
    }
    stats_.Disable();

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): deleting workers";
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        for (auto *worker : workers_) {
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(worker);
        }
        workers_.clear();
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): DONE";
}

StackfulCoroutineContext *StackfulCoroutineManager::GetCurrentContext()
{
    auto *co = Coroutine::GetCurrent();
    return co->GetContext<StackfulCoroutineContext>();
}

StackfulCoroutineWorker *StackfulCoroutineManager::GetCurrentWorker()
{
    return GetCurrentContext()->GetWorker();
}

bool StackfulCoroutineManager::IsMainWorker(Coroutine *co) const
{
    auto *worker = co->GetContext<StackfulCoroutineContext>()->GetWorker();
    return worker->GetId() == stackful_coroutines::MAIN_WORKER_ID;
}

bool StackfulCoroutineManager::IsJsMode()
{
    return jsMode_;
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
                                                           void *param, PandaString name)
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
    auto *co = GetCoroutineFactory()(runtime, vm, std::move(name), ctx, Coroutine::NativeEntrypointInfo(entry, param));
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

}  // namespace ark
