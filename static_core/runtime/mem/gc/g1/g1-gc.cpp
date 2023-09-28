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

#include <atomic>
#include "libpandabase/mem/space.h"
#include "mem/gc/gc.h"
#include "runtime/include/language_config.h"
#include "runtime/include/class.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/mem/gc/dynamic/gc_marker_dynamic-inl.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/g1/ref_cache_builder.h"
#include "runtime/mem/gc/g1/update_remset_thread.h"
#include "runtime/mem/gc/workers/gc_workers_task_queue.h"
#include "runtime/mem/gc/workers/gc_workers_thread_pool.h"
#include "runtime/mem/gc/generational-gc-base-inl.h"
#include "runtime/mem/gc/static/gc_marker_static-inl.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/managed_thread.h"
#include "runtime/mem/gc/g1/ref_updater.h"
#include "runtime/mem/gc/g1/card_handler.h"
#include "runtime/mem/region_space.h"

namespace panda::mem {

#ifndef NDEBUG
static bool IsCardTableClear(CardTable *card_table)
{
    bool clear = true;
    card_table->VisitMarked(
        [&clear](const MemRange &range) {
            LOG(ERROR, GC) << "Card [" << ToVoidPtr(range.GetStartAddress()) << " - "
                           << ToVoidPtr(range.GetEndAddress()) << "] is not clear";
            clear = false;
        },
        CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED);
    return clear;
}
#endif

static inline GCG1BarrierSet *GetG1BarrierSet()
{
    Thread *thread = Thread::GetCurrent();
    ASSERT(thread != nullptr);
    GCBarrierSet *barrier_set = thread->GetBarrierSet();
    ASSERT(barrier_set != nullptr);
    return static_cast<GCG1BarrierSet *>(barrier_set);
}

extern "C" void PreWrbFuncEntrypoint(void *old_value)
{
    // The cast below is needed to truncate high 32bits from 64bit pointer
    // in case object pointers have 32bit.
    old_value = ToVoidPtr(ToObjPtr(old_value));
    ASSERT(IsAddressInObjectsHeap(old_value));
    ASSERT(IsAddressInObjectsHeap(static_cast<const ObjectHeader *>(old_value)->ClassAddr<BaseClass>()));
    LOG(DEBUG, GC) << "G1GC pre barrier val = " << std::hex << old_value;
    auto *thread = ManagedThread::GetCurrent();
    // thread can't be null here because pre-barrier is called only in concurrent-mark, but we don't process
    // weak-references in concurrent mark
    ASSERT(thread != nullptr);
    auto buffer_vec = thread->GetPreBuff();
    buffer_vec->push_back(static_cast<ObjectHeader *>(old_value));
}

// PostWrbUpdateCardFuncEntrypoint
extern "C" void PostWrbUpdateCardFuncEntrypoint(const void *from, const void *to)
{
    ASSERT(from != nullptr);
    ASSERT(to != nullptr);
    // The cast below is needed to truncate high 32bits from 64bit pointer
    // in case object pointers have 32bit.
    from = ToVoidPtr(ToObjPtr(from));
    GCG1BarrierSet *barriers = GetG1BarrierSet();
    ASSERT(barriers != nullptr);
    auto card_table = barriers->GetCardTable();
    ASSERT(card_table != nullptr);
    // No need to keep remsets for young->young
    // TODO(dtrubenkov): add assert that we do not have young -> young reference here
    auto *card = card_table->GetCardPtr(ToUintPtr(from));
    LOG(DEBUG, GC) << "G1GC post queue add ref: " << std::hex << from << " -> " << ToVoidPtr(ToObjPtr(to))
                   << " from_card: " << card_table->GetMemoryRange(card);
    // TODO(dtrubenkov): remove !card->IsYoung() after it will be encoded in compiler barrier
    if ((card->IsClear()) && (!card->IsYoung())) {
        // TODO(dtrubenkov): either encode this in compiler barrier or remove from Interpreter barrier (if move to
        // INT/JIT parts then don't check IsClear here cause it will be marked already)
        card->Mark();
        barriers->Enqueue(card);
    }
}

/* static */
template <class LanguageConfig>
void G1GC<LanguageConfig>::CalcLiveBytesMarkPreprocess(const ObjectHeader *object, BaseClass *base_klass)
{
    Region *region = ObjectToRegion(object);
    size_t object_size = GetAlignedObjectSize(object->ObjectSize<LanguageConfig::LANG_TYPE>(base_klass));
    region->AddLiveBytes<true>(object_size);
}

/* static */
template <class LanguageConfig>
void G1GC<LanguageConfig>::CalcLiveBytesNotAtomicallyMarkPreprocess(const ObjectHeader *object, BaseClass *base_klass)
{
    Region *region = ObjectToRegion(object);
    size_t object_size = GetAlignedObjectSize(object->ObjectSize<LanguageConfig::LANG_TYPE>(base_klass));
    region->AddLiveBytes<false>(object_size);
}

template <class LanguageConfig>
G1GC<LanguageConfig>::G1GC(ObjectAllocatorBase *object_allocator, const GCSettings &settings)
    : GenerationalGC<LanguageConfig>(object_allocator, settings),
      marker_(this),
      conc_marker_(this),
      mixed_marker_(this),
      concurrent_marking_stack_(this),
      number_of_mixed_tenured_regions_(settings.GetG1NumberOfTenuredRegionsAtMixedCollection()),
      region_garbage_rate_threshold_(settings.G1RegionGarbageRateThreshold()),
      g1_promotion_region_alive_rate_(settings.G1PromotionRegionAliveRate()),
      g1_track_freed_objects_(settings.G1TrackFreedObjects()),
      is_explicit_concurrent_gc_enabled_(settings.IsExplicitConcurrentGcEnabled()),
      region_size_bits_(panda::helpers::math::GetIntLog2(this->GetG1ObjectAllocator()->GetRegionSize())),
      g1_pause_tracker_(settings.GetG1GcPauseIntervalInMillis(), settings.GetG1MaxGcPauseInMillis()),
      analytics_(panda::time::GetCurrentTimeInNanos())
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    this->SetType(GCType::G1_GC);
    this->SetTLABsSupported();
    updated_refs_queue_ = allocator->New<GCG1BarrierSet::ThreadLocalCardQueues>();
    auto *first_ref_vector = allocator->New<RefVector>();
    first_ref_vector->reserve(MAX_REFS);
    unique_refs_from_remsets_.push_back(first_ref_vector);
    GetG1ObjectAllocator()->ReserveRegionIfNeeded();
}

template <class LanguageConfig>
G1GC<LanguageConfig>::~G1GC()
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    {
        for (auto obj_vector : satb_buff_list_) {
            allocator->Delete(obj_vector);
        }
    }
    allocator->Delete(updated_refs_queue_);
    ASSERT(unique_refs_from_remsets_.size() == 1);
    allocator->Delete(unique_refs_from_remsets_.front());
    unique_refs_from_remsets_.clear();
    this->GetInternalAllocator()->Delete(update_remset_thread_);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::InitGCBits(panda::ObjectHeader *obj_header)
{
    // The mutator may create a new object during concurrent marking phase.
    // In this case GC may don't mark it (for example only vregs may contain reference to the new object)
    // and collect. To avoid such situations add objects to a special buffer which
    // will be processed at remark stage.
    if (this->GetCardTable()->GetCardPtr(ToUintPtr(obj_header))->IsYoung() ||
        // Atomic with acquire order reason: read variable modified in GC thread
        !concurrent_marking_flag_.load(std::memory_order_acquire)) {
        return;
    }
    os::memory::LockHolder lock(satb_and_newobj_buf_lock_);
    newobj_buffer_.push_back(obj_header);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::PreStartupImp()
{
    GenerationalGC<LanguageConfig>::DisableTenuredGC();
}

template <class LanguageConfig>
template <RegionFlag REGION_TYPE, bool FULL_GC>
void G1GC<LanguageConfig>::DoRegionCompacting(Region *region, bool use_gc_workers,
                                              PandaVector<PandaVector<ObjectHeader *> *> *moved_objects_vector)
{
    auto internal_allocator = this->GetInternalAllocator();
    ObjectVisitor moved_object_saver;
    if constexpr (FULL_GC) {
        PandaVector<ObjectHeader *> *moved_objects;
        if (use_gc_workers) {
            moved_objects = internal_allocator->template New<PandaVector<ObjectHeader *>>();
            moved_objects_vector->push_back(moved_objects);
            size_t move_size = region->GetAllocatedBytes();
            moved_objects->reserve(move_size / GetMinimalObjectSize());
        } else {
            ASSERT(moved_objects_vector->size() == 1);
            moved_objects = moved_objects_vector->back();
        }
        moved_object_saver = [moved_objects](ObjectHeader *object) { moved_objects->push_back(object); };
    } else {
        moved_object_saver = []([[maybe_unused]] const ObjectHeader *object) {};
    }

    if (use_gc_workers) {
        auto *storage =
            internal_allocator->template New<GCRegionCompactWorkersTask::RegionDataType>(region, moved_object_saver);
        if (!this->GetWorkersPool()->AddTask(GCRegionCompactWorkersTask(storage))) {
            // We couldn't send a task to workers. Therefore, do it here.
            internal_allocator->Delete(storage);
            RegionCompactingImpl<true, REGION_TYPE>(region, moved_object_saver);
        }
    } else {
        RegionCompactingImpl<false, REGION_TYPE>(region, moved_object_saver);
    }
}

class ScopedRegionCollectionInfo {
public:
    ScopedRegionCollectionInfo(const GC *gc, const char *title, const Region *region, bool is_young,
                               const size_t &moved_size)
        : gc_(gc),
          title_(title),
          region_(region),
          is_young_(is_young),
          moved_size_(moved_size),
          start_time_ns_(time::GetCurrentTimeInNanos())
    {
    }

    NO_COPY_SEMANTIC(ScopedRegionCollectionInfo);
    NO_MOVE_SEMANTIC(ScopedRegionCollectionInfo);

    ~ScopedRegionCollectionInfo()
    {
        if (gc_->IsLogDetailedGcCompactionInfoEnabled()) {
            LOG(INFO, GC) << *this;
        }
    }

private:
    const GC *gc_;
    const char *title_;
    const Region *region_;
    bool is_young_;
    const size_t &moved_size_;
    uint64_t start_time_ns_;

    friend std::ostream &operator<<(std::ostream &log, const ScopedRegionCollectionInfo &region_info)
    {
        auto region = region_info.region_;
        log << '[' << region_info.gc_->GetCounter() << "] " << region_info.title_ << ": ";
        // Need to use saved is_young_ flag since region flags can be changed during region promotion
        if (region_info.is_young_) {
            log << 'Y';
        } else {
            log << 'T';
        }
        DumpRegionRange(log, *region) << " A " << panda::helpers::MemoryConverter(region->GetAllocatedBytes()) << " L ";
        if (region_info.is_young_) {
            log << '-';
        } else {
            log << panda::helpers::MemoryConverter(region->GetLiveBytes());
        }
        log << " RS " << region->GetRemSetSize() << " M " << panda::helpers::MemoryConverter(region_info.moved_size_)
            << " D " << panda::helpers::TimeConverter(time::GetCurrentTimeInNanos() - region_info.start_time_ns_);
        return log;
    }
};

template <class LanguageConfig>
template <bool ATOMIC>
void G1GC<LanguageConfig>::RegionPromotionImpl(Region *region, const ObjectVisitor &moved_object_saver)
{
    size_t move_size = region->GetAllocatedBytes();
    size_t alive_move_count = 0;
    size_t dead_move_count = 0;
    auto object_allocator = this->GetG1ObjectAllocator();
    auto promotion_move_checker = [&alive_move_count, &moved_object_saver](ObjectHeader *src) {
        ++alive_move_count;
        LOG_DEBUG_OBJECT_EVENTS << "PROMOTE YOUNG object " << src;
        ASSERT(ObjectToRegion(src)->HasFlag(RegionFlag::IS_EDEN));
        moved_object_saver(src);
    };
    auto promotion_death_checker = [this, &dead_move_count](ObjectHeader *object_header) {
        if (IsMarked(object_header)) {
            return ObjectStatus::ALIVE_OBJECT;
        }
        ++dead_move_count;
        LOG_DEBUG_OBJECT_EVENTS << "PROMOTE DEAD YOUNG object " << object_header;
        return ObjectStatus::DEAD_OBJECT;
    };
    ScopedRegionCollectionInfo collection_info(this, "Region promoted", region, true, move_size);
    if (g1_track_freed_objects_) {
        // We want to track all moved objects (including), therefore, iterate over all objects in region.
        object_allocator->template PromoteYoungRegion<false>(region, promotion_death_checker, promotion_move_checker);
    } else {
        object_allocator->template PromoteYoungRegion<true>(region, promotion_death_checker, promotion_move_checker);
        ASSERT(dead_move_count == 0);
    }
    region->RmvFlag(RegionFlag::IS_COLLECTION_SET);
    this->mem_stats_.template RecordSizeMovedYoung<ATOMIC>(move_size);
    this->mem_stats_.template RecordCountMovedYoung<ATOMIC>(alive_move_count + dead_move_count);
    analytics_.ReportPromotedRegion();
    analytics_.ReportLiveObjects(alive_move_count);
}

template <class LanguageConfig>
template <typename Handler>
void G1GC<LanguageConfig>::IterateOverRefsInMemRange(const MemRange &mem_range, Region *region, Handler &refs_handler)
{
    MarkBitmap *bitmap = nullptr;
    if (region->IsEden()) {
        ASSERT(this->IsFullGC());
        bitmap = region->GetMarkBitmap();
    } else {
        bitmap = region->GetLiveBitmap();
    }
    auto *start_address = ToVoidPtr(mem_range.GetStartAddress());
    auto *end_address = ToVoidPtr(mem_range.GetEndAddress());
    auto visitor = [&refs_handler, start_address, end_address](void *mem) {
        ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(
            static_cast<ObjectHeader *>(mem), refs_handler, start_address, end_address);
    };
    if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
        bitmap->CallForMarkedChunkInHumongousRegion<false>(ToVoidPtr(region->Begin()), visitor);
    } else {
        bitmap->IterateOverMarkedChunkInRange(start_address, end_address, visitor);
    }
}

template <class LanguageConfig, bool CONCURRENTLY, bool COLLECT_CLASSES>
class NonRegularObjectsDeathChecker {
public:
    NonRegularObjectsDeathChecker(size_t *delete_size, size_t *delete_count)
        : delete_size_(delete_size), delete_count_(delete_count)
    {
    }

    ~NonRegularObjectsDeathChecker() = default;

    ObjectStatus operator()(ObjectHeader *object_header)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (CONCURRENTLY) {
            // We may face a newly created object without live bitmap initialization.
            if (object_header->template AtomicClassAddr<BaseClass>() == nullptr) {
                return ObjectStatus::ALIVE_OBJECT;
            }
        }
        Region *region = ObjectToRegion(object_header);
        auto live_bitmap = region->GetLiveBitmap();
        if (live_bitmap->AtomicTest(object_header)) {
            return ObjectStatus::ALIVE_OBJECT;
        }
        if constexpr (!COLLECT_CLASSES) {
            if (ObjectHelpers<LanguageConfig::LANG_TYPE>::IsClassObject(object_header)) {
                LOG_DEBUG_OBJECT_EVENTS << "DELETE NON MOVABLE class object " << object_header
                                        << " but don't free memory";
                return ObjectStatus::ALIVE_OBJECT;
            }
        }

        if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
            LOG_DEBUG_OBJECT_EVENTS << "DELETE HUMONGOUS object " << object_header;
            // humongous allocator increases size by region size
            *delete_size_ += region->Size();
            ++(*delete_count_);
        } else {
            ASSERT(region->HasFlag(RegionFlag::IS_NONMOVABLE));
            LOG_DEBUG_OBJECT_EVENTS << "DELETE NON MOVABLE object " << object_header;
        }
        return ObjectStatus::DEAD_OBJECT;
    }

    DEFAULT_COPY_SEMANTIC(NonRegularObjectsDeathChecker);
    DEFAULT_MOVE_SEMANTIC(NonRegularObjectsDeathChecker);

private:
    size_t *delete_size_;
    size_t *delete_count_;
};

template <class LanguageConfig>
template <bool ATOMIC, bool CONCURRENTLY>
void G1GC<LanguageConfig>::CollectEmptyRegions(GCTask &task, PandaVector<Region *> *empty_tenured_regions)
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    CollectNonRegularObjects<ATOMIC, CONCURRENTLY>();
    ClearEmptyTenuredMovableRegions<ATOMIC, CONCURRENTLY>(empty_tenured_regions);
    task.UpdateGCCollectionType(GCCollectionType::TENURED);
}

template <class LanguageConfig>
template <bool ATOMIC, bool CONCURRENTLY>
void G1GC<LanguageConfig>::CollectNonRegularObjects()
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    size_t delete_size = 0;
    size_t delete_count = 0;
    // Don't collect classes if --g1-track-free-objects is enabled.
    // We need to know size of objects while iterating over all objects in the collected region.
    auto death_checker =
        g1_track_freed_objects_
            ? GCObjectVisitor(
                  NonRegularObjectsDeathChecker<LanguageConfig, CONCURRENTLY, false>(&delete_size, &delete_count))
            : GCObjectVisitor(
                  NonRegularObjectsDeathChecker<LanguageConfig, CONCURRENTLY, true>(&delete_size, &delete_count));
    auto region_visitor = [this](PandaVector<Region *> &regions) {
        if constexpr (CONCURRENTLY) {
            update_remset_thread_->InvalidateRegions(&regions);
        } else {
            update_remset_thread_->GCInvalidateRegions(&regions);
        }
    };
    this->GetG1ObjectAllocator()->CollectNonRegularRegions(region_visitor, death_checker);
    this->mem_stats_.template RecordCountFreedTenured<ATOMIC>(delete_count);
    this->mem_stats_.template RecordSizeFreedTenured<ATOMIC>(delete_size);
}

PandaVector<Region *> GetEmptyTenuredRegularRegionsFromQueue(
    PandaPriorityQueue<std::pair<uint32_t, Region *>> garbage_regions)
{
    PandaVector<Region *> empty_tenured_regions;
    while (!garbage_regions.empty()) {
        auto *top_region = garbage_regions.top().second;
        if (top_region->GetLiveBytes() == 0U) {
            empty_tenured_regions.push_back(top_region);
        }
        garbage_regions.pop();
    }
    return empty_tenured_regions;
}

template <class LanguageConfig>
template <bool ATOMIC, bool CONCURRENTLY>
void G1GC<LanguageConfig>::ClearEmptyTenuredMovableRegions(PandaVector<Region *> *empty_tenured_regions)
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    {
        ScopedTiming t1("Region Invalidation", *this->GetTiming());
        if constexpr (CONCURRENTLY) {
            update_remset_thread_->InvalidateRegions(empty_tenured_regions);
        } else {
            update_remset_thread_->GCInvalidateRegions(empty_tenured_regions);
        }
    }
    size_t delete_size = 0;
    size_t delete_count = 0;
    auto death_visitor = [](ObjectHeader *object_header) {
        LOG_DEBUG_OBJECT_EVENTS << "DELETE tenured object " << object_header;
    };
    for (auto i : *empty_tenured_regions) {
        delete_count += i->GetAllocatedObjects();
        delete_size += i->GetAllocatedBytes();
        ASSERT(i->GetLiveBitmap()->FindFirstMarkedChunks() == nullptr);
        if (g1_track_freed_objects_) {
            i->IterateOverObjects(death_visitor);
        }
    }
    {
        ScopedTiming t2("Reset regions", *this->GetTiming());
        if (CONCURRENTLY) {
            this->GetG1ObjectAllocator()
                ->template ResetRegions<RegionFlag::IS_OLD, RegionSpace::ReleaseRegionsPolicy::NoRelease,
                                        OSPagesPolicy::IMMEDIATE_RETURN, true, PandaVector<Region *>>(
                    *empty_tenured_regions);
        } else {
            this->GetG1ObjectAllocator()
                ->template ResetRegions<RegionFlag::IS_OLD, RegionSpace::ReleaseRegionsPolicy::Release,
                                        OSPagesPolicy::NO_RETURN, false, PandaVector<Region *>>(*empty_tenured_regions);
        }
    }
    this->mem_stats_.template RecordCountFreedTenured<ATOMIC>(delete_count);
    this->mem_stats_.template RecordSizeFreedTenured<ATOMIC>(delete_size);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::NeedToPromote(const Region *region) const
{
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    if (region->HasPinnedObjects()) {
        return true;
    }
    if ((g1_promotion_region_alive_rate_ < PERCENT_100_D) && !this->IsFullGC()) {
        size_t alive_bytes = region->GetLiveBytes();
        double alive_percentage = static_cast<double>(alive_bytes) / region->Size() * PERCENT_100_D;
        if (alive_percentage >= g1_promotion_region_alive_rate_) {
            return true;
        }
    }
    return false;
}

template <class LanguageConfig>
template <bool ATOMIC, RegionFlag REGION_TYPE>
void G1GC<LanguageConfig>::RegionCompactingImpl(Region *region, const ObjectVisitor &moved_object_saver)
{
    auto object_allocator = this->GetG1ObjectAllocator();
    // Calculated live bytes in region for all marked objects during MixedMark
    size_t move_size = region->GetLiveBytes();
    size_t move_count = 0;
    size_t allocated_size = region->GetAllocatedBytes();
    ASSERT(move_size <= allocated_size);
    size_t delete_size = allocated_size - move_size;
    size_t delete_count = 0;

    auto move_checker = [this, &move_count, &moved_object_saver](ObjectHeader *src, ObjectHeader *dst) {
        LOG_DEBUG_OBJECT_EVENTS << "MOVE object " << src << " -> " << dst;
        ASSERT(ObjectToRegion(dst)->HasFlag(RegionFlag::IS_OLD));
        this->SetForwardAddress(src, dst);
        ++move_count;
        moved_object_saver(dst);
    };

    auto death_checker = [this, &delete_count](ObjectHeader *object_header) {
        if (IsMarked(object_header)) {
            return ObjectStatus::ALIVE_OBJECT;
        }
        ++delete_count;
        if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
            LOG_DEBUG_OBJECT_EVENTS << "DELETE YOUNG object " << object_header;
        } else {
            ASSERT(REGION_TYPE == RegionFlag::IS_OLD);
            LOG_DEBUG_OBJECT_EVENTS << "DELETE TENURED object " << object_header;
        }
        return ObjectStatus::DEAD_OBJECT;
    };
    if constexpr (REGION_TYPE == RegionFlag::IS_EDEN) {
        if (!this->NeedToPromote(region)) {
            ScopedRegionCollectionInfo collection_info(this, "Region compacted", region, true, move_size);
            if (g1_track_freed_objects_) {
                // We want to track all freed objects, therefore, iterate over all objects in region.
                object_allocator->template CompactRegion<RegionFlag::IS_EDEN, false>(region, death_checker,
                                                                                     move_checker);
            } else {
                object_allocator->template CompactRegion<RegionFlag::IS_EDEN, true>(region, death_checker,
                                                                                    move_checker);
                // delete_count is equal to 0 because we don't track allocation in TLABs by a default.
                // We will do it only with PANDA_TRACK_TLAB_ALLOCATIONS key
                ASSERT(delete_count == 0);
            }
            this->mem_stats_.template RecordSizeMovedYoung<ATOMIC>(move_size);
            this->mem_stats_.template RecordCountMovedYoung<ATOMIC>(move_count);
            this->mem_stats_.template RecordSizeFreedYoung<ATOMIC>(delete_size);
            this->mem_stats_.template RecordCountFreedYoung<ATOMIC>(delete_count);
            analytics_.ReportEvacuatedBytes(move_size);
            analytics_.ReportLiveObjects(move_count);
        } else {
            RegionPromotionImpl<ATOMIC>(region, moved_object_saver);
        }
    } else {
        ScopedRegionCollectionInfo collection_info(this, "Region compacted", region, false, move_size);
        ASSERT(region->HasFlag(RegionFlag::IS_OLD));
        ASSERT(!region->HasFlag(RegionFlag::IS_NONMOVABLE) && !region->HasFlag(RegionFlag::IS_LARGE_OBJECT));
        if (g1_track_freed_objects_) {
            // We want to track all freed objects, therefore, iterate over all objects in region.
            object_allocator->template CompactRegion<RegionFlag::IS_OLD, false>(region, death_checker, move_checker);
        } else {
            object_allocator->template CompactRegion<RegionFlag::IS_OLD, true>(region, death_checker, move_checker);
            size_t allocated_objects = region->GetAllocatedObjects();
            ASSERT(move_count <= allocated_objects);
            ASSERT(delete_count == 0);
            delete_count = allocated_objects - move_count;
        }
        this->mem_stats_.template RecordSizeMovedTenured<ATOMIC>(move_size);
        this->mem_stats_.template RecordCountMovedTenured<ATOMIC>(move_count);
        this->mem_stats_.template RecordSizeFreedTenured<ATOMIC>(delete_size);
        this->mem_stats_.template RecordCountFreedTenured<ATOMIC>(delete_count);
    }
}

template <class LanguageConfig, typename RefUpdater, bool FULL_GC>
void DoUpdateReferencesToMovedObjectsRange(typename GCUpdateRefsWorkersTask<FULL_GC>::MovedObjectsRange *moved_objects,
                                           RefUpdater &ref_updater)
{
    for (auto *obj : *moved_objects) {
        if constexpr (!FULL_GC) {
            obj = obj->IsForwarded() ? GetForwardAddress(obj) : obj;
        }
        ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(obj, ref_updater);
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::WorkerTaskProcessing(GCWorkersTask *task, [[maybe_unused]] void *worker_data)
{
    switch (task->GetType()) {
        case GCWorkersTaskTypes::TASK_MARKING: {
            auto objects_stack = task->Cast<GCMarkWorkersTask>()->GetMarkingStack();
            MarkStackMixed(objects_stack);
            ASSERT(objects_stack->Empty());
            this->GetInternalAllocator()->Delete(objects_stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_REMARK: {
            auto *objects_stack = task->Cast<GCMarkWorkersTask>()->GetMarkingStack();
            this->MarkStack(&marker_, objects_stack, CalcLiveBytesMarkPreprocess);
            ASSERT(objects_stack->Empty());
            this->GetInternalAllocator()->Delete(objects_stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_FULL_MARK: {
            const ReferenceCheckPredicateT &ref_enable_pred = []([[maybe_unused]] const ObjectHeader *obj) {
                // process all refs
                return true;
            };
            auto *objects_stack = task->Cast<GCMarkWorkersTask>()->GetMarkingStack();
            this->MarkStack(&marker_, objects_stack, CalcLiveBytesMarkPreprocess, ref_enable_pred);
            ASSERT(objects_stack->Empty());
            this->GetInternalAllocator()->Delete(objects_stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_REGION_COMPACTING: {
            auto *data = task->Cast<GCRegionCompactWorkersTask>()->GetRegionData();
            Region *region = data->first;
            const ObjectVisitor &moved_objects_saver = data->second;
            if (region->HasFlag(RegionFlag::IS_EDEN)) {
                RegionCompactingImpl<true, RegionFlag::IS_EDEN>(region, moved_objects_saver);
            } else if (region->HasFlag(RegionFlag::IS_OLD)) {
                RegionCompactingImpl<true, RegionFlag::IS_OLD>(region, moved_objects_saver);
            } else {
                LOG(FATAL, GC) << "Unsupported region type";
            }
            this->GetInternalAllocator()->Delete(data);
            break;
        }
        case GCWorkersTaskTypes::TASK_RETURN_FREE_PAGES_TO_OS: {
            PoolManager::GetMmapMemPool()->ReleasePagesInFreePools();
            break;
        }
        case GCWorkersTaskTypes::TASK_ENQUEUE_REMSET_REFS: {
            auto *moved_objects_range = task->Cast<GCUpdateRefsWorkersTask<false>>()->GetMovedObjectsRange();
            auto *task_updated_refs_queue =
                this->GetInternalAllocator()->template New<GCG1BarrierSet::ThreadLocalCardQueues>();
            EnqueueRemsetRefUpdater<LanguageConfig> ref_updater(this->GetCardTable(), task_updated_refs_queue,
                                                                region_size_bits_);
            DoUpdateReferencesToMovedObjectsRange<LanguageConfig, decltype(ref_updater), false>(moved_objects_range,
                                                                                                ref_updater);
            {
                os::memory::LockHolder lock(gc_worker_queue_lock_);
                updated_refs_queue_->insert(updated_refs_queue_->end(), task_updated_refs_queue->begin(),
                                            task_updated_refs_queue->end());
            }
            this->GetInternalAllocator()->Delete(moved_objects_range);
            this->GetInternalAllocator()->Delete(task_updated_refs_queue);
            break;
        }
        default:
            LOG(FATAL, GC) << "Unimplemented for " << GCWorkersTaskTypesToString(task->GetType());
            UNREACHABLE();
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::UpdateCollectionSet(const CollectionSet &collectible_regions)
{
    collection_set_ = collectible_regions;
    for (auto r : collection_set_) {
        // we don't need to reset flag, because we don't reuse collection_set region
        r->AddFlag(RegionFlag::IS_COLLECTION_SET);
        LOG_DEBUG_GC << "dump region: " << *r;
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunPhasesForRegions(panda::GCTask &task, const CollectionSet &collectible_regions)
{
    if (collectible_regions.empty()) {
        LOG_DEBUG_GC << "No regions specified for collection " << task.reason;
    }
    ASSERT(concurrent_marking_stack_.Empty());
    this->GetObjectGenAllocator()->InvalidateSpaceData();
    this->GetObjectGenAllocator()->UpdateSpaceData();
    RunGC(task, collectible_regions);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::NeedToRunGC(const panda::GCTask &task)
{
    return (task.reason == GCTaskCause::YOUNG_GC_CAUSE) || (task.reason == GCTaskCause::OOM_CAUSE) ||
           (task.reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE && !this->GetSettings()->G1EnablePauseTimeGoal()) ||
           (task.reason == GCTaskCause::STARTUP_COMPLETE_CAUSE) || (task.reason == GCTaskCause::EXPLICIT_CAUSE) ||
           (task.reason == GCTaskCause::NATIVE_ALLOC_CAUSE) || (task.reason == GCTaskCause::MIXED);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::NeedFullGC(const panda::GCTask &task)
{
    return this->IsExplicitFull(task) || (task.reason == GCTaskCause::OOM_CAUSE);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunPhasesImpl(panda::GCTask &task)
{
    SuspendUpdateRemsetThreadScope urt_scope(update_remset_thread_);
    interrupt_concurrent_flag_ = false;
    LOG_DEBUG_GC << "G1GC start, reason: " << task.reason;
    LOG_DEBUG_GC << "Footprint before GC: " << this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    task.UpdateGCCollectionType(GCCollectionType::YOUNG);

    size_t bytes_in_heap_before_move = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    {
        ScopedTiming t("G1 GC", *this->GetTiming());
        {
            GCScopedPauseStats scoped_pause_stats(this->GetPandaVm()->GetGCStats());
            this->mem_stats_.Reset();
            if (NeedToRunGC(task)) {
                // Check there is no concurrent mark running by another thread.
                [[maybe_unused]] auto callback = [](ManagedThread *thread) {
                    return thread->GetPreWrbEntrypoint() == nullptr;
                };
                ASSERT(Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(callback));

                if (NeedFullGC(task)) {
                    task.collection_type = GCCollectionType::FULL;
                    RunFullGC(task);
                } else {
                    bool is_mixed = false;
                    if (task.reason == GCTaskCause::MIXED && !interrupt_concurrent_flag_) {
                        region_garbage_rate_threshold_ = 0;
                        is_mixed = true;
                    } else {
                        // Atomic with acquire order reason: to see changes made by GC thread (which do concurrent
                        // marking and than set is_mixed_gc_required_) in mutator thread which waits for the end of
                        // concurrent marking.
                        is_mixed = is_mixed_gc_required_.load(std::memory_order_acquire);
                    }
                    if (is_mixed) {
                        task.collection_type = GCCollectionType::MIXED;
                    } else {
                        task.collection_type = GCCollectionType::YOUNG;
                    }
                    auto collectible_regions = GetCollectibleRegions(task, is_mixed);
                    if (!collectible_regions.empty() && HaveEnoughSpaceToMove(collectible_regions)) {
                        // Ordinary collection flow
                        RunMixedGC(task, collectible_regions);
                    } else {
                        LOG_DEBUG_GC << "Failed to run gc: "
                                     << (collectible_regions.empty() ? "nothing to collect in movable space"
                                                                     : "not enough free regions to move");
                    }
                }
            }
        }
        if (task.reason == GCTaskCause::MIXED) {
            // There was forced a mixed GC. This GC type sets specific settings.
            // So we need to restore them.
            region_garbage_rate_threshold_ = this->GetSettings()->G1RegionGarbageRateThreshold();
        }
        if (ScheduleMixedGCAndConcurrentMark(task)) {
            RunConcurrentMark(task);
        }
    }
    // Update global and GC memstats based on generational memstats information
    // We will update tenured stats and record allocations, so set 'true' values
    this->UpdateMemStats(bytes_in_heap_before_move, true, true);

    LOG_DEBUG_GC << "Footprint after GC: " << this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    this->SetFullGC(false);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunFullGC(panda::GCTask &task)
{
    ScopedTiming t("Run Full GC", *this->GetTiming());
    GetG1ObjectAllocator()->template ReleaseEmptyRegions<RegionFlag::IS_OLD, OSPagesPolicy::NO_RETURN>();
    LOG_DEBUG_GC << "Explicit Full GC invocation due to a reason: " << task.reason;
    this->SetFullGC(true);
    FullMarking(task);
    if (!HaveEnoughRegionsToMove(1)) {
        GetG1ObjectAllocator()->ReleaseReservedRegion();
        // After release reserved region we always have minimum 1 region for tenured collection
        ASSERT(HaveEnoughRegionsToMove(1));
    }
    CollectionSet collection_set = GetFullCollectionSet();
    PrepareYoungRegionsForFullGC(collection_set);
    auto cur_region_it = collection_set.Tenured().begin();
    auto end_region_it = collection_set.Tenured().end();
    while (cur_region_it != end_region_it) {
        ASSERT(HaveEnoughRegionsToMove(1));
        CollectionSet cs;
        while ((cur_region_it != end_region_it) && (HaveEnoughRegionsToMove(cs.Movable().size() + 1))) {
            Region *region = *cur_region_it;
            cur_region_it++;
            if (region->GetGarbageBytes() == 0) {
                double region_fragmentation = region->GetFragmentation();
                if (region_fragmentation < this->GetSettings()->G1FullGCRegionFragmentationRate()) {
                    LOG_DEBUG_GC << "Skip region " << *region << " because it has no garbage inside";
                    continue;
                }
                LOG_DEBUG_GC << "Add region " << *region
                             << " to a collection set because it has a big fragmentation = " << region_fragmentation;
            } else {
                LOG_DEBUG_GC << "Add region " << *region << " to a collection set";
            }
            cs.AddRegion(region);
        }
        UpdateCollectionSet(cs);
        CollectAndMove<true>(cs);
        LOG_DEBUG_GC << "Iterative full GC, collected " << cs.size() << " regions";
    }
    // Reserve a region to prevent OOM in case a lot of garbage in tenured space
    GetG1ObjectAllocator()->ReserveRegionIfNeeded();
    if (!collection_set.Young().empty()) {
        CollectionSet cs(collection_set.Young());
        if (HaveEnoughSpaceToMove(cs)) {
            LOG_DEBUG_GC << "Iterative full GC. Collecting " << cs.size() << " young regions";
            UpdateCollectionSet(cs);
            CollectAndMove<true>(cs);
        } else {
            RestoreYoungRegionsAfterFullGC(cs);
            LOG_INFO_GC << "Failed to run gc, not enough free regions for young";
            LOG_INFO_GC << "Accounted total object used bytes = "
                        << PoolManager::GetMmapMemPool()->GetObjectUsedBytes();
        }
    }
    {
        ScopedTiming release_pages("Release Pages in Free Pools", *this->GetTiming());
        bool use_gc_workers = this->GetSettings()->GCWorkersCount() != 0;
        if (use_gc_workers) {
            if (!this->GetWorkersPool()->AddTask(GCWorkersTaskTypes::TASK_RETURN_FREE_PAGES_TO_OS)) {
                PoolManager::GetMmapMemPool()->ReleasePagesInFreePools();
            }
        } else {
            PoolManager::GetMmapMemPool()->ReleasePagesInFreePools();
        }
    }
    this->SetFullGC(false);
    collection_set_.clear();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunMixedGC(panda::GCTask &task, const CollectionSet &collection_set)
{
    auto start_time = panda::time::GetCurrentTimeInNanos();
    analytics_.ReportCollectionStart(start_time);
    LOG_DEBUG_GC << "Collect regions size:" << collection_set.size();
    UpdateCollectionSet(collection_set);
    RunPhasesForRegions(task, collection_set);
    auto end_time = panda::time::GetCurrentTimeInNanos();
    this->GetStats()->AddTimeValue(end_time - start_time, TimeTypeStats::YOUNG_TOTAL_TIME);
    g1_pause_tracker_.AddPauseInNanos(start_time, end_time);
    analytics_.ReportCollectionEnd(end_time, collection_set);
    collection_set_.clear();
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::ScheduleMixedGCAndConcurrentMark(panda::GCTask &task)
{
    // Atomic with acquire order reason: to see changes made by GC thread (which do concurrent marking and than set
    // is_mixed_gc_required_) in mutator thread which waits for the end of concurrent marking.
    if (is_mixed_gc_required_.load(std::memory_order_acquire)) {
        if (!HaveGarbageRegions()) {
            // Atomic with release order reason: to see changes made by GC thread (which do concurrent marking and
            // than set is_mixed_gc_required_) in mutator thread which waits for the end of concurrent marking.
            is_mixed_gc_required_.store(false, std::memory_order_release);
        }
        return false;  // don't run concurrent mark
    }
    concurrent_marking_flag_ = !interrupt_concurrent_flag_ && this->ShouldRunTenuredGC(task);
    // Atomic with relaxed order reason: read variable modified in the same thread
    return concurrent_marking_flag_.load(std::memory_order_relaxed);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunConcurrentMark(panda::GCTask &task)
{
    ASSERT(collection_set_.empty());
    // Init concurrent marking
    auto addr = this->GetBarrierSet()->GetBarrierOperand(panda::mem::BarrierPosition::BARRIER_POSITION_PRE,
                                                         "STORE_IN_BUFF_TO_MARK_FUNC");
    auto pre_wrb_entrypoint = std::get<void (*)(void *)>(addr.GetValue());
    auto callback = [&pre_wrb_entrypoint](ManagedThread *thread) {
        ASSERT(thread->GetPreWrbEntrypoint() == nullptr);
        thread->SetPreWrbEntrypoint(reinterpret_cast<void *>(pre_wrb_entrypoint));
        return true;
    };
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(callback);

    if (this->GetSettings()->BeforeG1ConcurrentHeapVerification()) {
        trace::ScopedTrace post_heap_verifier_trace("PostGCHeapVeriFier before concurrent");
        size_t fail_count = this->VerifyHeap();
        if (this->GetSettings()->FailOnHeapVerification() && fail_count > 0) {
            LOG(FATAL, GC) << "Heap corrupted after GC, HeapVerifier found " << fail_count << " corruptions";
        }
    }
    ConcurrentMarking(task);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::HaveGarbageRegions()
{
    // Use GetTopGarbageRegions because it doesn't return current regions
    auto regions = GetG1ObjectAllocator()->template GetTopGarbageRegions<false>();
    return HaveGarbageRegions(regions);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::HaveGarbageRegions(const PandaPriorityQueue<std::pair<uint32_t, Region *>> &regions)
{
    if (regions.empty()) {
        return false;
    }
    auto *top_region = regions.top().second;
    double garbage_rate = static_cast<double>(top_region->GetGarbageBytes()) / top_region->Size();
    return garbage_rate >= region_garbage_rate_threshold_;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ProcessDirtyCards()
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    update_remset_thread_->GCProcessCards();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::InitializeImpl()
{
    // GC saved the PandaVM instance, so we get allocator from the PandaVM.
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    this->CreateCardTable(allocator, PoolManager::GetMmapMemPool()->GetMinObjectAddress(),
                          PoolManager::GetMmapMemPool()->GetTotalObjectSize());

    // TODO(dtrubenkov): initialize barriers
    auto barrier_set =
        allocator->New<GCG1BarrierSet>(allocator, &PreWrbFuncEntrypoint, &PostWrbUpdateCardFuncEntrypoint,
                                       panda::helpers::math::GetIntLog2(this->GetG1ObjectAllocator()->GetRegionSize()),
                                       this->GetCardTable(), updated_refs_queue_, &queue_lock_);
    ASSERT(barrier_set != nullptr);
    this->SetGCBarrierSet(barrier_set);

    if (this->IsWorkerThreadsExist()) {
        GCWorkersTaskPool *gc_task_pool = nullptr;
        if (this->GetSettings()->UseThreadPoolForGCWorkers()) {
            // Use internal gc thread pool
            gc_task_pool = allocator->New<GCWorkersThreadPool>(this, this->GetSettings()->GCWorkersCount());
        } else {
            // Use common TaskManager
            gc_task_pool = allocator->New<GCWorkersTaskQueue>(this);
        }
        ASSERT(gc_task_pool != nullptr);
        this->SetWorkersPool(gc_task_pool);
    }
    {
        // to make TSAN happy because we access updated_refs_queue_ inside constructor of UpdateRemsetThread
        os::memory::LockHolder lock(queue_lock_);
        update_remset_thread_ = allocator->template New<UpdateRemsetThread<LanguageConfig>>(
            this, this->GetPandaVm(), updated_refs_queue_, &queue_lock_, this->GetG1ObjectAllocator()->GetRegionSize(),
            this->GetSettings()->G1EnableConcurrentUpdateRemset(), this->GetSettings()->G1MinConcurrentCardsToProcess(),
            this->GetCardTable());
    }
    ASSERT(update_remset_thread_ != nullptr);
    LOG_DEBUG_GC << "G1GC initialized";
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MarkObject(ObjectHeader *object)
{
    G1GCPauseMarker<LanguageConfig>::Mark(object);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::MarkObjectIfNotMarked(ObjectHeader *object)
{
    ASSERT(object != nullptr);
    if (this->GetGCPhase() == GCPhase::GC_PHASE_MARK_YOUNG) {
        return mixed_marker_.MarkIfNotMarked(object);
    }
    return marker_.MarkIfNotMarked(object);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] panda::ObjectHeader *object)
{
    LOG(FATAL, GC) << "Not implemented";
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::IsMarked(panda::ObjectHeader const *object) const
{
    return G1GCPauseMarker<LanguageConfig>::IsMarked(object);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MarkStackMixed(GCMarkingStackType *stack)
{
    ASSERT(stack != nullptr);
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    auto ref_pred = [this](const ObjectHeader *obj) { return InGCSweepRange(obj); };
    auto visitor = [this, stack, &ref_pred](const ObjectHeader *object) {
        ASSERT(mixed_marker_.IsMarked(object));
        ValidateObject(nullptr, object);
        auto *object_class = object->template ClassAddr<BaseClass>();
        // We need annotation here for the FullMemoryBarrier used in InitializeClassByIdEntrypoint
        TSAN_ANNOTATE_HAPPENS_AFTER(object_class);
        LOG_DEBUG_GC << "Current object: " << GetDebugInfoAboutObject(object);

        ASSERT(!object->IsForwarded());
        ASSERT(InGCSweepRange(object));
        CalcLiveBytesMarkPreprocess(object, object_class);
        mixed_marker_.MarkInstance(stack, object, object_class, ref_pred);
    };
    {
        auto marked_objects = stack->TraverseObjects(visitor);
        os::memory::LockHolder lh(mixed_marked_objects_mutex_);
        if (mixed_marked_objects_.empty()) {
            mixed_marked_objects_ = std::move(marked_objects);
        } else {
            mixed_marked_objects_.insert(mixed_marked_objects_.end(), marked_objects.begin(), marked_objects.end());
        }
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MarkStackFull(GCMarkingStackType *stack)
{
    this->MarkStack(&marker_, stack, CalcLiveBytesMarkPreprocess, GC::EmptyReferenceProcessPredicate);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MarkReferences(GCMarkingStackType *references, GCPhase gc_phase)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    LOG_DEBUG_GC << "Start marking " << references->Size() << " references";
    // mark refs only on mixed-gc and on full_gc. On concurrent mark we don't handle any references
    if (gc_phase == GCPhase::GC_PHASE_MARK_YOUNG) {
        MarkStackMixed(references);
    } else if (this->IsFullGC()) {
        MarkStackFull(references);
    } else if (gc_phase == GCPhase::GC_PHASE_INITIAL_MARK || gc_phase == GCPhase::GC_PHASE_MARK ||
               gc_phase == GCPhase::GC_PHASE_REMARK) {
        // nothing
    } else {
        LOG_DEBUG_GC << "phase: " << GCScopedPhase::GetPhaseName(gc_phase);
        UNREACHABLE();
    }
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::InGCSweepRange(const ObjectHeader *object) const
{
    ASSERT_DO(!this->collection_set_.empty() || this->IsFullGC(),
              std::cerr << "Incorrect phase in InGCSweepRange: " << static_cast<size_t>(this->GetGCPhase()) << "\n");
    ASSERT(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(object)));
    Region *obj_region = ObjectToRegion(object);
    return obj_region->IsInCollectionSet();
}

static bool RemsetRegionPredicate(const Region *r)
{
    // In case of mixed GC don't process remsets of the tenured regions which are in the collection set
    return !r->HasFlag(IS_COLLECTION_SET);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunGC(GCTask &task, const CollectionSet &collectible_regions)
{
    ASSERT(!this->IsFullGC());
    GCScope<TRACE_TIMING> scoped_trace(__FUNCTION__, this);
    LOG_DEBUG_GC << "GC start";
    uint64_t young_pause_time;
    {
        // TODO(bwx983476) Measure only those that are on pause
        time::Timer timer(&young_pause_time, true);
        HandlePendingDirtyCards();
        MemRange dirty_cards_range = MixedMarkAndCacheRefs(task, collectible_regions);
        ClearDirtyAndYoungCards(dirty_cards_range);
        CollectAndMove<false>(collectible_regions);
        ReenqueueDirtyCards();
        ClearRefsFromRemsetsCache();
        this->GetObjectGenAllocator()->InvalidateSpaceData();
    }
    if (young_pause_time > 0) {
        this->GetStats()->AddTimeValue(young_pause_time, TimeTypeStats::YOUNG_PAUSED_TIME);
    }
    LOG_DEBUG_GC << "G1GC RunGC end";
}

template <class LanguageConfig>
MemRange G1GC<LanguageConfig>::MixedMarkAndCacheRefs(const GCTask &task, const CollectionSet &collectible_regions)
{
    GCScope<TRACE_TIMING_PHASE> scoped_trace(__FUNCTION__, this, GCPhase::GC_PHASE_MARK_YOUNG);
    bool use_gc_workers = this->GetSettings()->ParallelMarkingEnabled();
    GCMarkingStackType objects_stack(this, use_gc_workers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                     use_gc_workers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                     GCWorkersTaskTypes::TASK_MARKING,
                                     this->GetSettings()->GCMarkingStackNewTasksFrequency());
    for (Region *region : collectible_regions) {
        region->GetMarkBitmap()->ClearAllBits();
        // Calculate live bytes during marking phase
        region->SetLiveBytes(0U);
    }
    ASSERT(this->GetReferenceProcessor()->GetReferenceQueueSize() ==
           0);  // all references should be processed on previous-gc
    // Iterate over roots and add other roots
    // 0. Pre-process refs queue and fill RemSets (should be done later in background)
    // Note: We need to process only tenured -> young refs,
    // since we reach this by graph from tenured roots,
    // because we will process all young regions at young GC we will find all required references
    RefCacheBuilder<LanguageConfig> builder(this, &unique_refs_from_remsets_, region_size_bits_, &objects_stack);
    auto refs_checker = [this, &builder](const MemRange &mem_range, Region *region) {
        IterateOverRefsInMemRange(mem_range, region, builder);
        return builder.AllCrossRegionRefsProcessed();
    };
    MemRange dirty_cards_range = CacheRefsFromRemsets(refs_checker);

    auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    GCRootVisitor gc_mark_collection_set = [&objects_stack, this, &ref_pred](const GCRoot &gc_root) {
        ObjectHeader *root_object = gc_root.GetObjectHeader();
        ObjectHeader *from_object = gc_root.GetFromObjectHeader();
        LOG_DEBUG_GC << "Handle root " << GetDebugInfoAboutObject(root_object) << " from: " << gc_root.GetType();
        if (UNLIKELY(from_object != nullptr) &&
            this->IsReference(from_object->ClassAddr<BaseClass>(), from_object, ref_pred)) {
            LOG_DEBUG_GC << "Add reference: " << GetDebugInfoAboutObject(from_object) << " to stack";
            mixed_marker_.Mark(from_object);
            this->ProcessReference(&objects_stack, from_object->ClassAddr<BaseClass>(), from_object,
                                   GC::EmptyReferenceProcessPredicate);
        } else {
            // Skip non-collection-set roots
            auto root_object_ptr = gc_root.GetObjectHeader();
            ASSERT(root_object_ptr != nullptr);
            if (mixed_marker_.MarkIfNotMarked(root_object_ptr)) {
                ASSERT(this->InGCSweepRange(root_object_ptr));
                LOG_DEBUG_GC << "root " << GetDebugInfoAboutObject(root_object_ptr);
                objects_stack.PushToStack(gc_root.GetType(), root_object_ptr);
            } else {
                LOG_DEBUG_GC << "Skip root for young mark: " << std::hex << root_object_ptr;
            }
        }
    };

    analytics_.ReportMarkingStart(panda::time::GetCurrentTimeInNanos());
    {
        GCScope<TRACE_TIMING> marking_collection_set_roots_trace("Marking roots collection-set", this);

        this->VisitRoots(gc_mark_collection_set, VisitGCRootFlags::ACCESS_ROOT_NONE);
    }
    {
        GCScope<TRACE_TIMING> mark_stack_timing("MarkStack", this);
        this->MarkStackMixed(&objects_stack);
        ASSERT(objects_stack.Empty());
        if (use_gc_workers) {
            this->GetWorkersPool()->WaitUntilTasksEnd();
        }
    }

    auto ref_clear_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    this->GetPandaVm()->HandleReferences(task, ref_clear_pred);

    analytics_.ReportMarkingEnd(panda::time::GetCurrentTimeInNanos());

    // HandleReferences could write a new barriers - so we need to handle them before moving
    ProcessDirtyCards();
    return dirty_cards_range;
}

template <class LanguageConfig>
HeapVerifierIntoGC<LanguageConfig> G1GC<LanguageConfig>::CollectVerificationInfo(const CollectionSet &collection_set)
{
    HeapVerifierIntoGC<LanguageConfig> collect_verifier(this->GetPandaVm()->GetHeapManager());
    if (this->GetSettings()->IntoGCHeapVerification()) {
        ScopedTiming collect_verification_timing(__FUNCTION__, *this->GetTiming());
        PandaVector<MemRange> mem_ranges;
        mem_ranges.reserve(collection_set.size());
        std::for_each(collection_set.begin(), collection_set.end(),
                      [&mem_ranges](const Region *region) { mem_ranges.emplace_back(region->Begin(), region->End()); });
        collect_verifier.CollectVerificationInfo(std::move(mem_ranges));
    }
    return collect_verifier;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::VerifyCollectAndMove(HeapVerifierIntoGC<LanguageConfig> &&collect_verifier,
                                                const CollectionSet &collection_set)
{
    if (this->GetSettings()->IntoGCHeapVerification()) {
        ScopedTiming verification_timing(__FUNCTION__, *this->GetTiming());
        PandaVector<MemRange> alive_mem_range;
        std::for_each(collection_set.begin(), collection_set.end(), [&alive_mem_range](const Region *region) {
            if (region->HasFlag(RegionFlag::IS_PROMOTED)) {
                alive_mem_range.emplace_back(region->Begin(), region->End());
            }
        });
        size_t fails_count = collect_verifier.VerifyAll(std::move(alive_mem_range));
        if (this->GetSettings()->FailOnHeapVerification() && fails_count > 0U) {
            PandaStringStream log_stream;
            log_stream << "Collection set size: " << collection_set.size() << "\n";
            for (const auto r : collection_set) {
                log_stream << *r << (r->HasFlag(RegionFlag::IS_PROMOTED) ? " was promoted\n" : "\n");
            }
            LOG(FATAL, GC) << "Heap was corrupted during CollectAndMove GC phase, HeapVerifier found " << fails_count
                           << " corruptions\n"
                           << log_stream.str();
        }
    }
}

template <class LanguageConfig>
template <bool FULL_GC>
// NOLINTNEXTLINE(readability-function-size)
bool G1GC<LanguageConfig>::CollectAndMove(const CollectionSet &collection_set)
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE);
    LOG_DEBUG_GC << "== G1GC CollectAndMove start ==";
    auto internal_allocator = this->GetInternalAllocator();
    bool use_gc_workers = this->GetSettings()->ParallelCompactingEnabled();

    PandaVector<PandaVector<ObjectHeader *> *> moved_objects_vector;
    HeapVerifierIntoGC<LanguageConfig> collect_verifier = this->CollectVerificationInfo(collection_set);
    {
        GCScope<TRACE_TIMING> compact_regions("CompactRegions", this);
        analytics_.ReportEvacuationStart(panda::time::GetCurrentTimeInNanos());
        if constexpr (FULL_GC) {
            if (!use_gc_workers) {
                auto vector = internal_allocator->template New<PandaVector<ObjectHeader *>>();
                moved_objects_vector.push_back(vector);
            }
        }
        for (auto r : collection_set.Young()) {
            this->DoRegionCompacting<RegionFlag::IS_EDEN, FULL_GC>(r, use_gc_workers, &moved_objects_vector);
        }
        for (auto r : collection_set.Tenured()) {
            this->DoRegionCompacting<RegionFlag::IS_OLD, FULL_GC>(r, use_gc_workers, &moved_objects_vector);
        }

        if (use_gc_workers) {
            this->GetWorkersPool()->WaitUntilTasksEnd();
        }

        analytics_.ReportEvacuationEnd(panda::time::GetCurrentTimeInNanos());
    }

    MovedObjectsContainer<FULL_GC> *moved_objects_container = nullptr;
    if constexpr (FULL_GC) {
        moved_objects_container = &moved_objects_vector;
    } else {
        moved_objects_container = &mixed_marked_objects_;
    }

    {
        os::memory::LockHolder lock(queue_lock_);
        analytics_.ReportUpdateRefsStart(panda::time::GetCurrentTimeInNanos());
        if (this->GetSettings()->ParallelRefUpdatingEnabled()) {
            UpdateRefsToMovedObjects<FULL_GC, true>(moved_objects_container);
        } else {
            UpdateRefsToMovedObjects<FULL_GC, false>(moved_objects_container);
        }
        analytics_.ReportUpdateRefsEnd(panda::time::GetCurrentTimeInNanos());
        ActualizeRemSets();
    }

    VerifyCollectAndMove(std::move(collect_verifier), collection_set);
    SweepRegularVmRefs();

    auto object_allocator = this->GetG1ObjectAllocator();
    if (!collection_set.Young().empty()) {
        object_allocator->ResetYoungAllocator();
    }
    {
        GCScope<TRACE_TIMING> reset_regions("ResetRegions", this);
        if (!this->IsFullGC()) {
            object_allocator->template ResetRegions<RegionFlag::IS_OLD, RegionSpace::ReleaseRegionsPolicy::NoRelease,
                                                    OSPagesPolicy::IMMEDIATE_RETURN, false>(collection_set.Tenured());
        } else {
            object_allocator->template ResetRegions<RegionFlag::IS_OLD, RegionSpace::ReleaseRegionsPolicy::Release,
                                                    OSPagesPolicy::NO_RETURN, false>(collection_set.Tenured());
        }
    }
    {
        // Don't forget to delete all temporary elements
        GCScope<TRACE_TIMING> clear_moved_objects("ClearMovedObjects", this);
        if constexpr (FULL_GC) {
            if (use_gc_workers) {
                for (auto r : moved_objects_vector) {
                    internal_allocator->Delete(r);
                }
            } else {
                ASSERT(moved_objects_vector.size() == 1);
                internal_allocator->Delete(moved_objects_vector.back());
            }
        } else {
            for (auto r : mixed_marked_objects_) {
                internal_allocator->Delete(r);
            }
            mixed_marked_objects_.clear();
        }
    }

    LOG_DEBUG_GC << "== G1GC CollectAndMove end ==";
    return true;
}

template <class LanguageConfig>
template <bool FULL_GC, bool NEED_LOCK>
std::conditional_t<FULL_GC, UpdateRemsetRefUpdater<LanguageConfig, NEED_LOCK>, EnqueueRemsetRefUpdater<LanguageConfig>>
G1GC<LanguageConfig>::CreateRefUpdater([[maybe_unused]] GCG1BarrierSet::ThreadLocalCardQueues *updated_ref_queue) const
{
    if constexpr (FULL_GC) {
        return UpdateRemsetRefUpdater<LanguageConfig, NEED_LOCK>(region_size_bits_);
    } else {
        return EnqueueRemsetRefUpdater<LanguageConfig>(this->GetCardTable(), updated_ref_queue, region_size_bits_);
    }
}

template <class LanguageConfig>
template <bool FULL_GC, bool USE_WORKERS>
void G1GC<LanguageConfig>::UpdateRefsToMovedObjects(MovedObjectsContainer<FULL_GC> *moved_objects_container)
{
    GCScope<TRACE_TIMING> scope(__FUNCTION__, this);
    // Currently lock for RemSet too much influences for pause, so don't use workers on FULL-GC
    constexpr bool ENABLE_WORKERS = USE_WORKERS && !FULL_GC;
    auto internal_allocator = this->GetInternalAllocator();
    auto *updated_ref_queue = (ENABLE_WORKERS)
                                  ? internal_allocator->template New<GCG1BarrierSet::ThreadLocalCardQueues>()
                                  : updated_refs_queue_;
    auto ref_updater = this->CreateRefUpdater<FULL_GC, /* NEED_LOCK */ ENABLE_WORKERS>(updated_ref_queue);
    //  update reference from objects which were moved while garbage collection
    LOG_DEBUG_GC << "=== Update ex-cset -> ex-cset references. START. ===";
    {
        ScopedTiming t("UpdateMovedObjectsReferences", *this->GetTiming());
        for (auto *moved_objects : *moved_objects_container) {
            if constexpr (ENABLE_WORKERS) {
                auto range_begin = moved_objects->begin();
                auto range_end = range_begin;
                while (range_begin != moved_objects->end()) {
                    if (std::distance(range_begin, moved_objects->end()) < GCUpdateRefsWorkersTask<false>::RANGE_SIZE) {
                        range_end = moved_objects->end();
                    } else {
                        std::advance(range_end, GCUpdateRefsWorkersTask<false>::RANGE_SIZE);
                    }
                    auto *moved_objects_range =
                        internal_allocator->template New<typename GCUpdateRefsWorkersTask<false>::MovedObjectsRange>(
                            range_begin, range_end);
                    range_begin = range_end;
                    GCUpdateRefsWorkersTask<false> gc_worker_task(moved_objects_range);
                    if (this->GetWorkersPool()->AddTask(GCUpdateRefsWorkersTask<false>(gc_worker_task))) {
                        continue;
                    }
                    // Couldn't add new task, so do task processing immediately
                    this->WorkerTaskProcessing(&gc_worker_task, nullptr);
                }
            } else {  // GC workers are not used
                typename GCUpdateRefsWorkersTask<FULL_GC>::MovedObjectsRange moved_objects_range(moved_objects->begin(),
                                                                                                 moved_objects->end());
                DoUpdateReferencesToMovedObjectsRange<LanguageConfig, decltype(ref_updater), FULL_GC>(
                    &moved_objects_range, ref_updater);
            }
        }
    }
    LOG_DEBUG_GC << "=== Update ex-cset -> ex-cset references. END. ===";

    // update references from objects which are not part of collection set
    LOG_DEBUG_GC << "=== Update non ex-cset -> ex-cset references. START. ===";
    if constexpr (FULL_GC) {
        UpdateRefsFromRemSets(ref_updater);
    } else {
        VisitRemSets(ref_updater);
    }
    LOG_DEBUG_GC << "=== Update non ex-cset -> ex-cset references. END. ===";
    if constexpr (ENABLE_WORKERS) {
        {
            os::memory::LockHolder lock(gc_worker_queue_lock_);
            updated_refs_queue_->insert(updated_refs_queue_->end(), updated_ref_queue->begin(),
                                        updated_ref_queue->end());
            this->GetInternalAllocator()->Delete(updated_ref_queue);
        }
        this->GetWorkersPool()->WaitUntilTasksEnd();
    }
    this->CommonUpdateRefsToMovedObjects();
}

template <class LanguageConfig>
NO_THREAD_SAFETY_ANALYSIS void G1GC<LanguageConfig>::OnPauseMark(GCTask &task, GCMarkingStackType *objects_stack,
                                                                 bool use_gc_workers)
{
    GCScope<TRACE_TIMING> scope(__FUNCTION__, this);
    LOG_DEBUG_GC << "OnPause marking started";
    auto *object_allocator = GetG1ObjectAllocator();
    this->MarkImpl(
        &marker_, objects_stack, CardTableVisitFlag::VISIT_DISABLED,
        // process references on FULL-GC
        GC::EmptyReferenceProcessPredicate,
        // non-young mem-range checker
        [object_allocator](MemRange &mem_range) { return !object_allocator->IsIntersectedWithYoung(mem_range); },
        // mark predicate
        CalcLiveBytesMarkPreprocess);
    if (use_gc_workers) {
        this->GetWorkersPool()->WaitUntilTasksEnd();
    }
    /**
     * We don't collect non-movable regions right now, if there was a reference from non-movable to
     * young/tenured region then we reset markbitmap for non-nonmovable, but don't update livebitmap and we
     * can traverse over non-reachable object (in CacheRefsFromRemsets) and visit DEAD object in
     * tenured space (was delete on young-collection or in Iterative-full-gc phase.
     */
    auto ref_clear_pred = []([[maybe_unused]] const ObjectHeader *obj) { return true; };
    this->GetPandaVm()->HandleReferences(task, ref_clear_pred);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::FullMarking(panda::GCTask &task)
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    auto *object_allocator = GetG1ObjectAllocator();
    bool use_gc_workers = this->GetSettings()->ParallelMarkingEnabled();

    GCMarkingStackType full_collection_stack(
        this, use_gc_workers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
        use_gc_workers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0, GCWorkersTaskTypes::TASK_FULL_MARK,
        this->GetSettings()->GCMarkingStackNewTasksFrequency());

    InitialMark(full_collection_stack);

    this->OnPauseMark(task, &full_collection_stack, use_gc_workers);
    // We will sweep VM refs in tenured space during mixed collection, but only for non empty regions.
    // therefore, sweep it here only for NonMovable, Humongous objects, and empty movable regions:
    SweepNonRegularVmRefs();
    auto all_regions = object_allocator->GetAllRegions();
    for (auto *r : all_regions) {
        if (r->GetLiveBitmap() != nullptr) {
            r->CloneMarkBitmapToLiveBitmap();
        }
    }
    // Force card updater here, after swapping bitmap, to skip dead objects
    ProcessDirtyCards();
    auto garbage_regions = GetG1ObjectAllocator()->template GetTopGarbageRegions<false>();
    auto empty_tenured_regions = GetEmptyTenuredRegularRegionsFromQueue(std::move(garbage_regions));
    CollectEmptyRegions<false, false>(task, &empty_tenured_regions);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ConcurrentMarking(panda::GCTask &task)
{
    {
        PauseTimeGoalDelay();
        // initial mark is always separate pause with pause-time-goal GC trigger
        auto same_pause = !this->GetSettings()->G1EnablePauseTimeGoal();
        auto scoped_tracker = g1_pause_tracker_.CreateScope();
        GCScopedPauseStats scoped_pause_stats(this->GetPandaVm()->GetGCStats(), nullptr,
                                              same_pause ? PauseTypeStats::COMMON_PAUSE
                                                         : PauseTypeStats::INITIAL_MARK_PAUSE);
        InitialMark(concurrent_marking_stack_);
    }

    LOG_DEBUG_GC << "Concurrent marking started";
    ConcurrentMark(&concurrent_marking_stack_);
    PauseTimeGoalDelay();
    // weak refs shouldn't be added to the queue on concurrent-mark
    ASSERT(this->GetReferenceProcessor()->GetReferenceQueueSize() == 0);

    auto set_entrypoint = [](ManagedThread *thread) {
        ASSERT(thread->GetPreWrbEntrypoint() != nullptr);
        thread->SetPreWrbEntrypoint(nullptr);
        return true;
    };
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(set_entrypoint);
    concurrent_marking_flag_ = false;
    if (!interrupt_concurrent_flag_) {
        Remark(task);
        // Enable mixed GC
        auto garbage_regions = GetG1ObjectAllocator()->template GetTopGarbageRegions<false>();
        if (HaveGarbageRegions(garbage_regions)) {
            // Atomic with release order reason: to see changes made by GC thread (which do concurrent marking
            // and than set is_mixed_gc_required_) in mutator thread which waits for the end of concurrent
            // marking.
            is_mixed_gc_required_.store(true, std::memory_order_release);
        }

        {
            ScopedTiming t("Concurrent Sweep", *this->GetTiming());
            ConcurrentScope concurrent_scope(this);
            auto empty_tenured_regions = GetEmptyTenuredRegularRegionsFromQueue(std::move(garbage_regions));
            if (this->IsConcurrencyAllowed()) {
                CollectEmptyRegions<true, true>(task, &empty_tenured_regions);
            } else {
                CollectEmptyRegions<false, false>(task, &empty_tenured_regions);
            }
        }
    } else {
        concurrent_marking_stack_.Clear();
        ClearSatb();
    }
    ASSERT(concurrent_marking_stack_.Empty());
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::PauseTimeGoalDelay()
{
    if (this->GetSettings()->G1EnablePauseTimeGoal() && !interrupt_concurrent_flag_) {
        auto start = panda::time::GetCurrentTimeInMicros();
        // Instead of max pause it should be estimated to calculate delay
        auto remained = g1_pause_tracker_.MinDelayBeforeMaxPauseInMicros(panda::time::GetCurrentTimeInMicros());
        if (remained > 0) {
            ConcurrentScope concurrent_scope(this);
            os::memory::LockHolder lh(concurrent_mark_mutex_);
            while (!interrupt_concurrent_flag_ && remained > 0) {
                auto ms = remained / panda::os::time::MILLIS_TO_MICRO;
                auto ns = (remained - ms * panda::os::time::MILLIS_TO_MICRO) * panda::os::time::MICRO_TO_NANO;
                concurrent_mark_cond_var_.TimedWait(&concurrent_mark_mutex_, ms, ns);
                remained -= panda::time::GetCurrentTimeInMicros() - start;
            }
        }
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::InitialMark(GCMarkingStackType &marking_stack)
{
    {
        // First we need to unmark all heap
        GCScope<TRACE_TIMING> un_mark_scope("UnMark", this);
        LOG_DEBUG_GC << "Start unmark all heap before mark";
        auto all_region = GetG1ObjectAllocator()->GetAllRegions();
        for (Region *r : all_region) {
            auto *bitmap = r->GetMarkBitmap();
            // Calculate live bytes during mark-phase
            r->SetLiveBytes(0U);
            // unmark full-heap except Humongous-space
            bitmap->ClearAllBits();
        }
#ifndef NDEBUG
        this->GetObjectAllocator()->IterateOverObjects(
            [this](ObjectHeader *obj) { ASSERT(!this->marker_.IsMarked(obj)); });
#endif
    }
    ASSERT(this->GetReferenceProcessor()->GetReferenceQueueSize() ==
           0);  // all references should be processed on mixed-gc
    {
        GCScope<TRACE_TIMING_PHASE> initial_mark_scope("InitialMark", this, GCPhase::GC_PHASE_INITIAL_MARK);
        // Collect non-heap roots.
        // Mark the whole heap by using only these roots.
        // The interregion roots will be processed at pause

        // InitialMark. STW
        GCRootVisitor gc_mark_roots = [this, &marking_stack](const GCRoot &gc_root) {
            ValidateObject(gc_root.GetType(), gc_root.GetObjectHeader());
            if (marker_.MarkIfNotMarked(gc_root.GetObjectHeader())) {
                marking_stack.PushToStack(gc_root.GetType(), gc_root.GetObjectHeader());
            }
        };
        this->VisitRoots(gc_mark_roots, VisitGCRootFlags::ACCESS_ROOT_ALL);
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ConcurrentMark(GCMarkingStackType *objects_stack)
{
    ConcurrentScope concurrent_scope(this);
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    this->ConcurentMarkImpl(objects_stack);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::Remark(panda::GCTask const &task)
{
    /**
     * Make remark on pause to have all marked objects in tenured space, it gives possibility to check objects in
     * remsets. If they are not marked - we don't process this object, because it's dead already
     */
    auto scoped_tracker = g1_pause_tracker_.CreateScope();
    GCScope<TIMING_PHASE> gc_scope(__FUNCTION__, this, GCPhase::GC_PHASE_REMARK);
    GCScopedPauseStats scoped_pause_stats(this->GetPandaVm()->GetGCStats(), nullptr, PauseTypeStats::REMARK_PAUSE);
    {
        ScopedTiming t("Stack Remarking", *this->GetTiming());
        bool use_gc_workers = this->GetSettings()->ParallelMarkingEnabled();
        GCMarkingStackType stack(this, use_gc_workers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                 use_gc_workers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                 GCWorkersTaskTypes::TASK_REMARK,
                                 this->GetSettings()->GCMarkingStackNewTasksFrequency());

        // The mutator may create new regions.
        // If so we should bind bitmaps of new regions.
        DrainSatb(&stack);
        this->MarkStack(&marker_, &stack, CalcLiveBytesMarkPreprocess);

        if (use_gc_workers) {
            this->GetWorkersPool()->WaitUntilTasksEnd();
        }

        // ConcurrentMark doesn't visit young objects - so we can't clear references which are in young-space because we
        // don't know which objects are marked. We will process them on young/mixed GC separately later, here we process
        // only refs in tenured-space
        auto ref_clear_pred = []([[maybe_unused]] const ObjectHeader *obj) {
            return !ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN);
        };
        this->GetPandaVm()->HandleReferences(task, ref_clear_pred);
    }

    // We will sweep VM refs in tenured space during mixed collection,
    // therefore, sweep it here only for NonMovable and Humongous objects:
    SweepNonRegularVmRefs();
    auto g1_allocator = this->GetG1ObjectAllocator();
    auto all_regions = g1_allocator->GetAllRegions();
    for (const auto &region : all_regions) {
        // TODO(alovkov): set IS_OLD for NON_MOVABLE region when we create it
        if (region->HasFlag(IS_OLD) || region->HasFlag(IS_NONMOVABLE)) {
            region->SwapMarkBitmap();
        }
    }
    // Force card updater here, after swapping bitmap, to skip dead objects
    ProcessDirtyCards();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::SweepNonRegularVmRefs()
{
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());

    this->GetPandaVm()->SweepVmRefs([this](ObjectHeader *object) {
        Region *region = ObjectToRegion(object);
        if (region->HasFlag(RegionFlag::IS_EDEN)) {
            return ObjectStatus::ALIVE_OBJECT;
        }
        bool non_regular_object =
            region->HasFlag(RegionFlag::IS_NONMOVABLE) || region->HasFlag(RegionFlag::IS_LARGE_OBJECT);
        if (!non_regular_object) {
            ASSERT(region->GetLiveBytes() != 0U || !this->IsMarked(object));
            return region->GetLiveBytes() == 0U ? ObjectStatus::DEAD_OBJECT : ObjectStatus::ALIVE_OBJECT;
        }
        return this->IsMarked(object) ? ObjectStatus::ALIVE_OBJECT : ObjectStatus::DEAD_OBJECT;
    });
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::SweepRegularVmRefs()
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());

    this->GetPandaVm()->SweepVmRefs([this](ObjectHeader *obj) {
        if (this->InGCSweepRange(obj)) {
            return ObjectStatus::DEAD_OBJECT;
        }
        return ObjectStatus::ALIVE_OBJECT;
    });
}

template <class LanguageConfig>
CollectionSet G1GC<LanguageConfig>::GetCollectibleRegions(panda::GCTask const &task, bool is_mixed)
{
    ASSERT(!this->IsFullGC());
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());
    auto g1_allocator = this->GetG1ObjectAllocator();
    LOG_DEBUG_GC << "Start GetCollectibleRegions is_mixed: " << is_mixed << " reason: " << task.reason;
    CollectionSet collection_set(g1_allocator->GetYoungRegions());
    if (is_mixed) {
        if (!this->GetSettings()->G1EnablePauseTimeGoal()) {
            AddOldRegionsMaxAllowed(collection_set);
        } else {
            AddOldRegionsAccordingPauseTimeGoal(collection_set);
        }
    }
    LOG_DEBUG_GC << "collectible_regions size: " << collection_set.size() << " young " << collection_set.Young().size()
                 << " old " << std::distance(collection_set.Young().end(), collection_set.end())
                 << " reason: " << task.reason << " is_mixed: " << is_mixed;
    return collection_set;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::AddOldRegionsMaxAllowed(CollectionSet &collection_set)
{
    auto regions = this->GetG1ObjectAllocator()->template GetTopGarbageRegions<false>();
    for (size_t i = 0; i < number_of_mixed_tenured_regions_ && !regions.empty(); i++) {
        auto *garbage_region = regions.top().second;
        regions.pop();
        ASSERT(!garbage_region->HasFlag(IS_EDEN));
        ASSERT(!garbage_region->HasPinnedObjects());
        ASSERT(!garbage_region->HasFlag(IS_RESERVED));
        ASSERT(garbage_region->GetAllocatedBytes() != 0U);
        double garbage_rate =
            static_cast<double>(garbage_region->GetGarbageBytes()) / garbage_region->GetAllocatedBytes();
        if (garbage_rate >= region_garbage_rate_threshold_) {
            LOG_DEBUG_GC << "Garbage percentage in " << std::hex << garbage_region << " region = " << std::dec
                         << garbage_rate << " %, add to collection set";
            collection_set.AddRegion(garbage_region);
        } else {
            LOG_DEBUG_GC << "Garbage percentage in " << std::hex << garbage_region << " region = " << std::dec
                         << garbage_rate << " %, don't add to collection set";
            break;
        }
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::AddOldRegionsAccordingPauseTimeGoal(CollectionSet &collection_set)
{
    auto gc_pause_time_budget =
        static_cast<int64_t>(this->GetSettings()->GetG1MaxGcPauseInMillis() * panda::os::time::MILLIS_TO_MICRO);
    auto regions = this->GetG1ObjectAllocator()->template GetTopGarbageRegions<false>();
    // add at least one old region to guarantee a progress in mixed collection
    auto *top_region = regions.top().second;
    collection_set.AddRegion(top_region);
    auto expected_top_region_collection_time = analytics_.PredictOldCollectionTimeInMicros(top_region);
    if (gc_pause_time_budget < expected_top_region_collection_time) {
        LOG_DEBUG_GC << "Not enough budget to add more than one old region";
        return;
    }
    gc_pause_time_budget -= expected_top_region_collection_time;
    auto prediction_error = analytics_.EstimatePredictionErrorInMicros();
    if (gc_pause_time_budget < prediction_error) {
        LOG_DEBUG_GC << "Not enough budget to add old regions";
        return;
    }
    gc_pause_time_budget -= prediction_error;
    auto expected_young_collection_time = analytics_.PredictYoungCollectionTimeInMicros(collection_set.Young().size());
    if (gc_pause_time_budget < expected_young_collection_time) {
        LOG_DEBUG_GC << "Not enough budget to add old regions";
        return;
    }
    gc_pause_time_budget -= expected_young_collection_time;

    regions.pop();
    while (!regions.empty()) {
        auto &score_and_region = regions.top();
        auto *garbage_region = score_and_region.second;
        ASSERT(!garbage_region->HasFlag(IS_EDEN));
        ASSERT(!garbage_region->HasPinnedObjects());
        ASSERT(!garbage_region->HasFlag(IS_RESERVED));
        ASSERT(garbage_region->GetAllocatedBytes() != 0U);

        regions.pop();

        double garbage_rate =
            static_cast<double>(garbage_region->GetGarbageBytes()) / garbage_region->GetAllocatedBytes();
        if (garbage_rate < region_garbage_rate_threshold_) {
            LOG_DEBUG_GC << "Garbage percentage in " << std::hex << garbage_region << " region = " << std::dec
                         << garbage_rate << " %, don't add to collection set";
            break;
        }

        auto expected_region_collection_time = analytics_.PredictOldCollectionTimeInMicros(garbage_region);

        if (gc_pause_time_budget < expected_region_collection_time) {
            LOG_DEBUG_GC << "Not enough budget to add old regions anymore";
            break;
        }

        gc_pause_time_budget -= expected_region_collection_time;

        LOG_DEBUG_GC << "Garbage percentage in " << std::hex << garbage_region << " region = " << std::dec
                     << garbage_rate << " %, add to collection set";
        collection_set.AddRegion(garbage_region);
    }
}

template <class LanguageConfig>
CollectionSet G1GC<LanguageConfig>::GetFullCollectionSet()
{
    ASSERT(this->IsFullGC());
    // FillRemSet should be always finished before GetCollectibleRegions
    ASSERT(update_remset_thread_->GetQueueSize() == 0);
    auto g1_allocator = this->GetG1ObjectAllocator();
    g1_allocator->ClearCurrentTenuredRegion();
    CollectionSet collection_set(g1_allocator->GetYoungRegions());
    auto movable_garbage_regions = g1_allocator->template GetTopGarbageRegions<true>();
    LOG_DEBUG_GC << "Regions for FullGC:";
    while (!movable_garbage_regions.empty()) {
        auto *region = movable_garbage_regions.top().second;
        movable_garbage_regions.pop();
        if (region->HasFlag(IS_EDEN) || region->HasPinnedObjects()) {
            LOG_DEBUG_GC << (region->HasFlags(IS_EDEN) ? "Young regions" : "Region with pinned objects") << " ("
                         << *region << ") is not added to collection set";
            continue;
        }
        LOG_DEBUG_GC << *region;
        ASSERT(!region->HasFlag(IS_NONMOVABLE) && !region->HasFlag(IS_LARGE_OBJECT));
        ASSERT(region->HasFlag(IS_OLD));
        collection_set.AddRegion(region);
    }
    return collection_set;
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::HaveEnoughSpaceToMove(const CollectionSet &collectible_regions)
{
    return HaveEnoughRegionsToMove(collectible_regions.Movable().size());
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::HaveEnoughRegionsToMove(size_t num)
{
    return GetG1ObjectAllocator()->HaveTenuredSize(num) && GetG1ObjectAllocator()->HaveFreeRegions(num);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::OnThreadTerminate(ManagedThread *thread, mem::BuffersKeepingFlag keep_buffers)
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    // The method must be called while the lock which guards thread/coroutine list is hold
    LOG(DEBUG, GC) << "Call OnThreadTerminate";
    PandaVector<ObjectHeader *> *pre_buff = nullptr;
    if (keep_buffers == mem::BuffersKeepingFlag::KEEP) {
        pre_buff = allocator->New<PandaVector<ObjectHeader *>>(*thread->GetPreBuff());
        thread->GetPreBuff()->clear();
    } else {  // keep_buffers == mem::BuffersKeepingFlag::DELETE
        pre_buff = thread->MovePreBuff();
    }
    ASSERT(pre_buff != nullptr);
    {
        os::memory::LockHolder lock(satb_and_newobj_buf_lock_);
        satb_buff_list_.push_back(pre_buff);
    }
    {
        auto *local_buffer = thread->GetG1PostBarrierBuffer();
        ASSERT(local_buffer != nullptr);
        if (!local_buffer->IsEmpty()) {
            auto *temp_buffer = allocator->New<PandaVector<mem::CardTable::CardPtr>>();
            while (!local_buffer->IsEmpty()) {
                temp_buffer->push_back(local_buffer->Pop());
            }
            update_remset_thread_->AddPostBarrierBuffer(temp_buffer);
        }
        if (keep_buffers == mem::BuffersKeepingFlag::DELETE) {
            thread->ResetG1PostBarrierBuffer();
            allocator->Delete(local_buffer);
        }
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::PreZygoteFork()
{
    GC::PreZygoteFork();
    if (this->GetWorkersPool() != nullptr) {
        auto allocator = this->GetInternalAllocator();
        allocator->Delete(this->GetWorkersPool());
        this->ClearWorkersPool();
    }
    this->DisableWorkerThreads();
    update_remset_thread_->DestroyThread();
    // don't use thread while we are in zygote
    update_remset_thread_->SetUpdateConcurrent(false);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::PostZygoteFork()
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    this->EnableWorkerThreads();
    if (this->IsWorkerThreadsExist()) {
        GCWorkersTaskPool *gc_task_pool = nullptr;
        if (this->GetSettings()->UseThreadPoolForGCWorkers()) {
            // Use internal gc thread pool
            gc_task_pool = allocator->New<GCWorkersThreadPool>(this, this->GetSettings()->GCWorkersCount());
        } else {
            // Use common TaskManager
            gc_task_pool = allocator->New<GCWorkersTaskQueue>(this);
        }
        ASSERT(gc_task_pool != nullptr);
        this->SetWorkersPool(gc_task_pool);
    }
    GC::PostZygoteFork();
    // use concurrent-option after zygote
    update_remset_thread_->SetUpdateConcurrent(this->GetSettings()->G1EnableConcurrentUpdateRemset());
    update_remset_thread_->CreateThread(allocator);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::DrainSatb(GCAdaptiveStack *object_stack)
{
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());
    // Process satb buffers of the active threads
    auto callback = [this, object_stack](ManagedThread *thread) {
        // Acquire lock here to avoid data races with the threads
        // which are terminating now.
        // Data race is happens in thread.pre_buf_. The terminating thread may
        // release own pre_buf_ while GC thread iterates over threads and gets theirs
        // pre_buf_.
        os::memory::LockHolder lock(satb_and_newobj_buf_lock_);
        auto pre_buff = thread->GetPreBuff();
        if (pre_buff == nullptr) {
            // This can happens when the thread gives us own satb_buffer but
            // doesn't unregister from ThreadManaged.
            // At this perion GC can happen and we get pre_buff null here.
            return true;
        }
        for (auto obj : *pre_buff) {
            if (marker_.MarkIfNotMarked(obj)) {
                object_stack->PushToStack(RootType::SATB_BUFFER, obj);
            }
        }
        pre_buff->clear();
        return true;
    };
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(callback);

    // Process satb buffers of the terminated threads
    os::memory::LockHolder lock(satb_and_newobj_buf_lock_);
    for (auto obj_vector : satb_buff_list_) {
        ASSERT(obj_vector != nullptr);
        for (auto obj : *obj_vector) {
            if (marker_.MarkIfNotMarked(obj)) {
                object_stack->PushToStack(RootType::SATB_BUFFER, obj);
            }
        }
        this->GetInternalAllocator()->Delete(obj_vector);
    }
    satb_buff_list_.clear();
    for (auto obj : newobj_buffer_) {
        if (marker_.MarkIfNotMarked(obj)) {
            object_stack->PushToStack(RootType::SATB_BUFFER, obj);
        }
    }
    newobj_buffer_.clear();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::WaitForUpdateRemsetThread()
{
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());
    LOG_DEBUG_GC << "Execute WaitForUpdateRemsetThread";

    // we forced to call EnumerateThreads many times because it takes WriteLock and we block update_remset_thread
    // can be done only once if EnumerateThreads will be implemented via ReadLock
    update_remset_thread_->WaitUntilTasksEnd();

#ifndef NDEBUG
    bool all_post_barrier_buffers_empty = true;
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(
        [&all_post_barrier_buffers_empty](ManagedThread *thread) {
            auto local_buffer = thread->GetG1PostBarrierBuffer();
            if (local_buffer != nullptr && !local_buffer->IsEmpty()) {
                all_post_barrier_buffers_empty = false;
                return false;
            }
            return true;
        });
    ASSERT(all_post_barrier_buffers_empty);
#endif
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::HandlePendingDirtyCards()
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    update_remset_thread_->DrainAllCards(&dirty_cards_);
    std::for_each(dirty_cards_.cbegin(), dirty_cards_.cend(), [](auto card) { card->Clear(); });
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ReenqueueDirtyCards()
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    os::memory::LockHolder lock(queue_lock_);
    std::for_each(dirty_cards_.cbegin(), dirty_cards_.cend(), [this](auto card) {
        card->Mark();
        updated_refs_queue_->push_back(card);
    });
    dirty_cards_.clear();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ClearSatb()
{
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());
    // Acquire lock here to avoid data races with the threads
    // which are terminating now.
    // Data race is happens in thread.pre_buf_. The terminating thread may
    // release own pre_buf_ while GC thread iterates over threads and gets theirs
    // pre_buf_.
    os::memory::LockHolder lock(satb_and_newobj_buf_lock_);
    // Process satb buffers of the active threads
    auto thread_callback = [](ManagedThread *thread) {
        auto pre_buff = thread->GetPreBuff();
        if (pre_buff != nullptr) {
            pre_buff->clear();
        }
        return true;
    };
    Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(thread_callback);

    // Process satb buffers of the terminated threads
    for (auto obj_vector : satb_buff_list_) {
        this->GetInternalAllocator()->Delete(obj_vector);
    }
    satb_buff_list_.clear();
    newobj_buffer_.clear();
}

template <class LanguageConfig>
template <class Visitor>
void G1GC<LanguageConfig>::VisitRemSets(const Visitor &visitor)
{
    GCScope<TRACE_TIMING> visit_remset_scope(__FUNCTION__, this);

    ASSERT(unique_cards_initialized_);
    // Iterate over stored references to the collection set
    for (auto &entry_vector : unique_refs_from_remsets_) {
        for (auto &entry : *entry_vector) {
            ObjectHeader *object = entry.GetObject();
            uint32_t offset = entry.GetReferenceOffset();
            visitor(object, ObjectAccessor::GetObject(object, offset), offset);
        }
    }
}

template <class LanguageConfig>
template <class Visitor>
void G1GC<LanguageConfig>::UpdateRefsFromRemSets(const Visitor &visitor)
{
    auto field_visitor = [this, &visitor](ObjectHeader *object, ObjectHeader *field, uint32_t offset,
                                          [[maybe_unused]] bool is_volatile) {
        if (!InGCSweepRange(field)) {
            return true;
        }
        visitor(object, ObjectAccessor::GetObject(object, offset), offset);
        return true;
    };
    auto refs_checker = [this, &field_visitor](const MemRange &mem_range, Region *region) {
        IterateOverRefsInMemRange(mem_range, region, field_visitor);
        return true;
    };
    MemRange dirty_cards = CacheRefsFromRemsets(refs_checker);
    ClearDirtyAndYoungCards(dirty_cards);
}

template <class LanguageConfig>
MemRange G1GC<LanguageConfig>::CacheRefsFromRemsets(const MemRangeRefsChecker &refs_checker)
{
    GCScope<TRACE_TIMING> cache_refs_from_remset_scope(__FUNCTION__, this);
    // Collect only unique objects to not proceed them more than once.
    ASSERT(!unique_cards_initialized_);
    CardTable *card_table = this->GetCardTable();
    uintptr_t min_dirty_addr = card_table->GetMinAddress() + card_table->GetCardsCount() * card_table->GetCardSize();
    uintptr_t max_dirty_addr = card_table->GetMinAddress();

    ASSERT(IsCardTableClear(card_table));
    auto visitor = [card_table, &min_dirty_addr, &max_dirty_addr, &refs_checker](Region *r, const MemRange &range) {
        // Use the card table to mark the ranges we already processed.
        // Each card is uint8_t. Use it as a bitmap. Set bit means the corresponding memory
        // range is processed.
        CardTable::CardPtr card = card_table->GetCardPtr(range.GetStartAddress());
        uintptr_t card_addr = card_table->GetCardStartAddress(card);
        size_t mem_size = DEFAULT_REGION_SIZE / RemSet<>::Bitmap::GetNumBits();
        size_t bit_idx = (range.GetStartAddress() - card_addr) / mem_size;
        if ((card->GetCard() & (1U << bit_idx)) == 0) {
            card->SetCard(card->GetCard() | (1U << bit_idx));
            if (min_dirty_addr > card_addr) {
                min_dirty_addr = card_addr;
            }
            if (max_dirty_addr < card_addr + card_table->GetCardSize()) {
                max_dirty_addr = card_addr + card_table->GetCardSize();
            }
            return refs_checker(range, r);
        }
        // some cross region refs might be not processed
        return false;
    };
    analytics_.ReportScanRemsetStart(panda::time::GetCurrentTimeInNanos());
    for (auto region : collection_set_) {
        region->GetRemSet()->Iterate(RemsetRegionPredicate, visitor);
    }
    analytics_.ReportScanRemsetEnd(panda::time::GetCurrentTimeInNanos());

    if (!this->IsFullGC()) {
        CacheRefsFromDirtyCards(visitor);
#ifndef NDEBUG
        unique_cards_initialized_ = true;
#endif  // NDEBUG
    }
    if (min_dirty_addr > max_dirty_addr) {
        min_dirty_addr = max_dirty_addr;
    }
    return MemRange(min_dirty_addr, max_dirty_addr);
}

template <class LanguageConfig>
template <typename Visitor>
void G1GC<LanguageConfig>::CacheRefsFromDirtyCards(Visitor visitor)
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    auto card_table = this->GetCardTable();
    constexpr size_t MEM_SIZE = DEFAULT_REGION_SIZE / RemSet<>::Bitmap::GetNumBits();
    for (auto it = dirty_cards_.cbegin(); it != dirty_cards_.cend();) {
        auto range = card_table->GetMemoryRange(*it);
        auto addr = range.GetStartAddress();
        ASSERT_DO(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(ToVoidPtr(addr))),
                  std::cerr << "Invalid space type for the " << addr << std::endl);
        auto end_addr = range.GetEndAddress();
        auto region = panda::mem::AddrToRegion(ToVoidPtr(addr));
        if (!RemsetRegionPredicate(region)) {
            it = dirty_cards_.erase(it);
            continue;
        }

        auto all_cross_region_refs_processed = true;
        while (addr < end_addr) {
            if (!visitor(region, MemRange(addr, addr + MEM_SIZE))) {
                all_cross_region_refs_processed = false;
            }
            addr += MEM_SIZE;
        }
        if (all_cross_region_refs_processed) {
            it = dirty_cards_.erase(it);
            continue;
        }
        ++it;
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RestoreYoungCards(const CollectionSet &collection_set)
{
    CardTable *card_table = this->GetCardTable();
    for (Region *region : collection_set.Young()) {
        card_table->MarkCardsAsYoung(MemRange(region->Begin(), region->End()));
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ClearYoungCards(const CollectionSet &collection_set)
{
    CardTable *card_table = this->GetCardTable();
    for (Region *region : collection_set.Young()) {
        card_table->ClearCardRange(ToUintPtr(region), ToUintPtr(region) + DEFAULT_REGION_SIZE);
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ClearDirtyAndYoungCards(const MemRange &dirty_cards_range)
{
    CardTable *card_table = this->GetCardTable();
    ClearYoungCards(collection_set_);
    card_table->ClearCardRange(dirty_cards_range.GetStartAddress(), dirty_cards_range.GetEndAddress());
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ClearRefsFromRemsetsCache()
{
    ASSERT(!unique_refs_from_remsets_.empty());
    // Resize list of unique refs from remset to 1, to reduce memory usage
    size_t elemets_to_remove = unique_refs_from_remsets_.size() - 1;
    for (size_t i = 0; i < elemets_to_remove; i++) {
        RefVector *entry = unique_refs_from_remsets_.back();
        this->GetInternalAllocator()->Delete(entry);
        unique_refs_from_remsets_.pop_back();
    }
    ASSERT(unique_refs_from_remsets_.size() == 1);
    unique_refs_from_remsets_.front()->clear();
    ASSERT(unique_refs_from_remsets_.front()->capacity() == MAX_REFS);
#ifndef NDEBUG
    unique_cards_initialized_ = false;
#endif  // NDEBUG
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ActualizeRemSets()
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());

    // Invalidate regions from collection set in all remsets
    for (Region *region : collection_set_.Young()) {
        if (!region->HasFlag(RegionFlag::IS_PROMOTED)) {
            RemSet<>::template InvalidateRegion<false>(region);
        } else {
            region->RmvFlag(RegionFlag::IS_PROMOTED);
        }
    }
    for (Region *region : collection_set_.Tenured()) {
        RemSet<>::template InvalidateRegion<false>(region);
    }
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::ShouldRunTenuredGC(const GCTask &task)
{
    return this->IsOnPygoteFork() || task.reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE ||
           task.reason == GCTaskCause::STARTUP_COMPLETE_CAUSE;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::OnWaitForIdleFail()
{
    if (this->GetGCPhase() == GCPhase::GC_PHASE_MARK) {
        // Atomic with release order reason: write to this variable should become visible in concurrent marker check
        interrupt_concurrent_flag_.store(true, std::memory_order_release);
        if (this->GetSettings()->G1EnablePauseTimeGoal()) {
            os::memory::LockHolder lh(concurrent_mark_mutex_);
            concurrent_mark_cond_var_.Signal();
        }
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::PostponeGCStart()
{
    region_garbage_rate_threshold_ = 0;
    g1_promotion_region_alive_rate_ = 0;
    GC::PostponeGCStart();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::PostponeGCEnd()
{
    ASSERT(!this->IsPostponeEnabled() || (region_garbage_rate_threshold_ == 0 && g1_promotion_region_alive_rate_ == 0));
    region_garbage_rate_threshold_ = this->GetSettings()->G1RegionGarbageRateThreshold();
    g1_promotion_region_alive_rate_ = this->GetSettings()->G1PromotionRegionAliveRate();
    GC::PostponeGCEnd();
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::IsPostponeGCSupported() const
{
    return true;
}

template <class LanguageConfig>
size_t G1GC<LanguageConfig>::GetMaxMixedRegionsCount()
{
    return this->GetG1ObjectAllocator()->GetMaxYoungRegionsCount() + number_of_mixed_tenured_regions_;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::PrepareYoungRegionsForFullGC(const CollectionSet &collection_set)
{
    BuildCrossYoungRemSets(collection_set.Young());
    ClearYoungCards(collection_set);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RestoreYoungRegionsAfterFullGC(const CollectionSet &collection_set)
{
    RestoreYoungCards(collection_set);
    for (Region *region : collection_set.Young()) {
        RemSet<>::template InvalidateRefsFromRegion<false>(region);
    }
}

template <class LanguageConfig>
template <typename Container>
void G1GC<LanguageConfig>::BuildCrossYoungRemSets(const Container &young)
{
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());
    ASSERT(this->IsFullGC());
    auto allocator = this->GetG1ObjectAllocator();
    size_t region_size_bits = panda::helpers::math::GetIntLog2(allocator->GetRegionSize());
    auto update_remsets = [region_size_bits](ObjectHeader *object, ObjectHeader *ref, size_t offset,
                                             [[maybe_unused]] bool is_volatile) {
        if (!IsSameRegion(object, ref, region_size_bits) && !ObjectToRegion(ref)->IsYoung()) {
            RemSet<>::AddRefWithAddr<false>(object, offset, ref);
        }
        return true;
    };
    for (Region *region : young) {
        region->GetMarkBitmap()->IterateOverMarkedChunks([&update_remsets](void *addr) {
            ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(
                reinterpret_cast<ObjectHeader *>(addr), update_remsets);
        });
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::StartConcurrentScopeRoutine() const
{
    update_remset_thread_->ResumeThread();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::EndConcurrentScopeRoutine() const
{
    update_remset_thread_->SuspendThread();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ComputeNewSize()
{
    if (this->GetSettings()->G1EnablePauseTimeGoal()) {
        auto desired_eden_length_by_pause_delay = CalculateDesiredEdenLengthByPauseDelay();
        auto desired_eden_length_by_pause_duration = CalculateDesiredEdenLengthByPauseDuration();
        auto desired_eden_length = std::max(desired_eden_length_by_pause_delay, desired_eden_length_by_pause_duration);
        GetG1ObjectAllocator()->GetHeapSpace()->UpdateSize(desired_eden_length *
                                                           GetG1ObjectAllocator()->GetRegionSize());
        GetG1ObjectAllocator()->SetDesiredEdenLength(desired_eden_length);
    } else {
        GenerationalGC<LanguageConfig>::ComputeNewSize();
    }
}

template <class LanguageConfig>
size_t G1GC<LanguageConfig>::CalculateDesiredEdenLengthByPauseDelay()
{
    auto delay_before_pause = g1_pause_tracker_.MinDelayBeforeMaxPauseInMicros(panda::time::GetCurrentTimeInMicros());
    return static_cast<size_t>(ceil(analytics_.PredictAllocationRate() * delay_before_pause));
}

template <class LanguageConfig>
size_t G1GC<LanguageConfig>::CalculateDesiredEdenLengthByPauseDuration()
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    if (is_mixed_gc_required_.load(std::memory_order_relaxed)) {
        // Schedule next mixed collections as often as possible to maximize old regions collection
        return 1;
    }

    // Calculate desired_eden_size according to pause time goal
    size_t min_eden_length = 1;
    size_t max_eden_length =
        GetG1ObjectAllocator()->GetHeapSpace()->GetMaxYoungSize() / GetG1ObjectAllocator()->GetRegionSize();

    auto prediction_error = analytics_.EstimatePredictionErrorInMicros();
    auto max_pause =
        static_cast<int64_t>(this->GetSettings()->GetG1MaxGcPauseInMillis() * panda::os::time::MILLIS_TO_MICRO);
    auto eden_length_predicate = [this, prediction_error, max_pause](size_t eden_length) {
        if (!HaveEnoughRegionsToMove(eden_length)) {
            return false;
        }
        auto pause_time = prediction_error + analytics_.PredictYoungCollectionTimeInMicros(eden_length);
        return pause_time <= max_pause;
    };

    if (!eden_length_predicate(min_eden_length)) {
        return min_eden_length;
    }

    if (eden_length_predicate(max_eden_length)) {
        return max_eden_length;
    }
    auto delta = (max_eden_length - min_eden_length) / 2;
    while (delta > 0) {
        auto eden_length = min_eden_length + delta;
        if (eden_length_predicate(eden_length)) {
            min_eden_length = eden_length;
        } else {
            max_eden_length = eden_length;
        }
        ASSERT(min_eden_length < max_eden_length);
        delta = (max_eden_length - min_eden_length) / 2;
    }
    return min_eden_length;
}

template <class LanguageConfig>
NO_THREAD_SAFETY_ANALYSIS void G1GC<LanguageConfig>::ConcurentMarkImpl(GCMarkingStackType *objects_stack)
{
    {
        ScopedTiming t("VisitClassRoots", *this->GetTiming());
        this->VisitClassRoots([this, objects_stack](const GCRoot &gc_root) {
            if (conc_marker_.MarkIfNotMarked(gc_root.GetObjectHeader())) {
                ASSERT(gc_root.GetObjectHeader() != nullptr);
                objects_stack->PushToStack(RootType::ROOT_CLASS, gc_root.GetObjectHeader());
            } else {
                LOG_DEBUG_GC << "Skip root: " << gc_root.GetObjectHeader();
            }
        });
    }
    {
        ScopedTiming t("VisitInternalStringTable", *this->GetTiming());
        this->GetPandaVm()->VisitStringTable(
            [this, objects_stack](ObjectHeader *str) {
                if (conc_marker_.MarkIfNotMarked(str)) {
                    ASSERT(str != nullptr);
                    objects_stack->PushToStack(RootType::STRING_TABLE, str);
                }
            },
            VisitGCRootFlags::ACCESS_ROOT_ALL | VisitGCRootFlags::START_RECORDING_NEW_ROOT);
    }
    // Atomic with acquire order reason: load to this variable should become visible
    while (!objects_stack->Empty() && !interrupt_concurrent_flag_.load(std::memory_order_acquire)) {
        auto *object = this->PopObjectFromStack(objects_stack);
        ASSERT(conc_marker_.IsMarked(object));
        ValidateObject(nullptr, object);
        auto *object_class = object->template ClassAddr<BaseClass>();
        // We need annotation here for the FullMemoryBarrier used in InitializeClassByIdEntrypoint
        TSAN_ANNOTATE_HAPPENS_AFTER(object_class);
        LOG_DEBUG_GC << "Current object: " << GetDebugInfoAboutObject(object);

        ASSERT(!object->IsForwarded());
        CalcLiveBytesNotAtomicallyMarkPreprocess(object, object_class);
        conc_marker_.MarkInstance(objects_stack, object, object_class);
    }
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(G1GC);
TEMPLATE_CLASS_LANGUAGE_CONFIG(G1GCConcurrentMarker);
TEMPLATE_CLASS_LANGUAGE_CONFIG(G1GCMixedMarker);
TEMPLATE_CLASS_LANGUAGE_CONFIG(G1GCPauseMarker);

}  // namespace panda::mem
