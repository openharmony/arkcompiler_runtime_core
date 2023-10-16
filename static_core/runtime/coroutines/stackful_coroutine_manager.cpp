/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

namespace panda {

uint8_t *StackfulCoroutineManager::AllocCoroutineStack()
{
    Pool stack_pool = PoolManager::GetMmapMemPool()->AllocPool<OSPagesAllocPolicy::NO_POLICY>(
        coro_stack_size_bytes_, SpaceType::SPACE_TYPE_NATIVE_STACKS, AllocatorType::NATIVE_STACKS_ALLOCATOR);
    return static_cast<uint8_t *>(stack_pool.GetMem());
}

void StackfulCoroutineManager::FreeCoroutineStack(uint8_t *stack)
{
    if (stack != nullptr) {
        PoolManager::GetMmapMemPool()->FreePool(stack, coro_stack_size_bytes_);
    }
}

void StackfulCoroutineManager::CreateWorkers(uint32_t how_many, Runtime *runtime, PandaVM *vm)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();

    auto *w_main = allocator->New<StackfulCoroutineWorker>(
        runtime, vm, this, StackfulCoroutineWorker::ScheduleLoopType::FIBER, "[main] worker 0");
    workers_.push_back(w_main);

    for (uint32_t i = 1; i < how_many; ++i) {
        auto *w = allocator->New<StackfulCoroutineWorker>(
            runtime, vm, this, StackfulCoroutineWorker::ScheduleLoopType::THREAD, "worker " + ToPandaString(i));
        workers_.push_back(w);
    }
    active_workers_count_ = how_many;

    auto *main_co = CreateMainCoroutine(runtime, vm);
    main_co->GetContext<StackfulCoroutineContext>()->SetWorker(w_main);
    Coroutine::SetCurrent(main_co);
}

void StackfulCoroutineManager::OnWorkerShutdown()
{
    os::memory::LockHolder lock(workers_lock_);
    --active_workers_count_;
    workers_shutdown_cv_.Signal();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnWorkerShutdown(): COMPLETED, workers left = "
                           << active_workers_count_;
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
    coro_stack_size_bytes_ = Runtime::GetCurrent()->GetOptions().GetCoroutineStackSizePages() * os::mem::GetPageSize();
    if (coro_stack_size_bytes_ != AlignUp(coro_stack_size_bytes_, PANDA_POOL_ALIGNMENT_IN_BYTES)) {
        size_t alignment_pages = PANDA_POOL_ALIGNMENT_IN_BYTES / os::mem::GetPageSize();
        LOG(FATAL, COROUTINES) << "Coroutine stack size should be >= " << alignment_pages
                               << " pages and should be aligned to " << alignment_pages << "-page boundary!";
    }
    size_t coro_stack_area_size_bytes = Runtime::GetCurrent()->GetOptions().GetCoroutinesStackMemLimit();
    coroutine_count_limit_ = coro_stack_area_size_bytes / coro_stack_size_bytes_;
    js_mode_ = config.emulate_js;

    os::memory::LockHolder lock(workers_lock_);
    if (config.workers_count == CoroutineManagerConfig::WORKERS_COUNT_AUTO) {
        CreateWorkers(std::thread::hardware_concurrency(), runtime, vm);
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): setting number of coroutine workers to CPU count = "
                               << workers_.size();
    } else {
        CreateWorkers(static_cast<uint32_t>(config.workers_count), runtime, vm);
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): setting number of coroutine workers to "
                               << workers_.size();
    }
    program_completion_event_ = Runtime::GetCurrent()->GetInternalAllocator()->New<GenericEvent>();
}

void StackfulCoroutineManager::Finalize()
{
    os::memory::LockHolder lock(coro_pool_lock_);

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(program_completion_event_);
    for (auto *co : coroutine_pool_) {
        co->DestroyInternalResources();
        CoroutineManager::DestroyEntrypointfulCoroutine(co);
    }
    coroutine_pool_.clear();
}

void StackfulCoroutineManager::AddToRegistry(Coroutine *co)
{
    os::memory::LockHolder lock(coro_list_lock_);
    auto *main_co = GetMainThread();
    if (main_co != nullptr) {
        // TODO(konstanting, #I67QXC): we should get this callback from GC instead of copying from the main thread
        co->SetPreWrbEntrypoint(main_co->GetPreWrbEntrypoint());
    }
    coroutines_.insert(co);
    coroutine_count_++;
}

void StackfulCoroutineManager::RemoveFromRegistry(Coroutine *co)
{
    coroutines_.erase(co);
    coroutine_count_--;
}

void StackfulCoroutineManager::RegisterCoroutine(Coroutine *co)
{
    AddToRegistry(co);
}

bool StackfulCoroutineManager::TerminateCoroutine(Coroutine *co)
{
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::TerminateCoroutine() started";
    co->NativeCodeEnd();
    co->UpdateStatus(ThreadStatus::TERMINATING);

    {
        os::memory::LockHolder l_list(coro_list_lock_);
        RemoveFromRegistry(co);
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
        UnblockWaiters(co->GetCompletionEvent());
        CheckProgramCompletion();
        GetCurrentWorker()->RequestFinalization(co);
    } else if (co->HasNativeEntrypoint()) {
        GetCurrentWorker()->RequestFinalization(co);
    } else {
        // entrypointless and NOT native: e.g. MAIN
        // (do nothing, as entrypointless coroutines should should be destroyed manually)
    }

    return false;
}

size_t StackfulCoroutineManager::GetActiveWorkersCount()
{
    os::memory::LockHolder lk_workers(workers_lock_);
    return active_workers_count_;
}

void StackfulCoroutineManager::CheckProgramCompletion()
{
    os::memory::LockHolder lk_completion(program_completion_lock_);
    size_t active_worker_coros = GetActiveWorkersCount();

    if (coroutine_count_ == 1 + active_worker_coros) {  // 1 here is for MAIN
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::CheckProgramCompletion(): all coroutines finished execution!";
        // program_completion_event_ acts as a stackful-friendly cond var
        program_completion_event_->SetHappened();
        UnblockWaiters(program_completion_event_);
    } else {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::CheckProgramCompletion(): still "
                               << coroutine_count_ - 1 - active_worker_coros << " coroutines exist...";
    }
}

CoroutineContext *StackfulCoroutineManager::CreateCoroutineContext(bool coro_has_entrypoint)
{
    return CreateCoroutineContextImpl(coro_has_entrypoint);
}

void StackfulCoroutineManager::DeleteCoroutineContext(CoroutineContext *ctx)
{
    FreeCoroutineStack(static_cast<StackfulCoroutineContext *>(ctx)->GetStackLoAddrPtr());
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(ctx);
}

size_t StackfulCoroutineManager::GetCoroutineCount()
{
    return coroutine_count_;
}

size_t StackfulCoroutineManager::GetCoroutineCountLimit()
{
    return coroutine_count_limit_;
}

Coroutine *StackfulCoroutineManager::Launch(CompletionEvent *completion_event, Method *entrypoint,
                                            PandaVector<Value> &&arguments)
{
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Launch started";

    auto *result = LaunchImpl(completion_event, entrypoint, std::move(arguments));
    if (result == nullptr) {
        ThrowOutOfMemoryError("Launch failed");
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Launch finished";
    return result;
}

void StackfulCoroutineManager::Await(CoroutineEvent *awaitee)
{
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
    os::memory::LockHolder lk_workers(workers_lock_);
    ASSERT(blocker != nullptr);
#ifndef NDEBUG
    {
        os::memory::LockHolder lk_blocker(*blocker);
        ASSERT(blocker->Happened());
    }
#endif

    for (auto *w : workers_) {
        w->UnblockWaiters(blocker);
    }
}

void StackfulCoroutineManager::Schedule()
{
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Schedule() request from "
                           << Coroutine::GetCurrent()->GetName();
    GetCurrentWorker()->RequestSchedule();
}

bool StackfulCoroutineManager::EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int inc_mask,
                                                    unsigned int xor_mask) const
{
    os::memory::LockHolder lock(coro_list_lock_);
    for (auto *t : coroutines_) {
        if (!ApplyCallbackToThread(cb, t, inc_mask, xor_mask)) {
            return false;
        }
    }
    return true;
}

void StackfulCoroutineManager::SuspendAllThreads()
{
    os::memory::LockHolder lock(coro_list_lock_);
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::SuspendAllThreads started";
    for (auto *t : coroutines_) {
        t->SuspendImpl(true);
    }
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::SuspendAllThreads finished";
}

void StackfulCoroutineManager::ResumeAllThreads()
{
    os::memory::LockHolder lock(coro_list_lock_);
    for (auto *t : coroutines_) {
        t->ResumeImpl(true);
    }
}

bool StackfulCoroutineManager::IsRunningThreadExist()
{
    UNREACHABLE();
    // TODO(konstanting): correct implementation. Which coroutine do we consider running?
    return false;
}

void StackfulCoroutineManager::WaitForDeregistration()
{
    MainCoroutineCompleted();
}

void StackfulCoroutineManager::ReuseCoroutineInstance(Coroutine *co, CompletionEvent *completion_event,
                                                      Method *entrypoint, PandaVector<Value> &&arguments,
                                                      PandaString name)
{
    auto *ctx = co->GetContext<CoroutineContext>();
    co->ReInitialize(std::move(name), ctx,
                     Coroutine::ManagedEntrypointInfo {completion_event, entrypoint, std::move(arguments)});
}

Coroutine *StackfulCoroutineManager::TryGetCoroutineFromPool()
{
    os::memory::LockHolder lk_pool(coro_pool_lock_);
    if (coroutine_pool_.empty()) {
        return nullptr;
    }
    Coroutine *co = coroutine_pool_.back();
    coroutine_pool_.pop_back();
    return co;
}

Coroutine *StackfulCoroutineManager::LaunchImpl(CompletionEvent *completion_event, Method *entrypoint,
                                                PandaVector<Value> &&arguments)
{
#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl begin");
#endif
    auto coro_name = entrypoint->GetFullName();

    Coroutine *co = nullptr;
    if (Runtime::GetOptions().IsUseCoroutinePool()) {
        co = TryGetCoroutineFromPool();
    }
    if (co != nullptr) {
        ReuseCoroutineInstance(co, completion_event, entrypoint, std::move(arguments), std::move(coro_name));
    } else {
        co = CreateCoroutineInstance(completion_event, entrypoint, std::move(arguments), std::move(coro_name));
    }
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchImpl: failed to create a coroutine!";
        return co;
    }
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(co);

    {
        os::memory::LockHolder lk_workers(workers_lock_);
        auto w = std::min_element(workers_.begin(), workers_.end(),
                                  [](StackfulCoroutineWorker *a, StackfulCoroutineWorker *b) {
                                      return a->GetLoadFactor() < b->GetLoadFactor();
                                  });
        (*w)->AddRunnableCoroutine(co, IsJsMode());
    }
#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl end");
#endif
    return co;
}

void StackfulCoroutineManager::MainCoroutineCompleted()
{
    // precondition: MAIN is already in the native mode
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): STARTED";

    // block till only schedule loop coroutines are present
    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): waiting for other coroutines to complete";

    {
        os::memory::LockHolder lk_completion(program_completion_lock_);
        auto *main = Coroutine::GetCurrent();
        while (coroutine_count_ > 1 + GetActiveWorkersCount()) {  // 1 is for MAIN
            program_completion_event_->SetNotHappened();
            program_completion_event_->Lock();
            program_completion_lock_.Unlock();
            ScopedManagedCodeThread s(main);  // perf?
            GetCurrentWorker()->WaitForEvent(program_completion_event_);
            LOG(DEBUG, COROUTINES)
                << "StackfulCoroutineManager::MainCoroutineCompleted(): possibly spurious wakeup from wait...";
            // TODO(konstanting, #I67QXC): test for the spurious wakeup
            program_completion_lock_.Lock();
        }
    }

    // TODO(konstanting, #I67QXC): correct state transitions for MAIN
    GetCurrentContext()->MainThreadFinished();
    GetCurrentContext()->EnterAwaitLoop();

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping workers";
    {
        os::memory::LockHolder lock(workers_lock_);
        for (auto *worker : workers_) {
            worker->SetActive(false);
        }
        while (active_workers_count_ > 1) {  // 1 is for MAIN
            // TODO(konstanting, #I67QXC): need timed wait?..
            workers_shutdown_cv_.Wait(&workers_lock_);
        }
    }

    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping await loop on the main worker";
    while (coroutine_count_ > 1) {
        GetCurrentWorker()->FinalizeFiberScheduleLoop();
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): deleting workers";
    {
        os::memory::LockHolder lk_workers(workers_lock_);
        for (auto *worker : workers_) {
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(worker);
        }
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

bool StackfulCoroutineManager::IsJsMode()
{
    return js_mode_;
}

void StackfulCoroutineManager::DestroyEntrypointfulCoroutine(Coroutine *co)
{
    if (Runtime::GetOptions().IsUseCoroutinePool() && co->HasManagedEntrypoint()) {
        co->CleanUp();
        os::memory::LockHolder lock(coro_pool_lock_);
        coroutine_pool_.push_back(co);
    } else {
        CoroutineManager::DestroyEntrypointfulCoroutine(co);
    }
}

StackfulCoroutineContext *StackfulCoroutineManager::CreateCoroutineContextImpl(bool need_stack)
{
    uint8_t *stack = nullptr;
    size_t stack_size_bytes = 0;
    if (need_stack) {
        stack = AllocCoroutineStack();
        if (stack == nullptr) {
            return nullptr;
        }
        stack_size_bytes = coro_stack_size_bytes_;
    }
    return Runtime::GetCurrent()->GetInternalAllocator()->New<StackfulCoroutineContext>(stack, stack_size_bytes);
}

Coroutine *StackfulCoroutineManager::CreateNativeCoroutine(Runtime *runtime, PandaVM *vm,
                                                           Coroutine::NativeEntrypointInfo::NativeEntrypointFunc entry,
                                                           void *param)
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
    auto *co = GetCoroutineFactory()(runtime, vm, "_native_coro_", ctx, Coroutine::NativeEntrypointInfo(entry, param));
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

}  // namespace panda
