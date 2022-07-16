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


#include "runtime/mem/gc/gc.h"

#include <memory>

#include "libpandabase/os/mem.h"
#include "libpandabase/os/thread.h"
#include "libpandabase/utils/time.h"
#include "runtime/assert_gc_scope.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/mem/gc/epsilon/epsilon.h"
#include "runtime/mem/gc/gc_root-inl.h"
#include "runtime/mem/gc/gc_queue.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/gen-gc/gen-gc.h"
#include "runtime/mem/gc/stw-gc/stw-gc.h"
#include "runtime/mem/gc/gc_workers_thread_pool.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/gc/gc-hung/gc_hung.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/object_accessor-inl.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/thread_manager.h"

namespace panda::mem {
using TaggedValue = coretypes::TaggedValue;
using TaggedType = coretypes::TaggedType;
using DynClass = coretypes::DynClass;

GC::GC(ObjectAllocatorBase *object_allocator, const GCSettings &settings)
    : gc_settings_(settings),
      object_allocator_(object_allocator),
      internal_allocator_(InternalAllocator<>::GetInternalAllocatorFromRuntime())
{
}

GC::~GC()
{
    InternalAllocatorPtr allocator = GetInternalAllocator();
    if (gc_queue_ != nullptr) {
        allocator->Delete(gc_queue_);
    }
    if (gc_listeners_ptr_ != nullptr) {
        allocator->Delete(gc_listeners_ptr_);
    }
    if (gc_barrier_set_ != nullptr) {
        allocator->Delete(gc_barrier_set_);
    }
    if (cleared_references_ != nullptr) {
        allocator->Delete(cleared_references_);
    }
    if (cleared_references_lock_ != nullptr) {
        allocator->Delete(cleared_references_lock_);
    }
    if (workers_pool_ != nullptr) {
        allocator->Delete(workers_pool_);
    }
}

Logger::Buffer GC::GetLogPrefix() const
{
    const char *phase = GCScopedPhase::GetPhaseAbbr(GetGCPhase());
    // Atomic with acquire order reason: data race with gc_counter_
    size_t counter = gc_counter_.load(std::memory_order_acquire);

    Logger::Buffer buffer;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    buffer.printf("[%zu, %s]: ", counter, phase);

    return buffer;
}

GCType GC::GetType()
{
    return gc_type_;
}

void GC::SetPandaVM(PandaVM *vm)
{
    vm_ = vm;
    reference_processor_ = vm->GetReferenceProcessor();
}

NativeGcTriggerType GC::GetNativeGcTriggerType()
{
    return gc_settings_.GetNativeGcTriggerType();
}

size_t GC::SimpleNativeAllocationGcWatermark()
{
    return GetPandaVm()->GetOptions().GetMaxFree();
}

NO_THREAD_SAFETY_ANALYSIS void GC::WaitForIdleGC()
{
    while (!CASGCPhase(GCPhase::GC_PHASE_IDLE, GCPhase::GC_PHASE_RUNNING)) {
        GetPandaVm()->GetRendezvous()->SafepointEnd();
        // Interrupt the running GC if possible
        OnWaitForIdleFail();
        // TODO(dtrubenkov): resolve it more properly
        constexpr uint64_t WAIT_FINISHED = 10;
        // Use NativeSleep for all threads, as this thread shouldn't hold Mutator lock here
        os::thread::NativeSleep(WAIT_FINISHED);
        GetPandaVm()->GetRendezvous()->SafepointBegin();
    }
}

inline void GC::TriggerGCForNative()
{
    auto native_gc_trigger_type = GetNativeGcTriggerType();
    ASSERT_PRINT((native_gc_trigger_type == NativeGcTriggerType::NO_NATIVE_GC_TRIGGER) ||
                     (native_gc_trigger_type == NativeGcTriggerType::SIMPLE_STRATEGY),
                 "Unknown Native GC Trigger type");
    switch (native_gc_trigger_type) {
        case NativeGcTriggerType::NO_NATIVE_GC_TRIGGER:
            break;
        case NativeGcTriggerType::SIMPLE_STRATEGY:
            // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or
            // ordering constraints imposed on other reads or writes
            if (native_bytes_registered_.load(std::memory_order_relaxed) > SimpleNativeAllocationGcWatermark()) {
                auto task = MakePandaUnique<GCTask>(GCTaskCause::NATIVE_ALLOC_CAUSE, time::GetCurrentTimeInNanos());
                AddGCTask(false, std::move(task), true);
                ManagedThread::GetCurrent()->SafepointPoll();
            }
            break;
        default:
            LOG(FATAL, GC) << "Unknown Native GC Trigger type";
            break;
    }
}

void GC::Initialize(PandaVM *vm)
{
    trace::ScopedTrace scoped_trace(__PRETTY_FUNCTION__);
    // GC saved the PandaVM instance, so we get allocator from the PandaVM.
    auto allocator = GetInternalAllocator();
    gc_listeners_ptr_ = allocator->template New<PandaVector<GCListener *>>(allocator->Adapter());
    cleared_references_lock_ = allocator->New<os::memory::Mutex>();
    os::memory::LockHolder holder(*cleared_references_lock_);
    cleared_references_ = allocator->New<PandaVector<panda::mem::Reference *>>(allocator->Adapter());
    gc_queue_ = allocator->New<GCQueueWithTime>(this);
    this->SetPandaVM(vm);
    InitializeImpl();
}

void GC::StartGC()
{
    CreateWorker();
}

void GC::StopGC()
{
    JoinWorker();
    ASSERT(gc_queue_ != nullptr);
    gc_queue_->Finalize();
    if (workers_pool_ != nullptr) {
        InternalAllocatorPtr allocator = GetInternalAllocator();
        allocator->Delete(workers_pool_);
        workers_pool_ = nullptr;
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void GC::RunPhases(GCTask &task)
{
    DCHECK_ALLOW_GARBAGE_COLLECTION;
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto old_counter = gc_counter_.load(std::memory_order_acquire);
    WaitForIdleGC();
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto new_counter = gc_counter_.load(std::memory_order_acquire);
    if (new_counter > old_counter) {
        SetGCPhase(GCPhase::GC_PHASE_IDLE);
        return;
    }
    // Atomic with release order reason: data race with last_cause_ with dependecies on writes before the store which
    // should become visible acquire
    last_cause_.store(task.reason_, std::memory_order_release);
    if (gc_settings_.PreGCHeapVerification()) {
        trace::ScopedTrace pre_heap_verifier_trace("PreGCHeapVeriFier");
        size_t fail_count = VerifyHeap();
        if (gc_settings_.FailOnHeapVerification() && fail_count > 0) {
            LOG(FATAL, GC) << "Heap corrupted before GC, HeapVerifier found " << fail_count << " corruptions";
        }
    }
    // Atomic with acq_rel order reason: data race with gc_counter_ with dependecies on reads after the load and on
    // writes before the store
    gc_counter_.fetch_add(1, std::memory_order_acq_rel);
    if (gc_settings_.IsDumpHeap()) {
        PandaOStringStream os;
        os << "Heap dump before GC" << std::endl;
        GetPandaVm()->GetHeapManager()->DumpHeap(&os);
        std::cerr << os.str() << std::endl;
    }
    size_t bytes_in_heap_before_gc = GetPandaVm()->GetMemStats()->GetFootprintHeap();
    LOG_DEBUG_GC << "Bytes in heap before GC " << std::dec << bytes_in_heap_before_gc;
    {
        GCScopedStats scoped_stats(GetPandaVm()->GetGCStats(), gc_type_ == GCType::STW_GC ? GetStats() : nullptr);
        ScopedGcHung scoped_hung(&task);
        for (auto listener : *gc_listeners_ptr_) {
            if (listener != nullptr) {
                listener->GCStarted(bytes_in_heap_before_gc);
            }
        }

        PreRunPhasesImpl();
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        RunPhasesImpl(task);
        // Clear Internal allocator unused pools (must do it on pause to avoid race conditions):
        // - Clear global part:
        InternalAllocator<>::GetInternalAllocatorFromRuntime()->VisitAndRemoveFreePools(
            [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
        // - Clear local part:
        ClearLocalInternalAllocatorPools();

        size_t bytes_in_heap_after_gc = GetPandaVm()->GetMemStats()->GetFootprintHeap();
        // There is case than bytes_in_heap_after_gc > 0 and bytes_in_heap_before_gc == 0.
        // Because TLABs are registered during GC
        if (bytes_in_heap_after_gc > 0 && bytes_in_heap_before_gc > 0) {
            GetStats()->AddReclaimRatioValue(1 - static_cast<double>(bytes_in_heap_after_gc) / bytes_in_heap_before_gc);
        }
        LOG_DEBUG_GC << "Bytes in heap after GC " << std::dec << bytes_in_heap_after_gc;
        for (auto listener : *gc_listeners_ptr_) {
            if (listener != nullptr) {
                listener->GCFinished(task, bytes_in_heap_before_gc, bytes_in_heap_after_gc);
            }
        }
    }
    ASSERT(task.collection_type_ != GCCollectionType::NONE);
    LOG(INFO, GC) << "[" << gc_counter_ << "] [" << task.collection_type_ << " (" << task.reason_ << ")] "
                  << GetPandaVm()->GetGCStats()->GetStatistics();
    if (gc_settings_.IsDumpHeap()) {
        PandaOStringStream os;
        os << "Heap dump after GC" << std::endl;
        GetPandaVm()->GetHeapManager()->DumpHeap(&os);
        std::cerr << os.str() << std::endl;
    }

    if (gc_settings_.PostGCHeapVerification()) {
        trace::ScopedTrace post_heap_verifier_trace("PostGCHeapVeriFier");
        size_t fail_count = VerifyHeap();
        if (gc_settings_.FailOnHeapVerification() && fail_count > 0) {
            LOG(FATAL, GC) << "Heap corrupted after GC, HeapVerifier found " << fail_count << " corruptions";
        }
    }

    SetGCPhase(GCPhase::GC_PHASE_IDLE);
}

template <class LanguageConfig>
GC *CreateGC(GCType gc_type, ObjectAllocatorBase *object_allocator, const GCSettings &settings)
{
    GC *ret = nullptr;
    ASSERT_PRINT((gc_type == GCType::EPSILON_GC) || (gc_type == GCType::STW_GC) || (gc_type == GCType::GEN_GC) ||
                     (gc_type == GCType::G1_GC),
                 "Unknown GC type");
    InternalAllocatorPtr allocator {InternalAllocator<>::GetInternalAllocatorFromRuntime()};

    switch (gc_type) {
        case GCType::EPSILON_GC:
            ret = allocator->New<EpsilonGC<LanguageConfig>>(object_allocator, settings);
            break;
        case GCType::STW_GC:
            ret = allocator->New<StwGC<LanguageConfig>>(object_allocator, settings);
            break;
        case GCType::GEN_GC:
            ret = allocator->New<GenGC<LanguageConfig>>(object_allocator, settings);
            break;
        case GCType::G1_GC:
            ret = allocator->New<G1GC<LanguageConfig>>(object_allocator, settings);
            break;
        default:
            LOG(FATAL, GC) << "Unknown GC type";
            break;
    }
    return ret;
}

bool GC::MarkObjectIfNotMarked(ObjectHeader *object_header)
{
    ASSERT(object_header != nullptr);
    if (IsMarked(object_header)) {
        return false;
    }
    MarkObject(object_header);
    return true;
}

void GC::ProcessReference(GCMarkingStackType *objects_stack, const BaseClass *cls, const ObjectHeader *ref,
                          const ReferenceProcessPredicateT &pred)
{
    ASSERT(reference_processor_ != nullptr);
    reference_processor_->HandleReference(this, objects_stack, cls, ref, pred);
}

void GC::AddReference(ObjectHeader *from_obj, ObjectHeader *object)
{
    ASSERT(IsMarked(object));
    GCMarkingStackType references(this);
    // TODO(alovkov): support stack with workers here & put all refs in stack and only then process altogether for once
    ASSERT(!references.IsWorkersTaskSupported());
    references.PushToStack(from_obj, object);
    MarkReferences(&references, phase_);
    if (gc_type_ != GCType::EPSILON_GC) {
        ASSERT(references.Empty());
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void GC::ProcessReferences(GCPhase gc_phase, const GCTask &task, const ReferenceClearPredicateT &pred)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    ASSERT(reference_processor_ != nullptr);
    bool clear_soft_references = task.reason_ == GCTaskCause::OOM_CAUSE || task.reason_ == GCTaskCause::EXPLICIT_CAUSE;
    reference_processor_->ProcessReferences(false, clear_soft_references, gc_phase, pred);
    Reference *processed_ref = reference_processor_->CollectClearedReferences();
    if (processed_ref != nullptr) {
        os::memory::LockHolder holder(*cleared_references_lock_);
        // TODO(alovkov): ged rid of cleared_references_ and just enqueue refs here?
        cleared_references_->push_back(processed_ref);
    }
}

void GC::GCWorkerEntry(GC *gc, PandaVM *vm)
{
    // We need to set VM to current_thread, since GC can call ObjectAccessor::GetBarrierSet() methods
    Thread gc_thread(vm, Thread::ThreadType::THREAD_TYPE_GC);
    ScopedCurrentThread sct(&gc_thread);
    while (true) {
        auto task = gc->gc_queue_->GetTask();
        if (!gc->IsGCRunning()) {
            LOG(DEBUG, GC) << "Stopping GC thread";
            if (task != nullptr) {
                task->Release(Runtime::GetCurrent()->GetInternalAllocator());
            }
            break;
        }
        if (task == nullptr) {
            continue;
        }
        if (task->reason_ == GCTaskCause::INVALID_CAUSE) {
            task->Release(Runtime::GetCurrent()->GetInternalAllocator());
            continue;
        }
        LOG(DEBUG, GC) << "Running GC task, reason " << task->reason_;
        task->Run(*gc);
        task->Release(Runtime::GetCurrent()->GetInternalAllocator());
    }
}

void GC::JoinWorker()
{
    // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    gc_running_.store(false, std::memory_order_seq_cst);
    if (!gc_settings_.RunGCInPlace()) {
        ASSERT(worker_ != nullptr);
    }
    if (worker_ != nullptr && !gc_settings_.RunGCInPlace()) {
        ASSERT(gc_queue_ != nullptr);
        gc_queue_->Signal();
        worker_->join();
        InternalAllocatorPtr allocator = GetInternalAllocator();
        allocator->Delete(worker_);
        worker_ = nullptr;
    }
}

void GC::CreateWorker()
{
    // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    gc_running_.store(true, std::memory_order_seq_cst);
    ASSERT(worker_ == nullptr);
    if (!gc_settings_.RunGCInPlace()) {
        ASSERT(gc_queue_ != nullptr);
        InternalAllocatorPtr allocator = GetInternalAllocator();
        worker_ = allocator->New<std::thread>(GC::GCWorkerEntry, this, this->GetPandaVm());
        if (worker_ == nullptr) {
            LOG(FATAL, COMPILER) << "Cannot create a GC thread";
        }
        int res = os::thread::SetThreadName(worker_->native_handle(), "GCThread");
        if (res != 0) {
            LOG(ERROR, RUNTIME) << "Failed to set a name for the gc thread";
        }
    }
}

void GC::DisableWorkerThreads()
{
    gc_settings_.SetGCWorkersCount(0);
    gc_settings_.SetParallelMarkingEnabled(false);
    gc_settings_.SetParallelCompactingEnabled(false);
}

void GC::EnableWorkerThreads()
{
    const RuntimeOptions &options = Runtime::GetOptions();
    gc_settings_.SetGCWorkersCount(options.GetGcWorkersCount());
    gc_settings_.SetParallelMarkingEnabled(options.IsGcParallelMarkingEnabled() && (options.GetGcWorkersCount() != 0));
    gc_settings_.SetParallelCompactingEnabled(options.IsGcParallelCompactingEnabled() &&
                                              (options.GetGcWorkersCount() != 0));
}

class GC::PostForkGCTask : public GCTask {
public:
    PostForkGCTask(GCTaskCause reason, uint64_t target_time) : GCTask(reason, target_time) {}

    void Run(mem::GC &gc) override
    {
        LOG(DEBUG, GC) << "Runing PostForkGCTask";
        gc.GetPandaVm()->GetGCTrigger()->RestoreMinTargetFootprint();
        gc.PostForkCallback();
        GCTask::Run(gc);
    }

    ~PostForkGCTask() override = default;

    NO_COPY_SEMANTIC(PostForkGCTask);
    NO_MOVE_SEMANTIC(PostForkGCTask);
};

void GC::PreStartup()
{
    // Add a delay GCTask.
    if ((!Runtime::GetCurrent()->IsZygote()) && (!gc_settings_.RunGCInPlace())) {
        // divide 2 to temporarily set target footprint to a high value to disable GC during App startup.
        GetPandaVm()->GetGCTrigger()->SetMinTargetFootprint(Runtime::GetOptions().GetHeapSizeLimit() / 2);
        PreStartupImp();
        constexpr uint64_t DISABLE_GC_DURATION_NS = 2000 * 1000 * 1000;
        auto task = MakePandaUnique<PostForkGCTask>(GCTaskCause::STARTUP_COMPLETE_CAUSE,
                                                    time::GetCurrentTimeInNanos() + DISABLE_GC_DURATION_NS);
        AddGCTask(true, std::move(task), false);
        LOG(DEBUG, GC) << "Add PostForkGCTask";
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void GC::AddGCTask(bool is_managed, PandaUniquePtr<GCTask> task, bool triggered_by_threshold)
{
    if (gc_settings_.RunGCInPlace()) {
        auto *gc_task = task.release();
        if (IsGCRunning()) {
            if (is_managed) {
                WaitForGCInManaged(*gc_task);
            } else {
                WaitForGC(*gc_task);
            }
        }
        gc_task->Release(Runtime::GetCurrent()->GetInternalAllocator());
    } else {
        if (triggered_by_threshold) {
            bool expect = true;
            if (can_add_gc_task_.compare_exchange_strong(expect, false, std::memory_order_seq_cst)) {
                gc_queue_->AddTask(task.release());
            }
        } else {
            gc_queue_->AddTask(task.release());
        }
    }
}

bool GC::IsReference(const BaseClass *cls, const ObjectHeader *ref, const ReferenceCheckPredicateT &pred)
{
    ASSERT(reference_processor_ != nullptr);
    return reference_processor_->IsReference(cls, ref, pred);
}

void GC::EnqueueReferences()
{
    while (true) {
        panda::mem::Reference *ref = nullptr;
        {
            os::memory::LockHolder holder(*cleared_references_lock_);
            if (cleared_references_->empty()) {
                break;
            }
            ref = cleared_references_->back();
            cleared_references_->pop_back();
        }
        ASSERT(ref != nullptr);
        ASSERT(reference_processor_ != nullptr);
        reference_processor_->ScheduleForEnqueue(ref);
    }
}

bool GC::IsFullGC() const
{
    // Atomic with relaxed order reason: data race with is_full_gc_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    return is_full_gc_.load(std::memory_order_relaxed);
}

void GC::SetFullGC(bool value)
{
    // Atomic with relaxed order reason: data race with is_full_gc_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    is_full_gc_.store(value, std::memory_order_relaxed);
}

void GC::NotifyNativeAllocations()
{
    // Atomic with relaxed order reason: data race with native_objects_notified_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    native_objects_notified_.fetch_add(NOTIFY_NATIVE_INTERVAL, std::memory_order_relaxed);
    TriggerGCForNative();
}

void GC::RegisterNativeAllocation(size_t bytes)
{
    size_t allocated;
    do {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        allocated = native_bytes_registered_.load(std::memory_order_relaxed);
    } while (!native_bytes_registered_.compare_exchange_weak(allocated, allocated + bytes));
    if (allocated > std::numeric_limits<size_t>::max() - bytes) {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        native_bytes_registered_.store(std::numeric_limits<size_t>::max(), std::memory_order_relaxed);
    }
    TriggerGCForNative();
}

void GC::RegisterNativeFree(size_t bytes)
{
    size_t allocated;
    size_t new_freed_bytes;
    do {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        allocated = native_bytes_registered_.load(std::memory_order_relaxed);
        new_freed_bytes = std::min(allocated, bytes);
    } while (!native_bytes_registered_.compare_exchange_weak(allocated, allocated - new_freed_bytes));
}

size_t GC::GetNativeBytesFromMallinfoAndRegister() const
{
    size_t mallinfo_bytes = panda::os::mem::GetNativeBytesFromMallinfo();
    // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    size_t all_bytes = mallinfo_bytes + native_bytes_registered_.load(std::memory_order_relaxed);
    return all_bytes;
}

void GC::WaitForGCInManaged(const GCTask &task)
{
    Thread *base_thread = Thread::GetCurrent();
    if (ManagedThread::ThreadIsManagedThread(base_thread)) {
        ManagedThread *thread = ManagedThread::CastFromThread(base_thread);
        ASSERT(Locks::mutator_lock->HasLock());
        [[maybe_unused]] bool is_daemon = MTManagedThread::ThreadIsMTManagedThread(base_thread) &&
                                          MTManagedThread::CastFromThread(base_thread)->IsDaemon();
        ASSERT(!is_daemon || thread->GetStatus() == ThreadStatus::RUNNING);
        Locks::mutator_lock->Unlock();
        thread->PrintSuspensionStackIfNeeded();
        WaitForGC(task);
        Locks::mutator_lock->ReadLock();
        ASSERT(Locks::mutator_lock->HasLock());
    }
}

ConcurrentScope::ConcurrentScope(GC *gc, bool auto_start)
{
    LOG(DEBUG, GC) << "Start ConcurrentScope";
    gc_ = gc;
    if (auto_start) {
        Start();
    }
}

ConcurrentScope::~ConcurrentScope()
{
    LOG(DEBUG, GC) << "Stop ConcurrentScope";
    if (started_ && gc_->IsConcurrencyAllowed()) {
        gc_->GetPandaVm()->GetRendezvous()->SafepointBegin();
        gc_->GetPandaVm()->GetMemStats()->RecordGCPauseStart();
    }
}

NO_THREAD_SAFETY_ANALYSIS void ConcurrentScope::Start()
{
    if (!started_ && gc_->IsConcurrencyAllowed()) {
        gc_->GetPandaVm()->GetRendezvous()->SafepointEnd();
        gc_->GetPandaVm()->GetMemStats()->RecordGCPauseEnd();
        started_ = true;
    }
}

void GC::WaitForGCOnPygoteFork(const GCTask &task)
{
    // do nothing if no pygote space
    auto pygote_space_allocator = object_allocator_->GetPygoteSpaceAllocator();
    if (pygote_space_allocator == nullptr) {
        return;
    }

    // do nothing if not at first pygote fork
    if (pygote_space_allocator->GetState() != PygoteSpaceState::STATE_PYGOTE_INIT) {
        return;
    }

    LOG(DEBUG, GC) << "== GC WaitForGCOnPygoteFork Start ==";

    // do we need a lock?
    // looks all other threads have been stopped before pygote fork

    // 0. indicate that we're rebuilding pygote space
    pygote_space_allocator->SetState(PygoteSpaceState::STATE_PYGOTE_FORKING);

    // 1. trigger gc
    WaitForGC(task);

    // 2. move other space to pygote space
    MoveObjectsToPygoteSpace();

    // 3. indicate that we have done
    pygote_space_allocator->SetState(PygoteSpaceState::STATE_PYGOTE_FORKED);

    // 4. disable pygote for allocation
    object_allocator_->DisablePygoteAlloc();

    LOG(DEBUG, GC) << "== GC WaitForGCOnPygoteFork End ==";
}

bool GC::IsOnPygoteFork() const
{
    auto pygote_space_allocator = object_allocator_->GetPygoteSpaceAllocator();
    return pygote_space_allocator != nullptr &&
           pygote_space_allocator->GetState() == PygoteSpaceState::STATE_PYGOTE_FORKING;
}

void GC::MoveObjectsToPygoteSpace()
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    LOG(DEBUG, GC) << "MoveObjectsToPygoteSpace: start";

    size_t all_size_move = 0;
    size_t moved_objects_num = 0;
    size_t bytes_in_heap_before_move = GetPandaVm()->GetMemStats()->GetFootprintHeap();
    auto pygote_space_allocator = object_allocator_->GetPygoteSpaceAllocator();
    ObjectVisitor move_visitor(
        [this, &pygote_space_allocator, &moved_objects_num, &all_size_move](ObjectHeader *src) -> void {
            size_t size = GetObjectSize(src);
            auto dst = reinterpret_cast<ObjectHeader *>(pygote_space_allocator->Alloc(size));
            ASSERT(dst != nullptr);
            memcpy_s(dst, size, src, size);
            all_size_move += size;
            moved_objects_num++;
            SetForwardAddress(src, dst);
            LOG_DEBUG_GC << "object MOVED from " << std::hex << src << " to " << dst << ", size = " << std::dec << size;
        });

    // move all small movable objects to pygote space
    object_allocator_->IterateRegularSizeObjects(move_visitor);

    LOG(DEBUG, GC) << "MoveObjectsToPygoteSpace: move_num = " << moved_objects_num << ", move_size = " << all_size_move;

    if (all_size_move > 0) {
        GetStats()->AddMemoryValue(all_size_move, MemoryTypeStats::MOVED_BYTES);
        GetStats()->AddObjectsValue(moved_objects_num, ObjectTypeStats::MOVED_OBJECTS);
    }
    if (bytes_in_heap_before_move > 0) {
        GetStats()->AddCopiedRatioValue(static_cast<double>(all_size_move) / bytes_in_heap_before_move);
    }

    // Update because we moved objects from object_allocator -> pygote space
    UpdateRefsToMovedObjectsInPygoteSpace();
    CommonUpdateRefsToMovedObjects();

    // Clear the moved objects in old space
    object_allocator_->FreeObjectsMovedToPygoteSpace();

    LOG(DEBUG, GC) << "MoveObjectsToPygoteSpace: finish";
}

void GC::SetForwardAddress(ObjectHeader *src, ObjectHeader *dst)
{
    auto base_cls = src->ClassAddr<BaseClass>();
    if (base_cls->IsDynamicClass()) {
        auto cls = static_cast<HClass *>(base_cls);
        // Note: During moving phase, 'src => dst'. Consider the src is a DynClass,
        //       since 'dst' is not in GC-status the 'manage-object' inside 'dst' won't be updated to
        //       'dst'. To fix it, we update 'manage-object' here rather than upating phase.
        if (cls->IsHClass()) {
            size_t offset = ObjectHeader::ObjectHeaderSize() + HClass::GetManagedObjectOffset();
            dst->SetFieldObject<false, false, true>(GetPandaVm()->GetAssociatedThread(), offset, dst);
        }
    }

    // Set fwd address in src
    bool update_res = false;
    do {
        MarkWord mark_word = src->AtomicGetMark();
        MarkWord fwd_mark_word =
            mark_word.DecodeFromForwardingAddress(static_cast<MarkWord::markWordSize>(ToUintPtr(dst)));
        update_res = src->AtomicSetMark<false>(mark_word, fwd_mark_word);
    } while (!update_res);
}

void GC::UpdateRefsInVRegs(ManagedThread *thread)
{
    LOG_DEBUG_GC << "Update frames for thread: " << thread->GetId();
    for (auto pframe = StackWalker::Create(thread); pframe.HasFrame(); pframe.NextFrame()) {
        LOG_DEBUG_GC << "Frame for method " << pframe.GetMethod()->GetFullName();
        auto iterator = [&pframe, this](auto &reg_info, auto &vreg) {
            ObjectHeader *object_header = vreg.GetReference();
            if (object_header == nullptr) {
                return true;
            }
            MarkWord mark_word = object_header->AtomicGetMark();
            if (mark_word.GetState() == MarkWord::ObjectState::STATE_GC) {
                MarkWord::markWordSize addr = mark_word.GetForwardingAddress();
                LOG_DEBUG_GC << "Update vreg, vreg old val = " << std::hex << object_header << ", new val = 0x" << addr;
                LOG_IF(reg_info.IsAccumulator(), DEBUG, GC) << "^ acc reg";
                if (!pframe.IsCFrame() && reg_info.IsAccumulator()) {
                    LOG_DEBUG_GC << "^ acc updated";
                    vreg.SetReference(reinterpret_cast<ObjectHeader *>(addr));
                } else {
                    pframe.template SetVRegValue<std::is_same_v<decltype(vreg), interpreter::DynamicVRegisterRef &>>(
                        reg_info, reinterpret_cast<ObjectHeader *>(addr));
                }
            }
            return true;
        };
        pframe.IterateObjectsWithInfo(iterator);
    }
}

const ObjectHeader *GC::PopObjectFromStack(GCMarkingStackType *objects_stack)
{
    auto *object = objects_stack->PopFromStack();
    ASSERT(object != nullptr);
    return object;
}

bool GC::IsGenerational() const
{
    return IsGenerationalGCType(gc_type_);
}

void GC::FireGCPhaseStarted(GCPhase phase)
{
    for (auto listener : *gc_listeners_ptr_) {
        if (listener != nullptr) {
            listener->GCPhaseStarted(phase);
        }
    }
}

void GC::FireGCPhaseFinished(GCPhase phase)
{
    for (auto listener : *gc_listeners_ptr_) {
        if (listener != nullptr) {
            listener->GCPhaseFinished(phase);
        }
    }
}

void GC::OnWaitForIdleFail() {}

TEMPLATE_GC_CREATE_GC();

}  // namespace panda::mem
