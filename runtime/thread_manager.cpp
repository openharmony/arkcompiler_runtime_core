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

#include "libpandabase/os/thread.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/utf.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread-inl.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/lock_order_graph.h"
#include "runtime/thread_manager.h"

namespace panda {

ThreadManager::ThreadManager(mem::InternalAllocatorPtr allocator) : threads_(allocator->Adapter())
{
    last_id_ = 0;
    pending_threads_ = 0;
}

ThreadManager::~ThreadManager()
{
    threads_.clear();
}

uint32_t ThreadManager::GetInternalThreadId()
{
    os::memory::LockHolder lock(ids_lock_);
    for (size_t i = 0; i < internal_thread_ids_.size(); i++) {
        last_id_ = (last_id_ + 1) % internal_thread_ids_.size();
        if (!internal_thread_ids_[last_id_]) {
            internal_thread_ids_.set(last_id_);
            return last_id_ + 1;  // 0 is reserved as uninitialized value.
        }
    }
    LOG(FATAL, RUNTIME) << "Out of internal thread ids";
    UNREACHABLE();
}

void ThreadManager::RemoveInternalThreadId(uint32_t id)
{
    id--;  // 0 is reserved as uninitialized value.
    os::memory::LockHolder lock(ids_lock_);
    ASSERT(internal_thread_ids_[id]);
    internal_thread_ids_.reset(id);
}

MTManagedThread *ThreadManager::GetThreadByInternalThreadIdWithLockHeld(uint32_t thread_id)
{
    // Do not optimize with std::find_if - sometimes there are problems with incorrect memory accesses
    for (auto thread : threads_) {
        if (thread->GetInternalId() == thread_id) {
            return thread;
        }
    }
    return nullptr;
}

bool ThreadManager::DeregisterSuspendedThreads()
{
    auto current = MTManagedThread::GetCurrent();
    auto i = threads_.begin();
    bool is_potentially_blocked_thread_present = false;
    bool is_nonblocked_thread_present = false;
    while (i != threads_.end()) {
        MTManagedThread *thread = *i;
        auto status = thread->GetStatus();
        // Do not deregister current thread (which should be in status NATIVE) as HasNoActiveThreads
        // assumes it stays registered; only threads in statuses FINISHED, IS_TERMINATED_LOOP and NATIVE
        // can be deregistered.
        if (thread != current && CanDeregister(status)) {
            DecreaseCountersForThread(thread);
            i = threads_.erase(i);
            continue;
        }
        if (status == ThreadStatus::NATIVE || status == ThreadStatus::IS_BLOCKED) {
            // We have a blocked thread - there is a potential deadlock
            is_potentially_blocked_thread_present = true;
        } else if (thread != current) {
            // We have at least one non-blocked thread - deadlock is impossible
            is_nonblocked_thread_present = true;
        }
        if (thread != current) {
            LOG(DEBUG, RUNTIME) << "Daemon thread " << thread->GetId()
                                << " remains in DeregisterSuspendedThreads, status = "
                                << ManagedThread::ThreadStatusAsString(status);
        }
        i++;
    }
    if (is_potentially_blocked_thread_present && !is_nonblocked_thread_present) {
        // All threads except current are blocked (have BLOCKED or NATIVE status)
        LOG(DEBUG, RUNTIME) << "Potential deadlock with daemon threads is detected";
        return StopThreadsOnDeadlock(current);
    }
    // Sanity check, we should get at least current thread in that list.
    ASSERT(!threads_.empty());
    return threads_.size() == 1;
}

void ThreadManager::DecreaseCountersForThread(MTManagedThread *thread)
{
    if (thread->IsDaemon()) {
        daemon_threads_count_--;
        // Do not delete this thread structure as it may be used by suspended thread
        daemon_threads_.push_back(thread);
    }
    threads_count_--;
}

bool ThreadManager::StopThreadsOnDeadlock(MTManagedThread *current)
{
    if (!LockOrderGraph::CheckForTerminationLoops(threads_, daemon_threads_, current)) {
        LOG(DEBUG, RUNTIME) << "Deadlock with daemon threads was not confirmed";
        return false;
    }

    os::memory::Mutex::IgnoreChecksOnDeadlock();
    auto i = threads_.begin();
    while (i != threads_.end()) {
        MTManagedThread *thread = *i;
        if (thread != current) {
            DecreaseCountersForThread(thread);
            i = threads_.erase(i);
            continue;
        }
        i++;
    }
    return true;
}

void ThreadManager::WaitForDeregistration()
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    {
        os::memory::LockHolder lock(thread_lock_);

        // First wait for non-daemon threads to finish
        while (!HasNoActiveThreads()) {
            stop_var_.TimedWait(&thread_lock_, WAIT_INTERVAL);
        }

        // Then stop daemon threads
        StopDaemonThreads();

        // Finally wait until all threads are suspended
        while (true) {
            if (pending_threads_ != 0) {
                // There are threads, which are not completely registered
                // We can not destroy other threads, as they may use shared data (waiting mutexes)
                stop_var_.TimedWait(&thread_lock_, WAIT_INTERVAL);
                continue;
            }
            if (DeregisterSuspendedThreads()) {
                break;
            }
            stop_var_.TimedWait(&thread_lock_, WAIT_INTERVAL);
        }

        for (const auto &thread : daemon_threads_) {
            thread->FreeInternalMemory();
        }
    }
    auto threshold = Runtime::GetOptions().GetIgnoreDaemonMemoryLeaksThreshold();
    Runtime::GetCurrent()->SetDaemonMemoryLeakThreshold(daemon_threads_.size() * threshold);
}

void ThreadManager::StopDaemonThreads() REQUIRES(thread_lock_)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    for (auto thread : threads_) {
        if (thread->IsDaemon()) {
            LOG(DEBUG, RUNTIME) << "Stopping daemon thread " << thread->GetId();
            thread->StopDaemonThread();
        }
    }
    // Suspend any future new threads
    suspend_new_count_++;
}

int ThreadManager::GetThreadsCount()
{
    return threads_count_;
}

#ifndef NDEBUG
uint32_t ThreadManager::GetAllRegisteredThreadsCount()
{
    return registered_threads_count_;
}
#endif  // NDEBUG

void ThreadManager::SuspendAllThreads()
{
    trace::ScopedTrace scoped_trace("Suspending mutator threads");
    auto cur_thread = MTManagedThread::GetCurrent();
    os::memory::LockHolder lock(thread_lock_);
    EnumerateThreadsWithLockheld([cur_thread](MTManagedThread *thread) {
        if (thread != cur_thread) {
            thread->SuspendImpl(true);
        }
        return true;
    });
    suspend_new_count_++;
}

bool ThreadManager::IsRunningThreadExist()
{
    auto cur_thread = MTManagedThread::GetCurrent();
    os::memory::LockHolder lock(thread_lock_);
    bool is_exists = false;
    EnumerateThreadsWithLockheld([cur_thread, &is_exists](MTManagedThread *thread) {
        if (thread != cur_thread) {
            if (thread->GetStatus() == ThreadStatus::RUNNING) {
                is_exists = true;
                return false;
            };
        }
        return true;
    });
    return is_exists;
}

void ThreadManager::ResumeAllThreads()
{
    trace::ScopedTrace scoped_trace("Resuming mutator threads");
    auto cur_thread = MTManagedThread::GetCurrent();
    os::memory::LockHolder lock(thread_lock_);
    if (suspend_new_count_ > 0) {
        suspend_new_count_--;
    }
    EnumerateThreadsWithLockheld([cur_thread](MTManagedThread *thread) {
        if (thread != cur_thread) {
            thread->ResumeImpl(true);
        }
        return true;
    });
}

bool ThreadManager::UnregisterExitedThread(MTManagedThread *thread)
{
    ASSERT(MTManagedThread::GetCurrent() == thread);
    {
        thread->NativeCodeEnd();

        os::memory::LockHolder lock(thread_lock_);
        // While this thread is suspended, do not delete it as other thread can be accessing it.
        // TestAllFlags is required because termination request can be sent while thread_lock_ is unlocked
        while (thread->TestAllFlags()) {
            thread_lock_.Unlock();
            thread->SafepointPoll();
            thread_lock_.Lock();
        }

        thread->DestroyInternalResources();

        LOG(DEBUG, RUNTIME) << "Stopping thread " << thread->GetId();
        thread->UpdateStatus(ThreadStatus::FINISHED);
        // Do not delete main thread, Runtime::GetMainThread is expected to always return valid object
        if (thread == main_thread_) {
            return false;
        }

        // This code should happen after thread has been resumed: Both WaitSuspension and ResumeImps requires locking
        // suspend_lock_, so it acts as a memory barrier; flag clean should be visible in this thread after exit from
        // WaitSuspenion
        TSAN_ANNOTATE_HAPPENS_AFTER(&thread->fts_);

        threads_.remove(thread);
        if (thread->IsDaemon()) {
            daemon_threads_count_--;
        }
        threads_count_--;

        // If managed_thread, its nativePeer should be 0 before
        delete thread;
        stop_var_.Signal();
        return true;
    }
}

void ThreadManager::RegisterSensitiveThread() const
{
    LOG(INFO, RUNTIME) << __func__ << " is an empty implementation now.";
}

MTManagedThread *ThreadManager::SuspendAndWaitThreadByInternalThreadId(uint32_t thread_id)
{
    static constexpr uint32_t YIELD_ITERS = 500;
    // NB! Expected to be called in registered thread, change implementation if this function used elsewhere
    MTManagedThread *current = MTManagedThread::GetCurrent();
    MTManagedThread *suspended = nullptr;
    ASSERT(current->GetStatus() != ThreadStatus::RUNNING);

    // Extract target thread
    while (true) {
        // If two threads call SuspendAndWaitThreadByInternalThreadId concurrently, one has to get suspended
        // while other waits for thread to be suspended, so thread_lock_ is required to be held until
        // SuspendImpl is called
        current->SafepointPoll();
        {
            os::memory::LockHolder lock(thread_lock_);

            suspended = GetThreadByInternalThreadIdWithLockHeld(thread_id);
            if (UNLIKELY(suspended == nullptr)) {
                // no thread found, exit
                return nullptr;
            }
            ASSERT(current != suspended);
            if (LIKELY(!current->IsSuspended())) {
                suspended->SuspendImpl(true);
                break;
            }
            // Unsafe to suspend as other thread may be waiting for this thread to suspend;
            // Should get suspended on Safepoint()
        }
    }

    // Now wait until target thread is really suspended
    for (uint32_t loop_iter = 0;; loop_iter++) {
        if (suspended->GetStatus() != ThreadStatus::RUNNING) {
            // Thread is suspended now
            return suspended;
        }
        if (loop_iter < YIELD_ITERS) {
            MTManagedThread::Yield();
        } else {
            static constexpr uint32_t SHORT_SLEEP_MS = 1;
            os::thread::NativeSleep(SHORT_SLEEP_MS);
        }
    }
    UNREACHABLE();
}

}  // namespace panda
