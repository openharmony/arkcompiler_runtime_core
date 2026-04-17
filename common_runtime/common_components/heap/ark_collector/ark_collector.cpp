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
#include "common_components/heap/ark_collector/ark_collector.h"

#include "common_components/log/log.h"
#include "common_components/mutator/mutator_manager-inl.h"
#include "common_components/heap/verification.h"
#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/regional_heap.h"

#ifdef ENABLE_QOS
#include "qos.h"
#endif

namespace common_vm {
bool ArkCollector::IsUnmovableFromObject(BaseObject* obj) const
{
    // filter const string object.
    if (!Heap::IsHeapAddress(obj)) {
        return false;
    }

    RegionDesc* regionInfo = nullptr;
    regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj));
    return regionInfo->IsUnmovableFromRegion();
}

bool ArkCollector::MarkObject(BaseObject* obj) const
{
    bool marked = RegionalHeap::MarkObject(obj);
    if (!marked) {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        DCHECK_CC(!region->IsGarbageRegion());
        DLOG(TRACE, "mark obj %p<%p> in region %p(%u)@%#zx, live %u", obj, obj->GetTypeInfo(),
             region, region->GetRegionType(), region->GetRegionStart(), region->GetLiveByteCount());
    }
    return marked;
}

// this api updates current pointer as well as old pointer, caller should take care of this.
template<bool copy>
bool ArkCollector::TryUpdateRefFieldImpl(BaseObject* obj, RefField<>& field, BaseObject*& fromObj,
                                         BaseObject*& toObj) const
{
    RefField<> oldRef(field);
    fromObj = oldRef.GetTargetObject();
    if (IsFromObject(fromObj)) { //LCOV_EXCL_BR_LINE
        if (copy) { //LCOV_EXCL_BR_LINE
            toObj = const_cast<ArkCollector*>(this)->TryForwardObject(fromObj);
        } else { //LCOV_EXCL_BR_LINE
            toObj = FindToVersion(fromObj);
        }
        if (toObj == nullptr) { //LCOV_EXCL_BR_LINE
            return false;
        }
        RefField<> tmpField(toObj, oldRef.IsWeak());
        if (field.CompareExchange(oldRef.GetFieldValue(), tmpField.GetFieldValue())) { //LCOV_EXCL_BR_LINE
            if (obj != nullptr) { //LCOV_EXCL_BR_LINE
                DLOG(TRACE, "update obj %p<%p>(%zu)+%zu ref-field@%p: %#zx -> %#zx", obj, obj->GetTypeInfo(),
                    obj->GetSize(), BaseObject::FieldOffset(obj, &field), &field, oldRef.GetFieldValue(),
                    tmpField.GetFieldValue());
                LOG_MM_OBJ_EVENTS(DEBUG) << "BARRIER ref in obj " << obj
                                         << " field@" << &field
                                         << ": " << fromObj << " -> " << toObj;
            } else { //LCOV_EXCL_BR_LINE
                DLOG(TRACE, "update ref@%p: 0x%zx -> %p", &field, oldRef.GetFieldValue(), toObj);
                LOG_MM_OBJ_EVENTS(DEBUG) << "BARRIER ref @" << &field
                                         << ": " << fromObj << " -> " << toObj;
            }
            return true;
        } else { //LCOV_EXCL_BR_LINE
            if (obj != nullptr) { //LCOV_EXCL_BR_LINE
                DLOG(TRACE,
                    "update obj %p<%p>(%zu)+%zu but cas failed ref-field@%p: %#zx(%#zx) -> %#zx but cas failed ",
                     obj, obj->GetTypeInfo(), obj->GetSize(), BaseObject::FieldOffset(obj, &field), &field,
                     oldRef.GetFieldValue(), field.GetFieldValue(), tmpField.GetFieldValue());
            } else { //LCOV_EXCL_BR_LINE
                DLOG(TRACE, "update but cas failed ref@%p: 0x%zx(%zx) -> %p", &field, oldRef.GetFieldValue(),
                     field.GetFieldValue(), toObj);
            }
            return true;
        }
    }

    return false;
}

bool ArkCollector::TryUpdateRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const
{
    BaseObject* oldRef = nullptr;
    return TryUpdateRefFieldImpl<false>(obj, field, oldRef, newRef);
}

bool ArkCollector::TryForwardRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const
{
    BaseObject* oldRef = nullptr;
    return TryUpdateRefFieldImpl<true>(obj, field, oldRef, newRef);
}

static void MarkingRefField(BaseObject *obj, BaseObject *targetObj, RefField<> &field,
                            ParallelLocalMarkStack &markStack, RegionDesc *targetRegion);
// note each ref-field will not be marked twice, so each old pointer the markingr meets must come from previous gc.
static void MarkingRefField(BaseObject *obj, RefField<> &field, ParallelLocalMarkStack &markStack,
                            const GCReason gcReason)
{
    RefField<> oldField(field);
    BaseObject* targetObj = oldField.GetTargetObject();

    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // field is tagged object, should be in heap
    DCHECK_CC(Heap::IsHeapAddress(targetObj));

    auto targetRegion = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<MAddress>((void*)targetObj));
    // cannot skip objects in EXEMPTED_FROM_REGION, because its rset is incomplete
    if (gcReason == GC_REASON_YOUNG && !targetRegion->IsInYoungSpace()) {
        DLOG(TRACE, "marking: skip non-young object %p@%p, target object: %p<%p>(%zu)",
            obj, &field, targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
        return;
    }
    common_vm::MarkingRefField(obj, targetObj, field, markStack, targetRegion);
}

// note each ref-field will not be marked twice, so each old pointer the markingr meets must come from previous gc.
static void MarkingRefField(BaseObject *obj, BaseObject *targetObj, RefField<> &field,
                            ParallelLocalMarkStack &markStack, RegionDesc *targetRegion)
{
    if (targetRegion->IsNewObjectSinceMarking(targetObj)) {
        DLOG(TRACE, "marking: skip new obj %p<%p>(%zu)", targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
        return;
    }

    if (targetRegion->MarkObject(targetObj)) {
        DLOG(TRACE, "marking: obj has been marked %p", targetObj);
        return;
    }

    DLOG(TRACE, "marking obj %p ref@%p: %p<%p>(%zu)",
        obj, &field, targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
    markStack.Push(targetObj);
}

// GC passes this function to ObjectOperator::ForEachRefField.
// ObjectOperator must call it for reference fields of WeakRef objects.
// Its responcibility of ObjectOperator to determine whether 'obj' is kind of WeakRef or not.
static void MarkWeakRefField(BaseObject *obj, RefField<> &field, WeakStack &weakStack, const GCReason gcReason)
{
    RefField<> oldField(field);
    BaseObject* targetObj = oldField.GetTargetObject();

    if (gcReason == GC_REASON_YOUNG) {
        return;
    }

    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // field is tagged object, should be in heap
    DCHECK_CC(Heap::IsHeapAddress(targetObj));
    DLOG(TRACE, "marking: skip weak obj when full gc, object: %p@%p, targetObj: %p", obj, &field, targetObj);
    // weak ref is cleared after roots pre-forward, so there might be a to-version weak ref which also need to be
    // cleared, offset recorded here will help us find it
    weakStack.emplace_back(&field, reinterpret_cast<uintptr_t>(&field) - reinterpret_cast<uintptr_t>(obj));
}

MarkingCollector::MarkingRefFieldVisitor ArkCollector::CreateMarkingObjectRefFieldsVisitor(
    ParallelLocalMarkStack &markStack, WeakStack &weakStack)
{
    MarkingRefFieldVisitor visitor;

    if (gcReason_ == GCReason::GC_REASON_YOUNG) {
        auto func = [this, obj = visitor.GetClosure(), &markStack](RefField<> &field) {
            MarkingRefField(*obj, field, markStack, GCReason::GC_REASON_YOUNG);
        };
        visitor.SetVisitor(func);
        visitor.SetWeakVisitor(func);
    } else {
        visitor.SetVisitor([obj = visitor.GetClosure(), &markStack](RefField<> &field) {
            MarkingRefField(*obj, field, markStack, GCReason::GC_REASON_HEU);
        });
        visitor.SetWeakVisitor([obj = visitor.GetClosure(), &weakStack](RefField<> &field) {
            MarkWeakRefField(*obj, field, weakStack, GCReason::GC_REASON_HEU);
        });
    }
    return visitor;
}

#ifdef PANDA_JS_ETS_HYBRID_MODE
// note each ref-field will not be traced twice, so each old pointer the tracer meets must come from previous gc.
void ArkCollector::MarkingXRef(RefField<> &field, ParallelLocalMarkStack &workStack) const
{
    BaseObject* targetObj = field.GetTargetObject();
    auto region = RegionDesc::GetRegionDescAt(reinterpret_cast<MAddress>(targetObj));
    // field is tagged object, should be in heap
    DCHECK_CC(Heap::IsHeapAddress(targetObj));

    DLOG(TRACE, "trace obj %p <%p>(%zu)", targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
    if (region->IsNewObjectSinceForward(targetObj)) {
        DLOG(TRACE, "trace: skip new obj %p<%p>(%zu)", targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
        return;
    }
    DCHECK_CC(!field.IsWeak());
    if (!region->MarkObject(targetObj)) {
        workStack.Push(targetObj);
    }
}

void ArkCollector::MarkingObjectXRef(BaseObject *obj, ParallelLocalMarkStack &workStack)
{
    auto refFunc = [this, &workStack] (RefField<>& field) {
        MarkingXRef(field, workStack);
    };

    obj->IterateXRef(refFunc);
}
#endif

void ArkCollector::FixRefField(BaseObject* obj, RefField<>& field) const
{
    RefField<> oldField(field);
    BaseObject* targetObj = oldField.GetTargetObject();
    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // target object could be null or non-heap for some static variable.
    if (!Heap::IsHeapAddress(targetObj)) {
        return;
    }

    RegionDesc::InlinedRegionMetaData *refRegion =  RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
        reinterpret_cast<uintptr_t>(targetObj));
    bool isFrom = refRegion->IsFromRegion();
    bool isInRcent = refRegion->IsInRecentSpace();
    if (isInRcent) {
        RegionDesc::InlinedRegionMetaData *objRegion =  RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
            reinterpret_cast<uintptr_t>(obj));
        if (!objRegion->IsInRecentSpace() &&
            objRegion->MarkRSetCardTable(obj)) {
            DLOG(TRACE,
                 "fix phase update point-out remember set of region %p, obj "
                 "%p, ref: <%p>",
                 objRegion, obj, targetObj->GetTypeInfo());
        }
        return;
    } else if (!isFrom) {
        return;
    }
    BaseObject* latest = FindToVersion(targetObj);

    if (latest == nullptr) { return; }

    CHECK_CC(latest->IsValidObject());
    RefField<> newField(latest, oldField.IsWeak());
    if (field.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
        DLOG(FIX, "fix obj %p+%zu ref@%p: %#zx => %p<%p>(%zu)", obj, obj->GetSize(), &field,
             oldField.GetFieldValue(), latest, latest->GetTypeInfo(), latest->GetSize());
        LOG_MM_OBJ_EVENTS(DEBUG) << "FIX ref in obj " << obj
                                 << " field@" << &field
                                 << ": " << targetObj << " -> " << latest;
    }
}

void ArkCollector::FixObjectRefFields(BaseObject* obj) const
{
    DLOG(FIX, "fix obj %p<%p>(%zu)", obj, obj->GetTypeInfo(), obj->GetSize());
    auto refFunc = [this, obj](RefField<>& field) { FixRefField(obj, field); };
    obj->ForEachRefField(refFunc, refFunc);
}

class RemarkAndPreforwardVisitor {
public:
    RemarkAndPreforwardVisitor(LocalCollectStack &collectStack, ArkCollector *collector)
        : collectStack_(collectStack), collector_(collector) {}

    void operator()(RefField<> &refField)
    {
        RefField<> oldField(refField);
        BaseObject* oldObj = oldField.GetTargetObject();
        DLOG(FIX, "visit raw-ref @%p: %p", &refField, oldObj);

        auto regionType =
            RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(oldObj))
                ->GetRegionType();
        if (regionType == RegionDesc::RegionType::FROM_REGION) {
            BaseObject* toVersion = collector_->TryForwardObject(oldObj);
            if (toVersion == nullptr) { //LCOV_EXCL_BR_LINE
                Heap::throwOOM();
                return;
            }
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
                LOG_MM_OBJ_EVENTS(DEBUG) << "REMARK ref @" << &refField
                                         << ": " << oldObj << " -> " << toVersion;
            }
            MarkToObject(oldObj, toVersion);
        } else {
            if (Heap::GetHeap().GetGCReason() != GC_REASON_YOUNG) {
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
        if (!RegionalHeap::IsNewObjectSinceMarking(object) && !collector_->MarkObject(object)) {
            collectStack_.Push(object);
        }
    }

    void MarkToObject(BaseObject *oldVersion, BaseObject *toVersion)
    {
        // We've checked oldVersion is in fromSpace, no need to check markingLine
        collector_->MarkObject(oldVersion);
        if (!collector_->MarkObject(toVersion)) {
            // oldVersion don't have valid type info, cannot push it
            collectStack_.Push(toVersion);
        }
    }

private:
    LocalCollectStack &collectStack_;
    ArkCollector *collector_;
};

class RemarkingAndPreforwardTask : public common_vm::Task {
public:
    RemarkingAndPreforwardTask(ArkCollector *collector, GlobalMarkStack &globalMarkStack, TaskPackMonitor &monitor,
                               std::function<Mutator*()>& next)
        : Task(0), collector_(collector), globalMarkStack_(globalMarkStack), monitor_(monitor), getNextMutator_(next)
    {}

    bool Run([[maybe_unused]] uint32_t threadIndex) override
    {
        ThreadLocal::SetThreadType(ThreadType::GC_THREAD);
        LocalCollectStack collectStack(&globalMarkStack_);
        RemarkAndPreforwardVisitor visitor(collectStack, collector_);
        Mutator *mutator = getNextMutator_();
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
    ArkCollector *collector_ {nullptr};
    GlobalMarkStack &globalMarkStack_;
    TaskPackMonitor &monitor_;
    std::function<Mutator*()> &getNextMutator_;
};

void ArkCollector::ParallelRemarkAndPreforward(GlobalMarkStack &globalMarkStack)
{
    std::vector<Mutator*> taskList;
    MutatorManager &mutatorManager = MutatorManager::Instance();
    mutatorManager.VisitAllMutators([&taskList](Mutator &mutator) {
        taskList.push_back(&mutator);
    });
    std::atomic<int> taskIter = 0;
    std::function<Mutator*()> getNextMutator = [&taskIter, &taskList]() -> Mutator* {
        uint32_t idx = static_cast<uint32_t>(taskIter.fetch_add(1U, std::memory_order_relaxed));
        if (idx < taskList.size()) {
            return taskList[idx];
        }
        return nullptr;
    };

    const uint32_t runningWorkers = std::min<uint32_t>(GetGCThreadCount(true), taskList.size());
    uint32_t parallelCount = runningWorkers + 1; // 1 ：DaemonThread
    TaskPackMonitor monitor(runningWorkers, runningWorkers);
    for (uint32_t i = 1; i < parallelCount; ++i) {
        GetThreadPool()->PostTask(std::make_unique<RemarkingAndPreforwardTask>(this, globalMarkStack, monitor,
                                                                               getNextMutator));
    }
    // Run in daemon thread.
    LocalCollectStack collectStack(&globalMarkStack);
    RemarkAndPreforwardVisitor visitor(collectStack, this);
    VisitGlobalRoots(visitor);
    Mutator *mutator = getNextMutator();
    while (mutator != nullptr) {
        mutator->VisitMutatorRoots(visitor);
        mutator = getNextMutator();
    }
    collectStack.Publish();
    monitor.WaitAllFinished();
}

void ArkCollector::RemarkAndPreforwardStaticRoots(GlobalMarkStack &globalMarkStack)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::RemarkAndPreforwardStaticRoots", "");
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;
    if (maxWorkers > 0) {
        ParallelRemarkAndPreforward(globalMarkStack);
    } else {
        LocalCollectStack collectStack(&globalMarkStack);
        RemarkAndPreforwardVisitor visitor(collectStack, this);
        VisitSTWRoots(visitor);
        collectStack.Publish();
    }
}

void ArkCollector::PreforwardConcurrentRoots()
{
    RefFieldVisitor visitor = [this](RefField<> &refField) {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        DLOG(FIX, "visit raw-ref @%p: %p", &refField, oldObj);
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            ASSERT_LOGF(toVersion != nullptr, "TryForwardObject failed");
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
                LOG_MM_OBJ_EVENTS(DEBUG) << "PREFORWARD concurrent ref @" << &refField
                                         << ": " << oldObj << " -> " << toVersion;
            }
        }
    };
    VisitConcurrentRoots(visitor);
}

void ArkCollector::PreforwardStaticWeakRoots()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PreforwardStaticRoots", "");

    WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
    VisitWeakRoots(weakVisitor);
    MutatorManager::Instance().VisitAllMutators([](Mutator& mutator) {
        // Request finalize callback in each vm-thread when gc finished.
        mutator.SetFinalizeRequest();
    });

    AllocationBuffer* allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY_CC(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();
    }
}

void ArkCollector::PreforwardConcurrencyModelRoots()
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

class EnumRootsBuffer {
public:
    EnumRootsBuffer();
    void UpdateBufferSize();
    CArrayList<BaseObject *> *GetBuffer() { return &buffer_; }

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
        LOG_COMMON(INFO) << "too many roots, allocated buffer too large: " << buffer_.size() << ", allocate "
                         << (static_cast<double>(buffer_.capacity()) / MB);
    }
}

template <ArkCollector::EnumRootsPolicy policy>
CArrayList<BaseObject *> ArkCollector::EnumRoots()
{
    STWParam stwParam{"wgc-enumroot"};
    EnumRootsBuffer buffer;
    CArrayList<common_vm::BaseObject *> *results = buffer.GetBuffer();
    common_vm::RefFieldVisitor visitor = [&results](RefField<>& field) { results->push_back(field.GetTargetObject()); };

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

void ArkCollector::MarkingHeap(const CArrayList<BaseObject *> &collectedRoots)
{
    COMMON_PHASE_TIMER("marking live objects");
    markedObjectCount_.store(0, std::memory_order_relaxed);
    TransitionToGCPhase(GCPhase::GC_PHASE_MARK, true);

    MarkingRoots(collectedRoots);
    ProcessFinalizers();
    ExemptFromSpace();
}

void ArkCollector::PostMarking()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PostMarking", "");
    COMMON_PHASE_TIMER("PostMarking");
    TransitionToGCPhase(GC_PHASE_POST_MARK, true);

    // clear satb buffer when gc finish tracing.
    SatbBuffer::Instance().ClearBuffer();

    WVerify::VerifyAfterMark(*this);
}

WeakRefFieldVisitor ArkCollector::GetWeakRefFieldVisitor()
{
    return [this](RefField<> &refField) -> bool {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (gcReason_ == GC_REASON_YOUNG) {
            if (RegionalHeap::IsYoungSpaceObject(oldObj) && !IsMarkedObject(oldObj) &&
                !RegionalHeap::IsNewObjectSinceMarking(oldObj)) {
                return false;
            }
        } else {
            if (!IsMarkedObject(oldObj) && !RegionalHeap::IsNewObjectSinceMarking(oldObj)) {
                return false;
            }
        }

        DLOG(FIX, "visit weak raw-ref @%p: %p", &refField, oldObj);
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK_CC(toVersion != nullptr);
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix weak raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
                LOG_MM_OBJ_EVENTS(DEBUG) << "PREFORWARD weak ref @" << &refField
                                         << ": " << oldObj << " -> " << toVersion;
            }
        }
        return true;
    };
}

RefFieldVisitor ArkCollector::GetPrefowardRefFieldVisitor()
{
    return [this](RefField<> &refField) -> void {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK_CC(toVersion != nullptr);
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
                LOG_MM_OBJ_EVENTS(DEBUG) << "PREFORWARD ref @" << &refField
                                         << ": " << oldObj << " -> " << toVersion;
            }
        }
    };
}

void ArkCollector::PreforwardFlip()
{
    auto remarkAndForwardGlobalRoot = [this]() {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PreforwardFlip[STW]", "");
        SetGCThreadQosPriority(common_vm::PriorityMode::STW);
        ASSERT_LOGF(GetThreadPool() != nullptr, "thread pool is null");
        TransitionToGCPhase(GCPhase::GC_PHASE_FINAL_MARK, true);
        Remark();
        PostMarking();
        reinterpret_cast<RegionalHeap&>(theAllocator_).PrepareForward();

        TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY, true);
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
        SetGCThreadQosPriority(common_vm::PriorityMode::FOREGROUND);

        VisitWeakGlobalRoots(weakVisitor, Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG);
    };
    FlipFunction forwardMutatorRoot = [this](Mutator &mutator) {
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
        mutator.VisitMutatorRoots(weakVisitor);
        RefFieldVisitor visitor = GetPrefowardRefFieldVisitor();
        VisitMutatorPreforwardRoot(visitor, mutator);
        // Request finalize callback in each vm-thread when gc finished.
        mutator.SetFinalizeRequest();
    };
    STWParam stwParam{"final-mark"};
    MutatorManager::Instance().FlipMutators(stwParam, remarkAndForwardGlobalRoot, &forwardMutatorRoot);
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
    AllocationBuffer* allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY_CC(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();
    }
}

void ArkCollector::Preforward()
{
    COMMON_PHASE_TIMER("Preforward");
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::Preforward[STW]", "");
    TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY, true);

    [[maybe_unused]] Taskpool *threadPool = GetThreadPool();
    ASSERT_LOGF(threadPool != nullptr, "thread pool is null");

    // copy and fix finalizer roots.
    // Only one root task, no need to post task.
    PreforwardStaticWeakRoots();
    RefFieldVisitor visitor = GetPrefowardRefFieldVisitor();
    VisitPreforwardRoots(visitor);
}

void ArkCollector::ConcurrentPreforward()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::ConcurrentPreforward", "");
    PreforwardConcurrentRoots();
}

void ArkCollector::PrepareFix()
{
    if (Heap::GetHeap().GetGCReason() == GCReason::GC_REASON_YOUNG) {
        // string table objects are always not in young space, skip it
        return;
    }

    COMMON_PHASE_TIMER("PrepareFix");

    // we cannot re-enter STW, check it first
    if (!MutatorManager::Instance().WorldStopped()) {
        STWParam prepareFixStwParam{"wgc-preparefix"};
        ScopedStopTheWorld stw(prepareFixStwParam);
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PrepareFix[STW]", "");

        GetGCStats().recordSTWTime(prepareFixStwParam.GetElapsedNs());
    }
}

void ArkCollector::ParallelFixHeap()
{
    auto& regionalHeap = reinterpret_cast<RegionalHeap&>(theAllocator_);
    auto taskList = regionalHeap.CollectFixTasks();
    std::atomic<int> taskIter = 0;
    std::function<FixHeapTask *()> getNextTask = [&taskIter, &taskList]() -> FixHeapTask* {
        uint32_t idx = static_cast<uint32_t>(taskIter.fetch_add(1U, std::memory_order_relaxed));
        if (idx < taskList.size()) {
            return &taskList[idx];
        }
        return nullptr;
    };

    const uint32_t runningWorkers = GetGCThreadCount(true) - 1;
    uint32_t parallelCount = runningWorkers + 1; // 1 ：DaemonThread
    FixHeapWorker::Result results[parallelCount];
    {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::FixHeap [Parallel]", "");
        // Fix heap
        TaskPackMonitor monitor(runningWorkers, runningWorkers);
        for (uint32_t i = 1; i < parallelCount; ++i) {
            GetThreadPool()->PostTask(std::make_unique<FixHeapWorker>(this, monitor, results[i], getNextTask));
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
            GetThreadPool()->PostTask(std::make_unique<PostFixHeapWorker>(results[i], monitor));
        }

        PostFixHeapWorker gcWorker(results[0], monitor);
        gcWorker.PostClearTask();
        PostFixHeapWorker::CollectEmptyRegions();
        monitor.WaitAllFinished();
    }
}

void ArkCollector::FixHeap()
{
    TransitionToGCPhase(GCPhase::GC_PHASE_FIX, true);
    COMMON_PHASE_TIMER("FixHeap");
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::FixHeap", "");
    ParallelFixHeap();

    WVerify::VerifyAfterFix(*this);
}

void ArkCollector::CollectGarbageWithXRef()
{
    const bool isNotYoungGC = gcReason_ != GCReason::GC_REASON_YOUNG;
#ifdef ENABLE_CMC_RB_DFX
    WVerify::DisableReadBarrierDFX(*this);
#endif
    STWParam stwParam{"stw-gc"};
    ScopedStopTheWorld stw(stwParam);
    RemoveXRefFromRoots();

    auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
    MarkingHeap(collectedRoots);
    TransitionToGCPhase(GCPhase::GC_PHASE_FINAL_MARK, true);
    Remark();
    SweepUnmarkedXRefs();
    PostMarking();

    AddXRefToRoots();
    Preforward();
    ConcurrentPreforward();
    // reclaim large objects should after preforward(may process weak ref) and
    // before fix heap(may clear live bit)
    if (isNotYoungGC) {
        CollectLargeGarbage();
    }

    CopyFromSpace();
    WVerify::VerifyAfterForward(*this);

    PrepareFix();
    FixHeap();
    if (isNotYoungGC) {
        CollectNonMovableGarbage();
    }

    TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);

    ClearAllGCInfo();
    CollectSmallSpace();
    UnmarkAllXRefs();

#if defined(ENABLE_CMC_RB_DFX)
    WVerify::EnableReadBarrierDFX(*this);
#endif
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
}

class GCTracer final {
public:
    GCTracer(GCReason reason, GCType type) : gcReason_(reason), gcType_(type)
    {
        Heap::GetHeap().GetCollector().NotifyGCListeners([reason, type] (GCListener *l) {
            l->OnGCStart(reason, type);
        });
    }

    ~GCTracer()
    {
        Heap::GetHeap().GetCollector().NotifyGCListeners([reason = gcReason_, type = gcType_] (GCListener *l) {
            l->OnGCFinish(reason, type);
        });
    }
private:
    GCReason gcReason_;
    GCType gcType_;
};

class ConcurrentEvacuationTask : public common_vm::Task {
public:
    ConcurrentEvacuationTask(uint32_t id, ArkCollector &tc, ParallelMarkingMonitor &monitor,
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
    ArkCollector &collector_;
    ParallelMarkingMonitor &monitor_;
    GlobalEvacuationStack &globalStack_;
};

class RemarkTask : public common_vm::Task {
public:
    RemarkTask(uint32_t id, ArkCollector &tc, ParallelMarkingMonitor &monitor,
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
    ArkCollector &collector_;
    ParallelMarkingMonitor &monitor_;
    GlobalEvacuationStack &globalStack_;
};

void ArkCollector::ProcessReferencesAfterCopy()
{
    BaseRuntime::GetInstance()->ForEachVM([](VMInterface *vm) { vm->ProcessReferencesAfterCopy(); });
}

void ArkCollector::DoGarbageCollection()
{
    GCTracer gcTracer(gcReason_, gcType_);
    const bool isYoungGC = gcReason_ == GCReason::GC_REASON_YOUNG;
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::DoGarbageCollection", "");
    if (gcReason_ == GCReason::GC_REASON_XREF) {
        CollectGarbageWithXRef();
        return;
    }
    if (gcMode_ == GCMode::STW) { // 2: stw-gc
#ifdef ENABLE_CMC_RB_DFX
        WVerify::DisableReadBarrierDFX(*this);
#endif
        STWParam stwParam{"stw-gc"};
    {
        ScopedStopTheWorld stw(stwParam);
        auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
        MarkingHeap(collectedRoots);
        TransitionToGCPhase(GCPhase::GC_PHASE_FINAL_MARK, true);
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
        WVerify::VerifyAfterForward(*this);

        PrepareFix();
        FixHeap();
        if (!isYoungGC) {
            CollectNonMovableGarbage();
        }

        TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);

        ClearAllGCInfo();
        CollectSmallSpace();

#if defined(ENABLE_CMC_RB_DFX)
        WVerify::EnableReadBarrierDFX(*this);
#endif
    }
        GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
        return;
    } else if (gcMode_ == GCMode::CONCURRENT_MARK) { // 1: concurrent-mark
        auto collectedRoots = EnumRoots<EnumRootsPolicy::STW_AND_NO_FLIP_MUTATOR>();
        MarkingHeap(collectedRoots);
        STWParam finalMarkStwParam{"final-mark"};
    {
        ScopedStopTheWorld stw(finalMarkStwParam, true, GCPhase::GC_PHASE_FINAL_MARK);
        Remark();
        PostMarking();
        reinterpret_cast<RegionalHeap&>(theAllocator_).PrepareForward();
        Preforward();
    }
        GetGCStats().recordSTWTime(finalMarkStwParam.GetElapsedNs());
        ConcurrentPreforward();
        // reclaim large objects should after preforward(may process weak ref) and
        // before fix heap(may clear live bit)
        if (!isYoungGC) {
            CollectLargeGarbage();
        }

        CopyFromSpace();
        WVerify::VerifyAfterForward(*this);

        PrepareFix();
        FixHeap();
        if (!isYoungGC) {
            CollectNonMovableGarbage();
        }

        TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);
        ClearAllGCInfo();
        CollectSmallSpace();
        return;
    }

    const auto avoidConcurrentMarking =
        BaseRuntime::GetInstance()->GetGCParam().singlePassCompactionEnabled && isYoungGC;
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
        WVerify::VerifyAfterForward(*this);

        PrepareFix();
        FixHeap();

        if (!isYoungGC) {
            CollectNonMovableGarbage();
        }
    } else {
        DoGarbageCollectionWithoutConcurrentMarking();
    }

    TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);
    ClearAllGCInfo();
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    space.DumpAllRegionSummary("Peak GC log");
    space.DumpAllRegionStats("region statistics when gc ends");
    CollectSmallSpace();
}

void ArkCollector::DoGarbageCollectionWithoutConcurrentMarking()
{
    CHECK_CC(Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG);
    GlobalEvacuationStack globalEvacuationStack;
    PreforwardNonHeapRoots<EnumRootsPolicy::STW_AND_FLIP_MUTATOR>(globalEvacuationStack);
    EnqueueRememberedSetRefs(globalEvacuationStack);
    ProcessEvacuationStack(globalEvacuationStack);
    RemarkYoungCollectionSpace(globalEvacuationStack);

    // We do not evacuate objects on remark pause to avoid too long STW
    CopyFromSpace();

    WVerify::VerifyAfterForward(*this);

    PrepareFix();
    FixHeap();
}

template <ArkCollector::EnumRootsPolicy policy>
void ArkCollector::PreforwardNonHeapRoots(GlobalEvacuationStack &globalStack)
{
    CArrayList<common_vm::BaseObject *> roots;

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
        if (!MarkObject(obj)) {
            EnqueueRefsToYoungCollectionSpace(obj, stack);
        }
    }
    stack.Publish();
}

template <void (&rootsVisitFunc)(const common_vm::RefFieldVisitor &)>
void ArkCollector::PreforwardNonHeapRootsImpl(CArrayList<BaseObject *> &forwardedRoots)
{
    auto &heap = static_cast<RegionalHeap &>(theAllocator_);
    heap.AssembleGarbageCandidates();
    heap.PrepareMarking();
    heap.PrepareForward();

    TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY, true);
    rootsVisitFunc([this, &forwardedRoots](RefField<> &root) { PreforwardNonHeapRoot(root, forwardedRoots); });

    TransitionToGCPhase(GCPhase::GC_PHASE_YOUNG_COPY, true);
}

void ArkCollector::PreforwardNonHeapRoot(RefField<> &root, CArrayList<BaseObject *> &forwardedRoots)
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

void ArkCollector::PreforwardNonHeapRootsFlip(CArrayList<BaseObject *> &forwardedRoots)
{
    const auto enumGlobalRoots = [this, &forwardedRoots]() {
        SetGCThreadQosPriority(common_vm::PriorityMode::STW);
        PreforwardNonHeapRootsImpl<VisitGlobalRoots>(forwardedRoots);
        SetGCThreadQosPriority(common_vm::PriorityMode::FOREGROUND);
    };

    std::mutex stackMutex;
    CArrayList<CArrayList<BaseObject *>> rootSet;  // allocate for each mutator
    FlipFunction enumMutatorRoot = [this, &rootSet, &stackMutex](Mutator &mutator) {
        CArrayList<BaseObject *> roots;
        mutator.VisitMutatorRoots([this, &roots](RefField<> &root) { PreforwardNonHeapRoot(root, roots); });
        std::lock_guard<std::mutex> lockGuard(stackMutex);
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
            CHECK_CC(!region->IsFromRegion());
            forwardedRoots.push_back(obj);
        }
    });
}

void ArkCollector::RemarkYoungCollectionSpace(GlobalEvacuationStack &globalStack)
{
    STWParam param {"remark-young-collection-space"};
    ScopedStopTheWorld stw(param);

    TransitionToGCPhase(GCPhase::GC_PHASE_FINAL_MARK, true);

    RemarkNonHeapRoots(globalStack);
    MarkSatbBuffer(globalStack);
    MarkEvacuationStack(globalStack);

    SatbBuffer::Instance().ClearBuffer();
    GetGCStats().recordSTWTime(param.GetElapsedNs());
}

void ArkCollector::RemarkNonHeapRoots(GlobalEvacuationStack &globalStack)
{
    LocalEvacuationStack stack(&globalStack);
    VisitRoots([this, &stack](RefField<> &root) { RemarkNonHeapRoot(root, stack); });
    stack.Publish();
}

void ArkCollector::EnqueueRememberedSetRefs(GlobalEvacuationStack &globalStack)
{
    LocalEvacuationStack stack(&globalStack);
    auto &space = static_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
    space.MarkRememberSet([this, &stack](BaseObject *object) { EnqueueRefsToYoungCollectionSpace(object, stack); });
    stack.Publish();
}

void ArkCollector::MarkSatbBuffer(GlobalEvacuationStack &globalStack)
{
    std::vector<BaseObject *> remarkStack;
    MutatorManager::Instance().VisitAllMutators([&remarkStack](Mutator &mutator) {
        const SatbBuffer::TreapNode* node = static_cast<const SatbBuffer::TreapNode*>(mutator.GetSatbBufferNode());
        if (node != nullptr) {
            const_cast<SatbBuffer::TreapNode *>(node)->GetObjects(remarkStack);
        }
    });
    SatbBuffer::Instance().GetRetiredObjects(remarkStack);

    LocalEvacuationStack localStack(&globalStack);
    while (!remarkStack.empty()) {
        BaseObject *obj = remarkStack.back();
        remarkStack.pop_back();
        CHECK_CC(Heap::IsHeapAddress(obj));
        auto *region = RegionDesc::GetAliveRegionDescAt(obj);
        CHECK_CC(!region->IsFromRegion());
        if (!MarkObject(obj)) {
            auto size = EnqueueRefsToYoungCollectionSpaceAndGetSize(obj, localStack);
            region->AddLiveByteCount(size);
        }
    }
    localStack.Publish();
}

void ArkCollector::ProcessEvacuationStack(GlobalEvacuationStack &globalEvacuationStack)
{
    const auto maxWorkers = GetGCThreadCount(true) - 1;
    if (maxWorkers > 0) {
        auto parallelCount = maxWorkers;
        ParallelMarkingMonitor monitor(parallelCount, parallelCount);

        auto *threadPool = GetThreadPool();
        for (uint32_t i = 0; i < parallelCount; ++i) {
            threadPool->PostTask(std::make_unique<ConcurrentEvacuationTask>(0, *this, monitor, globalEvacuationStack));
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
        // Fixme: this `ParallelLocalMarkStack` could be replaced with `SequentialLocalMarkStack`, and no need to
        // use monitor, but this need to add template param to `ProcessMarkStack`.
        // So for convenience just use a fake dummy parallel one.
        ParallelMarkingMonitor dummyMonitor(0, 0);
        ParallelLocalEvacuationStack stack(&globalEvacuationStack, &dummyMonitor);
        ProcessEvacuationStack(stack);
    }
}

void ArkCollector::ProcessEvacuationStack(ParallelLocalEvacuationStack &stack)
{
    // NOTE: it is performed concurrently with other GC threads and mutators
    std::vector<BaseObject *> remarkStack;
    auto fetchFromSatbBuffer = [this, &stack, &remarkStack]() {
        SatbBuffer::Instance().TryFetchOneRetiredNode(remarkStack);
        bool needProcess = false;
        while (!remarkStack.empty()) {
            BaseObject *obj = remarkStack.back();
            remarkStack.pop_back();
            CHECK_CC(Heap::IsHeapAddress(obj));
            if (!MarkObject(obj)) {
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

void ArkCollector::MarkEvacuationStack(GlobalEvacuationStack &globalEvacuationStack)
{
    const auto maxWorkers = GetGCThreadCount(true) - 1;
    if (maxWorkers > 0) {
        auto parallelCount = maxWorkers;
        ParallelMarkingMonitor monitor(parallelCount, parallelCount);

        auto *threadPool = GetThreadPool();
        for (uint32_t i = 0; i < parallelCount; ++i) {
            threadPool->PostTask(std::make_unique<RemarkTask>(0, *this, monitor, globalEvacuationStack));
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
        // Fixme: this `ParallelLocalMarkStack` could be replaced with `SequentialLocalMarkStack`, and no need to
        // use monitor, but this need to add template param to `ProcessMarkStack`.
        // So for convenience just use a fake dummy parallel one.
        ParallelMarkingMonitor dummyMonitor(0, 0);
        ParallelLocalEvacuationStack stack(&globalEvacuationStack, &dummyMonitor);
        MarkEvacuationStack(stack);
    }
}

void ArkCollector::MarkEvacuationStack(ParallelLocalEvacuationStack &stack)
{
    RefField<> *ref;
    while (stack.Pop(&ref)) {
        MarkRef(*ref, stack);
    }
}

template <typename Stack>
void ArkCollector::RemarkNonHeapRoot(RefField<> &ref, Stack &stack)
{
    auto *obj = ref.GetTargetObject();
    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    CHECK_CC(Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG);
    // Root was updated while concurrent copying phase so it cannot be in FromRegion
    CHECK_CC(!region->IsFromRegion());

    if (!InYoungCollectionSpace(region)) {
        return;
    }

    if (region->IsNewObjectSinceMarking(obj)) {
        return;
    }

    if (!MarkObject(obj)) {
        EnqueueRefsToYoungCollectionSpace(obj, stack);
    }
}

template <typename Stack>
void ArkCollector::ProcessRef(RefField<> &ref, Stack &stack)
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
        if (!MarkObject(fwd)) {
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

    if (!MarkObject(obj)) {
        EnqueueRefsToYoungCollectionSpace(obj, stack);
    }
}

template <typename Stack>
void ArkCollector::MarkRef(RefField<> &ref, Stack &stack)
{
    auto *obj = ref.GetTargetObject();
    auto *region = RegionDesc::GetAliveRegionDescAt(obj);

    CHECK_CC(Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG);

    if (!InYoungCollectionSpace(obj)) {
        return;
    }

    if (region->IsNewObjectSinceMarking(obj)) {
        return;
    }

    if (region->IsFromRegion() && obj->IsForwarded()) {
        obj = GetForwardingPointer(obj);
    }

    if (!MarkObject(obj)) {
        auto size = EnqueueRefsToYoungCollectionSpaceAndGetSize(obj, stack);
        region->AddLiveByteCount(size);
    }
}

template <typename Stack>
void ArkCollector::EnqueueRefsToYoungCollectionSpace(BaseObject *obj, Stack &stack)
{
    auto fieldHandler = [this, &stack](RefField<> &field) {
        if (InYoungCollectionSpace(field.GetTargetObject())) {
            stack.Push(&field);
        }
    };

    obj->ForEachRefField(fieldHandler, fieldHandler);
}

template <typename Stack>
size_t ArkCollector::EnqueueRefsToYoungCollectionSpaceAndGetSize(BaseObject *obj, Stack &stack)
{
    auto fieldHandler = [this, &stack](RefField<> &field) {
        if (InYoungCollectionSpace(field.GetTargetObject())) {
            stack.Push(&field);
        }
    };

    return obj->ForEachRefFieldAndGetSize(fieldHandler, fieldHandler);
}

bool ArkCollector::InYoungCollectionSpace(const RegionDesc::InlinedRegionMetaData *region) const
{
    return InYoungCollectionSpace(region->GetRegionType());
}

bool ArkCollector::InYoungCollectionSpace(const RegionDesc *region) const
{
    return InYoungCollectionSpace(region->GetRegionType());
}

bool ArkCollector::InYoungCollectionSpace(const BaseObject *obj) const
{
    return Heap::IsHeapAddress(obj) &&
           InYoungCollectionSpace(RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(obj));
}

bool ArkCollector::InYoungCollectionSpace(RegionDesc::RegionType type) const
{
    // During single pass evacuation in young GC objects might be concurrently evacuated by mutator. So we should
    // inspect references which point to them too.
    return RegionDesc::IsInYoungSpace(type) || RegionDesc::IsInToSpace(type);
}

CArrayList<CArrayList<BaseObject *>> ArkCollector::EnumRootsFlip(STWParam& param,
                                                                 const common_vm::RefFieldVisitor &visitor)
{
    const auto enumGlobalRoots = [this, &visitor]() {
        SetGCThreadQosPriority(common_vm::PriorityMode::STW);
        EnumRootsImpl<VisitGlobalRoots>(visitor);
        SetGCThreadQosPriority(common_vm::PriorityMode::FOREGROUND);
    };

    std::mutex stackMutex;
    CArrayList<CArrayList<BaseObject *>> rootSet;  // allcate for each mutator
    FlipFunction enumMutatorRoot = [&rootSet, &stackMutex](Mutator &mutator) {
        CArrayList<BaseObject *> roots;
        RefFieldVisitor localVisitor = [&roots](RefField<> &root) { roots.emplace_back(root.GetTargetObject()); };
        mutator.VisitMutatorRoots(localVisitor);
        std::lock_guard<std::mutex> lockGuard(stackMutex);
        rootSet.emplace_back(std::move(roots));
    };
    MutatorManager::Instance().FlipMutators(param, enumGlobalRoots, &enumMutatorRoot);
    return rootSet;
}

void ArkCollector::ProcessFinalizers()
{
    std::function<bool(BaseObject*)> finalizable = [this](BaseObject* obj) { return !IsMarkedObject(obj); };
    FinalizerProcessor& fp = collectorResources_.GetFinalizerProcessor();
    fp.EnqueueFinalizables(finalizable, snapshotFinalizerNum_);
    fp.Notify();
}

BaseObject* ArkCollector::ForwardObject(BaseObject* obj)
{
    BaseObject* to = TryForwardObject(obj);
    return (to != nullptr) ? to : obj;
}

BaseObject *ArkCollector::TryForwardObject(BaseObject *obj)
{
    return CopyObjectImpl(obj);
}

// ConcurrentGC
BaseObject* ArkCollector::CopyObjectImpl(BaseObject* obj)
{
    // reconsider phase difference between mutator and GC thread during transition.
    if (IsGcThread()) {
        CHECK_CC(GetGCPhase() == GCPhase::GC_PHASE_PRECOPY || GetGCPhase() == GCPhase::GC_PHASE_COPY ||
                 GetGCPhase() == GCPhase::GC_PHASE_YOUNG_COPY || GetGCPhase() == GCPhase::GC_PHASE_FIX ||
                 GetGCPhase() == GCPhase::GC_PHASE_FINAL_MARK);
    } else {
        auto phase = Mutator::GetMutator()->GetMutatorPhase();
        CHECK_CC(phase == GCPhase::GC_PHASE_PRECOPY || phase == GCPhase::GC_PHASE_COPY ||
                 phase == GCPhase::GC_PHASE_YOUNG_COPY || phase == GCPhase::GC_PHASE_FIX);
    }

    do {
        BaseStateWord oldWord = obj->GetBaseStateWord();

        // 1. object has already been forwarded
        if (obj->IsForwarded()) {
            auto toObj = GetForwardingPointer(obj);
            DLOG(COPY, "skip forwarded obj %p -> %p<%p>(%zu)", obj, toObj, toObj->GetTypeInfo(), toObj->GetSize());
            return toObj;
        }

        // ConcurrentGC
        // 2. object is being forwarded, spin until it is forwarded (or gets its own forwarded address)
        if (oldWord.IsForwarding()) {
            sched_yield();
            continue;
        }

        // 3. hope we can copy this object
        if (obj->TryLockExclusive(oldWord)) {
            return CopyObjectAfterExclusive(obj);
        }
    } while (true);
    LOG_COMMON(FATAL) << "forwardObject exit in wrong path";
    UNREACHABLE_CC();
    return nullptr;
}

BaseObject* ArkCollector::CopyObjectAfterExclusive(BaseObject* obj)
{
    size_t size = RegionalHeap::GetAllocSize(*obj);
    // 8: size of free object, but free object can not be copied.
    if (size == 8) {
        LOG_COMMON(FATAL) << "forward free obj: " << obj <<
            "is survived: " << (IsSurvivedObject(obj) ? "true" : "false");
    }
    BaseObject* toObj = reinterpret_cast<RegionalHeap&>(theAllocator_).RouteObject(obj, size);
    if (toObj == nullptr) {
        // ConcurrentGC
        obj->UnlockExclusive(BaseStateWord::ForwardState::NORMAL);
        return toObj;
    }
    DLOG(COPY, "copy obj %p<%p>(%zu) to %p", obj, obj->GetTypeInfo(), size, toObj);
    LOG_MM_OBJ_EVENTS(DEBUG) << "MOVE object " << obj << " -> " << toObj;
    CopyObject(*obj, *toObj, size);

    ASSERT_LOGF(IsToObject(toObj), "Copy object to invalid region");
    toObj->SetForwardState(BaseStateWord::ForwardState::NORMAL);

    std::atomic_thread_fence(std::memory_order_release);
    obj->SetSizeForwarded(size);
    // Avoid seeing the fwd pointer before observing the size modification
    // when calling GetSize during the CopyPhase.
    std::atomic_thread_fence(std::memory_order_release);
    obj->SetForwardingPointerAfterExclusive(toObj);
    return toObj;
}

void ArkCollector::ClearAllGCInfo()
{
    COMMON_PHASE_TIMER("ClearAllGCInfo");
    RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
    space.ClearAllGCInfo();
    reinterpret_cast<RegionalHeap&>(theAllocator_).ClearJitFortAwaitingMark();
}

void ArkCollector::CollectSmallSpace()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CollectSmallSpace", "");
    GCStats& stats = GetGCStats();
    RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
    {
        COMMON_PHASE_TIMER("CollectFromSpaceGarbage");
        stats.collectedBytes += stats.smallGarbageSize;
        space.CollectFromSpaceGarbage();
        space.HandlePromotion();
    }

    size_t candidateBytes = stats.fromSpaceSize + stats.nonMovableSpaceSize + stats.largeSpaceSize;
    stats.garbageRatio = (candidateBytes > 0) ? static_cast<float>(stats.collectedBytes) / candidateBytes : 0;

    stats.liveBytesAfterGC = space.GetAllocatedBytes();

    VLOG(DEBUG,
         "collect %zu B: small %zu - %zu B, non-movable %zu - %zu B, large %zu - %zu B. garbage ratio %.2f%%",
         stats.collectedBytes, stats.fromSpaceSize, stats.smallGarbageSize, stats.nonMovableSpaceSize,
         stats.nonMovableGarbageSize, stats.largeSpaceSize, stats.largeGarbageSize,
         stats.garbageRatio * 100); // The base of the percentage is 100
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CollectSmallSpace END", (
                    "collect:" + std::to_string(stats.collectedBytes) +
                    "B;small:" + std::to_string(stats.fromSpaceSize) +
                    "-" + std::to_string(stats.smallGarbageSize) +
                    "B;non-movable:" + std::to_string(stats.nonMovableSpaceSize) +
                    "-" + std::to_string(stats.nonMovableGarbageSize) +
                    "B;large:" + std::to_string(stats.largeSpaceSize) +
                    "-" + std::to_string(stats.largeGarbageSize) +
                    "B;garbage ratio:" + std::to_string(stats.garbageRatio)
                ).c_str());

    collectorResources_.GetFinalizerProcessor().NotifyToReclaimGarbage();
}

void ArkCollector::SetGCThreadQosPriority(common_vm::PriorityMode mode)
{
#ifdef ENABLE_QOS
    LOG_COMMON(DEBUG) << "SetGCThreadQosPriority gettid " << gettid();
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
            UNREACHABLE_CC();
            break;
    }
    common_vm::Taskpool::GetCurrentTaskpool()->SetThreadPriority(mode);
#endif
}

bool ArkCollector::ShouldIgnoreRequest(GCRequest& request) { return request.ShouldBeIgnored(); }
} // namespace common_vm
