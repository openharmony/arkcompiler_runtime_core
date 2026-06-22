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
#include "include/mem/panda_containers.h"
#if defined(ARK_USE_COMMON_RUNTIME)

#include <iomanip>

#include "runtime/mem/gc/cmc/cmc-gc.h"
#include "runtime/mem/gc/cmc/common_components/heap/heap_manager.h"
#include "runtime/mem/gc/gc_adaptive_stack_inl.h"
#include "runtime/mem/gc/gc_stats.h"

#include "common_components/base/globals.h"
#include "common_components/heap/verification.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/allocator/alloc_buffer.h"
#include "common_components/heap/collector/heuristic_gc_policy.h"
#include "common_components/heap/collector/utils.h"
#include "common_interfaces/base/runtime_param.h"
#include "common_components/heap/collector/collector_resources.h"
#include "common_components/mutator/satb_buffer.h"

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/math_helpers.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/trace.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/object-references-iterator-inl.h"

#include "common_interfaces/heap/region_desc.h"

#ifdef ENABLE_QOS
#include "qos.h"
#endif

namespace ark::common_vm {
void RemoveXRefFromRoots() {}
void SweepUnmarkedXRefs() {}
void AddXRefToRoots() {}
void UnmarkAllXRefs() {}
};  // namespace ark::common_vm

namespace ark::mem {
using ark::common_vm::AllocationBuffer;
using ark::common_vm::FixHeapTask;
using ark::common_vm::FixHeapWorker;
using ark::common_vm::FlipFunction;
using ark::common_vm::MB;
using ark::common_vm::PostFixHeapWorker;
using ark::common_vm::PriorityMode;
using ark::common_vm::RegionalHeap;
using ark::common_vm::SatbBuffer;
using ark::common_vm::Task;
using ark::common_vm::TaskPackMonitor;
using ark::common_vm::WVerify;

static BaseObject *ReadRefSlot(ObjectPointerType *ref)
{
    auto value = ObjectAccessor::Load(ref);
    return reinterpret_cast<BaseObject *>(static_cast<uintptr_t>(value));
}

static BaseObject *AtomicReadRefSlot(ObjectPointerType *ref)
{
    auto value = ObjectAccessor::LoadAtomic(ref);
    return reinterpret_cast<BaseObject *>(static_cast<uintptr_t>(value));
}

static bool CompareExchangeRefSlot(ObjectPointerType *ref, ObjectHeader *expected, ObjectHeader *desired)
{
    auto expectedValue = ToObjPtrType(expected);
    auto desiredValue = ToObjPtrType(desired);
    return AtomicCmpxchgStrong(ref, expectedValue, desiredValue, std::memory_order_relaxed) == expectedValue;
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsUnmovableFromObject(BaseObject *obj) const
{
    // filter const string object.
    if (!Heap::IsHeapAddress(obj)) {
        return false;
    }

    RegionDesc *regionInfo = nullptr;
    regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
    return regionInfo->IsUnmovableFromRegion();
}

// this api updates current pointer as well as old pointer, caller should take care of this.
template <class LanguageConfig>
template <bool copy>
bool CmcGC<LanguageConfig>::TryUpdateRefFieldImpl(BaseObject *obj, RefField<> &field, BaseObject *&fromObj,
                                                  BaseObject *&toObj) const
{
    RefField<> oldRef(field);
    fromObj = oldRef.GetTargetObject();
    if (IsFromObject(fromObj)) {  // LCOV_EXCL_BR_LINE
        if (copy) {               // LCOV_EXCL_BR_LINE
            toObj = const_cast<CmcGC<LanguageConfig> *>(this)->TryForwardObject(fromObj);
        } else {  // LCOV_EXCL_BR_LINE
            toObj = FindToVersion(fromObj);
        }
        if (toObj == nullptr) {  // LCOV_EXCL_BR_LINE
            return false;
        }
        RefField<> tmpField(toObj);
        if (field.CompareExchange(oldRef.GetFieldValue(), tmpField.GetFieldValue())) {  // LCOV_EXCL_BR_LINE
            if (obj != nullptr) {                                                       // LCOV_EXCL_BR_LINE
                LOG(DEBUG, GC) << "update obj " << obj << "<" << obj->GetTypeInfo() << ">(" << obj->GetSize() << ")+"
                               << BaseObject::FieldOffset(obj, &field) << " ref-field@" << &field << ": 0x" << std::hex
                               << oldRef.GetFieldValue() << " -> 0x" << tmpField.GetFieldValue() << std::dec;
                LOG(DEBUG, MM_OBJECT_EVENTS)
                    << "[CMC] "
                    << "BARRIER ref in obj " << obj << " field@" << &field << ": " << fromObj << " -> " << toObj;
            } else {  // LCOV_EXCL_BR_LINE
                LOG(DEBUG, GC) << "update ref@" << &field << ": 0x" << std::hex << oldRef.GetFieldValue() << " -> "
                               << std::dec << toObj;
                LOG(DEBUG, MM_OBJECT_EVENTS) << "[CMC] "
                                             << "BARRIER ref @" << &field << ": " << fromObj << " -> " << toObj;
            }
            return true;
        } else {                   // LCOV_EXCL_BR_LINE
            if (obj != nullptr) {  // LCOV_EXCL_BR_LINE
                LOG(DEBUG, GC) << "update obj " << obj << "<" << obj->GetTypeInfo() << ">(" << obj->GetSize() << ")+"
                               << BaseObject::FieldOffset(obj, &field) << " but cas failed ref-field@" << &field
                               << ": 0x" << std::hex << oldRef.GetFieldValue() << "(0x" << field.GetFieldValue()
                               << ") -> 0x" << tmpField.GetFieldValue() << std::dec << " but cas failed ";
            } else {  // LCOV_EXCL_BR_LINE
                LOG(DEBUG, GC) << "update but cas failed ref@" << &field << ": 0x" << std::hex << oldRef.GetFieldValue()
                               << "(" << field.GetFieldValue() << std::dec << ") -> " << toObj;
            }
            return true;
        }
    }

    return false;
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::TryForwardRefField(BaseObject *obj, RefField<> &field, BaseObject *&newRef) const
{
    BaseObject *oldRef = nullptr;
    return TryUpdateRefFieldImpl<true>(obj, field, oldRef, newRef);
}

template <bool PARALLEL>
class ObjectMarker {
public:
    using MarkingStack = std::conditional_t<PARALLEL, ParallelLocalMarkStack, LocalCollectStack>;

    ObjectMarker(MarkingStack *stack, GCTaskCause reason) : stack_(stack), gcReason_(reason)
    {
        ASSERT(stack_ != nullptr);
    }

    bool ProcessObjectPointer(ObjectHeader *obj, ObjectPointerType *field)
    {
        if (ObjectAccessor::Load(field) == 0) {
            return true;
        }
        ProcessImpl(obj, reinterpret_cast<RefField<> &>(*field));
        return true;
    }

private:
    void ProcessImpl(ObjectHeader *obj, RefField<> &field)
    {
        RefField<> oldField(field);
        BaseObject *targetObj = oldField.GetTargetObject();

        if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
            return;
        }
        // field is tagged object, should be in heap
        DCHECK(Heap::IsHeapAddress(targetObj));

        auto *targetRegion = RegionDesc::GetAliveRegionDescAt(targetObj);
        // cannot skip objects in EXEMPTED_FROM_REGION, because its rset is incomplete
        if (gcReason_ == GCTaskCause::YOUNG_GC_CAUSE && !targetRegion->IsInYoungSpace()) {
            LOG(DEBUG, GC) << "marking: skip non-young object " << obj << "@" << &field
                           << ", target object: " << targetObj << "<" << targetObj->GetTypeInfo() << ">("
                           << targetObj->GetSize() << ")";
            return;
        }

        if (targetRegion->IsNewObjectSinceMarking(targetObj)) {
            LOG(DEBUG, GC) << "marking: skip new obj " << targetObj << "<" << targetObj->GetTypeInfo() << ">("
                           << targetObj->GetSize() << ")";
            return;
        }

        if (targetRegion->MarkObject(targetObj)) {
            LOG(DEBUG, GC) << "marking: obj has been marked " << targetObj;
            return;
        }
        LOG(DEBUG, GC) << "marking obj " << obj << " ref@" << &field << ": " << targetObj << "<"
                       << targetObj->GetTypeInfo() << ">(" << targetObj->GetSize() << ")";

        stack_->Push(targetObj);
    }

    MarkingStack *stack_;
    GCTaskCause gcReason_;
};

#ifdef PANDA_JS_ETS_HYBRID_MODE
// note each ref-field will not be traced twice, so each old pointer the tracer meets must come from previous gc.
template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkingXRef(RefField<> &field, ParallelLocalMarkStack &workStack) const
{
    BaseObject *targetObj = field.GetTargetObject();
    auto region = RegionDesc::GetRegionDescAt(reinterpret_cast<MAddress>(targetObj));
    // field is tagged object, should be in heap
    DCHECK(Heap::IsHeapAddress(targetObj));

    LOG(DEBUG, GC) << "trace obj " << targetObj << " <" << targetObj->GetTypeInfo() << ">(" << targetObj->GetSize()
                   << ")";
    if (region->IsNewObjectSinceForward(targetObj)) {
        LOG(DEBUG, GC) << "trace: skip new obj " << targetObj << "<" << targetObj->GetTypeInfo() << ">("
                       << targetObj->GetSize() << ")";
        return;
    }
    if (!region->MarkObject(targetObj)) {
        workStack.Push(targetObj);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkingObjectXRef(BaseObject *obj, ParallelLocalMarkStack &workStack)
{
    auto refFunc = [this, &workStack](RefField<> &field) { MarkingXRef(field, workStack); };

    obj->IterateXRef(refFunc);
}
#endif

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixRefField(BaseObject *obj, RefField<> &field) const
{
    RefField<> oldField(field);
    BaseObject *targetObj = oldField.GetTargetObject();
    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // target object could be null or non-heap for some static variable.
    if (!Heap::IsHeapAddress(targetObj)) {
        return;
    }

    auto *refRegion = RegionDesc::GetRegionDescAt(targetObj);
    bool isFrom = refRegion->IsFromRegion();
    bool isInRcent = refRegion->IsInRecentSpace();
    if (isInRcent) {
        auto *objRegion = RegionDesc::GetRegionDescAt(obj);
        if (!objRegion->IsInRecentSpace() && objRegion->MarkRSetCardTable(obj)) {
            LOG(DEBUG, GC) << "fix phase update point-out remember set of region " << objRegion << ", obj " << obj
                           << ", ref: <" << targetObj->GetTypeInfo() << ">";
        }
        return;
    } else if (!isFrom) {
        return;
    }
    BaseObject *latest = FindToVersion(targetObj);

    if (latest == nullptr) {
        return;
    }

    CHECK(latest->IsValidObject());
    RefField<> newField(latest);
    if (field.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
        LOG(DEBUG, GC) << "fix obj " << obj << "+" << obj->GetSize() << " ref@" << &field << ": 0x" << std::hex
                       << oldField.GetFieldValue() << " => " << std::dec << latest << "<" << latest->GetTypeInfo()
                       << ">(" << latest->GetSize() << ")";
        LOG(DEBUG, MM_OBJECT_EVENTS) << "[CMC] "
                                     << "FIX ref in obj " << obj << " field@" << &field << ": " << targetObj << " -> "
                                     << latest;
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixObjectRefFields(BaseObject *obj) const
{
    LOG(DEBUG, GC) << "fix obj " << obj << "<" << obj->GetTypeInfo() << ">(" << obj->GetSize() << ")";
    auto refFunc = [this, obj](RefField<> &field) { FixRefField(obj, field); };
    obj->ForEachRefField(refFunc, refFunc);
}

template <class LanguageConfig>
class PreforwardVisitor {
public:
    explicit PreforwardVisitor(CmcGC<LanguageConfig> *collector) : collector_(collector) {}

    void operator()(GCRoot root)
    {
        ObjectHeader *oldObj = root.GetObjectHeader();
        LOG(DEBUG, GC) << "visit raw-ref @" << root.GetObjectPointer() << ": " << oldObj;

        auto regionType = RegionDesc::GetRegionDescAt(oldObj)->GetRegionType();
        if (regionType != RegionDesc::RegionType::FROM_REGION) {
            return;
        }

        BaseObject *toVersion = collector_->TryForwardObject(oldObj);
        if (toVersion == nullptr) {  // LCOV_EXCL_BR_LINE
            Heap::throwOOM();
            return;
        }

        ASSERT(root.GetObjectHeader() == oldObj);
        root.Update(toVersion);
        LOG(DEBUG, GC) << "fix raw-ref @" << root.GetObjectPointer() << ": " << oldObj << " -> " << toVersion;
        MarkToObject(toVersion);
    }

private:
    void MarkToObject(BaseObject *toVersion)
    {
        if (collector_->MarkObjectIfNotMarked(toVersion)) {
            auto *region = RegionDesc::GetAliveRegionDescAt(toVersion);
            region->AddLiveByteCount(toVersion->GetSize());
        }
    }

private:
    CmcGC<LanguageConfig> *collector_;
};

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardStaticRoots()
{
    ScopedTrace tracer("PreforwardStaticRoots", ark::common_vm::ENABLE_GC_TRACING);
    PreforwardVisitor<LanguageConfig> visitor(this);
    VisitRoots(visitor);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardConcurrentRoots()
{
    GCRootVisitor visitor = [this](GCRoot root) {
        ObjectHeader *oldObj = root.GetObjectHeader();
        LOG(DEBUG, GC) << "visit raw-ref @" << root.GetObjectPointer() << ": " << oldObj;
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            ASSERT_PRINT(toVersion != nullptr, "TryForwardObject failed");
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (CompareExchangeRefSlot(root.GetObjectPointer(), oldObj, toVersion)) {
                LOG(DEBUG, GC) << "fix raw-ref @" << root.GetObjectPointer() << ": " << oldObj << " -> " << toVersion;
                LOG(DEBUG, MM_OBJECT_EVENTS) << "[CMC] "
                                             << "PREFORWARD concurrent ref @" << root.GetObjectPointer() << ": "
                                             << oldObj << " -> " << toVersion;
            }
        }
    };
    VisitConcurrentRoots(visitor);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardStaticWeakRoots(GCTaskCause reason)
{
    ScopedTrace tracer("PreforwardStaticRoots", ark::common_vm::ENABLE_GC_TRACING);

    WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor(reason);
    VisitWeakRoots(weakVisitor);
    ForEachManagedMutator([](Mutator *mutator) { mutator->RequestReferencesCleanup(); });

    auto *allocBuffer = ark::common_vm::AllocationBuffer::GetAllocBuffer();
    if (LIKELY(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardConcurrencyModelRoots()
{
    LOG(FATAL, COMMON) << "Unresolved fatal";
    UNREACHABLE();
}

class EnumRootsBuffer {
public:
    EnumRootsBuffer();
    void UpdateBufferSize();
    CArrayList<ObjectHeader *> *GetBuffer()
    {
        return &buffer_;
    }

private:
    static size_t bufferSize_;
    CArrayList<ObjectHeader *> buffer_;
};

size_t EnumRootsBuffer::bufferSize_ = 16;
EnumRootsBuffer::EnumRootsBuffer() : buffer_(bufferSize_)
{
    buffer_.clear();  // memset to zero and allocated real memory
}

void EnumRootsBuffer::UpdateBufferSize()
{
    if (buffer_.empty()) {
        return;
    }
    const size_t decreaseBufferThreshold = bufferSize_ >> 2;
    if (buffer_.size() < decreaseBufferThreshold) {
        bufferSize_ = bufferSize_ >> 1;
    } else {
        bufferSize_ = std::max(buffer_.capacity(), bufferSize_);
    }
    if (buffer_.capacity() > UINT16_MAX) {
        LOG(INFO, COMMON) << "too many roots, allocated buffer too large: " << buffer_.size() << ", allocate "
                          << (static_cast<double>(buffer_.capacity()) / MB);
    }
}

template <class LanguageConfig>
template <typename CmcGC<LanguageConfig>::EnumRootsPolicy policy>
CArrayList<ObjectHeader *> CmcGC<LanguageConfig>::EnumRoots()
{
    EnumRootsBuffer buffer;
    CArrayList<ObjectHeader *> *results = buffer.GetBuffer();
    GCRootVisitor visitor = [&results](GCRoot root) { results->push_back(root.GetObjectHeader()); };

    if constexpr (policy == EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR) {
        EnumRootsImpl<VisitRoots>(visitor);
    } else if constexpr (policy == EnumRootsPolicy::STW_AND_NO_FLIP_MUTATOR) {
        ScopedStopTheWorld stw;
        GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::COMMON_PAUSE);
        ScopedTrace tracer(("EnumRoots-STW-bufferSize(" + ToPandaString(results->capacity()) + ")").c_str(),
                           ark::common_vm::ENABLE_GC_TRACING);
        EnumRootsImpl<VisitRoots>(visitor);
    } else if constexpr (policy == EnumRootsPolicy::STW_AND_FLIP_MUTATOR) {
        auto rootSet = EnumRootsFlip(visitor);
        for (const auto &roots : rootSet) {
            std::copy(roots.begin(), roots.end(), std::back_inserter(*results));
        }
        VisitConcurrentRoots(visitor);
    }
    buffer.UpdateBufferSize();
    return std::move(*results);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkingHeap(const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason)
{
    mem::GCScope<mem::TRACE_TIMING> gcScope("Marking live objects", this);
    // Atomic with relaxed order reason: data race with markedObjectCount_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    markedObjectCount_.store(0, std::memory_order_relaxed);
    {
        ScopedStopTheWorld stw;
        TransitionToGCPhase(GCPhase::GC_PHASE_MARK);
    }

    MarkingRoots(collectedRoots, reason);
    ExemptFromSpace();
}

template <class LanguageConfig>
WeakRefFieldVisitor CmcGC<LanguageConfig>::GetWeakRefFieldVisitor(GCTaskCause reason)
{
    return [this, reason](RefField<> &refField) -> bool {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (reason == GCTaskCause::YOUNG_GC_CAUSE) {
            if (RegionalHeap::IsYoungSpaceObject(oldObj) && !IsMarkedObject(oldObj) &&
                !RegionalHeap::IsNewObjectSinceMarking(oldObj)) {
                return false;
            }
        } else {
            if (!IsMarkedObject(oldObj) && !RegionalHeap::IsNewObjectSinceMarking(oldObj)) {
                return false;
            }
        }

        LOG(DEBUG, GC) << "visit weak raw-ref @" << &refField << ": " << oldObj;
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK(toVersion != nullptr);
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                LOG(DEBUG, GC) << "fix weak raw-ref @" << &refField << ": " << oldObj << " -> " << toVersion;
                LOG(DEBUG, MM_OBJECT_EVENTS)
                    << "[CMC] "
                    << "PREFORWARD weak ref @" << &refField << ": " << oldObj << " -> " << toVersion;
            }
        }
        return true;
    };
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardFlip(GCTaskCause reason)
{
    {
        ScopedStopTheWorld stw;
        GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::REMARK_PAUSE);

        ScopedTrace tracer("PreforwardFlip[STW]", ark::common_vm::ENABLE_GC_TRACING);
        SetGCThreadQosPriority(ark::common_vm::PriorityMode::STW);
        ASSERT_PRINT(GetThreadPool() != nullptr, "thread pool is null");
        TransitionToGCPhase(GCPhase::GC_PHASE_REMARK);
        Remark(reason);
        reinterpret_cast<RegionalHeap &>(theAllocator_).PrepareForward();

        TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY);
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor(reason);
        SetGCThreadQosPriority(ark::common_vm::PriorityMode::FOREGROUND);

        VisitWeakGlobalRoots(weakVisitor, reason == GCTaskCause::YOUNG_GC_CAUSE);

        ForEachManagedMutator([](Mutator *mutator) {
            // Request finalize callback in each vm-thread when gc finished.
            mutator->RequestReferencesCleanup();
        });
    }

    AllocationBuffer *allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::Preforward(GCTaskCause reason)
{
    mem::GCScope<mem::TRACE_TIMING> gcScope("Preforward", this);

    TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY);

    [[maybe_unused]] auto *threadPool = GetThreadPool();
    ASSERT_PRINT(threadPool != nullptr, "thread pool is null");

    // copy and fix finalizer roots.
    // Only one root task, no need to post task.
    PreforwardStaticWeakRoots(reason);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ConcurrentPreforward()
{
    ScopedTrace tracer("ConcurrentPreforward", ark::common_vm::ENABLE_GC_TRACING);
    PreforwardConcurrentRoots();
}

template <class LanguageConfig>
size_t CmcGC<LanguageConfig>::ParallelFixHeap()
{
    auto &regionalHeap = reinterpret_cast<RegionalHeap &>(theAllocator_);
    auto taskList = regionalHeap.CollectFixTasks();
    std::atomic<int> taskIter = 0;
    std::function<FixHeapTask *()> getNextTask = [&taskIter, &taskList]() {
        // Atomic with relaxed order reason: data race with taskIter with no synchronization or ordering constraints
        // imposed on other reads or writes
        uint32_t idx = static_cast<uint32_t>(taskIter.fetch_add(1U, std::memory_order_relaxed));
        if (idx < taskList.size()) {
            return &taskList[idx];
        }
        return (FixHeapTask *)nullptr;
    };

    const uint32_t runningWorkers = GetGCThreadCount(true) - 1;
    uint32_t parallelCount = runningWorkers + 1;  // 1 ：DaemonThread
    PandaVector<FixHeapWorker::Result> results(parallelCount);
    size_t nonMovableGarbageSize = 0;
    {
        ScopedTrace tracer("FixHeap [Parallel]", ark::common_vm::ENABLE_GC_TRACING);

        // Fix heap
        TaskPackMonitor monitor(runningWorkers, runningWorkers);
        for (uint32_t i = 1; i < parallelCount; ++i) {
            GetThreadPool()->PostTask(MakePandaUnique<FixHeapWorker>(this, monitor, results[i], getNextTask));
        }

        FixHeapWorker gcWorker(this, monitor, results[0], getNextTask);
        auto task = getNextTask();
        while (task != nullptr) {
            gcWorker.DispatchRegionFixTask(task);
            task = getNextTask();
        }
        monitor.WaitAllFinished();
    }

    {
        ScopedTrace tracer("Post FixHeap Clear [Parallel]", ark::common_vm::ENABLE_GC_TRACING);

        // Post clear task
        TaskPackMonitor monitor(runningWorkers, runningWorkers);
        for (uint32_t i = 1; i < parallelCount; ++i) {
            GetThreadPool()->PostTask(MakePandaUnique<PostFixHeapWorker>(results[i], monitor));
        }

        PostFixHeapWorker gcWorker(results[0], monitor);
        gcWorker.PostClearTask();
        nonMovableGarbageSize = PostFixHeapWorker::CollectEmptyRegions();
        monitor.WaitAllFinished();
    }
    return nonMovableGarbageSize;
}

template <class LanguageConfig>
size_t CmcGC<LanguageConfig>::FixHeap(bool isWorldStopped)
{
    if (!isWorldStopped) {
        ScopedStopTheWorld stw;
        TransitionToGCPhase(GCPhase::GC_PHASE_FIX);
    } else {
        TransitionToGCPhase(GCPhase::GC_PHASE_FIX);
    }
    mem::GCScope<mem::TRACE_TIMING> gcScope("FixHeap", this);

    ScopedTrace tracer("FixHeap", ark::common_vm::ENABLE_GC_TRACING);

    size_t nonMovableGarbageSize = ParallelFixHeap();

    WVerify::VerifyAfterFix(this->GetGCPhase(), isWorldStopped);

    return nonMovableGarbageSize;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::CollectGarbageWithXRef(GCTaskCause reason)
{
    const bool isNotYoungGC = reason != GCTaskCause::YOUNG_GC_CAUSE;
#ifdef ENABLE_CMC_RB_DFX
    WVerify::DisableReadBarrierDFX(false);
#endif
    ScopedStopTheWorld stw;
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::COMMON_PAUSE);
    ark::common_vm::RemoveXRefFromRoots();

    auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
    MarkingHeap(collectedRoots, reason);
    TransitionToGCPhase(GCPhase::GC_PHASE_REMARK);
    Remark(reason);
    ark::common_vm::SweepUnmarkedXRefs();

    ark::common_vm::AddXRefToRoots();
    Preforward(reason);
    ConcurrentPreforward();
    // reclaim large objects should after preforward(may process weak ref) and
    // before fix heap(may clear live bit)
    if (isNotYoungGC) {
        CollectLargeGarbage();
    }

    CopyFromSpace();
    WVerify::VerifyAfterForward(this->GetGCPhase(), true);

    size_t nonMovableGarbageSize = FixHeap(true);
    if (isNotYoungGC) {
        CollectNonMovableGarbage(nonMovableGarbageSize);
    }

    TransitionToGCPhase(GCPhase::GC_PHASE_IDLE);

    ClearAllGCInfo();
    CollectSmallSpace();
    ark::common_vm::UnmarkAllXRefs();

#if defined(ENABLE_CMC_RB_DFX)
    WVerify::EnableReadBarrierDFX(true);
#endif
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ForEachManagedMutator(const ManagedMutatorCallback &callback)
{
    Mutator::GetCurrent()->GetVM()->GetMutatorManager()->ForEachMutator([&callback](Mutator *mutator) {
        if (mutator->GetMutatorType() == Mutator::MutatorType::MANAGED) {
            callback(mutator);
        }
        return true;
    });
}

template <class LanguageConfig>
// CC-OFFNXT(G.FUD.05) solid logic
void CmcGC<LanguageConfig>::DoGarbageCollection(ark::GCTask &task)
{
    const bool isYoungGC = task.reason == GCTaskCause::YOUNG_GC_CAUSE;

    mem::GCScope<mem::TRACE_TIMING> gcScope(__FUNCTION__, this);

    if (task.reason == GCTaskCause::CROSSREF_CAUSE) {
        CollectGarbageWithXRef(task.reason);
        return;
    }
    if (gcMode_ == GCMode::STW) {  // 2: stw-gc
#ifdef ENABLE_CMC_RB_DFX
        WVerify::DisableReadBarrierDFX(false);
#endif
        {
            ScopedStopTheWorld stw;
            GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr,
                                                PauseTypeStats::COMMON_PAUSE);
            auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
            MarkingHeap(collectedRoots, task.reason);
            TransitionToGCPhase(GCPhase::GC_PHASE_REMARK);
            Remark(task.reason);

            Preforward(task.reason);
            ConcurrentPreforward();
            // reclaim large objects should after preforward(may process weak ref) and
            // before fix heap(may clear live bit)
            if (!isYoungGC) {
                CollectLargeGarbage();
            }

            CopyFromSpace();
            WVerify::VerifyAfterForward(this->GetGCPhase(), true);

            size_t nonMovableGarbageSize = FixHeap(true);
            if (!isYoungGC) {
                CollectNonMovableGarbage(nonMovableGarbageSize);
            }

            TransitionToGCPhase(GCPhase::GC_PHASE_IDLE);

            ClearAllGCInfo();
            CollectSmallSpace();

#if defined(ENABLE_CMC_RB_DFX)
            WVerify::EnableReadBarrierDFX(true);
#endif
        }
        return;
    }

    if (!isYoungGC) {
        auto collectedRoots = EnumRoots<EnumRootsPolicy::STW_AND_FLIP_MUTATOR>();
        MarkingHeap(collectedRoots, task.reason);
        PreforwardFlip(task.reason);
        ConcurrentPreforward();
        // reclaim large objects should after preforward(may process weak ref)
        // and before fix heap(may clear live bit)
        if (!isYoungGC) {
            CollectLargeGarbage();
        }

        CopyFromSpace();
        WVerify::VerifyAfterForward(this->GetGCPhase(), false);

        size_t nonMovableGarbageSize = FixHeap(false);

        if (!isYoungGC) {
            CollectNonMovableGarbage(nonMovableGarbageSize);
        }
    } else {
        DoGarbageCollectionWithoutConcurrentMarking();
    }

    {
        ScopedStopTheWorld stw;
        TransitionToGCPhase(GCPhase::GC_PHASE_IDLE);
    }
    ClearAllGCInfo();
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    space.DumpAllRegionSummary("Peak GC log");
    space.DumpAllRegionStats("region statistics when gc ends");
    CollectSmallSpace();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::DoGarbageCollectionWithoutConcurrentMarking()
{
    CHECK(Heap::GetHeap().GetGCReason() == GCTaskCause::YOUNG_GC_CAUSE);

    auto useGcWorkers = this->GetSettings()->ParallelCompactingEnabled();
    CMCGCAdaptiveStack evacuationStack(this, useGcWorkers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                       useGcWorkers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                       GCWorkersTaskTypes::TASK_EVACUATE_REGIONS,
                                       this->GetSettings()->GCMarkingStackNewTasksFrequency());
    PreforwardNonHeapRoots(evacuationStack);
    EnqueueRememberedSetRefs(evacuationStack);
    ProcessEvacuationStack(evacuationStack);
    if (useGcWorkers) {
        this->GetWorkersTaskPool()->WaitUntilTasksEnd();
    }

    RemarkYoungCollectionSpace();

    // We do not evacuate objects on remark pause to avoid too long STW
    CopyFromSpace();

    WVerify::VerifyAfterForward(this->GetGCPhase(), false);

    FixHeap(false);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardNonHeapRoots(CMCGCAdaptiveStack &stack)
{
    PandaVector<ObjectHeader *> roots = PreforwardNonHeapRootsFlip();
    for (auto *obj : roots) {
        if (MarkObjectIfNotMarked(obj)) {
            EnqueueRefsToYoungCollectionSpace(obj, stack);
        }
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardNonHeapRoot(GCRoot root, PandaVector<ObjectHeader *> &forwardedRoots)
{
    auto *obj = root.GetObjectHeader();
    auto *region = RegionDesc::GetRegionDescAt(obj);

    if (!InYoungCollectionSpace(region)) {
        // we do not trace from such root
        return;
    }

    if (region->IsFromRegion()) {
        obj = ForwardObject(obj);
        root.Update(obj);
    }

    forwardedRoots.push_back(obj);
}

template <class LanguageConfig>
PandaVector<ObjectHeader *> CmcGC<LanguageConfig>::PreforwardNonHeapRootsFlip()
{
    PandaVector<ObjectHeader *> forwardedRoots;
    {
        ScopedStopTheWorld stw;
        GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::COMMON_PAUSE);

        SetGCThreadQosPriority(PriorityMode::STW);
        auto &heap = static_cast<RegionalHeap &>(theAllocator_);
        heap.AssembleGarbageCandidates();
        heap.PrepareMarking();
        heap.PrepareForward();

        TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY);
        mem::RootManager<LanguageConfig> rootManager(this->GetPandaVm());

        auto callback = [this, &forwardedRoots](GCRoot root) { PreforwardNonHeapRoot(root, forwardedRoots); };
        rootManager.VisitAotStringRoots(callback, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
        rootManager.VisitVmRoots(callback);
        rootManager.VisitLocalRoots(callback);

        TransitionToGCPhase(GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE);
        SetGCThreadQosPriority(PriorityMode::FOREGROUND);
    }

    VisitConcurrentRoots([&forwardedRoots](GCRoot root) {
        auto *obj = root.GetObjectHeader();
        auto *region = RegionDesc::GetRegionDescAt(obj);
        if (InYoungCollectionSpace(region)) {
            CHECK(!region->IsFromRegion());
            forwardedRoots.push_back(obj);
        }
    });
    return forwardedRoots;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkYoungCollectionSpace()
{
    ScopedStopTheWorld stw;
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::REMARK_PAUSE);

    TransitionToGCPhase(GCPhase::GC_PHASE_REMARK);

    bool useGcWorkers = this->GetSettings()->ParallelMarkingEnabled();
    CMCGCAdaptiveStack remarkStack(this, useGcWorkers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                   useGcWorkers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                   GCWorkersTaskTypes::TASK_REMARK,
                                   this->GetSettings()->GCMarkingStackNewTasksFrequency());
    RemarkNonHeapRoots(remarkStack);
    MarkSatbBuffer(remarkStack);
    MarkEvacuationStack(remarkStack);
    if (useGcWorkers) {
        this->GetWorkersTaskPool()->WaitUntilTasksEnd();
    }

    // we should update forwarded weak references located in global storage
    auto weakVisitor = [this](RefField<> &refField) -> bool {
        RefField<> oldField(refField);
        auto *oldObj = oldField.GetTargetObject();
        auto *region = RegionDesc::GetRegionDescAt(oldObj);
        if (!region->IsFromRegion()) {
            return true;
        }
        if (!oldObj->IsForwarded()) {
            // it is unreachable so it is not forwarded
            return false;
        }
        auto *toVersion = GetForwardingPointer(oldObj);
        RefField<> newField(toVersion);
        refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue());
        return true;
    };
    VisitWeakGlobalRoots(weakVisitor, true);

    SatbBuffer::Instance().ClearBuffer();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkNonHeapRoots(CMCGCAdaptiveStack &stack)
{
    VisitRoots([this, &stack](GCRoot root) { RemarkNonHeapRoot(root, stack); });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::EnqueueRememberedSetRefs(CMCGCAdaptiveStack &stack)
{
    auto &space = static_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
    space.MarkRememberSet([this, &stack](BaseObject *object) { EnqueueRefsToYoungCollectionSpace(object, stack); });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkSatbBuffer(CMCGCAdaptiveStack &stack)
{
    PandaStack<BaseObject *> localStack;
    ForEachManagedMutator([&localStack](Mutator *mutator) {
        const SatbBuffer::TreapNode *node = static_cast<const SatbBuffer::TreapNode *>(mutator->GetSatbBufferNode());
        if (node != nullptr) {
            const_cast<SatbBuffer::TreapNode *>(node)->GetObjects(localStack);
        }
    });
    SatbBuffer::Instance().GetRetiredObjects(localStack);

    while (!localStack.empty()) {
        ObjectHeader *obj = localStack.top();
        localStack.pop();
        CHECK(Heap::IsHeapAddress(obj));
        if (IsFromObject(obj)) {
            obj = ForwardObject(obj);
        }
        auto *region = RegionDesc::GetAliveRegionDescAt(obj);
        if (MarkObjectIfNotMarked(obj)) {
            auto size = obj->ObjectSize();
            EnqueueRefsToYoungCollectionSpace(obj, stack);
            region->AddLiveByteCount(size);
        }
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessEvacuationStack(CMCGCAdaptiveStack &stack)
{
    // NOTE: it is performed concurrently with other GC threads and mutators
    PandaStack<BaseObject *> remarkStack;
    auto fetchFromSatbBuffer = [this, &stack, &remarkStack]() {
        SatbBuffer::Instance().TryFetchOneRetiredNode(remarkStack);
        bool needProcess = false;
        while (!remarkStack.empty()) {
            ObjectHeader *obj = remarkStack.top();
            remarkStack.pop();
            CHECK(Heap::IsHeapAddress(obj));
            if (IsFromObject(obj)) {
                obj = ForwardObject(obj);
            }
            if (this->MarkObjectIfNotMarked(obj)) {
                EnqueueRefsToYoungCollectionSpace(obj, stack);
                needProcess = true;
            }
        }
        return needProcess;
    };
    size_t iterationCount = 0;
    const size_t maxIterationCount = 1000;
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        while (!stack.Empty()) {
            ObjectPointerType *ref = stack.PopFromStack();
            ProcessRef(ref, stack);
        }

        if (++iterationCount >= maxIterationCount) {
            break;
        }

        if (!fetchFromSatbBuffer()) {
            break;
        }
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkEvacuationStack(CMCGCAdaptiveStack &stack)
{
    while (!stack.Empty()) {
        ObjectPointerType *ref = stack.PopFromStack();
        MarkRef(ReadRefSlot(ref), stack);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkNonHeapRoot(GCRoot root, CMCGCAdaptiveStack &stack)
{
    auto *obj = root.GetObjectHeader();
    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    CHECK(Heap::GetHeap().GetGCReason() == GCTaskCause::YOUNG_GC_CAUSE);
    // Root was updated while concurrent copying phase so it cannot be in FromRegion
    CHECK(!region->IsFromRegion());

    if (!InYoungCollectionSpace(region)) {
        return;
    }

    if (region->IsNewObjectSinceMarking(obj)) {
        return;
    }

    if (MarkObjectIfNotMarked(obj)) {
        EnqueueRefsToYoungCollectionSpace(obj, stack);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessRef(ObjectPointerType *ref, CMCGCAdaptiveStack &stack)
{
    auto *obj = AtomicReadRefSlot(ref);

    if (!Heap::IsHeapAddress(obj)) {
        return;
    }

    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    if (region->IsFromRegion()) {
        ObjectHeader *fwd = ForwardObject(obj);

        // Object can be evacuated by mutator so mark it
        if (MarkObjectIfNotMarked(fwd)) {
            EnqueueRefsToYoungCollectionSpace(fwd, stack);
        }
        CompareExchangeRefSlot(ref, obj, fwd);
        return;
    }

    if (!InYoungCollectionSpace(region)) {
        return;
    }

    if (region->IsNewObjectSinceMarking(obj)) {
        return;
    }

    if (MarkObjectIfNotMarked(obj)) {
        EnqueueRefsToYoungCollectionSpace(obj, stack);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkRef(BaseObject *obj, CMCGCAdaptiveStack &stack)
{
    CHECK(Heap::GetHeap().GetGCReason() == GCTaskCause::YOUNG_GC_CAUSE);

    if (!Heap::IsHeapAddress(obj)) {
        return;
    }

    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    if (!InYoungCollectionSpace(obj)) {
        return;
    }

    if (region->IsNewObjectSinceMarking(obj)) {
        return;
    }

    if (region->IsFromRegion() && obj->IsForwarded()) {
        obj = GetForwardingPointer(obj);
    }

    if (MarkObjectIfNotMarked(obj)) {
        auto size = obj->ObjectSize();
        EnqueueRefsToYoungCollectionSpace(obj, stack);
        region->AddLiveByteCount(size);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::EnqueueRefsToYoungCollectionSpace(ObjectHeader *obj, CMCGCAdaptiveStack &stack)
{
    class Handler final {
    public:
        explicit Handler(CMCGCAdaptiveStack &stack) : stack_(stack) {}

        ~Handler() = default;

        bool ProcessObjectPointer([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p)
        {
            auto ref = ReadRefSlot(p);
            if (ref == 0) {
                return true;
            }
            if (InYoungCollectionSpace(ref)) {
                stack_.PushToStack(p);
            }
            return true;
        }

    private:
        CMCGCAdaptiveStack &stack_;
    };

    auto *cls = obj->template ClassAddr<Class>();
    Handler handler {stack};
    mem::ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(cls, obj, &handler);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(const RegionDesc *region)
{
    return InYoungCollectionSpace(region->GetRegionType());
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(const BaseObject *obj)
{
    return Heap::IsHeapAddress(obj) && InYoungCollectionSpace(RegionDesc::GetRegionDescAt(obj));
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(RegionDesc::RegionType type)
{
    // During single pass evacuation in young GC objects might be concurrently evacuated by mutator. So we should
    // inspect references which point to them too.
    return RegionDesc::IsInYoungSpace(type) || RegionDesc::IsInToSpace(type);
}

template <class LanguageConfig>
CArrayList<CArrayList<BaseObject *>> CmcGC<LanguageConfig>::EnumRootsFlip(const GCRootVisitor &visitor)
{
    ark::os::memory::Mutex stackMutex;
    CArrayList<CArrayList<BaseObject *>> rootSet;  // allcate for each mutator
    {
        ScopedStopTheWorld stw;
        GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::COMMON_PAUSE);

        SetGCThreadQosPriority(PriorityMode::STW);
        EnumRootsImpl<VisitGlobalRoots>(visitor);
        SetGCThreadQosPriority(PriorityMode::FOREGROUND);

        ForEachManagedMutator([&rootSet, &stackMutex](Mutator *mutator) {
            CArrayList<BaseObject *> roots;
            RefFieldVisitor localVisitor = [&roots](RefField<> &root) { roots.emplace_back(root.GetTargetObject()); };
            mutator->VisitMutatorRoots(localVisitor);
            ark::os::memory::LockHolder lockGuard(stackMutex);
            rootSet.emplace_back(std::move(roots));
        });
    }

    return rootSet;
}

template <class LanguageConfig>
ObjectHeader *CmcGC<LanguageConfig>::ForwardObject(ObjectHeader *obj)
{
    ObjectHeader *to = TryForwardObject(obj);
    return (to != nullptr) ? to : obj;
}

template <class LanguageConfig>
BaseObject *CmcGC<LanguageConfig>::TryForwardObject(ObjectHeader *obj)
{
    return static_cast<BaseObject *>(CopyObjectImpl(obj));
}

// ConcurrentGC
template <class LanguageConfig>
ObjectHeader *CmcGC<LanguageConfig>::CopyObjectImpl(ObjectHeader *object)
{
    // reconsider phase difference between mutator and GC thread during transition.
    if (ark::common_vm::IsGcThread()) {
        CHECK(this->GetGCPhase() == GCPhase::GC_PHASE_PRECOPY || this->GetGCPhase() == GCPhase::GC_PHASE_COPY ||
              this->GetGCPhase() == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE ||
              this->GetGCPhase() == GCPhase::GC_PHASE_FIX || this->GetGCPhase() == GCPhase::GC_PHASE_REMARK);
    } else {
        auto phase = ark::Mutator::GetCurrent()->GetMutatorPhase();
        CHECK(phase == GCPhase::GC_PHASE_PRECOPY || phase == GCPhase::GC_PHASE_COPY ||
              phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE || phase == GCPhase::GC_PHASE_FIX);
    }

    do {
        MarkWord oldWord = object->GetMark();
        // 1. object has already been forwarded
        if (oldWord.IsForwarded()) {
            return reinterpret_cast<ObjectHeader *>(oldWord.GetForwardingAddress());
        }
        // ConcurrentGC
        // 2. object is being forwarded, spin until it is forwarded (or gets its own forwarded address)
        if (oldWord.IsReadBarrierSet()) {
            sched_yield();
            continue;
        }
        // 3. hope we can copy this object
        MarkWord newWord = oldWord.SetReadBarrier();
        if (object->AtomicSetMark(oldWord, newWord)) {
            return CopyObjectAfterExclusive(object, newWord);
        }
    } while (true);
    LOG(FATAL, COMMON) << "forwardObject exit in wrong path";
    UNREACHABLE();
    return nullptr;
}

template <class LanguageConfig>
ObjectHeader *CmcGC<LanguageConfig>::CopyObjectAfterExclusive(ObjectHeader *object, MarkWord markWord)
{
    size_t size = RegionalHeap::ToAllocatedSize(object->ObjectSize());
    AllocationBuffer *buffer = AllocationBuffer::GetOrCreateAllocBuffer();
    uintptr_t toAddr = buffer->ToSpaceAllocate(size);
    if (toAddr == 0) {
        // ConcurrentGC
        UnlockObject(object);
        return nullptr;
    }
    LOG(DEBUG, MM_OBJECT_EVENTS) << "MOVE object " << object << " -> " << ToVoidPtr(toAddr);
    CopyObject(*reinterpret_cast<BaseObject *>(object), *reinterpret_cast<BaseObject *>(toAddr), size);

    auto *toObj = reinterpret_cast<ObjectHeader *>(toAddr);
    toObj->SetMark(toObj->GetMark().ClearReadBarrier());

    // Atomic with release order reason: ensure copied object content is visible before setting forwarding state
    std::atomic_thread_fence(std::memory_order_release);
    object->SetMark(markWord.DecodeFromForwardingAddress(reinterpret_cast<MarkWord::MarkWordSize>(toAddr)));
    return reinterpret_cast<ObjectHeader *>(toObj);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::UnlockObject(ObjectHeader *object)
{
    do {
        MarkWord current = object->AtomicGetMark(std::memory_order_acquire);
        MarkWord newMarkWord = current.ClearReadBarrier();
        if (object->AtomicSetMark<false>(current, newMarkWord)) {
            return;
        }
    } while (true);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ClearAllGCInfo()
{
    mem::GCScope<mem::TRACE_TIMING> gcScope("ClearAllGCInfo", this);
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    space.ClearAllGCInfo();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::CollectSmallSpace()
{
    ScopedTrace tracer("CollectSmallSpace", ark::common_vm::ENABLE_GC_TRACING);
    auto &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    mem::GCScope<mem::TRACE_TIMING> gcScope("CollectFromSpaceGarbage", this);
    size_t youngGarbage = space.FromRegionSize() - space.ToSpaceSize();
    size_t freedObjectCount = 0;
    if (cmcAllocator_->NeedToTrackFreedObjects()) {
        auto countFreedObjects = [&freedObjectCount](BaseObject *obj) {
            if (!RegionalHeap::IsSurvivedObject(obj)) {
                LOG_DEBUG_OBJECT_EVENTS << "DELETE YOUNG object " << obj;
                ++freedObjectCount;
            }
        };
        space.GetFromSpace().GetFromRegionList().VisitAllRegions(
            [&countFreedObjects](RegionDesc *region) { region->VisitAllObjects(countFreedObjects); });
    }
    space.CollectFromSpaceGarbage();
    space.HandlePromotion();
    this->GetPandaVm()->GetMemStats()->RecordFreeObjects(freedObjectCount, youngGarbage,
                                                         ark::SpaceType::SPACE_TYPE_OBJECT);
    cmcAllocator_->SetLiveBytesAfterGC(space.GetAllocatedBytes());
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::SetGCThreadQosPriority(PriorityMode mode)
{
#ifdef ENABLE_QOS
    LOG(DEBUG, COMMON) << "SetGCThreadQosPriority gettid " << gettid();

    ScopedTrace tracer("SetGCThreadQosPriority", ark::common_vm::ENABLE_GC_TRACING);
    switch (mode) {
        case PriorityMode::STW: {
            OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE, gettid());
            break;
        }
        case PriorityMode::FOREGROUND: {
            OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INITIATED, gettid());
            break;
        }
        case PriorityMode::BACKGROUND: {
            OHOS::QOS::ResetQosForOtherThread(gettid());
            break;
        }
        default:
            UNREACHABLE();
            break;
    }
    Taskpool::GetCurrentTaskpool()->SetThreadPriority(mode);
#endif
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::UpdateBarrierEntrypoint(ark::common_vm::Mutator *mutator, GCPhase phase)
{
    if (phase >= GCPhase::GC_PHASE_INITIAL_MARK && phase < GCPhase::GC_PHASE_REMARK) {
        EnablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase >= GCPhase::GC_PHASE_REMARK && phase < GCPhase::GC_PHASE_PRECOPY) {
        DisableReadBarrier(static_cast<ark::Mutator *>(mutator));
        DisablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase >= GCPhase::GC_PHASE_PRECOPY && phase < GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE) {
        EnableReadBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE) {
        EnableReadBarrier(static_cast<ark::Mutator *>(mutator));
        EnablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase == GCPhase::GC_PHASE_IDLE) {
        DisableReadBarrier(static_cast<ark::Mutator *>(mutator));
        DisablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    }
    // Check if a new phase has been added
    ASSERT(phase == GCPhase::GC_PHASE_INITIAL_MARK || phase == GCPhase::GC_PHASE_MARK ||
           phase == GCPhase::GC_PHASE_PRECOPY || phase == GCPhase::GC_PHASE_COPY || phase == GCPhase::GC_PHASE_FIX ||
           phase == GCPhase::GC_PHASE_IDLE || phase == GCPhase::GC_PHASE_REMARK ||
           phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::OnMutatorTerminate(Mutator *mutator, [[maybe_unused]] mem::BuffersKeepingFlag keepBuffers)
{
    this->GetPandaVm()->GetMutatorManager()->UnregisterMutator(mutator, [this](Mutator *mutator) {
        DCHECK(static_cast<const ark::Mutator *>(mutator)->GetStatus() != ark::MutatorStatus::RUNNING);
        mutator->ReleaseAllocBuffer();

        UpdateBarrierEntrypoint(mutator, GCPhase::GC_PHASE_IDLE);
        mutator->SetMutatorPhase(GCPhase::GC_PHASE_IDLE);
        mutator->ResetMutator();
        return true;
    });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::OnMutatorCreate(Mutator *mutator)
{
    GCPhase phase = this->GetGCPhase();
    mutator->SetMutatorPhase(phase);
    // Enable pre write barrier for mutators created during concurrent marking and enable read barrier for mutators
    // created during concurrent copy/fix.
    if (phase >= GCPhase::GC_PHASE_INITIAL_MARK) {
        mutator->HandleGCPhase(phase);
        UpdateBarrierEntrypoint(mutator, phase);
    }
    GC::OnMutatorCreate(mutator);
}

template <class LanguageConfig>
const size_t CmcGC<LanguageConfig>::MAX_MARKING_WORK_SIZE = 16;  // fork task if bigger
template <class LanguageConfig>
const size_t CmcGC<LanguageConfig>::MIN_MARKING_WORK_SIZE = 8;  // forbid forking task if smaller

template <class LanguageConfig, bool ProcessXRef>
class ConcurrentMarkingTask : public Task {
public:
    ConcurrentMarkingTask(uint32_t id, CmcGC<LanguageConfig> &tc, ParallelMarkingMonitor &monitor,
                          GlobalMarkStack &globalMarkStack, GCTaskCause reason)
        : Task(id), collector_(tc), monitor_(monitor), globalMarkStack_(globalMarkStack), reason_(reason)
    {
    }

    ~ConcurrentMarkingTask() override = default;

    // run concurrent marking task.
    bool Run([[maybe_unused]] uint32_t threadIndex) override
    {
        ParallelLocalMarkStack markStack(&globalMarkStack_, &monitor_);
        do {
            if (!monitor_.TryStartStep()) {
                break;
            }
            collector_.template ProcessMarkStack<ProcessXRef>(threadIndex, markStack, reason_);
            monitor_.FinishStep();
        } while (monitor_.WaitNextStepOrFinished());
        monitor_.NotifyFinishOne();
        return true;
    }

private:
    CmcGC<LanguageConfig> &collector_;
    ParallelMarkingMonitor &monitor_;
    GlobalMarkStack &globalMarkStack_;
    GCTaskCause reason_;
};

template <class LanguageConfig>
template <bool HANDLE_WEAK_REFS, typename ObjectMarkerT>
void CmcGC<LanguageConfig>::HandleMarkedObject(ObjectMarkerT *handler, BaseObject *object)
{
    if constexpr (HANDLE_WEAK_REFS) {
        if (this->IsWeakReference(object)) {
            this->HandleWeakReference(handler, object);
            return;
        }
    }
    ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(object->ClassAddr<Class>(), object, handler);
}

static bool IsFullCollection(GCTaskCause reason)
{
    return reason == GCTaskCause::EXPLICIT_CAUSE || reason == GCTaskCause::OOM_CAUSE;
}

template <class LanguageConfig>
template <bool ProcessXRef>
void CmcGC<LanguageConfig>::ProcessMarkStack([[maybe_unused]] uint32_t threadIndex, ParallelLocalMarkStack &markStack,
                                             GCTaskCause reason)
{
    size_t nNewlyMarked = 0;
    PandaStack<BaseObject *> remarkStack;
    auto fetchFromSatbBuffer = [this, &markStack, &remarkStack]() {
        SatbBuffer::Instance().TryFetchOneRetiredNode(remarkStack);
        bool needProcess = false;
        while (!remarkStack.empty()) {
            BaseObject *obj = remarkStack.top();
            remarkStack.pop();
            if (Heap::IsHeapAddress(obj) && (MarkObjectIfNotMarked(obj))) {
                markStack.Push(obj);
                needProcess = true;
                LOG(DEBUG, GC) << "tracing take from satb buffer: obj " << obj;
            }
        }
        return needProcess;
    };
    size_t iterationCnt = 0;
    constexpr size_t maxIterationLoopNum = 1000;

    ObjectMarker<true> marker(&markStack, reason);

    // loop until work stack empty.
    while (true) {
        BaseObject *object;
        while (markStack.Pop(&object)) {
            ++nNewlyMarked;
            auto *region = RegionDesc::GetAliveRegionDescAt(object);
            auto objSize = object->GetSize();
            if (IsFullCollection(reason)) {
                HandleMarkedObject<true>(&marker, object);
            } else {
                HandleMarkedObject<false>(&marker, object);
            }
            region->AddLiveByteCount(objSize);
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if constexpr (ProcessXRef) {
                MarkingObjectXRef(object, markStack);
            }
#endif
        }
        // Try some task from satb buffer, bound the loop to make sure it converges in time
        if (++iterationCnt >= maxIterationLoopNum) {
            break;
        }
        if (!fetchFromSatbBuffer()) {
            break;
        }
    }
    DCHECK(markStack.IsEmpty());
    // newly marked statistics.
    // Atomic with relaxed order reason: data race with markedObjectCount_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    markedObjectCount_.fetch_add(nNewlyMarked, std::memory_order_relaxed);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::TracingSerial(GlobalMarkStack &globalMarkStack, GCTaskCause reason)
{
    // serial marking with a single mark task.
    // NOTE: this `ParallelLocalMarkStack` could be replaced with `SequentialLocalMarkStack`, and no need to
    // use monitor, but this need to add template param to `ProcessMarkStack`.
    // So for convenience just use a fake dummy parallel one.
    ParallelMarkingMonitor dummyMonitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &dummyMonitor);
#ifdef PANDA_JS_ETS_HYBRID_MODE
    if (reason == GCTaskCause::CROSSREF_CAUSE) {
        ProcessMarkStack<true>(0, markStack, reason);
    } else {
        ProcessMarkStack<false>(0, markStack, reason);
    }
#else
    ProcessMarkStack<false>(0, markStack, reason);
#endif
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::TracingImpl(GlobalMarkStack &globalMarkStack, bool parallel, bool Remark,
                                        GCTaskCause reason)
{
    ScopedTrace tracer(("TracingImpl_" + ToPandaString(globalMarkStack.Count())).c_str(),
                       ark::common_vm::ENABLE_GC_TRACING);

    // enable parallel marking if we have thread pool.
    auto *threadPool = GetThreadPool();
    ASSERT_PRINT(threadPool != nullptr, "thread pool is null");
    if (parallel) {  // parallel marking.
        uint32_t parallelCount = 0;
        // During the STW remark phase, Expect it to utilize all GC threads.
        if (Remark) {
            parallelCount = GetGCThreadCount(true);
        } else {
            parallelCount = GetGCThreadCount(true) - 1;
        }
        ParallelMarkingMonitor monitor(parallelCount, parallelCount);
        for (uint32_t i = 0; i < parallelCount; ++i) {
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if (reason == GCTaskCause::CROSSREF_CAUSE) {
                threadPool->PostTask(MakePandaUnique<ConcurrentMarkingTask<LanguageConfig, true>>(
                    0, *this, monitor, globalMarkStack, reason));
            } else {
                threadPool->PostTask(MakePandaUnique<ConcurrentMarkingTask<LanguageConfig, false>>(
                    0, *this, monitor, globalMarkStack, reason));
            }
#else
            threadPool->PostTask(MakePandaUnique<ConcurrentMarkingTask<LanguageConfig, false>>(
                0, *this, monitor, globalMarkStack, reason));
#endif
        }
        ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
        do {
            if (!monitor.TryStartStep()) {
                break;
            }
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if (reason == GCTaskCause::CROSSREF_CAUSE) {
                ProcessMarkStack<true>(0, markStack, reason);
            } else {
                ProcessMarkStack<false>(0, markStack, reason);
            }
#else
            ProcessMarkStack<false>(0, markStack, reason);
#endif
            monitor.FinishStep();
        } while (monitor.WaitNextStepOrFinished());
        monitor.WaitAllFinished();
    } else {
        TracingSerial(globalMarkStack, reason);
    }
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsWeakReference(BaseObject *obj)
{
    return this->GetReferenceProcessor()->IsReference(obj->ClassAddr<BaseClass>(), obj,
                                                      GC::EmptyReferenceProcessPredicate);
}

template <class LanguageConfig>
template <typename ObjectMarkerT>
void CmcGC<LanguageConfig>::HandleWeakReference(ObjectMarkerT *handler, BaseObject *weakRef)
{
    this->GetReferenceProcessor()->HandleReference(
        this, weakRef->ClassAddr<BaseClass>(), weakRef, [weakRef, handler](void *ref) {
            handler->ProcessObjectPointer(weakRef, reinterpret_cast<ObjectPointerType *>(ref));
        });
}

template <class LanguageConfig>
template <bool HANDLE_WEAK_REFS>
bool CmcGC<LanguageConfig>::PushRootToWorkStack(LocalCollectStack &collectStack, ObjectHeader *obj, GCTaskCause reason)
{
    BaseObject *baseObj = static_cast<BaseObject *>(obj);
    RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
    if (reason == GCTaskCause::YOUNG_GC_CAUSE && !regionInfo->IsInYoungSpace()) {
        LOG(DEBUG, GC) << "enum: skip old object " << obj << "<" << baseObj->GetTypeInfo() << ">(" << baseObj->GetSize()
                       << ")";
        return false;
    }

    // inline MarkObject
    bool marked = regionInfo->MarkObject(baseObj);
    if (!marked) {
        DCHECK(!regionInfo->IsGarbageRegion());
        LOG(DEBUG, GC) << "mark obj " << obj << "<" << baseObj->GetTypeInfo() << ">(" << baseObj->GetSize()
                       << ") in region " << regionInfo << "(" << static_cast<size_t>(regionInfo->GetRegionType())
                       << ")@0x" << std::hex << regionInfo->GetRegionStart() << std::dec << ", live "
                       << regionInfo->GetLiveByteCount();
        if constexpr (HANDLE_WEAK_REFS) {
            ObjectMarker<false> marker(&collectStack, reason);
            if (IsWeakReference(baseObj)) {
                HandleWeakReference(&marker, baseObj);
                regionInfo->AddLiveByteCount(baseObj->GetSize());
                return true;
            }
        }
        collectStack.Push(baseObj);
        return true;
    } else {
        return false;
    }
}

template <class LanguageConfig>
template <bool HANDLE_WEAK_REFS>
void CmcGC<LanguageConfig>::PushRootsToWorkStack(LocalCollectStack &collectStack,
                                                 const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason)
{
    ScopedTrace tracer(("PushRootsToWorkStack_" + ToPandaString(collectedRoots.size())).c_str(),
                       ark::common_vm::ENABLE_GC_TRACING);

    for (ObjectHeader *obj : collectedRoots) {
        PushRootToWorkStack<HANDLE_WEAK_REFS>(collectStack, obj, reason);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkingRoots(const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason)
{
    ScopedTrace tracer("MarkingRoots", ark::common_vm::ENABLE_GC_TRACING);

    GlobalMarkStack globalMarkStack;

    {
        LocalCollectStack collectStack(&globalMarkStack);

        if (IsFullCollection(reason)) {
            PushRootsToWorkStack<true>(collectStack, collectedRoots, reason);
        } else {
            PushRootsToWorkStack<false>(collectStack, collectedRoots, reason);
        }

        if (reason == GCTaskCause::YOUNG_GC_CAUSE) {
            ScopedTrace tracer("PushRootInRSet", ark::common_vm::ENABLE_GC_TRACING);
            auto func = [this, &collectStack](BaseObject *object) { MarkRememberSetImpl(object, collectStack); };
            RegionalHeap &space = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
            space.MarkRememberSet(func);
        }

        collectStack.Publish();
    }

    mem::GCScope<mem::TRACE_TIMING> gcScope("MarkingRoots", this);

    ASSERT_PRINT(GetThreadPool() != nullptr, "null thread pool");

    // use fewer threads and lower priority for concurrent mark.
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;

    {
        mem::GCScope<mem::TRACE_TIMING> gcScope("Concurrent marking", this);
        TracingImpl(globalMarkStack, maxWorkers > 0, false, reason);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::Remark(GCTaskCause reason)
{
    GlobalMarkStack globalMarkStack;
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;

    mem::GCScope<mem::TRACE_TIMING> gcScope("STW re-marking", this);
    ConcurrentRemark(globalMarkStack, maxWorkers > 0);  // Mark enqueue
    TracingImpl(globalMarkStack, maxWorkers > 0, true, reason);
    ProcessWeakReferences(GCPhase::GC_PHASE_REMARK, reason);
    PreforwardStaticRoots();
    // clear satb buffer when gc finish tracing.
    SatbBuffer::Instance().ClearBuffer();

    // Atomic with relaxed order reason: data race with markedObjectCount_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    LOG(DEBUG, GC) << "mark " << markedObjectCount_.load(std::memory_order_relaxed) << " objects";

    WVerify::VerifyAfterMark(this->GetGCPhase(), true);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessWeakReferences(GCPhase phase, GCTaskCause reason)
{
    ScopedTrace tracer("ProcessWeakReferences", ark::common_vm::ENABLE_GC_TRACING);
    if (reason == GCTaskCause::YOUNG_GC_CAUSE) {
        return;
    }
    this->GetReferenceProcessor()->ProcessReferences(false, false, phase, GC::EmptyReferenceProcessPredicate);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::MarkSatbBuffer(GlobalMarkStack &globalMarkStack)
{
    mem::GCScope<mem::TRACE_TIMING> gcScope("MarkSatbBuffer", this);
    auto visitSatbObj = [this, &globalMarkStack]() {
        PandaStack<BaseObject *> remarkStack;
        auto func = [&remarkStack](Mutator *mutator) {
            const SatbBuffer::TreapNode *node =
                static_cast<const SatbBuffer::TreapNode *>(mutator->GetSatbBufferNode());
            if (node != nullptr) {
                const_cast<SatbBuffer::TreapNode *>(node)->GetObjects(remarkStack);
            }
        };
        ForEachManagedMutator(func);
        SatbBuffer::Instance().GetRetiredObjects(remarkStack);

        LocalCollectStack collectStack(&globalMarkStack);
        while (!remarkStack.empty()) {  // LCOV_EXCL_BR_LINE
            BaseObject *obj = remarkStack.top();
            remarkStack.pop();
            if (Heap::IsHeapAddress(obj) && this->MarkObjectIfNotMarked(obj)) {
                collectStack.Push(obj);
                LOG(DEBUG, GC) << "satb buffer add obj " << obj;
            }
        }
        collectStack.Publish();
    };

    visitSatbObj();
    return true;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkRememberSetImpl(BaseObject *object, LocalCollectStack &collectStack)
{
    auto fieldHandler = [this, &collectStack, &object](RefField<> &field) {
        (void)object;
        BaseObject *targetObj = field.GetTargetObject();
        if (Heap::IsHeapAddress(targetObj)) {
            RegionDesc *region = RegionDesc::GetAliveRegionDescAt(targetObj);
            if (region->IsInYoungSpace() && !region->IsNewObjectSinceMarking(targetObj) &&
                this->MarkObjectIfNotMarked(targetObj)) {
                collectStack.Push(targetObj);
                LOG(DEBUG, GC) << "remember set marking obj: " << object << "@" << &field << ", ref: " << targetObj;
            }
        }
    };
    object->ForEachRefField(fieldHandler, fieldHandler);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ConcurrentRemark(GlobalMarkStack &globalMarkStack, bool parallel)
{
    LOG_IF(UNLIKELY(!MarkSatbBuffer(globalMarkStack)), FATAL, GC) << "not cleared\n";
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreGarbageCollection(GCTaskCause reason, [[maybe_unused]] bool isConcurrent)
{
    // SatbBuffer should be initialized before concurrent enumeration.
    SatbBuffer::Instance().Init();
    // prepare thread pool.

    gcReason_ = reason;
    collectedBytes_ = 0;
    gcStartTime_ = ark::common_vm::TimeUtil::NanoSeconds();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PostGarbageCollection(uint64_t gcIndex)
{
    SatbBuffer::Instance().ReclaimALLPages();
    // release pages in PagePool
    ark::common_vm::PagePool::Instance().Trim();
    collectorResources_.MarkGCFinish(gcIndex);
}

template <class LanguageConfig>
// CC-OFFNXT(G.FUD.05) solid logic
void CmcGC<LanguageConfig>::UpdateGCStats()
{
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);

    size_t oldThreshold = cmcAllocator_->GetHeapThreshold();
    size_t oldTargetFootprint = targetFootprint_;
    size_t recentBytes = space.GetRecentAllocatedSize();
    size_t survivedBytes = space.GetSurvivedSize();
    size_t bytesAllocated = space.GetAllocatedBytes();

    size_t targetSize;
    Heap *heap = &common_vm::Heap::GetHeap();
    const HeapParam &heapParam = heap->GetHeapParam();
    GCParam &gcParam = heap->GetGCParam();
    if (!IsYoungGC()) {
        cmcAllocator_->SetShouldRequestYoung(true);
        size_t delta = bytesAllocated * (1.0 / heapParam.heapUtilization - 1.0);
        size_t growBytes = std::min(delta, gcParam.maxGrowBytes);
        growBytes = std::max(growBytes, gcParam.minGrowBytes);
        targetSize = bytesAllocated + growBytes * gcParam.multiplier;
    } else {
        auto shouldRequestYoung = cmcAllocator_->GetCollectionRate() * gcParam.ygcRateAdjustment >= fullGCMeanRate_ &&
                                  bytesAllocated <= oldThreshold;
        cmcAllocator_->SetShouldRequestYoung(shouldRequestYoung);
        size_t adjustMaxGrowBytes = gcParam.maxGrowBytes * gcParam.multiplier;
        if (bytesAllocated + adjustMaxGrowBytes < oldTargetFootprint) {
            targetSize = bytesAllocated + adjustMaxGrowBytes;
        } else {
            targetSize = std::max(bytesAllocated, oldTargetFootprint);
        }
    }

    targetFootprint_ = targetSize;
    size_t remainingBytes = recentBytes;
    remainingBytes = std::min(remainingBytes, gcParam.kMaxConcurrentRemainingBytes);
    remainingBytes = std::max(remainingBytes, gcParam.kMinConcurrentRemainingBytes);
    if (UNLIKELY(remainingBytes > targetFootprint_)) {
        remainingBytes = std::min(gcParam.kMinConcurrentRemainingBytes, targetFootprint_);
    }
    size_t heapThreshold = std::max(targetFootprint_ - remainingBytes, bytesAllocated);
    heapThreshold = std::max(heapThreshold, 20 * MB);  // 20 MB:set 20 MB as min heapThreshold
    heapThreshold = std::min(heapThreshold, gcParam.gcThreshold);
    cmcAllocator_->SetHeapThreshold(heapThreshold);

    UpdateNativeThreshold(gcParam);
    Heap::GetHeap().RecordAliveSizeAfterLastGC(bytesAllocated);
    if (!IsYoungGC()) {
        Heap::GetHeap().SetRecordHeapObjectSizeBeforeSensitive(bytesAllocated);
    }

    PandaOStringStream oss;
    oss << "allocated bytes " << bytesAllocated << " (survive bytes " << survivedBytes << ", recent-allocated "
        << recentBytes << "), update target footprint " << oldTargetFootprint << " -> " << targetFootprint_
        << ", update gc threshold " << oldThreshold << " -> " << heapThreshold << ", native size "
        << Heap::GetHeap().GetNotifiedNativeSize() << ", new native threshold "
        << Heap::GetHeap().GetNativeHeapThreshold();
    LOG(DEBUG, GC) << oss.str();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::UpdateNativeThreshold(GCParam &gcParam)
{
    size_t nativeHeapSize = Heap::GetHeap().GetNotifiedNativeSize();
    size_t newNativeHeapThreshold = Heap::GetHeap().GetNotifiedNativeSize();
    if (nativeHeapSize < ark::common_vm::MAX_NATIVE_SIZE_INC) {
        newNativeHeapThreshold =
            std::max(nativeHeapSize + gcParam.minGrowBytes, nativeHeapSize * ark::common_vm::NATIVE_MULTIPLIER);
    } else {
        newNativeHeapThreshold += ark::common_vm::MAX_NATIVE_STEP;
    }
    newNativeHeapThreshold = std::min(newNativeHeapThreshold, ark::common_vm::MAX_GLOBAL_NATIVE_LIMIT);
    Heap::GetHeap().SetNativeHeapThreshold(newNativeHeapThreshold);
    collectorResources_.SetIsNativeGCInvoked(false);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::CopyObject(const BaseObject &fromObj, BaseObject &toObj, size_t size) const
{
    uintptr_t from = reinterpret_cast<uintptr_t>(&fromObj);
    uintptr_t to = reinterpret_cast<uintptr_t>(&toObj);
    LOG_IF(UNLIKELY(memmove_s(reinterpret_cast<void *>(to), size, reinterpret_cast<void *>(from), size) != EOK), ERROR,
           GC)
        << "memmove_s fail";
#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanFixShadow(reinterpret_cast<void *>(from), reinterpret_cast<void *>(to), size);
#endif
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ReclaimGarbageMemory(GCTaskCause reason)
{
    if (reason == GCTaskCause::OOM_CAUSE) {
        Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(true);
    } else {
        Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(false);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RunGarbageCollection(uint64_t gcIndex, ark::GCTask &task)
{
    MarkGCStart();
    gcType_ = task.collectionType;
    auto currentAllocatedSize = Heap::GetHeap().GetAllocatedSize();
    auto currentThreshold = cmcAllocator_->GetHeapThreshold();
    LOG(DEBUG, GC) << "Begin GC log. GCType: " << task.collectionType << ", Current allocated "
                   << common_vm::TimeUtil::PrettyDigitsFormat(currentAllocatedSize) << ", Current threshold "
                   << common_vm::TimeUtil::PrettyDigitsFormat(currentThreshold) << ", gcIndex=" << gcIndex
                   << ", Sensitive " + ToPandaString(static_cast<int>(Heap::GetHeap().GetSensitiveStatus()))
                   << ", Startup " + ToPandaString(static_cast<int>(Heap::GetHeap().GetStartupStatus()))
                   << ", Current Native " +
                          common_vm::TimeUtil::PrettyDigitsFormat(Heap::GetHeap().GetNotifiedNativeSize())
                   << ", NativeThreshold " +
                          common_vm::TimeUtil::PrettyDigitsFormat(Heap::GetHeap().GetNativeHeapThreshold());
    PreGarbageCollection(task.reason, true);
    Heap::GetHeap().SetGCReason(task.reason);

    DoGarbageCollection(task);

    HeapBitmapManager::GetHeapBitmapManager().ClearHeapBitmap();

    ReclaimGarbageMemory(task.reason);

    PostGarbageCollection(gcIndex);

    UpdateGCCompletionStats();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::UpdateGCCompletionStats()
{
    uint64_t gcEndTime = ark::common_vm::TimeUtil::NanoSeconds();
    uint64_t gcTimeNs = gcEndTime - gcStartTime_;
    double rate =
        (static_cast<double>(collectedBytes_) / gcTimeNs) * (static_cast<double>(ark::common_vm::NS_PER_S) / MB);
    {
        PandaOStringStream oss;
        const int prec = 3;
        oss << "total gc time: " << common_vm::TimeUtil::PrettyDigitsFormat(gcTimeNs / ark::common_vm::NS_PER_US)
            << " us, collection rate ";
        oss << std::setprecision(prec) << rate << " MB/s";
        LOG(DEBUG, GC) << oss.str();
    }

    cmcAllocator_->SetCollectionRate(rate);

    if (!IsYoungGC()) {
        if (fullGCCount_ == 0) {
            fullGCMeanRate_ = rate;
        } else {
            fullGCMeanRate_ = (fullGCMeanRate_ * fullGCCount_ + rate) / (fullGCCount_ + 1);
        }
        fullGCCount_++;
    }

    UpdateGCStats();

    if (Heap::GetHeap().GetForceThrowOOM()) {
        // NOTE (shemetov.philip, #34958) Workaround to fix OOM for GC with JIT enabled
        RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
        size_t maxCapacity = space.GetMaxCapacity();
        if (cmcAllocator_->GetLiveBytesAfterGC() * 2U > maxCapacity) {
            Heap::throwOOM();
        } else {
            Heap::GetHeap().SetForceThrowOOM(false);
        }
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::CopyFromSpace()
{
    ScopedTrace tracer("CopyFromSpace", ark::common_vm::ENABLE_GC_TRACING);
    {
        ScopedStopTheWorld stw;
        TransitionToGCPhase(GCPhase::GC_PHASE_COPY);
    }
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    space.CopyFromSpace(GetThreadPool());

    collectedBytes_ += space.FromRegionSize() - space.ToSpaceSize();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ExemptFromSpace()
{
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    space.ExemptFromSpace();
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsMarkedObject(const BaseObject *obj) const
{
    return RegionalHeap::IsMarkedObject(obj);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsToObject(const BaseObject *obj) const
{
    return RegionDesc::GetAliveRegionDescAt(obj)->IsToRegion();
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsHeapMarked() const
{
    return collectorResources_.IsHeapMarked();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::SetHeapMarked(bool value)
{
    collectorResources_.SetHeapMarked(value);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::SetGcStarted(bool val)
{
    collectorResources_.SetGcStarted(val);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkGCStart()
{
    collectorResources_.MarkGCStart();
}
template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkGCFinish(uint64_t gcIndex)
{
    collectorResources_.MarkGCFinish(gcIndex);
}
template <class LanguageConfig>
uint32_t CmcGC<LanguageConfig>::GetGCThreadCount(const bool isConcurrent) const
{
    return collectorResources_.GetGCThreadCount(isConcurrent);
}
template <class LanguageConfig>
ark::common_vm::Taskpool *CmcGC<LanguageConfig>::GetThreadPool() const
{
    return collectorResources_.GetThreadPool();
}

template <class LanguageConfig>
CmcGC<LanguageConfig>::CmcGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GCLang<LanguageConfig>(objectAllocator, settings),
      Collector(),
      theAllocator_(Heap::GetHeap().GetAllocator()),
      collectorResources_(Heap::GetHeap().GetCollectorResources())
{
    auto &heap = Heap::GetHeap();
    this->SetType(GCType::CMC_GC);
    this->SetTLABsSupported();
    heap.SetCollector(this);

    cmcAllocator_ = static_cast<CMCObjectAllocator *>(objectAllocator);
    cmcAllocator_->SetNeedToTrackFreedObjects(settings.G1TrackFreedObjects());

    size_t heapThreshold = std::min(heap.GetGCParam().gcThreshold, 20 * MB);  // 20 MB initial threshold
    heapThreshold = std::min(static_cast<size_t>(heap.GetMaxCapacity() * 0.2F), heapThreshold);
    cmcAllocator_->SetHeapThreshold(heapThreshold);
    targetFootprint_ = heapThreshold;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::InitializeImpl()
{
    ark::mem::InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrierSet =
        allocator->New<GCCMCBarrierSet>(allocator, ark::helpers::math::GetIntLog2(ark::mem::RegionDesc::UNIT_SIZE));
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);
    this->CreateWorkersTaskPool();
    LOG(DEBUG, GC) << "CMC GC adapter initialized...";
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RunPhasesImpl(ark::GCTask &task)
{
    LOG(DEBUG, GC) << "CMC GC adapter RunPhases...";
    if (task.collectionType == GCCollectionType::NONE) {
        task.UpdateGCCollectionType(task.reason == GCTaskCause::YOUNG_GC_CAUSE ? GCCollectionType::YOUNG
                                                                               : GCCollectionType::FULL);
    }
    ConcurrentScope concurrentScope(this);
    RunGarbageCollection(ark::common_vm::GCTask::TASK_INDEX_SYNC_GC_MIN, task);
}

// NOLINTNEXTLINE(misc-unused-parameters)
template <class LanguageConfig>
bool CmcGC<LanguageConfig>::WaitForGC(ark::GCTask task)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    ark::common_vm::ScopedGcThreadType scopedGcThreadType;
    return GC::WaitForGC(task);
#endif  // ARK_USE_COMMON_RUNTIME
    return false;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::InitGCBits([[maybe_unused]] ark::ObjectHeader *objHeader)
{
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] ark::ObjectHeader *objHeader)
{
    LOG(FATAL, GC) << "TLABs are not supported by this GC";
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::Trigger(ark::PandaUniquePtr<ark::GCTask> task)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    return this->AddGCTask(true, std::move(task));
#endif  // ARK_USE_COMMON_RUNTIME
    return false;
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsPostponeGCSupported() const
{
    return false;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkObject(ObjectHeader *object)
{
    MarkObjectIfNotMarked(object);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::MarkObjectIfNotMarked(ObjectHeader *object)
{
    auto *obj = reinterpret_cast<BaseObject *>(object);
    bool marked = RegionalHeap::MarkObject(obj);
    if (!marked) {
        [[maybe_unused]] RegionDesc *region = RegionDesc::GetAliveRegionDescAt(obj);
        DCHECK(!region->IsGarbageRegion());
        LOG(DEBUG, GC) << "mark obj " << GetDebugInfoAboutObject(object);
    }
    return !marked;
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsMarked(const ark::ObjectHeader *object) const
{
    return RegionalHeap::IsMarkedObject(static_cast<const BaseObject *>(object));
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkReferences([[maybe_unused]] ark::mem::GCMarkingStackType *references,
                                           [[maybe_unused]] ark::mem::GCPhase gcPhase)
{
}

// Helper for root visitors
GCRootVisitor MakeCallback(const RefFieldVisitor &visitor)
{
    return [visitor](mem::GCRoot root) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        visitor(reinterpret_cast<ark::mem::RefField<> &>(*root.GetObjectPointer()));
    };
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::VisitRoots(const GCRootVisitor &visitor)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    rootManager.VisitNonHeapRoots(visitor);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::VisitSTWRoots(const RefFieldVisitor &visitor)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    const GCRootVisitor &callback = MakeCallback(visitor);
    rootManager.VisitAotStringRoots(callback, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitVmRoots(callback);
    rootManager.VisitClassRoots(callback, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitClassLinkerContextRoots(callback);
    vm->VisitStringTable(callback, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::VisitConcurrentRoots(const GCRootVisitor &visitor)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    rootManager.VisitClassRoots(visitor, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitClassLinkerContextRoots(visitor);
    vm->VisitStringTable(visitor, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::VisitWeakRoots(const WeakRefFieldVisitor &visitor)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    const GCRootVisitor &callback = MakeCallback(visitor);

    rootManager.VisitNonHeapRoots(callback, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.UpdateAndSweep([&visitor](ObjectHeader **ref) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        return (visitor(reinterpret_cast<ark::mem::RefField<> &>(*ref))) ? ObjectStatus::ALIVE_OBJECT
                                                                         : ObjectStatus::ALIVE_OBJECT;
    });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::VisitGlobalRoots(const GCRootVisitor &visitor)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    rootManager.VisitAotStringRoots(visitor, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitVmRoots(visitor);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::VisitWeakGlobalRoots(const WeakRefFieldVisitor &visitor, bool isYoung)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    const GCRootVisitor &callback = MakeCallback(visitor);
    rootManager.VisitAotStringRoots(callback, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitVmRoots(callback);
    rootManager.UpdateAndSweep([&visitor](ObjectHeader **ref) {
        static_assert(sizeof(ObjectPointerType) == sizeof(uintptr_t));
        return (visitor(reinterpret_cast<ark::mem::RefField<> &>(*ref))) ? ObjectStatus::ALIVE_OBJECT
                                                                         : ObjectStatus::ALIVE_OBJECT;
    });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::WorkerTaskProcessing(GCWorkersTask *task, [[maybe_unused]] void *workerData)
{
    common_vm::ScopedGcThreadType scopedGcThreadType;
    auto *refStack = task->Cast<CMCGCWorkersTask>()->GetStack();
    switch (task->GetType()) {
        case GCWorkersTaskTypes::TASK_REMARK: {
            MarkEvacuationStack(*refStack);
            break;
        }
        case GCWorkersTaskTypes::TASK_EVACUATE_REGIONS: {
            ProcessEvacuationStack(*refStack);
            break;
        }
        default:
            LOG(FATAL, GC) << "Unimplemented for " << GCWorkersTaskTypesToString(task->GetType());
            UNREACHABLE();
    }
    ASSERT(refStack->Empty());
    this->GetInternalAllocator()->Delete(refStack);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(CmcGC);

}  // namespace ark::mem

#endif  // #if defined(ARK_USE_COMMON_RUNTIME)
