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
#include "common_components/mutator/mutator_manager.h"  // STWParam
#include "common_components/heap/allocator/regional_heap.h"

#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/gc/cmc/cmc-allocator-adapter.h"
#include "runtime/mem/gc/cmc/common_components/mutator/mutator_manager.h"
#include "runtime/include/managed_thread.h"

namespace ark::common_vm {
class Allocator;
class CollectorResources;
}  // namespace ark::common_vm

namespace ark::mem {

template <MTModeT MT_MODE>
class AllocConfig<GCType::CMC_GC, MT_MODE> {
public:
    using ObjectAllocatorType = CMCObjectAllocatorAdapter<MT_MODE>;
    using CodeAllocatorType = CodeAllocator;
};

template <typename T>
using CArrayList = std::vector<T>;

template <typename StackType>
class GlobalStackQueue;

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

template <class LanguageConfig>
class MarkingWork;
template <class LanguageConfig, bool ProcessXRef>
class ConcurrentMarkingWork;
constexpr size_t LOCAL_MARK_STACK_CAPACITY = 128;
using GlobalMarkStack = ark::common_vm::StackList<BaseObject, LOCAL_MARK_STACK_CAPACITY>;
using ParallelLocalMarkStack =
    ark::common_vm::LocalStack<BaseObject, LOCAL_MARK_STACK_CAPACITY, ParallelMarkingMonitor>;
using SequentialLocalMarkStack = ark::common_vm::LocalStack<BaseObject, LOCAL_MARK_STACK_CAPACITY>;
using LocalCollectStack = ark::common_vm::LocalStack<BaseObject, LOCAL_MARK_STACK_CAPACITY>;

using WeakStack = CArrayList<std::pair<RefField<> *, size_t>>;

constexpr size_t LOCAL_EVACUATION_STACK_CAPACITY = 128;
using GlobalEvacuationStack = ark::common_vm::StackList<RefField<>, LOCAL_EVACUATION_STACK_CAPACITY>;
using LocalEvacuationStack = ark::common_vm::LocalStack<RefField<>, LOCAL_EVACUATION_STACK_CAPACITY>;
using ParallelLocalEvacuationStack =
    ark::common_vm::LocalStack<RefField<>, LOCAL_MARK_STACK_CAPACITY, ParallelMarkingMonitor>;

enum class GCMode : uint8_t { CMC = 0, CONCURRENT_MARK = 1, STW = 2 };

template <class LanguageConfig>
class CmcGC : public GCLang<LanguageConfig>, public ark::common_vm::Collector {
    friend class MarkingWork<LanguageConfig>;
    template <class LC, bool ProcessXRef>
    friend class ConcurrentMarkingWork;

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

    void StartGC() override {}
    void StopGC() override;

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

    virtual void PreGarbageCollection(bool isConcurrent);
    virtual void PostGarbageCollection(uint64_t gcIndex);

    void Init() override
    {
        HeapBitmapManager::GetHeapBitmapManager().InitializeHeapBitmap();
        DCHECK(GetGCPhase() == ark::common_vm::GC_PHASE_IDLE);
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
        common_vm::MutatorManager::Create();
    }

    void Fini() override
    {
        common_vm::MutatorManager::Destroy();
        HeapBitmapManager::GetHeapBitmapManager().DestroyHeapBitmap();
    }

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    void DumpRoots(LogType logType);
    void DumpHeap(const CString &tag);
    void DumpBeforeGC()
    {
        if (ENABLE_LOG(FRAGMENT)) {
            if (common_vm::MutatorManager::Instance().WorldStopped()) {
                DumpHeap("before_gc");
            } else {
                STWParam stwParam {"dump-heap-before-gc"};
                ScopedStopTheWorld stw(stwParam);
                DumpHeap("before_gc");
            }
        }
    }

    void DumpAfterGC()
    {
        if (ENABLE_LOG(FRAGMENT)) {
            if (common_vm::MutatorManager::Instance().WorldStopped()) {
                DumpHeap("after_gc");
            } else {
                STWParam stwParam {"dump-heap-after-gc"};
                ScopedStopTheWorld stw(stwParam);
                DumpHeap("after_gc");
            }
        }
    }
#endif

    bool ShouldIgnoreRequest(ark::common_vm::GCRequest &request) override;
    bool MarkObject(BaseObject *obj) const;

    template <bool ProcessXRef>
    void ProcessMarkStack(uint32_t threadIndex, ParallelLocalMarkStack &workStack);
    void ProcessWeakStack(WeakStack &weakStack);

    // live but not resurrected object.
    bool IsMarkedObject(const BaseObject *obj) const;

    // live or resurrected object.
    bool IsToObject(const BaseObject *obj) const;

    // avoid std::function allocation for each object marking
    class MarkingRefFieldVisitor {
    public:
        MarkingRefFieldVisitor() : closure_(std::make_shared<BaseObject *>(nullptr)) {}

        template <typename Functor>
        void SetVisitor(Functor &&f)
        {
            visitor_ = std::forward<Functor>(f);
        }

        template <typename Functor>
        void SetWeakVisitor(Functor &&f)
        {
            weakVisitor_ = std::forward<Functor>(f);
        }

        const auto &GetRefFieldVisitor() const
        {
            return visitor_;
        }
        const auto &GetWeakRefFieldVisitor() const
        {
            return weakVisitor_;
        }
        void SetMarkingRefFieldArgs(BaseObject *obj)
        {
            *closure_ = obj;
        }
        const auto &GetClosure() const
        {
            return closure_;
        }

    private:
        ark::mem::RefFieldVisitor visitor_;
        ark::mem::RefFieldVisitor weakVisitor_;
        std::shared_ptr<BaseObject *> closure_;
    };
    MarkingRefFieldVisitor CreateMarkingObjectRefFieldsVisitor(ParallelLocalMarkStack &workStack, WeakStack &weakStack);
#ifdef PANDA_JS_ETS_HYBRID_MODE
    void MarkingXRef(RefField<> &ref, ParallelLocalMarkStack &workStack) const;
    void MarkingObjectXRef(BaseObject *obj, ParallelLocalMarkStack &workStack);
#endif

    void FixObjectRefFields(BaseObject *obj) const override;
    void FixRefField(BaseObject *obj, RefField<> &field) const;

    BaseObject *ForwardObject(BaseObject *fromVersion) override;

    bool IsFromObject(BaseObject *obj) const override
    {
        // filter const string object.
        if (Heap::IsHeapAddress(obj)) {
            RegionDesc::InlinedRegionMetaData *objMetaRegion =
                RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(obj));
            return objMetaRegion->IsFromRegion();
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

    void RunGarbageCollection(uint64_t, ark::common_vm::GCReason, ark::common_vm::GCType) override;

    void ReclaimGarbageMemory(ark::common_vm::GCReason reason);

    void TransitionToGCPhase(const ark::common_vm::GCPhase phase)
    {
        ASSERT(ark::common_vm::MutatorManager::Instance().WorldStopped());
        using ark::common_vm::GCListener;
        const auto currentPhase = Heap::GetHeap().GetGCPhase();
        NotifyGCListeners([currentPhase](GCListener *l) { l->OnGCPhaseEnd(currentPhase); });
        SetGCPhase(phase);
        LOG(DEBUG, GC) << "transition gc phase: " << Collector::GetGCPhaseName(currentPhase) << "(" << currentPhase
                       << ") -> " << Collector::GetGCPhaseName(phase) << "(" << phase << ")";
        ark::common_vm::MutatorManager::Instance().VisitAllMutators([this, phase](ark::common_vm::Mutator &mutator) {
            mutator.HandleGCPhase(phase);
            mutator.SetMutatorPhase(phase);
            UpdateBarrierEntrypoint(&mutator, phase);
        });
        NotifyGCListeners([phase](GCListener *l) { l->OnGCPhaseStart(phase); });
    }

    ark::common_vm::GCStats &GetGCStats() override;

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

    BaseObject *CopyObjectImpl(BaseObject *obj);
    BaseObject *CopyObjectAfterExclusive(BaseObject *obj);

    BaseObject *TryForwardObject(BaseObject *fromVersion);

    bool TryUpdateRefField(BaseObject *obj, RefField<> &field, BaseObject *&newRef) const override;
    bool TryForwardRefField(BaseObject *obj, RefField<> &field, BaseObject *&newRef) const override;

    void ProcessEvacuationStack(GlobalEvacuationStack &globalStack);
    void ProcessEvacuationStack(ParallelLocalEvacuationStack &markStack);

    void RemarkYoungCollectionSpace(GlobalEvacuationStack &globalStack);
    void MarkEvacuationStack(GlobalEvacuationStack &globalStack);
    void MarkEvacuationStack(ParallelLocalEvacuationStack &markStack);

    static const size_t MAX_MARKING_WORK_SIZE;
    static const size_t MIN_MARKING_WORK_SIZE;

    void CopyObject(const BaseObject &fromObj, BaseObject &toObj, size_t size) const;

    void AddGCListener(ark::common_vm::GCListener *listener) override
    {
        ark::os::memory::LockHolder gcListenersLock(gcListenersLock_);
        gcListeners_.push_back(listener);
    }

    void RemoveGCListener(ark::common_vm::GCListener *listener) override
    {
        ark::os::memory::LockHolder gcListenersLock(gcListenersLock_);
        gcListeners_.erase(std::remove(gcListeners_.begin(), gcListeners_.end(), listener), gcListeners_.end());
    }

    void NotifyGCListeners(std::function<void(ark::common_vm::GCListener *)> notifier) override
    {
        ark::os::memory::LockHolder gcListenersLock(gcListenersLock_);
        std::for_each(gcListeners_.begin(), gcListeners_.end(), notifier);
    }

    ark::common_vm::GCPhase GetGCPhase() const override
    {
        // Atomic with acquire order reason: data race with gcPhase_ with dependecies on reads after the load
        return gcPhase_.load(std::memory_order_acquire);
    }

    void SetGCPhase(const ark::common_vm::GCPhase phase) override
    {
        // Atomic with release order reason: data race with gcPhase_ with dependecies on writes before the store
        gcPhase_.store(phase, std::memory_order_release);
    }

    void UpdateBarrierEntrypoint(ark::common_vm::Mutator *mutator, ark::common_vm::GCPhase phase);
    void OnMutatorTerminate(Mutator *mutator, MutatorUnregistrationMode mode,
                            mem::BuffersKeepingFlag keepBuffers) override;
    void OnMutatorCreate(Mutator *mutator) override;

protected:
    void CollectLargeGarbage()
    {
        COMMON_PHASE_TIMER("Collect large garbage");
        auto &space = reinterpret_cast<ark::common_vm::RegionalHeap &>(theAllocator_);
        auto &stats = GetGCStats();
        stats.largeSpaceSize = space.LargeObjectSize();
        stats.largeGarbageSize = space.CollectLargeGarbage();
        stats.collectedBytes += stats.largeGarbageSize;
    }

    void CollectNonMovableGarbage()
    {
        auto &space = reinterpret_cast<ark::common_vm::RegionalHeap &>(theAllocator_);
        auto &stats = GetGCStats();
        stats.nonMovableSpaceSize = space.NonMovableSpaceSize();
        stats.collectedBytes += stats.nonMovableGarbageSize;
    }

    void CollectSmallSpace();
    void ClearAllGCInfo();

    void DoGarbageCollection();

    void ProcessFinalizers();

    void CopyFromSpace();
    void ExemptFromSpace();

    void RequestGCInternal(ark::common_vm::GCReason reason, bool async, ark::common_vm::GCType gcType,
                           bool explicitRequest = false) override;
    void MergeWeakStack(WeakStack &weakStack);
    void UpdateNativeThreshold(ark::mem::GCParam &gcParam);

    void UpdateGCCompletionStats(ark::common_vm::GCStats &gcStats);

    void MarkObject(ark::ObjectHeader *object) override;
    bool IsMarked(const ark::ObjectHeader *object) const override;
    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void MarkReferences(ark::mem::GCMarkingStackType *references, ark::mem::GCPhase gcPhase) override;

    ark::common_vm::Allocator &theAllocator_;

    // A collectorResources provides the resources that the tracing collector need,
    // such as gc thread/threadPool, gc task queue.
    // Also provides the resource access interfaces, such as invokeGC, waitGC.
    // This resource should be singleton and shared for multi-collectors
    ark::common_vm::CollectorResources &collectorResources_;

    WeakStack globalWeakStack_;
    ark::os::memory::Mutex weakStackLock_;

    uint32_t snapshotFinalizerNum_ = 0;

    // reason for current GC.
    ark::common_vm::GCReason gcReason_ = ark::common_vm::GC_REASON_USER;

    ark::common_vm::GCType gcType_ = ark::common_vm::GC_TYPE_FULL;

    // indicate whether to fix references (including global roots and reference fields).
    // this member field is useful for optimizing concurrent copying gc.
    bool fixReferences_ = false;

    std::atomic<size_t> markedObjectCount_ = {0};

    void ResetBitmap(bool heapMarked)
    {
        // if heap is marked and tracing result will be used during next gc, we should not reset liveInfo_.
    }

    uint32_t GetGCThreadCount(const bool isConcurrent) const;

    inline void SetGCReason(const ark::common_vm::GCReason reason)
    {
        gcReason_ = reason;
    }

    ark::common_vm::Taskpool *GetThreadPool() const;

    // let finalizerProcessor process finalizers, and mark resurrected if in stw gc
    void ClearWeakStack(bool parallel);

    void RemarkAndPreforwardStaticRoots(GlobalMarkStack &globalMarkStack);
    void PreforwardStaticRoots();

    bool PushRootToWorkStack(LocalCollectStack &markStack, BaseObject *obj);
    void PushRootsToWorkStack(LocalCollectStack &markStack, const CArrayList<BaseObject *> &collectedRoots);
    void MarkingRoots(const CArrayList<BaseObject *> &collectedRoots);
    void Remark();

    bool MarkSatbBuffer(GlobalMarkStack &globalMarkStack);

    // concurrent marking.
    void TracingImpl(GlobalMarkStack &globalMarkStack, bool parallel, bool Remark);
    void TracingSerial(GlobalMarkStack &globalMarkStack);

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

    void MarkRememberSetImpl(BaseObject *object, LocalCollectStack &markStack);
    void ConcurrentRemark(GlobalMarkStack &globalMarkStack, bool parallel);
    void MarkAwaitingJitFort();

    template <bool copy>
    bool TryUpdateRefFieldImpl(BaseObject *obj, RefField<> &ref, BaseObject *&oldRef, BaseObject *&newRef) const;

    enum class EnumRootsPolicy {
        NO_STW_AND_NO_FLIP_MUTATOR,
        STW_AND_NO_FLIP_MUTATOR,
        STW_AND_FLIP_MUTATOR,
    };

    template <EnumRootsPolicy policy>
    CArrayList<BaseObject *> EnumRoots();

    template <void (&rootsVisitFunc)(const ark::mem::RefFieldVisitor &)>
    void EnumRootsImpl(const ark::mem::RefFieldVisitor &visitor)
    {
        // assemble garbage candidates.
        reinterpret_cast<ark::common_vm::RegionalHeap &>(theAllocator_).AssembleGarbageCandidates();
        reinterpret_cast<ark::common_vm::RegionalHeap &>(theAllocator_).PrepareMarking();

        COMMON_PHASE_TIMER("enum roots & update old pointers within");
        TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_ENUM);

        rootsVisitFunc(visitor);
    }
    CArrayList<CArrayList<BaseObject *>> EnumRootsFlip(ark::common_vm::STWParam &param,
                                                       const ark::mem::RefFieldVisitor &visitor);

    void MarkingHeap(const CArrayList<BaseObject *> &collectedRoots);
    void PostMarking();
    void ParallelRemarkAndPreforward(GlobalMarkStack &globalMarkStack);
    void Preforward();
    void ConcurrentPreforward();

    void PreforwardConcurrentRoots();
    void PreforwardStaticWeakRoots();
    void PreforwardConcurrencyModelRoots();

    void PrepareFix();
    void ParallelFixHeap();
    void FixHeap();  // roots and ref-fields
    WeakRefFieldVisitor GetWeakRefFieldVisitor();
    RefFieldVisitor GetPrefowardRefFieldVisitor();
    void PreforwardFlip();

    void CollectGarbageWithXRef();

    template <EnumRootsPolicy policy>
    void PreforwardNonHeapRoots(GlobalEvacuationStack &globalStack);
    template <void (&rootsVisitFunc)(const ark::mem::RefFieldVisitor &)>
    void PreforwardNonHeapRootsImpl(CArrayList<BaseObject *> &forwardedRoots);
    void PreforwardNonHeapRoot(RefField<> &root, CArrayList<BaseObject *> &forwardedRoots);
    void PreforwardNonHeapRootsFlip(CArrayList<BaseObject *> &forwardedRoots);
    void RemarkNonHeapRoots(GlobalEvacuationStack &globalStack);
    void EnqueueRememberedSetRefs(GlobalEvacuationStack &globalStack);
    void MarkSatbBuffer(GlobalEvacuationStack &globalStack);
    template <typename Stack>
    void EnqueueRefsToYoungCollectionSpace(BaseObject *obj, Stack &stack);
    template <typename Stack>
    size_t EnqueueRefsToYoungCollectionSpaceAndGetSize(BaseObject *obj, Stack &stack);
    bool InYoungCollectionSpace(const RegionDesc::InlinedRegionMetaData *region) const;
    bool InYoungCollectionSpace(const RegionDesc *region) const;
    bool InYoungCollectionSpace(const BaseObject *obj) const;
    bool InYoungCollectionSpace(RegionDesc::RegionType type) const;
    void DoGarbageCollectionWithoutConcurrentMarking();

    template <typename Stack>
    void RemarkNonHeapRoot(RefField<> &ref, Stack &stack);

    template <typename Stack>
    void ProcessRef(RefField<> &ref, Stack &stack);

    template <typename Stack>
    void MarkRef(RefField<> &ref, Stack &stack);

    void ProcessReferencesAfterCopy();

    GCMode gcMode_ = GCMode::CMC;
    std::atomic<ark::common_vm::GCPhase> gcPhase_ = {ark::common_vm::GCPhase::GC_PHASE_IDLE};
    std::vector<ark::common_vm::GCListener *> gcListeners_;
    ark::os::memory::Mutex gcListenersLock_;
};

template <typename StackType>
class GlobalStackQueue {
public:
    GlobalStackQueue() = default;
    ~GlobalStackQueue() = default;

    void AddWorkStack(StackType &&stack)
    {
        DCHECK(!stack.empty());
        ark::os::memory::LockHolder guard(mtx_);
        stacks_.push_back(std::move(stack));
        cv_.Signal();
    }

    StackType PopWorkStack()
    {
        ark::os::memory::LockHolder lock(mtx_);
        while (true) {
            if (!stacks_.empty()) {
                StackType stack(std::move(stacks_.back()));
                stacks_.pop_back();
                return stack;
            }
            if (finished_) {
                return StackType();
            }
            cv_.Wait(&mtx_);
        }
    }

    StackType DrainAllWorkStack()
    {
        ark::os::memory::LockHolder lock(mtx_);
        while (!stacks_.empty()) {
            StackType stack(std::move(stacks_.back()));
            stacks_.pop_back();
            return stack;
        }
        return StackType();
    }

    void NotifyFinish()
    {
        ark::os::memory::LockHolder guard(mtx_);
        DCHECK(!finished_);
        finished_ = true;
        cv_.SignalAll();
    }

private:
    bool finished_ {false};
    ark::os::memory::ConditionVariable cv_;
    ark::os::memory::Mutex mtx_;
    std::vector<StackType> stacks_;
};

}  // namespace ark::mem

#endif  // if defined(ARK_USE_COMMON_RUNTIME)
#endif  // PANDA_RUNTIME_MEM_GC_CMC_GC_CMC_GC_H
