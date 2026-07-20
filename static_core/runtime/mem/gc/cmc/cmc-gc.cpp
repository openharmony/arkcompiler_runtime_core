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
#include "runtime/mem/gc/cmc/heap/heap_manager.h"
#include "runtime/mem/gc/gc_adaptive_stack_inl.h"
#include "runtime/mem/gc/gc_stats.h"

#include "common_components/base/globals.h"
#include "runtime/mem/gc/cmc/heap/verification.h"
#include "runtime/mem/gc/cmc/heap/allocator/fix_heap.h"
#include "runtime/mem/gc/cmc/heap/allocator/regional_heap.h"
#include "runtime/mem/gc/cmc/heap/allocator/alloc_buffer.h"
#include "runtime/mem/gc/cmc/heap/collector/heuristic_gc_policy.h"
#include "common_interfaces/base/runtime_param.h"
#include "runtime/mem/gc/cmc/heap/collector/collector_resources.h"
#include "common_components/mutator/satb_buffer.h"

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/math_helpers.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/trace.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/object-references-iterator-inl.h"

#include "runtime/mem/gc/cmc/heap/region_desc.h"

#ifdef ENABLE_QOS
#include "qos.h"
#endif

namespace ark::mem {
using ark::common_vm::AllocationBuffer;
using ark::common_vm::MB;
using ark::common_vm::PriorityMode;
using ark::common_vm::RegionalHeap;
using ark::common_vm::SatbBuffer;
using ark::common_vm::WVerify;

static BaseObject *ReadRefSlot(ObjectPointerType *ref)
{
    auto value = ObjectAccessor::Load(ref);
    return reinterpret_cast<BaseObject *>(static_cast<uintptr_t>(value));
}

static BaseObject *AtomicReadRefSlot(ObjectPointerType *ref)
{
    auto value = AtomicLoad(ref, std::memory_order_relaxed);
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

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixRefField(ObjectHeader *obj, ObjectPointerType *field)
{
    ObjectHeader *targetObj = AtomicReadRefSlot(field);

    // target object could be null or non-heap for some static variable.
    if (!Heap::IsHeapAddress(targetObj)) {
        return;
    }

    auto *refRegion = RegionDesc::GetRegionDescAt(targetObj);
    bool isFrom = refRegion->IsFromRegion();
    bool isInRecent = refRegion->IsInRecentSpace();
    if (isInRecent) {
        auto *objRegion = RegionDesc::GetRegionDescAt(obj);
        if (!objRegion->IsInRecentSpace() && objRegion->MarkRSetCardTable(obj)) {
            LOG(DEBUG, GC) << "fix phase update point-out remember set of region " << objRegion << ", obj " << obj
                           << ", ref: <" << static_cast<BaseObject *>(targetObj)->GetTypeInfo() << ">";
        }
        return;
    } else if (!isFrom) {
        return;
    }
    BaseObject *latest = FindToVersion(static_cast<BaseObject *>(targetObj));

    if (latest == nullptr) {
        return;
    }

    CHECK(latest->IsValidObject());
    if (CompareExchangeRefSlot(field, targetObj, latest)) {
        LOG(DEBUG, GC) << "fix obj " << obj << "+" << static_cast<BaseObject *>(obj)->GetSize() << " ref@" << field
                       << ": 0x" << std::hex << targetObj << " => " << std::dec << latest << "<"
                       << latest->GetTypeInfo() << ">(" << latest->GetSize() << ")";
        LOG(DEBUG, MM_OBJECT_EVENTS) << "[CMC] "
                                     << "FIX ref in obj " << obj << " field@" << field << ": " << targetObj << " -> "
                                     << latest;
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixObjectRefFields(BaseObject *obj) const
{
    LOG(DEBUG, GC) << "fix obj " << obj << "<" << obj->GetTypeInfo() << ">(" << obj->GetSize() << ")";

    auto handler = []([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p) {
        CmcGC<LanguageConfig>::FixRefField(obj, p);
        return true;
    };

    auto *cls = obj->template ClassAddr<Class>();
    mem::ObjectIterator<LanguageConfig::LANG_TYPE>::template Iterate<false>(cls, obj, handler);
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
CArrayList<ObjectHeader *> CmcGC<LanguageConfig>::EnumRoots()
{
    EnumRootsBuffer buffer;
    CArrayList<ObjectHeader *> *results = buffer.GetBuffer();
    GCRootVisitor visitor = [&results](GCRoot root) { results->push_back(root.GetObjectHeader()); };

    EnumRootsFlip(visitor);
    VisitConcurrentRoots(visitor);
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
WeakRefFieldVisitor CmcGC<LanguageConfig>::GetWeakRefFieldVisitor()
{
    return [this](ObjectPointerType *refField) -> bool {
        BaseObject *oldObj = ReadRefSlot(refField);
        if (!IsMarkedObject(oldObj) && !RegionalHeap::IsNewObjectSinceMarking(oldObj) && !IsToObject(oldObj)) {
            return false;
        }

        LOG(DEBUG, GC) << "visit weak raw-ref @" << refField << ": " << oldObj;
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK(toVersion != nullptr);
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (CompareExchangeRefSlot(refField, oldObj, toVersion)) {
                LOG(DEBUG, GC) << "fix weak raw-ref @" << refField << ": " << oldObj << " -> " << toVersion;
                LOG(DEBUG, MM_OBJECT_EVENTS)
                    << "[CMC] "
                    << "PREFORWARD weak ref @" << refField << ": " << oldObj << " -> " << toVersion;
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
        ASSERT(reason != GCTaskCause::YOUNG_GC_CAUSE);
        SetGCThreadQosPriority(ark::common_vm::PriorityMode::STW);
        ASSERT_PRINT(GetThreadPool() != nullptr, "thread pool is null");
        TransitionToGCPhase(GCPhase::GC_PHASE_REMARK);
        Remark(reason);
        reinterpret_cast<RegionalHeap &>(theAllocator_).PrepareForward();

        TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY);
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
        SetGCThreadQosPriority(ark::common_vm::PriorityMode::FOREGROUND);

        VisitWeakGlobalRoots(weakVisitor);

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

    auto useGcWorkers = this->GetSettings()->ParallelCompactingEnabled();
    uint32_t parallelCount = useGcWorkers ? GetGCThreadCount(true) : 1;
    PandaVector<common_vm::FixRegionResult> results(parallelCount);
    PandaVector<FixHeapWorkerData> workerData(parallelCount);
    std::atomic<size_t> taskIter {0};

    for (uint32_t i = 0; i < parallelCount; ++i) {
        workerData[i] = FixHeapWorkerData {&taskList, &taskIter, &results[i]};
    }

    for (uint32_t i = 1; useGcWorkers && i < parallelCount; ++i) {
        this->GetWorkersTaskPool()->AddTask(CmcGCFixTask(&workerData[i]));
    }

    DispatchFixRegionTasks(&workerData[0]);
    if (useGcWorkers) {
        this->GetWorkersTaskPool()->WaitUntilTasksEnd();
    }

    bool hasPostFixWorkerTasks = false;
    for (uint32_t i = 1; useGcWorkers && i < parallelCount; ++i) {
        if (results[i].monoSizeNonMovableGarbages.empty()) {
            continue;
        }
        this->GetWorkersTaskPool()->AddTask(CmcGCPostFixTask(&results[i]));
        hasPostFixWorkerTasks = true;
    }
    PostClearTask(&results[0]);
    size_t nonMovableGarbageSize = common_vm::PostFixHeap::CollectEmptyRegions();
    if (hasPostFixWorkerTasks) {
        this->GetWorkersTaskPool()->WaitUntilTasksEnd();
    }
    return nonMovableGarbageSize;
}

template <class LanguageConfig>
size_t CmcGC<LanguageConfig>::FixHeap()
{
    {
        ScopedStopTheWorld stw;
        TransitionToGCPhase(GCPhase::GC_PHASE_FIX);
    }
    mem::GCScope<mem::TRACE_TIMING> gcScope("FixHeap", this);

    ScopedTrace tracer("FixHeap", ark::common_vm::ENABLE_GC_TRACING);

    size_t nonMovableGarbageSize = ParallelFixHeap();

    WVerify::VerifyAfterFix(this->GetGCPhase(), false);

    return nonMovableGarbageSize;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::DispatchFixRegionTasks(FixHeapWorkerData *workerData)
{
    ASSERT(workerData != nullptr);
    ASSERT(workerData->taskList != nullptr);
    ASSERT(workerData->taskIter != nullptr);
    ASSERT(workerData->result != nullptr);

    auto *taskList = workerData->taskList;
    auto *result = workerData->result;
    for (;;) {
        // Atomic with relaxed order reason: data race with taskIter with no synchronization or ordering constraints
        // imposed on other reads or writes.
        size_t idx = workerData->taskIter->fetch_add(1U, std::memory_order_relaxed);
        if (idx >= taskList->size()) {
            break;
        }
        DispatchFixRegionTask((*taskList)[idx], result);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::DispatchFixRegionTask(const common_vm::FixTaskInfo &task,
                                                  common_vm::FixRegionResult *result)
{
    ASSERT(result != nullptr);

    result->numProcessedRegions++;
    auto region = task.GetRegion();
    switch (task.GetType()) {
        case common_vm::FixRegionType::FIX_OLD_REGION:
            FixOldRegion(region);
            break;
        case common_vm::FixRegionType::FIX_RECENT_OLD_REGION:
            FixRecentOldRegion(region);
            break;
        case common_vm::FixRegionType::FIX_RECENT_REGION:
            if (region->IsMonoSizeNonMovableRegion()) {
                FixRecentRegion<DeadObjectHandlerType::COLLECT_MONOSIZE_NONMOVABLE>(region, result);
            } else if (region->IsPolySizeNonMovableRegion()) {
                FixRecentRegion<DeadObjectHandlerType::COLLECT_POLYSIZE_NONMOVABLE>(region, result);
            } else if (region->IsLargeRegion()) {
                FixRecentRegion<DeadObjectHandlerType::IGNORED>(region, result);
            } else {
                FixRecentRegion<DeadObjectHandlerType::FILL_FREE>(region, result);
            }
            break;
        case common_vm::FixRegionType::FIX_REGION:
            if (region->IsMonoSizeNonMovableRegion()) {
                FixRegion<DeadObjectHandlerType::COLLECT_MONOSIZE_NONMOVABLE>(region, result);
            } else if (region->IsPolySizeNonMovableRegion()) {
                FixRegion<DeadObjectHandlerType::COLLECT_POLYSIZE_NONMOVABLE>(region, result);
            } else if (region->IsLargeRegion()) {
                FixRegion<DeadObjectHandlerType::IGNORED>(region, result);
            } else if (region->IsUnmovableFromRegion()) {
                region->ClearRSet();
                FixRegion<DeadObjectHandlerType::FILL_FREE>(region, result);
            } else {
                FixRegion<DeadObjectHandlerType::FILL_FREE>(region, result);
            }
            break;
        case common_vm::FixRegionType::FIX_TO_REGION:
            FixToRegion(region);
            break;
        default:
            UNREACHABLE();
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixOldRegion(RegionDesc *region)
{
    auto visitFunc = [this](BaseObject *object) {
        LOG(DEBUG, GC) << "fix: old obj " << object << "<" << object->GetTypeInfo() << ">(" << object->GetSize() << ")";
        FixObjectRefFields(object);
    };
    region->VisitRememberSet(visitFunc);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixRecentOldRegion(RegionDesc *region)
{
    auto visitFunc = [this](BaseObject *object) {
        LOG(DEBUG, GC) << "fix: old obj " << object << "<" << object->GetTypeInfo() << ">(" << object->GetSize() << ")";
        FixObjectRefFields(object);
    };
    region->VisitRememberSetBeforeCopy(visitFunc);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixToRegion(RegionDesc *region)
{
    region->VisitAllObjects([this](BaseObject *object) { FixObjectRefFields(object); });
}

template <class LanguageConfig>
template <typename CmcGC<LanguageConfig>::DeadObjectHandlerType type>
void CmcGC<LanguageConfig>::FixRegion(RegionDesc *region, common_vm::FixRegionResult *result)
{
    size_t cellCount = 0;
    if constexpr (type == DeadObjectHandlerType::COLLECT_MONOSIZE_NONMOVABLE) {
        cellCount = region->GetRegionCellCount();
    }

    region->VisitAllObjects([this, region, cellCount, result](BaseObject *object) {
        (void)region;
        (void)cellCount;
        (void)result;
        if (RegionalHeap::IsSurvivedObject(object)) {
            FixObjectRefFields(object);
        } else {
            if constexpr (type == DeadObjectHandlerType::COLLECT_MONOSIZE_NONMOVABLE) {
                result->monoSizeNonMovableGarbages.emplace_back(region, object, cellCount);
            } else if constexpr (type == DeadObjectHandlerType::COLLECT_POLYSIZE_NONMOVABLE) {
                result->polySizeNonMovableGarbages.emplace_back(object, RegionalHeap::GetAllocSize(*object));
            } else if constexpr (type == DeadObjectHandlerType::IGNORED) {
                /* Ignore */
            }
            LOG(DEBUG, GC) << "fix: skip dead obj " << object << "<" << object->GetTypeInfo() << ">("
                           << object->GetSize() << ")";
        }
    });
}

template <class LanguageConfig>
template <typename CmcGC<LanguageConfig>::DeadObjectHandlerType type>
void CmcGC<LanguageConfig>::FixRecentRegion(RegionDesc *region, common_vm::FixRegionResult *result)
{
    size_t cellCount = 0;
    if constexpr (type == DeadObjectHandlerType::COLLECT_MONOSIZE_NONMOVABLE) {
        cellCount = region->GetRegionCellCount();
    }

    region->VisitAllObjectsBeforeCopy([this, region, cellCount, result](BaseObject *object) {
        (void)cellCount;
        (void)result;
        if (region->IsNewObjectSinceMarking(object) || RegionalHeap::IsSurvivedObject(object)) {
            FixObjectRefFields(object);
        } else {  // handle dead objects in tl-regions for concurrent gc.
            if constexpr (type == DeadObjectHandlerType::COLLECT_MONOSIZE_NONMOVABLE) {
                result->monoSizeNonMovableGarbages.emplace_back(region, object, cellCount);
            } else if constexpr (type == DeadObjectHandlerType::COLLECT_POLYSIZE_NONMOVABLE) {
                result->polySizeNonMovableGarbages.emplace_back(object, RegionalHeap::GetAllocSize(*object));
            } else if constexpr (type == DeadObjectHandlerType::IGNORED) {
                /* Ignore */
            }
            LOG(DEBUG, GC) << "skip dead obj " << object << "<" << object->GetTypeInfo() << ">(" << object->GetSize()
                           << ")";
        }
    });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PostClearTask(common_vm::FixRegionResult *result)
{
    ASSERT(result != nullptr);

    for (auto [region, object, cellCount] : result->monoSizeNonMovableGarbages) {
        region->CollectNonMovableGarbage(object, cellCount);
    }
    LOG(DEBUG, GC) << "Fix heap worker processed " << result->numProcessedRegions << " Regions, "
                   << result->monoSizeNonMovableGarbages.size() << " monoSizeNonMovableGarbages, "
                   << result->polySizeNonMovableGarbages.size() << " polySizeNonMovableGarbages";
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

    if (!isYoungGC) {
        auto collectedRoots = EnumRoots();
        MarkingHeap(collectedRoots, task.reason);
        PreforwardFlip(task.reason);
        ConcurrentPreforward();
        // reclaim large objects should after preforward(may process weak ref)
        // and before fix heap(may clear live bit)
        CollectLargeGarbage();

        CopyFromSpace();
        WVerify::VerifyAfterForward(this->GetGCPhase(), false);

        size_t nonMovableGarbageSize = FixHeap();

        CollectNonMovableGarbage(nonMovableGarbageSize);
    } else {
        DoGarbageCollectionWithoutConcurrentMarking();
    }

    {
        ScopedStopTheWorld stw;
        TransitionToGCPhase(GCPhase::GC_PHASE_RUNNING);
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
    CmcGCEvacuationStack evacuationStack(this, useGcWorkers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
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

    FixHeap();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardNonHeapRoots(CmcGCEvacuationStack &stack)
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
    CmcGCMarkingStack remarkStack(this, useGcWorkers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                  useGcWorkers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                  GCWorkersTaskTypes::TASK_REMARK,
                                  this->GetSettings()->GCMarkingStackNewTasksFrequency());
    RemarkNonHeapRoots(remarkStack);
    MarkSatbBufferYoung(remarkStack);
    MarkEvacuationStack(remarkStack);
    if (useGcWorkers) {
        this->GetWorkersTaskPool()->WaitUntilTasksEnd();
    }

    // we should update forwarded weak references located in global storage
    auto weakVisitor = [this](ObjectPointerType *refField) -> bool {
        auto *oldObj = ReadRefSlot(refField);
        auto *region = RegionDesc::GetRegionDescAt(oldObj);
        if (!region->IsFromRegion()) {
            return true;
        }
        if (!oldObj->IsForwarded()) {
            // it is unreachable so it is not forwarded
            return false;
        }
        auto *toVersion = GetForwardingPointer(oldObj);
        CompareExchangeRefSlot(refField, oldObj, toVersion);
        return true;
    };
    VisitWeakGlobalRoots(weakVisitor);

    SatbBuffer::Instance().ClearBuffer();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkNonHeapRoots(CmcGCMarkingStack &stack)
{
    VisitRoots([this, &stack](GCRoot root) { RemarkNonHeapRoot(root, stack); });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::EnqueueRememberedSetRefs(CmcGCEvacuationStack &stack)
{
    auto &space = static_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
    space.MarkRememberSet([this, &stack](BaseObject *object) { EnqueueRefsToYoungCollectionSpace(object, stack); });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkSatbBufferYoung(CmcGCMarkingStack &stack)
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
void CmcGC<LanguageConfig>::MarkSatbBufferFull(CmcGCMarkingStack &stack)
{
    mem::GCScope<mem::TRACE_TIMING> gcScope("MarkSatbBufferFull", this);
    auto visitSatbObj = [this, &stack]() {
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

        while (!remarkStack.empty()) {  // LCOV_EXCL_BR_LINE
            BaseObject *obj = remarkStack.top();
            remarkStack.pop();
            CHECK(Heap::IsHeapAddress(obj));
            if (this->MarkObjectIfNotMarked(obj)) {
                stack.PushToStack(obj);
                LOG(DEBUG, GC) << "satb buffer add obj " << obj;
            }
        }
    };

    visitSatbObj();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessEvacuationStack(CmcGCEvacuationStack &stack)
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
void CmcGC<LanguageConfig>::MarkEvacuationStack(CmcGCMarkingStack &stack)
{
    while (!stack.Empty()) {
        MarkRefYoung(stack.PopFromStack(), stack);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkNonHeapRoot(GCRoot root, CmcGCMarkingStack &stack)
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
void CmcGC<LanguageConfig>::ProcessRef(ObjectPointerType *ref, CmcGCEvacuationStack &stack)
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
void CmcGC<LanguageConfig>::MarkRefYoung(ObjectHeader *obj, CmcGCMarkingStack &stack)
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
void CmcGC<LanguageConfig>::MarkRefFull(ObjectHeader *obj, CmcGCMarkingStack &stack)
{
    DCHECK(Heap::IsHeapAddress(obj));

    auto targetRegion = RegionDesc::GetAliveRegionDescAt(obj);
    if (targetRegion->IsNewObjectSinceMarking(obj)) {
        return;
    }

    if (targetRegion->MarkObject(obj)) {
        return;
    }
    stack.PushToStack(obj);
    return;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::EnqueueRefsToYoungCollectionSpace(ObjectHeader *obj, CmcGCEvacuationStack &stack)
{
    auto handler = [&stack]([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p) {
        auto ref = ReadRefSlot(p);
        if (ref == 0) {
            return true;
        }
        if (CmcGC<LanguageConfig>::InYoungCollectionSpace(ref)) {
            stack.PushToStack(p);
        }
        return true;
    };

    auto *cls = obj->template ClassAddr<Class>();
    mem::ObjectIterator<LanguageConfig::LANG_TYPE>::template Iterate<false>(cls, obj, handler);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::EnqueueRefsToYoungCollectionSpace(ObjectHeader *obj, CmcGCMarkingStack &stack)
{
    auto handler = [&stack]([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p) {
        auto ref = ReadRefSlot(p);
        if (ref == 0) {
            return true;
        }
        if (CmcGC<LanguageConfig>::InYoungCollectionSpace(ref)) {
            stack.PushToStack(ref);
        }
        return true;
    };

    auto *cls = obj->template ClassAddr<Class>();
    mem::ObjectIterator<LanguageConfig::LANG_TYPE>::template Iterate<false>(cls, obj, handler);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(const RegionDesc *region)
{
    return InYoungCollectionSpace(region->GetRegionType());
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(const ObjectHeader *obj)
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
void CmcGC<LanguageConfig>::EnumRootsFlip(const GCRootVisitor &visitor)
{
    ScopedStopTheWorld stw;
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::COMMON_PAUSE);

    SetGCThreadQosPriority(PriorityMode::STW);
    EnumRootsImpl<VisitGlobalRoots>(visitor);
    mem::RootManager<LanguageConfig> rootManager(this->GetPandaVm());
    rootManager.VisitLocalRoots(visitor);
    SetGCThreadQosPriority(PriorityMode::FOREGROUND);
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
    CHECK(this->GetGCPhase() == GCPhase::GC_PHASE_PRECOPY || this->GetGCPhase() == GCPhase::GC_PHASE_COPY ||
          this->GetGCPhase() == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE ||
          this->GetGCPhase() == GCPhase::GC_PHASE_FIX || this->GetGCPhase() == GCPhase::GC_PHASE_REMARK);

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
    } else if (phase == GCPhase::GC_PHASE_RUNNING || phase == GCPhase::GC_PHASE_IDLE) {
        DisableReadBarrier(static_cast<ark::Mutator *>(mutator));
        DisablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    }
    // Check if a new phase has been added
    ASSERT(phase == GCPhase::GC_PHASE_INITIAL_MARK || phase == GCPhase::GC_PHASE_MARK ||
           phase == GCPhase::GC_PHASE_PRECOPY || phase == GCPhase::GC_PHASE_COPY || phase == GCPhase::GC_PHASE_FIX ||
           phase == GCPhase::GC_PHASE_IDLE || phase == GCPhase::GC_PHASE_REMARK ||
           phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE || phase == GCPhase::GC_PHASE_RUNNING);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::OnMutatorTerminate(Mutator *mutator, [[maybe_unused]] mem::BuffersKeepingFlag keepBuffers)
{
    this->GetPandaVm()->GetMutatorManager()->UnregisterMutator(mutator, [this](Mutator *mutator) {
        DCHECK(static_cast<const ark::Mutator *>(mutator)->GetStatus() != ark::MutatorStatus::RUNNING);
        mutator->ReleaseAllocBuffer();

        UpdateBarrierEntrypoint(mutator, GCPhase::GC_PHASE_IDLE);
        mutator->ResetMutator();
        return true;
    });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::OnMutatorCreate(Mutator *mutator)
{
    GCPhase phase = this->GetGCPhase();
    // Enable pre write barrier for mutators created during concurrent marking and enable read barrier for mutators
    // created during concurrent copy/fix.
    if (phase >= GCPhase::GC_PHASE_INITIAL_MARK) {
        mutator->HandleGCPhase(phase);
        UpdateBarrierEntrypoint(mutator, phase);
    }
    GC::OnMutatorCreate(mutator);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::HandleMarkedObject(ObjectHeader *object, CmcGCMarkingStack &stack)
{
    auto handler = [&stack]([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *field) {
        if (ObjectAccessor::Load(field) == 0) {
            return true;
        }
        CmcGC<LanguageConfig>::MarkRefFull(ReadRefSlot(field), stack);
        return true;
    };

    if (this->IsWeakReference(object)) {
        HandleWeakReference(object, stack);
        return;
    }
    ObjectIterator<LanguageConfig::LANG_TYPE>::template Iterate<false>(object->ClassAddr<Class>(), object, handler);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessMarkStack(CmcGCMarkingStack &stack)
{
    size_t nNewlyMarked = 0;
    PandaStack<BaseObject *> remarkStack;
    auto fetchFromSatbBuffer = [this, &stack, &remarkStack]() {
        SatbBuffer::Instance().TryFetchOneRetiredNode(remarkStack);
        bool needProcess = false;
        while (!remarkStack.empty()) {
            BaseObject *obj = remarkStack.top();
            remarkStack.pop();
            if (Heap::IsHeapAddress(obj) && (MarkObjectIfNotMarked(obj))) {
                stack.PushToStack(obj);
                needProcess = true;
                LOG(DEBUG, GC) << "tracing take from satb buffer: obj " << obj;
            }
        }
        return needProcess;
    };
    size_t iterationCnt = 0;
    constexpr size_t maxIterationLoopNum = 1000;

    // loop until work stack empty.
    while (true) {
        while (!stack.Empty()) {
            ObjectHeader *object = stack.PopFromStack();
            BaseObject *baseObject = static_cast<BaseObject *>(object);
            ++nNewlyMarked;
            auto *region = RegionDesc::GetAliveRegionDescAt(object);
            auto objSize = baseObject->GetSize();
            HandleMarkedObject(object, stack);
            region->AddLiveByteCount(objSize);
        }
        // Try some task from satb buffer, bound the loop to make sure it converges in time
        if (++iterationCnt >= maxIterationLoopNum) {
            break;
        }
        if (!fetchFromSatbBuffer()) {
            break;
        }
    }
    DCHECK(stack.Empty());
    // newly marked statistics.
    // Atomic with relaxed order reason: data race with markedObjectCount_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    markedObjectCount_.fetch_add(nNewlyMarked, std::memory_order_relaxed);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsWeakReference(ObjectHeader *obj)
{
    return this->GetReferenceProcessor()->IsReference(obj->ClassAddr<BaseClass>(), obj,
                                                      GC::EmptyReferenceProcessPredicate);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::HandleWeakReference(ObjectHeader *weakRef, CmcGCMarkingStack &stack)
{
    this->GetReferenceProcessor()->HandleReference(this, weakRef->ClassAddr<BaseClass>(), weakRef, [&stack](void *ref) {
        MarkRefFull(ReadRefSlot(reinterpret_cast<ObjectPointerType *>(ref)), stack);
    });
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::PushRootToWorkStack(CmcGCMarkingStack &collectStack, ObjectHeader *obj, GCTaskCause reason)
{
    BaseObject *baseObj = static_cast<BaseObject *>(obj);
    RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(obj);
    ASSERT(reason != GCTaskCause::YOUNG_GC_CAUSE);

    // inline MarkObject
    bool marked = regionInfo->MarkObject(baseObj);
    if (!marked) {
        DCHECK(!regionInfo->IsGarbageRegion());
        LOG(DEBUG, GC) << "mark obj " << obj << "<" << baseObj->GetTypeInfo() << ">(" << baseObj->GetSize()
                       << ") in region " << regionInfo << "(" << static_cast<size_t>(regionInfo->GetRegionType())
                       << ")@0x" << std::hex << regionInfo->GetRegionStart() << std::dec << ", live "
                       << regionInfo->GetLiveByteCount();
        collectStack.PushToStack(baseObj);
        return true;
    } else {
        return false;
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PushRootsToWorkStack(CmcGCMarkingStack &collectStack,
                                                 const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason)
{
    ScopedTrace tracer(("PushRootsToWorkStack_" + ToPandaString(collectedRoots.size())).c_str(),
                       ark::common_vm::ENABLE_GC_TRACING);

    for (ObjectHeader *obj : collectedRoots) {
        PushRootToWorkStack(collectStack, obj, reason);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkingRoots(const CArrayList<ObjectHeader *> &collectedRoots, GCTaskCause reason)
{
    ASSERT(reason != GCTaskCause::YOUNG_GC_CAUSE);
    ScopedTrace tracer("MarkingRoots", ark::common_vm::ENABLE_GC_TRACING);

    bool useGcWorkers = this->GetSettings()->ParallelMarkingEnabled();
    CmcGCMarkingStack markStack(this, useGcWorkers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                useGcWorkers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                GCWorkersTaskTypes::TASK_FULL_MARK,
                                this->GetSettings()->GCMarkingStackNewTasksFrequency());
    PushRootsToWorkStack(markStack, collectedRoots, reason);

    {
        mem::GCScope<mem::TRACE_TIMING> gcScope("Concurrent marking", this);
        ProcessMarkStack(markStack);
        if (useGcWorkers) {
            this->GetWorkersTaskPool()->WaitUntilTasksEnd();
        }
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::Remark(GCTaskCause reason)
{
    mem::GCScope<mem::TRACE_TIMING> gcScope("STW re-marking", this);
    bool useGcWorkers = this->GetSettings()->ParallelMarkingEnabled();
    CmcGCMarkingStack markStack(this, useGcWorkers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                useGcWorkers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                GCWorkersTaskTypes::TASK_FULL_MARK,
                                this->GetSettings()->GCMarkingStackNewTasksFrequency());
    MarkSatbBufferFull(markStack);
    ProcessMarkStack(markStack);
    if (useGcWorkers) {
        this->GetWorkersTaskPool()->WaitUntilTasksEnd();
    }

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
    CHECK(task.reason != GCTaskCause::CROSSREF_CAUSE);
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
    return [visitor](mem::GCRoot root) { visitor(root.GetObjectPointer()); };
}

template <class Visitor>
ObjectStatus UpdateWeakRoot(ObjectHeader **ref, const Visitor &visitor)
{
    ObjectPointerType slot = ToObjPtrType(*ref);
    bool isAlive = visitor(&slot);
    *ref = ToNativePtr<ObjectHeader>(static_cast<uintptr_t>(slot));
    return isAlive ? ObjectStatus::ALIVE_OBJECT : ObjectStatus::DEAD_OBJECT;
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
void CmcGC<LanguageConfig>::VisitSTWRoots(const GCRootVisitor &visitor)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    rootManager.VisitAotStringRoots(visitor, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitVmRoots(visitor);
    rootManager.VisitClassRoots(visitor, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitClassLinkerContextRoots(visitor);
    vm->VisitStringTable(visitor, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
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
    rootManager.UpdateAndSweep([&visitor](ObjectHeader **ref) { return UpdateWeakRoot(ref, visitor); });
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
void CmcGC<LanguageConfig>::VisitWeakGlobalRoots(const WeakRefFieldVisitor &visitor)
{
    auto *vm = PandaVM::GetCurrent();
    ASSERT(vm != nullptr);
    mem::RootManager<LanguageConfig> rootManager(vm);

    const GCRootVisitor &callback = MakeCallback(visitor);
    rootManager.VisitAotStringRoots(callback, mem::VisitGCRootFlags::ACCESS_ROOT_ALL);
    rootManager.VisitVmRoots(callback);
    rootManager.UpdateAndSweep([&visitor](ObjectHeader **ref) { return UpdateWeakRoot(ref, visitor); });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::WorkerTaskProcessing(GCWorkersTask *task, [[maybe_unused]] void *workerData)
{
    common_vm::ScopedGcThreadType scopedGcThreadType;
    switch (task->GetType()) {
        case GCWorkersTaskTypes::TASK_REMARK: {
            auto *stack = task->Cast<CmcGCMarkingTask>()->GetStack();
            MarkEvacuationStack(*stack);
            ASSERT(stack->Empty());
            this->GetInternalAllocator()->Delete(stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_EVACUATE_REGIONS: {
            auto *stack = task->Cast<CmcGCEvacuationTask>()->GetStack();
            ProcessEvacuationStack(*stack);
            ASSERT(stack->Empty());
            this->GetInternalAllocator()->Delete(stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_FULL_MARK: {
            auto *stack = task->Cast<CmcGCMarkingTask>()->GetStack();
            ProcessMarkStack(*stack);
            ASSERT(stack->Empty());
            this->GetInternalAllocator()->Delete(stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_CONCURRENT_FIX: {
            auto *workerData = task->Cast<CmcGCFixTask>()->GetData();
            DispatchFixRegionTasks(workerData);
            break;
        }
        case GCWorkersTaskTypes::TASK_CONCURRENT_POST_FIX: {
            auto *result = task->Cast<CmcGCPostFixTask>()->GetResult();
            PostClearTask(result);
            break;
        }
        default:
            LOG(FATAL, GC) << "Unimplemented for " << GCWorkersTaskTypesToString(task->GetType());
            UNREACHABLE();
    }
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(CmcGC);

}  // namespace ark::mem

#endif  // #if defined(ARK_USE_COMMON_RUNTIME)
