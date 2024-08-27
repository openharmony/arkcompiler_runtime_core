/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/gen-gc/gen-gc.h"
#include "runtime/mem/gc/gc_root-inl.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/timing.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/gc/static/gc_marker_static-inl.h"
#include "runtime/mem/gc/dynamic/gc_marker_dynamic-inl.h"
#include "runtime/mem/gc/generational-gc-base-inl.h"
#include "runtime/mem/gc/gc_adaptive_stack_inl.h"

namespace ark::mem {

template <class LanguageConfig>
GenGC<LanguageConfig>::GenGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GenerationalGC<LanguageConfig>(objectAllocator, settings),
      marker_(this),
      isExplicitConcurrentGcEnabled_(settings.IsExplicitConcurrentGcEnabled())
{
    this->SetType(GCType::GEN_GC);
    this->SetTLABsSupported();
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::InitializeImpl()
{
    // GC saved the PandaVM instance, so we get allocator from the PandaVM.
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    this->CreateCardTable(allocator, PoolManager::GetMmapMemPool()->GetMinObjectAddress(),
                          PoolManager::GetMmapMemPool()->GetTotalObjectSize());
    auto barrierSet = allocator->New<GCGenBarrierSet>(allocator, this->GetCardTable(), CardTable::GetCardBits(),
                                                      CardTable::GetCardDirtyValue());
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);
    LOG_DEBUG_GC << "GenGC initialized";
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::ShouldRunTenuredGC(const GCTask &task)
{
    return task.reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE;
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::ShouldRunFullGC(const GCTask &task, bool haveEnoughSpaceForYoung) const
{
    return !haveEnoughSpaceForYoung || task.reason == GCTaskCause::OOM_CAUSE || this->IsExplicitFull(task) ||
           this->IsOnPygoteFork() || task.reason == GCTaskCause::STARTUP_COMPLETE_CAUSE;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::RunPhasesImpl(GCTask &task)
{
    LOG(DEBUG, GC) << "GenGC start";
    uint64_t footprintBefore = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    LOG_DEBUG_GC << "Footprint before GC: " << footprintBefore;
    if (this->IsLogDetailedGcInfoEnabled()) {
        this->footprintList_.clear();
        this->footprintList_.push_back({"Footprint before GC", footprintBefore});
    }
    uint64_t youngTotalTime = 0;
    {
        ScopedTiming t("Generational GC", *this->GetTiming());
        this->memStats_.Reset();
        // We trigger a full gc at first pygote fork
        if (ShouldRunFullGC(task, HaveEnoughSpaceToMove())) {
            GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats());
            marker_.BindBitmaps(true);  // clear pygote live bitmaps, we will rebuild it
            this->GetObjectGenAllocator()->InvalidateSpaceData();
            this->GetObjectGenAllocator()->UpdateSpaceData();
            RunFullGC(task);
        } else {
            {
                GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats());
                time::Timer timer(&youngTotalTime, true);
                marker_.BindBitmaps(false);
                this->GetObjectGenAllocator()->InvalidateSpaceData();
                this->GetObjectGenAllocator()->UpdateSpaceData();
                LOG_DEBUG_GC << "Young range: " << this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
                RunYoungGC(task);
                if (youngTotalTime > 0) {
                    this->GetStats()->AddTimeValue(youngTotalTime, TimeTypeStats::YOUNG_TOTAL_TIME);
                }
                uint64_t footprintYoung = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
                LOG_DEBUG_GC << "Footprint after young: " << footprintYoung;
                if (this->IsLogDetailedGcInfoEnabled()) {
                    this->footprintList_.push_back({"Footprint after young", footprintYoung});
                }
            }
            if (ShouldRunTenuredGC(task)) {
                marker_.BindBitmaps(true);  // clear pygote live bitmaps, we will rebuild it
                this->GetObjectGenAllocator()->InvalidateSpaceData();
                this->GetObjectGenAllocator()->UpdateSpaceData();
                RunTenuredGC(task);
            }
        }
    }
    uint64_t footprintAfter = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    LOG_DEBUG_GC << "Footprint after GC: " << footprintAfter;
    if (this->IsLogDetailedGcInfoEnabled()) {
        this->footprintList_.push_back({"Footprint after GC", footprintAfter});
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::PreStartupImp()
{
    GenerationalGC<LanguageConfig>::DisableTenuredGC();
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::InitGCBits(ark::ObjectHeader *objHeader)
{
    if (UNLIKELY(this->GetGCPhase() == GCPhase::GC_PHASE_SWEEP) &&
        (!this->GetObjectAllocator()->IsObjectInYoungSpace(objHeader))) {
        objHeader->SetMarkedForGC();
        // do unmark if out of sweep phase otherwise we may miss it in sweep
        if (UNLIKELY(this->GetGCPhase() != GCPhase::GC_PHASE_SWEEP)) {
            objHeader->SetUnMarkedForGC();
        }
    } else {
        objHeader->SetUnMarkedForGC();
    }
    LOG_DEBUG_GC << "Init gc bits for object: " << std::hex << objHeader << " bit: " << objHeader->IsMarkedForGC()
                 << ", is marked = " << IsMarked(objHeader);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::InitGCBitsForAllocationInTLAB(ark::ObjectHeader *objHeader)
{
    // Compiler will allocate objects in TLABs only in young space
    // Therefore, set unmarked for GC here.
    objHeader->SetUnMarkedForGC();
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::RunYoungGC(GCTask &task)
{
    GCScope<TRACE_TIMING> scopedTrace(__FUNCTION__, this);
    LOG_DEBUG_GC << "GenGC RunYoungGC start";
    uint64_t youngPauseTime;
    {
        NoAtomicGCMarkerScope scope(&this->marker_);
        time::Timer timer(&youngPauseTime, true);
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        MarkYoung(task);
        CollectYoungAndMove();
        this->GetCardTable()->ClearAll();
    }
    if (youngPauseTime > 0) {
        this->GetStats()->AddTimeValue(youngPauseTime, TimeTypeStats::YOUNG_PAUSED_TIME);
    }
    LOG_DEBUG_GC << "GenGC RunYoungGC end";
    task.collectionType = GCCollectionType::YOUNG;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkYoung(const GCTask &task)
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK_YOUNG);

    // Iterate over roots and add other roots
    GCMarkingStackType objectsStack(this);
    ASSERT(this->GetObjectAllocator()->GetYoungSpaceMemRanges().size() == 1);
    auto youngMr = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
    GCRootVisitor gcMarkYoung = [&objectsStack, &youngMr, this](const GCRoot &gcRoot) {
        // Skip non-young roots
        auto rootObjectPtr = gcRoot.GetObjectHeader();
        ASSERT(rootObjectPtr != nullptr);
        if (!youngMr.IsAddressInRange(ToUintPtr(rootObjectPtr))) {
            LOG_DEBUG_GC << "Skip root for young mark: " << std::hex << rootObjectPtr;
            return;
        }
        LOG(DEBUG, GC) << "root " << GetDebugInfoAboutObject(rootObjectPtr);
        if (this->MarkObjectIfNotMarked(rootObjectPtr)) {
            objectsStack.PushToStack(gcRoot.GetType(), rootObjectPtr);
            this->MarkYoungStack(&objectsStack);
        }
    };
    {
        GCScope<TRACE_TIMING> markingYoungRootsTrace("Marking roots young", this);
        this->VisitRoots(gcMarkYoung,
                         VisitGCRootFlags::ACCESS_ROOT_NONE | VisitGCRootFlags::ACCESS_ROOT_AOT_STRINGS_ONLY_YOUNG);
    }
    {
        ScopedTiming visitCardTableRootsTiming("VisitCardTableRoots", *this->GetTiming());
        LOG_DEBUG_GC << "START Marking tenured -> young roots";
        MemRangeChecker tenuredRangeChecker = [&youngMr](MemRange &memRange) -> bool {
            return !youngMr.IsIntersect(memRange);
        };
        ObjectChecker tenuredRangeYoungObjectChecker = [&youngMr](const ObjectHeader *objectHeader) -> bool {
            return youngMr.IsAddressInRange(ToUintPtr(objectHeader));
        };

        ObjectChecker fromObjectChecker = []([[maybe_unused]] const ObjectHeader *objectHeader) -> bool {
            return true;
        };

        this->VisitCardTableRoots(this->GetCardTable(), gcMarkYoung, tenuredRangeChecker,
                                  tenuredRangeYoungObjectChecker, fromObjectChecker,
                                  CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED);
    }
    // reference-processor in VisitCardTableRoots can add new objects to stack
    this->MarkYoungStack(&objectsStack);
    LOG_DEBUG_GC << "END Marking tenured -> young roots";
    auto refClearPred = [this]([[maybe_unused]] const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    this->GetPandaVm()->HandleReferences(task, refClearPred);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkYoungStack(GCMarkingStackType *stack)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    ASSERT(stack != nullptr);
    auto allocator = this->GetObjectAllocator();
    auto &youngRanges = allocator->GetYoungSpaceMemRanges();
    auto refPred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    while (!stack->Empty()) {
        auto *object = this->PopObjectFromStack(stack);
        ValidateObject(nullptr, object);
        auto *cls = object->template ClassAddr<BaseClass>();
        LOG_DEBUG_GC << "current object " << GetDebugInfoAboutObject(object);

        bool inRange = false;
        for (const auto &r : youngRanges) {
            if (r.IsAddressInRange(ToUintPtr(object))) {
                inRange = true;
                break;
            }
        }
        if (inRange) {
            marker_.MarkInstance(stack, object, cls, refPred);
        }
    }
}

template <class LanguageConfig>
HeapVerifierIntoGC<LanguageConfig> GenGC<LanguageConfig>::CollectVerificationInfo(const MemRange &youngMemRange)
{
    HeapVerifierIntoGC<LanguageConfig> youngVerifier(this->GetPandaVm()->GetHeapManager());
    if (this->GetSettings()->IntoGCHeapVerification() && !this->IsFullGC()) {
        ScopedTiming collectVerificationTiming("CollectVerificationInfo", *this->GetTiming());
        youngVerifier.CollectVerificationInfo(PandaVector<MemRange>(1U, youngMemRange));
    }
    return youngVerifier;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::VerifyCollectAndMove(HeapVerifierIntoGC<LanguageConfig> &&youngVerifier)
{
    if (this->GetSettings()->IntoGCHeapVerification() && !this->IsFullGC()) {
        ScopedTiming verificationTiming("Verification", *this->GetTiming());
        size_t failsCount = youngVerifier.VerifyAll();
        if (this->GetSettings()->FailOnHeapVerification() && failsCount > 0) {
            LOG(FATAL, GC) << "Heap was corrupted during GC, HeapVerifier found " << failsCount << " corruptions";
        }
    }
}

// NOLINTNEXTLINE(readability-function-size)
template <class LanguageConfig>
void GenGC<LanguageConfig>::CollectYoungAndMove()
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE);
    LOG_DEBUG_GC << "== GenGC CollectYoungAndMove start ==";
    // NOTE(dtrubenkov): add assert that we in STW
    PandaVector<ObjectHeader *> movedObjects;
    size_t prevMovedSize = this->GetPandaVm()->GetMemStats()->GetLastYoungObjectsMovedBytes();
    constexpr size_t MINIMAL_PREALLOC_MOVE_OBJ = 32U;
    // Adaptive preallocate buffer for moved_objects to avoid useless reallocations
    movedObjects.reserve(std::max(MINIMAL_PREALLOC_MOVE_OBJ, prevMovedSize / GetMinimalObjectSize()));
    size_t youngMoveSize = 0;
    size_t youngMoveCount = 0;
    size_t youngDeleteSize = 0;
    size_t youngDeleteCount = 0;

    auto *objectAllocator = this->GetObjectGenAllocator();
    ASSERT(this->GetObjectAllocator()->GetYoungSpaceMemRanges().size() == 1);
    auto youngMemRange = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
    HeapVerifierIntoGC<LanguageConfig> youngVerifier = CollectVerificationInfo(youngMemRange);

    std::function<void(ObjectHeader * objectHeader)> moveVisitor(
        [this, &objectAllocator, &movedObjects, &youngMoveSize, &youngMoveCount, &youngDeleteSize,
         &youngDeleteCount](ObjectHeader *objectHeader) -> void {
            size_t size = GetObjectSize(objectHeader);
            ASSERT(size <= Runtime::GetOptions().GetMaxTlabSize());
            // Use aligned size here, because we need to proceed MemStats correctly.
            size_t alignedSize = GetAlignedObjectSize(size);
            if (objectHeader->IsMarkedForGC<false>()) {
                auto dst = reinterpret_cast<ObjectHeader *>(objectAllocator->AllocateTenuredWithoutLocks(size));
                ASSERT(dst != nullptr);
                memcpy_s(dst, size, objectHeader, size);
                youngMoveSize += alignedSize;
                youngMoveCount++;
                LOG_DEBUG_OBJECT_EVENTS << "MOVE object " << objectHeader << " -> " << dst << ", size = " << size;
                movedObjects.push_back(dst);
                // set unmarked dst
                UnMarkObject(dst);
                this->SetForwardAddress(objectHeader, dst);
            } else {
                LOG_DEBUG_OBJECT_EVENTS << "DELETE OBJECT young: " << objectHeader;
                ++youngDeleteCount;
                youngDeleteSize += alignedSize;
            }
            // We will record all object in MemStats as SPACE_TYPE_OBJECT, so check it
            ASSERT(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(objectHeader) == SpaceType::SPACE_TYPE_OBJECT);
        });
    {
        ScopedTiming moveTiming("MoveAndSweep", *this->GetTiming());
        objectAllocator->IterateOverYoungObjects(moveVisitor);
    }
    this->memStats_.RecordYoungStats(youngMoveSize, youngMoveCount, youngDeleteSize, youngDeleteCount);
    UpdateRefsToMovedObjects(&movedObjects);
    this->VerifyCollectAndMove(std::move(youngVerifier));
    SweepYoungVmRefs();
    // Remove young
    objectAllocator->ResetYoungAllocator();

    this->UpdateMemStats(this->GetPandaVm()->GetMemStats()->GetFootprintHeap(), false);

    LOG_DEBUG_GC << "== GenGC CollectYoungAndMove end ==";
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::UpdateRefsToMovedObjects(PandaVector<ObjectHeader *> *movedObjects)
{
    GCScope<TRACE_TIMING> scope("UpdateRefsToMovedObjects", this);

    auto objAllocator = this->GetObjectAllocator();
    // Update references exyoung -> young
    LOG_DEBUG_GC << "process moved objects cnt = " << std::dec << movedObjects->size();
    LOG_DEBUG_GC << "=== Update exyoung -> young references. START. ===";
    for (auto obj : *movedObjects) {
        ObjectHelpers<LanguageConfig::LANG_TYPE>::UpdateRefsToMovedObjects(obj);
    }

    LOG_DEBUG_GC << "=== Update exyoung -> young references. END. ===";
    // update references tenured -> young
    LOG_DEBUG_GC << "=== Update tenured -> young references. START. ===";
    auto youngSpace = objAllocator->GetYoungSpaceMemRanges().at(0);
    auto updateRefsInObject(
        [](ObjectHeader *obj) { ObjectHelpers<LanguageConfig::LANG_TYPE>::UpdateRefsToMovedObjects(obj); });
    this->GetCardTable()->VisitMarked(
        [&updateRefsInObject, &objAllocator, &youngSpace](const MemRange &memRange) {
            if (!youngSpace.Contains(memRange)) {
                objAllocator->IterateOverObjectsInRange(memRange, updateRefsInObject);
            }
        },
        CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED);
    LOG_DEBUG_GC << "=== Update tenured -> young references. END. ===";
    this->CommonUpdateRefsToMovedObjects();
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::RunTenuredGC(GCTask &task)
{
    GCScope<TRACE_TIMING> scope(__FUNCTION__, this);
    LOG_DEBUG_GC << "GC tenured start";
    GCMarkingStackType objectsStack(this);
    {
        GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::COMMON_PAUSE);
        {
            ScopedTiming unMarkTiming("UnMark", *this->GetTiming());
            // Unmark all because no filter out tenured when mark young
            // NOTE(dtrubenk): remove this
            this->GetObjectAllocator()->IterateOverObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
        }
        InitialMark(&objectsStack);
    }
    this->ConcurrentMark(&objectsStack);
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    ReMark(&objectsStack, task);

    ASSERT(objectsStack.Empty());
    {
        ScopedTiming unMarkYoungTiming("UnMarkYoung", *this->GetTiming());
        this->GetObjectAllocator()->IterateOverYoungObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
    }
    Sweep<true>();
    LOG_DEBUG_GC << "GC tenured end";
    task.collectionType = GCCollectionType::TENURED;
}

// Full GC is ran on pause
template <class LanguageConfig>
void GenGC<LanguageConfig>::RunFullGC(GCTask &task)
{
    GCScope<TRACE_TIMING> fullGcScope(__FUNCTION__, this);
    LOG_DEBUG_GC << "Full GC start";
    this->SetFullGC(true);
    {
        ScopedTiming unMarkTiming("UnMark", *this->GetTiming());
        this->GetObjectAllocator()->IterateOverObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
    }
    FullMark(task);
    Sweep<false>();
    // Young GC
    if (LIKELY(HaveEnoughSpaceToMove())) {
        // We already marked objects above so just collect and move
        CollectYoungAndMove();
        this->GetCardTable()->ClearAll();
    }
    this->SetFullGC(false);
    LOG_DEBUG_GC << "Full GC end";
    task.collectionType = GCCollectionType::FULL;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkRoots(GCMarkingStackType *objectsStack, CardTableVisitFlag visitCardTableRoots,
                                      const ReferenceCheckPredicateT &refPred, VisitGCRootFlags flags)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    GCRootVisitor gcMarkRoots = [this, &objectsStack, &refPred](const GCRoot &gcRoot) {
        ObjectHeader *rootObject = gcRoot.GetObjectHeader();
        ObjectHeader *fromObject = gcRoot.GetFromObjectHeader();
        LOG_DEBUG_GC << "Handle root " << GetDebugInfoAboutObject(rootObject);
        if (UNLIKELY(fromObject != nullptr) &&
            this->IsReference(fromObject->NotAtomicClassAddr<BaseClass>(), fromObject, refPred)) {
            LOG_DEBUG_GC << "Add reference: " << GetDebugInfoAboutObject(fromObject) << " to stack";
            marker_.Mark(fromObject);
            this->ProcessReference(objectsStack, fromObject->NotAtomicClassAddr<BaseClass>(), fromObject,
                                   GC::EmptyReferenceProcessPredicate);
        } else {
            // we should always add this object to the stack, because we could mark this object in InitialMark, but
            // write to some fields in ConcurrentMark - need to iterate over all fields again, MarkObjectIfNotMarked
            // can't be used here
            marker_.Mark(rootObject);
            objectsStack->PushToStack(gcRoot.GetType(), rootObject);
        }
    };
    this->VisitRoots(gcMarkRoots, flags);
    if (visitCardTableRoots == CardTableVisitFlag::VISIT_ENABLED) {
        auto allocator = this->GetObjectAllocator();
        ASSERT(allocator->GetYoungSpaceMemRanges().size() == 1);
        MemRange youngMr = allocator->GetYoungSpaceMemRanges().at(0);
        MemRangeChecker youngRangeChecker = []([[maybe_unused]] MemRange &memRange) -> bool { return true; };
        ObjectChecker youngRangeTenuredObjectChecker = [&youngMr](const ObjectHeader *objectHeader) -> bool {
            return !youngMr.IsAddressInRange(ToUintPtr(objectHeader));
        };
        ObjectChecker fromObjectChecker = [&youngMr, this](const ObjectHeader *objectHeader) -> bool {
            // Don't visit objects which are in tenured and not marked.
            return youngMr.IsAddressInRange(ToUintPtr(objectHeader)) || IsMarked(objectHeader);
        };
        this->VisitCardTableRoots(this->GetCardTable(), gcMarkRoots, youngRangeChecker, youngRangeTenuredObjectChecker,
                                  fromObjectChecker, CardTableProcessedFlag::VISIT_MARKED);
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::InitialMark(GCMarkingStackType *objectsStack)
{
    GCScope<TRACE_TIMING_PHASE> gcScope(__FUNCTION__, this, GCPhase::GC_PHASE_INITIAL_MARK);
    {
        NoAtomicGCMarkerScope scope(&this->marker_);
        auto refPred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
        MarkRoots(objectsStack, CardTableVisitFlag::VISIT_DISABLED, refPred,
                  VisitGCRootFlags::ACCESS_ROOT_NONE | VisitGCRootFlags::START_RECORDING_NEW_ROOT);
    }
}

template <class LanguageConfig>
NO_THREAD_SAFETY_ANALYSIS void GenGC<LanguageConfig>::ConcurrentMark(GCMarkingStackType *objectsStack)
{
    GCScope<TRACE_TIMING_PHASE> scopedFunc(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    ConcurrentScope concurrentScope(this);
    auto *objectAllocator = this->GetObjectAllocator();
    this->MarkImpl(
        &marker_, objectsStack, CardTableVisitFlag::VISIT_ENABLED,
        // Process 'weak' references as regular object on concurrent phase to avoid
        // concurrent access to referent
        []([[maybe_unused]] const ObjectHeader *obj) { return false; },
        // non-young mem range checker
        [objectAllocator](MemRange &memRange) { return !objectAllocator->IsIntersectedWithYoung(memRange); });
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::ReMark(GCMarkingStackType *objectsStack, const GCTask &task)
{
    GCScope<TRACE_TIMING_PHASE> gcScope(__FUNCTION__, this, GCPhase::GC_PHASE_REMARK);
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::REMARK_PAUSE);

    // NOTE(dtrubenkov): consider iterational concurrent marking of card table
    {
        NoAtomicGCMarkerScope scope(&this->marker_);
        auto refPred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
        MarkRoots(objectsStack, CardTableVisitFlag::VISIT_ENABLED, refPred,
                  VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW | VisitGCRootFlags::END_RECORDING_NEW_ROOT);
        this->MarkStack(&marker_, objectsStack, GC::EmptyMarkPreprocess, refPred);
        {
            ScopedTiming t1("VisitInternalStringTable", *this->GetTiming());
            this->GetPandaVm()->VisitStringTable(
                [this, &objectsStack](ObjectHeader *str) {
                    if (this->MarkObjectIfNotMarked(str)) {
                        ASSERT(str != nullptr);
                        objectsStack->PushToStack(RootType::STRING_TABLE, str);
                    }
                },
                VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW | VisitGCRootFlags::END_RECORDING_NEW_ROOT);
            this->MarkStack(&marker_, objectsStack, GC::EmptyMarkPreprocess, refPred);
        }
        // ConcurrentMark doesn't visit young objects - so we can't clear references which are in young-space because we
        // don't know which objects are marked. We will process them on young GC separately later, here we process
        // only refs in tenured-space
        this->GetPandaVm()->HandleReferences(task, refPred);
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::FullMark(const GCTask &task)
{
    GCScope<TRACE_TIMING_PHASE> fullMarkScope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    NoAtomicGCMarkerScope markerScope(&this->marker_);

    GCMarkingStackType objectsStack(this);
    VisitGCRootFlags flags = VisitGCRootFlags::ACCESS_ROOT_ALL;
    auto refPred = GC::EmptyReferenceProcessPredicate;
    // Mark all reachable objects
    MarkRoots(&objectsStack, CardTableVisitFlag::VISIT_DISABLED, refPred, flags);
    this->GetPandaVm()->VisitStringTable(
        [this, &objectsStack](ObjectHeader *str) {
            if (this->MarkObjectIfNotMarked(str)) {
                ASSERT(str != nullptr);
                objectsStack.PushToStack(RootType::STRING_TABLE, str);
            }
        },
        flags);
    this->MarkStack(&marker_, &objectsStack, GC::EmptyMarkPreprocess, refPred);
    auto refClearPred = []([[maybe_unused]] const ObjectHeader *obj) { return true; };
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    this->GetPandaVm()->HandleReferences(task, refClearPred);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkReferences(GCMarkingStackType *references, GCPhase gcPhase)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    LOG_DEBUG_GC << "Start marking " << references->Size() << " references";
    auto refPred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    if (gcPhase == GCPhase::GC_PHASE_MARK_YOUNG) {
        this->MarkYoungStack(references);
    } else if (gcPhase == GCPhase::GC_PHASE_INITIAL_MARK || gcPhase == GCPhase::GC_PHASE_MARK ||
               gcPhase == GCPhase::GC_PHASE_REMARK) {
        this->MarkStack(&marker_, references, GC::EmptyMarkPreprocess, refPred);
    } else {
        UNREACHABLE();
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkObject(ObjectHeader *object)
{
    marker_.Mark(object);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::UnMarkObject(ObjectHeader *objectHeader)
{
    LOG_DEBUG_GC << "Set unmark for GC " << GetDebugInfoAboutObject(objectHeader);
    this->marker_.UnMark(objectHeader);
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::IsMarked(const ObjectHeader *object) const
{
    return this->marker_.IsMarked(object);
}

// NO_THREAD_SAFETY_ANALYSIS because clang thread safety analysis
template <class LanguageConfig>
template <bool CONCURRENT>
NO_THREAD_SAFETY_ANALYSIS void GenGC<LanguageConfig>::Sweep()
{
    GCScope<TRACE_TIMING> gcScope(__FUNCTION__, this);
    ConcurrentScope concurrentScope(this, false);
    size_t freedObjectSize = 0U;
    size_t freedObjectCount = 0U;

    // NB! can't move block out of brace, we need to make sure GC_PHASE_SWEEP cleared
    {
        GCScopedPhase scopedPhase(this, GCPhase::GC_PHASE_SWEEP);
        // NOTE(dtrubenkov): make concurrent
        ASSERT(this->GetObjectAllocator()->GetYoungSpaceMemRanges().size() == 1);
        // new strings may be created in young space during tenured gc, we shouldn't collect them
        auto youngMemRange = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
        this->GetPandaVm()->SweepVmRefs([this, &youngMemRange](ObjectHeader *object) {
            if (youngMemRange.IsAddressInRange(ToUintPtr(object))) {
                return ObjectStatus::ALIVE_OBJECT;
            }
            return this->marker_.MarkChecker(object);
        });
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (CONCURRENT) {
            concurrentScope.Start();  // enable concurrent after GC_PHASE_SWEEP has been set
        }

        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (CONCURRENT && LanguageConfig::MT_MODE != MT_MODE_SINGLE) {
            // Run monitor deflation again, to avoid object was reclaimed before monitor deflate.
            auto youngMr = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
            this->GetPandaVm()->GetMonitorPool()->DeflateMonitorsWithCallBack([&youngMr, this](Monitor *monitor) {
                ObjectHeader *objectHeader = monitor->GetObject();
                return (!IsMarked(objectHeader)) && (!youngMr.IsAddressInRange(ToUintPtr(objectHeader)));
            });
        }

        this->GetObjectAllocator()->Collect(
            [this, &freedObjectSize, &freedObjectCount](ObjectHeader *object) {
                auto status = this->marker_.MarkChecker(object);
                if (status == ObjectStatus::DEAD_OBJECT) {
                    LOG_DEBUG_OBJECT_EVENTS << "DELETE OBJECT tenured: " << object;
                    freedObjectSize += GetAlignedObjectSize(GetObjectSize(object));
                    freedObjectCount++;
                }
                return status;
            },
            GCCollectMode::GC_ALL);
        this->GetObjectAllocator()->VisitAndRemoveFreePools([this](void *mem, size_t size) {
            this->GetCardTable()->ClearCardRange(ToUintPtr(mem), ToUintPtr(mem) + size);
            PoolManager::GetMmapMemPool()->FreePool(mem, size);
        });
    }

    this->memStats_.RecordSizeFreedTenured(freedObjectSize);
    this->memStats_.RecordCountFreedTenured(freedObjectCount);

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONCURRENT) {
        // In concurrent sweep phase, the new created objects may being marked in InitGCBits,
        // so we need wait for that done, then we can safely unmark objects concurrent with mutator.
        ASSERT(this->GetGCPhase() != GCPhase::GC_PHASE_SWEEP);  // Make sure we are out of sweep scope
        this->GetObjectAllocator()->IterateOverTenuredObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::SweepYoungVmRefs()
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_SWEEP);
    // new strings may be created in young space during tenured gc, we shouldn't collect them
    // Sweep string table here to avoid dangling references
    ASSERT(this->GetObjectAllocator()->GetYoungSpaceMemRanges().size() == 1);
    // new strings may be created in young space during tenured gc, we shouldn't collect them
    auto youngMemRange = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
    this->GetPandaVm()->SweepVmRefs([&youngMemRange](ObjectHeader *objectHeader) {
        if (youngMemRange.IsAddressInRange(ToUintPtr(objectHeader))) {
            return ObjectStatus::DEAD_OBJECT;
        }
        return ObjectStatus::ALIVE_OBJECT;
    });
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::InGCSweepRange(const ObjectHeader *obj) const
{
    bool inYoungSpace = this->GetObjectAllocator()->IsObjectInYoungSpace(obj);
    auto phase = this->GetGCPhase();
    // Do young GC and the object is in the young space
    if (phase == GCPhase::GC_PHASE_MARK_YOUNG && inYoungSpace) {
        return true;
    }

    // Do tenured GC and the object is in the tenured space
    if (phase != GCPhase::GC_PHASE_MARK_YOUNG && !inYoungSpace) {
        return true;
    }

    return this->IsFullGC();
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::IsPostponeGCSupported() const
{
    // Gen GC doesn't support GC postponing because
    // we have to move young space objects
    return false;
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::HaveEnoughSpaceToMove() const
{
    // hack for pools because we have 2 type of pools in tenures space, in bad cases objects can be moved to different
    // spaces. And move 4M objects in bump-allocator to other allocator, may need more than 4M space in other allocator
    // - so we need 3 empty pools.
    // NOTE(xucheng) : remove the checker when we can do part young collection.
    // The min num that can guarantee that we move all objects in young space.
    constexpr size_t POOLS_NUM = 3;
    return this->GetObjectAllocator()->HaveEnoughPoolsInObjectSpace(POOLS_NUM);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(GenGC);

}  // namespace ark::mem
