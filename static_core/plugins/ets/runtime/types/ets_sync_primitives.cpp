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
#include "runtime/execution/job_events.h"
#include "runtime/execution/job_execution_context.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "runtime/include/thread_scopes.h"

#include <atomic>

namespace ark::ets {

/*static*/
EtsMutex *EtsMutex::Create(EtsExecutionContext *executionCtx)
{
    auto *klass = PlatformTypes(executionCtx)->coreMutex;
    return EtsMutex::FromEtsObject(EtsObject::Create(executionCtx, klass));
}

ALWAYS_INLINE static void BackOff(uint32_t spinCount)
{
#if defined(PANDA_TARGET_AMD64)
    for (uint32_t spin = 0; spin < spinCount; spin++) {
        __builtin_ia32_pause();
    }
#else
    volatile int64_t x = 0;
    static constexpr int64_t FACTOR = 10;
    for (uint32_t spin = 0; spin < FACTOR * spinCount; spin++) {
        x = x + 1;
    }
#endif
}

bool EtsMutex::TrySpinLockFor(std::atomic<uintptr_t> &state, uintptr_t &expected)
{
    constexpr uint32_t SHORT_REPEAT = 16;
    constexpr uint32_t SHORT_DELAY = 2;

    constexpr uint32_t MEDIUM_REPEAT = 12;
    constexpr uint32_t MEDIUM_DELAY = 8;

    constexpr uint32_t LONG_REPEAT = 6;
    constexpr uint32_t LONG_DELAY = 24;

    constexpr auto SPINLOCK_FUNC = [](std::atomic<uintptr_t> &state, uintptr_t &expected, uint32_t repeat,
                                      uint32_t delay) {
        for (uint32_t i = 0; i < repeat; ++i) {
            if (!IsLocked(expected) &&
                // Atomic with acquire order reason: to make the changes in the critical section visible
                state.compare_exchange_weak(expected, expected | LOCKED_STATE, std::memory_order_acquire,
                                            std::memory_order_relaxed)) {
                return true;
            }
            BackOff(delay);
            // Atomic with relaxed order reason: sync is not needed here because of CAS
            expected = state.load(std::memory_order_relaxed);
        }
        return false;
    };

    return SPINLOCK_FUNC(state, expected, SHORT_REPEAT, SHORT_DELAY) ||
           SPINLOCK_FUNC(state, expected, MEDIUM_REPEAT, MEDIUM_DELAY) ||
           SPINLOCK_FUNC(state, expected, LONG_REPEAT, LONG_DELAY);
}

void EtsMutex::Lock()
{
    uintptr_t head = UNLOCKED_STATE;
    // Atomic with acquire order reason: to make the changes in the critical section visible
    if LIKELY (state_.compare_exchange_strong(head, LOCKED_STATE, std::memory_order_acquire,
                                              std::memory_order_relaxed)) {
        return;
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    auto handleScope = EtsHandleScope(executionCtx);
    auto mutexH = EtsHandle<EtsMutex>(executionCtx, this);
    auto *state = &mutexH.GetPtr()->state_;

    while (!TrySpinLockFor(*state, head)) {
        if (IsLocked(head)) {
            auto awaitee = Node(jobMan);
            awaitee.next = reinterpret_cast<Node *>(head & ~LOCKED_STATE);
            auto newHead = reinterpret_cast<uintptr_t>(&awaitee) | LOCKED_STATE;
            awaitee.GetEvent().Lock();
            TSAN_ANNOTATE_HAPPENS_BEFORE(state);
            // Atomic with release order reason: to make the store in the .next visible in unlock
            if (state->compare_exchange_weak(head, newHead, std::memory_order_release, std::memory_order_relaxed)) {
                awaitee.GetEvent().Wait();
                state = &mutexH.GetPtr()->state_;
                // Atomic with relaxed order reason: sync is not needed here because of CAS
                head = state->load(std::memory_order_relaxed);
                continue;
            }
            awaitee.GetEvent().Unlock();
        }
    }
}

void EtsMutex::Unlock()
{
    uintptr_t head = LOCKED_STATE;
    // Atomic with release order reason: to make the changes in the critical section visible in other threads
    // Atomic with acquire order reason: to make the store in .next visible
    if LIKELY (state_.compare_exchange_strong(head, UNLOCKED_STATE, std::memory_order_release,
                                              std::memory_order_acquire)) {
        return;
    }

    constexpr uint32_t MAX_UNBLOCK_CNT = 8;

    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        TSAN_ANNOTATE_HAPPENS_AFTER(&state_);
        auto *node = reinterpret_cast<Node *>(head & ~LOCKED_STATE);
        auto *unblockList = node;
        uint32_t unblockCnt = 0;
        for (; unblockCnt != MAX_UNBLOCK_CNT && node != nullptr; ++unblockCnt) {
            node = node->next;
        }
        auto newHead = reinterpret_cast<uintptr_t>(node);
        // Atomic with release order reason: to make the changes in the critical section visible in other threads
        // Atomic with acquire order reason: to make the store in .next visible
        if (state_.compare_exchange_weak(head, newHead, std::memory_order_release, std::memory_order_acquire)) {
            for (; unblockCnt != 0; --unblockCnt) {
                std::exchange(unblockList, unblockList->next)->GetEvent().Happen();
            }
            return;
        }
    }
}

bool EtsMutex::IsHeld()
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Lock/Unlock
    return IsLocked(state_.load(std::memory_order_relaxed));
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
