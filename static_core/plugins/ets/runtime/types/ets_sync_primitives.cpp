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
#include "runtime/execution/job_execution_context.h"
#include "include/managed_thread.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "runtime/include/thread_scopes.h"

#include <atomic>

namespace ark::ets {

/*static*/
EtsMutex *EtsMutex::Create(EtsExecutionContext *executionCtx)
{
    EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->coreMutex;
    auto hMutex = EtsHandle<EtsMutex>(executionCtx, EtsMutex::FromEtsObject(EtsObject::Create(executionCtx, klass)));
    auto *waitersList = EtsWaitersList::Create(executionCtx);
    ASSERT(hMutex.GetPtr() != nullptr);
    hMutex->SetWaitersList(executionCtx, waitersList);
    return hMutex.GetPtr();
}

ALWAYS_INLINE inline static void BackOff(uint32_t i)
{
    volatile uint32_t x;  // Volatile to make sure loop is not optimized out.
    const uint32_t spinCount = 10 * i;
    for (uint32_t spin = 0; spin < spinCount; spin++) {
        x = x + 1;
    }
}

static bool TrySpinLockFor(std::atomic<uint32_t> &waiters, uint32_t expected, uint32_t desired)
{
    static constexpr uint32_t maxBackOff = 3;  // NOLINT(readability-identifier-naming)
    static constexpr uint32_t maxIter = 3;     // NOLINT(readability-identifier-naming)
    uint32_t exp;
    for (uint32_t i = 1; i <= maxIter; ++i) {
        exp = expected;
        if (waiters.compare_exchange_weak(exp, desired, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            return true;
        }
        BackOff(std::min<uint32_t>(i, maxBackOff));
    }
    return false;
}

void EtsMutex::Lock()
{
    if (TrySpinLockFor(waiters_, 0, 1)) {
        return;
    }
    // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    if (waiters_.fetch_add(1, std::memory_order_acq_rel) == 0) {
        return;
    }
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto event = BlockingEvent {executionCtx->GetManager()};
    auto awaitee = EtsWaitersList::Node(&event);
    SuspendCoroutine(&awaitee);
}

void EtsMutex::Unlock()
{
    // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    if (waiters_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        return;
    }
    ResumeCoroutine();
}

bool EtsMutex::IsHeld()
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Lock/Unlock
    return waiters_.load(std::memory_order_relaxed) != 0;
}

/*static*/
EtsEvent *EtsEvent::Create(EtsExecutionContext *executionCtx)
{
    EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->coreEvent;
    auto hEvent = EtsHandle<EtsEvent>(executionCtx, EtsEvent::FromEtsObject(EtsObject::Create(executionCtx, klass)));
    auto *waitersList = EtsWaitersList::Create(executionCtx);
    ASSERT(hEvent.GetPtr() != nullptr);
    hEvent->SetWaitersList(executionCtx, waitersList);
    return hEvent.GetPtr();
}

void EtsEvent::Wait()
{
    // Atomic with acq_rel order reason: sync Wait/Fire in other threads
    auto state = state_.fetch_add(ONE_WAITER, std::memory_order_acq_rel);
    ASSERT(!HasDependencyBit(state));
    if (IsFireState(state)) {
        return;
    }
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto event = BlockingEvent {executionCtx->GetManager()};
    auto awaitee = EtsWaitersList::Node(&event);
    SuspendCoroutine(&awaitee);
}

void EtsEvent::Fire()
{
    // Atomic with acq_rel order reason: sync Wait/Fire in other threads
    auto state = state_.exchange(FIRE_STATE, std::memory_order_acq_rel);
    ASSERT(!HasDependencyBit(state));
    if (IsFireState(state)) {
        return;
    }
    for (auto waiters = GetNumberOfWaiters(state); waiters > 0; --waiters) {
        ResumeCoroutine();
    }
}

/* static */
EtsCondVar *EtsCondVar::Create(EtsExecutionContext *executionCtx)
{
    EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->coreCondVar;
    auto hCondVar = EtsHandle<EtsCondVar>(executionCtx, EtsCondVar::FromEtsObject(EtsObject::Create(klass)));
    auto *waitersList = EtsWaitersList::Create(executionCtx);
    ASSERT(hCondVar.GetPtr() != nullptr);
    hCondVar->SetWaitersList(executionCtx, waitersList);
    return hCondVar.GetPtr();
}

void EtsCondVar::Wait(EtsHandle<EtsMutex> &mutex)
{
    ASSERT(mutex->IsHeld());
    waiters_++;
    mutex->Unlock();
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto event = BlockingEvent {executionCtx->GetManager()};
    auto awaitee = EtsWaitersList::Node(&event);
    SuspendCoroutine(&awaitee);
    mutex->Lock();
}

void EtsCondVar::NotifyOne([[maybe_unused]] EtsMutex *mutex)
{
    ASSERT(mutex->IsHeld());
    if (waiters_ != 0) {
        ResumeCoroutine();
        waiters_--;
    }
}

void EtsCondVar::NotifyAll([[maybe_unused]] EtsMutex *mutex)
{
    ASSERT(mutex->IsHeld());
    while (waiters_ != 0) {
        ResumeCoroutine();
        waiters_--;
    }
}

/* static */
EtsQueueSpinlock *EtsQueueSpinlock::Create(EtsExecutionContext *executionCtx)
{
    auto *klass = PlatformTypes(executionCtx)->coreQueueSpinlock;
    return EtsQueueSpinlock::FromEtsObject(EtsObject::Create(klass));
}

void EtsQueueSpinlock::Acquire(Guard *waiter)
{
    // Atomic with acq_rel order reason: to guarantee happens-before for critical sections
    auto *oldTail = tail_.exchange(waiter, std::memory_order_acq_rel);
    if (oldTail == nullptr) {
        return;
    }
    // Atomic with release order reason: to guarantee happens-before with waiter constructor
    oldTail->next_.store(waiter, std::memory_order_release);
    auto spinWait = SpinWait();
    ScopedNativeCodeThread nativeCode(ManagedThread::GetCurrent());
    // Atomic with acquire order reason: to guarantee happens-before for critical sections
    while (!waiter->isOwner_.load(std::memory_order_acquire)) {
        spinWait();
    }
}

void EtsQueueSpinlock::Release(Guard *owner)
{
    auto *head = owner;
    // Atomic with release order reason: to guarantee happens-before for critical sections
    if (tail_.compare_exchange_strong(head, nullptr, std::memory_order_release, std::memory_order_relaxed)) {
        return;
    }
    // Atomic with acquire order reason: to guarantee happens-before with next constructor
    Guard *next = owner->next_.load(std::memory_order_acquire);
    auto spinWait = SpinWait();
    while (next == nullptr) {
        spinWait();
        // Atomic with acquire order reason: to guarantee happens-before with next constructor
        next = owner->next_.load(std::memory_order_acquire);
    }
    // Atomic with release order reason: to guarantee happens-before for critical sections
    next->isOwner_.store(true, std::memory_order_release);
}

bool EtsQueueSpinlock::IsHeld() const
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Acquire/Release
    return tail_.load(std::memory_order_relaxed) != nullptr;
}

EtsQueueSpinlock::Guard::Guard(EtsHandle<EtsQueueSpinlock> &spinlock) : spinlock_(spinlock)
{
    ASSERT(spinlock_.GetPtr() != nullptr);
    spinlock_->Acquire(this);
}

EtsQueueSpinlock::Guard::~Guard()
{
    ASSERT(spinlock_.GetPtr() != nullptr);
    spinlock_->Release(this);
}

void EtsRWLock::ReadLock()
{
    // Atomic with relaxed order reason: sync is not needed here because CAS has acq_rel order
    uint64_t oldState = state_.load(std::memory_order_relaxed);
    uint64_t newState = 0;
    do {
        newState = State::IncReaders(oldState) | (State::HasWriteLock(oldState) ? 0 : State::READ_LOCK_STATE);
        // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    } while (!state_.compare_exchange_weak(oldState, newState, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (!State::HasReadLock(newState)) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        auto event = BlockingEvent {JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager()};
        auto readAwaitee = EtsWaitersList::Node(&event);
        EtsSyncPrimitive<EtsRWLock>::SuspendCoroutine(GetReaders(executionCtx), &readAwaitee);
    }
}

void EtsRWLock::WriteLock()
{
    // Atomic with relaxed order reason: sync is not needed here because CAS has acq_rel order
    uint64_t oldState = state_.load(std::memory_order_relaxed);
    uint64_t newState = 0;
    do {
        newState = State::IncWriters(oldState) | (State::HasReadLock(oldState) ? 0 : State::WRITE_LOCK_STATE);
        // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    } while (!state_.compare_exchange_weak(oldState, newState, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (!State::HasWriteLock(newState) || State::HasWriteLock(oldState)) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        auto event = BlockingEvent {JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager()};
        auto writeAwaitee = EtsWaitersList::Node(&event);
        EtsSyncPrimitive<EtsRWLock>::SuspendCoroutine(GetWriters(executionCtx), &writeAwaitee);
    }
}

void EtsRWLock::Unlock()
{
    ASSERT(!State::IsUnlocked(GetState()));
    // Atomic with relaxed order reason: sync is not needed here because CAS has acq_rel order
    uint64_t oldState = state_.load(std::memory_order_relaxed);
    uint64_t newState = 0;
    do {
        newState = State::HasReadLock(oldState) ? State::DecReadersClearState(oldState)
                                                : State::DecWritersClearState(oldState);
        newState |= State::HasReaders(newState)   ? State::READ_LOCK_STATE
                    : State::HasWriters(newState) ? State::WRITE_LOCK_STATE
                                                  : State::UNLOCKED_STATE;
        // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    } while (!state_.compare_exchange_weak(oldState, newState, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (newState == State::UNLOCKED_STATE || (State::HasReadLock(oldState) && State::HasReadLock(newState))) {
        return;
    }
    if (State::HasWriteLock(newState)) {
        EtsSyncPrimitive<EtsRWLock>::ResumeCoroutine(GetWriters(EtsExecutionContext::GetCurrent()));
        return;
    }
    for (auto readers = State::GetReaders(newState); readers != 0; --readers) {
        EtsSyncPrimitive<EtsRWLock>::ResumeCoroutine(GetReaders(EtsExecutionContext::GetCurrent()));
    }
}

uint64_t EtsRWLock::GetState() const
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Lock/Unlock
    return state_.load(std::memory_order_relaxed);
}

bool EtsRWLock::IsHeld() const
{
    return !State::IsUnlocked(GetState());
}

EtsEventWithDependencies *EtsEventWithDependencies::Create(EtsExecutionContext *executionCtx)
{
    return static_cast<EtsEventWithDependencies *>(EtsEvent::Create(executionCtx));
}

void EtsEventWithDependencies::AddDependency(JobEvent *dependency)
{
    // Atomic with relaxed order reason: acq_rel in CAS
    auto state = state_.load(std::memory_order_relaxed);
    // Atomic with acq_rel order reason: sync Wait/Fire in other threads
    while (!state_.compare_exchange_weak(state, (state + ONE_WAITER) | DEPENDENCY_BIT, std::memory_order_acq_rel,
                                         std::memory_order_relaxed)) {
    }
    ASSERT(HasDependencyBit(state_));
    if (IsFireState(state)) {
        dependency->Happen();
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(dependency);
        return;
    }
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *awaitee = Runtime::GetCurrent()->GetInternalAllocator()->New<EtsWaitersList::Node>(dependency);
    GetWaitersList(executionCtx)->PushBack(awaitee);
}

void EtsEventWithDependencies::ResolveDependencies()
{
    auto state = state_.exchange(FIRE_STATE | DEPENDENCY_BIT, std::memory_order_acq_rel);
    ASSERT(HasDependencyBit(state_));
    if (IsFireState(state)) {
        return;
    }

    auto *waitersList = GetWaitersList(EtsExecutionContext::GetCurrent());
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    for (auto waiters = GetNumberOfWaiters(state); waiters > 0; --waiters) {
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        auto *node = waitersList->PopFront();
        auto *blocker = node->GetEvent();
        bool externallyOwned = node->IsOwnedExternally();

        allocator->Delete(node);
        blocker->Happen();

        if (!externallyOwned) {
            allocator->Delete(blocker);
        }
    }
}

void EtsEventWithDependencies::WaitBlocking()
{
    // Atomic with relaxed order reason: acq_rel in CAS
    auto state = state_.load(std::memory_order_relaxed);
    // Atomic with acq_rel order reason: sync with ResolveDependencies
    while (!state_.compare_exchange_weak(state, (state + ONE_WAITER) | DEPENDENCY_BIT, std::memory_order_acq_rel,
                                         std::memory_order_relaxed)) {
    }

    if (IsFireState(state)) {
        return;
    }

    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);

    auto blockingEvent = BlockingEvent {executionCtx->GetManager()};
    auto *node = Runtime::GetCurrent()->GetInternalAllocator()->New<EtsWaitersList::Node>(&blockingEvent);
    node->SetOwnedExternally(true);

    blockingEvent.Lock();
    GetWaitersList(EtsExecutionContext::GetCurrent())->PushBack(node);
    blockingEvent.Wait();
}

}  // namespace ark::ets
