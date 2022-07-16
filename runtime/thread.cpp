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

#include "runtime/include/thread-inl.h"
#include "libpandabase/os/stacktrace.h"
#include "runtime/handle_base-inl.h"
#include "runtime/include/locks.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/object_helpers.h"
#include "tooling/pt_thread_info.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/runslots_allocator-inl.h"

namespace panda {
using TaggedValue = coretypes::TaggedValue;
using TaggedType = coretypes::TaggedType;

mem::TLAB *ManagedThread::zero_tlab = nullptr;
static const int MIN_PRIORITY = os::thread::LOWEST_PRIORITY;

static mem::InternalAllocatorPtr GetInternalAllocator(Thread *thread)
{
    // WORKAROUND(v.cherkashin): EcmaScript side build doesn't have HeapManager, so we get internal allocator from
    // runtime
    mem::HeapManager *heap_manager = thread->GetVM()->GetHeapManager();
    if (heap_manager != nullptr) {
        return heap_manager->GetInternalAllocator();
    }
    return Runtime::GetCurrent()->GetInternalAllocator();
}

MTManagedThread::ThreadId MTManagedThread::GetInternalId()
{
    ASSERT(internal_id_ != 0);
    return internal_id_;
}

Thread::~Thread()
{
    FreeAllocatedMemory();
}

void Thread::FreeInternalMemory()
{
    FreeAllocatedMemory();
}

void Thread::FreeAllocatedMemory()
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    ASSERT(allocator != nullptr);
    allocator->Delete(pre_buff_);
    allocator->Delete(g1_post_barrier_ring_buffer_);
    pre_buff_ = nullptr;
    g1_post_barrier_ring_buffer_ = nullptr;
}

Thread::Thread(PandaVM *vm, ThreadType thread_type) : vm_(vm), thread_type_(thread_type)
{
    // WORKAROUND(v.cherkashin): EcmaScript side build doesn't have GC, so we skip setting barriers for this case
    mem::GC *gc = vm->GetGC();
    if (gc != nullptr) {
        barrier_set_ = vm->GetGC()->GetBarrierSet();
        InitCardTableData(barrier_set_);
        if (barrier_set_->GetPreType() != panda::mem::BarrierType::PRE_WRB_NONE) {
            auto addr = barrier_set_->GetBarrierOperand(panda::mem::BarrierPosition::BARRIER_POSITION_PRE,
                                                        "CONCURRENT_MARKING_ADDR");
            concurrent_marking_addr_ = std::get<std::atomic<bool> *>(addr.GetValue());
        }
    }
}

void Thread::InitCardTableData(mem::GCBarrierSet *barrier)
{
    auto post_barrier_type = barrier->GetPostType();
    switch (post_barrier_type) {
        case panda::mem::BarrierType::POST_INTERGENERATIONAL_BARRIER:
            card_table_min_addr_ = std::get<void *>(barrier->GetPostBarrierOperand("MIN_ADDR").GetValue());
            card_table_addr_ = std::get<uint8_t *>(barrier->GetPostBarrierOperand("CARD_TABLE_ADDR").GetValue());
            break;
        case panda::mem::BarrierType::POST_INTERREGION_BARRIER:
            card_table_addr_ = std::get<uint8_t *>(barrier->GetPostBarrierOperand("CARD_TABLE_ADDR").GetValue());
            card_table_min_addr_ = std::get<void *>(barrier->GetPostBarrierOperand("MIN_ADDR").GetValue());
            // TODO(dtrubenkov): add REGION_SIZE_BITS
            break;
        case panda::mem::BarrierType::POST_WRB_NONE:
        case mem::POST_RB_NONE:
            break;
        case mem::PRE_WRB_NONE:
        case mem::PRE_RB_NONE:
        case mem::PRE_SATB_BARRIER:
            LOG(FATAL, RUNTIME) << "Post barrier expected";
            break;
    }
}

void Thread::InitPreBuff()
{
    auto allocator = GetInternalAllocator(this);
    mem::GC *gc = GetVM()->GetGC();
    auto barrier = gc->GetBarrierSet();
    if (barrier->GetPreType() != panda::mem::BarrierType::PRE_WRB_NONE) {
        pre_buff_ = allocator->New<PandaVector<ObjectHeader *>>();
    }
}

/* static */
void ManagedThread::Initialize()
{
    ASSERT(!Thread::GetCurrent());
    ASSERT(!zero_tlab);
    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    zero_tlab = allocator->New<mem::TLAB>(nullptr, 0U);
}

/* static */
void ManagedThread::Shutdown()
{
    ASSERT(zero_tlab);
    ManagedThread::SetCurrent(nullptr);
    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(zero_tlab);
    zero_tlab = nullptr;
}

/* static */
void MTManagedThread::Yield()
{
    LOG(DEBUG, RUNTIME) << "Reschedule the execution of a current thread";
    os::thread::ThreadYield();
}

/* static - creation of the initial Managed thread */
ManagedThread *ManagedThread::Create(Runtime *runtime, PandaVM *vm, panda::panda_file::SourceLang thread_lang)
{
    trace::ScopedTrace scoped_trace("ManagedThread::Create");
    mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
    // Create thread structure using new, we rely on this structure to be accessible in child threads after
    // runtime is destroyed
    return new ManagedThread(os::thread::GetCurrentThreadId(), allocator, vm, Thread::ThreadType::THREAD_TYPE_MANAGED,
                             thread_lang);
}

/* static - creation of the initial MT Managed thread */
MTManagedThread *MTManagedThread::Create(Runtime *runtime, PandaVM *vm, panda::panda_file::SourceLang thread_lang)
{
    trace::ScopedTrace scoped_trace("MTManagedThread::Create");
    mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
    // Create thread structure using new, we rely on this structure to be accessible in child threads after
    // runtime is destroyed
    auto thread = new MTManagedThread(os::thread::GetCurrentThreadId(), allocator, vm, thread_lang);
    thread->ProcessCreatedThread();
    return thread;
}

ManagedThread::ManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *panda_vm,
                             Thread::ThreadType thread_type, panda::panda_file::SourceLang thread_lang)
    : Thread(panda_vm, thread_type),
      id_(id),
      thread_lang_(thread_lang),
      pt_thread_info_(allocator->New<tooling::PtThreadInfo>()),
      thread_frame_states_(allocator->Adapter())
{
    ASSERT(zero_tlab != nullptr);
    tlab_ = zero_tlab;

    // WORKAROUND(v.cherkashin): EcmaScript side build doesn't have GC, so we skip setting barriers for this case
    mem::GC *gc = panda_vm->GetGC();
    if (gc != nullptr) {
        pre_barrier_type_ = gc->GetBarrierSet()->GetPreType();
        post_barrier_type_ = gc->GetBarrierSet()->GetPostType();
        auto barrier_set = gc->GetBarrierSet();
        if (barrier_set->GetPreType() != panda::mem::BarrierType::PRE_WRB_NONE) {
            auto addr = barrier_set->GetBarrierOperand(panda::mem::BarrierPosition::BARRIER_POSITION_PRE,
                                                       "CONCURRENT_MARKING_ADDR");
            concurrent_marking_addr_ = std::get<std::atomic<bool> *>(addr.GetValue());
            pre_buff_ = allocator->New<PandaVector<ObjectHeader *>>();
            // need to initialize in constructor because we have barriers between constructor and InitBuffers in
            // InitializedClasses
            g1_post_barrier_ring_buffer_ = allocator->New<mem::GCG1BarrierSet::G1PostBarrierRingBufferType>();
        }
    }

    stack_frame_allocator_ =
        allocator->New<mem::StackFrameAllocator>(Runtime::GetOptions().UseMallocForInternalAllocations());
    internal_local_allocator_ =
        mem::InternalAllocator<>::SetUpLocalInternalAllocator(static_cast<mem::Allocator *>(allocator));
    tagged_handle_storage_ = allocator->New<HandleStorage<TaggedType>>(allocator);
    tagged_global_handle_storage_ = allocator->New<GlobalHandleStorage<TaggedType>>(allocator);
    object_header_handle_storage_ = allocator->New<HandleStorage<ObjectHeader *>>(allocator);
}

ManagedThread::~ManagedThread()
{
    // ManagedThread::ShutDown() may not be called when exiting js_thread, so need set current_thread = nullptr
    // NB! ThreadManager is expected to store finished threads in separate list and GC destroys them,
    // current_thread should be nullified in Destroy()
    // (zero_tlab == nullptr means that we destroyed Runtime and do not need to register TLAB)
    if (zero_tlab != nullptr) {
        // We should register TLAB size for MemStats during thread destroy.
        GetVM()->GetHeapManager()->RegisterTLAB(GetTLAB());
    }

    mem::InternalAllocatorPtr allocator = GetInternalAllocator(this);
    allocator->Delete(object_header_handle_storage_);
    allocator->Delete(tagged_global_handle_storage_);
    allocator->Delete(tagged_handle_storage_);
    mem::InternalAllocator<>::FinalizeLocalInternalAllocator(internal_local_allocator_,
                                                             static_cast<mem::Allocator *>(allocator));
    internal_local_allocator_ = nullptr;
    allocator->Delete(stack_frame_allocator_);
    allocator->Delete(pt_thread_info_.release());

    ASSERT(thread_frame_states_.empty() && "stack should be empty");
}

void ManagedThread::InitBuffers()
{
    auto allocator = GetInternalAllocator(this);
    mem::GC *gc = GetVM()->GetGC();
    auto barrier = gc->GetBarrierSet();
    if (barrier->GetPreType() != panda::mem::BarrierType::PRE_WRB_NONE) {
        // we need to recreate buffers if it was detach (we removed all structures) and attach again
        // skip initializing in first attach after constructor
        if (pre_buff_ == nullptr) {
            ASSERT(pre_buff_ == nullptr);
            pre_buff_ = allocator->New<PandaVector<ObjectHeader *>>();
            ASSERT(g1_post_barrier_ring_buffer_ == nullptr);
            g1_post_barrier_ring_buffer_ = allocator->New<mem::GCG1BarrierSet::G1PostBarrierRingBufferType>();
        }
    }
}

NO_INLINE static uintptr_t GetStackTop()
{
    return ToUintPtr(__builtin_frame_address(0));
}

NO_INLINE static void LoadStackPages(uintptr_t end_addr)
{
    // ISO C++ forbids variable length array and alloca is unsafe,
    // so we have to extend stack step by step via recursive call
    constexpr size_t margin = 512;
    constexpr size_t page_size = 4_KB;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    volatile uint8_t stack_buffer[page_size - margin];
    if (ToUintPtr(&(stack_buffer[0])) >= end_addr + page_size) {
        LoadStackPages(end_addr);
    }
    stack_buffer[0] = 0;
}

void ManagedThread::InitForStackOverflowCheck(size_t native_stack_reserved_size, size_t native_stack_protected_size)
{
    void *stack_base = nullptr;
    size_t guard_size;
    size_t stack_size;
#if defined(PANDA_ASAN_ON) || defined(PANDA_TSAN_ON) || !defined(NDEBUG)
    static constexpr size_t reserved_size = 64_KB;
#else
    static constexpr size_t reserved_size = 8_KB;
#endif
    static_assert(STACK_OVERFLOW_RESERVED_SIZE == reserved_size);  // compiler depends on this to test load!!!
    int error = os::thread::ThreadGetStackInfo(os::thread::GetNativeHandle(), &stack_base, &stack_size, &guard_size);
    if (error != 0) {
        LOG(ERROR, RUNTIME) << "InitForStackOverflowCheck: fail to get stack info, error = " << strerror(errno);
        return;
    }
    if (guard_size < panda::os::mem::GetPageSize()) {
        guard_size = panda::os::mem::GetPageSize();
    }
    if (stack_size <= native_stack_reserved_size + native_stack_protected_size + guard_size) {
        LOG(ERROR, RUNTIME) << "InitForStackOverflowCheck: stack size not enough, stack_base = " << stack_base
                            << ", stack_size = " << stack_size << ", guard_size = " << guard_size;
        return;
    }
    LOG(DEBUG, RUNTIME) << "InitForStackOverflowCheck: stack_base = " << stack_base << ", stack_size = " << stack_size
                        << ", guard_size = " << guard_size;
    native_stack_begin_ = ToUintPtr(stack_base) + guard_size;
    native_stack_end_ = native_stack_begin_ + native_stack_protected_size + native_stack_reserved_size;
    native_stack_reserved_size_ = native_stack_reserved_size;
    native_stack_protected_size_ = native_stack_protected_size;
    native_stack_guard_size_ = guard_size;
    native_stack_size_ = stack_size;
    iframe_stack_size_ = stack_size;  // init frame stack size same with native stack size
    ProtectNativeStack();
}

void ManagedThread::ProtectNativeStack()
{
    if (native_stack_protected_size_ == 0) {
        return;
    }

    // Try to mprotect directly
    if (!panda::os::mem::MakeMemProtected(ToVoidPtr(native_stack_begin_), native_stack_protected_size_)) {
        return;
    }

    // If fail to mprotect, try to load stack page and then retry to mprotect
    uintptr_t native_stack_top = AlignDown(GetStackTop(), panda::os::mem::GetPageSize());
    LOG(DEBUG, RUNTIME) << "ProtectNativeStack: try to load pages, mprotect error = " << strerror(errno)
                        << ", stack_begin = " << native_stack_begin_ << ", stack_top = " << native_stack_top
                        << ", stack_size = " << native_stack_size_ << ", guard_size = " << native_stack_guard_size_;
    if (native_stack_size_ > STACK_MAX_SIZE_OVERFLOW_CHECK || native_stack_end_ >= native_stack_top ||
        native_stack_top > native_stack_end_ + STACK_MAX_SIZE_OVERFLOW_CHECK) {
        LOG(ERROR, RUNTIME) << "ProtectNativeStack: too large stack, mprotect error = " << strerror(errno)
                            << ", max_stack_size = " << STACK_MAX_SIZE_OVERFLOW_CHECK
                            << ", stack_begin = " << native_stack_begin_ << ", stack_top = " << native_stack_top
                            << ", stack_size = " << native_stack_size_ << ", guard_size = " << native_stack_guard_size_;
        return;
    }
    LoadStackPages(native_stack_begin_);
    if (panda::os::mem::MakeMemProtected(ToVoidPtr(native_stack_begin_), native_stack_protected_size_)) {
        LOG(ERROR, RUNTIME) << "ProtectNativeStack: fail to protect pages, error = " << strerror(errno)
                            << ", stack_begin = " << native_stack_begin_ << ", stack_top = " << native_stack_top
                            << ", stack_size = " << native_stack_size_ << ", guard_size = " << native_stack_guard_size_;
    }
    size_t release_size = native_stack_top - native_stack_begin_ - panda::os::mem::GetPageSize();
    if (panda::os::mem::ReleasePages(native_stack_begin_, native_stack_begin_ + release_size) != 0) {
        LOG(ERROR, RUNTIME) << "ProtectNativeStack: fail to release pages, error = " << strerror(errno)
                            << ", stack_begin = " << native_stack_begin_ << ", stack_top = " << native_stack_top
                            << ", stack_size = " << native_stack_size_ << ", guard_size = " << native_stack_guard_size_
                            << ", release_size = " << release_size;
    }
}

void ManagedThread::DisableStackOverflowCheck()
{
    native_stack_end_ = native_stack_begin_;
    iframe_stack_size_ = std::numeric_limits<size_t>::max();
    if (native_stack_protected_size_ > 0) {
        panda::os::mem::MakeMemReadWrite(ToVoidPtr(native_stack_begin_), native_stack_protected_size_);
    }
}

void ManagedThread::EnableStackOverflowCheck()
{
    native_stack_end_ = native_stack_begin_ + native_stack_protected_size_ + native_stack_reserved_size_;
    iframe_stack_size_ = native_stack_size_;
    if (native_stack_protected_size_ > 0) {
        panda::os::mem::MakeMemProtected(ToVoidPtr(native_stack_begin_), native_stack_protected_size_);
    }
}

// NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
void ManagedThread::SuspendCheck() NO_THREAD_SAFETY_ANALYSIS
{
    // We should use internal suspension to avoid missing call of IncSuspend
    SuspendImpl(true);
    Locks::mutator_lock->Unlock();
    Locks::mutator_lock->ReadLock();
    ResumeImpl(true);
}

void ManagedThread::SuspendImpl(bool internal_suspend)
{
    os::memory::LockHolder lock(suspend_lock_);
    LOG(DEBUG, RUNTIME) << "Suspending thread " << GetId();
    if (!internal_suspend) {
        if (IsUserSuspended()) {
            LOG(DEBUG, RUNTIME) << "thread " << GetId() << " is already suspended";
            return;
        }
        user_code_suspend_count_++;
    }
    auto old_count = suspend_count_++;
    if (old_count == 0) {
        SetFlag(SUSPEND_REQUEST);
    }
}

void ManagedThread::ResumeImpl(bool internal_resume)
{
    os::memory::LockHolder lock(suspend_lock_);
    LOG(DEBUG, RUNTIME) << "Resuming thread " << GetId();
    if (!internal_resume) {
        if (!IsUserSuspended()) {
            LOG(DEBUG, RUNTIME) << "thread " << GetId() << " is already resumed";
            return;
        }
        ASSERT(user_code_suspend_count_ != 0);
        user_code_suspend_count_--;
    }
    if (suspend_count_ > 0) {
        suspend_count_--;
        if (suspend_count_ == 0) {
            ClearFlag(SUSPEND_REQUEST);
        }
    }
    // Help for UnregisterExitedThread
    TSAN_ANNOTATE_HAPPENS_BEFORE(&fts_);
    suspend_var_.Signal();
}

void ManagedThread::SafepointPoll()
{
    if (this->TestAllFlags()) {
        trace::ScopedTrace scoped_trace("RunSafepoint");
        panda::interpreter::RuntimeInterface::Safepoint();
    }
}

void ManagedThread::NativeCodeBegin()
{
    LOG_IF(!(thread_frame_states_.empty() || thread_frame_states_.top() != NATIVE_CODE), FATAL, RUNTIME)
        << LogThreadStack(NATIVE_CODE) << " or stack should be empty";
    thread_frame_states_.push(NATIVE_CODE);
    UpdateStatus(ThreadStatus::NATIVE);
    is_managed_scope_ = false;
}

void ManagedThread::NativeCodeEnd()
{
    // thread_frame_states_ should not be accessed without MutatorLock (as runtime could have been destroyed)
    // If this was last frame, it should have been called from Destroy() and it should UpdateStatus to FINISHED
    // after this method
    UpdateStatus(ThreadStatus::RUNNING);
    is_managed_scope_ = true;
    LOG_IF(thread_frame_states_.empty(), FATAL, RUNTIME) << "stack should be not empty";
    LOG_IF(thread_frame_states_.top() != NATIVE_CODE, FATAL, RUNTIME) << LogThreadStack(NATIVE_CODE);
    thread_frame_states_.pop();
}

bool ManagedThread::IsInNativeCode() const
{
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    return thread_frame_states_.top() == NATIVE_CODE;
}

void ManagedThread::ManagedCodeBegin()
{
    // thread_frame_states_ should not be accessed without MutatorLock (as runtime could have been destroyed)
    UpdateStatus(ThreadStatus::RUNNING);
    is_managed_scope_ = true;
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    LOG_IF(thread_frame_states_.top() != NATIVE_CODE, FATAL, RUNTIME) << LogThreadStack(MANAGED_CODE);
    thread_frame_states_.push(MANAGED_CODE);
}

void ManagedThread::ManagedCodeEnd()
{
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    LOG_IF(thread_frame_states_.top() != MANAGED_CODE, FATAL, RUNTIME) << LogThreadStack(MANAGED_CODE);
    thread_frame_states_.pop();
    // Should be NATIVE_CODE
    UpdateStatus(ThreadStatus::NATIVE);
    is_managed_scope_ = false;
}

bool ManagedThread::IsManagedCode() const
{
    LOG_IF(HasClearStack(), FATAL, RUNTIME) << "stack should be not empty";
    return thread_frame_states_.top() == MANAGED_CODE;
}

// Since we don't allow two consecutive NativeCode frames, there is no managed code on stack if
// its size is 1 and last frame is Native
bool ManagedThread::HasManagedCodeOnStack() const
{
    if (HasClearStack()) {
        return false;
    }
    if (thread_frame_states_.size() == 1 && IsInNativeCode()) {
        return false;
    }
    return true;
}

bool ManagedThread::HasClearStack() const
{
    return thread_frame_states_.empty();
}

PandaString ManagedThread::ThreadStatusAsString(enum ThreadStatus status)
{
    switch (status) {
        case ThreadStatus::CREATED:
            return "New";
        case ThreadStatus::RUNNING:
            return "Runnable";
        case ThreadStatus::IS_BLOCKED:
            return "Blocked";
        case ThreadStatus::IS_WAITING:
            return "Waiting";
        case ThreadStatus::IS_TIMED_WAITING:
            return "Timed_waiting";
        case ThreadStatus::IS_SUSPENDED:
            return "Suspended";
        case ThreadStatus::IS_COMPILER_WAITING:
            return "Compiler_waiting";
        case ThreadStatus::IS_WAITING_INFLATION:
            return "Waiting_inflation";
        case ThreadStatus::IS_SLEEPING:
            return "Sleeping";
        case ThreadStatus::IS_TERMINATED_LOOP:
            return "Terminated_loop";
        case ThreadStatus::TERMINATING:
            return "Terminating";
        case ThreadStatus::NATIVE:
            return "Native";
        case ThreadStatus::FINISHED:
            return "Terminated";
        default:
            return "unknown";
    }
}

PandaString ManagedThread::LogThreadStack(ThreadState new_state) const
{
    PandaStringStream debug_message;
    static std::unordered_map<ThreadState, std::string> thread_state_to_string_map = {
        {ThreadState::NATIVE_CODE, "NATIVE_CODE"}, {ThreadState::MANAGED_CODE, "MANAGED_CODE"}};
    auto new_state_it = thread_state_to_string_map.find(new_state);
    auto top_frame_it = thread_state_to_string_map.find(thread_frame_states_.top());
    ASSERT(new_state_it != thread_state_to_string_map.end());
    ASSERT(top_frame_it != thread_state_to_string_map.end());

    debug_message << "threadId: " << GetId() << " "
                  << "tried go to " << new_state_it->second << " state, but last frame is: " << top_frame_it->second
                  << ", " << thread_frame_states_.size() << " frames in stack (from up to bottom): [";

    PandaStack<ThreadState> copy_stack(thread_frame_states_);
    while (!copy_stack.empty()) {
        auto it = thread_state_to_string_map.find(copy_stack.top());
        ASSERT(it != thread_state_to_string_map.end());
        debug_message << it->second;
        if (copy_stack.size() > 1) {
            debug_message << "|";
        }
        copy_stack.pop();
    }
    debug_message << "]";
    return debug_message.str();
}

MTManagedThread::MTManagedThread(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *panda_vm,
                                 panda::panda_file::SourceLang thread_lang)
    : ManagedThread(id, allocator, panda_vm, Thread::ThreadType::THREAD_TYPE_MT_MANAGED, thread_lang),
      waiting_monitor_(nullptr),
      entering_monitor_(nullptr)
{
    ASSERT(panda_vm != nullptr);
    internal_id_ = GetVM()->GetThreadManager()->GetInternalThreadId();

    auto ext = Runtime::GetCurrent()->GetClassLinker()->GetExtension(GetThreadLang());
    if (ext != nullptr) {
        string_class_ptr_ = ext->GetClassRoot(ClassRoot::STRING);
    }

    auto *rs = allocator->New<mem::ReferenceStorage>(panda_vm->GetGlobalObjectStorage(), allocator, false);
    LOG_IF((rs == nullptr || !rs->Init()), FATAL, RUNTIME) << "Cannot create pt reference storage";
    pt_reference_storage_ = PandaUniquePtr<mem::ReferenceStorage>(rs);
}

MTManagedThread::~MTManagedThread()
{
    ASSERT(internal_id_ != 0);
    GetVM()->GetThreadManager()->RemoveInternalThreadId(internal_id_);
}

void ManagedThread::PushLocalObject(ObjectHeader **object_header)
{
#ifdef PANDA_WITH_ECMASCRIPT
    // Object handles can be created during class initialization, so check lock state only after GC is started.
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           (Locks::mutator_lock->GetState() != MutatorLock::MutatorLockState::UNLOCKED) ||
           this->GetThreadLang() == panda::panda_file::SourceLang::ECMASCRIPT);
#else
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           (Locks::mutator_lock->GetState() != MutatorLock::MutatorLockState::UNLOCKED));
#endif

    local_objects_.push_back(object_header);
    LOG(DEBUG, GC) << "PushLocalObject for thread " << std::hex << this << ", obj = " << *object_header;
}

void ManagedThread::PopLocalObject()
{
#ifdef PANDA_WITH_ECMASCRIPT
    // Object handles can be created during class initialization, so check lock state only after GC is started.
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           (Locks::mutator_lock->GetState() != MutatorLock::MutatorLockState::UNLOCKED) ||
           this->GetThreadLang() == panda::panda_file::SourceLang::ECMASCRIPT);
#else
    ASSERT(!ManagedThread::GetCurrent()->GetVM()->GetGC()->IsGCRunning() ||
           (Locks::mutator_lock->GetState() != MutatorLock::MutatorLockState::UNLOCKED));
#endif

    ASSERT(!local_objects_.empty());
    LOG(DEBUG, GC) << "PopLocalObject from thread " << std::hex << this << ", obj = " << *local_objects_.back();
    local_objects_.pop_back();
}

void MTManagedThread::PushLocalObjectLocked(ObjectHeader *obj)
{
    local_objects_locked_.emplace_back(obj, GetFrame());
}

void MTManagedThread::PopLocalObjectLocked([[maybe_unused]] ObjectHeader *out)
{
    if (LIKELY(!local_objects_locked_.empty())) {
#ifndef NDEBUG
        ObjectHeader *obj = local_objects_locked_.back().GetObject();
        if (obj != out) {
            LOG(WARNING, RUNTIME) << "Locked object is not paired";
        }
#endif  // !NDEBUG
        local_objects_locked_.pop_back();
    } else {
        LOG(WARNING, RUNTIME) << "PopLocalObjectLocked failed, current thread locked object is empty";
    }
}

Span<LockedObjectInfo> MTManagedThread::GetLockedObjectInfos()
{
    return local_objects_locked_.data();
}

void ManagedThread::UpdateTLAB(mem::TLAB *tlab)
{
    ASSERT(tlab_ != nullptr);
    ASSERT(tlab != nullptr);
    tlab_ = tlab;
}

void ManagedThread::ClearTLAB()
{
    ASSERT(zero_tlab != nullptr);
    tlab_ = zero_tlab;
}

/* Common actions for creation of the thread. */
void MTManagedThread::ProcessCreatedThread()
{
    ManagedThread::SetCurrent(this);
    // Runtime takes ownership of the thread
    trace::ScopedTrace scoped_trace2("ThreadManager::RegisterThread");
    GetVM()->GetThreadManager()->RegisterThread(this);
    NativeCodeBegin();
}

void ManagedThread::UpdateGCRoots()
{
    if ((exception_ != nullptr) && (exception_->IsForwarded())) {
        exception_ = ::panda::mem::GetForwardAddress(exception_);
    }
    for (auto &&it : local_objects_) {
        if ((*it)->IsForwarded()) {
            (*it) = ::panda::mem::GetForwardAddress(*it);
        }
    }

    if (!tagged_handle_scopes_.empty()) {
        tagged_handle_storage_->UpdateHeapObject();
        tagged_global_handle_storage_->UpdateHeapObject();
    }

    if (!object_header_handle_scopes_.empty()) {
        object_header_handle_storage_->UpdateHeapObject();
    }
}

/* return true if sleep is interrupted */
bool MTManagedThread::Sleep(uint64_t ms)
{
    auto thread = MTManagedThread::GetCurrent();
    bool is_interrupted = thread->IsInterrupted();
    if (!is_interrupted) {
        thread->TimedWait(ThreadStatus::IS_SLEEPING, ms, 0);
        is_interrupted = thread->IsInterrupted();
    }
    return is_interrupted;
}

void ManagedThread::SetThreadPriority(int32_t prio)
{
    ThreadId tid = GetId();
    int res = os::thread::SetPriority(tid, prio);
    if (!os::thread::IsSetPriorityError(res)) {
        LOG(DEBUG, RUNTIME) << "Successfully changed priority for thread " << tid << " to " << prio;
    } else {
        LOG(DEBUG, RUNTIME) << "Cannot change priority for thread " << tid << " to " << prio;
    }
}

uint32_t ManagedThread::GetThreadPriority()
{
    ThreadId tid = GetId();
    return os::thread::GetPriority(tid);
}

void MTManagedThread::UpdateGCRoots()
{
    ManagedThread::UpdateGCRoots();
    for (auto &it : local_objects_locked_.data()) {
        if (it.GetObject()->IsForwarded()) {
            it.SetObject(panda::mem::GetForwardAddress(it.GetObject()));
        }
    }

    // Update enter_monitor_object_
    if (enter_monitor_object_ != nullptr && enter_monitor_object_->IsForwarded()) {
        enter_monitor_object_ = panda::mem::GetForwardAddress(enter_monitor_object_);
    }

    pt_reference_storage_->UpdateMovedRefs();
}

void MTManagedThread::VisitGCRoots(const ObjectVisitor &cb)
{
    ManagedThread::VisitGCRoots(cb);

    // Visit enter_monitor_object_
    if (enter_monitor_object_ != nullptr) {
        cb(enter_monitor_object_);
    }

    pt_reference_storage_->VisitObjects([&cb](const mem::GCRoot &gc_root) { cb(gc_root.GetObjectHeader()); },
                                        mem::RootType::ROOT_PT_LOCAL);
}
void MTManagedThread::SetDaemon()
{
    is_daemon_ = true;
    GetVM()->GetThreadManager()->AddDaemonThread();
    SetThreadPriority(MIN_PRIORITY);
}

void MTManagedThread::Interrupt(MTManagedThread *thread)
{
    os::memory::LockHolder lock(thread->cond_lock_);
    LOG(DEBUG, RUNTIME) << "Interrupt a thread " << thread->GetId();
    thread->SetInterruptedWithLockHeld(true);
    thread->SignalWithLockHeld();
    thread->InterruptPostImpl();
}

bool MTManagedThread::Interrupted()
{
    os::memory::LockHolder lock(cond_lock_);
    bool res = IsInterruptedWithLockHeld();
    SetInterruptedWithLockHeld(false);
    return res;
}

void MTManagedThread::StopDaemonThread()
{
    SetRuntimeTerminated();
    MTManagedThread::Interrupt(this);
}

void ManagedThread::VisitGCRoots(const ObjectVisitor &cb)
{
    if (exception_ != nullptr) {
        cb(exception_);
    }
    for (auto it : local_objects_) {
        cb(*it);
    }

    if (!tagged_handle_scopes_.empty()) {
        tagged_handle_storage_->VisitGCRoots(cb);
        tagged_global_handle_storage_->VisitGCRoots(cb);
    }
    if (!object_header_handle_scopes_.empty()) {
        object_header_handle_storage_->VisitGCRoots(cb);
    }
}

void MTManagedThread::Destroy()
{
    ASSERT(this == ManagedThread::GetCurrent());
    ASSERT(GetStatus() != ThreadStatus::FINISHED);

    UpdateStatus(ThreadStatus::TERMINATING);  // Set this status to prevent runtime for destroying itself while this
                                              // NATTIVE thread
    // is trying to acquire runtime.
    ReleaseMonitors();
    if (!IsDaemon()) {
        Runtime *runtime = Runtime::GetCurrent();
        runtime->GetNotificationManager()->ThreadEndEvent(this);
    }

    if (GetVM()->GetThreadManager()->UnregisterExitedThread(this)) {
        // Clear current_thread only if unregistration was successfull
        ManagedThread::SetCurrent(nullptr);
    }
}

CustomTLSData *ManagedThread::GetCustomTLSData(const char *key)
{
    os::memory::LockHolder lock(*Locks::custom_tls_lock);
    auto it = custom_tls_cache_.find(key);
    if (it == custom_tls_cache_.end()) {
        return nullptr;
    }
    return it->second.get();
}

void ManagedThread::SetCustomTLSData(const char *key, CustomTLSData *data)
{
    os::memory::LockHolder lock(*Locks::custom_tls_lock);
    PandaUniquePtr<CustomTLSData> tls_data(data);
    auto it = custom_tls_cache_.find(key);
    if (it == custom_tls_cache_.end()) {
        custom_tls_cache_[key] = {PandaUniquePtr<CustomTLSData>()};
    }
    custom_tls_cache_[key].swap(tls_data);
}

bool ManagedThread::EraseCustomTLSData(const char *key)
{
    os::memory::LockHolder lock(*Locks::custom_tls_lock);
    return custom_tls_cache_.erase(key) != 0;
}

LanguageContext ManagedThread::GetLanguageContext()
{
    return Runtime::GetCurrent()->GetLanguageContext(thread_lang_);
}

void MTManagedThread::FreeInternalMemory()
{
    local_objects_locked_.~LockedObjectList<>();
    pt_reference_storage_.reset();

    ManagedThread::FreeInternalMemory();
}

void ManagedThread::DestroyInternalResources()
{
    GetVM()->GetGC()->OnThreadTerminate(this);
    ASSERT(pre_buff_ == nullptr);
    ASSERT(g1_post_barrier_ring_buffer_ == nullptr);
    pt_thread_info_->Destroy();
}

void ManagedThread::FreeInternalMemory()
{
    thread_frame_states_.~PandaStack<ThreadState>();
    DestroyInternalResources();

    local_objects_.~PandaVector<ObjectHeader **>();
    {
        os::memory::LockHolder lock(*Locks::custom_tls_lock);
        custom_tls_cache_.~PandaMap<const char *, PandaUniquePtr<CustomTLSData>>();
    }

    mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(stack_frame_allocator_);
    allocator->Delete(internal_local_allocator_);

    allocator->Delete(pt_thread_info_.release());

    tagged_handle_scopes_.~PandaVector<HandleScope<coretypes::TaggedType> *>();
    allocator->Delete(tagged_handle_storage_);
    allocator->Delete(tagged_global_handle_storage_);

    allocator->Delete(object_header_handle_storage_);
    object_header_handle_scopes_.~PandaVector<HandleScope<ObjectHeader *> *>();

    Thread::FreeInternalMemory();
}

void ManagedThread::PrintSuspensionStackIfNeeded()
{
    if (!Runtime::GetOptions().IsSafepointBacktrace()) {
        return;
    }
    PandaStringStream out;
    out << "Thread " << GetId() << " is suspended at\n";
    PrintStack(out);
    LOG(INFO, RUNTIME) << out.str();
}

}  // namespace panda
