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
#ifndef PANDA_RUNTIME_MTMANAGED_THREAD_H
#define PANDA_RUNTIME_MTMANAGED_THREAD_H

#include "managed_thread.h"

namespace panda {
class MTManagedThread : public ManagedThread {
public:
    ThreadId GetInternalId();

    static MTManagedThread *Create(
        Runtime *runtime, PandaVM *vm,
        panda::panda_file::SourceLang thread_lang = panda::panda_file::SourceLang::PANDA_ASSEMBLY);

    explicit MTManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                             panda::panda_file::SourceLang thread_lang = panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    ~MTManagedThread() override;

    MonitorPool *GetMonitorPool();
    int32_t GetMonitorCount();
    void AddMonitor(Monitor *monitor);
    void RemoveMonitor(Monitor *monitor);
    void ReleaseMonitors();

    void PushLocalObjectLocked(ObjectHeader *obj);
    void PopLocalObjectLocked(ObjectHeader *out);
    Span<LockedObjectInfo> GetLockedObjectInfos();

    void VisitGCRoots(const ObjectVisitor &cb) override;
    void UpdateGCRoots() override;

    ThreadStatus GetWaitingMonitorOldStatus()
    {
        return monitor_old_status_;
    }

    void SetWaitingMonitorOldStatus(ThreadStatus status)
    {
        monitor_old_status_ = status;
    }

    void FreeInternalMemory() override;

    static bool Sleep(uint64_t ms);

    Monitor *GetWaitingMonitor()
    {
        return waiting_monitor_;
    }

    void SetWaitingMonitor(Monitor *monitor)
    {
        ASSERT(waiting_monitor_ == nullptr || monitor == nullptr);
        waiting_monitor_ = monitor;
    }

    Monitor *GetEnteringMonitor() const
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        return entering_monitor_.load(std::memory_order_relaxed);
    }

    void SetEnteringMonitor(Monitor *monitor)
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        ASSERT(entering_monitor_.load(std::memory_order_relaxed) == nullptr || monitor == nullptr);
        // Atomic with relaxed order reason: ordering constraints are not required
        entering_monitor_.store(monitor, std::memory_order_relaxed);
    }

    virtual void StopDaemonThread();

    bool IsDaemon()
    {
        return is_daemon_;
    }

    void SetDaemon();

    virtual void Destroy();

    static void Yield();

    static void Interrupt(MTManagedThread *thread);

    // Need to acquire the mutex before waiting to avoid scheduling between monitor release and clond_lock acquire
    os::memory::Mutex *GetWaitingMutex() RETURN_CAPABILITY(cond_lock_)
    {
        return &cond_lock_;
    }

    void Signal()
    {
        os::memory::LockHolder lock(cond_lock_);
        cond_var_.Signal();
    }

    bool Interrupted();

    bool IsInterrupted()
    {
        os::memory::LockHolder lock(cond_lock_);
        return is_interrupted_;
    }

    bool IsInterruptedWithLockHeld() const REQUIRES(cond_lock_)
    {
        return is_interrupted_;
    }

    void ClearInterrupted()
    {
        os::memory::LockHolder lock(cond_lock_);
        is_interrupted_ = false;
    }

    static bool ThreadIsMTManagedThread(Thread *thread)
    {
        ASSERT(thread != nullptr);
        return thread->GetThreadType() == Thread::ThreadType::THREAD_TYPE_MT_MANAGED;
    }

    static MTManagedThread *CastFromThread(Thread *thread)
    {
        ASSERT(thread != nullptr);
        ASSERT(ThreadIsMTManagedThread(thread));
        return static_cast<MTManagedThread *>(thread);
    }

    /**
     * @brief GetCurrentRaw Unsafe method to get current MTManagedThread.
     * It can be used in hotspots to get the best performance.
     * We can only use this method in places where the MTManagedThread exists.
     * @return pointer to MTManagedThread
     */
    static MTManagedThread *GetCurrentRaw()
    {
        return CastFromThread(Thread::GetCurrent());
    }

    /**
     * @brief GetCurrent Safe method to gets current MTManagedThread.
     * @return pointer to MTManagedThread or nullptr (if current thread is not a managed thread)
     */
    static MTManagedThread *GetCurrent()
    {
        Thread *thread = Thread::GetCurrent();
        ASSERT(thread != nullptr);
        if (ThreadIsMTManagedThread(thread)) {
            return CastFromThread(thread);
        }
        // no guarantee that we will return nullptr here in the future
        return nullptr;
    }

    void WaitWithLockHeld(ThreadStatus wait_status) REQUIRES(cond_lock_)
    {
        ASSERT(wait_status == ThreadStatus::IS_WAITING);
        auto old_status = GetStatus();
        UpdateStatus(wait_status);
        WaitWithLockHeldInternal();
        // Unlock before setting status RUNNING to handle MutatorReadLock without inversed lock order.
        cond_lock_.Unlock();
        UpdateStatus(old_status);
        cond_lock_.Lock();
    }

    static void WaitForSuspension(ManagedThread *thread)
    {
        static constexpr uint32_t YIELD_ITERS = 500;
        uint32_t loop_iter = 0;
        while (thread->GetStatus() == ThreadStatus::RUNNING) {
            if (!thread->IsSuspended()) {
                LOG(WARNING, RUNTIME) << "No request for suspension, do not wait thread " << thread->GetId();
                break;
            }

            loop_iter++;
            if (loop_iter < YIELD_ITERS) {
                MTManagedThread::Yield();
            } else {
                // Use native sleep over ManagedThread::Sleep to prevent potentially time consuming
                // mutator_lock locking and unlocking
                static constexpr uint32_t SHORT_SLEEP_MS = 1;
                os::thread::NativeSleep(SHORT_SLEEP_MS);
            }
        }
    }

    bool TimedWaitWithLockHeld(ThreadStatus wait_status, uint64_t timeout, uint64_t nanos, bool is_absolute = false)
        REQUIRES(cond_lock_)
    {
        ASSERT(wait_status == ThreadStatus::IS_TIMED_WAITING || wait_status == ThreadStatus::IS_SLEEPING ||
               wait_status == ThreadStatus::IS_BLOCKED || wait_status == ThreadStatus::IS_SUSPENDED ||
               wait_status == ThreadStatus::IS_COMPILER_WAITING || wait_status == ThreadStatus::IS_WAITING_INFLATION);
        auto old_status = GetStatus();
        UpdateStatus(wait_status);
        bool res = TimedWaitWithLockHeldInternal(timeout, nanos, is_absolute);
        // Unlock before setting status RUNNING to handle MutatorReadLock without inversed lock order.
        cond_lock_.Unlock();
        UpdateStatus(old_status);
        cond_lock_.Lock();
        return res;
    }

    bool TimedWait(ThreadStatus wait_status, uint64_t timeout, uint64_t nanos = 0, bool is_absolute = false)
    {
        ASSERT(wait_status == ThreadStatus::IS_TIMED_WAITING || wait_status == ThreadStatus::IS_SLEEPING ||
               wait_status == ThreadStatus::IS_BLOCKED || wait_status == ThreadStatus::IS_SUSPENDED ||
               wait_status == ThreadStatus::IS_COMPILER_WAITING || wait_status == ThreadStatus::IS_WAITING_INFLATION);
        auto old_status = GetStatus();
        bool res = false;
        {
            os::memory::LockHolder lock(cond_lock_);
            UpdateStatus(wait_status);
            res = TimedWaitWithLockHeldInternal(timeout, nanos, is_absolute);
        }
        UpdateStatus(old_status);
        return res;
    }

    void OnRuntimeTerminated() override
    {
        TerminationLoop();
    }

    void TerminationLoop()
    {
        ASSERT(IsRuntimeTerminated());
        if (GetStatus() == ThreadStatus::NATIVE) {
            // There is a chance, that the runtime will be destroyed at this time.
            // Thus we should not release monitors for NATIVE status
        } else {
            ReleaseMonitors();
            UpdateStatus(ThreadStatus::IS_TERMINATED_LOOP);
        }
        while (true) {
            static constexpr unsigned int LONG_SLEEP_MS = 1000000;
            os::thread::NativeSleep(LONG_SLEEP_MS);
        }
    }

    ObjectHeader *GetEnterMonitorObject()
    {
        ASSERT_MANAGED_CODE();
        return enter_monitor_object_;
    }

    void SetEnterMonitorObject(ObjectHeader *object_header)
    {
        ASSERT_MANAGED_CODE();
        enter_monitor_object_ = object_header;
    }

    MTManagedThread *GetNextWait() const
    {
        return next_;
    }

    void SetWaitNext(MTManagedThread *next)
    {
        next_ = next;
    }

    mem::ReferenceStorage *GetPtReferenceStorage() const
    {
        return pt_reference_storage_.get();
    }

    static constexpr uint32_t GetLockedObjectCapacityOffset()
    {
        return GetLocalObjectLockedOffset() + LockedObjectList<>::GetCapacityOffset();
    }

    static constexpr uint32_t GetLockedObjectSizeOffset()
    {
        return GetLocalObjectLockedOffset() + LockedObjectList<>::GetSizeOffset();
    }

    static constexpr uint32_t GetLockedObjectDataOffset()
    {
        return GetLocalObjectLockedOffset() + LockedObjectList<>::GetDataOffset();
    }

    static constexpr uint32_t GetLocalObjectLockedOffset()
    {
        return MEMBER_OFFSET(MTManagedThread, local_objects_locked_);
    }

protected:
    virtual void ProcessCreatedThread();

    void WaitWithLockHeldInternal() REQUIRES(cond_lock_)
    {
        ASSERT(this == ManagedThread::GetCurrent());
        cond_var_.Wait(&cond_lock_);
    }

    bool TimedWaitWithLockHeldInternal(uint64_t timeout, uint64_t nanos, bool is_absolute = false) REQUIRES(cond_lock_)
    {
        ASSERT(this == ManagedThread::GetCurrent());
        return cond_var_.TimedWait(&cond_lock_, timeout, nanos, is_absolute);
    }

    void SignalWithLockHeld() REQUIRES(cond_lock_)
    {
        cond_var_.Signal();
    }

    void SetInterruptedWithLockHeld(bool interrupted) REQUIRES(cond_lock_)
    {
        is_interrupted_ = interrupted;
    }

private:
    MTManagedThread *next_ {nullptr};

    LockedObjectList<> local_objects_locked_;

    // Implementation of Wait/Notify
    os::memory::ConditionVariable cond_var_ GUARDED_BY(cond_lock_);
    os::memory::Mutex cond_lock_;

    bool is_interrupted_ GUARDED_BY(cond_lock_) = false;

    bool is_daemon_ = false;

    Monitor *waiting_monitor_;

    // Count of monitors owned by this thread
    std::atomic_int32_t monitor_count_ {0};
    // Used for dumping stack info
    ThreadStatus monitor_old_status_ {ThreadStatus::FINISHED};
    ObjectHeader *enter_monitor_object_ {nullptr};

    // Monitor, in which this thread is entering. It is required only to detect deadlocks with daemon threads.
    std::atomic<Monitor *> entering_monitor_;

    PandaUniquePtr<mem::ReferenceStorage> pt_reference_storage_ {nullptr};

    NO_COPY_SEMANTIC(MTManagedThread);
    NO_MOVE_SEMANTIC(MTManagedThread);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_MTMANAGED_THREAD_H
