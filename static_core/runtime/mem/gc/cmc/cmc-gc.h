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

#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/heap/region_desc.h"
#include "common_components/taskpool/task.h"
#include "common_components/common/work_stack-inl.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/taskpool/runner.h"
#include "common_components/taskpool/taskpool.h"
#include "common_components/heap/allocator/regional_heap.h"

#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/gc/cmc/cmc-allocator.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/gc_task.h"
#include "runtime/mem/rendezvous.h"
#include "include/panda_vm.h"

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

class ParallelMarkingMonitor : public ark::common_vm::TaskPackMonitor {
public:
    explicit ParallelMarkingMonitor(int posted, int capacity) : TaskPackMonitor(posted, capacity) {}
    ~ParallelMarkingMonitor() override = default;

    void operator()()
    {
        WakeUpRunnerApproximately();
    }
};

constexpr size_t LOCAL_MARK_STACK_CAPACITY = 128;
using GlobalMarkStack = ark::common_vm::StackList<BaseObject, LOCAL_MARK_STACK_CAPACITY>;
using ParallelLocalMarkStack =
    ark::common_vm::LocalStack<BaseObject, LOCAL_MARK_STACK_CAPACITY, ParallelMarkingMonitor>;
using SequentialLocalMarkStack = ark::common_vm::LocalStack<BaseObject, LOCAL_MARK_STACK_CAPACITY>;
using LocalCollectStack = ark::common_vm::LocalStack<BaseObject, LOCAL_MARK_STACK_CAPACITY>;

enum class GCMode : uint8_t { CMC = 0, CONCURRENT_MARK = 1, STW = 2 };

template <class LanguageConfig>
class CmcGC : public GCLang<LanguageConfig>, public ark::common_vm::Collector {
    class CMCGCAdaptiveStack;

    class CMCGCWorkersTask : public GCWorkersTask {
    public:
        CMCGCWorkersTask(GCWorkersTaskTypes type, CMCGCAdaptiveStack *markingStack) : GCWorkersTask(type, markingStack)
        {
            ASSERT(type == GCWorkersTaskTypes::TASK_REMARK || type == GCWorkersTaskTypes::TASK_EVACUATE_REGIONS);
        }
        DEFAULT_COPY_SEMANTIC(CMCGCWorkersTask);
        DEFAULT_MOVE_SEMANTIC(CMCGCWorkersTask);
        ~CMCGCWorkersTask() = default;

        CMCGCAdaptiveStack *GetStack() const
        {
            return static_cast<CMCGCAdaptiveStack *>(storage_);
        }
    };

    class CMCGCAdaptiveStack : public GCAdaptiveStack<ObjectPointerType *> {
        using Ref = ObjectPointerType *;
        using GCAdaptiveStack<Ref>::GCAdaptiveStack;

    public:
        NO_COPY_SEMANTIC(CMCGCAdaptiveStack);
        NO_MOVE_SEMANTIC(CMCGCAdaptiveStack);

        ~CMCGCAdaptiveStack() = default;

        CMCGCAdaptiveStack *CreateStack() override
        {
            auto *gc = GCAdaptiveStack<Ref>::GetGC();
            auto allocator = gc->GetInternalAllocator();
            // New tasks will be created with the same new_task_stack_size_limit_ and stack_size_limit_
            return allocator->template New<CMCGCAdaptiveStack>(
                gc, GCAdaptiveStack<Ref>::GetNewTaskStackSizeLimit(), GCAdaptiveStack<Ref>::GetNewTaskStackSizeLimit(),
                GCAdaptiveStack<Ref>::GetTaskType(), GCAdaptiveStack<Ref>::GetTimeLimitForNewTaskCreation(),
                GCAdaptiveStack<Ref>::GetStackDst());
        }

        GCWorkersTask CreateTask(GCAdaptiveStack<Ref> *stack) override
        {
            return CMCGCWorkersTask(GCAdaptiveStack<Ref>::GetTaskType(), static_cast<CMCGCAdaptiveStack *>(stack));
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
#ifdef PANDA_TARGET_32
        // cmc is not adapted for 32-bit systems
        gcMode_ = GCMode::STW;
#else
        if (common_vm::Heap::GetHeap().GetGCParam().enableStwGC) {
            gcMode_ = GCMode::STW;
        } else {
            gcMode_ = GCMode::CMC;
        }
#endif

        // force STW for RB DFX
#ifdef ENABLE_CMC_RB_DFX
        gcMode_ = GCMode::STW;
#endif
    }

    void Fini() override
    {
        HeapBitmapManager::GetHeapBitmapManager().DestroyHeapBitmap();
    }

    template <bool ProcessXRef>
    void ProcessMarkStack(uint32_t threadIndex, ParallelLocalMarkStack &workStack, GCTaskCause reason);

    template <bool HANDLE_WEAK_REFS, typename ObjectMarkerT>
    void HandleMarkedObject(ObjectMarkerT *handler, BaseObject *object);

    // live but not resurrected object.
    bool IsMarkedObject(const BaseObject *obj) const;

    // live or resurrected object.
    bool IsToObject(const BaseObject *obj) const;

#ifdef PANDA_JS_ETS_HYBRID_MODE
    void MarkingXRef(RefField<> &ref, ParallelLocalMarkStack &workStack) const;
    void MarkingObjectXRef(BaseObject *obj, ParallelLocalMarkStack &workStack);
#endif

    void FixObjectRefFields(BaseObject *obj) const override;
    void FixRefField(BaseObject *obj, RefField<> &field) const;

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
    BaseObject *GetForwardingPointer(BaseObject *fromObj)
    {
        return fromObj->GetForwardingPointer();
    }

    BaseObject *FindToVersion(BaseObject *obj) const override
    {
        return obj->GetForwardingPointer();
    }

    void SetGCThreadQosPriority(ark::common_vm::PriorityMode mode);

    BaseObject *TryForwardObject(ObjectHeader *fromVersion);

    bool TryForwardRefField(BaseObject *obj, RefField<> &field, BaseObject *&newRef) const override;

    void ProcessEvacuationStack(CMCGCAdaptiveStack &markStack);

    void RemarkYoungCollectionSpace();
    void MarkEvacuationStack(CMCGCAdaptiveStack &objectsStack);

    void MarkObject(ObjectHeader *object) override;
    bool MarkObjectIfNotMarked(ObjectHeader *object) override;
    bool IsMarked(const ark::ObjectHeader *object) const override;

    static const size_t MAX_MARKING_WORK_SIZE;
    static const size_t MIN_MARKING_WORK_SIZE;

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

    bool IsWeakReference(BaseObject *obj);

    template <typename ObjectMarkerT>
    void HandleWeakReference(ObjectMarkerT *handler, BaseObject *weakRef);

    template <bool HANDLE_WEAK_REFS>
    bool PushRootToWorkStack(LocalCollectStack &markStack, ObjectHeader *obj, GCTaskCause reason);

    template <bool HANDLE_WEAK_REFS>
    void PushRootsToWorkStack(LocalCollectStack &markStack, const CArrayList<ObjectHeader *> &collectedRoots,
                              GCTaskCause reason);

    void MarkingRoots(const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason);

    void Remark(GCTaskCause reason);

    bool MarkSatbBuffer(GlobalMarkStack &globalMarkStack);

    // concurrent marking.
    void TracingImpl(GlobalMarkStack &globalMarkStack, bool parallel, bool Remark, GCTaskCause reason);
    void TracingSerial(GlobalMarkStack &globalMarkStack, GCTaskCause reason);

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
    void MarkRememberSetImpl(BaseObject *object, LocalCollectStack &markStack);
    void ConcurrentRemark(GlobalMarkStack &globalMarkStack, bool parallel);

    template <bool copy>
    bool TryUpdateRefFieldImpl(BaseObject *obj, RefField<> &ref, BaseObject *&oldRef, BaseObject *&newRef) const;

    enum class EnumRootsPolicy {
        NO_STW_AND_NO_FLIP_MUTATOR,
        STW_AND_NO_FLIP_MUTATOR,
        STW_AND_FLIP_MUTATOR,
    };

    template <EnumRootsPolicy policy>
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
    CArrayList<CArrayList<BaseObject *>> EnumRootsFlip(const GCRootVisitor &visitor);

    void MarkingHeap(const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason);
    void Preforward(GCTaskCause reason);
    void ConcurrentPreforward();

    void PreforwardConcurrentRoots();
    void PreforwardStaticWeakRoots(GCTaskCause reason);
    void PreforwardConcurrencyModelRoots();

    size_t ParallelFixHeap();
    size_t FixHeap(bool isWorldStopped);  // roots and ref-fields, returns reclaimed non-movable garbage
    WeakRefFieldVisitor GetWeakRefFieldVisitor(GCTaskCause reason);
    void PreforwardFlip(GCTaskCause reason);

    void CollectGarbageWithXRef(GCTaskCause reason);

    void PreforwardNonHeapRoots(CMCGCAdaptiveStack &stack);
    void PreforwardNonHeapRoot(GCRoot root, PandaVector<ObjectHeader *> &forwardedRoots);
    PandaVector<ObjectHeader *> PreforwardNonHeapRootsFlip();
    void RemarkNonHeapRoots(CMCGCAdaptiveStack &stack);
    void EnqueueRememberedSetRefs(CMCGCAdaptiveStack &stack);
    void MarkSatbBuffer(CMCGCAdaptiveStack &stack);
    void EnqueueRefsToYoungCollectionSpace(ObjectHeader *obj, CMCGCAdaptiveStack &stack);
    static bool InYoungCollectionSpace(const RegionDesc *region);
    static bool InYoungCollectionSpace(const BaseObject *obj);
    static bool InYoungCollectionSpace(RegionDesc::RegionType type);
    void DoGarbageCollectionWithoutConcurrentMarking();

    void RemarkNonHeapRoot(GCRoot root, CMCGCAdaptiveStack &stack);

    void ProcessRef(ObjectPointerType *obj, CMCGCAdaptiveStack &stack);
    void MarkRef(BaseObject *obj, CMCGCAdaptiveStack &stack);

    using ManagedMutatorCallback = std::function<void(Mutator *)>;
    void ForEachManagedMutator(const ManagedMutatorCallback &callback);

    static void VisitRoots(const GCRootVisitor &visitor);
    static void VisitSTWRoots(const RefFieldVisitor &visitor);
    static void VisitConcurrentRoots(const GCRootVisitor &visitor);
    static void VisitWeakRoots(const WeakRefFieldVisitor &visitor);
    static void VisitGlobalRoots(const GCRootVisitor &visitor);
    static void VisitWeakGlobalRoots(const WeakRefFieldVisitor &visitor, bool isYoung);

    void VisitRootsI(const RefFieldVisitor &visitor) override
    {
        auto rootVisitor = [&visitor](GCRoot root) {
            visitor(reinterpret_cast<RefField<> &>(*root.GetObjectPointer()));
        };
        CmcGC<LanguageConfig>::VisitRoots(rootVisitor);
    }

    void VisitSTWRootsI(const RefFieldVisitor &visitor) override
    {
        CmcGC<LanguageConfig>::VisitSTWRoots(visitor);
    }

    void VisitConcurrentRootsI(const RefFieldVisitor &visitor) override
    {
        auto rootVisitor = [&visitor](GCRoot root) {
            visitor(reinterpret_cast<RefField<> &>(*root.GetObjectPointer()));
        };
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

    GCMode gcMode_ = GCMode::CMC;

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
