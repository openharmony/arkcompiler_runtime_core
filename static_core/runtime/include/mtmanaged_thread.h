/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/mutator.h"
#include "runtime/include/managed_thread.h"

namespace ark {

class LockedObjectInfo {
public:
    LockedObjectInfo(ObjectHeader *obj, void *fp) : object_(obj), stack_(fp) {}
    inline ObjectHeader *GetObject() const
    {
        return object_;
    }

    inline void SetObject(ObjectHeader *objNew)
    {
        object_ = objNew;
    }

    ALWAYS_INLINE void UpdateObject(const ReferenceUpdater &updater)
    {
        [[maybe_unused]] ObjectStatus status = updater(&object_);
        ASSERT(status == ObjectStatus::ALIVE_OBJECT);
    }

    inline void *GetStack() const
    {
        return stack_;
    }

    inline void SetStack(void *stackNew)
    {
        stack_ = stackNew;
    }

    static constexpr uint32_t GetMonitorOffset()
    {
        return MEMBER_OFFSET(LockedObjectInfo, object_);
    }

    static constexpr uint32_t GetStackOffset()
    {
        return MEMBER_OFFSET(LockedObjectInfo, stack_);
    }

private:
    ObjectHeader *object_;
    void *stack_;
};

template <typename Adapter = mem::AllocatorAdapter<LockedObjectInfo>>
class LockedObjectList {
    static constexpr uint32_t DEFAULT_CAPACITY = 16;

public:
    LockedObjectList() : capacity_(DEFAULT_CAPACITY), allocator_(Adapter())
    {
        storage_ = allocator_.allocate(DEFAULT_CAPACITY);
    }

    ~LockedObjectList()
    {
        allocator_.deallocate(storage_, capacity_);
    }

    NO_COPY_SEMANTIC(LockedObjectList);
    NO_MOVE_SEMANTIC(LockedObjectList);

    void PushBack(LockedObjectInfo data)
    {
        ExtendIfNeeded();
        storage_[size_++] = data;
    }

    template <typename... Args>
    LockedObjectInfo &EmplaceBack(Args &&...args)
    {
        ExtendIfNeeded();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *rawMem = &storage_[size_];
        auto *datum = new (rawMem) LockedObjectInfo(std::forward<Args>(args)...);
        size_++;
        return *datum;
    }

    LockedObjectInfo &Back()
    {
        ASSERT(size_ > 0);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return storage_[size_ - 1];
    }

    bool Empty() const
    {
        return size_ == 0;
    }

    void PopBack()
    {
        ASSERT(size_ > 0);
        --size_;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        (&storage_[size_])->~LockedObjectInfo();
    }

    Span<LockedObjectInfo> Data()
    {
        return Span<LockedObjectInfo>(storage_, size_);
    }

    static constexpr uint32_t GetCapacityOffset()
    {
        return MEMBER_OFFSET(LockedObjectList, capacity_);
    }

    static constexpr uint32_t GetSizeOffset()
    {
        return MEMBER_OFFSET(LockedObjectList, size_);
    }

    static constexpr uint32_t GetDataOffset()
    {
        return MEMBER_OFFSET(LockedObjectList, storage_);
    }

private:
    void ExtendIfNeeded()
    {
        ASSERT(size_ <= capacity_);
        if (size_ < capacity_) {
            return;
        }
        uint32_t newCapacity = capacity_ * 3U / 2U;  // expand by 1.5
        LockedObjectInfo *newStorage = allocator_.allocate(newCapacity);
        ASSERT(newStorage != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::copy(storage_, storage_ + size_, newStorage);
        allocator_.deallocate(storage_, capacity_);
        storage_ = newStorage;
        capacity_ = newCapacity;
    }

    template <typename T, size_t ALIGNMENT = sizeof(T)>
    using Aligned __attribute__((aligned(ALIGNMENT))) = T;
    // Use uint32_t instead of size_t to guarantee the same size
    // on all platforms and simplify compiler stubs accessing this fields.
    // uint32_t is large enough to fit locked objects list's size.
    Aligned<uint32_t> capacity_;
    Aligned<uint32_t> size_ {0};
    Aligned<LockedObjectInfo *> storage_;
    Adapter allocator_;
};

class MTManagedThread : public ManagedThread {
public:
    PANDA_PUBLIC_API static MTManagedThread *Create(
        Runtime *runtime, PandaVM *vm,
        ark::panda_file::SourceLang threadLang = ark::panda_file::SourceLang::PANDA_ASSEMBLY);

    explicit MTManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                             ark::panda_file::SourceLang threadLang = ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    ~MTManagedThread() override;

    MonitorPool *GetMonitorPool();
    int32_t GetMonitorCount();
    void AddMonitor(Monitor *monitor);
    void RemoveMonitor(Monitor *monitor);
    void ReleaseMonitors();

    void PushLocalObjectLocked(ObjectHeader *obj);
    void PopLocalObjectLocked(ObjectHeader *out);
    Span<LockedObjectInfo> GetLockedObjectInfos();

    void VisitGCRoots(const GCRootVisitor &cb) override;
    void UpdateAndSweep(const ReferenceUpdater &updater) override;

    MutatorStatus GetWaitingMonitorOldStatus()
    {
        return monitorOldStatus_;
    }

    void SetWaitingMonitorOldStatus(MutatorStatus status)
    {
        monitorOldStatus_ = status;
    }

    void FreeInternalMemory() override;

    static bool Sleep(uint64_t ms);

    Monitor *GetWaitingMonitor()
    {
        return waitingMonitor_;
    }

    void SetWaitingMonitor(Monitor *monitor)
    {
        ASSERT(waitingMonitor_ == nullptr || monitor == nullptr);
        waitingMonitor_ = monitor;
    }

    Monitor *GetEnteringMonitor() const
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        return enteringMonitor_.load(std::memory_order_relaxed);
    }

    void SetEnteringMonitor(Monitor *monitor)
    {
        // Atomic with relaxed order reason: ordering constraints are not required
        ASSERT(enteringMonitor_.load(std::memory_order_relaxed) == nullptr || monitor == nullptr);
        // Atomic with relaxed order reason: ordering constraints are not required
        enteringMonitor_.store(monitor, std::memory_order_relaxed);
    }

    virtual void StopDaemonThread();

    /* @sync 1
     * @description This synchronization point can be used to add
     * new attributes or methods to this class.
     */

    bool IsDaemon()
    {
        return isDaemon_;
    }

    void SetDaemon();

    virtual void Destroy();

    PANDA_PUBLIC_API static void Yield();

    PANDA_PUBLIC_API static void Interrupt(MTManagedThread *thread);

    // Need to acquire the mutex before waiting to avoid scheduling between monitor release and clond_lock acquire
    os::memory::Mutex *GetWaitingMutex() RETURN_CAPABILITY(condLock_)
    {
        return &condLock_;
    }

    void Signal()
    {
        os::memory::LockHolder lock(condLock_);
        condVar_.Signal();
    }

    PANDA_PUBLIC_API bool Interrupted();

    bool IsInterrupted()
    {
        os::memory::LockHolder lock(condLock_);
        return isInterrupted_;
    }

    bool IsInterruptedWithLockHeld() const REQUIRES(condLock_)
    {
        return isInterrupted_;
    }

    void ClearInterrupted()
    {
        os::memory::LockHolder lock(condLock_);
        /* @sync 1
         * @description Before we clear is_interrupted_ flag.
         * */
        isInterrupted_ = false;
    }

    static bool MutatorIsMTManagedThread(Mutator *mutator)
    {
        ASSERT(mutator != nullptr);
        return (mutator->GetMutatorType() == MutatorType::MANAGED) &&
               (static_cast<ManagedThread *>(mutator)->GetManagedThreadType() ==
                ManagedThread::ThreadType::THREAD_TYPE_MT_MANAGED);
    }

    static MTManagedThread *CastFromMutator(Mutator *mutator)
    {
        ASSERT(MutatorIsMTManagedThread(mutator));
        return static_cast<MTManagedThread *>(mutator);
    }

    /**
     * @brief GetCurrentRaw Unsafe method to get current MTManagedThread.
     * It can be used in hotspots to get the best performance.
     * We can only use this method in places where the MTManagedThread exists.
     * @return pointer to MTManagedThread
     */
    static MTManagedThread *GetCurrentRaw()
    {
        return CastFromMutator(Mutator::GetCurrent());
    }

    /**
     * @brief GetCurrent Safe method to gets current MTManagedThread.
     * @return pointer to MTManagedThread or nullptr (if current thread is not a managed thread)
     */
    static PANDA_PUBLIC_API MTManagedThread *GetCurrent()
    {
        Mutator *mutator = Mutator::GetCurrent();
        ASSERT(mutator != nullptr);
        if (MutatorIsMTManagedThread(mutator)) {
            return CastFromMutator(mutator);
        }
        // no guarantee that we will return nullptr here in the future
        return nullptr;
    }

    void WaitWithLockHeld(MutatorStatus waitStatus) REQUIRES(condLock_)
    {
        ASSERT(waitStatus == MutatorStatus::IS_WAITING);
        auto oldStatus = GetStatus();
        UpdateStatus(waitStatus);
        WaitWithLockHeldInternal();
        // Unlock before setting status RUNNING to handle MutatorReadLock without inversed lock order.
        condLock_.Unlock();
        UpdateStatus(oldStatus);
        condLock_.Lock();
    }

    static void WaitForSuspension(ManagedThread *thread)
    {
        static constexpr uint32_t YIELD_ITERS = 500;
        uint32_t loopIter = 0;
        while (thread->GetStatus() == MutatorStatus::RUNNING) {
            if (!thread->IsSuspended()) {
                LOG(WARNING, RUNTIME) << "No request for suspension, do not wait thread " << thread->GetId();
                break;
            }

            loopIter++;
            if (loopIter < YIELD_ITERS) {
                MTManagedThread::Yield();
            } else {
                // Use native sleep over ManagedThread::Sleep to prevent potentially time consuming
                // mutator_lock locking and unlocking
                static constexpr uint32_t SHORT_SLEEP_MS = 1;
                os::thread::NativeSleep(SHORT_SLEEP_MS);
            }
        }
    }

    bool TimedWaitWithLockHeld(MutatorStatus waitStatus, uint64_t timeout, uint64_t nanos, bool isAbsolute = false)
        REQUIRES(condLock_)
    {
        ASSERT(waitStatus == MutatorStatus::IS_TIMED_WAITING || waitStatus == MutatorStatus::IS_SLEEPING ||
               waitStatus == MutatorStatus::IS_BLOCKED || waitStatus == MutatorStatus::IS_SUSPENDED ||
               waitStatus == MutatorStatus::IS_COMPILER_WAITING || waitStatus == MutatorStatus::IS_WAITING_INFLATION);
        auto oldStatus = GetStatus();
        UpdateStatus(waitStatus);
        bool res = TimedWaitWithLockHeldInternal(timeout, nanos, isAbsolute);
        // Unlock before setting status RUNNING to handle MutatorReadLock without inversed lock order.
        condLock_.Unlock();
        UpdateStatus(oldStatus);
        condLock_.Lock();
        return res;
    }

    bool TimedWait(MutatorStatus waitStatus, uint64_t timeout, uint64_t nanos = 0, bool isAbsolute = false)
    {
        ASSERT(waitStatus == MutatorStatus::IS_TIMED_WAITING || waitStatus == MutatorStatus::IS_SLEEPING ||
               waitStatus == MutatorStatus::IS_BLOCKED || waitStatus == MutatorStatus::IS_SUSPENDED ||
               waitStatus == MutatorStatus::IS_COMPILER_WAITING || waitStatus == MutatorStatus::IS_WAITING_INFLATION);
        auto oldStatus = GetStatus();
        bool res = false;
        {
            os::memory::LockHolder lock(condLock_);
            UpdateStatus(waitStatus);
            /* @sync 1
             * @description Right after changing the thread's status and before going to sleep
             * */
            res = TimedWaitWithLockHeldInternal(timeout, nanos, isAbsolute);
        }
        UpdateStatus(oldStatus);
        return res;
    }

    void OnRuntimeTerminated() override
    {
        TerminationLoop();
    }

    void TerminationLoop()
    {
        ASSERT(IsRuntimeTerminated());
        /* @sync 1
         * @description This point is right before the thread starts to release all his monitors.
         * All monitors should be released by the thread before completing its execution by stepping into the
         * termination loop.
         * */
        if (GetStatus() == MutatorStatus::NATIVE) {
            // There is a chance, that the runtime will be destroyed at this time.
            // Thus we should not release monitors for NATIVE status
        } else {
            ReleaseMonitors();
            /* @sync 2
             * @description This point is right after the thread has released all his monitors and right before it steps
             * into the termination loop.
             * */
            UpdateStatus(MutatorStatus::IS_TERMINATED_LOOP);
            /* @sync 3
             * @description This point is right after the thread has released all his monitors and changed status to
             * IS_TERMINATED_LOOP
             * */
        }
        while (true) {
            static constexpr unsigned int LONG_SLEEP_MS = 1000000;
            os::thread::NativeSleep(LONG_SLEEP_MS);
        }
    }

    ObjectHeader *GetEnterMonitorObject()
    {
        ASSERT_MANAGED_CODE();
        return enterMonitorObject_;
    }

    void SetEnterMonitorObject(ObjectHeader *objectHeader)
    {
        ASSERT_MANAGED_CODE();
        enterMonitorObject_ = objectHeader;
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
        return ptReferenceStorage_.get();
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
        return MEMBER_OFFSET(MTManagedThread, localObjectsLocked_);
    }

protected:
    virtual void ProcessCreatedThread();

    void WaitWithLockHeldInternal() REQUIRES(condLock_)
    {
        ASSERT(this == ManagedThread::GetCurrent());
        condVar_.Wait(&condLock_);
    }

    bool TimedWaitWithLockHeldInternal(uint64_t timeout, uint64_t nanos, bool isAbsolute = false) REQUIRES(condLock_)
    {
        ASSERT(this == ManagedThread::GetCurrent());
        return condVar_.TimedWait(&condLock_, timeout, nanos, isAbsolute);
    }

    void SignalWithLockHeld() REQUIRES(condLock_)
    {
        condVar_.Signal();
    }

    void SetInterruptedWithLockHeld(bool interrupted) REQUIRES(condLock_)
    {
        isInterrupted_ = interrupted;
    }

private:
    Thread *vmThread_ {nullptr};
    MTManagedThread *next_ {nullptr};

    LockedObjectList<> localObjectsLocked_;

    // Implementation of Wait/Notify
    os::memory::ConditionVariable condVar_ GUARDED_BY(condLock_);
    os::memory::Mutex condLock_;

    bool isInterrupted_ GUARDED_BY(condLock_) = false;

    bool isDaemon_ = false;

    Monitor *waitingMonitor_ {nullptr};

    // Count of monitors owned by this thread
    std::atomic_int32_t monitorCount_ {0};
    // Used for dumping stack info
    MutatorStatus monitorOldStatus_ {MutatorStatus::FINISHED};
    ObjectHeader *enterMonitorObject_ {nullptr};

    // Monitor, in which this thread is entering. It is required only to detect deadlocks with daemon threads.
    std::atomic<Monitor *> enteringMonitor_;

    PandaUniquePtr<mem::ReferenceStorage> ptReferenceStorage_ {nullptr};

    NO_COPY_SEMANTIC(MTManagedThread);
    NO_MOVE_SEMANTIC(MTManagedThread);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_MTMANAGED_THREAD_H
