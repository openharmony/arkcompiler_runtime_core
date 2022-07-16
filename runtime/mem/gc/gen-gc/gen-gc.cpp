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

#include "runtime/mem/gc/gen-gc/gen-gc.h"
#include "runtime/include/hclass.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/mem/gc/gc_root-inl.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/mem/gc/gc_workers_thread_pool.h"
#include "runtime/timing.h"
#include "runtime/include/exceptions.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/gc/static/gc_marker_static-inl.h"
#include "runtime/mem/gc/dynamic/gc_marker_dynamic-inl.h"
#include "runtime/mem/gc/generational-gc-base-inl.h"

namespace panda::mem {

template <class LanguageConfig>
GenGC<LanguageConfig>::GenGC(ObjectAllocatorBase *object_allocator, const GCSettings &settings)
    : GenerationalGC<LanguageConfig>(object_allocator, settings), marker_(this)
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
    auto barrier_set =
        allocator->New<GCGenBarrierSet>(allocator, PoolManager::GetMmapMemPool()->GetAddressOfMinObjectAddress(),
                                        this->GetCardTable(), CardTable::GetCardBits(), CardTable::GetCardDirtyValue());
    ASSERT(barrier_set != nullptr);
    this->SetGCBarrierSet(barrier_set);
    LOG_DEBUG_GC << "GenGC initialized";
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::ShouldRunTenuredGC(const GCTask &task)
{
    return task.reason_ == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE;
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::ShouldRunFullGC(const GCTask &task, bool have_enough_space_for_young) const
{
    return !have_enough_space_for_young || task.reason_ == GCTaskCause::OOM_CAUSE ||
           task.reason_ == GCTaskCause::EXPLICIT_CAUSE || this->IsOnPygoteFork() ||
           task.reason_ == GCTaskCause::STARTUP_COMPLETE_CAUSE;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::RunPhasesImpl(GCTask &task)
{
    LOG(DEBUG, GC) << "GenGC start";
    uint64_t footprint_before = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    LOG_DEBUG_GC << "Footprint before GC: " << footprint_before;
    if (this->IsLogDetailedGcInfoEnabled()) {
        this->footprint_list_.clear();
        this->footprint_list_.push_back({"Footprint before GC", footprint_before});
    }
    uint64_t young_total_time = 0;
    this->GetTiming()->Reset();
    {
        ScopedTiming t("Generational GC", *this->GetTiming());
        this->mem_stats_.Reset();
        // We trigger a full gc at first pygote fork
        if (ShouldRunFullGC(task, HaveEnoughSpaceToMove())) {
            marker_.BindBitmaps(true);  // clear pygote live bitmaps, we will rebuild it
            this->GetObjectGenAllocator()->InvalidateSpaceData();
            this->GetObjectGenAllocator()->UpdateSpaceData();
            RunFullGC(task);
        } else {
            {
                GCScopedPauseStats scoped_pause_stats(this->GetPandaVm()->GetGCStats());
                time::Timer timer(&young_total_time, true);
                this->GetPandaVm()->GetMemStats()->RecordGCPauseStart();
                marker_.BindBitmaps(false);
                this->GetObjectGenAllocator()->InvalidateSpaceData();
                this->GetObjectGenAllocator()->UpdateSpaceData();
                LOG_DEBUG_GC << "Young range: " << this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
                RunYoungGC(task);
                this->GetPandaVm()->GetMemStats()->RecordGCPhaseEnd();
                if (young_total_time > 0) {
                    this->GetStats()->AddTimeValue(young_total_time, TimeTypeStats::YOUNG_TOTAL_TIME);
                }
                uint64_t footprint_young = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
                LOG_DEBUG_GC << "Footprint after young: " << footprint_young;
                if (this->IsLogDetailedGcInfoEnabled()) {
                    this->footprint_list_.push_back({"Footprint after young", footprint_young});
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
    uint64_t footprint_after = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    LOG_DEBUG_GC << "Footprint after GC: " << footprint_after;
    if (this->IsLogDetailedGcInfoEnabled()) {
        this->footprint_list_.push_back({"Footprint after GC", footprint_after});
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::PreStartupImp()
{
    GenerationalGC<LanguageConfig>::DisableTenuredGC();
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::InitGCBits(panda::ObjectHeader *obj_header)
{
    if (UNLIKELY(this->GetGCPhase() == GCPhase::GC_PHASE_SWEEP) &&
        (!this->GetObjectAllocator()->IsAddressInYoungSpace(ToUintPtr(obj_header)))) {
        obj_header->SetMarkedForGC();
        // do unmark if out of sweep phase otherwise we may miss it in sweep
        if (UNLIKELY(this->GetGCPhase() != GCPhase::GC_PHASE_SWEEP)) {
            obj_header->SetUnMarkedForGC();
        }
    } else {
        obj_header->SetUnMarkedForGC();
    }
    LOG_DEBUG_GC << "Init gc bits for object: " << std::hex << obj_header << " bit: " << obj_header->IsMarkedForGC()
                 << ", is marked = " << IsMarked(obj_header);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::InitGCBitsForAllocationInTLAB(panda::ObjectHeader *obj_header)
{
    // Compiler will allocate objects in TLABs only in young space
    // Therefore, set unmarked for GC here.
    obj_header->SetUnMarkedForGC();
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::Trigger()
{
    // Check current heap size.
    // Collect Young gen.
    // If threshold for tenured gen - collect tenured gen.
    auto task = MakePandaUnique<GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, time::GetCurrentTimeInNanos());
    // TODO(dtrubenkov): change for concurrent mode
    this->AddGCTask(true, std::move(task), true);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::RunYoungGC(GCTask &task)
{
    GCScope<TRACE_TIMING> scoped_trace(__FUNCTION__, this);
    LOG_DEBUG_GC << "GenGC RunYoungGC start";
    uint64_t young_pause_time;
    {
        NoAtomicGCMarkerScope scope(&this->marker_);
        // TODO: Measure only those that are on pause
        time::Timer timer(&young_pause_time, true);
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        MarkYoung(task);
        CollectYoungAndMove();
        this->GetCardTable()->ClearAll();
    }
    if (young_pause_time > 0) {
        this->GetStats()->AddTimeValue(young_pause_time, TimeTypeStats::YOUNG_PAUSED_TIME);
    }
    LOG_DEBUG_GC << "GenGC RunYoungGC end";
    task.collection_type_ = GCCollectionType::YOUNG;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkYoung(const GCTask &task)
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK_YOUNG);

    // Iterate over roots and add other roots
    GCMarkingStackType objects_stack(this);
    ASSERT(this->GetObjectAllocator()->GetYoungSpaceMemRanges().size() == 1);
    auto young_mr = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
    GCRootVisitor gc_mark_young = [&objects_stack, &young_mr, this](const GCRoot &gc_root) {
        // Skip non-young roots
        auto root_object_ptr = gc_root.GetObjectHeader();
        ASSERT(root_object_ptr != nullptr);
        if (!young_mr.IsAddressInRange(ToUintPtr(root_object_ptr))) {
            LOG_DEBUG_GC << "Skip root for young mark: " << std::hex << root_object_ptr;
            return;
        }
        LOG(DEBUG, GC) << "root " << GetDebugInfoAboutObject(root_object_ptr);
        if (this->MarkObjectIfNotMarked(root_object_ptr)) {
            objects_stack.PushToStack(gc_root.GetType(), root_object_ptr);
            this->MarkYoungStack(&objects_stack);
        }
    };
    {
        GCScope<TRACE_TIMING> marking_young_roots_trace("Marking roots young", this);
        this->VisitRoots(gc_mark_young,
                         VisitGCRootFlags::ACCESS_ROOT_NONE | VisitGCRootFlags::ACCESS_ROOT_AOT_STRINGS_ONLY_YOUNG);
    }
    {
        ScopedTiming visit_card_table_roots_timing("VisitCardTableRoots", *this->GetTiming());
        LOG_DEBUG_GC << "START Marking tenured -> young roots";
        MemRangeChecker tenured_range_checker = [&young_mr](MemRange &mem_range) -> bool {
            return !young_mr.IsIntersect(mem_range);
        };
        ObjectChecker tenured_range_young_object_checker = [&young_mr](const ObjectHeader *object_header) -> bool {
            return young_mr.IsAddressInRange(ToUintPtr(object_header));
        };

        ObjectChecker from_object_checker = []([[maybe_unused]] const ObjectHeader *object_header) -> bool {
            return true;
        };

        this->VisitCardTableRoots(this->GetCardTable(), gc_mark_young, tenured_range_checker,
                                  tenured_range_young_object_checker, from_object_checker,
                                  CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED);
    }
    // reference-processor in VisitCardTableRoots can add new objects to stack
    this->MarkYoungStack(&objects_stack);
    LOG_DEBUG_GC << "END Marking tenured -> young roots";
    auto ref_clear_pred = [this]([[maybe_unused]] const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    this->GetPandaVm()->HandleReferences(task, ref_clear_pred);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkYoungStack(GCMarkingStackType *stack)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    ASSERT(stack != nullptr);
    auto allocator = this->GetObjectAllocator();
    auto &young_ranges = allocator->GetYoungSpaceMemRanges();
    auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    while (!stack->Empty()) {
        auto *object = this->PopObjectFromStack(stack);
        ValidateObject(nullptr, object);
        auto *cls = object->template ClassAddr<BaseClass>();
        LOG_DEBUG_GC << "current object " << GetDebugInfoAboutObject(object);

        bool in_range = false;
        for (const auto &r : young_ranges) {
            if (r.IsAddressInRange(ToUintPtr(object))) {
                in_range = true;
                break;
            }
        }
        if (in_range) {
            marker_.MarkInstance(stack, ref_pred, object, cls);
        }
    }
}

template <class LanguageConfig>
HeapVerifierIntoGC<LanguageConfig> GenGC<LanguageConfig>::CollectVerificationInfo(const MemRange &young_mem_range)
{
    HeapVerifierIntoGC<LanguageConfig> young_verifier(this->GetPandaVm()->GetHeapManager());
    if (this->GetSettings()->IntoGCHeapVerification() && !this->IsFullGC()) {
        ScopedTiming collect_verification_timing("CollectVerificationInfo", *this->GetTiming());
        young_verifier.CollectVerificationInfo(PandaVector<MemRange>(1U, young_mem_range));
    }
    return young_verifier;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::VerifyCollectAndMove(HeapVerifierIntoGC<LanguageConfig> &&young_verifier)
{
    if (this->GetSettings()->IntoGCHeapVerification() && !this->IsFullGC()) {
        ScopedTiming verification_timing("Verification", *this->GetTiming());
        size_t fails_count = young_verifier.VerifyAll();
        if (this->GetSettings()->FailOnHeapVerification() && fails_count > 0) {
            LOG(FATAL, GC) << "Heap was corrupted during GC, HeapVerifier found " << fails_count << " corruptions";
        }
    }
}

// NOLINTNEXTLINE(readability-function-size)
template <class LanguageConfig>
void GenGC<LanguageConfig>::CollectYoungAndMove()
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE);
    LOG_DEBUG_GC << "== GenGC CollectYoungAndMove start ==";
    // TODO(dtrubenkov): add assert that we in STW
    PandaVector<ObjectHeader *> moved_objects;
    size_t prev_moved_size = this->GetPandaVm()->GetMemStats()->GetLastYoungObjectsMovedBytes();
    constexpr size_t MINIMAL_PREALLOC_MOVE_OBJ = 32U;
    // Adaptive preallocate buffer for moved_objects to avoid useless reallocations
    moved_objects.reserve(std::max(MINIMAL_PREALLOC_MOVE_OBJ, prev_moved_size / GetMinimalObjectSize()));
    size_t young_move_size = 0;
    size_t young_move_count = 0;
    size_t young_delete_size = 0;
    size_t young_delete_count = 0;
    size_t bytes_in_heap_before_move = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();

    auto *object_allocator = this->GetObjectGenAllocator();
    ASSERT(this->GetObjectAllocator()->GetYoungSpaceMemRanges().size() == 1);
    auto young_mem_range = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
    HeapVerifierIntoGC<LanguageConfig> young_verifier = CollectVerificationInfo(young_mem_range);

    std::function<void(ObjectHeader * object_header)> move_visitor(
        [this, &object_allocator, &moved_objects, &young_move_size, &young_move_count, &young_delete_size,
         &young_delete_count](ObjectHeader *object_header) -> void {
            size_t size = GetObjectSize(object_header);
            ASSERT(size <= ObjectAllocatorGen<>::GetYoungAllocMaxSize());
            // Use aligned size here, because we need to proceed MemStats correctly.
            size_t aligned_size = GetAlignedObjectSize(size);
            if (object_header->IsMarkedForGC<false>()) {
                auto dst = reinterpret_cast<ObjectHeader *>(object_allocator->AllocateTenuredWithoutLocks(size));
                ASSERT(dst != nullptr);
                memcpy_s(dst, size, object_header, size);
                young_move_size += aligned_size;
                young_move_count++;
                LOG_DEBUG_OBJECT_EVENTS << "MOVE object " << object_header << " -> " << dst << ", size = " << size;
                moved_objects.push_back(dst);
                // set unmarked dst
                UnMarkObject(dst);
                this->SetForwardAddress(object_header, dst);
            } else {
                LOG_DEBUG_OBJECT_EVENTS << "DELETE OBJECT young: " << object_header;
                ++young_delete_count;
                young_delete_size += aligned_size;
            }
            // We will record all object in MemStats as SPACE_TYPE_OBJECT, so check it
            ASSERT(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(object_header) == SpaceType::SPACE_TYPE_OBJECT);
        });
    {
        ScopedTiming move_timing("MoveAndSweep", *this->GetTiming());
        object_allocator->IterateOverYoungObjects(move_visitor);
    }
    this->mem_stats_.RecordSizeMovedYoung(young_move_size);
    this->mem_stats_.RecordCountMovedYoung(young_move_count);
    this->mem_stats_.RecordSizeFreedYoung(young_delete_size);
    this->mem_stats_.RecordCountFreedYoung(young_delete_count);
    UpdateRefsToMovedObjects(&moved_objects);
    this->VerifyCollectAndMove(std::move(young_verifier));
    // Sweep string table here to avoid dangling references
    this->SweepStringTableYoung([&young_mem_range](ObjectHeader *object_header) {
        return young_mem_range.IsAddressInRange(ToUintPtr(object_header));
    });
    // Remove young
    object_allocator->ResetYoungAllocator();

    this->UpdateMemStats(bytes_in_heap_before_move, false);

    LOG_DEBUG_GC << "== GenGC CollectYoungAndMove end ==";
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::SweepStringTable()
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_SWEEP_STRING_TABLE);

    ASSERT(this->GetObjectAllocator()->GetYoungSpaceMemRanges().size() == 1);
    // new strings may be created in young space during tenured gc, we shouldn't collect them
    auto young_mem_range = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
    this->GetPandaVm()->SweepStringTable([this, &young_mem_range](ObjectHeader *object) {
        if (young_mem_range.IsAddressInRange(ToUintPtr(object))) {
            return ObjectStatus::ALIVE_OBJECT;
        }
        return this->marker_.MarkChecker(object);
    });
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::UpdateRefsToMovedObjects(PandaVector<ObjectHeader *> *moved_objects)
{
    GCScope<TRACE_TIMING> scope("UpdateRefsToMovedObjects", this);

    auto obj_allocator = this->GetObjectAllocator();
    // Update references exyoung -> young
    LOG_DEBUG_GC << "process moved objects cnt = " << std::dec << moved_objects->size();
    LOG_DEBUG_GC << "=== Update exyoung -> young references. START. ===";
    for (auto obj : *moved_objects) {
        ObjectHelpers<LanguageConfig::LANG_TYPE>::UpdateRefsToMovedObjects(obj);
    }

    LOG_DEBUG_GC << "=== Update exyoung -> young references. END. ===";
    // update references tenured -> young
    LOG_DEBUG_GC << "=== Update tenured -> young references. START. ===";
    auto young_space = obj_allocator->GetYoungSpaceMemRanges().at(0);
    auto update_refs_in_object(
        [](ObjectHeader *obj) { ObjectHelpers<LanguageConfig::LANG_TYPE>::UpdateRefsToMovedObjects(obj); });
    this->GetCardTable()->VisitMarked(
        [&update_refs_in_object, &obj_allocator, &young_space](const MemRange &mem_range) {
            if (!young_space.Contains(mem_range)) {
                obj_allocator->IterateOverObjectsInRange(mem_range, update_refs_in_object);
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
    this->GetPandaVm()->GetMemStats()->RecordGCPauseStart();
    {
        ScopedTiming un_mark_timing("UnMark", *this->GetTiming());
        // Unmark all because no filter out tenured when mark young
        // TODO(dtrubenk): remove this
        this->GetObjectAllocator()->IterateOverObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
    }
    GCMarkingStackType objects_stack(this);
    InitialMark(&objects_stack);
    auto object_allocator = this->GetObjectAllocator();
    this->GetPandaVm()->GetMemStats()->RecordGCPauseEnd();
    auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    this->ConcurrentMark(
        &marker_, &objects_stack, CardTableVisitFlag::VISIT_ENABLED,
        [&objects_stack]() { return !objects_stack.Empty(); }, ref_pred,
        [&object_allocator](MemRange &mem_range) { return !object_allocator->IsIntersectedWithYoung(mem_range); });
    this->GetPandaVm()->GetMemStats()->RecordGCPauseStart();
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    ReMark(&objects_stack, task);
    ASSERT(objects_stack.Empty());
    {
        ScopedTiming un_mark_young_timing("UnMarkYoung", *this->GetTiming());
        // TODO(yxr): remove this after not marking young objects in tenured gc
        this->GetObjectAllocator()->IterateOverYoungObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
    }
    // TODO(dtrubenkov): make concurrent
    SweepStringTable();
    ConcurrentSweep();
    this->GetPandaVm()->GetMemStats()->RecordGCPauseEnd();
    LOG_DEBUG_GC << "GC tenured end";
    task.collection_type_ = GCCollectionType::TENURED;
}

// Full GC is ran on pause
template <class LanguageConfig>
void GenGC<LanguageConfig>::RunFullGC(GCTask &task)
{
    GCScope<TRACE_TIMING> full_gc_scope(__FUNCTION__, this);
    LOG_DEBUG_GC << "Full GC start";
    this->GetPandaVm()->GetMemStats()->RecordGCPauseStart();
    this->SetFullGC(true);
    {
        ScopedTiming un_mark_timing("UnMark", *this->GetTiming());
        this->GetObjectAllocator()->IterateOverObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
    }
    FullMark(task);
    // Sweep dead objects from tenured space
    SweepStringTable();
    Sweep();
    // Young GC
    if (LIKELY(HaveEnoughSpaceToMove())) {
        GCScopedPauseStats scoped_pause_stats(this->GetPandaVm()->GetGCStats());  // Not clear young pause
        // We already marked objects above so just collect and move
        CollectYoungAndMove();
        this->GetCardTable()->ClearAll();
    }
    this->SetFullGC(false);
    this->GetPandaVm()->GetMemStats()->RecordGCPauseEnd();
    LOG_DEBUG_GC << "Full GC end";
    task.collection_type_ = GCCollectionType::FULL;
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkRoots(GCMarkingStackType *objects_stack, CardTableVisitFlag visit_card_table_roots,
                                      const ReferenceCheckPredicateT &ref_pred, VisitGCRootFlags flags)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    GCRootVisitor gc_mark_roots = [this, &objects_stack, &ref_pred](const GCRoot &gc_root) {
        ObjectHeader *root_object = gc_root.GetObjectHeader();
        ObjectHeader *from_object = gc_root.GetFromObjectHeader();
        LOG_DEBUG_GC << "Handle root " << GetDebugInfoAboutObject(root_object);
        if (UNLIKELY(from_object != nullptr) &&
            this->IsReference(from_object->ClassAddr<BaseClass>(), from_object, ref_pred)) {
            LOG_DEBUG_GC << "Add reference: " << GetDebugInfoAboutObject(from_object) << " to stack";
            marker_.Mark(from_object);
            this->ProcessReference(objects_stack, from_object->ClassAddr<BaseClass>(), from_object,
                                   GC::EmptyReferenceProcessPredicate);
        } else {
            // we should always add this object to the stack, because we could mark this object in InitialMark, but
            // write to some fields in ConcurrentMark - need to iterate over all fields again, MarkObjectIfNotMarked
            // can't be used here
            marker_.Mark(root_object);
            objects_stack->PushToStack(gc_root.GetType(), root_object);
        }
    };
    this->VisitRoots(gc_mark_roots, flags);
    if (visit_card_table_roots == CardTableVisitFlag::VISIT_ENABLED) {
        auto allocator = this->GetObjectAllocator();
        ASSERT(allocator->GetYoungSpaceMemRanges().size() == 1);
        MemRange young_mr = allocator->GetYoungSpaceMemRanges().at(0);
        MemRangeChecker young_range_checker = []([[maybe_unused]] MemRange &mem_range) -> bool { return true; };
        ObjectChecker young_range_tenured_object_checker = [&young_mr](const ObjectHeader *object_header) -> bool {
            return !young_mr.IsAddressInRange(ToUintPtr(object_header));
        };
        ObjectChecker from_object_checker = [&young_mr, this](const ObjectHeader *object_header) -> bool {
            // Don't visit objects which are in tenured and not marked.
            return young_mr.IsAddressInRange(ToUintPtr(object_header)) || IsMarked(object_header);
        };
        this->VisitCardTableRoots(this->GetCardTable(), gc_mark_roots, young_range_checker,
                                  young_range_tenured_object_checker, from_object_checker,
                                  CardTableProcessedFlag::VISIT_MARKED);
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::InitialMark(GCMarkingStackType *objects_stack)
{
    GCScope<TRACE_TIMING_PHASE> gc_scope(__FUNCTION__, this, GCPhase::GC_PHASE_INITIAL_MARK);
    {
        NoAtomicGCMarkerScope scope(&this->marker_);
        auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
        MarkRoots(objects_stack, CardTableVisitFlag::VISIT_DISABLED, ref_pred,
                  VisitGCRootFlags::ACCESS_ROOT_NONE | VisitGCRootFlags::START_RECORDING_NEW_ROOT);
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::ReMark(GCMarkingStackType *objects_stack, const GCTask &task)
{
    GCScope<TRACE_TIMING_PHASE> gc_scope(__FUNCTION__, this, GCPhase::GC_PHASE_REMARK);

    // TODO(dtrubenkov): consider iterational concurrent marking of card table
    {
        NoAtomicGCMarkerScope scope(&this->marker_);
        auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
        MarkRoots(objects_stack, CardTableVisitFlag::VISIT_ENABLED, ref_pred,
                  VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW | VisitGCRootFlags::END_RECORDING_NEW_ROOT);
        this->MarkStack(&marker_, objects_stack, ref_pred,
                        []([[maybe_unused]] const ObjectHeader *obj) { return true; });
        {
            ScopedTiming t1("VisitInternalStringTable", *this->GetTiming());
            this->GetPandaVm()->VisitStringTable(
                [this, &objects_stack](ObjectHeader *str) {
                    if (this->MarkObjectIfNotMarked(str)) {
                        ASSERT(str != nullptr);
                        objects_stack->PushToStack(RootType::STRING_TABLE, str);
                    }
                },
                VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW | VisitGCRootFlags::END_RECORDING_NEW_ROOT);
            this->MarkStack(&marker_, objects_stack, ref_pred, GC::EmptyReferenceProcessPredicate);
        }
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        this->GetPandaVm()->HandleReferences(task, GC::EmptyReferenceProcessPredicate);
    }
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::FullMark(const GCTask &task)
{
    GCScope<TRACE_TIMING_PHASE> full_mark_scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    NoAtomicGCMarkerScope marker_scope(&this->marker_);

    GCMarkingStackType objects_stack(this);
    VisitGCRootFlags flags = VisitGCRootFlags::ACCESS_ROOT_ALL;
    auto ref_pred = GC::EmptyReferenceProcessPredicate;
    // Mark all reachable objects
    MarkRoots(&objects_stack, CardTableVisitFlag::VISIT_DISABLED, ref_pred, flags);
    this->GetPandaVm()->VisitStringTable(
        [this, &objects_stack](ObjectHeader *str) {
            if (this->MarkObjectIfNotMarked(str)) {
                ASSERT(str != nullptr);
                objects_stack.PushToStack(RootType::STRING_TABLE, str);
            }
        },
        flags);
    this->MarkStack(&marker_, &objects_stack, ref_pred, []([[maybe_unused]] const ObjectHeader *obj) { return true; });
    auto ref_clear_pred = []([[maybe_unused]] const ObjectHeader *obj) { return true; };
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    this->GetPandaVm()->HandleReferences(task, ref_clear_pred);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::MarkReferences(GCMarkingStackType *references, GCPhase gc_phase)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    LOG_DEBUG_GC << "Start marking " << references->Size() << " references";
    auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    if (gc_phase == GCPhase::GC_PHASE_MARK_YOUNG) {
        this->MarkYoungStack(references);
    } else if (gc_phase == GCPhase::GC_PHASE_INITIAL_MARK || gc_phase == GCPhase::GC_PHASE_MARK ||
               gc_phase == GCPhase::GC_PHASE_REMARK) {
        this->MarkStack(&marker_, references, ref_pred, GC::EmptyReferenceProcessPredicate);
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
void GenGC<LanguageConfig>::UnMarkObject(ObjectHeader *object_header)
{
    LOG_DEBUG_GC << "Set unmark for GC " << GetDebugInfoAboutObject(object_header);
    this->marker_.UnMark(object_header);
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::IsMarked(const ObjectHeader *object) const
{
    return this->marker_.IsMarked(object);
}

template <class LanguageConfig>
void GenGC<LanguageConfig>::Sweep()
{
    GCScope<TRACE_TIMING_PHASE> gc_sweep_scope(__FUNCTION__, this, GCPhase::GC_PHASE_SWEEP);

    size_t freed_object_size = 0U;
    size_t freed_object_count = 0U;

    this->GetObjectAllocator()->Collect(
        [this, &freed_object_size, &freed_object_count](ObjectHeader *object) {
            auto status = this->marker_.MarkChecker(object);
            if (status == ObjectStatus::DEAD_OBJECT) {
                freed_object_size += GetAlignedObjectSize(GetObjectSize(object));
                freed_object_count++;
            }
            return status;
        },
        GCCollectMode::GC_ALL);
    this->GetObjectAllocator()->VisitAndRemoveFreePools([this](void *mem, size_t size) {
        this->GetCardTable()->ClearCardRange(ToUintPtr(mem), ToUintPtr(mem) + size);
        PoolManager::GetMmapMemPool()->FreePool(mem, size);
    });
    this->mem_stats_.RecordSizeFreedTenured(freed_object_size);
    this->mem_stats_.RecordCountFreedTenured(freed_object_count);
}

// NO_THREAD_SAFETY_ANALYSIS because clang thread safety analysis
template <class LanguageConfig>
NO_THREAD_SAFETY_ANALYSIS void GenGC<LanguageConfig>::ConcurrentSweep()
{
    GCScope<TRACE_TIMING> gc_scope(__FUNCTION__, this);
    ConcurrentScope concurrent_scope(this, false);
    size_t freed_object_size = 0U;
    size_t freed_object_count = 0U;

    // NB! can't move block out of brace, we need to make sure GC_PHASE_SWEEP cleared
    {
        GCScopedPhase scoped_phase(this->GetPandaVm()->GetMemStats(), this, GCPhase::GC_PHASE_SWEEP);
        concurrent_scope.Start();  // enable concurrent after GC_PHASE_SWEEP has been set

        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (LanguageConfig::MT_MODE == MT_MODE_MULTI) {
            // Run monitor deflation again, to avoid object was reclaimed before monitor deflate.
            auto young_mr = this->GetObjectAllocator()->GetYoungSpaceMemRanges().at(0);
            this->GetPandaVm()->GetMonitorPool()->DeflateMonitorsWithCallBack([&young_mr, this](Monitor *monitor) {
                ObjectHeader *object_header = monitor->GetObject();
                return (!IsMarked(object_header)) && (!young_mr.IsAddressInRange(ToUintPtr(object_header)));
            });
        }

        this->GetObjectAllocator()->Collect(
            [this, &freed_object_size, &freed_object_count](ObjectHeader *object) {
                auto status = this->marker_.MarkChecker(object);
                if (status == ObjectStatus::DEAD_OBJECT) {
                    LOG_DEBUG_OBJECT_EVENTS << "DELETE OBJECT tenured: " << object;
                    freed_object_size += GetAlignedObjectSize(GetObjectSize(object));
                    freed_object_count++;
                }
                return status;
            },
            GCCollectMode::GC_ALL);
        this->GetObjectAllocator()->VisitAndRemoveFreePools([this](void *mem, size_t size) {
            this->GetCardTable()->ClearCardRange(ToUintPtr(mem), ToUintPtr(mem) + size);
            PoolManager::GetMmapMemPool()->FreePool(mem, size);
        });
    }

    this->mem_stats_.RecordSizeFreedTenured(freed_object_size);
    this->mem_stats_.RecordCountFreedTenured(freed_object_count);

    // In concurrent sweep phase, the new created objects may being marked in InitGCBits,
    // so we need wait for that done, then we can safely unmark objects concurrent with mutator.
    ASSERT(this->GetGCPhase() != GCPhase::GC_PHASE_SWEEP);  // Make sure we are out of sweep scope
    this->GetObjectAllocator()->IterateOverTenuredObjects([this](ObjectHeader *obj) { this->marker_.UnMark(obj); });
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::InGCSweepRange(const ObjectHeader *obj) const
{
    bool in_young_space = this->GetObjectAllocator()->IsAddressInYoungSpace(ToUintPtr(obj));
    auto phase = this->GetGCPhase();

    // Do young GC and the object is in the young space
    if (phase == GCPhase::GC_PHASE_MARK_YOUNG && in_young_space) {
        return true;
    }

    // Do tenured GC and the object is in the tenured space
    if (phase != GCPhase::GC_PHASE_MARK_YOUNG && !in_young_space) {
        return true;
    }

    return this->IsFullGC();
}

template <class LanguageConfig>
bool GenGC<LanguageConfig>::HaveEnoughSpaceToMove() const
{
    // hack for pools because we have 2 type of pools in tenures space, in bad cases objects can be moved to different
    // spaces. And move 4M objects in bump-allocator to other allocator, may need more than 4M space in other allocator
    // - so we need 3 empty pools.
    // TODO(xucheng) : remove the checker when we can do part young collection.
    // The min num that can guarantee that we move all objects in young space.
    constexpr size_t POOLS_NUM = 3;
    return this->GetObjectAllocator()->HaveEnoughPoolsInObjectSpace(POOLS_NUM);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(GenGC);

}  // namespace panda::mem
