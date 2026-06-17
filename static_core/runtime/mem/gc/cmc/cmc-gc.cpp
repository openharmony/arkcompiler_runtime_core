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

#include "common_components/log/log.h"
#include "common_components/base/time_utils.h"
#include "common_components/mutator/mutator_manager-inl.h"
#include "common_components/heap/verification.h"
#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/allocator/alloc_buffer.h"
#include "common_components/heap/collector/heuristic_gc_policy.h"
#include "common_components/heap/collector/utils.h"
#include "common_interfaces/base/runtime_param.h"
#include "common_components/heap/collector/collector_resources.h"

#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/math_helpers.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

#include "common_interfaces/base_runtime.h"
#include "common_interfaces/heap/region_desc.h"

#ifdef ENABLE_QOS
#include "qos.h"
#endif

namespace ark::mem {
using ark::common_vm::AllocationBuffer;
using ark::common_vm::FixHeapTask;
using ark::common_vm::FixHeapWorker;
using ark::common_vm::FlipFunction;
using ark::common_vm::GCReason;
using ark::common_vm::MB;
using ark::common_vm::MutatorManager;
using ark::common_vm::PostFixHeapWorker;
using ark::common_vm::PriorityMode;
using ark::common_vm::RegionalHeap;
using ark::common_vm::SatbBuffer;
using ark::common_vm::ScopedStopTheWorld;
using ark::common_vm::STWParam;
using ark::common_vm::Task;
using ark::common_vm::TaskPackMonitor;
using ark::common_vm::ThreadLocal;
using ark::common_vm::ThreadType;
using ark::common_vm::VisitConcurrentRoots;
using ark::common_vm::VisitGlobalRoots;
using ark::common_vm::VisitPreforwardRoots;
using ark::common_vm::VisitRoots;
using ark::common_vm::VisitWeakGlobalRoots;
using ark::common_vm::VisitWeakRoots;
using ark::common_vm::WVerify;

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsUnmovableFromObject(BaseObject *obj) const
{
    // filter const string object.
    if (!Heap::IsHeapAddress(obj)) {
        return false;
    }

    RegionDesc *regionInfo = nullptr;
    regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj));
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
        RefField<> tmpField(toObj, oldRef.IsWeak());
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
bool CmcGC<LanguageConfig>::TryUpdateRefField(BaseObject *obj, RefField<> &field, BaseObject *&newRef) const
{
    BaseObject *oldRef = nullptr;
    return TryUpdateRefFieldImpl<false>(obj, field, oldRef, newRef);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::TryForwardRefField(BaseObject *obj, RefField<> &field, BaseObject *&newRef) const
{
    BaseObject *oldRef = nullptr;
    return TryUpdateRefFieldImpl<true>(obj, field, oldRef, newRef);
}

static void MarkingRefField(BaseObject *obj, BaseObject *targetObj, RefField<> &field,
                            ParallelLocalMarkStack &markStack, RegionDesc *targetRegion);
// note each ref-field will not be marked twice, so each old pointer the markingr meets must come from previous gc.
static void MarkingRefField(BaseObject *obj, RefField<> &field, ParallelLocalMarkStack &markStack,
                            const GCReason gcReason)
{
    RefField<> oldField(field);
    BaseObject *targetObj = oldField.GetTargetObject();

    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // field is tagged object, should be in heap
    DCHECK(Heap::IsHeapAddress(targetObj));

    auto targetRegion = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<MAddress>((void *)targetObj));
    // cannot skip objects in EXEMPTED_FROM_REGION, because its rset is incomplete
    if (gcReason == ark::common_vm::GC_REASON_YOUNG && !targetRegion->IsInYoungSpace()) {
        LOG(DEBUG, GC) << "marking: skip non-young object " << obj << "@" << &field << ", target object: " << targetObj
                       << "<" << targetObj->GetTypeInfo() << ">(" << targetObj->GetSize() << ")";
        return;
    }
    MarkingRefField(obj, targetObj, field, markStack, targetRegion);
}

// note each ref-field will not be marked twice, so each old pointer the markingr meets must come from previous gc.
static void MarkingRefField(BaseObject *obj, BaseObject *targetObj, RefField<> &field,
                            ParallelLocalMarkStack &markStack, RegionDesc *targetRegion)
{
    if (targetRegion->IsNewObjectSinceMarking(targetObj)) {
        LOG(DEBUG, GC) << "marking: skip new obj " << targetObj << "<" << targetObj->GetTypeInfo() << ">("
                       << targetObj->GetSize() << ")";
        return;
    }

    if (targetRegion->MarkObject(targetObj)) {
        LOG(DEBUG, GC) << "marking: obj has been marked " << targetObj;
        return;
    }
    LOG(DEBUG, GC) << "marking obj " << obj << " ref@" << &field << ": " << targetObj << "<" << targetObj->GetTypeInfo()
                   << ">(" << targetObj->GetSize() << ")";
    markStack.Push(targetObj);
}

// GC passes this function to ObjectOperator::ForEachRefField.
// ObjectOperator must call it for reference fields of WeakRef objects.
// Its responcibility of ObjectOperator to determine whether 'obj' is kind of WeakRef or not.
static void MarkWeakRefField(BaseObject *obj, RefField<> &field, WeakStack &weakStack, const GCReason gcReason)
{
    RefField<> oldField(field);
    [[maybe_unused]] BaseObject *targetObj = oldField.GetTargetObject();

    if (gcReason == ark::common_vm::GC_REASON_YOUNG) {
        return;
    }

    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // field is tagged object, should be in heap
    DCHECK(Heap::IsHeapAddress(targetObj));
    LOG(DEBUG, GC) << "marking: skip weak obj when full gc, object: " << obj << "@" << &field
                   << ", targetObj: " << targetObj;
    // weak ref is cleared after roots pre-forward, so there might be a to-version weak ref which also need to be
    // cleared, offset recorded here will help us find it
    weakStack.emplace_back(&field, reinterpret_cast<uintptr_t>(&field) - reinterpret_cast<uintptr_t>(obj));
}

template <class LanguageConfig>
typename CmcGC<LanguageConfig>::MarkingRefFieldVisitor CmcGC<LanguageConfig>::CreateMarkingObjectRefFieldsVisitor(
    ParallelLocalMarkStack &markStack, WeakStack &weakStack)
{
    MarkingRefFieldVisitor visitor;

    if (gcReason_ == GCReason::GC_REASON_YOUNG) {
        auto func = [obj = visitor.GetClosure().get(), &markStack](RefField<> &field) {
            MarkingRefField(*obj, field, markStack, GCReason::GC_REASON_YOUNG);
        };
        visitor.SetVisitor(func);
        visitor.SetWeakVisitor(func);
    } else {
        visitor.SetVisitor([obj = visitor.GetClosure().get(), &markStack](RefField<> &field) {
            MarkingRefField(*obj, field, markStack, GCReason::GC_REASON_HEU);
        });
        visitor.SetWeakVisitor([obj = visitor.GetClosure().get(), &weakStack](RefField<> &field) {
            MarkWeakRefField(*obj, field, weakStack, GCReason::GC_REASON_HEU);
        });
    }
    return visitor;
}

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
    DCHECK(!field.IsWeak());
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

    RegionDesc::InlinedRegionMetaData *refRegion =
        RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(targetObj));
    bool isFrom = refRegion->IsFromRegion();
    bool isInRcent = refRegion->IsInRecentSpace();
    if (isInRcent) {
        RegionDesc::InlinedRegionMetaData *objRegion =
            RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(obj));
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
    RefField<> newField(latest, oldField.IsWeak());
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
class RemarkAndPreforwardVisitor {
public:
    RemarkAndPreforwardVisitor(LocalCollectStack &collectStack, CmcGC<LanguageConfig> *collector)
        : collectStack_(collectStack), collector_(collector)
    {
    }

    void operator()(RefField<> &refField)
    {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        LOG(DEBUG, GC) << "visit raw-ref @" << &refField << ": " << oldObj;

        auto regionType =
            RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(oldObj))
                ->GetRegionType();
        if (regionType == RegionDesc::RegionType::FROM_REGION) {
            BaseObject *toVersion = collector_->TryForwardObject(oldObj);
            if (toVersion == nullptr) {  // LCOV_EXCL_BR_LINE
                Heap::throwOOM();
                return;
            }
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                LOG(DEBUG, GC) << "fix raw-ref @" << &refField << ": " << oldObj << " -> " << toVersion;
                LOG(DEBUG, MM_OBJECT_EVENTS) << "[CMC] "
                                             << "REMARK ref @" << &refField << ": " << oldObj << " -> " << toVersion;
            }
            MarkToObject(oldObj, toVersion);
        } else {
            if (Heap::GetHeap().GetGCReason() != ark::common_vm::GC_REASON_YOUNG) {
                MarkObject(oldObj);
            } else if (RegionalHeap::IsYoungSpaceObject(oldObj) && !RegionalHeap::IsNewObjectSinceMarking(oldObj) &&
                       !RegionalHeap::IsMarkedObject(oldObj)) {
                // RSet don't protect exempted objects, we need to mark it
                MarkObject(oldObj);
            }
        }
    }

private:
    void MarkObject(BaseObject *object)
    {
        if (!RegionalHeap::IsNewObjectSinceMarking(object) && collector_->MarkObjectIfNotMarked(object)) {
            collectStack_.Push(object);
        }
    }

    void MarkToObject(BaseObject *oldVersion, BaseObject *toVersion)
    {
        // We've checked oldVersion is in fromSpace, no need to check markingLine
        collector_->MarkObject(oldVersion);
        if (collector_->MarkObjectIfNotMarked(toVersion)) {
            // oldVersion don't have valid type info, cannot push it
            collectStack_.Push(toVersion);
        }
    }

private:
    LocalCollectStack &collectStack_;
    CmcGC<LanguageConfig> *collector_;
};

template <class LanguageConfig>
class PreforwardVisitor {
public:
    explicit PreforwardVisitor(CmcGC<LanguageConfig> *collector) : collector_(collector) {}

    void operator()(RefField<> &refField)
    {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        LOG(DEBUG, GC) << "visit raw-ref @" << &refField << ": " << oldObj;

        auto regionType =
            RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(oldObj))
                ->GetRegionType();
        if (regionType != RegionDesc::RegionType::FROM_REGION) {
            return;
        }

        BaseObject *toVersion = collector_->TryForwardObject(oldObj);
        if (toVersion == nullptr) {  // LCOV_EXCL_BR_LINE
            Heap::throwOOM();
            return;
        }
        RefField<> newField(toVersion);
        // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
        if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
            LOG(DEBUG, GC) << "fix raw-ref @" << &refField << ": " << oldObj << " -> " << toVersion;
        }
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
class RemarkingAndPreforwardTask : public ark::common_vm::Task {
public:
    RemarkingAndPreforwardTask(CmcGC<LanguageConfig> *collector, GlobalMarkStack &globalMarkStack,
                               ark::common_vm::TaskPackMonitor &monitor,
                               std::function<ark::common_vm::Mutator *()> &next)
        : Task(0), collector_(collector), globalMarkStack_(globalMarkStack), monitor_(monitor), getNextMutator_(next)
    {
    }

    bool Run([[maybe_unused]] uint32_t threadIndex) override
    {
        ThreadLocal::SetThreadType(ThreadType::GC_THREAD);
        LocalCollectStack collectStack(&globalMarkStack_);
        RemarkAndPreforwardVisitor<LanguageConfig> visitor(collectStack, collector_);
        auto *mutator = getNextMutator_();
        while (mutator != nullptr) {
            mutator->VisitMutatorRoots(visitor);
            mutator = getNextMutator_();
        }
        collectStack.Publish();
        ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
        ThreadLocal::ClearAllocBufferRegion();
        monitor_.NotifyFinishOne();
        return true;
    }

private:
    CmcGC<LanguageConfig> *collector_ {nullptr};
    GlobalMarkStack &globalMarkStack_;
    ark::common_vm::TaskPackMonitor &monitor_;
    std::function<ark::common_vm::Mutator *()> &getNextMutator_;
};

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ParallelRemarkAndPreforward(GlobalMarkStack &globalMarkStack)
{
    PandaVector<ark::common_vm::Mutator *> taskList;
    auto &mutatorManager = MutatorManager::Instance();
    mutatorManager.VisitAllMutators([&taskList](ark::common_vm::Mutator &mutator) { taskList.push_back(&mutator); });
    std::atomic<int> taskIter = 0;
    std::function<ark::common_vm::Mutator *()> getNextMutator = [&taskIter, &taskList]() {
        // Atomic with relaxed order reason: data race with taskIter with no synchronization or ordering constraints
        // imposed on other reads or writes
        uint32_t idx = static_cast<uint32_t>(taskIter.fetch_add(1U, std::memory_order_relaxed));
        if (idx < taskList.size()) {
            return taskList[idx];
        }
        return (ark::common_vm::Mutator *)nullptr;
    };

    const uint32_t runningWorkers = std::min<uint32_t>(GetGCThreadCount(true), taskList.size());
    uint32_t parallelCount = runningWorkers + 1;  // 1 ：DaemonThread
    ark::common_vm::TaskPackMonitor monitor(runningWorkers, runningWorkers);
    for (uint32_t i = 1; i < parallelCount; ++i) {
        GetThreadPool()->PostTask(MakePandaUnique<RemarkingAndPreforwardTask<LanguageConfig>>(this, globalMarkStack,
                                                                                              monitor, getNextMutator));
    }
    // Run in daemon thread.
    LocalCollectStack collectStack(&globalMarkStack);
    RemarkAndPreforwardVisitor<LanguageConfig> visitor(collectStack, this);
    VisitGlobalRoots(visitor);
    auto *mutator = getNextMutator();
    while (mutator != nullptr) {
        mutator->VisitMutatorRoots(visitor);
        mutator = getNextMutator();
    }
    collectStack.Publish();
    monitor.WaitAllFinished();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkAndPreforwardStaticRoots(GlobalMarkStack &globalMarkStack)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::RemarkAndPreforwardStaticRoots", "");
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;
    if (maxWorkers > 0) {
        ParallelRemarkAndPreforward(globalMarkStack);
    } else {
        LocalCollectStack collectStack(&globalMarkStack);
        RemarkAndPreforwardVisitor<LanguageConfig> visitor(collectStack, this);
        VisitRoots(visitor);
        collectStack.Publish();
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardStaticRoots()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PreforwardStaticRoots", "");
    PreforwardVisitor<LanguageConfig> visitor(this);
    VisitRoots(visitor);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardConcurrentRoots()
{
    RefFieldVisitor visitor = [this](RefField<> &refField) {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        LOG(DEBUG, GC) << "visit raw-ref @" << &refField << ": " << oldObj;
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            ASSERT_PRINT(toVersion != nullptr, "TryForwardObject failed");
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                LOG(DEBUG, GC) << "fix raw-ref @" << &refField << ": " << oldObj << " -> " << toVersion;
                LOG(DEBUG, MM_OBJECT_EVENTS)
                    << "[CMC] "
                    << "PREFORWARD concurrent ref @" << &refField << ": " << oldObj << " -> " << toVersion;
            }
        }
    };
    VisitConcurrentRoots(visitor);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardStaticWeakRoots()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PreforwardStaticRoots", "");

    WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
    VisitWeakRoots(weakVisitor);
    MutatorManager::Instance().VisitAllMutators([](ark::common_vm::Mutator &mutator) {
        // Request finalize callback in each vm-thread when gc finished.
        mutator.SetFinalizeRequest();
    });

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
    CArrayList<BaseObject *> *GetBuffer()
    {
        return &buffer_;
    }

private:
    static size_t bufferSize_;
    CArrayList<BaseObject *> buffer_;
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
CArrayList<BaseObject *> CmcGC<LanguageConfig>::EnumRoots()
{
    STWParam stwParam {"wgc-enumroot"};
    EnumRootsBuffer buffer;
    CArrayList<ark::common_vm::BaseObject *> *results = buffer.GetBuffer();
    ark::mem::RefFieldVisitor visitor = [&results](RefField<> &field) { results->push_back(field.GetTargetObject()); };

    if constexpr (policy == EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR) {
        EnumRootsImpl<VisitRoots>(visitor);
    } else if constexpr (policy == EnumRootsPolicy::STW_AND_NO_FLIP_MUTATOR) {
        ScopedStopTheWorld stw(stwParam);
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL,
                     ("CMCGC::EnumRoots-STW-bufferSize(" + std::to_string(results->capacity()) + ")").c_str(), "");
        EnumRootsImpl<VisitRoots>(visitor);
    } else if constexpr (policy == EnumRootsPolicy::STW_AND_FLIP_MUTATOR) {
        auto rootSet = EnumRootsFlip(stwParam, visitor);
        for (const auto &roots : rootSet) {
            std::copy(roots.begin(), roots.end(), std::back_inserter(*results));
        }
        VisitConcurrentRoots(visitor);
    }
    buffer.UpdateBufferSize();
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
    return std::move(*results);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkingHeap(const CArrayList<BaseObject *> &collectedRoots)
{
    COMMON_PHASE_TIMER("marking live objects");
    // Atomic with relaxed order reason: data race with markedObjectCount_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    markedObjectCount_.store(0, std::memory_order_relaxed);
    STWParam stwParam {"GC_PHASE_MARK transition"};
    {
        ScopedStopTheWorld stw(stwParam);
        TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_MARK);
    }

    MarkingRoots(collectedRoots);
    ExemptFromSpace();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PostMarking()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PostMarking", "");
    COMMON_PHASE_TIMER("PostMarking");
    TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_POST_MARK);

    // clear satb buffer when gc finish tracing.
    SatbBuffer::Instance().ClearBuffer();

    WVerify::VerifyAfterMark();
}

template <class LanguageConfig>
WeakRefFieldVisitor CmcGC<LanguageConfig>::GetWeakRefFieldVisitor()
{
    return [this](RefField<> &refField) -> bool {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (gcReason_ == ark::common_vm::GC_REASON_YOUNG) {
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
RefFieldVisitor CmcGC<LanguageConfig>::GetPrefowardRefFieldVisitor()
{
    return [this](RefField<> &refField) -> void {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK(toVersion != nullptr);
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                LOG(DEBUG, GC) << "fix raw-ref @" << &refField << ": " << oldObj << " -> " << toVersion;
                LOG(DEBUG, MM_OBJECT_EVENTS)
                    << "[CMC] "
                    << "PREFORWARD ref @" << &refField << ": " << oldObj << " -> " << toVersion;
            }
        }
    };
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardFlip()
{
    auto remarkAndForwardGlobalRoot = [this]() {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PreforwardFlip[STW]", "");
        SetGCThreadQosPriority(ark::common_vm::PriorityMode::STW);
        ASSERT_PRINT(GetThreadPool() != nullptr, "thread pool is null");
        TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK);
        Remark();
        PostMarking();
        reinterpret_cast<RegionalHeap &>(theAllocator_).PrepareForward();

        TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_PRECOPY);
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
        SetGCThreadQosPriority(ark::common_vm::PriorityMode::FOREGROUND);

        VisitWeakGlobalRoots(weakVisitor, Heap::GetHeap().GetGCReason() == ark::common_vm::GC_REASON_YOUNG);
    };
    FlipFunction forwardMutatorRoot = [this](ark::common_vm::Mutator &mutator) {
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
        mutator.VisitMutatorRoots(weakVisitor);
        RefFieldVisitor visitor = GetPrefowardRefFieldVisitor();
        VisitMutatorPreforwardRoot(visitor, mutator);
        // Request finalize callback in each vm-thread when gc finished.
        mutator.SetFinalizeRequest();
    };
    STWParam stwParam {"final-mark"};
    MutatorManager::Instance().FlipMutators(stwParam, remarkAndForwardGlobalRoot, &forwardMutatorRoot);
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
    AllocationBuffer *allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::Preforward()
{
    COMMON_PHASE_TIMER("Preforward");
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::Preforward[STW]", "");
    TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_PRECOPY);

    [[maybe_unused]] auto *threadPool = GetThreadPool();
    ASSERT_PRINT(threadPool != nullptr, "thread pool is null");

    // copy and fix finalizer roots.
    // Only one root task, no need to post task.
    PreforwardStaticWeakRoots();
    RefFieldVisitor visitor = GetPrefowardRefFieldVisitor();
    VisitPreforwardRoots(visitor);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ConcurrentPreforward()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::ConcurrentPreforward", "");
    PreforwardConcurrentRoots();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PrepareFix()
{
    if (Heap::GetHeap().GetGCReason() == GCReason::GC_REASON_YOUNG) {
        // string table objects are always not in young space, skip it
        return;
    }

    COMMON_PHASE_TIMER("PrepareFix");

    // we cannot re-enter STW, check it first
    if (!MutatorManager::Instance().WorldStopped()) {
        STWParam prepareFixStwParam {"wgc-preparefix"};
        ScopedStopTheWorld stw(prepareFixStwParam);
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PrepareFix[STW]", "");

        GetGCStats().recordSTWTime(prepareFixStwParam.GetElapsedNs());
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ParallelFixHeap()
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
    {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::FixHeap [Parallel]", "");
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
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::Post FixHeap Clear [Parallel]", "");
        // Post clear task
        TaskPackMonitor monitor(runningWorkers, runningWorkers);
        for (uint32_t i = 1; i < parallelCount; ++i) {
            GetThreadPool()->PostTask(MakePandaUnique<PostFixHeapWorker>(results[i], monitor));
        }

        PostFixHeapWorker gcWorker(results[0], monitor);
        gcWorker.PostClearTask();
        PostFixHeapWorker::CollectEmptyRegions();
        monitor.WaitAllFinished();
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::FixHeap()
{
    STWParam stwParam {"GC_PHASE_FIX transition"};
    {
        ScopedStopTheWorld stw(stwParam);
        TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_FIX);
    }
    COMMON_PHASE_TIMER("FixHeap");
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::FixHeap", "");
    ParallelFixHeap();

    WVerify::VerifyAfterFix();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::CollectGarbageWithXRef()
{
    const bool isNotYoungGC = gcReason_ != GCReason::GC_REASON_YOUNG;
#ifdef ENABLE_CMC_RB_DFX
    WVerify::DisableReadBarrierDFX(*this);
#endif
    STWParam stwParam {"stw-gc"};
    ScopedStopTheWorld stw(stwParam);
    ark::common_vm::RemoveXRefFromRoots();

    auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
    MarkingHeap(collectedRoots);
    TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK);
    Remark();
    ark::common_vm::SweepUnmarkedXRefs();
    PostMarking();

    ark::common_vm::AddXRefToRoots();
    Preforward();
    ConcurrentPreforward();
    // reclaim large objects should after preforward(may process weak ref) and
    // before fix heap(may clear live bit)
    if (isNotYoungGC) {
        CollectLargeGarbage();
    }

    CopyFromSpace();
    WVerify::VerifyAfterForward();

    PrepareFix();
    FixHeap();
    if (isNotYoungGC) {
        CollectNonMovableGarbage();
    }

    TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_IDLE);

    ClearAllGCInfo();
    CollectSmallSpace();
    ark::common_vm::UnmarkAllXRefs();

#if defined(ENABLE_CMC_RB_DFX)
    WVerify::EnableReadBarrierDFX(*this);
#endif
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
}

class GCTracer final {
public:
    GCTracer(ark::common_vm::GCReason reason, ark::common_vm::GCType type) : gcReason_(reason), gcType_(type)
    {
        Heap::GetHeap().GetCollector().NotifyGCListeners(
            [reason, type](ark::common_vm::GCListener *l) { l->OnGCStart(reason, type); });
    }

    ~GCTracer()
    {
        Heap::GetHeap().GetCollector().NotifyGCListeners(
            [reason = gcReason_, type = gcType_](ark::common_vm::GCListener *l) { l->OnGCFinish(reason, type); });
    }

private:
    ark::common_vm::GCReason gcReason_;
    ark::common_vm::GCType gcType_;
};

template <class LanguageConfig>
class ConcurrentEvacuationTask : public Task {
public:
    ConcurrentEvacuationTask(uint32_t id, CmcGC<LanguageConfig> &tc, ParallelMarkingMonitor &monitor,
                             GlobalEvacuationStack &globalStack)
        : Task(id), collector_(tc), monitor_(monitor), globalStack_(globalStack)
    {
    }

    ~ConcurrentEvacuationTask() override = default;

    // run concurrent marking task.
    bool Run(uint32_t threadIndex) override
    {
        ThreadLocal::SetThreadType(ThreadType::GC_THREAD);
        ParallelLocalEvacuationStack stack(&globalStack_, &monitor_);
        do {
            if (!monitor_.TryStartStep()) {
                break;
            }
            collector_.ProcessEvacuationStack(stack);
            monitor_.FinishStep();
        } while (monitor_.WaitNextStepOrFinished());
        monitor_.NotifyFinishOne();
        ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
        return true;
    }

private:
    CmcGC<LanguageConfig> &collector_;
    ParallelMarkingMonitor &monitor_;
    GlobalEvacuationStack &globalStack_;
};

template <class LanguageConfig>
class RemarkTask : public Task {
public:
    RemarkTask(uint32_t id, CmcGC<LanguageConfig> &tc, ParallelMarkingMonitor &monitor,
               GlobalEvacuationStack &globalStack)
        : Task(id), collector_(tc), monitor_(monitor), globalStack_(globalStack)
    {
    }

    ~RemarkTask() override = default;

    // run concurrent marking task.
    bool Run(uint32_t threadIndex) override
    {
        ThreadLocal::SetThreadType(ThreadType::GC_THREAD);
        ParallelLocalEvacuationStack stack(&globalStack_, &monitor_);
        do {
            if (!monitor_.TryStartStep()) {
                break;
            }
            collector_.MarkEvacuationStack(stack);
            monitor_.FinishStep();
        } while (monitor_.WaitNextStepOrFinished());
        monitor_.NotifyFinishOne();
        ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
        return true;
    }

private:
    CmcGC<LanguageConfig> &collector_;
    ParallelMarkingMonitor &monitor_;
    GlobalEvacuationStack &globalStack_;
};

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessReferencesAfterCopy()
{
    ForEachVM([](VMInterface *vm) { vm->ProcessReferencesAfterCopy(); });
}

template <class LanguageConfig>
// CC-OFFNXT(G.FUD.05) solid logic
void CmcGC<LanguageConfig>::DoGarbageCollection()
{
    GCTracer gcTracer(gcReason_, gcType_);
    const bool isYoungGC = gcReason_ == GCReason::GC_REASON_YOUNG;
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::DoGarbageCollection", "");
    if (gcReason_ == GCReason::GC_REASON_XREF) {
        CollectGarbageWithXRef();
        return;
    }
    if (gcMode_ == GCMode::STW) {  // 2: stw-gc
#ifdef ENABLE_CMC_RB_DFX
        WVerify::DisableReadBarrierDFX(*this);
#endif
        STWParam stwParam {"stw-gc"};
        {
            ScopedStopTheWorld stw(stwParam);
            auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
            MarkingHeap(collectedRoots);
            TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK);
            Remark();
            PostMarking();

            Preforward();
            ConcurrentPreforward();
            // reclaim large objects should after preforward(may process weak ref) and
            // before fix heap(may clear live bit)
            if (!isYoungGC) {
                CollectLargeGarbage();
            }

            CopyFromSpace();
            WVerify::VerifyAfterForward();

            PrepareFix();
            FixHeap();
            if (!isYoungGC) {
                CollectNonMovableGarbage();
            }

            TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_IDLE);

            ClearAllGCInfo();
            CollectSmallSpace();

#if defined(ENABLE_CMC_RB_DFX)
            WVerify::EnableReadBarrierDFX(*this);
#endif
        }
        GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
        return;
    }

    const auto avoidConcurrentMarking = Heap::GetHeap().GetGCParam().singlePassCompactionEnabled && isYoungGC;
    if (!avoidConcurrentMarking) {
        auto collectedRoots = EnumRoots<EnumRootsPolicy::STW_AND_FLIP_MUTATOR>();
        MarkingHeap(collectedRoots);
        PreforwardFlip();
        ConcurrentPreforward();
        // reclaim large objects should after preforward(may process weak ref)
        // and before fix heap(may clear live bit)
        if (!isYoungGC) {
            CollectLargeGarbage();
        }

        CopyFromSpace();
        ProcessReferencesAfterCopy();
        WVerify::VerifyAfterForward();

        PrepareFix();
        FixHeap();

        if (!isYoungGC) {
            CollectNonMovableGarbage();
        }
    } else {
        DoGarbageCollectionWithoutConcurrentMarking();
    }

    STWParam stwParam {"GC_PHASE_IDLE transition"};
    {
        ScopedStopTheWorld stw(stwParam);
        TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_IDLE);
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
    CHECK(Heap::GetHeap().GetGCReason() == ark::common_vm::GC_REASON_YOUNG);
    GlobalEvacuationStack globalEvacuationStack;
    PreforwardNonHeapRoots<EnumRootsPolicy::STW_AND_FLIP_MUTATOR>(globalEvacuationStack);
    EnqueueRememberedSetRefs(globalEvacuationStack);
    ProcessEvacuationStack(globalEvacuationStack);
    RemarkYoungCollectionSpace(globalEvacuationStack);

    // We do not evacuate objects on remark pause to avoid too long STW
    CopyFromSpace();

    WVerify::VerifyAfterForward();

    PrepareFix();
    FixHeap();
}

template <class LanguageConfig>
template <typename CmcGC<LanguageConfig>::EnumRootsPolicy policy>
void CmcGC<LanguageConfig>::PreforwardNonHeapRoots(GlobalEvacuationStack &globalStack)
{
    CArrayList<ark::common_vm::BaseObject *> roots;

    if constexpr (policy == EnumRootsPolicy::STW_AND_NO_FLIP_MUTATOR) {
        STWParam param {"preforward-nonheap-roots"};
        ScopedStopTheWorld stw(param);

        PreforwardNonHeapRootsImpl<VisitRoots>(roots);
        GetGCStats().recordSTWTime(param.GetElapsedNs());
    } else if constexpr (policy == EnumRootsPolicy::STW_AND_FLIP_MUTATOR) {
        PreforwardNonHeapRootsFlip(roots);
    } else {
        static_assert(policy == EnumRootsPolicy::STW_AND_FLIP_MUTATOR ||
                          policy == EnumRootsPolicy::STW_AND_NO_FLIP_MUTATOR,
                      "not implemented");
    }
    LocalEvacuationStack stack(&globalStack);
    for (auto *obj : roots) {
        if (MarkObjectIfNotMarked(obj)) {
            EnqueueRefsToYoungCollectionSpace(obj, stack);
        }
    }
    stack.Publish();
}

template <class LanguageConfig>
template <void (&rootsVisitFunc)(const ark::mem::RefFieldVisitor &)>
void CmcGC<LanguageConfig>::PreforwardNonHeapRootsImpl(CArrayList<BaseObject *> &forwardedRoots)
{
    auto &heap = static_cast<RegionalHeap &>(theAllocator_);
    heap.AssembleGarbageCandidates();
    heap.PrepareMarking();
    heap.PrepareForward();

    TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_PRECOPY);
    rootsVisitFunc([this, &forwardedRoots](RefField<> &root) { PreforwardNonHeapRoot(root, forwardedRoots); });

    TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_YOUNG_COPY);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardNonHeapRoot(RefField<> &root, CArrayList<BaseObject *> &forwardedRoots)
{
    auto *obj = root.GetTargetObject();
    auto *region = RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(obj);

    if (!InYoungCollectionSpace(region)) {
        // we do not trace from such root
        return;
    }

    if (region->IsFromRegion()) {
        obj = ForwardObject(obj);
        root.SetTargetObject(obj);
    }

    forwardedRoots.push_back(obj);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreforwardNonHeapRootsFlip(CArrayList<BaseObject *> &forwardedRoots)
{
    const auto enumGlobalRoots = [this, &forwardedRoots]() {
        SetGCThreadQosPriority(PriorityMode::STW);
        PreforwardNonHeapRootsImpl<VisitGlobalRoots>(forwardedRoots);
        SetGCThreadQosPriority(PriorityMode::FOREGROUND);
    };

    ark::os::memory::Mutex stackMutex;
    CArrayList<CArrayList<BaseObject *>> rootSet;  // allocate for each mutator
    FlipFunction enumMutatorRoot = [this, &rootSet, &stackMutex](ark::common_vm::Mutator &mutator) {
        CArrayList<BaseObject *> roots;
        mutator.VisitMutatorRoots([this, &roots](RefField<> &root) { PreforwardNonHeapRoot(root, roots); });
        ark::os::memory::LockHolder lockGuard(stackMutex);
        rootSet.emplace_back(std::move(roots));
    };

    STWParam param {"preforward-non-heap-roots"};
    MutatorManager::Instance().FlipMutators(param, enumGlobalRoots, &enumMutatorRoot);
    GetGCStats().recordSTWTime(param.GetElapsedNs());

    for (const auto &roots : rootSet) {
        std::copy(roots.begin(), roots.end(), std::back_inserter(forwardedRoots));
    }

    VisitConcurrentRoots([this, &forwardedRoots](RefField<> &root) {
        auto *obj = root.GetTargetObject();
        auto *region = RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(obj);
        if (InYoungCollectionSpace(region)) {
            CHECK(!region->IsFromRegion());
            forwardedRoots.push_back(obj);
        }
    });
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkYoungCollectionSpace(GlobalEvacuationStack &globalStack)
{
    STWParam param {"remark-young-collection-space"};
    ScopedStopTheWorld stw(param);

    TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK);

    RemarkNonHeapRoots(globalStack);
    MarkSatbBuffer(globalStack);
    MarkEvacuationStack(globalStack);

    // we should update forwarded weak references located in global storage
    auto weakVisitor = [this](RefField<> &refField) -> bool {
        RefField<> oldField(refField);
        auto *oldObj = oldField.GetTargetObject();
        auto *region = RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(oldObj);
        if (!region->IsFromRegion()) {
            return true;
        }
        if (!oldObj->IsForwarded()) {
            // it is unreachable so it is not forwarded
            return false;
        }
        auto *toVersion = GetForwardingPointer(oldObj);
        RefField<> newField(toVersion, oldField.IsWeak());
        refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue());
        return true;
    };
    VisitWeakGlobalRoots(weakVisitor, true);

    SatbBuffer::Instance().ClearBuffer();
    GetGCStats().recordSTWTime(param.GetElapsedNs());
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RemarkNonHeapRoots(GlobalEvacuationStack &globalStack)
{
    LocalEvacuationStack stack(&globalStack);
    VisitRoots([this, &stack](RefField<> &root) { RemarkNonHeapRoot(root, stack); });
    stack.Publish();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::EnqueueRememberedSetRefs(GlobalEvacuationStack &globalStack)
{
    LocalEvacuationStack stack(&globalStack);
    auto &space = static_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
    space.MarkRememberSet([this, &stack](BaseObject *object) { EnqueueRefsToYoungCollectionSpace(object, stack); });
    stack.Publish();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkSatbBuffer(GlobalEvacuationStack &globalStack)
{
    PandaStack<BaseObject *> remarkStack;
    MutatorManager::Instance().VisitAllMutators([&remarkStack](ark::common_vm::Mutator &mutator) {
        const SatbBuffer::TreapNode *node = static_cast<const SatbBuffer::TreapNode *>(mutator.GetSatbBufferNode());
        if (node != nullptr) {
            const_cast<SatbBuffer::TreapNode *>(node)->GetObjects(remarkStack);
        }
    });
    SatbBuffer::Instance().GetRetiredObjects(remarkStack);

    LocalEvacuationStack localStack(&globalStack);
    while (!remarkStack.empty()) {
        BaseObject *obj = remarkStack.top();
        remarkStack.pop();
        CHECK(Heap::IsHeapAddress(obj));
        if (IsFromObject(obj)) {
            obj = ForwardObject(obj);
        }
        auto *region = RegionDesc::GetAliveRegionDescAt(obj);
        if (MarkObjectIfNotMarked(obj)) {
            auto size = EnqueueRefsToYoungCollectionSpaceAndGetSize(obj, localStack);
            region->AddLiveByteCount(size);
        }
    }
    localStack.Publish();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessEvacuationStack(GlobalEvacuationStack &globalEvacuationStack)
{
    const auto maxWorkers = GetGCThreadCount(true) - 1;
    if (maxWorkers > 0) {
        auto parallelCount = maxWorkers;
        ParallelMarkingMonitor monitor(parallelCount, parallelCount);

        auto *threadPool = GetThreadPool();
        for (uint32_t i = 0; i < parallelCount; ++i) {
            threadPool->PostTask(
                MakePandaUnique<ConcurrentEvacuationTask<LanguageConfig>>(0, *this, monitor, globalEvacuationStack));
        }
        ParallelLocalEvacuationStack stack(&globalEvacuationStack, &monitor);
        do {
            if (!monitor.TryStartStep()) {
                break;
            }
            ProcessEvacuationStack(stack);
            monitor.FinishStep();
        } while (monitor.WaitNextStepOrFinished());
        monitor.WaitAllFinished();
    } else {
        // serial marking with a single mark task.
        // NOTE: this `ParallelLocalMarkStack` could be replaced with `SequentialLocalMarkStack`, and no need to
        // use monitor, but this need to add template param to `ProcessMarkStack`.
        // So for convenience just use a fake dummy parallel one.
        ParallelMarkingMonitor dummyMonitor(0, 0);
        ParallelLocalEvacuationStack stack(&globalEvacuationStack, &dummyMonitor);
        ProcessEvacuationStack(stack);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessEvacuationStack(ParallelLocalEvacuationStack &stack)
{
    // NOTE: it is performed concurrently with other GC threads and mutators
    PandaStack<BaseObject *> remarkStack;
    auto fetchFromSatbBuffer = [this, &stack, &remarkStack]() {
        SatbBuffer::Instance().TryFetchOneRetiredNode(remarkStack);
        bool needProcess = false;
        while (!remarkStack.empty()) {
            BaseObject *obj = remarkStack.top();
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
        RefField<> *ref;
        while (stack.Pop(&ref)) {
            ProcessRef(*ref, stack);
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
void CmcGC<LanguageConfig>::MarkEvacuationStack(GlobalEvacuationStack &globalEvacuationStack)
{
    const auto maxWorkers = GetGCThreadCount(true) - 1;
    if (maxWorkers > 0) {
        auto parallelCount = maxWorkers;
        ParallelMarkingMonitor monitor(parallelCount, parallelCount);

        auto *threadPool = GetThreadPool();
        for (uint32_t i = 0; i < parallelCount; ++i) {
            threadPool->PostTask(MakePandaUnique<RemarkTask<LanguageConfig>>(0, *this, monitor, globalEvacuationStack));
        }
        ParallelLocalEvacuationStack stack(&globalEvacuationStack, &monitor);
        do {
            if (!monitor.TryStartStep()) {
                break;
            }
            MarkEvacuationStack(stack);
            monitor.FinishStep();
        } while (monitor.WaitNextStepOrFinished());
        monitor.WaitAllFinished();
    } else {
        // serial marking with a single mark task.
        // NOTE: this `ParallelLocalMarkStack` could be replaced with `SequentialLocalMarkStack`, and no need to
        // use monitor, but this need to add template param to `ProcessMarkStack`.
        // So for convenience just use a fake dummy parallel one.
        ParallelMarkingMonitor dummyMonitor(0, 0);
        ParallelLocalEvacuationStack stack(&globalEvacuationStack, &dummyMonitor);
        MarkEvacuationStack(stack);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkEvacuationStack(ParallelLocalEvacuationStack &stack)
{
    RefField<> *ref;
    while (stack.Pop(&ref)) {
        MarkRef(*ref, stack);
    }
}

template <class LanguageConfig>
template <typename Stack>
void CmcGC<LanguageConfig>::RemarkNonHeapRoot(RefField<> &ref, Stack &stack)
{
    auto *obj = ref.GetTargetObject();
    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    CHECK(Heap::GetHeap().GetGCReason() == ark::common_vm::GC_REASON_YOUNG);
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
template <typename Stack>
void CmcGC<LanguageConfig>::ProcessRef(RefField<> &ref, Stack &stack)
{
    RefField<> oldRef(ref);
    auto *obj = oldRef.GetTargetObject();
    if (!Heap::IsHeapAddress(obj)) {
        return;
    }

    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    if (region->IsFromRegion()) {
        auto *fwd = ForwardObject(obj);

        // Object can be evacuated by mutator so mark it
        if (MarkObjectIfNotMarked(fwd)) {
            EnqueueRefsToYoungCollectionSpace(fwd, stack);
        }
        RefField<> newRef(fwd, oldRef.IsWeak());
        ref.CompareExchange(oldRef.GetFieldValue(), newRef.GetFieldValue());
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
template <typename Stack>
void CmcGC<LanguageConfig>::MarkRef(RefField<> &ref, Stack &stack)
{
    auto *obj = ref.GetTargetObject();
    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    CHECK(Heap::GetHeap().GetGCReason() == ark::common_vm::GC_REASON_YOUNG);

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
        auto size = EnqueueRefsToYoungCollectionSpaceAndGetSize(obj, stack);
        region->AddLiveByteCount(size);
    }
}

template <class LanguageConfig>
template <typename Stack>
void CmcGC<LanguageConfig>::EnqueueRefsToYoungCollectionSpace(BaseObject *obj, Stack &stack)
{
    auto fieldHandler = [this, &stack](RefField<> &field) {
        if (InYoungCollectionSpace(field.GetTargetObject())) {
            stack.Push(&field);
        }
    };

    obj->ForEachRefField(fieldHandler, fieldHandler);
}

template <class LanguageConfig>
template <typename Stack>
size_t CmcGC<LanguageConfig>::EnqueueRefsToYoungCollectionSpaceAndGetSize(BaseObject *obj, Stack &stack)
{
    auto fieldHandler = [this, &stack](RefField<> &field) {
        if (InYoungCollectionSpace(field.GetTargetObject())) {
            stack.Push(&field);
        }
    };

    return obj->ForEachRefFieldAndGetSize(fieldHandler, fieldHandler);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(const RegionDesc::InlinedRegionMetaData *region) const
{
    return InYoungCollectionSpace(region->GetRegionType());
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(const RegionDesc *region) const
{
    return InYoungCollectionSpace(region->GetRegionType());
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(const BaseObject *obj) const
{
    return Heap::IsHeapAddress(obj) &&
           InYoungCollectionSpace(RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(obj));
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::InYoungCollectionSpace(RegionDesc::RegionType type) const
{
    // During single pass evacuation in young GC objects might be concurrently evacuated by mutator. So we should
    // inspect references which point to them too.
    return RegionDesc::IsInYoungSpace(type) || RegionDesc::IsInToSpace(type);
}

template <class LanguageConfig>
CArrayList<CArrayList<BaseObject *>> CmcGC<LanguageConfig>::EnumRootsFlip(STWParam &param,
                                                                          const ark::mem::RefFieldVisitor &visitor)
{
    const auto enumGlobalRoots = [this, &visitor]() {
        SetGCThreadQosPriority(PriorityMode::STW);
        EnumRootsImpl<VisitGlobalRoots>(visitor);
        SetGCThreadQosPriority(PriorityMode::FOREGROUND);
    };

    ark::os::memory::Mutex stackMutex;
    CArrayList<CArrayList<BaseObject *>> rootSet;  // allcate for each mutator
    FlipFunction enumMutatorRoot = [&rootSet, &stackMutex](ark::common_vm::Mutator &mutator) {
        CArrayList<BaseObject *> roots;
        RefFieldVisitor localVisitor = [&roots](RefField<> &root) { roots.emplace_back(root.GetTargetObject()); };
        mutator.VisitMutatorRoots(localVisitor);
        ark::os::memory::LockHolder lockGuard(stackMutex);
        rootSet.emplace_back(std::move(roots));
    };
    MutatorManager::Instance().FlipMutators(param, enumGlobalRoots, &enumMutatorRoot);
    return rootSet;
}

template <class LanguageConfig>
BaseObject *CmcGC<LanguageConfig>::ForwardObject(BaseObject *obj)
{
    BaseObject *to = TryForwardObject(obj);
    return (to != nullptr) ? to : obj;
}

template <class LanguageConfig>
BaseObject *CmcGC<LanguageConfig>::TryForwardObject(BaseObject *obj)
{
    return static_cast<BaseObject *>(CopyObjectImpl(obj));
}

// ConcurrentGC
template <class LanguageConfig>
ObjectHeader *CmcGC<LanguageConfig>::CopyObjectImpl(ObjectHeader *object)
{
    // reconsider phase difference between mutator and GC thread during transition.
    if (ark::common_vm::IsGcThread()) {
        CHECK(GetGCPhase() == ark::common_vm::GCPhase::GC_PHASE_PRECOPY ||
              GetGCPhase() == ark::common_vm::GCPhase::GC_PHASE_COPY ||
              GetGCPhase() == ark::common_vm::GCPhase::GC_PHASE_YOUNG_COPY ||
              GetGCPhase() == ark::common_vm::GCPhase::GC_PHASE_FIX ||
              GetGCPhase() == ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK);
    } else {
        auto phase = Mutator::GetMutator()->GetMutatorPhase();
        CHECK(phase == ark::common_vm::GCPhase::GC_PHASE_PRECOPY || phase == ark::common_vm::GCPhase::GC_PHASE_COPY ||
              phase == ark::common_vm::GCPhase::GC_PHASE_YOUNG_COPY || phase == ark::common_vm::GCPhase::GC_PHASE_FIX);
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
    COMMON_PHASE_TIMER("ClearAllGCInfo");
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    space.ClearAllGCInfo();
    reinterpret_cast<RegionalHeap &>(theAllocator_).ClearJitFortAwaitingMark();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::CollectSmallSpace()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CollectSmallSpace", "");
    auto &stats = GetGCStats();
    auto &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    {
        COMMON_PHASE_TIMER("CollectFromSpaceGarbage");
        stats.collectedBytes += stats.smallGarbageSize;
        space.CollectFromSpaceGarbage();
        space.HandlePromotion();
    }

    size_t candidateBytes = stats.fromSpaceSize + stats.nonMovableSpaceSize + stats.largeSpaceSize;
    stats.garbageRatio = (candidateBytes > 0) ? static_cast<float>(stats.collectedBytes) / candidateBytes : 0;

    stats.liveBytesAfterGC = space.GetAllocatedBytes();

    [[maybe_unused]] constexpr int logPercentagePrecision = 2;
    [[maybe_unused]] constexpr size_t logBasePercentage = 100UL;
    LOG(DEBUG, GC) << "collect " << stats.collectedBytes << " B: small " << stats.fromSpaceSize << " - "
                   << stats.smallGarbageSize << " B, non-movable " << stats.nonMovableSpaceSize << " - "
                   << stats.nonMovableGarbageSize << " B, large " << stats.largeSpaceSize << " - "
                   << stats.largeGarbageSize << " B. garbage ratio " << std::fixed
                   << std::setprecision(logPercentagePrecision) << stats.garbageRatio * logBasePercentage
                   << "%";  // The base of the percentage is 100
    OHOS_HITRACE(
        HITRACE_LEVEL_COMMERCIAL, "CMCGC::CollectSmallSpace END",
        ("collect:" + std::to_string(stats.collectedBytes) + "B;small:" + std::to_string(stats.fromSpaceSize) + "-" +
         std::to_string(stats.smallGarbageSize) + "B;non-movable:" + std::to_string(stats.nonMovableSpaceSize) + "-" +
         std::to_string(stats.nonMovableGarbageSize) + "B;large:" + std::to_string(stats.largeSpaceSize) + "-" +
         std::to_string(stats.largeGarbageSize) + "B;garbage ratio:" + std::to_string(stats.garbageRatio))
            .c_str());
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::SetGCThreadQosPriority(PriorityMode mode)
{
#ifdef ENABLE_QOS
    LOG(DEBUG, COMMON) << "SetGCThreadQosPriority gettid " << gettid();
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::SetGCThreadQosPriority", "");
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
void CmcGC<LanguageConfig>::UpdateBarrierEntrypoint(ark::common_vm::Mutator *mutator, ark::common_vm::GCPhase phase)
{
    if (phase >= ark::common_vm::GCPhase::GC_PHASE_ENUM && phase < ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK) {
        EnablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase >= ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK &&
               phase < ark::common_vm::GCPhase::GC_PHASE_PRECOPY) {
        DisableReadBarrier(static_cast<ark::Mutator *>(mutator));
        DisablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase >= ark::common_vm::GCPhase::GC_PHASE_PRECOPY &&
               phase < ark::common_vm::GCPhase::GC_PHASE_YOUNG_COPY) {
        EnableReadBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase == ark::common_vm::GCPhase::GC_PHASE_YOUNG_COPY) {
        EnableReadBarrier(static_cast<ark::Mutator *>(mutator));
        EnablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    } else if (phase == ark::common_vm::GCPhase::GC_PHASE_IDLE) {
        DisableReadBarrier(static_cast<ark::Mutator *>(mutator));
        DisablePreWriteBarrier(static_cast<ark::Mutator *>(mutator));
    }
    // Check if a new phase has been added
    ASSERT(phase == ark::common_vm::GCPhase::GC_PHASE_ENUM || phase == ark::common_vm::GCPhase::GC_PHASE_MARK ||
           phase == ark::common_vm::GCPhase::GC_PHASE_PRECOPY || phase == ark::common_vm::GCPhase::GC_PHASE_COPY ||
           phase == ark::common_vm::GCPhase::GC_PHASE_FIX || phase == ark::common_vm::GCPhase::GC_PHASE_IDLE ||
           phase == ark::common_vm::GCPhase::GC_PHASE_POST_MARK ||
           phase == ark::common_vm::GCPhase::GC_PHASE_FINAL_MARK ||
           phase == ark::common_vm::GCPhase::GC_PHASE_REMARK_SATB ||
           phase == ark::common_vm::GCPhase::GC_PHASE_YOUNG_COPY);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::OnMutatorTerminate(Mutator *mutator, [[maybe_unused]] MutatorUnregistrationMode mode,
                                               [[maybe_unused]] mem::BuffersKeepingFlag keepBuffers)
{
    GC::OnMutatorTerminate(mutator, mode, keepBuffers);

    DCHECK(!mutator->IsInRunningState());
    mutator->ReleaseAllocBuffer();

    if (mutator->GetMutatorType() == ark::Mutator::MutatorType::MANAGED) {
        auto &mutatorManager = MutatorManager::Instance();
        ark::os::memory::LockHolder guard(mutatorManager.allMutatorListLock_);
        auto &list = mutatorManager.allMutatorList_;
        auto it = std::find(list.begin(), list.end(), mutator);
        if (it != list.end()) {
            list.erase(it);
        }
    }
    UpdateBarrierEntrypoint(mutator, ark::common_vm::GCPhase::GC_PHASE_IDLE);
    mutator->SetMutatorPhase(ark::common_vm::GCPhase::GC_PHASE_IDLE);
    mutator->ResetMutator();
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::OnMutatorCreate(Mutator *mutator)
{
    ark::common_vm::GCPhase phase = GetGCPhase();
    if (mutator->GetMutatorType() == ark::Mutator::MutatorType::MANAGED) {
        auto &mutatorManager = MutatorManager::Instance();
        ark::os::memory::LockHolder<ark::os::memory::Mutex> guard(mutatorManager.allMutatorListLock_);
        DCHECK(std::find(mutatorManager.allMutatorList_.begin(), mutatorManager.allMutatorList_.end(), mutator) ==
               mutatorManager.allMutatorList_.end());
        if (UNLIKELY(mutatorManager.StwTriggered())) {
            mutator->SetSuspensionFlag(ark::SUSPEND_REQUEST);
        }
        mutatorManager.allMutatorList_.push_back(mutator);
    }
    mutator->SetMutatorPhase(phase);
    // Enable pre write barrier for mutators created during concurrent marking and enable read barrier for mutators
    // created during concurrent copy/fix.
    if (phase >= ark::common_vm::GCPhase::GC_PHASE_ENUM) {
        mutator->HandleGCPhase(phase);
        UpdateBarrierEntrypoint(mutator, phase);
    }
    GC::OnMutatorCreate(mutator);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::ShouldIgnoreRequest(ark::common_vm::GCRequest &request)
{
    return request.ShouldBeIgnored();
}

template <class LanguageConfig>
const size_t CmcGC<LanguageConfig>::MAX_MARKING_WORK_SIZE = 16;  // fork task if bigger
template <class LanguageConfig>
const size_t CmcGC<LanguageConfig>::MIN_MARKING_WORK_SIZE = 8;  // forbid forking task if smaller

template <class LanguageConfig, bool ProcessXRef>
class ConcurrentMarkingTask : public Task {
public:
    ConcurrentMarkingTask(uint32_t id, CmcGC<LanguageConfig> &tc, ParallelMarkingMonitor &monitor,
                          GlobalMarkStack &globalMarkStack)
        : Task(id), collector_(tc), monitor_(monitor), globalMarkStack_(globalMarkStack)
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
            collector_.template ProcessMarkStack<ProcessXRef>(threadIndex, markStack);
            monitor_.FinishStep();
        } while (monitor_.WaitNextStepOrFinished());
        monitor_.NotifyFinishOne();
        return true;
    }

private:
    CmcGC<LanguageConfig> &collector_;
    ParallelMarkingMonitor &monitor_;
    GlobalMarkStack &globalMarkStack_;
};

static void ClearWeakRef(WeakStack::value_type *begin, WeakStack::value_type *end)
{
    for (auto iter = begin; iter != end; ++iter) {
        RefField<> *fieldPointer = iter->first;
        size_t offset = iter->second;
        ASSERT_PRINT(offset % sizeof(RefField<>) == 0, "offset is not aligned");

        RefField<> &field = reinterpret_cast<RefField<> &>(*fieldPointer);
        RefField<> oldField(field);

        if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
            continue;
        }
        BaseObject *targetObj = oldField.GetTargetObject();
        DCHECK(Heap::IsHeapAddress(targetObj));

        auto targetRegion = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<MAddress>(targetObj));
        if (targetRegion->IsMarkedObject(targetObj) || targetRegion->IsNewObjectSinceMarking(targetObj)) {
            continue;
        }

        BaseObject *obj = reinterpret_cast<BaseObject *>(reinterpret_cast<uintptr_t>(&field) - offset);
        if (RegionDesc::GetAliveRegionType(reinterpret_cast<MAddress>(obj)) == RegionDesc::RegionType::FROM_REGION) {
            BaseObject *toObj = obj->GetForwardingPointer();

            // Make sure even the object that contains the weak reference is trimed before forwarding, the weak ref
            // field is still within the object
            if (toObj != nullptr && offset < obj->GetSize()) {
                RefField<> &toField = *reinterpret_cast<RefField<> *>(reinterpret_cast<uintptr_t>(toObj) + offset);
                toObj->ClearRef(toField);
            }
        }
        obj->ClearRef(field);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ProcessWeakStack(WeakStack &weakStack)
{
    WeakStack::value_type *begin = &(*weakStack.begin());
    WeakStack::value_type *end = begin + weakStack.size();
    ClearWeakRef(begin, end);
}

template <class LanguageConfig>
template <bool ProcessXRef>
void CmcGC<LanguageConfig>::ProcessMarkStack([[maybe_unused]] uint32_t threadIndex, ParallelLocalMarkStack &markStack)
{
    size_t nNewlyMarked = 0;
    WeakStack weakStack;
    auto visitor = CreateMarkingObjectRefFieldsVisitor(markStack, weakStack);
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
    // loop until work stack empty.
    while (true) {
        BaseObject *object;
        while (markStack.Pop(&object)) {
            ++nNewlyMarked;
            auto region = RegionDesc::GetAliveRegionDescAt(static_cast<MAddress>(reinterpret_cast<uintptr_t>(object)));
            visitor.SetMarkingRefFieldArgs(object);
            auto objSize =
                object->ForEachRefFieldAndGetSize(visitor.GetRefFieldVisitor(), visitor.GetWeakRefFieldVisitor());
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
    MergeWeakStack(weakStack);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MergeWeakStack(WeakStack &weakStack)
{
    ark::os::memory::LockHolder lock(weakStackLock_);

    // Preprocess the weak stack to minimize work during STW remark.
    for (const auto &pair : weakStack) {
        RefField<> oldField(*pair.first);

        if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
            continue;
        }
        auto obj = oldField.GetTargetObject();
        DCHECK(Heap::IsHeapAddress(obj));

        auto region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<MAddress>(obj));
        if (region->IsNewObjectSinceMarking(obj) || region->IsMarkedObject(obj)) {
            continue;
        }

        globalWeakStack_.push_back(pair);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::TracingSerial(GlobalMarkStack &globalMarkStack)
{
    // serial marking with a single mark task.
    // NOTE: this `ParallelLocalMarkStack` could be replaced with `SequentialLocalMarkStack`, and no need to
    // use monitor, but this need to add template param to `ProcessMarkStack`.
    // So for convenience just use a fake dummy parallel one.
    ParallelMarkingMonitor dummyMonitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &dummyMonitor);
#ifdef PANDA_JS_ETS_HYBRID_MODE
    if (gcReason_ == GCReason::GC_REASON_XREF) {
        ProcessMarkStack<true>(0, markStack);
    } else {
        ProcessMarkStack<false>(0, markStack);
    }
#else
    ProcessMarkStack<false>(0, markStack);
#endif
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::TracingImpl(GlobalMarkStack &globalMarkStack, bool parallel, bool Remark)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, ("CMCGC::TracingImpl_" + std::to_string(globalMarkStack.Count())).c_str(),
                 "");

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
            if (gcReason_ == GCReason::GC_REASON_XREF) {
                threadPool->PostTask(
                    MakePandaUnique<ConcurrentMarkingTask<LanguageConfig, true>>(0, *this, monitor, globalMarkStack));
            } else {
                threadPool->PostTask(
                    MakePandaUnique<ConcurrentMarkingTask<LanguageConfig, false>>(0, *this, monitor, globalMarkStack));
            }
#else
            threadPool->PostTask(
                MakePandaUnique<ConcurrentMarkingTask<LanguageConfig, false>>(0, *this, monitor, globalMarkStack));
#endif
        }
        ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
        do {
            if (!monitor.TryStartStep()) {
                break;
            }
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if (gcReason_ == GCReason::GC_REASON_XREF) {
                ProcessMarkStack<true>(0, markStack);
            } else {
                ProcessMarkStack<false>(0, markStack);
            }
#else
            ProcessMarkStack<false>(0, markStack);
#endif
            monitor.FinishStep();
        } while (monitor.WaitNextStepOrFinished());
        monitor.WaitAllFinished();
    } else {
        TracingSerial(globalMarkStack);
    }
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::PushRootToWorkStack(LocalCollectStack &collectStack, BaseObject *obj)
{
    RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
    if (gcReason_ == GCReason::GC_REASON_YOUNG && !regionInfo->IsInYoungSpace()) {
        LOG(DEBUG, GC) << "enum: skip old object " << obj << "<" << obj->GetTypeInfo() << ">(" << obj->GetSize() << ")";
        return false;
    }

    // inline MarkObject
    bool marked = regionInfo->MarkObject(obj);
    if (!marked) {
        DCHECK(!regionInfo->IsGarbageRegion());
        LOG(DEBUG, GC) << "mark obj " << obj << "<" << obj->GetTypeInfo() << ">(" << obj->GetSize() << ") in region "
                       << regionInfo << "(" << static_cast<size_t>(regionInfo->GetRegionType()) << ")@0x" << std::hex
                       << regionInfo->GetRegionStart() << std::dec << ", live " << regionInfo->GetLiveByteCount();
        collectStack.Push(obj);
        return true;
    } else {
        return false;
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PushRootsToWorkStack(LocalCollectStack &collectStack,
                                                 const CArrayList<BaseObject *> &collectedRoots)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL,
                 ("CMCGC::PushRootsToWorkStack_" + std::to_string(collectedRoots.size())).c_str(), "");
    for (BaseObject *obj : collectedRoots) {
        PushRootToWorkStack(collectStack, obj);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkingRoots(const CArrayList<BaseObject *> &collectedRoots)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::MarkingRoots", "");

    GlobalMarkStack globalMarkStack;

    {
        LocalCollectStack collectStack(&globalMarkStack);

        PushRootsToWorkStack(collectStack, collectedRoots);

        if (Heap::GetHeap().GetGCReason() == ark::common_vm::GC_REASON_YOUNG) {
            OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PushRootInRSet", "");
            auto func = [this, &collectStack](BaseObject *object) { MarkRememberSetImpl(object, collectStack); };
            RegionalHeap &space = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
            space.MarkRememberSet(func);
        }

        collectStack.Publish();
    }

    COMMON_PHASE_TIMER("MarkingRoots");

    ASSERT_PRINT(GetThreadPool() != nullptr, "null thread pool");

    // use fewer threads and lower priority for concurrent mark.
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;

    {
        COMMON_PHASE_TIMER("Concurrent marking");
        TracingImpl(globalMarkStack, maxWorkers > 0, false);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::Remark()
{
    GlobalMarkStack globalMarkStack;
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::Remark[STW]", "");
    COMMON_PHASE_TIMER("STW re-marking");
    ConcurrentRemark(globalMarkStack, maxWorkers > 0);  // Mark enqueue
    TracingImpl(globalMarkStack, maxWorkers > 0, true);
    PreforwardStaticRoots();
    MarkAwaitingJitFort();  // Mark awaiting
    ClearWeakStack(maxWorkers > 0);

    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::MarkingRoots END",
                 // Atomic with relaxed order reason: data race with markedObjectCount_ with no synchronization or
                 // ordering constraints imposed on other reads or writes
                 ("mark obejects:" + std::to_string(markedObjectCount_.load(std::memory_order_relaxed))).c_str());
    // Atomic with relaxed order reason: data race with markedObjectCount_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    LOG(DEBUG, GC) << "mark " << markedObjectCount_.load(std::memory_order_relaxed) << " objects";
}

class ClearWeakRefTask : public ark::common_vm::ArrayTaskDispatcher::ArrayTask {
public:
    using T = WeakStack::value_type;
    explicit ClearWeakRefTask(CArrayList<T> *inputs) : data_(inputs) {}
    void Run(void *begin, void *end) override
    {
        T *first = reinterpret_cast<T *>(begin);
        T *last = reinterpret_cast<T *>(end);
        DCHECK(((uintptr_t)last - (uintptr_t)first) % sizeof(T) == 0 &&
               ((uintptr_t)first - (uintptr_t)data_->data()) % sizeof(T) == 0);
        DCHECK(first == last || (first >= data_->data() && last > data_->data()));
        DCHECK(first == last || (first < (data_->data() + data_->size()) && last <= (data_->data() + data_->size())));
        ClearWeakRef(first, last);
    }
    CArrayList<T> *data_;
};

template <class LanguageConfig>
void CmcGC<LanguageConfig>::ClearWeakStack(bool parallel)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::ProcessGlobalWeakStack", "");
    if (gcReason_ == ark::common_vm::GC_REASON_YOUNG || globalWeakStack_.empty()) {
        return;
    }
    // the globalWeakStack_ cannot be modified during task execution
    auto *threadPool = GetThreadPool();
    ASSERT_PRINT(threadPool != nullptr, "thread pool is null");
    constexpr size_t BATCH_N = 200;
    if (parallel && globalWeakStack_.size() > BATCH_N) {
        auto inputs = &globalWeakStack_;
        ClearWeakRefTask callback(inputs);
        DCHECK(inputs->size() < (SIZE_MAX / sizeof(WeakStack::value_type)));
        ark::common_vm::ArrayTaskDispatcher dispatcher(inputs->data(), inputs->size() * sizeof(WeakStack::value_type),
                                                       BATCH_N * sizeof(WeakStack::value_type), &callback);
        dispatcher.Dispatch(threadPool, threadPool->GetTotalThreadNum());
        dispatcher.JoinAndWait();
    } else {
        ProcessWeakStack(globalWeakStack_);
    }
    globalWeakStack_.clear();
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::MarkSatbBuffer(GlobalMarkStack &globalMarkStack)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::MarkSatbBuffer", "");
    COMMON_PHASE_TIMER("MarkSatbBuffer");
    auto visitSatbObj = [this, &globalMarkStack]() {
        PandaStack<BaseObject *> remarkStack;
        auto func = [&remarkStack](ark::common_vm::Mutator &mutator) {
            const SatbBuffer::TreapNode *node = static_cast<const SatbBuffer::TreapNode *>(mutator.GetSatbBufferNode());
            if (node != nullptr) {
                const_cast<SatbBuffer::TreapNode *>(node)->GetObjects(remarkStack);
            }
        };
        MutatorManager::Instance().VisitAllMutators(func);
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
            RegionDesc *region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(targetObj));
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
void CmcGC<LanguageConfig>::MarkAwaitingJitFort()
{
    reinterpret_cast<RegionalHeap &>(theAllocator_).MarkAwaitingJitFort();
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
template <class LanguageConfig>
void CmcGC<LanguageConfig>::DumpHeap(const CString &tag)
{
    ASSERT_PRINT(MutatorManager::Instance().WorldStopped(), "Not In STW");
    LOG(DEBUG, GC) << "DumpHeap " << tag.Str();
    // dump roots
    DumpRoots(FRAGMENT);
    // dump object contents
    auto dumpVisitor = [](BaseObject *obj) {
        // support Dump
        // obj->DumpObject(FRAGMENT)
    };
    bool ret = Heap::GetHeap().ForEachObject(dumpVisitor, false);
    LOG_IF(UNLIKELY(!ret), ERROR, GC) << "theAllocator.ForEachObject() in DumpHeap() return false.";

    // dump object types
    LOG(DEBUG, GC) << "Print Type information";
    PandaSet<TypeInfo *> classinfoSet;
    auto assembleClassInfoVisitor = [&classinfoSet](BaseObject *obj) {
        TypeInfo *classInfo = obj->GetTypeInfo();
        // No need to check the result of insertion, because there are multiple-insertions.
        (void)classinfoSet.insert(classInfo);
    };
    ret = Heap::GetHeap().ForEachObject(assembleClassInfoVisitor, false);
    LOG_IF(UNLIKELY(!ret), ERROR, GC) << "theAllocator.ForEachObject()#2 in DumpHeap() return false.";

    for (auto it = classinfoSet.begin(); it != classinfoSet.end(); it++) {
        TypeInfo *classInfo = *it;
    }
    LOG(DEBUG, GC) << "Dump Allocator";
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::DumpRoots(LogType logType)
{
    LOG(FATAL, COMMON) << "Unresolved fatal";
    UNREACHABLE();
}
#endif

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PreGarbageCollection(bool isConcurrent)
{
    // SatbBuffer should be initialized before concurrent enumeration.
    SatbBuffer::Instance().Init();
    // prepare thread pool.

    auto &gcStats = GetGCStats();
    gcStats.reason = gcReason_;
    gcStats.async = !ark::common_vm::g_gcRequests[gcReason_].IsSyncGC();
    gcStats.gcType = gcType_;
    gcStats.isConcurrentMark = isConcurrent;
    gcStats.collectedBytes = 0;
    gcStats.smallGarbageSize = 0;
    gcStats.nonMovableGarbageSize = 0;
    gcStats.largeGarbageSize = 0;
    gcStats.gcStartTime = ark::common_vm::TimeUtil::NanoSeconds();
    gcStats.totalSTWTime = 0;
    gcStats.maxSTWTime = 0;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    DumpBeforeGC();
#endif
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::PostGarbageCollection(uint64_t gcIndex)
{
    SatbBuffer::Instance().ReclaimALLPages();
    // release pages in PagePool
    ark::common_vm::PagePool::Instance().Trim();
    collectorResources_.MarkGCFinish(gcIndex);

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    DumpAfterGC();
#endif
}

template <class LanguageConfig>
// CC-OFFNXT(G.FUD.05) solid logic
void CmcGC<LanguageConfig>::UpdateGCStats()
{
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    auto &gcStats = GetGCStats();
    gcStats.Dump();

    size_t oldThreshold = gcStats.heapThreshold;
    size_t oldTargetFootprint = gcStats.targetFootprint;
    size_t recentBytes = space.GetRecentAllocatedSize();
    size_t survivedBytes = space.GetSurvivedSize();
    size_t bytesAllocated = space.GetAllocatedBytes();

    size_t targetSize;
    Heap *heap = &common_vm::Heap::GetHeap();
    const HeapParam &heapParam = heap->GetHeapParam();
    GCParam &gcParam = heap->GetGCParam();
    if (!gcStats.isYoungGC()) {
        gcStats.shouldRequestYoung = true;
        size_t delta = bytesAllocated * (1.0 / heapParam.heapUtilization - 1.0);
        size_t growBytes = std::min(delta, gcParam.maxGrowBytes);
        growBytes = std::max(growBytes, gcParam.minGrowBytes);
        targetSize = bytesAllocated + growBytes * gcParam.multiplier;
    } else {
        gcStats.shouldRequestYoung =
            gcStats.collectionRate * gcParam.ygcRateAdjustment >= ark::common_vm::g_fullGCMeanRate &&
            bytesAllocated <= oldThreshold;
        size_t adjustMaxGrowBytes = gcParam.maxGrowBytes * gcParam.multiplier;
        if (bytesAllocated + adjustMaxGrowBytes < oldTargetFootprint) {
            targetSize = bytesAllocated + adjustMaxGrowBytes;
        } else {
            targetSize = std::max(bytesAllocated, oldTargetFootprint);
        }
    }

    gcStats.targetFootprint = targetSize;
    size_t remainingBytes = recentBytes;
    remainingBytes = std::min(remainingBytes, gcParam.kMaxConcurrentRemainingBytes);
    remainingBytes = std::max(remainingBytes, gcParam.kMinConcurrentRemainingBytes);
    if (UNLIKELY(remainingBytes > gcStats.targetFootprint)) {
        remainingBytes = std::min(gcParam.kMinConcurrentRemainingBytes, gcStats.targetFootprint);
    }
    gcStats.heapThreshold = std::max(gcStats.targetFootprint - remainingBytes, bytesAllocated);
    gcStats.heapThreshold = std::max(gcStats.heapThreshold, 20 * MB);  // 20 MB:set 20 MB as min heapThreshold
    gcStats.heapThreshold = std::min(gcStats.heapThreshold, gcParam.gcThreshold);

    UpdateNativeThreshold(gcParam);
    Heap::GetHeap().RecordAliveSizeAfterLastGC(bytesAllocated);
    if (!gcStats.isYoungGC()) {
        Heap::GetHeap().SetRecordHeapObjectSizeBeforeSensitive(bytesAllocated);
    }

    if (!gcStats.isYoungGC()) {
        ark::common_vm::g_gcRequests[ark::common_vm::GC_REASON_HEU].SetMinInterval(gcParam.gcInterval);
    } else {
        ark::common_vm::g_gcRequests[ark::common_vm::GC_REASON_YOUNG].SetMinInterval(gcParam.gcInterval);
    }
    gcStats.IncreaseAccumulatedFreeSize(bytesAllocated);
    PandaOStringStream oss;
    oss << "allocated bytes " << bytesAllocated << " (survive bytes " << survivedBytes << ", recent-allocated "
        << recentBytes << "), update target footprint " << oldTargetFootprint << " -> " << gcStats.targetFootprint
        << ", update gc threshold " << oldThreshold << " -> " << gcStats.heapThreshold;
    LOG(DEBUG, GC) << oss.str();
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::UpdateGCStats END",
                 ("allocated bytes:" + std::to_string(bytesAllocated) + ";survive bytes:" +
                  std::to_string(survivedBytes) + ";recent allocated:" + std::to_string(recentBytes) +
                  ";update target footprint:" + std::to_string(oldTargetFootprint) + ";new target footprint:" +
                  std::to_string(gcStats.targetFootprint) + ";old gc threshold:" + std::to_string(oldThreshold) +
                  ";new gc threshold:" + std::to_string(gcStats.heapThreshold) +
                  ";native size:" + std::to_string(Heap::GetHeap().GetNotifiedNativeSize()) +
                  ";new native threshold:" + std::to_string(Heap::GetHeap().GetNativeHeapThreshold()))
                     .c_str());
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
void CmcGC<LanguageConfig>::ReclaimGarbageMemory(GCReason reason)
{
    if (reason == ark::common_vm::GC_REASON_OOM) {
        Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(true);
    } else {
        Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(false);
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RunGarbageCollection(uint64_t gcIndex, ark::common_vm::GCReason reason,
                                                 ark::common_vm::GCType gcType)
{
    MarkGCStart();
    gcReason_ = reason;
    gcType_ = gcType;
    auto gcReasonName = PandaString(ark::common_vm::g_gcRequests[gcReason_].name);
    auto currentAllocatedSize = Heap::GetHeap().GetAllocatedSize();
    auto currentThreshold = GetGCStats().GetThreshold();
    LOG(DEBUG, GC) << "Begin GC log. GCReason: " << gcReasonName << ", GCType: " << GCTypeToString(gcType)
                   << ", Current allocated " << ::ark::mem::Pretty(currentAllocatedSize) << ", Current threshold "
                   << ::ark::mem::Pretty(currentThreshold) << ", gcIndex=" << gcIndex;
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::RunGarbageCollection",
                 ("GCReason:" + gcReasonName + ";GCType:" + GCTypeToString(gcType) +
                  ";Sensitive:" + std::to_string(static_cast<int>(Heap::GetHeap().GetSensitiveStatus())) +
                  ";Startup:" + std::to_string(static_cast<int>(Heap::GetHeap().GetStartupStatus())) +
                  ";Current Allocated:" + ::ark::mem::Pretty(currentAllocatedSize) +
                  ";Current Threshold:" + ::ark::mem::Pretty(currentThreshold) +
                  ";Current Native:" + ::ark::mem::Pretty(Heap::GetHeap().GetNotifiedNativeSize()) +
                  ";NativeThreshold:" + ::ark::mem::Pretty(Heap::GetHeap().GetNativeHeapThreshold()))
                     .c_str());
    // prevent other threads stop-the-world during GC.
    // this may be removed in the future.
    ark::common_vm::ScopedSTWLock stwLock;
    PreGarbageCollection(true);
    Heap::GetHeap().SetGCReason(reason);
    auto &gcStats = GetGCStats();

    DoGarbageCollection();

    HeapBitmapManager::GetHeapBitmapManager().ClearHeapBitmap();

    ReclaimGarbageMemory(reason);

    // Call Recalculate byte-level heap footprint after ReclaimGarbageMemory
    // (garbage regions have been already reclaimed here).
    // Now safe to call RecalculateFootprint, because no mutator can allocate concurrently
    Heap::GetHeap().GetAllocator().RecalculateFootprint();

    PostGarbageCollection(gcIndex);
    MutatorManager::Instance().DestroyExpiredMutators();
    gcStats.gcEndTime = ark::common_vm::TimeUtil::NanoSeconds();

    UpdateGCCompletionStats(gcStats);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::UpdateGCCompletionStats(ark::common_vm::GCStats &gcStats)
{
    uint64_t gcTimeNs = gcStats.gcEndTime - gcStats.gcStartTime;
    double rate =
        (static_cast<double>(gcStats.collectedBytes) / gcTimeNs) * (static_cast<double>(ark::common_vm::NS_PER_S) / MB);
    {
        PandaOStringStream oss;
        const int prec = 3;
        oss << "total gc time: " << ::ark::mem::Pretty(gcTimeNs / ark::common_vm::NS_PER_US) << " us, collection rate ";
        oss << std::setprecision(prec) << rate << " MB/s";
        LOG(DEBUG, GC) << oss.str();
    }

    ark::common_vm::g_gcCount++;
    ark::common_vm::g_gcTotalTimeUs += (gcTimeNs / ark::common_vm::NS_PER_US);
    ark::common_vm::g_gcCollectedTotalBytes += gcStats.collectedBytes;
    gcStats.collectionRate = rate;

    if (!gcStats.isYoungGC()) {
        if (ark::common_vm::g_fullGCCount == 0) {
            ark::common_vm::g_fullGCMeanRate = rate;
        } else {
            ark::common_vm::g_fullGCMeanRate =
                (ark::common_vm::g_fullGCMeanRate * ark::common_vm::g_fullGCCount + rate) /
                (ark::common_vm::g_fullGCCount + 1);
        }
        ark::common_vm::g_fullGCCount++;
    }

    UpdateGCStats();

    if (Heap::GetHeap().GetForceThrowOOM()) {
        // NOTE (shemetov.philip, #34958) Workaround to fix OOM for GC with JIT enabled
        RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
        auto gcStats = GetGCStats();
        size_t maxCapacity = space.GetMaxCapacity();
        if (gcStats.liveBytesAfterGC * 2U > maxCapacity) {
            Heap::throwOOM();
        } else {
            Heap::GetHeap().SetForceThrowOOM(false);
        }
    }
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::CopyFromSpace()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CopyFromSpace", "");
    STWParam stwParam {"GC_PHASE_COPY transition"};
    {
        ScopedStopTheWorld stw(stwParam);
        TransitionToGCPhase(ark::common_vm::GCPhase::GC_PHASE_COPY);
    }
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    auto &stats = GetGCStats();
    stats.liveBytesBeforeGC = space.GetAllocatedBytes();
    stats.fromSpaceSize = space.FromSpaceSize();
    space.CopyFromSpace(GetThreadPool());

    stats.smallGarbageSize = space.FromRegionSize() - space.ToSpaceSize();
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
    return RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj))->IsToRegion();
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
ark::common_vm::GCStats &CmcGC<LanguageConfig>::GetGCStats()
{
    return collectorResources_.GetGCStats();
}
template <class LanguageConfig>
void CmcGC<LanguageConfig>::RequestGCInternal(ark::common_vm::GCReason reason, bool async,
                                              ark::common_vm::GCType gcType, bool explicitRequest)
{
    collectorResources_.RequestGC(reason, async, gcType, explicitRequest);
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
    this->SetType(GCType::CMC_GC);
    this->SetTLABsSupported();
    Heap::GetHeap().SetCollector(this);
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::InitializeImpl()
{
    ark::mem::InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrierSet =
        allocator->New<GCCMCBarrierSet>(allocator, ark::helpers::math::GetIntLog2(ark::mem::RegionDesc::UNIT_SIZE));
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);
    LOG(DEBUG, GC) << "CMC GC adapter initialized...";
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::RunPhasesImpl([[maybe_unused]] ark::GCTask &task)
{
    LOG(DEBUG, GC) << "CMC GC adapter RunPhases...";
    ark::mem::GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats());
}

// NOLINTNEXTLINE(misc-unused-parameters)
template <class LanguageConfig>
bool CmcGC<LanguageConfig>::WaitForGC([[maybe_unused]] ark::GCTask task)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    ark::common_vm::GCReason reason = ark::common_vm::GCReason::GC_REASON_INVALID;
    ark::common_vm::GCType type = ark::common_vm::GCType::GC_TYPE_FULL;
    switch (task.reason) {
        case ark::GCTaskCause::YOUNG_GC_CAUSE:
            reason = ark::common_vm::GCReason::GC_REASON_YOUNG;
            type = ark::common_vm::GCType::GC_TYPE_YOUNG;
            break;
        case ark::GCTaskCause::PYGOTE_FORK_CAUSE:
            reason = ark::common_vm::GCReason::GC_REASON_USER;
            break;
        case ark::GCTaskCause::STARTUP_COMPLETE_CAUSE:
            reason = ark::common_vm::GCReason::GC_REASON_HINT;
            break;
        case ark::GCTaskCause::NATIVE_ALLOC_CAUSE:
            reason = ark::common_vm::GCReason::GC_REASON_NATIVE;
            break;
        case ark::GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE:
        case ark::GCTaskCause::MIXED:
            reason = ark::common_vm::GCReason::GC_REASON_HEU;
            break;
        case ark::GCTaskCause::EXPLICIT_CAUSE:
            reason = ark::common_vm::GCReason::GC_REASON_FORCE;
            break;
        case ark::GCTaskCause::OOM_CAUSE:
            reason = ark::common_vm::GCReason::GC_REASON_OOM;
            break;
        case ark::GCTaskCause::CROSSREF_CAUSE:
            reason = ark::common_vm::GCReason::GC_REASON_BACKUP;
            break;
        default:
            UNREACHABLE();
    }
    ark::common_vm::HeapManager::RequestGC(reason, false, type);
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
bool CmcGC<LanguageConfig>::Trigger([[maybe_unused]] ark::PandaUniquePtr<ark::GCTask> task)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    ark::common_vm::HeapManager::RequestGC(ark::common_vm::GCReason::GC_REASON_OOM, false,
                                           ark::common_vm::GCType::GC_TYPE_FULL);
#endif  // ARK_USE_COMMON_RUNTIME
    return false;
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsPostponeGCSupported() const
{
    return false;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::StopGC()
{
#if defined(ARK_USE_COMMON_RUNTIME)
    ark::common_vm::Heap::GetHeap().StopGCWork();
#endif  // ARK_USE_COMMON_RUNTIME
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkObject(ObjectHeader *object)
{
    MarkObjectIfNotMarked(object);
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::MarkObjectIfNotMarked(ObjectHeader *object)
{
    bool marked = RegionalHeap::MarkObject(reinterpret_cast<BaseObject *>(object));
    if (!marked) {
        [[maybe_unused]] RegionDesc *region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(object));
        DCHECK(!region->IsGarbageRegion());
        LOG(DEBUG, GC) << "mark obj " << GetDebugInfoAboutObject(object);
    }
    return !marked;
}

template <class LanguageConfig>
bool CmcGC<LanguageConfig>::IsMarked([[maybe_unused]] const ark::ObjectHeader *object) const
{
    return false;
}

template <class LanguageConfig>
void CmcGC<LanguageConfig>::MarkReferences([[maybe_unused]] ark::mem::GCMarkingStackType *references,
                                           [[maybe_unused]] ark::mem::GCPhase gcPhase)
{
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(CmcGC);

}  // namespace ark::mem

#endif  // #if defined(ARK_USE_COMMON_RUNTIME)
