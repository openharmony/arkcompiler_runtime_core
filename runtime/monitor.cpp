/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "runtime/monitor.h"

#include "libpandabase/os/thread.h"
#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/thread-inl.h"
#include "runtime/mark_word.h"
#include "runtime/monitor_pool.h"
#include "runtime/handle_base-inl.h"
#include "runtime/mem/vm_handle.h"

#include <cinttypes>
#include <string>
#include <sched.h>

namespace panda {

template <typename T>
template <typename Predicate>
bool ThreadList<T>::RemoveIf(Predicate pred)
{
    bool found = false;
    auto prev = head_;
    for (auto current = head_; current != nullptr; current = current->GetNextWait()) {
        if (pred(*current)) {
            found = true;
            EraseAfter(prev, current);
            current = prev;
        } else {
            prev = current;
        }
    }
    return found;
}

template <typename T>
void ThreadList<T>::Splice(ThreadList &other)
{
    if (Empty()) {
        head_ = other.head_;
    } else {
        T *last = head_;
        for (; last->GetNextWait() != nullptr; last = last->GetNextWait()) {
        }
        last->SetWaitNext(other.head_);
    }
    other.Clear();
}

template <typename T>
void ThreadList<T>::EraseAfter(T *prev, T *current)
{
    if (current == head_) {
        head_ = current->GetNextWait();
    } else {
        prev->SetWaitNext(current->GetNextWait());
    }
}

template <typename T>
void ThreadList<T>::PopFront()
{
    head_ = head_->GetNextWait();
}

template <typename T>
void ThreadList<T>::PushFront(T &thread)
{
    thread.SetWaitNext(head_);
    head_ = &thread;
}

void Monitor::InflateThinLock(MTManagedThread *thread, [[maybe_unused]] const VMHandle<ObjectHeader> &obj_handle)
{
#if defined(PANDA_USE_FUTEX)
    // Futex inflation policy: suspend target thread, wait until it actually gets suspended
    // and try inflating light monitor (`Inflate` expects lock to still be acquired by target;
    // otherwise markword CAS fails). If it fails (i.e. thread got suspended when this monitor is
    // no longer taken), we restart lightlock acquisition policy again.
    // Compared to forced inflation (actively retry inflation once MAX_TRYLOCK_RETRY is reached
    // or inflate monitor once this thread acquires light lock), this policy yields much better
    // performance for short running synchronized blocks or functions, and is still expected to
    // succeeed on longer blocks which should have safepoints and suspend successfully with
    // monitor still acquired.
    // We are trying to inflate light lock acquired by other thread, suspend it first
    MTManagedThread *owner = nullptr;
    ASSERT(obj_handle.GetPtr() != nullptr);
    MarkWord mark = obj_handle.GetPtr()->AtomicGetMark();
    os::thread::ThreadId owner_thread_id = mark.GetThreadId();
    {
        ScopedChangeThreadStatus sts(thread, ThreadStatus::IS_WAITING_INFLATION);
        owner = thread->GetVM()->GetThreadManager()->SuspendAndWaitThreadByInternalThreadId(owner_thread_id);
    }
    thread->SetEnterMonitorObject(nullptr);
    thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
    // Thread could have finished by the time we tried stopping it
    if (owner != nullptr) {
        // NB! Inflate can do nothing if monitor is already unlocked or acquired by other thread.
        Inflate<true>(obj_handle.GetPtr(), owner);
        owner->ResumeImpl(true);
    }
#else
    // Non-futex inflation policy: Wait until light lock is released, acquire it and inflate
    // to heavy monitor
    {
        static constexpr uint64_t SLEEP_MS = 10;
        thread->TimedWait(ThreadStatus::IS_WAITING_INFLATION, SLEEP_MS, 0);
    }
    thread->SetEnterMonitorObject(nullptr);
    thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
#endif
}

/**
 * Static call, which implements the basic functionality of monitors:
 * heavyweight, lightweight and so on.
 *
 * @param  obj  an object header of corresponding object
 * @param  trylock is true if the function should fail in case of lock was already acquired by other thread
 * @return      state of function execution (ok, illegal)
 */
Monitor::State Monitor::MonitorEnter(ObjectHeader *obj, bool trylock)
{
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    // This function can unlock MutatorLock, so GC can run during lock acquire waiting
    // so we need to use handle to get updated header pointer
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> obj_handle(thread, obj);
    bool ret = false;
    bool should_inflate = false;
    uint32_t lightlock_retry_count = 0;

    while (true) {
        MarkWord mark = obj_handle.GetPtr()->AtomicGetMark();
        MarkWord new_mark = mark;
        MarkWord::ObjectState state = mark.GetState();

        LOG(DEBUG, RUNTIME) << "Try to enter monitor " << std::hex << obj << "  with state " << std::dec << state;

        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED: {
                auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
                if (monitor == nullptr) {
                    // Not sure if it is possible
                    return State::ILLEGAL;
                }
                ret = monitor->Acquire(thread, obj_handle, trylock);
                if (ret) {
                    thread->PushLocalObjectLocked(obj_handle.GetPtr());
                }
                return ret ? State::OK : State::ILLEGAL;
            }
            case MarkWord::STATE_LIGHT_LOCKED: {
                os::thread::ThreadId owner_thread_id = mark.GetThreadId();
                if (owner_thread_id == thread->GetInternalId()) {
                    uint32_t new_count = mark.GetLockCount() + 1;
                    if (new_count < MarkWord::LIGHT_LOCK_LOCK_MAX_COUNT) {
                        new_mark = mark.DecodeFromLightLock(thread->GetInternalId(), new_count);
                        // Strong CAS as the loop iteration is large
                        ret = obj_handle.GetPtr()->AtomicSetMark(mark, new_mark);
                        if (ret) {
                            LOG(DEBUG, RUNTIME) << "The lightweight monitor was successfully recursively acquired";
                            TraceMonitorLock(obj_handle.GetPtr(), false);
                            thread->PushLocalObjectLocked(obj_handle.GetPtr());
                            return State::OK;
                        }
                    } else {
                        Inflate(obj_handle.GetPtr(), thread);
                        // Inflate set up recursive counter to just current amount, loop again.
                    }
                } else {
                    // Lock acquired by other thread.
                    if (trylock) {
                        return State::ILLEGAL;
                    }

                    // Retry acquiring light lock in loop first to avoid excessive inflation
                    static constexpr uint32_t MAX_TRYLOCK_RETRY = 100;
                    static constexpr uint32_t YIELD_AFTER = 50;

                    lightlock_retry_count++;
                    if (lightlock_retry_count < MAX_TRYLOCK_RETRY) {
                        if (lightlock_retry_count > YIELD_AFTER) {
                            MTManagedThread::Yield();
                        }
                    } else {
                        // Retried acquiring light lock for too long, do inflation

                        thread->SetEnterMonitorObject(obj_handle.GetPtr());
                        thread->SetWaitingMonitorOldStatus(ThreadStatus::IS_WAITING_INFLATION);
                        InflateThinLock(thread, obj_handle);
#if defined(PANDA_USE_FUTEX)
                        lightlock_retry_count = 0;
#else
                        should_inflate = true;
#endif
                    }
                }
                // Couldn't update mark.
                if (trylock) {
                    return State::ILLEGAL;
                }
                // Go to the next iteration
                continue;
            }
            case MarkWord::STATE_HASHED:
                if (Inflate(obj_handle.GetPtr(), thread)) {
                    thread->PushLocalObjectLocked(obj_handle.GetPtr());
                    return State::OK;
                }
                // Couldn't inflate.
                if (trylock) {
                    return State::ILLEGAL;
                }
                // Go to the next iteration
                continue;
            case MarkWord::STATE_UNLOCKED:
                if (should_inflate) {
                    if (Inflate(obj_handle.GetPtr(), thread)) {
                        thread->PushLocalObjectLocked(obj_handle.GetPtr());
                        return State::OK;
                    }
                    // Couldn't inflate.
                    if (trylock) {
                        return State::ILLEGAL;
                    }
                    // Go to the next iteration
                    continue;
                }

                ASSERT(thread->GetInternalId() <= MarkWord::LIGHT_LOCK_THREADID_MAX_COUNT);
                new_mark = mark.DecodeFromLightLock(thread->GetInternalId(), 1);
                // Strong CAS as the loop iteration is large
                ret = obj_handle.GetPtr()->AtomicSetMark(mark, new_mark);
                if (ret) {
                    LOG(DEBUG, RUNTIME) << "The lightweight monitor was successfully acquired for the first time";
                    TraceMonitorLock(obj_handle.GetPtr(), false);
                    thread->PushLocalObjectLocked(obj_handle.GetPtr());
                    return State::OK;
                }
                // Couldn't update mark.
                if (trylock) {
                    return State::ILLEGAL;
                }
                // Go to the next iteration
                continue;
            case MarkWord::STATE_GC:
                LOG(FATAL, RUNTIME) << "Not yet implemented";
                return State::ILLEGAL;
            default:
                LOG(FATAL, RUNTIME) << "Undefined object state";
                return State::ILLEGAL;
        }
    }
}

Monitor::State Monitor::MonitorExit(ObjectHeader *obj)
{
    auto thread = MTManagedThread::GetCurrent();
    bool ret = false;

    while (true) {
        MarkWord mark = obj->AtomicGetMark();
        MarkWord new_mark = mark;
        MarkWord::ObjectState state = mark.GetState();
        LOG(DEBUG, RUNTIME) << "Try to exit monitor " << std::hex << obj << "  with state " << std::dec << state;
        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED: {
                auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
                ret = monitor->Release(thread);
                if (ret) {
                    thread->PopLocalObjectLocked(obj);
                }
                return ret ? State::OK : State::ILLEGAL;
            }
            case MarkWord::STATE_LIGHT_LOCKED: {
                if (mark.GetThreadId() != thread->GetInternalId()) {
                    LOG(DEBUG, RUNTIME) << "Caling MonitorEnter on object which isn't owned by this thread";
                    return State::ILLEGAL;
                }
                uint32_t new_count = mark.GetLockCount() - 1;
                if (new_count != 0) {
                    new_mark = mark.DecodeFromLightLock(thread->GetInternalId(), new_count);
                } else {
                    new_mark = mark.DecodeFromUnlocked();
                }
                // Strong CAS as the loop iteration is large
                ret = obj->AtomicSetMark(mark, new_mark);
                if (ret) {
                    LOG(DEBUG, RUNTIME) << "Exited lightweight lock";
                    TraceMonitorUnLock();
                    thread->PopLocalObjectLocked(obj);
                    return State::OK;
                }
                // CAS failed, must have been heavy locked by other thread. Retry unlock.
                continue;
            }
            case MarkWord::STATE_HASHED:
            case MarkWord::STATE_UNLOCKED:
                LOG(ERROR, RUNTIME) << "Try to perform monitor exit from unlocked state";
                return State::ILLEGAL;
            case MarkWord::STATE_GC:
                LOG(FATAL, RUNTIME) << "Not yet implemented";
                return State::ILLEGAL;
            default:
                LOG(FATAL, RUNTIME) << "Undefined object state";
                return State::ILLEGAL;
        }
    }
}

/** Zero timeout is used as infinite wait (see docs)
 */
Monitor::State Monitor::Wait(ObjectHeader *obj, ThreadStatus status, uint64_t timeout, uint64_t nanos,
                             bool ignore_interruption)
{
    ASSERT(obj != nullptr);
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    State result_state = State::OK;

    // This function can unlock MutatorLock, so GC can run during wait
    // so we need to use handle to get updated header pointer
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> obj_handle(thread, obj);

    Runtime::GetCurrent()->GetNotificationManager()->MonitorWaitEvent(obj, timeout);

    while (true) {
        MarkWord mark = obj_handle->AtomicGetMark();
        MarkWord::ObjectState state = mark.GetState();
        LOG(DEBUG, RUNTIME) << "Try to wait with state " << state;
        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED: {
                auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());

                if (monitor->GetOwner() != thread) {
                    // The monitor is acquired by other thread
                    // throw an internal exception?
                    LOG(ERROR, RUNTIME) << "Illegal monitor state: try to wait with monitor acquired by other thread";
                    return State::ILLEGAL;
                }

                thread->GetWaitingMutex()->Lock();

                if (thread->IsInterruptedWithLockHeld() && !ignore_interruption) {
                    thread->GetWaitingMutex()->Unlock();
                    return State::INTERRUPTED;
                }

                // Use LockHolder inside scope
                uint64_t counter = monitor->recursive_counter_;
                // Wait should be called under the monitor. We checked it in the previous if.
                // Thus, the operation with queues are thread-safe
                monitor->waiters_.PushFront(*thread);
                thread->SetWaitingMonitor(monitor);
                thread->SetWaitingMonitorOldStatus(status);

                monitor->recursive_counter_ = 1;
                // Atomic with relaxed order reason: memory access in monitor
                monitor->waiters_counter_.fetch_add(1, std::memory_order_relaxed);
                monitor->Release(thread);

                TraceMonitorLock(obj_handle.GetPtr(), true);
                bool is_timeout = false;
                if (timeout == 0 && nanos == 0) {
                    // Normal wait
                    thread->WaitWithLockHeld(status);
                } else {
                    is_timeout = thread->TimedWaitWithLockHeld(status, timeout, nanos, false);
                }
                TraceMonitorUnLock();  // End Wait().

                // Unlock before to avoid deadlock
                // Nothing happen, if the thread is rescheduled between,
                // As the monitor was already released for external users
                thread->GetWaitingMutex()->Unlock();
                [[maybe_unused]] bool ret = monitor->Acquire(thread, obj_handle, false);
                ASSERT(ret);
                // Atomic with relaxed order reason: memory access in monitor
                monitor->waiters_counter_.fetch_sub(1, std::memory_order_relaxed);
                monitor->recursive_counter_ = counter;

                if (thread->IsInterrupted()) {
                    // TODO(dtrubenkov): call panda::ThrowException when it will be imlemented
                    result_state = State::INTERRUPTED;
                }

                // problems with equality of MTManagedThread's
                bool found = monitor->waiters_.RemoveIf(
                    [thread](MTManagedThread &t) { return thread->GetInternalId() == t.GetInternalId(); });
                // If no matching thread found in waiters_, it should have been moved to to_wakeup_
                // but this thread timed out or got interrupted
                if (!found) {
                    monitor->to_wakeup_.RemoveIf(
                        [thread](MTManagedThread &t) { return thread->GetInternalId() == t.GetInternalId(); });
                }

                thread->SetWaitingMonitor(nullptr);
                thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
                Runtime::GetCurrent()->GetNotificationManager()->MonitorWaitedEvent(obj_handle.GetPtr(), is_timeout);

                return result_state;
            }
            case MarkWord::STATE_LIGHT_LOCKED:
                if (mark.GetThreadId() != thread->GetInternalId()) {
                    LOG(FATAL, RUNTIME) << "Illegal monitor state: try to wait with monitor acquired by other thread";
                    return result_state;
                }
                Inflate(obj_handle.GetPtr(), thread);
                // Go to the next iteration.
                continue;
            case MarkWord::STATE_UNLOCKED:
            case MarkWord::STATE_HASHED:
            case MarkWord::STATE_GC:
                LOG(ERROR, RUNTIME) << "Try to perform Wait from unsupported state";
                return State::ILLEGAL;
            default:
                LOG(FATAL, RUNTIME) << "Undefined object state";
                UNREACHABLE();
        }
    }
}

Monitor::State Monitor::Notify(ObjectHeader *obj)
{
    ASSERT(obj != nullptr);
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();
    auto thread = MTManagedThread::GetCurrent();
    LOG(DEBUG, RUNTIME) << "Try to notify with state " << state;

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());

            if (monitor->GetOwner() != thread) {
                // The monitor is acquired by other thread
                // throw an internal exception?
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }

            // Notify should be called under the monitor. We checked it in the previous if.
            // Thus, the operation with queues are thread-safe

            // Move one thread from waiters to wake_up
            if (!monitor->waiters_.Empty()) {
                // With current panda::List implementation this reference is valid.
                // This can be broken with future changes.
                auto &waiter = monitor->waiters_.Front();
                monitor->waiters_.PopFront();
                monitor->to_wakeup_.PushFront(waiter);
            }
            return State::OK;  // Success
        }
        case MarkWord::STATE_LIGHT_LOCKED:
            if (mark.GetThreadId() != thread->GetInternalId()) {
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }
            return State::OK;  // Success
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            LOG(ERROR, RUNTIME) << "Try to perform Notify from unsupported state";
            return State::ILLEGAL;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            UNREACHABLE();
    }
}

Monitor::State Monitor::NotifyAll(ObjectHeader *obj)
{
    ASSERT(obj != nullptr);
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();
    auto thread = MTManagedThread::GetCurrent();
    LOG(DEBUG, RUNTIME) << "Try to notify all with state " << state;

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());

            if (monitor->GetOwner() != thread) {
                // The monitor is acquired by other thread
                // throw an internal exception?
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }

            // NotifyAll should be called under the monitor. We checked it in the previous if.
            // Thus, the operation with queues are thread-safe
            if (monitor->to_wakeup_.Empty()) {
                monitor->to_wakeup_.Swap(monitor->waiters_);
                return State::OK;
            }

            // Concatenate two queues
            if (!monitor->waiters_.Empty()) {
                monitor->to_wakeup_.Splice(monitor->waiters_);
                monitor->waiters_.Clear();
            }
            return State::OK;  // Success
        }
        case MarkWord::STATE_LIGHT_LOCKED:
            if (mark.GetThreadId() != thread->GetInternalId()) {
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }
            return State::OK;  // Success
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            LOG(ERROR, RUNTIME) << "Try to perform NotifyAll from unsupported state";
            return State::ILLEGAL;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            UNREACHABLE();
    }
}

bool Monitor::Acquire(MTManagedThread *thread, const VMHandle<ObjectHeader> &obj_handle, bool trylock)
{
    ASSERT_MANAGED_CODE();

    MTManagedThread *owner = this->GetOwner();
    if (owner == thread) {
        // Do we need to hold a lock here?
        this->recursive_counter_++;
        LOG(DEBUG, RUNTIME) << "The fat monitor was successfully recursively acquired";
        TraceMonitorLock(obj_handle.GetPtr(), false);
        return true;
    }

    // Use trylock first
    if (trylock) {
        if (!lock_.TryLock()) {
            return false;
        }
    } else {
#ifdef PANDA_USE_FUTEX
        if (!lock_.TryLockWithSpinning()) {
#else
        if (!lock_.TryLock()) {
#endif  // PANDA_USE_FUTEX
            Runtime::GetCurrent()->GetNotificationManager()->MonitorContendedEnterEvent(obj_handle.GetPtr());
            // If not trylock...
            // Do atomic add out of scope to prevent GC getting old waiters_counter_
            // Atomic with relaxed order reason: memory access in monitor
            waiters_counter_.fetch_add(1, std::memory_order_relaxed);
            thread->SetEnterMonitorObject(obj_handle.GetPtr());
            thread->SetWaitingMonitorOldStatus(ThreadStatus::IS_BLOCKED);
            {
                ScopedChangeThreadStatus sts(thread, ThreadStatus::IS_BLOCKED);
                // Save current monitor, on which the given thread is blocked.
                // It can be used to detect potential deadlock with daemon threds.
                thread->SetEnteringMonitor(this);
                lock_.Lock();
                // Deadlock is no longer possible with the given thread.
                thread->SetEnteringMonitor(nullptr);
                // Do this inside scope for thread to release this monitor during runtime destroy
                if (!this->SetOwner(nullptr, thread)) {
                    LOG(FATAL, RUNTIME) << "Set monitor owner failed in Acquire";
                }
                thread->AddMonitor(this);
                this->recursive_counter_++;
            }
            thread->SetEnterMonitorObject(nullptr);
            thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
            // Atomic with relaxed order reason: memory access in monitor
            waiters_counter_.fetch_sub(1, std::memory_order_relaxed);
            // Even thout these 2 warnings are valid, We suppress them. Reason is to have consistent logging
            // Otherwise we would see that lock was done on one monitor address,
            // and unlock (after GC) - ona different one
            // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
            Runtime::GetCurrent()->GetNotificationManager()->MonitorContendedEnteredEvent(obj_handle.GetPtr());
            LOG(DEBUG, RUNTIME) << "The fat monitor was successfully acquired for the first time";
            // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
            TraceMonitorLock(obj_handle.GetPtr(), false);
            return true;
        }
    }

    if (!this->SetOwner(nullptr, thread)) {
        LOG(FATAL, RUNTIME) << "Set monitor owner failed in Acquire";
    }
    thread->AddMonitor(this);
    this->recursive_counter_++;
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully acquired for the first time";
    TraceMonitorLock(obj_handle.GetPtr(), false);
    return true;
}

void Monitor::InitWithOwner(MTManagedThread *thread, ObjectHeader *obj)
{
    ASSERT(this->GetOwner() == nullptr);

#ifdef PANDA_USE_FUTEX
    ASSERT(thread == MTManagedThread::GetCurrent() || thread->GetStatus() != ThreadStatus::RUNNING);
    lock_.LockForOther(thread->GetId());
#else
    ASSERT(thread == MTManagedThread::GetCurrent());
    [[maybe_unused]] bool res = lock_.TryLock();
    ASSERT(res);
#endif  // PANDA_USE_FUTEX

    if (!this->SetOwner(nullptr, thread)) {
        LOG(FATAL, RUNTIME) << "Set monitor owner failed in InitWithOwner";
    }
    this->recursive_counter_++;
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully initialized for the first time";
    TraceMonitorLock(obj, false);
}

void Monitor::ReleaseOnFailedInflate(MTManagedThread *thread)
{
    if (thread != this->GetOwner()) {
        LOG(FATAL, RUNTIME) << "Releasing lock which isn't owned by this thread";
    }
    TraceMonitorUnLock();
    this->recursive_counter_--;
    ASSERT(this->recursive_counter_ == 0);
    // This should never fail
    [[maybe_unused]] bool success = this->SetOwner(thread, nullptr);
    ASSERT(success);
#ifdef PANDA_USE_FUTEX
    ASSERT(thread == MTManagedThread::GetCurrent() || thread->GetStatus() != ThreadStatus::RUNNING);
    lock_.UnlockForOther(thread->GetId());
#else
    ASSERT(thread == MTManagedThread::GetCurrent());
    lock_.Unlock();
#endif  // PANDA_USE_FUTEX
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully released after failed inflation";
}

bool Monitor::Release(MTManagedThread *thread)
{
    if (thread != this->GetOwner()) {
        LOG(FATAL, RUNTIME) << "Releasing lock which isn't owned by this thread";
        return false;
    }
    TraceMonitorUnLock();
    this->recursive_counter_--;
    if (this->recursive_counter_ == 0) {
        if (!this->SetOwner(thread, nullptr)) {
            LOG(FATAL, RUNTIME) << "Set monitor owner failed in Release";
        }
        // Signal the only waiter (the other one will be signaled after the next release)
        MTManagedThread *waiter = nullptr;
        Monitor *waiting_mon = nullptr;
        if (!this->to_wakeup_.Empty()) {
            // NB! Current list implementation leaves this pointer valid after PopFront, change this
            // if List implementation is changed.
            waiter = &(this->to_wakeup_.Front());
            waiting_mon = waiter->GetWaitingMonitor();
            this->to_wakeup_.PopFront();
        }
        thread->RemoveMonitor(this);
        // Signal waiter after mutex unlock so that signalled thread doesn't get stuck on lock_
        if (waiter != nullptr && waiting_mon == this) {
            waiter->Signal();
            LOG(DEBUG, RUNTIME) << "Send the notifing signal to " << waiter->GetId();
        }
        lock_.Unlock();
    }
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully released";
    return true;
}

template <bool for_other_thread>
bool Monitor::Inflate(ObjectHeader *obj, MTManagedThread *thread)
{
    ASSERT(obj != nullptr);
    Monitor *monitor = nullptr;
    MarkWord old_mark = obj->AtomicGetMark();
    MarkWord new_mark = old_mark;
    MarkWord::ObjectState state = old_mark.GetState();
    bool ret = false;

    // Dont inflate if someone already inflated the lock.
    if (state == MarkWord::STATE_HEAVY_LOCKED) {
        return false;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
    if constexpr (for_other_thread) {  // NOLINT(bugprone-suspicious-semicolon)
        // Dont inflate if monitor got unlocked or acquired by other thread.
        if (state != MarkWord::STATE_LIGHT_LOCKED || old_mark.GetThreadId() != thread->GetInternalId()) {
            return false;
        }
    }

    auto *monitor_pool = thread->GetMonitorPool();
    monitor = monitor_pool->CreateMonitor(obj);
    if (monitor == nullptr) {
        LOG(FATAL, RUNTIME) << "Couldn't create new monitor. Out of memory?";
        return false;
    }
    monitor->InitWithOwner(thread, obj);

    switch (state) {
        case MarkWord::STATE_LIGHT_LOCKED:
            if (old_mark.GetThreadId() != thread->GetInternalId()) {
                monitor->ReleaseOnFailedInflate(thread);
                monitor_pool->FreeMonitor(monitor->GetId());
                return false;
            }
            monitor->recursive_counter_ = old_mark.GetLockCount();
            break;
        case MarkWord::STATE_HASHED:
            monitor->SetHashCode(old_mark.GetHash());
            /* fallthrough */
            [[fallthrough]];
        case MarkWord::STATE_UNLOCKED:
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (for_other_thread) {  // NOLINT(bugprone-suspicious-semicolon)
                // We did check above, has to be unreachable
                UNREACHABLE();
            } else {  // NOLINT(readability-misleading-indentation)
                break;
            }
        case MarkWord::STATE_HEAVY_LOCKED:
            // Has to be unreachable
            UNREACHABLE();
        case MarkWord::STATE_GC:
            LOG(FATAL, RUNTIME) << "Trying to inflate object in GC state";
            return false;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            return false;
    }
    new_mark = old_mark.DecodeFromMonitor(monitor->GetId());
    ret = obj->AtomicSetMark(old_mark, new_mark);
    if (!ret) {
        // Means, someone changed the mark
        monitor->recursive_counter_ = 1;
        monitor->ReleaseOnFailedInflate(thread);
        monitor_pool->FreeMonitor(monitor->GetId());
    } else {
        // Unlike normal Acquire, AddMonitor should be done not in InitWithOwner but after successful inflation to avoid
        // data race
        thread->AddMonitor(monitor);
    }
    return ret;
}

bool Monitor::Deflate(ObjectHeader *obj)
{
    Monitor *monitor = nullptr;
    MarkWord old_mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = old_mark.GetState();
    bool ret = false;

    if (state != MarkWord::STATE_HEAVY_LOCKED) {
        LOG(DEBUG, RUNTIME) << "Trying to deflate non-heavy locked object";
        return false;
    }

    auto *monitor_pool = MTManagedThread::GetCurrent()->GetMonitorPool();
    monitor = monitor_pool->LookupMonitor(old_mark.GetMonitorId());
    if (monitor == nullptr) {
        LOG(DEBUG, RUNTIME) << "Monitor was already destroyed by someone else.";
        return false;
    }

    ret = monitor->DeflateInternal();
    if (ret) {
        monitor_pool->FreeMonitor(monitor->GetId());
    }
    return ret;
}

bool Monitor::DeflateInternal()
{
    if (GetOwner() != nullptr) {
        LOG(DEBUG, RUNTIME) << "Trying to deflate monitor which already has owner";
        return false;
    }
    // Atomic with relaxed order reason: memory access in monitor
    if (waiters_counter_.load(std::memory_order_relaxed) > 0) {
        LOG(DEBUG, RUNTIME) << "Trying to deflate monitor which is trying to be acquired by other threads";
        return false;
    }
    if (!lock_.TryLock()) {
        LOG(DEBUG, RUNTIME) << "Couldn't TryLock monitor for deflation";
        return false;
    }
    ASSERT(obj_ != nullptr);
    ASSERT(recursive_counter_ == 0);
    ASSERT(waiters_.Empty());
    ASSERT(to_wakeup_.Empty());
    ASSERT(GetOwner() == static_cast<MTManagedThread *>(nullptr));
    MarkWord old_mark = obj_->AtomicGetMark();
    MarkWord new_mark = old_mark;
    if (HasHashCode()) {
        new_mark = old_mark.DecodeFromHash(GetHashCode());
        LOG(DEBUG, RUNTIME) << "Deflating monitor to hash";
    } else {
        new_mark = old_mark.DecodeFromUnlocked();
        LOG(DEBUG, RUNTIME) << "Deflating monitor to unlocked";
    }

    // Warning: AtomicSetMark is weak, retry
    while (!obj_->AtomicSetMark<false>(old_mark, new_mark)) {
        MarkWord cur_mark = obj_->AtomicGetMark();
        if (old_mark.GetValue() != cur_mark.GetValue()) {
            old_mark = cur_mark;
            new_mark = HasHashCode() ? old_mark.DecodeFromHash(GetHashCode()) : old_mark.DecodeFromUnlocked();
        }
    }
    lock_.Unlock();
    return true;
}

uint8_t Monitor::HoldsLock(ObjectHeader *obj)
{
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();
    MTManagedThread *thread = MTManagedThread::GetCurrent();

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            Monitor *monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            // asm has no boolean type
            return (monitor->GetOwner() == thread) ? 1 : 0;
        }
        case MarkWord::STATE_LIGHT_LOCKED:
            return (mark.GetThreadId() == thread->GetInternalId()) ? 1 : 0;
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            return 0;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            return 0;
    }
}

uint32_t Monitor::GetLockOwnerOsThreadID(ObjectHeader *obj)
{
    if (obj == nullptr) {
        return MTManagedThread::NON_INITIALIZED_THREAD_ID;
    }
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            Monitor *monitor = MTManagedThread::GetCurrent()->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            MTManagedThread *owner = monitor->GetOwner();
            if (owner == nullptr) {
                return MTManagedThread::NON_INITIALIZED_THREAD_ID;
            }
            return owner->GetId();
        }
        case MarkWord::STATE_LIGHT_LOCKED: {
            return mark.GetThreadId();
        }
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            return 0;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            return 0;
    }
}

Monitor *Monitor::GetMonitorFromObject(ObjectHeader *obj)
{
    if (obj != nullptr) {
        MarkWord mark = obj->AtomicGetMark();
        MarkWord::ObjectState state = mark.GetState();
        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED:
                return MTManagedThread::GetCurrent()->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            case MarkWord::STATE_LIGHT_LOCKED:
                return nullptr;
            default:
                // Shouldn't happen, return nullptr
                LOG(WARNING, RUNTIME) << "obj:" << obj << " not locked by heavy or light locked";
        }
    }
    return nullptr;
}

inline void Monitor::TraceMonitorLock(ObjectHeader *obj, bool is_wait)
{
    if (UNLIKELY(panda::trace::IsEnabled())) {
        // Use stack memory to avoid "Too many allocations" error.
        constexpr int BUF_SIZE = 32;
        std::array<char, BUF_SIZE> buf = {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        int ret = snprintf_s(buf.data(), BUF_SIZE, BUF_SIZE - 1,
                             (is_wait ? "Waiting on 0x%" PRIxPTR : "Locking 0x%" PRIxPTR), ToUintPtr(obj));
        if (ret < 0) {
            UNREACHABLE();
        }
        trace::BeginTracePoint(buf.data());
    }
}

inline void Monitor::TraceMonitorUnLock()
{
    if (UNLIKELY(panda::trace::IsEnabled())) {
        trace::EndTracePoint();
    }
}

uint32_t Monitor::GetHashCode()
{
    while (!HasHashCode()) {
        uint32_t expected = 0;
        uint32_t newHash = ObjectHeader::GenerateHashCode();
        if (hash_code_.compare_exchange_weak(expected, newHash)) {
            return newHash;
        }
    }
    ASSERT(HasHashCode());
    // Atomic with relaxed order reason: memory access in monitor
    return hash_code_.load(std::memory_order_relaxed);
}

bool Monitor::HasHashCode() const
{
    // Atomic with relaxed order reason: memory access in monitor
    return hash_code_.load(std::memory_order_relaxed) != 0;
}

void Monitor::SetHashCode(uint32_t hash)
{
    ASSERT(GetOwner() == MTManagedThread::GetCurrent());
    if (!HasHashCode()) {
        // Atomic with relaxed order reason: memory access in monitor
        hash_code_.store(hash, std::memory_order_relaxed);
    } else {
        LOG(FATAL, RUNTIME) << "Attempt to rewrite hash in monitor";
    }
}

}  // namespace panda
