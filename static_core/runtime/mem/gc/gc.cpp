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

#include <memory>

#include "libpandabase/os/cpu_affinity.h"
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
#include "runtime/mem/gc/epsilon-g1/epsilon-g1.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root-inl.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/gen-gc/gen-gc.h"
#include "runtime/mem/gc/stw-gc/stw-gc.h"
#include "runtime/mem/gc/workers/gc_workers_task_queue.h"
#include "runtime/mem/gc/workers/gc_workers_thread_pool.h"
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
    if (gc_settings_.UseTaskManagerForGC()) {
        // Create gc task queue for task manager
        gc_workers_task_queue_ = internal_allocator_->New<taskmanager::TaskQueue>(
            taskmanager::TaskType::GC, taskmanager::VMType::STATIC_VM, GC_TASK_QUEUE_PRIORITY);
        ASSERT(gc_workers_task_queue_ != nullptr);
        // Register created gc task queue in task manager
        [[maybe_unused]] auto gc_queue_id =
            taskmanager::TaskScheduler::GetTaskScheduler()->RegisterQueue(gc_workers_task_queue_);
        ASSERT(gc_queue_id != taskmanager::INVALID_TASKQUEUE_ID);
    }
}

GC::~GC()
{
    InternalAllocatorPtr allocator = GetInternalAllocator();
    if (gc_worker_ != nullptr) {
        allocator->Delete(gc_worker_);
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
    if (workers_task_pool_ != nullptr) {
        allocator->Delete(workers_task_pool_);
    }
    if (gc_workers_task_queue_ != nullptr) {
        allocator->Delete(gc_workers_task_queue_);
    }
}

Logger::Buffer GC::GetLogPrefix() const
{
    const char *phase = GCScopedPhase::GetPhaseAbbr(GetGCPhase());
    // Atomic with acquire order reason: data race with gc_counter_
    size_t counter = gc_counter_.load(std::memory_order_acquire);

    Logger::Buffer buffer;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    buffer.Printf("[%zu, %s]: ", counter, phase);

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
        constexpr uint64_t WAIT_FINISHED = 100;
        // Use NativeSleep for all threads, as this thread shouldn't hold Mutator lock here
        os::thread::NativeSleepUS(std::chrono::microseconds(WAIT_FINISHED));
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
                AddGCTask(false, std::move(task));
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
    this->SetPandaVM(vm);
    InitializeImpl();
    gc_worker_ = allocator->New<GCWorker>(this);
}

void GC::CreateWorkersTaskPool()
{
    ASSERT(workers_task_pool_ == nullptr);
    if (this->IsWorkerThreadsExist()) {
        auto allocator = GetInternalAllocator();
        GCWorkersTaskPool *gc_task_pool = nullptr;
        if (this->GetSettings()->UseThreadPoolForGC()) {
            // Use internal gc thread pool
            gc_task_pool = allocator->New<GCWorkersThreadPool>(this, this->GetSettings()->GCWorkersCount());
        } else {
            // Use common TaskManager
            ASSERT(this->GetSettings()->UseTaskManagerForGC());
            gc_task_pool = allocator->New<GCWorkersTaskQueue>(this);
        }
        ASSERT(gc_task_pool != nullptr);
        workers_task_pool_ = gc_task_pool;
    }
}

void GC::DestroyWorkersTaskPool()
{
    if (workers_task_pool_ == nullptr) {
        return;
    }
    auto allocator = this->GetInternalAllocator();
    allocator->Delete(workers_task_pool_);
    workers_task_pool_ = nullptr;
}

void GC::StartGC()
{
    CreateWorker();
}

void GC::StopGC()
{
    DestroyWorker();
    DestroyWorkersTaskPool();
}

void GC::SetupCpuAffinity()
{
    if (!gc_settings_.ManageGcThreadsAffinity()) {
        return;
    }
    // Try to get CPU affinity fo GC Thread
    if (UNLIKELY(!os::CpuAffinityManager::GetCurrentThreadAffinity(affinity_before_gc_))) {
        affinity_before_gc_.Clear();
        return;
    }
    // Try to use best + middle for preventing issues when best core is used in another thread,
    // and GC waits for it to finish.
    if (!os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::BEST | os::CpuPower::MIDDLE)) {
        affinity_before_gc_.Clear();
    }
    // Some GCs don't use GC Workers
    if (workers_task_pool_ != nullptr && this->GetSettings()->UseThreadPoolForGC()) {
        static_cast<GCWorkersThreadPool *>(workers_task_pool_)->SetAffinityForGCWorkers();
    }
}

void GC::SetupCpuAffinityAfterConcurrent()
{
    if (!gc_settings_.ManageGcThreadsAffinity()) {
        return;
    }
    // Try to set GC Thread on best CPUs after concurrent
    if (!os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::BEST)) {
        // If it failed for only best CPUs then try to use best + middle CPUs mask after concurrent
        os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::BEST | os::CpuPower::MIDDLE);
    }
    // Some GCs don't use GC Workers
    if (workers_task_pool_ != nullptr && this->GetSettings()->UseThreadPoolForGC()) {
        static_cast<GCWorkersThreadPool *>(workers_task_pool_)->SetAffinityForGCWorkers();
    }
}

void GC::ResetCpuAffinity(bool before_concurrent)
{
    if (!gc_settings_.ManageGcThreadsAffinity()) {
        return;
    }
    if (!affinity_before_gc_.IsEmpty()) {
        // Set GC Threads on weak CPUs before concurrent if needed
        if (before_concurrent && gc_settings_.UseWeakCpuForGcConcurrent()) {
            os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::WEAK);
        } else {  // else set on saved affinity
            os::CpuAffinityManager::SetAffinityForCurrentThread(affinity_before_gc_);
        }
    }
    // Some GCs don't use GC Workers
    if (workers_task_pool_ != nullptr && this->GetSettings()->UseThreadPoolForGC()) {
        static_cast<GCWorkersThreadPool *>(workers_task_pool_)->UnsetAffinityForGCWorkers();
    }
}

void GC::SetupCpuAffinityBeforeConcurrent()
{
    ResetCpuAffinity(true);
}

void GC::RestoreCpuAffinity()
{
    ResetCpuAffinity(false);
}

bool GC::NeedRunGCAfterWaiting(size_t counter_before_waiting, const GCTask &task) const
{
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto new_counter = gc_counter_.load(std::memory_order_acquire);
    ASSERT(new_counter >= counter_before_waiting);
    // Atomic with acquire order reason: data race with last_cause_ with dependecies on reads after the load which
    // should become visible
    return (new_counter == counter_before_waiting || last_cause_.load(std::memory_order_acquire) < task.reason);
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
    if (!this->NeedRunGCAfterWaiting(old_counter, task)) {
        SetGCPhase(GCPhase::GC_PHASE_IDLE);
        return;
    }
    this->SetupCpuAffinity();
    this->GetTiming()->Reset();  // Clear records.
    // Atomic with release order reason: data race with last_cause_ with dependecies on writes before the store which
    // should become visible acquire
    last_cause_.store(task.reason, std::memory_order_release);
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
        GetPandaVm()->DumpHeap(&os);
        std::cerr << os.str() << std::endl;
    }
    size_t bytes_in_heap_before_gc = GetPandaVm()->GetMemStats()->GetFootprintHeap();
    LOG_DEBUG_GC << "Bytes in heap before GC " << std::dec << bytes_in_heap_before_gc;
    {
        GCScopedStats scoped_stats(GetPandaVm()->GetGCStats(), gc_type_ == GCType::STW_GC ? GetStats() : nullptr);
        ScopedGcHung scoped_hung(&task);
        GetPandaVm()->GetGCStats()->ResetLastPause();

        FireGCStarted(task, bytes_in_heap_before_gc);
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
        FireGCFinished(task, bytes_in_heap_before_gc, bytes_in_heap_after_gc);
    }
    ASSERT(task.collection_type != GCCollectionType::NONE);
    LOG(INFO, GC) << "[" << gc_counter_ << "] [" << task.collection_type << " (" << task.reason << ")] "
                  << GetPandaVm()->GetGCStats()->GetStatistics();

    if (gc_settings_.IsDumpHeap()) {
        PandaOStringStream os;
        os << "Heap dump after GC" << std::endl;
        GetPandaVm()->DumpHeap(&os);
        std::cerr << os.str() << std::endl;
    }

    if (gc_settings_.PostGCHeapVerification()) {
        trace::ScopedTrace post_heap_verifier_trace("PostGCHeapVeriFier");
        size_t fail_count = VerifyHeap();
        if (gc_settings_.FailOnHeapVerification() && fail_count > 0) {
            LOG(FATAL, GC) << "Heap corrupted after GC, HeapVerifier found " << fail_count << " corruptions";
        }
    }
    this->RestoreCpuAffinity();

    SetGCPhase(GCPhase::GC_PHASE_IDLE);
}

template <class LanguageConfig>
GC *CreateGC(GCType gc_type, ObjectAllocatorBase *object_allocator, const GCSettings &settings)
{
    GC *ret = nullptr;
    InternalAllocatorPtr allocator {InternalAllocator<>::GetInternalAllocatorFromRuntime()};

    switch (gc_type) {
        case GCType::EPSILON_GC:
            ret = allocator->New<EpsilonGC<LanguageConfig>>(object_allocator, settings);
            break;
        case GCType::EPSILON_G1_GC:
            ret = allocator->New<EpsilonG1GC<LanguageConfig>>(object_allocator, settings);
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

bool GC::CheckGCCause(GCTaskCause cause) const
{
    return cause != GCTaskCause::INVALID_CAUSE;
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
    bool clear_soft_references = task.reason == GCTaskCause::OOM_CAUSE || IsExplicitFull(task);
    reference_processor_->ProcessReferences(false, clear_soft_references, gc_phase, pred);
    Reference *processed_ref = reference_processor_->CollectClearedReferences();
    if (processed_ref != nullptr) {
        os::memory::LockHolder holder(*cleared_references_lock_);
        // TODO(alovkov): ged rid of cleared_references_ and just enqueue refs here?
        cleared_references_->push_back(processed_ref);
    }
}

void GC::DestroyWorker()
{
    // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    gc_running_.store(false, std::memory_order_seq_cst);
    gc_worker_->DestroyThreadIfNeeded();
}

void GC::CreateWorker()
{
    // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    gc_running_.store(true, std::memory_order_seq_cst);
    ASSERT(gc_worker_ != nullptr);
    gc_worker_->CreateThreadIfNeeded();
}

void GC::DisableWorkerThreads()
{
    gc_settings_.SetGCWorkersCount(0);
    gc_settings_.SetParallelMarkingEnabled(false);
    gc_settings_.SetParallelCompactingEnabled(false);
    gc_settings_.SetParallelRefUpdatingEnabled(false);
}

void GC::EnableWorkerThreads()
{
    const RuntimeOptions &options = Runtime::GetOptions();
    gc_settings_.SetGCWorkersCount(options.GetGcWorkersCount());
    gc_settings_.SetParallelMarkingEnabled(options.IsGcParallelMarkingEnabled() && (options.GetGcWorkersCount() != 0));
    gc_settings_.SetParallelCompactingEnabled(options.IsGcParallelCompactingEnabled() &&
                                              (options.GetGcWorkersCount() != 0));
    gc_settings_.SetParallelRefUpdatingEnabled(options.IsGcParallelRefUpdatingEnabled() &&
                                               (options.GetGcWorkersCount() != 0));
}

void GC::PreZygoteFork()
{
    DestroyWorker();
    if (gc_settings_.UseTaskManagerForGC()) {
        ASSERT(gc_workers_task_queue_ != nullptr);
        ASSERT(gc_workers_task_queue_->IsEmpty());
    }
}

void GC::PostZygoteFork()
{
    CreateWorker();
}

class GC::PostForkGCTask : public GCTask {
public:
    PostForkGCTask(GCTaskCause gc_reason, uint64_t gc_target_time) : GCTask(gc_reason, gc_target_time) {}

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
        AddGCTask(true, std::move(task));
        LOG(DEBUG, GC) << "Add PostForkGCTask";
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
bool GC::AddGCTask(bool is_managed, PandaUniquePtr<GCTask> task)
{
    bool triggered_by_threshold = (task->reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
    if (gc_settings_.RunGCInPlace()) {
        auto *gc_task = task.get();
        if (IsGCRunning()) {
            if (is_managed) {
                return WaitForGCInManaged(*gc_task);
            }
            return WaitForGC(*gc_task);
        }
    } else {
        if (triggered_by_threshold) {
            bool expect = true;
            if (can_add_gc_task_.compare_exchange_strong(expect, false, std::memory_order_seq_cst)) {
                return gc_worker_->AddTask(std::move(task));
            }
        } else {
            return gc_worker_->AddTask(std::move(task));
        }
    }
    return false;
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
    ASSERT_NATIVE_CODE();
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

bool GC::WaitForGC(GCTask task)
{
    // TODO(maksenov): Notify only about pauses (#4681)
    Runtime::GetCurrent()->GetNotificationManager()->GarbageCollectorStartEvent();
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto old_counter = this->gc_counter_.load(std::memory_order_acquire);
    Timing suspend_threads_timing;
    {
        ScopedTiming t("SuspendThreads", suspend_threads_timing);
        this->GetPandaVm()->GetRendezvous()->SafepointBegin();
    }
    if (!this->NeedRunGCAfterWaiting(old_counter, task)) {
        this->GetPandaVm()->GetRendezvous()->SafepointEnd();
        return false;
    }

    // Create a copy of the constant GCTask to be able to change its value
    this->RunPhases(task);

    if (UNLIKELY(this->IsLogDetailedGcInfoEnabled())) {
        PrintDetailedLog();
    }

    this->GetPandaVm()->GetRendezvous()->SafepointEnd();
    Runtime::GetCurrent()->GetNotificationManager()->GarbageCollectorFinishEvent();
    this->GetPandaVm()->HandleGCFinished();
    this->GetPandaVm()->HandleEnqueueReferences();
    return true;
}

bool GC::WaitForGCInManaged(const GCTask &task)
{
    Thread *base_thread = Thread::GetCurrent();
    if (ManagedThread::ThreadIsManagedThread(base_thread)) {
        ManagedThread *thread = ManagedThread::CastFromThread(base_thread);
        ASSERT(thread->GetMutatorLock()->HasLock());
        [[maybe_unused]] bool is_daemon = MTManagedThread::ThreadIsMTManagedThread(base_thread) &&
                                          MTManagedThread::CastFromThread(base_thread)->IsDaemon();
        ASSERT(!is_daemon || thread->GetStatus() == ThreadStatus::RUNNING);
        vm_->GetMutatorLock()->Unlock();
        thread->PrintSuspensionStackIfNeeded();
        WaitForGC(task);
        vm_->GetMutatorLock()->ReadLock();
        ASSERT(vm_->GetMutatorLock()->HasLock());
        this->GetPandaVm()->HandleGCRoutineInMutator();
        return true;
    }
    return false;
}

void GC::StartConcurrentScopeRoutine() const {}

void GC::EndConcurrentScopeRoutine() const {}

void GC::PrintDetailedLog()
{
    for (auto &footprint : this->footprint_list_) {
        LOG(INFO, GC) << footprint.first << " : " << footprint.second;
    }
    LOG(INFO, GC) << this->GetTiming()->Dump();
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
        gc_->EndConcurrentScopeRoutine();
        gc_->GetPandaVm()->GetRendezvous()->SafepointBegin();
        gc_->SetupCpuAffinityAfterConcurrent();
    }
}

NO_THREAD_SAFETY_ANALYSIS void ConcurrentScope::Start()
{
    if (!started_ && gc_->IsConcurrencyAllowed()) {
        gc_->StartConcurrentScopeRoutine();
        gc_->SetupCpuAffinityBeforeConcurrent();
        gc_->GetPandaVm()->GetRendezvous()->SafepointEnd();
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
            mark_word.DecodeFromForwardingAddress(static_cast<MarkWord::MarkWordSize>(ToUintPtr(dst)));
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
                MarkWord::MarkWordSize addr = mark_word.GetForwardingAddress();
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

void GC::FireGCStarted(const GCTask &task, size_t bytes_in_heap_before_gc)
{
    for (auto listener : *gc_listeners_ptr_) {
        if (listener != nullptr) {
            listener->GCStarted(task, bytes_in_heap_before_gc);
        }
    }
}

void GC::FireGCFinished(const GCTask &task, size_t bytes_in_heap_before_gc, size_t bytes_in_heap_after_gc)
{
    for (auto listener : *gc_listeners_ptr_) {
        if (listener != nullptr) {
            listener->GCFinished(task, bytes_in_heap_before_gc, bytes_in_heap_after_gc);
        }
    }
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
