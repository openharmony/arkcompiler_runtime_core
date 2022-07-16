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

#include "runtime/mem/gc/g1/g1-gc.h"

#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/mem/gc/dynamic/gc_marker_dynamic-inl.h"
#include "runtime/mem/gc/g1/ref_cache_builder.h"
#include "runtime/mem/gc/g1/update_remset_thread.h"
#include "runtime/mem/gc/gc_workers_thread_pool.h"
#include "runtime/mem/gc/generational-gc-base-inl.h"
#include "runtime/mem/gc/static/gc_marker_static-inl.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/managed_thread.h"

namespace panda::mem {

static inline object_pointer_type ToObjPtr(const void *ptr)
{
    return static_cast<object_pointer_type>(ToUintPtr(ptr));
}

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
    LOG(DEBUG, GC) << "G1GC pre barrier val = " << std::hex << old_value;
    auto *thread = ManagedThread::GetCurrent();
    // thread can't be null here because pre-barrier is called only in concurrent-mark, but we don't process
    // weak-references in concurrent mark
    ASSERT(thread != nullptr);
    auto buffer_vec = thread->GetPreBuff();
    buffer_vec->push_back(static_cast<ObjectHeader *>(old_value));
}

/*
 * TSAN doesn't understand that we hold a lock already when we call it from OnThreadTerminate with need_lock=false and
 * requires to defend by lock
 */
template <bool need_lock = true>
static NO_THREAD_SAFETY_ANALYSIS void PushCardToUpdateThread(mem::GCG1BarrierSet::G1PostBarrierRingBufferType *cards,
                                                             mem::GCG1BarrierSet *barrier_set)
{
    ASSERT(cards != nullptr);
    ASSERT(barrier_set != nullptr);
    LOG(DEBUG, GC) << "Call PushCardToUpdateThread with vector: " << cards;
    while (true) {
        mem::CardTable::CardPtr card;
        bool has_element = cards->TryPop(&card);
        if (!has_element) {
            break;
        }
        ASSERT(barrier_set != nullptr);
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (need_lock) {
            barrier_set->GetQueueLock()->Lock();
        }
        barrier_set->GetUpdatedRefsQueue()->push_back(card);
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (need_lock) {
            barrier_set->GetQueueLock()->Unlock();
        }
    }
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
    auto card = card_table->GetCardPtr(ToUintPtr(from));
    LOG(DEBUG, GC) << "G1GC post queue add ref: " << std::hex << from << " -> " << ToVoidPtr(ToObjPtr(to))
                   << " from_card: " << card;
    // TODO(dtrubenkov): remove !card->IsYoung() after it will be encoded in compiler barrier
    if ((card->IsClear()) && (!card->IsYoung())) {
        // TODO(dtrubenkov): either encode this in compiler barrier or remove from Interpreter barrier (if move to
        // INT/JIT parts then don't check IsClear here cause it will be marked already)
        card->Mark();

        auto thread = ManagedThread::GetCurrent();
        if (thread == nullptr) {  // slow path via shared-queue for VM threads: gc/compiler/etc
            os::memory::LockHolder lock(*barriers->GetQueueLock());
            barriers->GetUpdatedRefsQueue()->push_back(card);
        } else {
            // general fast-path for mutators
            ASSERT(thread->GetPreBuff() != nullptr);  // write barrier cant be called after Terminate
            auto buffer = thread->GetG1PostBarrierBuffer();
            ASSERT(buffer != nullptr);
            buffer->Push(card);
        }
    }
}

template <class LanguageConfig>
G1GC<LanguageConfig>::G1GC(ObjectAllocatorBase *object_allocator, const GCSettings &settings)
    : GenerationalGC<LanguageConfig>(object_allocator, settings),
      marker_(this),
      concurrent_marking_stack_(this),
      region_garbage_rate_threshold_(settings.G1RegionGarbageRateThreshold()),
      g1_promotion_region_alive_rate_(settings.G1PromotionRegionAliveRate()),
      g1_track_freed_objects_(settings.G1TrackFreedObjects())
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    this->SetType(GCType::G1_GC);
    this->SetTLABsSupported();
    updated_refs_queue_ = allocator->New<GCG1BarrierSet::ThreadLocalCardQueues>();
    calc_live_bytes_ = [this](const ObjectHeader *obj) {
        Region *region = ObjectToRegion(obj);
        if (!this->marker_.IsMarked(obj) && region->HasFlag(IS_OLD)) {
            size_t object_size = GetAlignedObjectSize(GetObjectSize(obj));
            region->AddLiveBytesConcurrently(object_size);
        }
        return true;
    };

    os::memory::LockHolder lock(unassigned_buffers_lock_);
    size_t workers_count = this->GetSettings()->GCWorkersCount() + 1;
    unique_refs_from_remsets_.resize(workers_count);
    unassigned_buffers_.reserve(workers_count);
    for (auto &buf : unique_refs_from_remsets_) {
        buf.reserve(MAX_REFS / workers_count);
        unassigned_buffers_.push_back(&buf);
    }
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
    this->GetInternalAllocator()->Delete(update_remset_thread_);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::WaitForGC(GCTask task)
{
    GenerationalGC<LanguageConfig>::WaitForGC(task);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::InitGCBits(panda::ObjectHeader *obj_header)
{
    // The mutator may create a new object during concurrent marking phase.
    // In this case GC may don't mark it (for example only vregs may contain reference to the new object)
    // and collect. To avoid such situations add objects to a special buffer which
    // will be processed at remark stage.
    if (this->GetCardTable()->GetCardPtr(ToUintPtr(obj_header))->IsYoung() || !concurrent_marking_flag_) {
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
void G1GC<LanguageConfig>::Trigger()
{
    auto task = MakePandaUnique<GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, time::GetCurrentTimeInNanos());
    this->AddGCTask(true, std::move(task), true);
}

template <class LanguageConfig>
template <RegionFlag region_type>
void G1GC<LanguageConfig>::DoRegionCompacting(Region *region, bool use_gc_workers,
                                              PandaVector<PandaVector<ObjectHeader *> *> *moved_objects_vector)
{
    auto internal_allocator = this->GetInternalAllocator();
    if (use_gc_workers) {
        auto vector = internal_allocator->template New<PandaVector<ObjectHeader *>>();
        moved_objects_vector->push_back(vector);
        auto storage = internal_allocator->template New<GCWorkersTask::RegionDataType>(vector, region);
        if (!this->GetWorkersPool()->AddTask(GCWorkersTaskTypes::TASK_REGION_COMPACTING, storage)) {
            // We couldn't send a task to workers. Therefore, do it here.
            internal_allocator->Delete(storage);
            RegionCompactingImpl<true, region_type>(vector, region);
        }
    } else {
        ASSERT(moved_objects_vector->size() == 1);
        RegionCompactingImpl<false, region_type>(moved_objects_vector->back(), region);
    }
}

template <class LanguageConfig>
template <bool atomic>
void G1GC<LanguageConfig>::RegionPromotionImpl(PandaVector<ObjectHeader *> *moved_objects, Region *region)
{
    size_t move_size = 0;
    size_t move_count = 0;
    auto object_allocator = this->GetG1ObjectAllocator();
    auto promotion_move_checker = [&moved_objects](ObjectHeader *src) {
        LOG_DEBUG_OBJECT_EVENTS << "PROMOTE YOUNG object " << src;
        ASSERT(ObjectToRegion(src)->HasFlag(RegionFlag::IS_EDEN));
        moved_objects->push_back(src);
    };
    auto promotion_death_checker = [this, &move_count, &move_size](ObjectHeader *object_header) {
        ++move_count;
        move_size += GetAlignedObjectSize(object_header->ObjectSize());
        if (IsMarked(object_header)) {
            return ObjectStatus::ALIVE_OBJECT;
        }
        LOG_DEBUG_OBJECT_EVENTS << "PROMOTE DEAD YOUNG object " << object_header;
        return ObjectStatus::DEAD_OBJECT;
    };
    object_allocator->PromoteYoungRegion(region, promotion_death_checker, promotion_move_checker);
    region->RmvFlag(RegionFlag::IS_COLLECTION_SET);
    this->mem_stats_.template RecordSizeMovedYoung<atomic>(move_size);
    this->mem_stats_.template RecordCountMovedYoung<atomic>(move_count);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::CollectRefsFromCard(CardTable::CardPtr card, Region *region, RefVector *refs_from_remsets)
{
    auto mem_range = this->GetCardTable()->GetMemoryRange(card);
    auto visitor = [this, refs_from_remsets](void *mem) {
        auto obj = static_cast<ObjectHeader *>(mem);
        RefCacheBuilder<LanguageConfig> builder(this, refs_from_remsets, &unique_objects_from_remsets_,
                                                &objects_from_remsets_lock_);
        ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjectsWithInfo(obj, builder);
    };
    region->GetLiveBitmap()->IterateOverMarkedChunkInRange(ToVoidPtr(mem_range.GetStartAddress()),
                                                           ToVoidPtr(mem_range.GetEndAddress()), visitor);
}

template <class LanguageConfig>
template <bool atomic, bool concurrently>
void G1GC<LanguageConfig>::CollectNonRegularObjects(GCTask &task)
{
    size_t delete_size = 0;
    size_t delete_count = 0;
    auto death_checker = [&delete_size, &delete_count](ObjectHeader *object_header) {
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (concurrently) {
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

        if (region->HasFlag(RegionFlag::IS_LARGE_OBJECT)) {
            LOG_DEBUG_OBJECT_EVENTS << "DELETE HUMONGOUS object " << object_header;
            // humongous allocator increases size by region size
            delete_size += region->Size();
            ++delete_count;
        } else {
            ASSERT(region->HasFlag(RegionFlag::IS_NONMOVABLE));
            LOG_DEBUG_OBJECT_EVENTS << "DELETE NON MOVABLE object " << object_header;
        }
        return ObjectStatus::DEAD_OBJECT;
    };
    auto region_visitor = [this](PandaVector<Region *> &regions) {
        update_remset_thread_->InvalidateRegions(&regions);
    };
    this->GetG1ObjectAllocator()->CollectNonRegularRegions(region_visitor, death_checker);
    this->mem_stats_.template RecordCountFreedTenured<atomic>(delete_count);
    this->mem_stats_.template RecordSizeFreedTenured<atomic>(delete_size);
    task.UpdateGCCollectionType(GCCollectionType::TENURED);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::NeedToPromote(const Region *region) const
{
    ASSERT(region->HasFlag(RegionFlag::IS_EDEN));
    // Issue 8183: Remove unnessesary IsYoungFullGC check after refactoring Full GC
    if ((g1_promotion_region_alive_rate_ < PERCENT_100_D) && !this->IsFullGC() && !IsYoungFullGC()) {
        size_t alive_bytes = region->CalcMarkBytes();
        double alive_percentage = static_cast<double>(alive_bytes) / region->Size() * PERCENT_100_D;
        if (alive_percentage >= g1_promotion_region_alive_rate_) {
            return true;
        }
    }
    return false;
}

template <class LanguageConfig>
template <bool atomic, RegionFlag region_type>
void G1GC<LanguageConfig>::RegionCompactingImpl(PandaVector<ObjectHeader *> *moved_objects, Region *region)
{
    auto object_allocator = this->GetG1ObjectAllocator();
    size_t move_size = 0;
    size_t move_count = 0;
    size_t delete_size = 0;
    size_t delete_count = 0;

    auto move_checker = [this, &moved_objects](ObjectHeader *src, ObjectHeader *dst) {
        LOG_DEBUG_OBJECT_EVENTS << "MOVE object " << src << " -> " << dst;
        ASSERT(ObjectToRegion(dst)->HasFlag(RegionFlag::IS_OLD));
        this->SetForwardAddress(src, dst);
        moved_objects->push_back(dst);
    };
    auto death_checker = [this, &move_count, &move_size, &delete_size, &delete_count](ObjectHeader *object_header) {
        if (IsMarked(object_header)) {
            ++move_count;
            move_size += GetAlignedObjectSize(object_header->ObjectSize());

            return ObjectStatus::ALIVE_OBJECT;
        }
        ++delete_count;
        delete_size += GetAlignedObjectSize(object_header->ObjectSize());

        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (region_type == RegionFlag::IS_EDEN) {
            LOG_DEBUG_OBJECT_EVENTS << "DELETE YOUNG object " << object_header;
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            ASSERT(region_type == RegionFlag::IS_OLD);
            LOG_DEBUG_OBJECT_EVENTS << "DELETE TENURED object " << object_header;
        }
        return ObjectStatus::DEAD_OBJECT;
    };
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (region_type == RegionFlag::IS_EDEN) {
        if (!this->NeedToPromote(region)) {
            if (g1_track_freed_objects_) {
                // We want to track all freed objects, therefore, iterate over all objects in region.
                object_allocator->template CompactRegion<RegionFlag::IS_EDEN, false>(region, death_checker,
                                                                                     move_checker);
            } else {
                object_allocator->template CompactRegion<RegionFlag::IS_EDEN, true>(region, death_checker,
                                                                                    move_checker);
                size_t allocated_size = region->GetAllocatedBytes();
                ASSERT(move_size <= allocated_size);
                // delete_count is equal to 0 because we don't track allocation in TLABs by a default.
                // We will do it only with PANDA_TRACK_TLAB_ALLOCATIONS key
                ASSERT(delete_count == 0);
                ASSERT(delete_size == 0);
                delete_size = allocated_size - move_size;
            }
            this->mem_stats_.template RecordSizeMovedYoung<atomic>(move_size);
            this->mem_stats_.template RecordCountMovedYoung<atomic>(move_count);
            this->mem_stats_.template RecordSizeFreedYoung<atomic>(delete_size);
            this->mem_stats_.template RecordCountFreedYoung<atomic>(delete_count);
        } else {
            RegionPromotionImpl<atomic>(moved_objects, region);
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        ASSERT(region->HasFlag(RegionFlag::IS_OLD));
        ASSERT(!region->HasFlag(RegionFlag::IS_NONMOVABLE) && !region->HasFlag(RegionFlag::IS_LARGE_OBJECT));
        if (g1_track_freed_objects_) {
            // We want to track all freed objects, therefore, iterate over all objects in region.
            object_allocator->template CompactRegion<RegionFlag::IS_OLD, false>(region, death_checker, move_checker);
        } else {
            object_allocator->template CompactRegion<RegionFlag::IS_OLD, true>(region, death_checker, move_checker);
            size_t allocated_objects = region->GetAllocatedObjects();
            size_t allocated_size = region->GetAllocatedBytes();
            ASSERT(move_count <= allocated_objects);
            ASSERT(move_size <= allocated_size);
            ASSERT(delete_count == 0);
            ASSERT(delete_size == 0);
            delete_count = allocated_objects - move_count;
            delete_size = allocated_size - move_size;
        }
        this->mem_stats_.template RecordSizeMovedTenured<atomic>(move_size);
        this->mem_stats_.template RecordCountMovedTenured<atomic>(move_count);
        this->mem_stats_.template RecordSizeFreedTenured<atomic>(delete_size);
        this->mem_stats_.template RecordCountFreedTenured<atomic>(delete_count);
    }
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::InitWorker(void **worker_data)
{
    os::memory::LockHolder lock(unassigned_buffers_lock_);
    ASSERT(!unassigned_buffers_.empty());
    *worker_data = unassigned_buffers_.back();
    unassigned_buffers_.pop_back();
    return true;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::DestroyWorker(void *worker_data)
{
    os::memory::LockHolder lock(unassigned_buffers_lock_);
    unassigned_buffers_.push_back(reinterpret_cast<RefVector *>(worker_data));
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::WorkerTaskProcessing(GCWorkersTask *task, void *worker_data)
{
    switch (task->GetType()) {
        case GCWorkersTaskTypes::TASK_MARKING: {
            auto objects_stack = task->GetMarkingStack();
            MarkStackMixed(objects_stack);
            ASSERT(objects_stack->Empty());
            this->GetInternalAllocator()->Delete(objects_stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_REMARK: {
            const ReferenceCheckPredicateT &ref_disable_pred = [this]([[maybe_unused]] const ObjectHeader *obj) {
                // dont process refs on conc-mark in G1, it can cause a high pause
                LOG(DEBUG, REF_PROC) << "Skip reference: " << obj
                                     << " because it's G1 with phase: " << static_cast<int>(this->GetGCPhase());
                return false;
            };
            auto objects_stack = task->GetMarkingStack();
            this->MarkStack(&marker_, objects_stack, ref_disable_pred, calc_live_bytes_);
            ASSERT(objects_stack->Empty());
            this->GetInternalAllocator()->Delete(objects_stack);
            break;
        }
        case GCWorkersTaskTypes::TASK_REGION_COMPACTING: {
            auto data = task->GetRegionData();
            PandaVector<ObjectHeader *> *moved_objects = data->first;
            Region *region = data->second;
            if (region->HasFlag(RegionFlag::IS_EDEN)) {
                RegionCompactingImpl<true, RegionFlag::IS_EDEN>(moved_objects, region);
            } else if (region->HasFlag(RegionFlag::IS_OLD)) {
                RegionCompactingImpl<true, RegionFlag::IS_OLD>(moved_objects, region);
            } else {
                LOG(FATAL, GC) << "Unsupported region type";
            }
            this->GetInternalAllocator()->Delete(data);
            break;
        }
        case GCWorkersTaskTypes::TASK_INIT_REFS_FROM_REMSETS: {
            CardTable::CardPtr card = task->GetCard();
            auto mem_range = this->GetCardTable()->GetMemoryRange(card);
            Region *region = AddrToRegion(ToVoidPtr(mem_range.GetStartAddress()));
            ASSERT(!region->HasFlag(IS_EDEN));
            auto *refs_from_remsets = reinterpret_cast<RefVector *>(worker_data);
            CollectRefsFromCard(card, region, refs_from_remsets);
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
        LOG_DEBUG_GC << "No regions specified for collection " << task.reason_;
    }
    ASSERT(concurrent_marking_stack_.Empty());
    this->GetObjectGenAllocator()->InvalidateSpaceData();
    this->GetObjectGenAllocator()->UpdateSpaceData();
    RunGC(task, collectible_regions);
    collection_set_ = CollectionSet();  // We can use a pointer to vector here
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunFullForTenured(panda::GCTask &task)
{
    this->SetFullGC(true);
    // At this point young is empty, we can mark+collect+move tenured as a whole
    // A single free tenured region is enough to compact the whole tenured space
    ASSERT(this->GetG1ObjectAllocator()->GetYoungRegions().empty());
    ASSERT(HaveEnoughRegionsToMove(1));
    LOG_DEBUG_GC << "Running Full gc for tenured";
    auto tenured_set = GetCollectibleRegions(task, false);
    RunFullMarkAndProcessRefs(task, tenured_set);

    // Collect regions 1 by 1
    RunPhasesForRegions(task, tenured_set);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunFullMarkAndProcessRefs(panda::GCTask &task, const CollectionSet &collectible_regions)
{
    this->SetFullGC(true);
    LOG_DEBUG_GC << "Mark regions set size:" << collectible_regions.size();
    UpdateCollectionSet(collectible_regions);
    StartMarking<false>(task);
    for (auto region : collectible_regions) {
        region->RmvFlag(RegionFlag::IS_COLLECTION_SET);
    }
    collection_set_ = CollectionSet();  // We can use pointer to vector here
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunFullProcessRefsNoCollect(panda::GCTask &task)
{
    this->SetFullGC(true);
    LOG_DEBUG_GC << "Scannig full heap and process references";
    auto scan_set = GetCollectibleRegions(task, false);
    RunFullMarkAndProcessRefs(task, scan_set);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunPhasesImpl(panda::GCTask &task)
{
    interrupt_concurrent_flag_ = false;
    LOG_DEBUG_GC << "G1GC start, reason: " << task.reason_;
    LOG_DEBUG_GC << "Footprint before GC: " << this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    task.UpdateGCCollectionType(GCCollectionType::YOUNG);

    uint64_t young_total_time {0};
    this->GetTiming()->Reset();
    size_t bytes_in_heap_before_move = this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    {
        ScopedTiming t("G1 GC", *this->GetTiming());
        {
            GCScopedPauseStats scoped_pause_stats(this->GetPandaVm()->GetGCStats());
            this->mem_stats_.Reset();
            if ((task.reason_ == GCTaskCause::YOUNG_GC_CAUSE) || (task.reason_ == GCTaskCause::OOM_CAUSE) ||
                (task.reason_ == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE) ||
                (task.reason_ == GCTaskCause::STARTUP_COMPLETE_CAUSE) ||
                (task.reason_ == GCTaskCause::EXPLICIT_CAUSE) || (task.reason_ == GCTaskCause::NATIVE_ALLOC_CAUSE)) {
                this->GetPandaVm()->GetMemStats()->RecordGCPauseStart();
                // Check there is no concurrent mark running by another thread.
                // Atomic with relaxed order reason: concurrent access with another thread which can running GC now
                ASSERT(!concurrent_marking_flag_.load(std::memory_order_relaxed));

                if ((task.reason_ == GCTaskCause::EXPLICIT_CAUSE) || (task.reason_ == GCTaskCause::OOM_CAUSE)) {
                    this->SetFullGC(true);
                }
                WaitForUpdateRemsetThread();
                // Atomic with acquire order reason: to see changes made by GC thread (which do concurrent marking and
                // than set is_mixed_gc_required_) in mutator thread which waits for the end of concurrent marking.
                auto collectible_regions =
                    GetCollectibleRegions(task, is_mixed_gc_required_.load(std::memory_order_acquire));
                if (this->IsFullGC()) {
                    LOG_DEBUG_GC << "Explicit Full GC invocation due to a reason: " << task.reason_;
                    // Clear young first. We tried to maintain enough regions for that
                    // If the cause is OOM - most likely it is not true and GC will give up
                    // GCing Young can be done using usual young collection routines
                    // so no need to launch it with "full" flag.
                    this->SetFullGC(false);
                    // Issue 8183: Remove unnessesary SetYoungFullGC check after refactoring Full GC
                    SetYoungFullGC(true);
                    auto g1_allocator = this->GetG1ObjectAllocator();
                    CollectionSet young_set(g1_allocator->GetYoungRegions());
                    if (!young_set.empty() && HaveEnoughSpaceToMove(young_set)) {
                        LOG_DEBUG_GC << "Collect young-space, size:" << young_set.size();
                        UpdateCollectionSet(young_set);
                        RunPhasesForRegions(task, young_set);
                    }
                    // Issue 8183: Remove unnessesary SetYoungFullGC check after refactoring Full GC
                    SetYoungFullGC(false);

                    // To efficiently get rid of garbage, we only work with tenured space, doing full mark
                    if (this->GetG1ObjectAllocator()->GetYoungRegions().empty() && HaveEnoughRegionsToMove(1)) {
                        RunFullForTenured(task);
                    } else {
                        // We cannot move or delete any garbage at this point
                        // However, some languages require some types of references being processed
                        // at OOM. That is possible since it doesn't require any free space
                        RunFullProcessRefsNoCollect(task);
                        LOG_INFO_GC << "Failed to run gc, not enough free regions";
                        LOG_INFO_GC << "Accounted total object used bytes = "
                                    << PoolManager::GetMmapMemPool()->GetObjectUsedBytes();
                    }
                } else if (!collectible_regions.empty() && HaveEnoughSpaceToMove(collectible_regions)) {
                    // Ordinary collection flow
                    time::Timer timer(&young_total_time, true);
                    LOG_DEBUG_GC << "Collect regions size:" << collectible_regions.size();
                    UpdateCollectionSet(collectible_regions);
                    RunPhasesForRegions(task, collectible_regions);
                    if (!HaveEnoughSpaceToMove(collectible_regions)) {
                        LOG_DEBUG_GC << "Not leaving enough regions for next young/mixed collection. Proceed to "
                                        "iterative implicit Full GC";
                        // Slow path, full GC. We're short on free regions. In order to prevent OOM at the next GC,
                        // try to free up enough regions so we can do mixed/young once again next time.
                        // Here, we have a chance to compact heap so at the next GC mixed is going to have enough
                        // regions to move to tenured. Without this step, we won't be able to do full at any time, since
                        // we permanently won't have enough regions to move young to, thus not collecting anything
                        RunFullForTenured(task);
                    }
                    if (!HaveEnoughSpaceToMove(collectible_regions)) {
                        LOG_DEBUG_GC << "Implicit Full GC failed to free up enough space. Expect OOM GC soon";
                        LOG_DEBUG_GC << "Accounted total object used bytes = "
                                     << PoolManager::GetMmapMemPool()->GetObjectUsedBytes();
                    }
                    if (young_total_time > 0) {
                        this->GetStats()->AddTimeValue(young_total_time, TimeTypeStats::YOUNG_TOTAL_TIME);
                    }
                } else {
                    LOG_DEBUG_GC << "Failed to run gc: "
                                 << (collectible_regions.empty() ? "nothing to collect in movable space"
                                                                 : "not enough free regions to move");
                }
                this->GetPandaVm()->GetMemStats()->RecordGCPauseEnd();
            }
        }
        // Atomic with acquire order reason: to see changes made by GC thread (which do concurrent marking and than set
        // is_mixed_gc_required_) in mutator thread which waits for the end of concurrent marking.
        if (is_mixed_gc_required_.load(std::memory_order_acquire)) {
            if (!HaveGarbageRegions()) {
                // Atomic with release order reason: to see changes made by GC thread (which do concurrent marking and
                // than set is_mixed_gc_required_) in mutator thread which waits for the end of concurrent marking.
                is_mixed_gc_required_.store(false, std::memory_order_release);
            }
        } else if (!interrupt_concurrent_flag_ && this->ShouldRunTenuredGC(task)) {
            ASSERT(collection_set_.empty());
            // Init concurrent marking
            concurrent_marking_flag_ = true;
        }
        if (concurrent_marking_flag_ && !interrupt_concurrent_flag_) {
            StartMarking<true>(task);
            concurrent_marking_flag_ = false;
            // interrupt_concurrent_flag_ may be set during concurrent marking.
            if (!interrupt_concurrent_flag_) {
                Remark(task);
                // Enable mixed GC
                if (HaveGarbageRegions()) {
                    // Atomic with release order reason: to see changes made by GC thread (which do concurrent marking
                    // and than set is_mixed_gc_required_) in mutator thread which waits for the end of concurrent
                    // marking.
                    is_mixed_gc_required_.store(true, std::memory_order_release);
                }
                {
                    ConcurrentScope concurrent_scope(this);
                    CollectNonRegularObjects<true, true>(task);
                }
            } else {
                ClearSatb();
            }
        }
    }
    // Update global and GC memstats based on generational memstats information
    // We will update tenured stats and record allocations, so set 'true' values
    this->UpdateMemStats(bytes_in_heap_before_move, true, true);

    LOG_DEBUG_GC << "Footprint after GC: " << this->GetPandaVm()->GetMemStats()->GetFootprintHeap();
    this->SetFullGC(false);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::HaveGarbageRegions()
{
    // Use GetTopGarbageRegions because it doesn't return current regions
    PandaVector<Region *> regions = GetG1ObjectAllocator()->template GetTopGarbageRegions<false>(1U);
    if (regions.empty()) {
        return false;
    }
    double garbage_rate = static_cast<double>(regions[0]->GetGarbageBytes()) / regions[0]->Size();
    return garbage_rate >= region_garbage_rate_threshold_;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::InitializeImpl()
{
    // GC saved the PandaVM instance, so we get allocator from the PandaVM.
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    this->CreateCardTable(allocator, PoolManager::GetMmapMemPool()->GetMinObjectAddress(),
                          PoolManager::GetMmapMemPool()->GetTotalObjectSize());

    // TODO(dtrubenkov): initialize barriers
    auto barrier_set = allocator->New<GCG1BarrierSet>(
        allocator, &concurrent_marking_flag_, &PreWrbFuncEntrypoint, &PostWrbUpdateCardFuncEntrypoint,
        panda::helpers::math::GetIntLog2(this->GetG1ObjectAllocator()->GetRegionSize()), this->GetCardTable(),
        updated_refs_queue_, &queue_lock_);
    ASSERT(barrier_set != nullptr);
    this->SetGCBarrierSet(barrier_set);

    if (this->IsWorkerThreadsExist()) {
        auto thread_pool = allocator->New<GCWorkersThreadPool>(allocator, this, this->GetSettings()->GCWorkersCount());
        ASSERT(thread_pool != nullptr);
        this->SetWorkersPool(thread_pool);
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
    marker_.Mark(object);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::MarkObjectIfNotMarked(ObjectHeader *object)
{
    ASSERT(object != nullptr);
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
    return marker_.IsMarked(object);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MarkStackMixed(GCMarkingStackType *stack)
{
    auto ref_pred = [this](const ObjectHeader *obj) { return InGCSweepRange(obj); };
    auto mark_pred = [this](const ObjectHeader *obj) { return InGCSweepRange(obj); };
    this->MarkStack(&marker_, stack, ref_pred, mark_pred);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MarkStackFull(GCMarkingStackType *stack)
{
    auto ref_pred = []([[maybe_unused]] const ObjectHeader *obj) { return true; };
    auto mark_pred = []([[maybe_unused]] const ObjectHeader *obj) { return true; };
    this->MarkStack(&marker_, stack, ref_pred, mark_pred);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MarkReferences(GCMarkingStackType *references, GCPhase gc_phase)
{
    trace::ScopedTrace scoped_trace(__FUNCTION__);
    LOG_DEBUG_GC << "Start marking " << references->Size() << " references";
    // mark refs only on mixed-gc and on full_gc. On concurrent mark we don't handle any references
    if (gc_phase == GCPhase::GC_PHASE_MARK_YOUNG || this->IsFullGC()) {
        MarkStackMixed(references);
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
    [[maybe_unused]] auto phase = this->GetGCPhase();
    ASSERT_DO(!this->collection_set_.empty() || this->IsFullGC() || phase == GCPhase::GC_PHASE_REMARK,
              std::cerr << "Incorrect phase in InGCSweepRange: " << static_cast<size_t>(phase) << "\n");

    ASSERT(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(object) == SpaceType::SPACE_TYPE_OBJECT ||
           PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(object) == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT ||
           PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(object) == SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    Region *obj_region = ObjectToRegion(object);
    return obj_region->IsInCollectionSet();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::RunGC(GCTask &task, const CollectionSet &collectible_regions)
{
    GCScope<TRACE_TIMING> scoped_trace(__FUNCTION__, this);
    LOG_DEBUG_GC << "GC start";
    uint64_t young_pause_time;
    {
        // TODO: Measure only those that are on pause
        time::Timer timer(&young_pause_time, true);
        if (is_mixed_gc_required_) {
            LOG_DEBUG_GC << "Mixed GC";
            task.collection_type_ = GCCollectionType::MIXED;
        } else {
            task.collection_type_ = GCCollectionType::YOUNG;
        }

        if (!this->IsFullGC()) {
            CacheRefsFromRemsets();
            MixedMark(task, collectible_regions);
            CollectAndMove(collectible_regions);
            this->GetCardTable()->ClearAll();
            ClearRefsFromRemsetsCache();
        } else {
            task.collection_type_ = GCCollectionType::FULL;
            for (auto r : collectible_regions) {
                LOG_DEBUG_GC << "Iterative full GC collecting region " << *r;
                CollectionSet cs {};
                cs.AddRegion(r);
                UpdateCollectionSet(cs);
                CacheRefsFromRemsets();
                CollectAndMove(cs);
                ClearRefsFromRemsetsCache();
                this->GetCardTable()->ClearAll();
            }
        }
        this->GetObjectGenAllocator()->InvalidateSpaceData();
    }
    if (young_pause_time > 0) {
        this->GetStats()->AddTimeValue(young_pause_time, TimeTypeStats::YOUNG_PAUSED_TIME);
    }
    LOG_DEBUG_GC << "G1GC RunGC end";
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::MixedMark(const GCTask &task, const CollectionSet &collectible_regions)
{
    GCScope<TRACE_TIMING_PHASE> scoped_trace(__FUNCTION__, this, GCPhase::GC_PHASE_MARK_YOUNG);

    bool use_gc_workers = this->GetSettings()->ParallelMarkingEnabled();

    GCMarkingStackType objects_stack(this, use_gc_workers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                     use_gc_workers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                     GCWorkersTaskTypes::TASK_MARKING);
    // Iterate over roots and add other roots
    // 0. Pre-process refs queue and fill RemSets (should be done later in background)
    // Note: We need to process only tenured -> young refs,
    // since we reach this by graph from tenured roots,
    // because we will process all young regions at young GC we will find all required references

    ASSERT(this->GetReferenceProcessor()->GetReferenceQueueSize() ==
           0);  // all references should be processed on previous-gc

    auto ref_pred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    GCRootVisitor gc_mark_collection_set = [&objects_stack, this, &ref_pred](const GCRoot &gc_root) {
        ObjectHeader *root_object = gc_root.GetObjectHeader();
        ObjectHeader *from_object = gc_root.GetFromObjectHeader();
        LOG_DEBUG_GC << "Handle root " << GetDebugInfoAboutObject(root_object) << " from: " << gc_root.GetType();
        if (UNLIKELY(from_object != nullptr) &&
            this->IsReference(from_object->ClassAddr<BaseClass>(), from_object, ref_pred)) {
            LOG_DEBUG_GC << "Add reference: " << GetDebugInfoAboutObject(from_object) << " to stack";
            marker_.Mark(from_object);
            this->ProcessReference(&objects_stack, from_object->ClassAddr<BaseClass>(), from_object,
                                   GC::EmptyReferenceProcessPredicate);
        } else {
            // Skip non-collection-set roots
            auto root_object_ptr = gc_root.GetObjectHeader();
            ASSERT(root_object_ptr != nullptr);
            if (this->InGCSweepRange(root_object_ptr)) {
                LOG_DEBUG_GC << "root " << GetDebugInfoAboutObject(root_object_ptr);
                if (marker_.MarkIfNotMarked(root_object_ptr)) {
                    objects_stack.PushToStack(gc_root.GetType(), root_object_ptr);
                }
            } else {
                LOG_DEBUG_GC << "Skip root for young mark: " << std::hex << root_object_ptr;
            }
        }
    };
    {
        GCScope<TRACE_TIMING> marking_collection_set_roots_trace("Marking roots collection-set", this);

        for (Region *region : collectible_regions) {
            region->GetMarkBitmap()->ClearAllBits();
        }

        this->VisitRoots(gc_mark_collection_set, VisitGCRootFlags::ACCESS_ROOT_NONE);
        // Visit roots from RemSets
        auto field_visitor = [this, &objects_stack]([[maybe_unused]] ObjectHeader *from_obj, ObjectHeader *to_obj,
                                                    [[maybe_unused]] uint32_t offset,
                                                    [[maybe_unused]] bool is_volatile) {
            if (!InGCSweepRange(to_obj)) {
                LOG_DEBUG_GC << "Skip root for collection_set mark: " << std::hex << to_obj;
                return true;
            }
            LOG_DEBUG_GC << "root " << GetDebugInfoAboutObject(to_obj);
            if (marker_.MarkIfNotMarked(to_obj)) {
                objects_stack.PushToStack(from_obj, to_obj);
            }
            return true;
        };
        VisitRemSets(field_visitor);
    }
    {
        ScopedTiming mark_stack_timing("MarkStack", *this->GetTiming());
        this->MarkStackMixed(&objects_stack);
        ASSERT(objects_stack.Empty());
        if (use_gc_workers) {
            this->GetWorkersPool()->WaitUntilTasksEnd();
        }
    }

    auto ref_clear_pred = [this]([[maybe_unused]] const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    this->GetPandaVm()->HandleReferences(task, ref_clear_pred);
    // HandleReferences could write a new barriers - so we need to handle them before moving
    WaitForUpdateRemsetThread();
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
            LOG(FATAL, GC) << "Heap was corrupted during CollectAndMove GC phase, HeapVerifier found " << fails_count
                           << " corruptions";
        }
    }
}

template <class LanguageConfig>
// NOLINTNEXTLINE(readability-function-size)
bool G1GC<LanguageConfig>::CollectAndMove(const CollectionSet &collection_set)
{
    GCScope<TRACE_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE);
    LOG_DEBUG_GC << "== G1GC CollectAndMove start ==";
    auto internal_allocator = this->GetInternalAllocator();
    bool use_gc_workers = this->GetSettings()->ParallelCompactingEnabled() && !this->IsFullGC();

    PandaVector<PandaVector<ObjectHeader *> *> moved_objects_vector;
    HeapVerifierIntoGC<LanguageConfig> collect_verifier = this->CollectVerificationInfo(collection_set);
    {
        ScopedTiming compact_regions("CompactRegions", *this->GetTiming());
        if (!use_gc_workers) {
            auto vector = internal_allocator->template New<PandaVector<ObjectHeader *>>();
            moved_objects_vector.push_back(vector);
        }
        for (auto r : collection_set.Young()) {
            this->DoRegionCompacting<RegionFlag::IS_EDEN>(r, use_gc_workers, &moved_objects_vector);
        }
        for (auto r : collection_set.Tenured()) {
            this->DoRegionCompacting<RegionFlag::IS_OLD>(r, use_gc_workers, &moved_objects_vector);
        }

        if (use_gc_workers) {
            this->GetWorkersPool()->WaitUntilTasksEnd();
        }
    }

    PandaVector<Region *> tenured_regions(collection_set.Tenured().begin(), collection_set.Tenured().end());
    UpdateRefsToMovedObjects(&moved_objects_vector);
    this->VerifyCollectAndMove(std::move(collect_verifier), collection_set);
    this->SweepStringTableYoung([this](ObjectHeader *obj) { return this->InGCSweepRange(obj); });

    ActualizeRemSets();

    auto object_allocator = this->GetG1ObjectAllocator();
    object_allocator->ResetYoungAllocator();
    object_allocator->template ResetRegions<RegionFlag::IS_OLD>(tenured_regions);

    // Don't forget to delete all temporary elements
    if (use_gc_workers) {
        for (auto r : moved_objects_vector) {
            internal_allocator->Delete(r);
        }
    } else {
        ASSERT(moved_objects_vector.size() == 1);
        internal_allocator->Delete(moved_objects_vector.back());
    }

    LOG_DEBUG_GC << "== G1GC CollectAndMove end ==";
    return true;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::UpdateRefsToMovedObjects(PandaVector<PandaVector<ObjectHeader *> *> *moved_objects_vector)
{
    GCScope<TRACE_TIMING> scope("UpdateRefsToMovedObjects", this);
    auto object_allocator = this->GetG1ObjectAllocator();
    size_t region_size_bits = panda::helpers::math::GetIntLog2(object_allocator->GetRegionSize());
    auto update_refs = [region_size_bits](ObjectHeader *object, ObjectHeader *ref, uint32_t offset, bool is_volatile) {
        ObjectHeader *forwarded =
            ObjectHelpers<LanguageConfig::LANG_TYPE>::UpdateRefToMovedObject(object, ref, offset, is_volatile);
        if (((ToUintPtr(object) ^ ToUintPtr(forwarded)) >> region_size_bits) != 0) {
            RemSet<>::AddRefWithAddr<false>(object, forwarded);
        }
        return true;
    };

    // Update references exyoung -> young
    LOG_DEBUG_GC << "=== Update exyoung -> young references. START. ===";
    for (auto moved_objects : *moved_objects_vector) {
        for (auto obj : *moved_objects) {
            ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjectsWithInfo(obj, update_refs);
        }
    }

    LOG_DEBUG_GC << "=== Update exyoung -> young references. END. ===";
    // update references tenured -> young
    LOG_DEBUG_GC << "=== Update tenured -> young references. START. ===";

    VisitRemSets(update_refs);
    LOG_DEBUG_GC << "=== Update tenured -> young references. END. ===";
    this->CommonUpdateRefsToMovedObjects();
}

template <class LanguageConfig>
template <bool is_concurrent>
void G1GC<LanguageConfig>::StartMarking(panda::GCTask &task)
{
    auto object_allocator = GetG1ObjectAllocator();
    PandaString mark_type;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (is_concurrent) {
        mark_type = "Concurrent";
    } else {  // NOLINT(readability-misleading-indentation)
        mark_type = "OnPause";
    }

    {
        // First we need to unmark all heap
        ScopedTiming un_mark_timing("UnMark", *this->GetTiming());
        LOG_DEBUG_GC << "Start unmark all heap before " << mark_type << " mark";
        auto all_region = object_allocator->GetAllRegions();
        for (Region *r : all_region) {
            auto *bitmap = r->GetMarkBitmap();
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
    LOG_DEBUG_GC << mark_type << " marking started";
    {
        GCScopedPhase scoped_phase(this->GetPandaVm()->GetMemStats(), this, GCPhase::GC_PHASE_INITIAL_MARK);
        // Collect non-heap roots.
        // Mark the whole heap by using only these roots.
        // The interregion roots will be processed at pause

        // InitialMark. STW
        ASSERT(concurrent_marking_stack_.Empty());
        auto &conc_stack = concurrent_marking_stack_;
        GCRootVisitor gc_mark_roots = [this, &conc_stack](const GCRoot &gc_root) {
            ValidateObject(gc_root.GetType(), gc_root.GetObjectHeader());
            if (marker_.MarkIfNotMarked(gc_root.GetObjectHeader())) {
                conc_stack.PushToStack(gc_root.GetType(), gc_root.GetObjectHeader());
            }
        };
        this->VisitRoots(gc_mark_roots, VisitGCRootFlags::ACCESS_ROOT_ALL);
    }
    // Concurrent/on-pause marking
    {
        this->GetPandaVm()->GetMemStats()->RecordGCPauseEnd();
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (is_concurrent) {
            const ReferenceCheckPredicateT &disable_ref_pred = []([[maybe_unused]] const ObjectHeader *obj) {
                return false;
            };
            auto interrupt_checker = [this]() {
                return !concurrent_marking_stack_.Empty() && !interrupt_concurrent_flag_;
            };
            ConcurrentMark(
                &marker_, &concurrent_marking_stack_, CardTableVisitFlag::VISIT_DISABLED, interrupt_checker,
                disable_ref_pred, [&object_allocator](MemRange &mem_range) {
                    return !object_allocator->IsIntersectedWithYoung(  // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
                        mem_range);                                    // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
                });                                                    // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
            // weak refs shouldn't be added to the queue on concurrent-mark
            ASSERT(this->GetReferenceProcessor()->GetReferenceQueueSize() == 0);
        } else {  // NOLINT(readability-misleading-indentation)
            const ReferenceCheckPredicateT &ref_pred = []([[maybe_unused]] const ObjectHeader *obj) { return true; };
            auto no_concurrent_interrupt_checker = [this]() { return !concurrent_marking_stack_.Empty(); };
            this->OnPauseMark(
                &marker_, &concurrent_marking_stack_, CardTableVisitFlag::VISIT_DISABLED,
                no_concurrent_interrupt_checker, ref_pred, [&object_allocator](MemRange &mem_range) {
                    return !object_allocator->IsIntersectedWithYoung(  // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
                        mem_range);                                    // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)
                });                                                    // CODECHECK-NOLINT(C_RULE_ID_INDENT_CHECK)

            {
                // we process all refs on FULL_GC
                GCScopedPhase scoped_phase(this->GetPandaVm()->GetMemStats(), this, GCPhase::GC_PHASE_MARK);
                ASSERT(this->IsFullGC());
                auto g1_allocator = this->GetG1ObjectAllocator();
                auto all_regions = g1_allocator->GetAllRegions();
                /**
                 * We don't collect non-movable regions right now, if there was a reference from non-movable to
                 * young/tenured region then we reset markbitmap for non-nonmovable, but don't update livebitmap and we
                 * can traverse over non-reachable object (in CacheRefsFromRemsets) and visit DEAD object in
                 * tenured space (was delete on young-collection or in Iterative-full-gc phase.
                 */
                // TODO(alovkov): add ASSERT(g1_allocator->GetRegions().size == collection_set_.size())
                auto ref_clear_pred = []([[maybe_unused]] const ObjectHeader *obj) { return true; };
                this->GetPandaVm()->HandleReferences(task, ref_clear_pred);
                // HandleReferences could write a new barriers - so we need to handle them before cloning maps
                WaitForUpdateRemsetThread();
                // TODO(alovkov): make iterations over collection_set when non-movable regions will be supported
                for (const auto &r : all_regions) {
                    if (r->GetLiveBitmap() != nullptr) {
                        r->CloneMarkBitmapToLiveBitmap();
                    }
                }
                CollectNonRegularObjects<false, false>(task);
            }
        }
        if (interrupt_concurrent_flag_) {
            concurrent_marking_stack_.Clear();
        }
        ASSERT(concurrent_marking_stack_.Empty());
        this->GetPandaVm()->GetMemStats()->RecordGCPauseStart();
    }
    ASSERT(concurrent_marking_stack_.Empty());
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ConcurrentMark(Marker *marker, GCMarkingStackType *objects_stack,
                                          CardTableVisitFlag visit_card_table_roots,
                                          const ConcurrentMarkPredicateT &pred,
                                          const ReferenceCheckPredicateT &ref_pred,
                                          const MemRangeChecker &mem_range_checker)
{
    GCScope<TRACE_TIMING_PHASE> scope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);
    ConcurrentScope concurrent_scope(this);
    this->MarkImpl(marker, objects_stack, visit_card_table_roots, pred, ref_pred, mem_range_checker);
    if (interrupt_concurrent_flag_) {
        return;
    }
    CalcLiveBytesForMovableTenuredRegions();
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::Remark(panda::GCTask const &task)
{
    /**
     * Make remark on pause to have all marked objects in tenured space, it gives possibility to check objects in
     * remsets. If they are not marked - we don't process this object, because it's dead already
     */
    GCScope<TIMING_PHASE> gc_scope(__FUNCTION__, this, GCPhase::GC_PHASE_REMARK);
    // TODO(alovkov): extract method for creation stack
    bool use_gc_workers = this->GetSettings()->ParallelMarkingEnabled();
    GCMarkingStackType stack(this, use_gc_workers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                             use_gc_workers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                             GCWorkersTaskTypes::TASK_REMARK);

    // The mutator may create new regions.
    // If so we should bind bitmaps of new regions.
    DrainSatb(&stack);
    const ReferenceCheckPredicateT &ref_disable_pred = [this]([[maybe_unused]] const ObjectHeader *obj) {
        LOG(DEBUG, REF_PROC) << "Skip reference: " << obj
                             << " because it's G1 with phase: " << static_cast<int>(this->GetGCPhase());
        return false;
    };
    this->MarkStack(&marker_, &stack, ref_disable_pred, calc_live_bytes_);

    if (use_gc_workers) {
        this->GetWorkersPool()->WaitUntilTasksEnd();
    }
    {
        ScopedTiming remsetThreadTiming("RemsetThread WaitUntilTasksEnd", *this->GetTiming());
        WaitForUpdateRemsetThread();
    }

    // ConcurrentMark doesn't visit young objects - so we can't clear references which are in young-space because we
    // don't know which objects are marked. We will process them on young/mixed GC separately later, here we process
    // only refs in tenured-space
    auto ref_clear_pred = []([[maybe_unused]] const ObjectHeader *obj) {
        return !ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN);
    };
    this->GetPandaVm()->HandleReferences(task, ref_clear_pred);

    auto g1_allocator = this->GetG1ObjectAllocator();
    auto all_regions = g1_allocator->GetAllRegions();
    for (const auto &region : all_regions) {
        // TODO(alovkov): set IS_OLD for NON_MOVABLE region when we create it
        if (region->HasFlag(IS_OLD) || region->HasFlag(IS_NONMOVABLE)) {
            region->SwapMarkBitmap();
        }
    }
}

template <class LanguageConfig>
CollectionSet G1GC<LanguageConfig>::GetCollectibleRegions(panda::GCTask const &task, bool is_mixed)
{
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());
    // FillRemSet should be always finished before GetCollectibleRegions
    ASSERT(update_remset_thread_->GetQueueSize() == 0);
    auto g1_allocator = this->GetG1ObjectAllocator();
    LOG_DEBUG_GC << "Start GetCollectibleRegions is_mixed: " << is_mixed << " reason: " << task.reason_;
    CollectionSet collection_set(g1_allocator->GetYoungRegions());
    bool is_full_gc = this->IsFullGC();
    if (is_mixed || is_full_gc) {
        if (is_full_gc) {
            auto all_movable_regions = g1_allocator->GetMovableRegions();
            LOG_DEBUG_GC << "all movable region size: " << all_movable_regions.size();
            for (const auto &region : all_movable_regions) {
                LOG_DEBUG_GC << "region: " << *region;
                if (region->HasFlag(IS_EDEN)) {
                    continue;
                }
                ASSERT(!region->HasFlag(IS_NONMOVABLE) && !region->HasFlag(IS_LARGE_OBJECT));
                ASSERT(region->HasFlag(IS_OLD));
                collection_set.AddRegion(region);
            }
            // make new region to move objects there
            g1_allocator->ClearCurrentRegion();
        } else {
            auto garbage_regions = g1_allocator->template GetTopGarbageRegions<false>(number_of_mixed_tenured_regions_);
            for (auto garbage_region : garbage_regions) {
                ASSERT(!garbage_region->HasFlag(IS_EDEN));
                ASSERT(is_mixed_gc_required_);  // to be sure that GetLiveBytes is calculated in concurrent
                double garbage_rate = static_cast<double>(garbage_region->GetGarbageBytes()) / garbage_region->Size();
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
    }
    LOG_DEBUG_GC << "collectible_regions size: " << collection_set.size() << " reason: " << task.reason_
                 << " is_mixed: " << is_mixed;
    return collection_set;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::CalcLiveBytesForMovableTenuredRegions()
{
    ScopedTiming scoped_timing(__FUNCTION__, *this->GetTiming());
    auto object_allocator = this->GetG1ObjectAllocator();
    auto movable_region = object_allocator->GetMovableRegions();
    for (const auto &region : movable_region) {
        if (region->HasFlag(IS_OLD)) {
            region->SetLiveBytes(region->CalcMarkBytes());
        }
    }
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::HaveEnoughSpaceToMove(const CollectionSet &collectible_regions)
{
    // Take parallel compacting into account
    size_t parallel_compacting_fragmentation =
        this->GetSettings()->ParallelCompactingEnabled() ? this->GetSettings()->GCWorkersCount() : 0;
    size_t required_regions = collectible_regions.Movable().size() + parallel_compacting_fragmentation;
    return HaveEnoughRegionsToMove(required_regions);
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::HaveEnoughRegionsToMove(size_t num)
{
    return GetG1ObjectAllocator()->HaveTenuredSize(num) && GetG1ObjectAllocator()->HaveFreeRegions(num);
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::OnThreadTerminate(ManagedThread *thread)
{
    auto tid = thread->GetId();
    LOG_DEBUG_GC << "Call OnThreadTerminate for thread: " << tid;
    {
        os::memory::LockHolder lock(satb_and_newobj_buf_lock_);
        auto pre_buff = thread->MovePreBuff();
        ASSERT(pre_buff != nullptr);
        satb_buff_list_.push_back(pre_buff);
    }
    {
        os::memory::LockHolder lock(queue_lock_);
        auto *local_buffer = thread->GetG1PostBarrierBuffer();

        ASSERT(local_buffer != nullptr);
        LOG_DEBUG_GC << "OnThreadTerminate push queue: " << local_buffer;
        // we need to make it under lock, because we can have situation when we made GetG1PostBarrierBuffer
        // -> FillRemset -> call PushCardToUpdateThread, but it's too late - thread processed all cards already
        auto barrier_set = static_cast<GCG1BarrierSet *>(thread->GetBarrierSet());
        PushCardToUpdateThread<false>(local_buffer, barrier_set);
        ASSERT(local_buffer->IsEmpty());
        thread->ResetG1PostBarrierRingBuffer();
        this->GetInternalAllocator()->template Delete(local_buffer);
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
        auto thread_pool =
            allocator->template New<GCWorkersThreadPool>(allocator, this, this->GetSettings()->GCWorkersCount());
        ASSERT(thread_pool != nullptr);
        this->SetWorkersPool(thread_pool);
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
            marker_.Mark(obj);
            object_stack->PushToStack(RootType::SATB_BUFFER, obj);
        }
        pre_buff->clear();
        return true;
    };
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (LanguageConfig::MT_MODE == MT_MODE_MULTI) {
        Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(
            callback, static_cast<unsigned int>(EnumerationFlag::ALL));
    } else if (LanguageConfig::MT_MODE == MT_MODE_SINGLE) {  // NOLINT(readability-misleading-indentation)
        callback(Thread::GetCurrent()->GetVM()->GetAssociatedThread());
    } else {
        UNREACHABLE();
    }

    // Process satb buffers of the terminated threads
    os::memory::LockHolder lock(satb_and_newobj_buf_lock_);
    for (auto obj_vector : satb_buff_list_) {
        ASSERT(obj_vector != nullptr);
        for (auto obj : *obj_vector) {
            marker_.Mark(obj);
            object_stack->PushToStack(RootType::SATB_BUFFER, obj);
        }
        this->GetInternalAllocator()->Delete(obj_vector);
    }
    satb_buff_list_.clear();
    for (auto obj : newobj_buffer_) {
        marker_.Mark(obj);
        object_stack->PushToStack(RootType::SATB_BUFFER, obj);
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

    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (LanguageConfig::MT_MODE == MT_MODE_SINGLE) {
        auto thread = this->GetPandaVm()->GetAssociatedThread();
        auto local_buffer = thread->GetG1PostBarrierBuffer();
        if (local_buffer == nullptr) {
            return;
        }
        while (!local_buffer->IsEmpty()) {
        }
    } else if (LanguageConfig::MT_MODE == MT_MODE_MULTI) {  // NOLINT(readability-misleading-indentation)
        while (true) {
            bool have_not_empty_queue = false;
            Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(
                [&have_not_empty_queue](ManagedThread *thread) {
                    auto local_buffer = thread->GetG1PostBarrierBuffer();
                    if (local_buffer != nullptr && !local_buffer->IsEmpty()) {
                        have_not_empty_queue = true;
                        return false;
                    }
                    return true;
                },
                static_cast<unsigned int>(EnumerationFlag::ALL));
            if (!have_not_empty_queue) {
                break;
            }
        }
    } else {
        UNREACHABLE();
    }
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
    if constexpr (LanguageConfig::MT_MODE == MT_MODE_MULTI) {  // NOLINT
        Thread::GetCurrent()->GetVM()->GetThreadManager()->EnumerateThreads(thread_callback);
    } else if (LanguageConfig::MT_MODE == MT_MODE_SINGLE) {  // NOLINT(readability-misleading-indentation)
        thread_callback(Thread::GetCurrent()->GetVM()->GetAssociatedThread());
    } else {
        UNREACHABLE();
    }

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
    ScopedTiming t(__FUNCTION__, *this->GetTiming());

    ASSERT(unique_cards_initialized_);
    // Iterate over stored references to the collection set
    for (auto &collection : unique_refs_from_remsets_) {
        for (auto &entry : collection) {
            ObjectHeader *object = entry.GetObject();
            uint32_t offset = entry.GetReferenceOffset();
            bool is_volatile = entry.IsVolatile();
            visitor(object, ObjectAccessor::GetObject(object, offset), offset, is_volatile);
        }
    }
    // Iterate over objects which have a reference to the collection set
    for (auto object : unique_objects_from_remsets_) {
        ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjectsWithInfo(object, visitor);
    }
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::CacheRefsFromRemsets()
{
    ScopedTiming t(__FUNCTION__, *this->GetTiming());
    // Collect only unique objects to not proceed them more than once.
    ASSERT(!unique_cards_initialized_);
    bool use_gc_workers = this->GetSettings()->ParallelCompactingEnabled() && !this->IsFullGC();
    auto card_visitor = [this, use_gc_workers](CardPtr card, Region *r) {
        ASSERT(!r->HasFlag(IS_EDEN));
        // In case of mixed GC don't process remsets of the tenured regions which are in the collection set
        bool to_process = !r->HasFlag(IS_COLLECTION_SET) || r->HasFlag(IS_NONMOVABLE) || r->HasFlag(IS_LARGE_OBJECT);
        if (!to_process) {
            return;
        }
        if (!card->IsProcessed()) {
            card->SetProcessed();
            if (!use_gc_workers ||
                !this->GetWorkersPool()->AddTask(GCWorkersTaskTypes::TASK_INIT_REFS_FROM_REMSETS, card)) {
                CollectRefsFromCard(card, r, &unique_refs_from_remsets_.front());
            }
        }
    };
    for (auto region : this->collection_set_) {
        region->GetRemSet()->ProceedMarkedCards(card_visitor);
    }
    if (use_gc_workers) {
        this->GetWorkersPool()->WaitUntilTasksEnd();
    }
#ifndef NDEBUG
    unique_cards_initialized_ = true;
#endif  // NDEBUG
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::ClearRefsFromRemsetsCache()
{
    for (auto &collection : unique_refs_from_remsets_) {
        collection.clear();
    }
    unique_objects_from_remsets_.clear();
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
        }
    }
    for (Region *region : collection_set_.Tenured()) {
        RemSet<>::template InvalidateRegion<false>(region);
    }
}

template <class LanguageConfig>
bool G1GC<LanguageConfig>::ShouldRunTenuredGC(const GCTask &task)
{
    return this->IsOnPygoteFork() || task.reason_ == GCTaskCause::OOM_CAUSE ||
           task.reason_ == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE ||
           task.reason_ == GCTaskCause::STARTUP_COMPLETE_CAUSE;
}

template <class LanguageConfig>
void G1GC<LanguageConfig>::OnWaitForIdleFail()
{
    if (this->GetGCPhase() == GCPhase::GC_PHASE_MARK) {
        interrupt_concurrent_flag_ = true;
    }
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(G1GC);
template class G1GCMarker<LANG_TYPE_STATIC, false>;
template class G1GCMarker<LANG_TYPE_DYNAMIC, false>;

}  // namespace panda::mem
