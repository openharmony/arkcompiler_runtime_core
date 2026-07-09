/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_MEM_GC_CMC_GC_CMC_GC_H
#define PANDA_RUNTIME_MEM_GC_CMC_GC_CMC_GC_H

#include "runtime/include/mem/panda_containers.h"
#if defined(ARK_USE_COMMON_RUNTIME)

#include <atomic>
#include <cstdint>

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/heap/region_desc.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/taskpool/runner.h"
#include "common_components/taskpool/taskpool.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/regional_heap.h"

#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/gc/cmc/cmc-allocator.h"
#include "runtime/include/gc_task.h"

namespace ark::common_vm {
class Allocator;
class CollectorResources;

}  // namespace ark::common_vm

namespace ark::mem {

template <MTModeT MT_MODE>
class AllocConfig<GCType::CMC_GC, MT_MODE> {
public:
    using ObjectAllocatorType = CMCObjectAllocator;
    using CodeAllocatorType = CodeAllocator;
};

template <typename T>
using CArrayList = PandaVector<T>;

// prefetch distance for mark.
#define MACRO_MARK_PREFETCH_DISTANCE 16     // this macro is used for check when pre-compiling.
constexpr int MARK_PREFETCH_DISTANCE = 16;  // when it is changed, remember to change MACRO_MARK_PREFETCH_DISTANCE.

// Small queue implementation, for prefetching.
#define COMMON_MAX_PREFETCH_QUEUE_SIZE_LOG 5UL
#define COMMON_MAX_PREFETCH_QUEUE_SIZE (1UL << COMMON_MAX_PREFETCH_QUEUE_SIZE_LOG)
#if COMMON_MAX_PREFETCH_QUEUE_SIZE <= MACRO_MARK_PREFETCH_DISTANCE
#error Prefetch queue size must be strictly greater than prefetch distance.
#endif

template <class LanguageConfig>
class CmcGC : public GCLang<LanguageConfig>, public ark::common_vm::Collector {
    template <class ElementType, GCWorkersTaskTypes... ALLOWED_TASKS>
    class CmcGCAdaptiveStack;

    template <class ElementType, GCWorkersTaskTypes... ALLOWED_TASKS>
    class CmcGCStackTask : public GCWorkersTask {
    public:
        using StackType = CmcGCAdaptiveStack<ElementType, ALLOWED_TASKS...>;

        CmcGCStackTask(GCWorkersTaskTypes type, StackType *stack) : GCWorkersTask(type, stack)
        {
            ASSERT(((type == ALLOWED_TASKS) || ...));
        }

        StackType *GetStack() const
        {
            return static_cast<StackType *>(storage_);
        }
    };

    template <class ElementType, GCWorkersTaskTypes... ALLOWED_TASKS>
    class CmcGCAdaptiveStack : public GCAdaptiveStack<ElementType> {
        using Base = GCAdaptiveStack<ElementType>;

    public:
        using TaskType = CmcGCStackTask<ElementType, ALLOWED_TASKS...>;
        using Base::Base;

        CmcGCAdaptiveStack *CreateStack() override
        {
            auto *gc = Base::GetGC();
            return gc->GetInternalAllocator()->template New<CmcGCAdaptiveStack>(
                gc, Base::GetNewTaskStackSizeLimit(), Base::GetNewTaskStackSizeLimit(), Base::GetTaskType(),
                Base::GetTimeLimitForNewTaskCreation(), Base::GetStackDst());
        }

        GCWorkersTask CreateTask(Base *stack) override
        {
            return TaskType(Base::GetTaskType(), static_cast<CmcGCAdaptiveStack *>(stack));
        }
    };

    using CmcGCEvacuationStack = CmcGCAdaptiveStack<ObjectPointerType *, GCWorkersTaskTypes::TASK_EVACUATE_REGIONS>;
    using CmcGCEvacuationTask = typename CmcGCEvacuationStack::TaskType;

    using CmcGCMarkingStack =
        CmcGCAdaptiveStack<ObjectHeader *, GCWorkersTaskTypes::TASK_REMARK, GCWorkersTaskTypes::TASK_FULL_MARK>;
    using CmcGCMarkingTask = typename CmcGCMarkingStack::TaskType;

    struct FixHeapWorkerData {
        common_vm::FixHeapTaskList *taskList {nullptr};
        std::atomic<size_t> *taskIter {nullptr};
        common_vm::FixRegionResult *result {nullptr};
    };

    class CmcGCFixTask : public GCWorkersTask {
    public:
        explicit CmcGCFixTask(FixHeapWorkerData *data) : GCWorkersTask(GCWorkersTaskTypes::TASK_CONCURRENT_FIX, data)
        {
            ASSERT(data != nullptr);
        }

        FixHeapWorkerData *GetData() const
        {
            return static_cast<FixHeapWorkerData *>(storage_);
        }
    };

    class CmcGCPostFixTask : public GCWorkersTask {
    public:
        explicit CmcGCPostFixTask(common_vm::FixRegionResult *result)
            : GCWorkersTask(GCWorkersTaskTypes::TASK_CONCURRENT_POST_FIX, result)
        {
            ASSERT(result != nullptr);
        }

        common_vm::FixRegionResult *GetResult() const
        {
            return static_cast<common_vm::FixRegionResult *>(storage_);
        }
    };

public:
    explicit CmcGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(CmcGC);
    NO_MOVE_SEMANTIC(CmcGC);
    ~CmcGC() override
    {
        Fini();
    }

    void RunPhases(GCTask &task);

    bool WaitForGC(GCTask task) override;

    bool IsPinningSupported() const final
    {
        return false;
    }

    void InitGCBits(ark::ObjectHeader *objHeader) override;

    void InitGCBitsForAllocationInTLAB(ark::ObjectHeader *objHeader) override;

    bool Trigger(ark::PandaUniquePtr<GCTask> task) override;

    bool IsPostponeGCSupported() const override;

    void EnableReadBarrier(ark::Mutator *thread)
    {
        UpdateReadBarrierEntrypointInMutator<true>(thread);
    }

    void DisableReadBarrier(ark::Mutator *thread)
    {
        UpdateReadBarrierEntrypointInMutator<false>(thread);
    }

    void EnablePreWriteBarrier(ark::Mutator *thread)
    {
        UpdatePreWriteBarrierEntrypointInMutator<true>(thread);
    }

    void DisablePreWriteBarrier(ark::Mutator *thread)
    {
        UpdatePreWriteBarrierEntrypointInMutator<false>(thread);
    }

    virtual void PreGarbageCollection(GCTaskCause reason, bool isConcurrent);
    virtual void PostGarbageCollection(uint64_t gcIndex);

    void Init() override
    {
        HeapBitmapManager::GetHeapBitmapManager().InitializeHeapBitmap();
        DCHECK(this->GetGCPhase() == mem::GCPhase::GC_PHASE_IDLE);
    }

    void Fini() override
    {
        HeapBitmapManager::GetHeapBitmapManager().DestroyHeapBitmap();
    };

    void ProcessMarkStack(CmcGCMarkingStack &stack);

    void HandleMarkedObject(ObjectHeader *object, CmcGCMarkingStack &stack);

    // live but not resurrected object.
    bool IsMarkedObject(const BaseObject *obj) const;

    // live or resurrected object.
    bool IsToObject(const BaseObject *obj) const;

    void FixObjectRefFields(BaseObject *obj) const;
    static void FixRefField(ObjectHeader *obj, ObjectPointerType *field);

    ObjectHeader *ForwardObject(ObjectHeader *fromVersion) override;

    bool IsFromObject(ObjectHeader *obj) const override
    {
        // filter const string object.
        if (Heap::IsHeapAddress(obj)) {
            return RegionDesc::GetRegionDescAt(obj)->IsFromRegion();
        }

        return false;
    }

    bool IsUnmovableFromObject(BaseObject *obj) const override;

    ark::common_vm::Allocator &GetAllocator() const
    {
        return theAllocator_;
    }

    bool IsHeapMarked() const;
    void SetHeapMarked(bool value);
    void SetGcStarted(bool val);
    void MarkGCStart();
    void MarkGCFinish(uint64_t gcIndex);

    void RunGarbageCollection(uint64_t, ark::GCTask &) override;

    void ReclaimGarbageMemory(GCTaskCause reason);

    void TransitionToGCPhase(const GCPhase phase)
    {
        const auto currentPhase = this->GetGCPhase();
        this->FireGCPhaseFinished(currentPhase);
        this->SetGCPhase(phase);
        LOG(DEBUG, GC) << "transition gc phase: " << GCScopedPhase::GetPhaseName(currentPhase) << "("
                       << static_cast<uint8_t>(currentPhase) << ") -> " << GCScopedPhase::GetPhaseName(phase) << "("
                       << static_cast<uint8_t>(phase) << ")";
        ForEachManagedMutator([this, phase](Mutator *mutator) {
            mutator->HandleGCPhase(phase);
            mutator->SetMutatorPhase(phase);
            UpdateBarrierEntrypoint(mutator, phase);
        });
        this->FireGCPhaseStarted(phase);
    }

    virtual void UpdateGCStats();

    // this is called when caller assures from-object/from-region still exists.
    BaseObject *GetForwardingPointer(ObjectHeader *fromObj)
    {
        return static_cast<BaseObject *>(fromObj)->GetForwardingPointer();
    }

    static BaseObject *FindToVersion(BaseObject *obj)
    {
        return obj->GetForwardingPointer();
    }

    void SetGCThreadQosPriority(ark::common_vm::PriorityMode mode);

    BaseObject *TryForwardObject(ObjectHeader *fromVersion);

    void ProcessEvacuationStack(CmcGCEvacuationStack &stack);

    void RemarkYoungCollectionSpace();
    void MarkEvacuationStack(CmcGCMarkingStack &stack);

    void MarkObject(ObjectHeader *object) override;
    bool MarkObjectIfNotMarked(ObjectHeader *object) override;
    bool IsMarked(const ark::ObjectHeader *object) const override;

    void CopyObject(const BaseObject &fromObj, BaseObject &toObj, size_t size) const;

    void UpdateBarrierEntrypoint(ark::common_vm::Mutator *mutator, mem::GCPhase phase);
    void OnMutatorTerminate(Mutator *mutator, MutatorUnregistrationMode mode,
                            mem::BuffersKeepingFlag keepBuffers) override;
    void OnMutatorCreate(Mutator *mutator) override;

protected:
    void CollectLargeGarbage()
    {
        mem::GCScope<mem::TRACE_TIMING> gcScope("Collect large garbage", this);
        auto &space = reinterpret_cast<ark::common_vm::RegionalHeap &>(theAllocator_);
        collectedBytes_ += space.CollectLargeGarbage();
    }

    void CollectNonMovableGarbage(size_t nonMovableGarbageSize)
    {
        collectedBytes_ += nonMovableGarbageSize;
    }

    void CollectSmallSpace();
    void ClearAllGCInfo();

    void DoGarbageCollection(ark::GCTask &task);

    void ProcessFinalizers();

    void CopyFromSpace();
    void ExemptFromSpace();

    void UpdateNativeThreshold(ark::mem::GCParam &gcParam);

    void UpdateGCCompletionStats();

    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void MarkReferences(ark::mem::GCMarkingStackType *references, ark::mem::GCPhase gcPhase) override;

    ark::common_vm::Allocator &theAllocator_;

    // A collectorResources provides the resources that the tracing collector need,
    // such as gc thread/threadPool, gc task queue.
    // Also provides the resource access interfaces, such as invokeGC, waitGC.
    // This resource should be singleton and shared for multi-collectors
    ark::common_vm::CollectorResources &collectorResources_;

    uint32_t snapshotFinalizerNum_ = 0;

    GCCollectionType gcType_ = GCCollectionType::FULL;

    // indicate whether to fix references (including global roots and reference fields).
    // this member field is useful for optimizing concurrent copying gc.
    bool fixReferences_ = false;

    std::atomic<size_t> markedObjectCount_ = {0};

    uint32_t GetGCThreadCount(const bool isConcurrent) const;

    ark::common_vm::Taskpool *GetThreadPool() const;

    // let finalizerProcessor process finalizers, and mark resurrected if in stw gc
    void ProcessWeakReferences(GCPhase phase, GCTaskCause reason);

    void PreforwardStaticRoots();

    bool IsWeakReference(ObjectHeader *obj);

    void HandleWeakReference(ObjectHeader *weakRef, CmcGCMarkingStack &stack);

    bool PushRootToWorkStack(CmcGCMarkingStack &markStack, ObjectHeader *obj, GCTaskCause reason);

    void PushRootsToWorkStack(CmcGCMarkingStack &markStack, const CArrayList<ObjectHeader *> &collectedRoots,
                              GCTaskCause reason);

    void MarkingRoots(const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason);

    void Remark(GCTaskCause reason);

private:
    template <bool ENABLE_BARRIER>
    void UpdateReadBarrierEntrypointInMutator(ark::Mutator *thread)
    {
        ark::mem::ObjFieldProcessFunc entrypointFunc = nullptr;

        if constexpr (ENABLE_BARRIER) {
            auto addr = this->GetBarrierSet()->GetBarrierOperand(ark::mem::BarrierPosition::BARRIER_POSITION_PRE,
                                                                 "CMC_READ_BARRIER");
            entrypointFunc = std::get<ark::mem::ObjFieldProcessFunc>(addr.GetValue());
        }

        void *entrypointFuncUntyped = reinterpret_cast<void *>(entrypointFunc);
        thread->SetReadBarrierEntrypoint(entrypointFuncUntyped);
    }

    template <bool ENABLE_BARRIER>
    void UpdatePreWriteBarrierEntrypointInMutator(ark::Mutator *thread)
    {
        ark::mem::ObjRefProcessFunc entrypointFunc = nullptr;

        if constexpr (ENABLE_BARRIER) {
            auto addr = this->GetBarrierSet()->GetBarrierOperand(ark::mem::BarrierPosition::BARRIER_POSITION_PRE,
                                                                 "PRE_CMC_WRITE_BARRIER");
            entrypointFunc = std::get<ark::mem::ObjRefProcessFunc>(addr.GetValue());
        }
        void *entrypointFuncUntyped = reinterpret_cast<void *>(entrypointFunc);
        thread->SetPreWrbEntrypoint(entrypointFuncUntyped);
    }

    ObjectHeader *CopyObjectImpl(ObjectHeader *object);
    ObjectHeader *CopyObjectAfterExclusive(ObjectHeader *object, MarkWord markWord);
    void UnlockObject(ObjectHeader *object);

    CArrayList<ObjectHeader *> EnumRoots();

    template <void (&rootsVisitFunc)(const GCRootVisitor &)>
    void EnumRootsImpl(const GCRootVisitor &visitor)
    {
        // assemble garbage candidates.
        reinterpret_cast<ark::common_vm::RegionalHeap &>(theAllocator_).AssembleGarbageCandidates();
        reinterpret_cast<ark::common_vm::RegionalHeap &>(theAllocator_).PrepareMarking();

        mem::GCScope<mem::TRACE_TIMING> gcScope("enum roots & update old pointers within", this);
        TransitionToGCPhase(mem::GCPhase::GC_PHASE_INITIAL_MARK);

        rootsVisitFunc(visitor);
    }
    void EnumRootsFlip(const GCRootVisitor &visitor);

    void MarkingHeap(const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason);
    void ConcurrentPreforward();

    void PreforwardConcurrentRoots();
    void PreforwardConcurrencyModelRoots();

    size_t ParallelFixHeap();
    size_t FixHeap();  // roots and ref-fields, returns reclaimed non-movable garbage
    void DispatchFixRegionTasks(FixHeapWorkerData *workerData);
    void DispatchFixRegionTask(const common_vm::FixTaskInfo &task, common_vm::FixRegionResult *result);
    void FixOldRegion(RegionDesc *region);
    void FixRecentOldRegion(RegionDesc *region);
    void FixToRegion(RegionDesc *region);
    enum class DeadObjectHandlerType {
        FILL_FREE,                    // Fill in free object immediately
        COLLECT_MONOSIZE_NONMOVABLE,  // Collect mono size non-movable objects (to be added to freelist)
        COLLECT_POLYSIZE_NONMOVABLE,  // Collect non-movable objects (to be filled free later)
        IGNORED,                      // Ignore dead objects
    };
    template <DeadObjectHandlerType HandlerType>
    void FixRegion(RegionDesc *task, common_vm::FixRegionResult *result);
    template <DeadObjectHandlerType HandlerType>
    void FixRecentRegion(RegionDesc *region, common_vm::FixRegionResult *result);

    void PostClearTask(common_vm::FixRegionResult *result);

    WeakRefFieldVisitor GetWeakRefFieldVisitor();
    void PreforwardFlip(GCTaskCause reason);

    void PreforwardNonHeapRoots(CmcGCEvacuationStack &stack);
    void PreforwardNonHeapRoot(GCRoot root, PandaVector<ObjectHeader *> &forwardedRoots);
    PandaVector<ObjectHeader *> PreforwardNonHeapRootsFlip();
    void RemarkNonHeapRoots(CmcGCMarkingStack &stack);
    void EnqueueRememberedSetRefs(CmcGCEvacuationStack &stack);
    void MarkSatbBufferYoung(CmcGCMarkingStack &stack);
    void MarkSatbBufferFull(CmcGCMarkingStack &stack);
    void EnqueueRefsToYoungCollectionSpace(ObjectHeader *obj, CmcGCEvacuationStack &stack);
    void EnqueueRefsToYoungCollectionSpace(ObjectHeader *obj, CmcGCMarkingStack &stack);
    static bool InYoungCollectionSpace(const RegionDesc *region);
    static bool InYoungCollectionSpace(const ObjectHeader *obj);
    static bool InYoungCollectionSpace(RegionDesc::RegionType type);
    void DoGarbageCollectionWithoutConcurrentMarking();

    void RemarkNonHeapRoot(GCRoot root, CmcGCMarkingStack &stack);

    void ProcessRef(ObjectPointerType *obj, CmcGCEvacuationStack &stack);
    void MarkRefYoung(ObjectHeader *obj, CmcGCMarkingStack &stack);
    static void MarkRefFull(ObjectHeader *obj, CmcGCMarkingStack &stack);

private:
    using ManagedMutatorCallback = std::function<void(Mutator *)>;
    void ForEachManagedMutator(const ManagedMutatorCallback &callback);

    static void VisitRoots(const GCRootVisitor &visitor);
    static void VisitSTWRoots(const GCRootVisitor &visitor);
    static void VisitConcurrentRoots(const GCRootVisitor &visitor);
    static void VisitWeakRoots(const WeakRefFieldVisitor &visitor);
    static void VisitGlobalRoots(const GCRootVisitor &visitor);
    static void VisitWeakGlobalRoots(const WeakRefFieldVisitor &visitor);

    void VisitRootsI(const RefFieldVisitor &visitor) override
    {
        auto rootVisitor = [&visitor](GCRoot root) { visitor(root.GetObjectPointer()); };
        CmcGC<LanguageConfig>::VisitRoots(rootVisitor);
    }

    void VisitSTWRootsI(const RefFieldVisitor &visitor) override
    {
        auto rootVisitor = [&visitor](GCRoot root) { visitor(root.GetObjectPointer()); };
        CmcGC<LanguageConfig>::VisitSTWRoots(rootVisitor);
    }

    void VisitConcurrentRootsI(const RefFieldVisitor &visitor) override
    {
        auto rootVisitor = [&visitor](GCRoot root) { visitor(root.GetObjectPointer()); };
        CmcGC<LanguageConfig>::VisitConcurrentRoots(rootVisitor);
    }

    void VisitWeakRootsI(const WeakRefFieldVisitor &visitor) override
    {
        CmcGC<LanguageConfig>::VisitWeakRoots(visitor);
    }

    void WorkerTaskProcessing(GCWorkersTask *task, void *workerData) override;

    bool IsYoungGC() const
    {
        return gcReason_ == GCTaskCause::YOUNG_GC_CAUSE;
    }

    GCTaskCause gcReason_ {GCTaskCause::INVALID_CAUSE};
    uint64_t gcStartTime_ {0};
    size_t targetFootprint_ {0};
    size_t fullGCCount_ {0};
    size_t collectedBytes_ {0};
    double fullGCMeanRate_ {0.0};
    CMCObjectAllocator *cmcAllocator_ {nullptr};
};
}  // namespace ark::mem

#endif  // if defined(ARK_USE_COMMON_RUNTIME)
#endif  // PANDA_RUNTIME_MEM_GC_CMC_GC_CMC_GC_H
