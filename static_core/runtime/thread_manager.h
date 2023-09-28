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
#ifndef PANDA_RUNTIME_THREAD_MANAGER_H_
#define PANDA_RUNTIME_THREAD_MANAGER_H_

#include <bitset>

#include "libpandabase/os/mutex.h"
#include "libpandabase/utils/time.h"
#include "libpandabase/os/time.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/include/thread_status.h"
#include "runtime/include/locks.h"

namespace panda {

// This interval is required for waiting for threads to stop.
static const int WAIT_INTERVAL = 10;
static constexpr int64_t K_MAX_DUMP_TIME_NS = UINT64_C(6 * 1000 * 1000 * 1000);   // 6s
static constexpr int64_t K_MAX_SINGLE_DUMP_TIME_NS = UINT64_C(50 * 1000 * 1000);  // 50ms

enum class EnumerationFlag {
    NONE = 0,                 // Nothing
    NON_CORE_THREAD = 1,      // Plugin type thread
    MANAGED_CODE_THREAD = 2,  // Thread which can execute managed code
    VM_THREAD = 4,            // Includes VM threads
    ALL = 8,                  // See the comment in the function SatisfyTheMask below
};

class ThreadManager {
public:
    NO_COPY_SEMANTIC(ThreadManager);
    NO_MOVE_SEMANTIC(ThreadManager);

    using Callback = std::function<bool(ManagedThread *)>;

    explicit ThreadManager() = default;
    virtual ~ThreadManager() = default;

    /**
     * @brief thread enumeration and applying @param cb to them
     * @return true if for every thread @param cb was successful (returned true) and false otherwise
     */
    bool EnumerateThreads(const Callback &cb, unsigned int inc_mask = static_cast<unsigned int>(EnumerationFlag::ALL),
                          unsigned int xor_mask = static_cast<unsigned int>(EnumerationFlag::NONE)) const
    {
        return EnumerateThreadsImpl(cb, inc_mask, xor_mask);
    }

    virtual void WaitForDeregistration() = 0;

    virtual void SuspendAllThreads() = 0;
    virtual void ResumeAllThreads() = 0;

    virtual bool IsRunningThreadExist() = 0;

    ManagedThread *GetMainThread() const
    {
        return main_thread_;
    }

    void SetMainThread(ManagedThread *thread)
    {
        main_thread_ = thread;
    }

protected:
    bool SatisfyTheMask(ManagedThread *t, unsigned int mask) const
    {
        if ((mask & static_cast<unsigned int>(EnumerationFlag::ALL)) != 0) {
            // Some uninitialized threads may not have attached flag,
            // So, they are not included as MANAGED_CODE_THREAD.
            // Newly created threads are using flag suspend new count.
            // The case leads to deadlocks, when the thread can not be resumed.
            // To deal with it, just add a specific ALL case
            return true;
        }

        // For NONE mask
        bool target = false;
        if ((mask & static_cast<unsigned int>(EnumerationFlag::MANAGED_CODE_THREAD)) != 0) {
            target = t->IsAttached();
            if ((mask & static_cast<unsigned int>(EnumerationFlag::NON_CORE_THREAD)) != 0) {
                // Due to hyerarhical structure, we need to conjunct types
                bool non_core_thread = t->GetThreadLang() != panda::panda_file::SourceLang::PANDA_ASSEMBLY;
                target = target && non_core_thread;
            }
        }

        if ((mask & static_cast<unsigned int>(EnumerationFlag::VM_THREAD)) != 0) {
            target = target || t->IsVMThread();
        }

        return target;
    }

    bool ApplyCallbackToThread(const Callback &cb, ManagedThread *t, unsigned int inc_mask, unsigned int xor_mask) const
    {
        bool inc_target = SatisfyTheMask(t, inc_mask);
        bool xor_target = SatisfyTheMask(t, xor_mask);
        if (inc_target != xor_target) {
            if (!cb(t)) {
                return false;
            }
        }
        return true;
    }

    virtual bool EnumerateThreadsImpl(const Callback &cb, unsigned int inc_mask, unsigned int xor_mask) const = 0;

private:
    ManagedThread *main_thread_ {nullptr};
};

class MTThreadManager : public ThreadManager {
public:
    NO_COPY_SEMANTIC(MTThreadManager);
    NO_MOVE_SEMANTIC(MTThreadManager);

    // For performance reasons don't exceed specified amount of bits.
    static constexpr size_t MAX_INTERNAL_THREAD_ID = std::min(0xffffU, ManagedThread::MAX_INTERNAL_THREAD_ID);

    explicit MTThreadManager(mem::InternalAllocatorPtr allocator);

    ~MTThreadManager() override;

    bool EnumerateThreadsWithLockheld(const Callback &cb,
                                      unsigned int inc_mask = static_cast<unsigned int>(EnumerationFlag::ALL),
                                      unsigned int xor_mask = static_cast<unsigned int>(EnumerationFlag::NONE)) const
    // REQUIRES(*GetThreadsLock())
    // Cannot enable the annotation, as the function is also called with thread_lock directly
    {
        for (auto t : GetThreadsList()) {
            if (!ApplyCallbackToThread(cb, t, inc_mask, xor_mask)) {
                return false;
            }
        }
        return true;
    }

    template <class Callback>
    void EnumerateThreadsForDump(const Callback &cb, std::ostream &os)
    {
        // TODO(00510180 & 00537420) can not get WriteLock() when other thread run code "while {}"
        // issue #3085
        SuspendAllThreads();
        MTManagedThread *self = MTManagedThread::GetCurrent();
        self->GetMutatorLock()->WriteLock();
        {
            os << "ARK THREADS (" << threads_count_ << "):\n";
        }
        if (self != nullptr) {
            os::memory::LockHolder lock(thread_lock_);
            int64_t start = panda::os::time::GetClockTimeInThreadCpuTime();
            int64_t end;
            int64_t last_time = start;
            cb(self, os);
            for (const auto &thread : threads_) {
                if (thread != self) {
                    cb(thread, os);
                    end = panda::os::time::GetClockTimeInThreadCpuTime();
                    if ((end - last_time) > K_MAX_SINGLE_DUMP_TIME_NS) {
                        LOG(ERROR, RUNTIME) << "signal catcher: thread_list_dump thread : " << thread->GetId()
                                            << "timeout : " << (end - last_time);
                    }
                    last_time = end;
                    if ((end - start) > K_MAX_DUMP_TIME_NS) {
                        LOG(ERROR, RUNTIME) << "signal catcher: thread_list_dump timeout : " << end - start << "\n";
                        break;
                    }
                }
            }
        }
        DumpUnattachedThreads(os);
        self->GetMutatorLock()->Unlock();
        ResumeAllThreads();
    }

    void DumpUnattachedThreads(std::ostream &os);

    void RegisterThread(MTManagedThread *thread)
    {
        os::memory::LockHolder lock(thread_lock_);
        auto main_thread = GetMainThread();
        if (main_thread != nullptr) {
            thread->SetPreWrbEntrypoint(main_thread->GetPreWrbEntrypoint());
        }
        threads_count_++;
#ifndef NDEBUG
        registered_threads_count_++;
#endif  // NDEBUG
        threads_.emplace_back(thread);
        for (uint32_t i = suspend_new_count_; i > 0; i--) {
            thread->SuspendImpl(true);
        }
    }

    void IncPendingThreads()
    {
        os::memory::LockHolder lock(thread_lock_);
        pending_threads_++;
    }

    void DecPendingThreads()
    {
        os::memory::LockHolder lock(thread_lock_);
        pending_threads_--;
    }

    void AddDaemonThread()
    {
        daemon_threads_count_++;
    }

    int GetThreadsCount();

#ifndef NDEBUG
    uint32_t GetAllRegisteredThreadsCount();
#endif  // NDEBUG

    void WaitForDeregistration() override;

    void SuspendAllThreads() override;
    void ResumeAllThreads() override;

    uint32_t GetInternalThreadId();

    void RemoveInternalThreadId(uint32_t id);

    bool IsRunningThreadExist() override;

    // Returns true if unregistration succeeded; for now it can fail when we are trying to unregister main thread
    bool UnregisterExitedThread(MTManagedThread *thread);

    MTManagedThread *SuspendAndWaitThreadByInternalThreadId(uint32_t thread_id);

    void RegisterSensitiveThread() const;

    os::memory::Mutex *GetThreadsLock()
    {
        return &thread_lock_;
    }

protected:
    bool EnumerateThreadsImpl(const Callback &cb, unsigned int inc_mask, unsigned int xor_mask) const override
    {
        os::memory::LockHolder lock(*GetThreadsLock());
        return EnumerateThreadsWithLockheld(cb, inc_mask, xor_mask);
    }

    // The methods are used only in EnumerateThreads in mt mode
    const PandaList<MTManagedThread *> &GetThreadsList() const
    {
        return threads_;
    }
    os::memory::Mutex *GetThreadsLock() const
    {
        return &thread_lock_;
    }

private:
    bool HasNoActiveThreads() const REQUIRES(thread_lock_)
    {
        ASSERT(threads_count_ >= daemon_threads_count_);
        auto thread = static_cast<uint32_t>(threads_count_ - daemon_threads_count_);
        return thread < 2 && pending_threads_ == 0;
    }

    bool StopThreadsOnTerminationLoops(MTManagedThread *current) REQUIRES(thread_lock_);

    /**
     * Tries to stop all daemon threads in case there are no active basic threads
     * returns false if we need to wait
     */
    void StopDaemonThreads() REQUIRES(thread_lock_);

    /**
     * Deregister all suspended threads including daemon threads.
     * Returns true on success and false otherwise.
     */
    bool DeregisterSuspendedThreads() REQUIRES(thread_lock_);

    void DecreaseCountersForThread(MTManagedThread *thread) REQUIRES(thread_lock_);

    MTManagedThread *GetThreadByInternalThreadIdWithLockHeld(uint32_t thread_id) REQUIRES(thread_lock_);

    bool CanDeregister(enum ThreadStatus status)
    {
        // Deregister thread only for IS_TERMINATED_LOOP.
        // In all other statuses we should wait:
        // * CREATED - wait until threads finish initializing which requires communication with ThreadManager;
        // * BLOCKED - it means we are trying to acquire lock in Monitor, which was created in internalAllocator;
        // * TERMINATING - threads which requires communication with Runtime;
        // * FINISHED threads should be deleted itself;
        // * NATIVE threads are either go to FINISHED status or considered a part of a deadlock;
        // * other statuses - should eventually go to IS_TERMINATED_LOOP or FINISHED status.
        return status == ThreadStatus::IS_TERMINATED_LOOP;
    }

    mutable os::memory::Mutex thread_lock_;
    // Counter used to suspend newly created threads after SuspendAllThreads/SuspendDaemonThreads
    uint32_t suspend_new_count_ GUARDED_BY(thread_lock_) = 0;
    // We should delete only finished thread structures, so call delete explicitly on finished threads
    // and don't touch other pointers
    PandaList<MTManagedThread *> threads_ GUARDED_BY(thread_lock_);
    os::memory::Mutex ids_lock_;
    std::bitset<MAX_INTERNAL_THREAD_ID> internal_thread_ids_ GUARDED_BY(ids_lock_);
    uint32_t last_id_ GUARDED_BY(ids_lock_);
    PandaList<MTManagedThread *> daemon_threads_;

    os::memory::ConditionVariable stop_var_;
    std::atomic_uint32_t threads_count_ = 0;
#ifndef NDEBUG
    // This field is required for counting all registered threads (including finished daemons)
    // in AttachThreadTest. It is not needed in production mode.
    std::atomic_uint32_t registered_threads_count_ = 0;
#endif  // NDEBUG
    std::atomic_uint32_t daemon_threads_count_ = 0;
    // A specific counter of threads, which are not completely created
    // When the counter != 0, operations with thread set are permitted to avoid destruction of shared data (mutexes)
    // Synchronized with lock (not atomic) for mutual exclusion with thread operations
    int pending_threads_ GUARDED_BY(thread_lock_);
};

}  // namespace panda

#endif  // PANDA_RUNTIME_THREAD_MANAGER_H_
