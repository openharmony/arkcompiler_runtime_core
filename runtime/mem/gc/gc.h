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
#ifndef PANDA_RUNTIME_MEM_GC_GC_H
#define PANDA_RUNTIME_MEM_GC_GC_H

#include <atomic>
#include <map>
#include <string_view>
#include <vector>

#include "libpandabase/os/mutex.h"
#include "libpandabase/os/thread.h"
#include "libpandabase/trace/trace.h"
#include "libpandabase/utils/expected.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/object_header.h"
#include "runtime/include/language_config.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/mem/allocator_adapter.h"
#include "runtime/mem/gc/gc_settings.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/gc/gc_adaptive_stack.h"
#include "runtime/mem/gc/gc_scope.h"
#include "runtime/mem/gc/gc_scoped_phase.h"
#include "runtime/mem/gc/gc_stats.h"
#include "runtime/mem/gc/gc_types.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/mem/gc/bitmap.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/timing.h"
#include "runtime/mem/region_allocator.h"

namespace panda {
class BaseClass;
class HClass;
class PandaVM;
class Timing;
namespace mem {
class G1GCTest;
class GlobalObjectStorage;
class ReferenceProcessor;
namespace test {
class MemStatsGenGCTest;
class ReferenceStorageTest;
class RemSetTest;
}  // namespace test
namespace ecmascript {
class EcmaReferenceProcessor;
}  // namespace ecmascript
}  // namespace mem
}  // namespace panda

namespace panda::coretypes {
class Array;
class DynClass;
}  // namespace panda::coretypes

namespace panda::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG_GC LOG(DEBUG, GC) << this->GetLogPrefix()
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INFO_GC LOG(INFO, GC) << this->GetLogPrefix()
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG_OBJECT_EVENTS LOG(DEBUG, MM_OBJECT_EVENTS)

// forward declarations:
class GCListener;
class GCScopePhase;
class HybridObjectAllocator;
class GCScopedPhase;
class GCQueueInterface;
class GCDynamicObjectHelpers;
class GCWorkersThreadPool;
class GCWorkersTask;

enum class GCError { GC_ERROR_NO_ROOTS, GC_ERROR_NO_FRAMES, GC_ERROR_LAST = GC_ERROR_NO_FRAMES };

enum ClassRootsVisitFlag : bool {
    ENABLED = true,
    DISABLED = false,
};

enum CardTableVisitFlag : bool {
    VISIT_ENABLED = true,
    VISIT_DISABLED = false,
};

class GCListener {
public:
    GCListener() = default;
    NO_COPY_SEMANTIC(GCListener);
    DEFAULT_MOVE_SEMANTIC(GCListener);
    virtual ~GCListener() = default;
    virtual void GCStarted([[maybe_unused]] size_t heap_size) {}
    virtual void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heap_size_before_gc,
                            [[maybe_unused]] size_t heap_size)
    {
    }
    virtual void GCPhaseStarted([[maybe_unused]] GCPhase phase) {}
    virtual void GCPhaseFinished([[maybe_unused]] GCPhase phase) {}
};

class GCExtensionData;

using UpdateRefInObject = std::function<void(ObjectHeader *)>;

// base class for all GCs
class GC {
public:
    using MarkPredicate = std::function<bool(const ObjectHeader *)>;
    using ReferenceCheckPredicateT = std::function<bool(const ObjectHeader *)>;
    using ReferenceClearPredicateT = std::function<bool(const ObjectHeader *)>;
    using ReferenceProcessPredicateT = std::function<bool(const ObjectHeader *)>;

    static bool EmptyReferenceProcessPredicate([[maybe_unused]] const ObjectHeader *ref)
    {
        return true;
    }

    explicit GC(ObjectAllocatorBase *object_allocator, const GCSettings &settings);
    NO_COPY_SEMANTIC(GC);
    NO_MOVE_SEMANTIC(GC);
    virtual ~GC() = 0;

    GCType GetType();

    /**
     * \brief Initialize GC
     */
    void Initialize(PandaVM *vm);

    /**
     * \brief Starts GC after initialization
     * Creates worker thread, sets gc_running_ to true
     */
    virtual void StartGC();

    /**
     * \brief Stops GC for runtime destruction
     * Joins GC thread, clears queue
     */
    virtual void StopGC();

    /**
     * Should be used to wait while GC should work exlusively
     * Note: for non-mt STW GC can be used to run GC
     */
    virtual void WaitForGC(GCTask task) = 0;

    /**
     * Should be used to wait while GC should be executed in managed scope
     */
    void WaitForGCInManaged(const GCTask &task) NO_THREAD_SAFETY_ANALYSIS;

    /**
     * Only be used to at first pygote fork
     */
    void WaitForGCOnPygoteFork(const GCTask &task);

    bool IsOnPygoteFork() const;

    /**
     * Initialize GC bits on object creation.
     * Required only for GCs with switched bits
     */
    virtual void InitGCBits(panda::ObjectHeader *obj_header) = 0;

    /**
     * Initialize GC bits on object creation for the TLAB allocation.
     */
    virtual void InitGCBitsForAllocationInTLAB(panda::ObjectHeader *obj_header) = 0;

    bool IsTLABsSupported()
    {
        return tlabs_supported_;
    }

    /**
     * Triggers GC
     */
    virtual void Trigger() = 0;

    virtual bool IsFullGC() const;

    /**
     * Return true if gc has generations, false otherwise
     */
    bool IsGenerational() const;

    PandaString DumpStatistics()
    {
        return instance_stats_.GetDump(gc_type_);
    }

    void AddListener(GCListener *listener)
    {
        ASSERT(gc_listeners_ptr_ != nullptr);
        gc_listeners_ptr_->push_back(listener);
    }

    void RemoveListener(GCListener *listener)
    {
        ASSERT(gc_listeners_ptr_ != nullptr);
        auto it = std::find(gc_listeners_ptr_->begin(), gc_listeners_ptr_->end(), listener);
        *it = nullptr;
    }

    GCBarrierSet *GetBarrierSet()
    {
        ASSERT(gc_barrier_set_ != nullptr);
        return gc_barrier_set_;
    }

    GCWorkersThreadPool *GetWorkersPool()
    {
        ASSERT(workers_pool_ != nullptr);
        return workers_pool_;
    }

    // Additional NativeGC
    void NotifyNativeAllocations();

    void RegisterNativeAllocation(size_t bytes);

    void RegisterNativeFree(size_t bytes);

    int32_t GetNotifyNativeInterval()
    {
        return NOTIFY_NATIVE_INTERVAL;
    }

    // Calling CheckGCForNative immediately for every NOTIFY_NATIVE_INTERVAL allocations
    static constexpr int32_t NOTIFY_NATIVE_INTERVAL = 32;

    // Calling CheckGCForNative immediately if size exceeds the following
    static constexpr size_t CHECK_IMMEDIATELY_THRESHOLD = 300000;

    inline bool IsLogDetailedGcInfoEnabled() const
    {
        return gc_settings_.LogDetailedGCInfoEnabled();
    }

    inline GCPhase GetGCPhase() const
    {
        return phase_;
    }

    inline bool IsGCRunning()
    {
        // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent
        // order where threads observe all modifications in the same order
        return gc_running_.load(std::memory_order_seq_cst);
    }

    void PreStartup();

    InternalAllocatorPtr GetInternalAllocator() const
    {
        return internal_allocator_;
    }

    /**
     * Enqueue all references in ReferenceQueue. Should be done after GC to avoid deadlock (lock in
     * ReferenceQueue.class)
     */
    void EnqueueReferences();

    /**
     * Process all references which GC found in marking phase.
     */
    void ProcessReferences(GCPhase gc_phase, const GCTask &task, const ReferenceClearPredicateT &pred);

    size_t GetNativeBytesRegistered()
    {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        return native_bytes_registered_.load(std::memory_order_relaxed);
    }

    virtual void SetPandaVM(PandaVM *vm);

    PandaVM *GetPandaVm() const
    {
        return vm_;
    }

    virtual void PreZygoteFork()
    {
        JoinWorker();
    }

    virtual void PostZygoteFork()
    {
        CreateWorker();
    }

    virtual void OnThreadTerminate([[maybe_unused]] ManagedThread *thread) {}

    void SetCanAddGCTask(bool can_add_task)
    {
        // Atomic with relaxed order reason: data race with can_add_gc_task_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        can_add_gc_task_.store(can_add_task, std::memory_order_relaxed);
    }

    GCExtensionData *GetExtensionData() const
    {
        return extension_data_;
    }

    void SetExtensionData(GCExtensionData *data)
    {
        extension_data_ = data;
    }

    virtual void PostForkCallback() {}

    /**
     * Check if the object addr is in the GC sweep range
     */
    virtual bool InGCSweepRange([[maybe_unused]] const ObjectHeader *obj) const
    {
        return true;
    }

    virtual CardTable *GetCardTable()
    {
        return nullptr;
    }

    /**
     * Called from GCWorker thread to assign thread specific data
     */
    virtual bool InitWorker(void **worker_data)
    {
        *worker_data = nullptr;
        return true;
    }

    /**
     * Called from GCWorker thread to destroy thread specific data
     */
    virtual void DestroyWorker([[maybe_unused]] void *worker_data) {}

    /**
     * Process a task sent to GC workers thread.
     */
    virtual void WorkerTaskProcessing([[maybe_unused]] GCWorkersTask *task, [[maybe_unused]] void *worker_data)
    {
        LOG(FATAL, GC) << "Unimplemented method";
    }

    virtual bool IsMutatorAllowed()
    {
        return false;
    }

    /**
     * Return true of ref is an instance of reference or it's ancestor, false otherwise
     */
    bool IsReference(const BaseClass *cls, const ObjectHeader *ref, const ReferenceCheckPredicateT &pred);

    void ProcessReference(GCMarkingStackType *objects_stack, const BaseClass *cls, const ObjectHeader *ref,
                          const ReferenceProcessPredicateT &pred);

    ALWAYS_INLINE ObjectAllocatorBase *GetObjectAllocator() const
    {
        return object_allocator_;
    }

    // called if we fail change state from idle to running
    virtual void OnWaitForIdleFail();

    virtual void PendingGC() {}

    /**
     * Check if the object is marked for GC(alive)
     * @param object
     * @return true if object marked for GC
     */
    virtual bool IsMarked(const ObjectHeader *object) const = 0;

    /**
     * Mark object.
     * Note: for some GCs it is not necessary set GC bit to 1.
     * @param object_header
     * @return true if object old state is not marked
     */
    virtual bool MarkObjectIfNotMarked(ObjectHeader *object_header);

    /**
     * Mark object.
     * Note: for some GCs it is not necessary set GC bit to 1.
     * @param object_header
     */
    virtual void MarkObject(ObjectHeader *object_header) = 0;

    /**
     * Add reference for later processing in marking phase
     * @param object - object from which we start to mark
     */
    void AddReference(ObjectHeader *from_object, ObjectHeader *object);

    inline void SetGCPhase(GCPhase gc_phase)
    {
        phase_ = gc_phase;
    }

protected:
    /**
     * \brief Runs all phases
     */
    void RunPhases(GCTask &task);

    /**
     * Add task to GC Queue to be run by GC thread (or run in place)
     */
    void AddGCTask(bool is_managed, PandaUniquePtr<GCTask> task, bool triggered_by_threshold);

    virtual void InitializeImpl() = 0;
    virtual void PreRunPhasesImpl() = 0;
    virtual void RunPhasesImpl(GCTask &task) = 0;
    virtual void PreStartupImp() {}

    inline bool IsTracingEnabled() const
    {
        return gc_settings_.IsGcEnableTracing();
    }

    inline void BeginTracePoint(const PandaString &trace_point_name) const
    {
        if (IsTracingEnabled()) {
            trace::BeginTracePoint(trace_point_name.c_str());
        }
    }

    inline void EndTracePoint() const
    {
        if (IsTracingEnabled()) {
            trace::EndTracePoint();
        }
    }

    virtual void VisitRoots(const GCRootVisitor &gc_root_visitor, VisitGCRootFlags flags) = 0;
    virtual void VisitClassRoots(const GCRootVisitor &gc_root_visitor) = 0;
    virtual void VisitCardTableRoots(CardTable *card_table, const GCRootVisitor &gc_root_visitor,
                                     const MemRangeChecker &range_checker, const ObjectChecker &range_object_checker,
                                     const ObjectChecker &from_object_checker, uint32_t processed_flag) = 0;

    inline bool CASGCPhase(GCPhase expected, GCPhase set)
    {
        return phase_.compare_exchange_strong(expected, set);
    }

    GCInstanceStats *GetStats()
    {
        return &instance_stats_;
    }

    inline void SetType(GCType gc_type)
    {
        gc_type_ = gc_type;
    }

    inline void SetTLABsSupported()
    {
        tlabs_supported_ = true;
    }

    void SetGCBarrierSet(GCBarrierSet *barrier_set)
    {
        ASSERT(gc_barrier_set_ == nullptr);
        gc_barrier_set_ = barrier_set;
    }

    void SetWorkersPool(GCWorkersThreadPool *thread_pool)
    {
        ASSERT(workers_pool_ == nullptr);
        workers_pool_ = thread_pool;
    }

    void ClearWorkersPool()
    {
        workers_pool_ = nullptr;
    }

    /**
     * Mark all references which we added by AddReference method
     */
    virtual void MarkReferences(GCMarkingStackType *references, GCPhase gc_phase) = 0;

    friend class HeapRootVisitor;

    virtual void UpdateRefsToMovedObjectsInPygoteSpace() = 0;
    /**
     * Update all refs to moved objects
     */
    virtual void CommonUpdateRefsToMovedObjects() = 0;

    virtual void UpdateVmRefs() = 0;

    virtual void UpdateGlobalObjectStorage() = 0;

    virtual void UpdateClassLinkerContextRoots() = 0;

    void UpdateRefsInVRegs(ManagedThread *thread);

    const ObjectHeader *PopObjectFromStack(GCMarkingStackType *objects_stack);

    Timing *GetTiming()
    {
        return &timing_;
    }

    template <GCScopeType gc_scope_type>
    friend class GCScope;

    void SetForwardAddress(ObjectHeader *src, ObjectHeader *dst);

    // vector here because we can add some references on young-gc and get new refs on old-gc
    // it's possible if we make 2 GCs for one safepoint
    // max length of this vector - is 2
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    PandaVector<panda::mem::Reference *> *cleared_references_ GUARDED_BY(cleared_references_lock_) {nullptr};

    os::memory::Mutex *cleared_references_lock_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)

    std::atomic<size_t> gc_counter_ {0};  // NOLINT(misc-non-private-member-variables-in-classes)
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<GCTaskCause> last_cause_ {GCTaskCause::INVALID_CAUSE};

    const GCSettings *GetSettings() const
    {
        return &gc_settings_;
    }

    const ReferenceProcessor *GetReferenceProcessor() const
    {
        return reference_processor_;
    }

    bool IsWorkerThreadsExist() const
    {
        return gc_settings_.GCWorkersCount() != 0;
    }

    void EnableWorkerThreads();
    void DisableWorkerThreads();

    /**
     * @return true if GC can work in concurrent mode
     */
    bool IsConcurrencyAllowed() const
    {
        return gc_settings_.IsConcurrencyEnabled();
    }

    Logger::Buffer GetLogPrefix() const;

    void FireGCPhaseStarted(GCPhase phase);
    void FireGCPhaseFinished(GCPhase phase);

    void SetFullGC(bool value);

    Timing timing_;  // NOLINT(misc-non-private-member-variables-in-classes)

    PandaVector<std::pair<PandaString, uint64_t>>
        footprint_list_;  // NOLINT(misc-non-private-member-variables-in-classes)
private:
    /**
     * Entrypoint for GC worker thread
     * @param gc pointer to GC structure
     * @param vm pointer to VM structure
     */
    static void GCWorkerEntry(GC *gc, PandaVM *vm);

    void JoinWorker();
    void CreateWorker();

    /**
     * Move small objects to pygote space at first pygote fork
     */
    void MoveObjectsToPygoteSpace();

    size_t GetNativeBytesFromMallinfoAndRegister() const;
    virtual void ClearLocalInternalAllocatorPools() = 0;
    virtual void UpdateThreadLocals() = 0;
    virtual size_t VerifyHeap() = 0;
    NativeGcTriggerType GetNativeGcTriggerType();

    volatile std::atomic<GCPhase> phase_ {GCPhase::GC_PHASE_IDLE};
    GCType gc_type_ {GCType::INVALID_GC};
    GCSettings gc_settings_;
    PandaVector<GCListener *> *gc_listeners_ptr_ {nullptr};
    GCBarrierSet *gc_barrier_set_ {nullptr};
    ObjectAllocatorBase *object_allocator_ {nullptr};
    InternalAllocatorPtr internal_allocator_ {nullptr};
    GCInstanceStats instance_stats_;

    // Additional NativeGC
    std::atomic<size_t> native_bytes_registered_ = 0;
    std::atomic<size_t> native_objects_notified_ = 0;

    ReferenceProcessor *reference_processor_ {nullptr};
    std::atomic_bool allow_soft_reference_processing_ = false;

    GCQueueInterface *gc_queue_ = nullptr;
    std::thread *worker_ = nullptr;
    std::atomic_bool gc_running_ = false;
    std::atomic<bool> can_add_gc_task_ = true;
    bool tlabs_supported_ = false;

    // Additional data for extensions
    GCExtensionData *extension_data_ {nullptr};

    GCWorkersThreadPool *workers_pool_ {nullptr};
    class PostForkGCTask;

    friend class ecmascript::EcmaReferenceProcessor;
    friend class panda::mem::test::MemStatsGenGCTest;
    friend class panda::mem::test::ReferenceStorageTest;
    friend class panda::mem::test::RemSetTest;
    friend class GCScopedPhase;
    friend class GlobalObjectStorage;
    // TODO(maksenov): Avoid using specific ObjectHelpers class here
    friend class GCDynamicObjectHelpers;
    friend class GCStaticObjectHelpers;
    friend class G1GCTest;
    friend class GCTestLog;

    void TriggerGCForNative();
    size_t SimpleNativeAllocationGcWatermark();
    /**
     * Waits while current GC task(if any) will be processed
     */
    void WaitForIdleGC() NO_THREAD_SAFETY_ANALYSIS;

    friend class GCScopedPhase;
    friend class ConcurrentScope;

    PandaVM *vm_ {nullptr};
    std::atomic<bool> is_full_gc_ {false};
};

// TODO(dtrubenkov): move configs in more appropriate place
template <MTModeT MTMode>
class AllocConfig<GCType::STW_GC, MTMode> {
public:
    using ObjectAllocatorType = ObjectAllocatorNoGen<MTMode>;
    using CodeAllocatorType = CodeAllocator;
};

template <MTModeT MTMode>
class AllocConfig<GCType::EPSILON_GC, MTMode> {
public:
    using ObjectAllocatorType = ObjectAllocatorNoGen<MTMode>;
    using CodeAllocatorType = CodeAllocator;
};

template <MTModeT MTMode>
class AllocConfig<GCType::GEN_GC, MTMode> {
public:
    using ObjectAllocatorType = ObjectAllocatorGen<MTMode>;
    using CodeAllocatorType = CodeAllocator;
};

template <MTModeT MTMode>
class AllocConfig<GCType::HYBRID_GC, MTMode> {
public:
    using ObjectAllocatorType = HybridObjectAllocator;
    using CodeAllocatorType = CodeAllocator;
};

/**
 * \brief Create GC with \param gc_type
 * @param gc_type - type of create GC
 * @return pointer to created GC on success, nullptr on failure
 */
template <class LanguageConfig>
GC *CreateGC(GCType gc_type, ObjectAllocatorBase *object_allocator, const GCSettings &settings);

/**
 * Enable concurrent mode. Should be used only from STW code.
 */
class ConcurrentScope final {
public:
    explicit ConcurrentScope(GC *gc, bool auto_start = true);
    NO_COPY_SEMANTIC(ConcurrentScope);
    NO_MOVE_SEMANTIC(ConcurrentScope);
    ~ConcurrentScope();
    void Start();

private:
    GC *gc_;
    bool started_ = false;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_HMA
