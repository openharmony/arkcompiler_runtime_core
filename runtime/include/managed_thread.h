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
#ifndef PANDA_RUNTIME_MANAGED_THREAD_H
#define PANDA_RUNTIME_MANAGED_THREAD_H

#include "thread.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_MANAGED_CODE() ASSERT(::panda::ManagedThread::GetCurrent()->IsManagedCode())
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_NATIVE_CODE() ASSERT(::panda::ManagedThread::GetCurrent()->IsInNativeCode())

namespace panda {
/**
 * \brief Class represents managed thread
 *
 * When the thread is created it registers itself in the runtime, so
 * runtime knows about all managed threads at any given time.
 *
 * This class should be used to store thread specitic information that
 * is necessary to execute managed code:
 *  - Frame
 *  - Exception
 *  - Interpreter cache
 *  - etc.
 *
 *  Now it's used by interpreter to store current frame only.
 */
class ManagedThread : public Thread {
public:
    enum ThreadState : uint8_t { NATIVE_CODE = 0, MANAGED_CODE = 1 };

    using native_handle_type = os::thread::native_handle_type;
    static constexpr ThreadId NON_INITIALIZED_THREAD_ID = 0;
    static constexpr ThreadId MAX_INTERNAL_THREAD_ID = MarkWord::LIGHT_LOCK_THREADID_MAX_COUNT;
    static constexpr size_t STACK_MAX_SIZE_OVERFLOW_CHECK = 256_MB;
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON) || !defined(NDEBUG)
    static constexpr size_t STACK_OVERFLOW_RESERVED_SIZE = 64_KB;
#else
    static constexpr size_t STACK_OVERFLOW_RESERVED_SIZE = 8_KB;
#endif
    static constexpr size_t STACK_OVERFLOW_PROTECTED_SIZE = 4_KB;

    void SetLanguageContext([[maybe_unused]] const LanguageContext &ctx)
    {
        // Deprecated method, don't use it. Only for copability with ets_runtime.
    }

    void SetCurrentFrame(Frame *f)
    {
        frame_ = f;
    }

    tooling::PtThreadInfo *GetPtThreadInfo() const
    {
        return pt_thread_info_.get();
    }

    Frame *GetCurrentFrame() const
    {
        return frame_;
    }

    void *GetFrame() const
    {
        void *fp = GetCurrentFrame();
        if (IsCurrentFrameCompiled()) {
            return (StackWalker::IsBoundaryFrame<FrameKind::INTERPRETER>(fp))
                       ? (StackWalker::GetPrevFromBoundary<FrameKind::COMPILER>(fp))
                       : fp;
        }
        return fp;
    }

    bool IsCurrentFrameCompiled() const
    {
        return is_compiled_frame_;
    }

    void SetCurrentFrameIsCompiled(bool value)
    {
        is_compiled_frame_ = value;
    }

    void SetException(ObjectHeader *exception)
    {
        exception_ = exception;
    }

    ObjectHeader *GetException() const
    {
        return exception_;
    }

    bool HasPendingException() const
    {
        return exception_ != nullptr;
    }

    void ClearException()
    {
        exception_ = nullptr;
    }

    static bool ThreadIsManagedThread(const Thread *thread)
    {
        ASSERT(thread != nullptr);
        Thread::ThreadType thread_type = thread->GetThreadType();
        return thread_type == Thread::ThreadType::THREAD_TYPE_MANAGED ||
               thread_type == Thread::ThreadType::THREAD_TYPE_MT_MANAGED ||
               thread_type == Thread::ThreadType::THREAD_TYPE_TASK;
    }

    static ManagedThread *CastFromThread(Thread *thread)
    {
        ASSERT(thread != nullptr);
        ASSERT(ThreadIsManagedThread(thread));
        return static_cast<ManagedThread *>(thread);
    }

    /**
     * @brief GetCurrentRaw Unsafe method to get current ManagedThread.
     * It can be used in hotspots to get the best performance.
     * We can only use this method in places where the ManagedThread exists.
     * @return pointer to ManagedThread
     */
    static ManagedThread *GetCurrentRaw()
    {
        return CastFromThread(Thread::GetCurrent());
    }

    /**
     * @brief GetCurrent Safe method to gets current ManagedThread.
     * @return pointer to ManagedThread or nullptr (if current thread is not a managed thread)
     */
    static ManagedThread *GetCurrent()
    {
        Thread *thread = Thread::GetCurrent();
        ASSERT(thread != nullptr);
        if (ThreadIsManagedThread(thread)) {
            return CastFromThread(thread);
        }
        return nullptr;
    }

    static void Initialize();

    static void Shutdown();

    bool IsThreadAlive()
    {
        return GetStatus() != ThreadStatus::FINISHED;
    }

    void UpdateStatus(enum ThreadStatus status)
    {
        ASSERT(ManagedThread::GetCurrent() == this);

        ThreadStatus old_status = GetStatus();
        if (old_status == ThreadStatus::RUNNING && status != ThreadStatus::RUNNING) {
            TransitionFromRunningToSuspended(status);
        } else if (old_status != ThreadStatus::RUNNING && status == ThreadStatus::RUNNING) {
            // NB! This thread is treated as suspended so when we transition from suspended state to
            // running we need to check suspension flag and counter so SafepointPoll has to be done before
            // acquiring mutator_lock.
            // StoreStatus acquires lock here
            StoreStatus<CHECK_SAFEPOINT, READLOCK>(ThreadStatus::RUNNING);
        } else if (old_status == ThreadStatus::NATIVE && status != ThreadStatus::IS_TERMINATED_LOOP &&
                   IsRuntimeTerminated()) {
            // If a daemon thread with NATIVE status was deregistered, it should not access any managed object,
            // i.e. change its status from NATIVE, because such object may already be deleted by the runtime.
            // In case its status is changed, we must call a Safepoint to terminate this thread.
            // For example, if a daemon thread calls ManagedCodeBegin (which changes status from NATIVE to
            // RUNNING), it may be interrupted by a GC thread, which changes status to IS_SUSPENDED.
            StoreStatus<CHECK_SAFEPOINT>(status);
        } else {
            // NB! Status is not a simple bit, without atomics it can produce faulty GetStatus.
            StoreStatus(status);
        }
    }

    enum ThreadStatus GetStatus()
    {
        // Atomic with acquire order reason: data race with flags with dependecies on reads after
        // the load which should become visible
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        uint32_t res_int = fts_.as_atomic.load(std::memory_order_acquire);
        return static_cast<enum ThreadStatus>(res_int >> THREAD_STATUS_OFFSET);
    }

    static PandaString ThreadStatusAsString(enum ThreadStatus status);

    panda::mem::StackFrameAllocator *GetStackFrameAllocator() const
    {
        return stack_frame_allocator_;
    }

    panda::mem::InternalAllocator<>::LocalSmallObjectAllocator *GetLocalInternalAllocator() const
    {
        return internal_local_allocator_;
    }

    mem::TLAB *GetTLAB() const
    {
        ASSERT(tlab_ != nullptr);
        return tlab_;
    }

    void UpdateTLAB(mem::TLAB *tlab);

    void ClearTLAB();

    void SetStringClassPtr(void *p)
    {
        string_class_ptr_ = p;
    }

#ifndef NDEBUG
    bool IsRuntimeCallEnabled() const
    {
        return runtime_call_enabled_ != 0;
    }
#endif

    static ManagedThread *Create(
        Runtime *runtime, PandaVM *vm,
        panda::panda_file::SourceLang thread_lang = panda::panda_file::SourceLang::PANDA_ASSEMBLY);
    ~ManagedThread() override;

    explicit ManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                           Thread::ThreadType thread_type,
                           panda::panda_file::SourceLang thread_lang = panda::panda_file::SourceLang::PANDA_ASSEMBLY);

    // Here methods which are just proxy or cache for runtime interface

    ALWAYS_INLINE mem::BarrierType GetPreBarrierType() const
    {
        return pre_barrier_type_;
    }

    ALWAYS_INLINE mem::BarrierType GetPostBarrierType() const
    {
        return post_barrier_type_;
    }

    // Methods to access thread local storage
    InterpreterCache *GetInterpreterCache()
    {
        return &interpreter_cache_;
    }

    uintptr_t GetNativePc() const
    {
        return native_pc_;
    }

    void SetNativePc(uintptr_t pc)
    {
        native_pc_ = pc;
    }

    // buffers may be destroyed during Detach(), so it should be initialized once more
    void InitBuffers();

    PandaVector<ObjectHeader *> *GetPreBuff() const
    {
        return pre_buff_;
    }

    PandaVector<ObjectHeader *> *MovePreBuff()
    {
        auto res = pre_buff_;
        pre_buff_ = nullptr;
        return res;
    }

    mem::GCG1BarrierSet::G1PostBarrierRingBufferType *GetG1PostBarrierBuffer()
    {
        return g1_post_barrier_ring_buffer_;
    }

    void ResetG1PostBarrierRingBuffer()
    {
        g1_post_barrier_ring_buffer_ = nullptr;
    }

    panda::panda_file::SourceLang GetThreadLang() const
    {
        return thread_lang_;
    }

    LanguageContext GetLanguageContext();

    inline bool IsSuspended()
    {
        return ReadFlag(SUSPEND_REQUEST);
    }

    inline bool IsRuntimeTerminated()
    {
        return ReadFlag(RUNTIME_TERMINATION_REQUEST);
    }

    inline void SetRuntimeTerminated()
    {
        SetFlag(RUNTIME_TERMINATION_REQUEST);
    }

    static constexpr uint32_t GetFrameKindOffset()
    {
        return MEMBER_OFFSET(ManagedThread, is_compiled_frame_);
    }
    static constexpr uint32_t GetFlagOffset()
    {
        return MEMBER_OFFSET(ManagedThread, fts_);
    }

    static constexpr uint32_t GetEntrypointsOffset()
    {
        return MEMBER_OFFSET(ManagedThread, entrypoints_);
    }
    static constexpr uint32_t GetObjectOffset()
    {
        return MEMBER_OFFSET(ManagedThread, object_);
    }
    static constexpr uint32_t GetFrameOffset()
    {
        return MEMBER_OFFSET(ManagedThread, frame_);
    }
    static constexpr uint32_t GetExceptionOffset()
    {
        return MEMBER_OFFSET(ManagedThread, exception_);
    }
    static constexpr uint32_t GetNativePcOffset()
    {
        return MEMBER_OFFSET(ManagedThread, native_pc_);
    }
    static constexpr uint32_t GetTLABOffset()
    {
        return MEMBER_OFFSET(ManagedThread, tlab_);
    }
    static constexpr uint32_t GetTlsCardTableAddrOffset()
    {
        return MEMBER_OFFSET(ManagedThread, card_table_addr_);
    }
    static constexpr uint32_t GetTlsCardTableMinAddrOffset()
    {
        return MEMBER_OFFSET(ManagedThread, card_table_min_addr_);
    }
    static constexpr uint32_t GetTlsConcurrentMarkingAddrOffset()
    {
        return MEMBER_OFFSET(ManagedThread, concurrent_marking_addr_);
    }
    static constexpr uint32_t GetTlsStringClassPointerOffset()
    {
        return MEMBER_OFFSET(ManagedThread, string_class_ptr_);
    }
    static constexpr uint32_t GetPreBuffOffset()
    {
        return MEMBER_OFFSET(ManagedThread, pre_buff_);
    }

    static constexpr uint32_t GetLanguageExtensionsDataOffset()
    {
        return MEMBER_OFFSET(ManagedThread, language_extension_data_);
    }

    static constexpr uint32_t GetRuntimeCallEnabledOffset()
    {
#ifndef NDEBUG
        return MEMBER_OFFSET(ManagedThread, runtime_call_enabled_);
#else
        // it should not be used
        return 0;
#endif
    }

    void *GetLanguageExtensionsData() const
    {
        return language_extension_data_;
    }

    void SetLanguageExtensionsData(void *data)
    {
        language_extension_data_ = data;
    }

    static constexpr uint32_t GetInternalIdOffset()
    {
        return MEMBER_OFFSET(ManagedThread, internal_id_);
    }

    virtual void VisitGCRoots(const ObjectVisitor &cb);

    virtual void UpdateGCRoots();

    void PushLocalObject(ObjectHeader **object_header);

    void PopLocalObject();

    void SetThreadPriority(int32_t prio);

    uint32_t GetThreadPriority();

    inline bool IsGcRequired()
    {
        return ReadFlag(GC_SAFEPOINT_REQUEST);
    }

    // NO_THREAD_SANITIZE for invalid TSAN data race report
    NO_THREAD_SANITIZE bool ReadFlag(ThreadFlag flag) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return (fts_.as_struct.flags & static_cast<uint16_t>(flag)) != 0;
    }

    NO_THREAD_SANITIZE bool TestAllFlags() const
    {
        return (fts_.as_struct.flags) != NO_FLAGS;  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    void SetFlag(ThreadFlag flag)
    {
        // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
        // where threads observe all modifications in the same order
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        fts_.as_atomic.fetch_or(flag, std::memory_order_seq_cst);
    }

    void ClearFlag(ThreadFlag flag)
    {
        // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
        // where threads observe all modifications in the same order
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        fts_.as_atomic.fetch_and(UINT32_MAX ^ flag, std::memory_order_seq_cst);
    }

    // Separate functions for NO_THREAD_SANITIZE to suppress TSAN data race report
    NO_THREAD_SANITIZE uint32_t ReadFlagsAndThreadStatusUnsafe()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return fts_.as_int;
    }

    bool IsManagedCodeAllowed() const
    {
        return is_managed_code_allowed_;
    }

    void SetManagedCodeAllowed(bool allowed)
    {
        is_managed_code_allowed_ = allowed;
    }

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline HandleScope<T> *PopHandleScope()
    {
        return nullptr;
    }

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline void PushHandleScope([[maybe_unused]] HandleScope<T> *handle_scope)
    {
    }

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline HandleScope<T> *GetTopScope() const
    {
        return nullptr;
    }

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline HandleStorage<T> *GetHandleStorage() const
    {
        return nullptr;
    }

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline GlobalHandleStorage<T> *GetGlobalHandleStorage() const
    {
        return nullptr;
    }

    CustomTLSData *GetCustomTLSData(const char *key);
    void SetCustomTLSData(const char *key, CustomTLSData *data);
    bool EraseCustomTLSData(const char *key);

#if EVENT_METHOD_ENTER_ENABLED || EVENT_METHOD_EXIT_ENABLED
    uint32_t RecordMethodEnter()
    {
        return call_depth_++;
    }

    uint32_t RecordMethodExit()
    {
        return --call_depth_;
    }
#endif

    bool IsAttached()
    {
        // Atomic with relaxed order reason: data race with is_attached_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        return is_attached_.load(std::memory_order_relaxed);
    }

    void SetAttached()
    {
        // Atomic with relaxed order reason: data race with is_attached_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        is_attached_.store(true, std::memory_order_relaxed);
    }

    void SetDetached()
    {
        // Atomic with relaxed order reason: data race with is_attached_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        is_attached_.store(false, std::memory_order_relaxed);
    }

    bool IsVMThread()
    {
        return is_vm_thread_;
    }

    void SetVMThread()
    {
        is_vm_thread_ = true;
    }

    bool IsThrowingOOM()
    {
        return throwing_oom_count_ > 0;
    }

    void SetThrowingOOM(bool is_throwing_oom)
    {
        if (is_throwing_oom) {
            throwing_oom_count_++;
            return;
        }
        ASSERT(throwing_oom_count_ > 0);
        throwing_oom_count_--;
    }

    bool IsUsePreAllocObj()
    {
        return use_prealloc_obj_;
    }

    void SetUsePreAllocObj(bool use_prealloc_obj)
    {
        use_prealloc_obj_ = use_prealloc_obj;
    }

    void PrintSuspensionStackIfNeeded();

    ThreadId GetId() const
    {
        // Atomic with relaxed order reason: data race with id_ with no synchronization or ordering constraints imposed
        // on other reads or writes
        return id_.load(std::memory_order_relaxed);
    }

    void FreeInternalMemory() override;
    void DestroyInternalResources();

    void InitForStackOverflowCheck(size_t native_stack_reserved_size, size_t native_stack_protected_size);

    void DisableStackOverflowCheck();

    void EnableStackOverflowCheck();

    template <bool check_native_stack = true, bool check_iframe_stack = true>
    ALWAYS_INLINE inline bool StackOverflowCheck();

    static size_t GetStackOverflowCheckOffset()
    {
        return STACK_OVERFLOW_RESERVED_SIZE;
    }

    void *const *GetDebugDispatchTable() const
    {
        return debug_dispatch_table;
    }

    void SetDebugDispatchTable(const void *const *dispatch_table)
    {
        debug_dispatch_table = const_cast<void *const *>(dispatch_table);
    }

    void *const *GetCurrentDispatchTable() const
    {
        return current_dispatch_table;
    }

    void SetCurrentDispatchTable(const void *const *dispatch_table)
    {
        current_dispatch_table = const_cast<void *const *>(dispatch_table);
    }

    void SuspendImpl(bool internal_suspend = false);
    void ResumeImpl(bool internal_resume = false);

    virtual void Suspend()
    {
        SuspendImpl();
    }

    virtual void Resume()
    {
        ResumeImpl();
    }

    /**
     * Transition to suspended and back to runnable, re-acquire share on mutator_lock_
     */
    void SuspendCheck();

    bool IsUserSuspended()
    {
        return user_code_suspend_count_ > 0;
    }

    void WaitSuspension()
    {
        constexpr int TIMEOUT = 100;
        auto old_status = GetStatus();
        PrintSuspensionStackIfNeeded();
        UpdateStatus(ThreadStatus::IS_SUSPENDED);
        {
            os::memory::LockHolder lock(suspend_lock_);
            while (suspend_count_ > 0) {
                suspend_var_.TimedWait(&suspend_lock_, TIMEOUT);
                // In case runtime is being terminated, we should abort suspension and release monitors
                if (UNLIKELY(IsRuntimeTerminated())) {
                    suspend_lock_.Unlock();
                    OnRuntimeTerminated();
                    UNREACHABLE();
                }
            }
            ASSERT(!IsSuspended());
        }
        UpdateStatus(old_status);
    }

    virtual void OnRuntimeTerminated() {}

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    void TransitionFromRunningToSuspended(enum ThreadStatus status) NO_THREAD_SAFETY_ANALYSIS
    {
        // Do Unlock after StoreStatus, because the thread requesting a suspension should see an updated status
        StoreStatus(status);
        Locks::mutator_lock->Unlock();
    }

    void SafepointPoll();

    /**
     * From NativeCode you can call ManagedCodeBegin.
     * From ManagedCode you can call NativeCodeBegin.
     * Call the same type is forbidden.
     */
    virtual void NativeCodeBegin();
    virtual void NativeCodeEnd();
    [[nodiscard]] virtual bool IsInNativeCode() const;

    virtual void ManagedCodeBegin();
    virtual void ManagedCodeEnd();
    [[nodiscard]] virtual bool IsManagedCode() const;

    static bool IsManagedScope()
    {
        auto thread = GetCurrent();
        return thread != nullptr && thread->is_managed_scope_;
    }

    [[nodiscard]] bool HasManagedCodeOnStack() const;
    [[nodiscard]] bool HasClearStack() const;

protected:
    void ProtectNativeStack();

    template <bool check_native_stack = true, bool check_iframe_stack = true>
    ALWAYS_INLINE inline bool StackOverflowCheckResult() const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (check_native_stack) {
            if (UNLIKELY(__builtin_frame_address(0) < ToVoidPtr(native_stack_end_))) {
                return false;
            }
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (check_iframe_stack) {
            if (UNLIKELY(GetStackFrameAllocator()->GetAllocatedSize() > iframe_stack_size_)) {
                return false;
            }
        }
        return true;
    }

    static const int WAIT_INTERVAL = 10;

    template <typename T = void>
    T *GetAssociatedObject()
    {
        return reinterpret_cast<T *>(object_);
    }

    template <typename T>
    void SetAssociatedObject(T *object)
    {
        object_ = object;
    }

    virtual void InterruptPostImpl() {}

    void UpdateId(ThreadId id)
    {
        // Atomic with relaxed order reason: data race with id_ with no synchronization or ordering constraints imposed
        // on other reads or writes
        id_.store(id, std::memory_order_relaxed);
    }

    bool GetOnThreadTerminationCalled() const
    {
        return on_thread_terminated_called;
    }

    void SetOnThreadTerminationCalled()
    {
        on_thread_terminated_called = true;
    }

private:
    enum SafepointFlag : bool { DONT_CHECK_SAFEPOINT = false, CHECK_SAFEPOINT = true };
    enum ReadlockFlag : bool { NO_READLOCK = false, READLOCK = true };

    PandaString LogThreadStack(ThreadState new_state) const;

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    template <SafepointFlag safepoint = DONT_CHECK_SAFEPOINT, ReadlockFlag readlock = NO_READLOCK>
    void StoreStatus(ThreadStatus status) NO_THREAD_SAFETY_ANALYSIS
    {
        while (true) {
            union FlagsAndThreadStatus old_fts {
            };
            union FlagsAndThreadStatus new_fts {
            };
            old_fts.as_int = ReadFlagsAndThreadStatusUnsafe();  // NOLINT(cppcoreguidelines-pro-type-union-access)

            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (safepoint == CHECK_SAFEPOINT) {   // NOLINT(bugprone-suspicious-semicolon)
                if (old_fts.as_struct.flags != NO_FLAGS) {  // NOLINT(cppcoreguidelines-pro-type-union-access)
                    // someone requires a safepoint
                    SafepointPoll();
                    continue;
                }
            }

            new_fts.as_struct.flags = old_fts.as_struct.flags;  // NOLINT(cppcoreguidelines-pro-type-union-access)
            new_fts.as_struct.status = status;                  // NOLINT(cppcoreguidelines-pro-type-union-access)

            // mutator lock should be acquired before change status
            // to avoid blocking in running state
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (readlock == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                Locks::mutator_lock->ReadLock();
            }

            // clang-format conflicts with CodeCheckAgent, so disable it here
            // clang-format off
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            if (fts_.as_atomic.compare_exchange_weak(
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                old_fts.as_nonvolatile_int, new_fts.as_nonvolatile_int, std::memory_order_release)) {
                // If CAS succeeded, we set new status and no request occurred here, safe to proceed.
                break;
            }
            // Release mutator lock to acquire it on the next loop iteration
            // clang-format on
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (readlock == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                Locks::mutator_lock->Unlock();
            }
        }
    }

    static constexpr uint32_t THREAD_STATUS_OFFSET = 16;
    static_assert(sizeof(fts_) == sizeof(uint32_t), "Wrong fts_ size");

    // Can cause data races if child thread's UpdateId is executed concurrently with GetNativeThreadId
    std::atomic<ThreadId> id_;

    static mem::TLAB *zero_tlab;
    PandaVector<ObjectHeader **> local_objects_;

    // Something like custom TLS - it is faster to access via ManagedThread than via thread_local
    InterpreterCache interpreter_cache_;

    PandaMap<const char *, PandaUniquePtr<CustomTLSData>> custom_tls_cache_ GUARDED_BY(Locks::custom_tls_lock);

    // Keep these here to speed up interpreter
    mem::BarrierType pre_barrier_type_ {mem::BarrierType::PRE_WRB_NONE};
    mem::BarrierType post_barrier_type_ {mem::BarrierType::POST_WRB_NONE};
    // Thread local storages to avoid locks in heap manager
    mem::StackFrameAllocator *stack_frame_allocator_;
    mem::InternalAllocator<>::LocalSmallObjectAllocator *internal_local_allocator_;
    std::atomic_bool is_attached_ {false};  // Can be changed after thread is registered and can cause data race
    bool is_vm_thread_ = false;

    bool is_managed_code_allowed_ {true};

    size_t throwing_oom_count_ {0};
    bool use_prealloc_obj_ {false};

    panda::panda_file::SourceLang thread_lang_ = panda::panda_file::SourceLang::PANDA_ASSEMBLY;

    PandaUniquePtr<tooling::PtThreadInfo> pt_thread_info_;

    // for stack overflow check
    // |.....     Method 1    ....|
    // |.....     Method 2    ....|
    // |.....     Method 3    ....|_ _ _ native_stack_top
    // |..........................|
    // |..........................|
    // |..........................|
    // |..........................|
    // |..........................|
    // |..........................|
    // |..........................|_ _ _ native_stack_end
    // |..... Reserved region ....|
    // |.... Protected region ....|_ _ _ native_stack_begin
    // |...... Guard region ......|
    uintptr_t native_stack_begin_ {0};
    // end of stack for managed thread, throw exception if native stack grow over it
    uintptr_t native_stack_end_ {0};
    // os thread stack size
    size_t native_stack_size_ {0};
    // guard region size of stack
    size_t native_stack_guard_size_ {0};
    // reserved region is for throw exception handle if stack overflow happen
    size_t native_stack_reserved_size_ {0};
    // protected region is for compiled code to test load [sp - native_stack_reserved_size_] to trigger segv
    size_t native_stack_protected_size_ {0};
    // max allowed size for interpreter frame
    size_t iframe_stack_size_ {std::numeric_limits<size_t>::max()};

    PandaVector<HandleScope<coretypes::TaggedType> *> tagged_handle_scopes_ {};
    HandleStorage<coretypes::TaggedType> *tagged_handle_storage_ {nullptr};
    GlobalHandleStorage<coretypes::TaggedType> *tagged_global_handle_storage_ {nullptr};

    PandaVector<HandleScope<ObjectHeader *> *> object_header_handle_scopes_ {};
    HandleStorage<ObjectHeader *> *object_header_handle_storage_ {nullptr};

    os::memory::ConditionVariable suspend_var_ GUARDED_BY(suspend_lock_);
    os::memory::Mutex suspend_lock_;
    uint32_t suspend_count_ GUARDED_BY(suspend_lock_) = 0;
    std::atomic_uint32_t user_code_suspend_count_ {0};

    PandaStack<ThreadState> thread_frame_states_;

    // Boolean which is safe to access after runtime is destroyed
    bool is_managed_scope_ {false};

    // TODO(Mordan Vitalii #6852): remove this flag when FreeInternalSpace will not be called after Detach for
    // daemon thread
    bool on_thread_terminated_called {false};

    friend class panda::test::ThreadTest;
    friend class panda::ThreadManager;

    // Used in mathod events
    uint32_t call_depth_ {0};

    void *const *debug_dispatch_table {nullptr};

    void *const *current_dispatch_table {nullptr};

    NO_COPY_SEMANTIC(ManagedThread);
    NO_MOVE_SEMANTIC(ManagedThread);
};
}  // namespace panda

#endif  // PANDA_RUNTIME_MANAGED_THREAD_H
