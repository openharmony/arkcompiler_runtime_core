/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define ASSERT_MANAGED_CODE() ASSERT(::ark::ManagedThread::GetCurrent()->IsManagedCode())
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_NATIVE_CODE() ASSERT(::ark::ManagedThread::GetCurrent()->IsInNativeCode())

namespace ark {
class MTThreadManager;
/**
 * @brief Class represents managed thread
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

    using NativeHandleType = os::thread::NativeHandleType;
    static constexpr ThreadId NON_INITIALIZED_THREAD_ID = 0;
    static constexpr ThreadId MAX_INTERNAL_THREAD_ID = MarkWord::LIGHT_LOCK_THREADID_MAX_COUNT;
    static constexpr size_t STACK_MAX_SIZE_OVERFLOW_CHECK = 256_MB;
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON) || !defined(NDEBUG)
    static constexpr size_t STACK_OVERFLOW_RESERVED_SIZE = 64_KB;
#else
    static constexpr size_t STACK_OVERFLOW_RESERVED_SIZE = 12_KB;
#endif
    static constexpr size_t STACK_OVERFLOW_PROTECTED_SIZE = 4_KB;

    void SetLanguageContext([[maybe_unused]] const LanguageContext &ctx)
    {
        // Deprecated method, don't use it. Only for copability with js_runtime.
    }

    void SetCurrentFrame(Frame *f)
    {
        frame_ = f;
    }

    tooling::PtThreadInfo *GetPtThreadInfo() const
    {
        return ptThreadInfo_.get();
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
        return isCompiledFrame_;
    }

    void SetCurrentFrameIsCompiled(bool value)
    {
        isCompiledFrame_ = value;
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

    size_t GetIFrameStackSize() const
    {
        return iframeStackSize_;
    }

    static bool ThreadIsManagedThread(const Thread *thread)
    {
        ASSERT(thread != nullptr);
        Thread::ThreadType threadType = thread->GetThreadType();
        return threadType == Thread::ThreadType::THREAD_TYPE_MANAGED ||
               threadType == Thread::ThreadType::THREAD_TYPE_MT_MANAGED ||
               threadType == Thread::ThreadType::THREAD_TYPE_TASK;
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
    PANDA_PUBLIC_API static ManagedThread *GetCurrent()
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

        ThreadStatus oldStatus = GetStatus();
        if (oldStatus == ThreadStatus::RUNNING && status != ThreadStatus::RUNNING) {
            TransitionFromRunningToSuspended(status);
        } else if (oldStatus != ThreadStatus::RUNNING && status == ThreadStatus::RUNNING) {
            // NB! This thread is treated as suspended so when we transition from suspended state to
            // running we need to check suspension flag and counter so SafepointPoll has to be done before
            // acquiring mutator_lock.
            // StoreStatus acquires lock here
            StoreStatus<CHECK_SAFEPOINT, READLOCK>(ThreadStatus::RUNNING);
        } else if (oldStatus == ThreadStatus::NATIVE && status != ThreadStatus::IS_TERMINATED_LOOP &&
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
        uint32_t resInt = fts_.asAtomic.load(std::memory_order_acquire);
        return static_cast<enum ThreadStatus>(resInt >> THREAD_STATUS_OFFSET);
    }

    static PandaString ThreadStatusAsString(enum ThreadStatus status);

    ark::mem::StackFrameAllocator *GetStackFrameAllocator() const
    {
        return stackFrameAllocator_;
    }

    ark::mem::InternalAllocator<>::LocalSmallObjectAllocator *GetLocalInternalAllocator() const
    {
        return internalLocalAllocator_;
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
        stringClassPtr_ = p;
    }

    void SetArrayU8ClassPtr(void *p)
    {
        arrayU8ClassPtr_ = p;
    }

    void SetArrayU16ClassPtr(void *p)
    {
        arrayU16ClassPtr_ = p;
    }

#ifndef NDEBUG
    bool IsRuntimeCallEnabled() const
    {
        return runtimeCallEnabled_ != 0;
    }
#endif

    static ManagedThread *Create(Runtime *runtime, PandaVM *vm,
                                 ark::panda_file::SourceLang threadLang = ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    ~ManagedThread() override;

    explicit ManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, Thread::ThreadType threadType,
                           ark::panda_file::SourceLang threadLang = ark::panda_file::SourceLang::PANDA_ASSEMBLY);

    // Here methods which are just proxy or cache for runtime interface

    ALWAYS_INLINE mem::BarrierType GetPreBarrierType() const
    {
        return preBarrierType_;
    }

    ALWAYS_INLINE mem::BarrierType GetPostBarrierType() const
    {
        return postBarrierType_;
    }

    // Methods to access thread local storage
    InterpreterCache *GetInterpreterCache()
    {
        return &interpreterCache_;
    }

    uintptr_t GetNativePc() const
    {
        return nativePc_;
    }

    void SetNativePc(uintptr_t pc)
    {
        nativePc_ = pc;
    }

    // buffers may be destroyed during Detach(), so it should be initialized once more
    void InitBuffers();

    PandaVector<ObjectHeader *> *GetPreBuff() const
    {
        return preBuff_;
    }

    PandaVector<ObjectHeader *> *MovePreBuff()
    {
        auto res = preBuff_;
        preBuff_ = nullptr;
        return res;
    }

    mem::GCG1BarrierSet::G1PostBarrierRingBufferType *GetG1PostBarrierBuffer()
    {
        return g1PostBarrierRingBuffer_;
    }

    void ResetG1PostBarrierBuffer()
    {
        g1PostBarrierRingBuffer_ = nullptr;
    }

    static constexpr uint32_t GetG1PostBarrierBufferOffset()
    {
        return MEMBER_OFFSET(ManagedThread, g1PostBarrierRingBuffer_);
    }

    ark::panda_file::SourceLang GetThreadLang() const
    {
        return threadLang_;
    }

    WeightedAdaptiveTlabAverage *GetWeightedTlabAverage() const
    {
        return weightedAdaptiveTlabAverage_;
    }

    PANDA_PUBLIC_API LanguageContext GetLanguageContext();

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
        return MEMBER_OFFSET(ManagedThread, isCompiledFrame_);
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
        return MEMBER_OFFSET(ManagedThread, nativePc_);
    }
    static constexpr uint32_t GetTLABOffset()
    {
        return MEMBER_OFFSET(ManagedThread, tlab_);
    }
    static constexpr uint32_t GetTlsCardTableAddrOffset()
    {
        return MEMBER_OFFSET(ManagedThread, cardTableAddr_);
    }
    static constexpr uint32_t GetTlsCardTableMinAddrOffset()
    {
        return MEMBER_OFFSET(ManagedThread, cardTableMinAddr_);
    }
    static constexpr uint32_t GetTlsPostWrbOneObjectOffset()
    {
        return MEMBER_OFFSET(ManagedThread, postWrbOneObject_);
    }
    static constexpr uint32_t GetTlsPostWrbTwoObjectsOffset()
    {
        return MEMBER_OFFSET(ManagedThread, postWrbTwoObjects_);
    }
    static constexpr uint32_t GetTlsPreWrbEntrypointOffset()
    {
        return MEMBER_OFFSET(ManagedThread, preWrbEntrypoint_);
    }
    static constexpr uint32_t GetTlsStringClassPointerOffset()
    {
        return MEMBER_OFFSET(ManagedThread, stringClassPtr_);
    }
    static constexpr uint32_t GetTlsArrayU8ClassPointerOffset()
    {
        return MEMBER_OFFSET(ManagedThread, arrayU8ClassPtr_);
    }
    static constexpr uint32_t GetTlsArrayU16ClassPointerOffset()
    {
        return MEMBER_OFFSET(ManagedThread, arrayU16ClassPtr_);
    }
    static constexpr uint32_t GetPreBuffOffset()
    {
        return MEMBER_OFFSET(ManagedThread, preBuff_);
    }

    static constexpr uint32_t GetLanguageExtensionsDataOffset()
    {
        return MEMBER_OFFSET(ManagedThread, languageExtensionData_);
    }

    static constexpr uint32_t GetRuntimeCallEnabledOffset()
    {
#ifndef NDEBUG
        return MEMBER_OFFSET(ManagedThread, runtimeCallEnabled_);
#else
        // it should not be used
        return 0;
#endif
    }

    static constexpr uint32_t GetInterpreterCacheOffset()
    {
        return MEMBER_OFFSET(ManagedThread, interpreterCache_);
    }

    void *GetLanguageExtensionsData() const
    {
        return languageExtensionData_;
    }

    void SetLanguageExtensionsData(void *data)
    {
        languageExtensionData_ = data;
    }

    static constexpr uint32_t GetInternalIdOffset()
    {
        return MEMBER_OFFSET(ManagedThread, internalId_);
    }

    virtual void VisitGCRoots(const ObjectVisitor &cb);

    virtual void UpdateGCRoots();

    PANDA_PUBLIC_API void PushLocalObject(ObjectHeader **objectHeader);

    PANDA_PUBLIC_API void PopLocalObject();

    void SetThreadPriority(int32_t prio);

    uint32_t GetThreadPriority();

    // NO_THREAD_SANITIZE for invalid TSAN data race report
    NO_THREAD_SANITIZE bool ReadFlag(ThreadFlag flag) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return (fts_.asStruct.flags & static_cast<uint16_t>(flag)) != 0;
    }

    NO_THREAD_SANITIZE bool TestAllFlags() const
    {
        return (fts_.asStruct.flags) != initialThreadFlag_;  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    void SetFlag(ThreadFlag flag)
    {
        // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
        // where threads observe all modifications in the same order
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        fts_.asAtomic.fetch_or(flag, std::memory_order_seq_cst);
    }

    void ClearFlag(ThreadFlag flag)
    {
        // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
        // where threads observe all modifications in the same order
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        fts_.asAtomic.fetch_and(UINT32_MAX ^ flag, std::memory_order_seq_cst);
    }

    // Separate functions for NO_THREAD_SANITIZE to suppress TSAN data race report
    NO_THREAD_SANITIZE uint32_t ReadFlagsAndThreadStatusUnsafe()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return fts_.asInt;
    }

    bool IsManagedCodeAllowed() const
    {
        return isManagedCodeAllowed_;
    }

    void SetManagedCodeAllowed(bool allowed)
    {
        isManagedCodeAllowed_ = allowed;
    }

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline HandleScope<T> *PopHandleScope();

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline void PushHandleScope([[maybe_unused]] HandleScope<T> *handleScope);

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline HandleScope<T> *GetTopScope() const;

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline HandleStorage<T> *GetHandleStorage() const;

    // TaggedType has been specialized for js, Other types are empty implementation
    template <typename T>
    inline GlobalHandleStorage<T> *GetGlobalHandleStorage() const;

    PANDA_PUBLIC_API CustomTLSData *GetCustomTLSData(const char *key);
    PANDA_PUBLIC_API void SetCustomTLSData(const char *key, CustomTLSData *data);
    PANDA_PUBLIC_API bool EraseCustomTLSData(const char *key);

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
        return isAttached_.load(std::memory_order_relaxed);
    }

    void SetAttached()
    {
        // Atomic with relaxed order reason: data race with is_attached_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        isAttached_.store(true, std::memory_order_relaxed);
    }

    void SetDetached()
    {
        // Atomic with relaxed order reason: data race with is_attached_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        isAttached_.store(false, std::memory_order_relaxed);
    }

    bool IsVMThread()
    {
        return isVmThread_;
    }

    void SetVMThread()
    {
        isVmThread_ = true;
    }

    bool IsThrowingOOM()
    {
        return throwingOomCount_ > 0;
    }

    void SetThrowingOOM(bool isThrowingOom)
    {
        if (isThrowingOom) {
            throwingOomCount_++;
            return;
        }
        ASSERT(throwingOomCount_ > 0);
        throwingOomCount_--;
    }

    bool IsUsePreAllocObj()
    {
        return usePreallocObj_;
    }

    void SetUsePreAllocObj(bool usePreallocObj)
    {
        usePreallocObj_ = usePreallocObj;
    }

    PANDA_PUBLIC_API void PrintSuspensionStackIfNeeded();

    ThreadId GetId() const
    {
        // Atomic with relaxed order reason: data race with id_ with no synchronization or ordering constraints imposed
        // on other reads or writes
        return id_.load(std::memory_order_relaxed);
    }

    void FreeInternalMemory() override;
    void DestroyInternalResources();

    /// Clears the pre/post barrier buffers (and other resources) without deallocation.
    void CleanupInternalResources();

    /// Collect TLAB metrics for memstats
    void CollectTLABMetrics();

    void InitForStackOverflowCheck(size_t nativeStackReservedSize, size_t nativeStackProtectedSize);
    virtual void DisableStackOverflowCheck();
    virtual void EnableStackOverflowCheck();
    /// Obtains current thread's native stack parameters and returns true on success
    virtual bool RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize);

    template <bool CHECK_NATIVE_STACK = true, bool CHECK_IFRAME_STACK = true>
    ALWAYS_INLINE inline bool StackOverflowCheck();

    static size_t GetStackOverflowCheckOffset()
    {
        return STACK_OVERFLOW_RESERVED_SIZE;
    }

    void *const *GetDebugDispatchTable() const
    {
#ifdef PANDA_WITH_QUICKENER
        return const_cast<void *const *>(GetOrSetInnerDebugDispatchTable());
#else
        return debugDispatchTable_;
#endif
    }

    void SetDebugDispatchTable(const void *const *dispatchTable)
    {
#ifdef PANDA_WITH_QUICKENER
        GetOrSetInnerDebugDispatchTable(true, dispatch_table);
#else
        debugDispatchTable_ = const_cast<void *const *>(dispatchTable);
#endif
    }

    template <bool IS_DEBUG>
    void *const *GetCurrentDispatchTable() const
    {
#ifdef PANDA_WITH_QUICKENER
        return const_cast<void *const *>(GetOrSetInnerDispatchTable<is_debug>());
#else
        if constexpr (IS_DEBUG) {
            return debugStubDispatchTable_;
        } else {
            return dispatchTable_;
        }
#endif
    }

    template <bool IS_DEBUG>
    void SetCurrentDispatchTable(const void *const *dispatchTable)
    {
#ifdef PANDA_WITH_QUICKENER
        GetOrSetInnerDispatchTable<is_debug>(true, dispatch_table);
#else
        if constexpr (IS_DEBUG) {
            debugStubDispatchTable_ = const_cast<void *const *>(dispatchTable);
        } else {
            dispatchTable_ = const_cast<void *const *>(dispatchTable);
        }
#endif
    }

    PANDA_PUBLIC_API void SuspendImpl(bool internalSuspend = false);
    PANDA_PUBLIC_API void ResumeImpl(bool internalResume = false);

    virtual void Suspend()
    {
        SuspendImpl();
    }

    virtual void Resume()
    {
        ResumeImpl();
    }

    /// Transition to suspended and back to runnable, re-acquire share on mutator_lock_
    PANDA_PUBLIC_API void SuspendCheck();

    bool IsUserSuspended()
    {
        return userCodeSuspendCount_ > 0;
    }

    /* @sync 1
     * @description This synchronization point can be used to insert a new attribute or method
     * into ManagedThread class.
     */

    void WaitSuspension()
    {
        constexpr int TIMEOUT = 100;
        auto oldStatus = GetStatus();
        PrintSuspensionStackIfNeeded();
        UpdateStatus(ThreadStatus::IS_SUSPENDED);
        {
            /* @sync 1
             * @description Right after the thread updates its status to IS_SUSPENDED and right before beginning to wait
             * for actual suspension
             */
            os::memory::LockHolder lock(suspendLock_);
            while (suspendCount_ > 0) {
                suspendVar_.TimedWait(&suspendLock_, TIMEOUT);
                // In case runtime is being terminated, we should abort suspension and release monitors
                if (UNLIKELY(IsRuntimeTerminated())) {
                    suspendLock_.Unlock();
                    OnRuntimeTerminated();
                    UNREACHABLE();
                }
            }
            ASSERT(!IsSuspended());
        }
        UpdateStatus(oldStatus);
    }

    virtual void OnRuntimeTerminated() {}

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    void TransitionFromRunningToSuspended(enum ThreadStatus status) NO_THREAD_SAFETY_ANALYSIS
    {
        // Do Unlock after StoreStatus, because the thread requesting a suspension should see an updated status
        StoreStatus(status);
        GetMutatorLock()->Unlock();
    }

    PANDA_PUBLIC_API void SafepointPoll();

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
        return thread != nullptr && thread->isManagedScope_;
    }

    [[nodiscard]] bool HasManagedCodeOnStack() const;
    [[nodiscard]] bool HasClearStack() const;

protected:
    void ProtectNativeStack();

    template <bool CHECK_NATIVE_STACK = true, bool CHECK_IFRAME_STACK = true>
    ALWAYS_INLINE inline bool StackOverflowCheckResult() const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (CHECK_NATIVE_STACK) {
            if (UNLIKELY(__builtin_frame_address(0) < ToVoidPtr(nativeStackEnd_))) {
                return false;
            }
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (CHECK_IFRAME_STACK) {
            if (UNLIKELY(GetStackFrameAllocator()->GetAllocatedSize() > iframeStackSize_)) {
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

    /**
     * Prepares the ManagedThread instance for caching and further reuse by resetting its member variables to their
     * default values.
     */
    virtual void CleanUp();

private:
    enum SafepointFlag : bool { DONT_CHECK_SAFEPOINT = false, CHECK_SAFEPOINT = true };
    enum ReadlockFlag : bool { NO_READLOCK = false, READLOCK = true };

    PandaString LogThreadStack(ThreadState newState) const;

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    template <SafepointFlag SAFEPOINT = DONT_CHECK_SAFEPOINT, ReadlockFlag READLOCK_FLAG = NO_READLOCK>
    void StoreStatus(ThreadStatus status) NO_THREAD_SAFETY_ANALYSIS
    {
        while (true) {
            union FlagsAndThreadStatus oldFts {
            };
            union FlagsAndThreadStatus newFts {
            };
            oldFts.asInt = ReadFlagsAndThreadStatusUnsafe();  // NOLINT(cppcoreguidelines-pro-type-union-access)

            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (SAFEPOINT == CHECK_SAFEPOINT) {  // NOLINT(bugprone-suspicious-semicolon)
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                if (oldFts.asStruct.flags != initialThreadFlag_) {
                    // someone requires a safepoint
                    SafepointPoll();
                    continue;
                }
            }

            newFts.asStruct.flags = oldFts.asStruct.flags;  // NOLINT(cppcoreguidelines-pro-type-union-access)
            newFts.asStruct.status = status;                // NOLINT(cppcoreguidelines-pro-type-union-access)

            // mutator lock should be acquired before change status
            // to avoid blocking in running state
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (READLOCK_FLAG == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                GetMutatorLock()->ReadLock();
            }

            // clang-format conflicts with CodeCheckAgent, so disable it here
            // clang-format off
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            if (fts_.asAtomic.compare_exchange_weak(
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                oldFts.asNonvolatileInt, newFts.asNonvolatileInt, std::memory_order_release)) {
                // If CAS succeeded, we set new status and no request occurred here, safe to proceed.
                break;
            }
            // Release mutator lock to acquire it on the next loop iteration
            // clang-format on
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (READLOCK_FLAG == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                GetMutatorLock()->Unlock();
            }
        }
    }

#ifdef PANDA_WITH_QUICKENER
    NO_OPTIMIZE const void *const *GetOrSetInnerDebugDispatchTable(bool set = false,
                                                                   const void *const *dispatch_table = nullptr) const
    {
        thread_local static const void *const *current_debug_dispatch_table = nullptr;
        if (set) {
            current_debug_dispatch_table = dispatch_table;
        }
        return current_debug_dispatch_table;
    }

    template <bool IS_DEBUG>
    NO_OPTIMIZE const void *const *GetOrSetInnerDispatchTable(bool set = false,
                                                              const void *const *dispatch_table = nullptr) const
    {
        thread_local static const void *const *current_dispatch_table = nullptr;
        if (set) {
            current_dispatch_table = dispatch_table;
        }
        return current_dispatch_table;
    }
#endif

    virtual bool TestLockState() const;

    static constexpr uint32_t THREAD_STATUS_OFFSET = 16;
    static_assert(sizeof(fts_) == sizeof(uint32_t), "Wrong fts_ size");

    // Can cause data races if child thread's UpdateId is executed concurrently with GetNativeThreadId
    std::atomic<ThreadId> id_;

    static mem::TLAB *zeroTlab_;
    PandaVector<ObjectHeader **> localObjects_;
    WeightedAdaptiveTlabAverage *weightedAdaptiveTlabAverage_ {nullptr};

    // Something like custom TLS - it is faster to access via ManagedThread than via thread_local
    InterpreterCache interpreterCache_;

    PandaMap<const char *, PandaUniquePtr<CustomTLSData>> customTlsCache_ GUARDED_BY(Locks::customTlsLock_);

    mem::GCG1BarrierSet::G1PostBarrierRingBufferType *g1PostBarrierRingBuffer_ {nullptr};
    // Keep these here to speed up interpreter
    mem::BarrierType preBarrierType_ {mem::BarrierType::PRE_WRB_NONE};
    mem::BarrierType postBarrierType_ {mem::BarrierType::POST_WRB_NONE};
    // Thread local storages to avoid locks in heap manager
    mem::StackFrameAllocator *stackFrameAllocator_;
    mem::InternalAllocator<>::LocalSmallObjectAllocator *internalLocalAllocator_;
    std::atomic_bool isAttached_ {false};  // Can be changed after thread is registered and can cause data race
    bool isVmThread_ = false;

    bool isManagedCodeAllowed_ {true};

    size_t throwingOomCount_ {0};
    bool usePreallocObj_ {false};

    ark::panda_file::SourceLang threadLang_ = ark::panda_file::SourceLang::PANDA_ASSEMBLY;

    PandaUniquePtr<tooling::PtThreadInfo> ptThreadInfo_;

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
    uintptr_t nativeStackBegin_ {0};
    // end of stack for managed thread, throw exception if native stack grow over it
    uintptr_t nativeStackEnd_ {0};
    // os thread stack size
    size_t nativeStackSize_ {0};
    // guard region size of stack
    size_t nativeStackGuardSize_ {0};
    // reserved region is for throw exception handle if stack overflow happen
    size_t nativeStackReservedSize_ {0};
    // protected region is for compiled code to test load [sp - native_stack_reserved_size_] to trigger segv
    size_t nativeStackProtectedSize_ {0};
    // max allowed size for interpreter frame
    size_t iframeStackSize_ {std::numeric_limits<size_t>::max()};

    PandaVector<HandleScope<coretypes::TaggedType> *> taggedHandleScopes_ {};
    HandleStorage<coretypes::TaggedType> *taggedHandleStorage_ {nullptr};
    GlobalHandleStorage<coretypes::TaggedType> *taggedGlobalHandleStorage_ {nullptr};

    PandaVector<HandleScope<ObjectHeader *> *> objectHeaderHandleScopes_ {};
    HandleStorage<ObjectHeader *> *objectHeaderHandleStorage_ {nullptr};

    os::memory::ConditionVariable suspendVar_ GUARDED_BY(suspendLock_);
    os::memory::Mutex suspendLock_;
    uint32_t suspendCount_ GUARDED_BY(suspendLock_) = 0;
    std::atomic_uint32_t userCodeSuspendCount_ {0};

    PandaStack<ThreadState> threadFrameStates_;

    // Boolean which is safe to access after runtime is destroyed
    bool isManagedScope_ {false};

    friend class ark::test::ThreadTest;
    friend class ark::MTThreadManager;

    // Used in mathod events
    uint32_t callDepth_ {0};
#ifndef PANDA_WITH_QUICKENER
    void *const *debugDispatchTable_ {nullptr};
    void *const *debugStubDispatchTable_ {nullptr};
    void *const *dispatchTable_ {nullptr};
#endif

    NO_COPY_SEMANTIC(ManagedThread);
    NO_MOVE_SEMANTIC(ManagedThread);
};
}  // namespace ark

#endif  // PANDA_RUNTIME_MANAGED_THREAD_H
