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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_GC_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_GC_H

#include <functional>

#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_marker.h"
#include "runtime/mem/gc/gc_workers_thread_pool.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/gc/g1/collection_set.h"
#include "runtime/mem/gc/generational-gc-base.h"
#include "runtime/mem/heap_verifier.h"

namespace panda {
class ManagedThread;
}  // namespace panda
namespace panda::mem {
template <typename T>
class UpdateRemsetThread;

template <LangTypeT LANG_TYPE, bool HAS_VALUE_OBJECT_TYPES>
class G1GCMarker : public GCMarker<G1GCMarker<LANG_TYPE, HAS_VALUE_OBJECT_TYPES>, LANG_TYPE, HAS_VALUE_OBJECT_TYPES> {
public:
    explicit G1GCMarker(GC *gc)
        : GCMarker<G1GCMarker<LANG_TYPE, HAS_VALUE_OBJECT_TYPES>, LANG_TYPE, HAS_VALUE_OBJECT_TYPES>(gc)
    {
    }

    bool MarkIfNotMarked(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        return !bitmap->AtomicTestAndSet(object);
    }

    bool IsMarked(const ObjectHeader *object) const
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        return bitmap->AtomicTest(object);
    }

    template <bool atomic = true>
    void Mark(ObjectHeader *object)
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        bitmap->AtomicTestAndSet(object);
    }
};

/**
 * \brief G1 alike GC
 */
template <class LanguageConfig>
class G1GC final : public GenerationalGC<LanguageConfig> {
    using Marker = G1GCMarker<LanguageConfig::LANG_TYPE, LanguageConfig::HAS_VALUE_OBJECT_TYPES>;
    using ConcurrentMarkPredicateT = typename GenerationalGC<LanguageConfig>::ConcurrentMarkPredicateT;
    using RefVector = PandaVector<RefInfo>;
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

public:
    explicit G1GC(ObjectAllocatorBase *object_allocator, const GCSettings &settings);

    ~G1GC() override;

    void StopGC() override
    {
        GC::StopGC();
        // GC is using update_remset_thread so we need to stop GC first before we destroy the thread
        update_remset_thread_->DestroyThread();
    }

    NO_MOVE_SEMANTIC(G1GC);
    NO_COPY_SEMANTIC(G1GC);

    void WaitForGC(GCTask task) override;

    void InitGCBits(panda::ObjectHeader *obj_header) override;

    void InitGCBitsForAllocationInTLAB(panda::ObjectHeader *object) override;

    void Trigger() override;

    bool InitWorker(void **worker_data) override;

    void DestroyWorker(void *worker_data) override;

    void WorkerTaskProcessing(GCWorkersTask *task, void *worker_data) override;

    void MarkReferences(GCMarkingStackType *references, GCPhase gc_phase) override;

    void MarkObject(ObjectHeader *object) override;

    bool MarkObjectIfNotMarked(ObjectHeader *object) override;

    bool InGCSweepRange(const ObjectHeader *object) const override;

    void OnThreadTerminate(ManagedThread *thread) override;

    void PreZygoteFork() override;
    void PostZygoteFork() override;

    void OnWaitForIdleFail() override;

    void StartGC() override
    {
        GC::StartGC();
        InternalAllocatorPtr allocator = this->GetInternalAllocator();
        update_remset_thread_->CreateThread(allocator);
    }

private:
    bool HaveGarbageRegions();

    template <RegionFlag region_type>
    void DoRegionCompacting(Region *region, bool use_gc_workers,
                            PandaVector<PandaVector<ObjectHeader *> *> *moved_objects_vector);

    template <bool atomic, bool concurrently>
    void CollectNonRegularObjects(GCTask &task);

    bool NeedToPromote(const Region *region) const;

    template <bool atomic, RegionFlag region_type>
    void RegionCompactingImpl(PandaVector<ObjectHeader *> *moved_objects, Region *region);

    template <bool atomic>
    void RegionPromotionImpl(PandaVector<ObjectHeader *> *moved_objects, Region *region);

    void CollectRefsFromCard(CardTable::CardPtr card, Region *region, RefVector *refs_from_remsets);

    void InitializeImpl() override;

    void RunPhasesImpl(GCTask &task) override;

    void RunPhasesForRegions([[maybe_unused]] panda::GCTask &task, const CollectionSet &collectible_regions);

    void RunFullForTenured(panda::GCTask &task);

    void RunFullMarkAndProcessRefs(panda::GCTask &task, const CollectionSet &collectible_regions);

    void RunFullProcessRefsNoCollect(panda::GCTask &task);

    void PreStartupImp() override;

    void VisitCard(CardTable::CardPtr card, const ObjectVisitor &object_visitor, const CardVisitor &card_visitor);

    /**
     * GC for young generation. Runs with STW.
     */
    void RunGC(GCTask &task, const CollectionSet &collectible_regions);

    /**
     * GC for tenured generation.
     */
    void RunTenuredGC(const GCTask &task);

    /**
     * Marks objects in collection set (young-generation + maybe some tenured regions).
     */
    void MixedMark(const GCTask &task, const CollectionSet &collectible_regions);

    /**
     * Mark roots and add them to the stack
     * @param objects_stack
     * @param visit_class_roots
     * @param visit_card_table_roots
     */
    void MarkRoots(GCMarkingStackType *objects_stack, CardTableVisitFlag visit_card_table_roots,
                   VisitGCRootFlags flags = VisitGCRootFlags::ACCESS_ROOT_ALL);

    /**
     * Initial marks roots and fill in 1st level from roots into stack.
     * STW
     * @param objects_stack
     */
    void InitialMark(GCMarkingStackType *objects_stack);

    /**
     * ReMarks objects after Concurrent marking
     * @param objects_stack
     */
    void ReMark(GCMarkingStackType *objects_stack, GCTask task);

    void MarkStackMixed(GCMarkingStackType *stack);

    void MarkStackFull(GCMarkingStackType *stack);

    bool IsInCollectionSet(ObjectHeader *object);

    /**
     * Collect dead objects in young generation and move survivors
     * @return true if moving was success, false otherwise
     */
    bool CollectAndMove(const CollectionSet &collection_set);

    /**
     * Collect verification info for CollectAndMove phase
     * @param collection_set collection set for the current phase
     * @return instance of verifier to be used to verify for updated references
     */
    [[nodiscard]] HeapVerifierIntoGC<LanguageConfig> CollectVerificationInfo(const CollectionSet &collection_set);

    /**
     * Verify updted references
     * @param collect_verifier instance of the verifier that was obtained before references were updated
     * @param collection_set collection set for the current phase
     *
     * @see CollectVerificationInfo
     * @see UpdateRefsToMovedObjects
     */
    void VerifyCollectAndMove(HeapVerifierIntoGC<LanguageConfig> &&collect_verifier,
                              const CollectionSet &collection_set);

    /**
     * Update all refs to moved objects
     */
    void UpdateRefsToMovedObjects(PandaVector<PandaVector<ObjectHeader *> *> *moved_objects_vector);

    void Sweep();

    bool IsMarked(const ObjectHeader *object) const override;

    ALWAYS_INLINE ObjectAllocatorG1<LanguageConfig::MT_MODE> *GetG1ObjectAllocator() const
    {
        return static_cast<ObjectAllocatorG1<LanguageConfig::MT_MODE> *>(this->GetObjectAllocator());
    }

    /**
     * Start process of concurrent marking
     */
    template <bool is_concurrent>
    void StartMarking(panda::GCTask &task);

    /*
     * Mark the heap in concurrent mode and calculate live bytes
     */
    void ConcurrentMark(Marker *marker, GCMarkingStackType *objects_stack, CardTableVisitFlag visit_card_table_roots,
                        const ConcurrentMarkPredicateT &pred, const ReferenceCheckPredicateT &ref_pred,
                        const MemRangeChecker &mem_range_checker);

    /**
     * ReMarks objects after Concurrent marking and actualize information about live bytes
     */
    void Remark(panda::GCTask const &task);

    /**
     * Return collectible regions
     */
    CollectionSet GetCollectibleRegions(panda::GCTask const &task, bool is_mixed);

    void UpdateCollectionSet(const CollectionSet &collectible_regions);

    void CalcLiveBytesForMovableTenuredRegions();

    /**
     * Estimate space in tenured to objects from collectible regions
     */
    bool HaveEnoughSpaceToMove(const CollectionSet &collectible_regions);

    /**
     * Check if we have enough free regions in tenured space
     */
    bool HaveEnoughRegionsToMove(size_t num);

    /**
     * Add data from SATB buffer to the object stack
     * @param object_stack - stack to add data to
     */
    void DrainSatb(GCAdaptiveStack *object_stack);

    void WaitForUpdateRemsetThread();

    void ClearSatb();

    /**
     * Iterate over object references in rem sets.
     * The Visitor is a functor which accepts an object (referee), the reference value,
     * offset of the reference in the object and the flag whether the reference is volatile.
     * The visitor can be called for the references to the collection set in the object or
     * for all references in an object which has at least one reference to the collection set.
     * The decision is implementation dependent.
     */
    template <class Visitor>
    void VisitRemSets(const Visitor &visitor);

    void CacheRefsFromRemsets();

    void ClearRefsFromRemsetsCache();

    // Issue 8183: Remove unnessesary SetYoungFullGC methods after refactoring Full GC
    void SetYoungFullGC(bool value)
    {
        is_young_full_gc_ = value;
    }

    // Issue 8183: Remove unnessesary IsYoungFullGC methods after refactoring Full GC
    bool IsYoungFullGC() const
    {
        return is_young_full_gc_;
    }

    void ActualizeRemSets();

    bool ShouldRunTenuredGC(const GCTask &task) override;

    Marker marker_;
    std::atomic<bool> concurrent_marking_flag_ {false};  //! flag indicates if we currently in concurrent marking phase
    std::atomic<bool> interrupt_concurrent_flag_ {false};  //! flag indicates if we need to interrupt concurrent marking
    std::function<void(const void *, const void *)> post_queue_func_ {nullptr};  //! function called in the post WRB
    /**
     * After first process it stores humongous objects only, after marking them it's still store them for updating
     * pointers from Humongous
     */
    PandaList<PandaVector<ObjectHeader *> *> satb_buff_list_ GUARDED_BY(satb_and_newobj_buf_lock_) {};
    PandaVector<ObjectHeader *> newobj_buffer_ GUARDED_BY(satb_and_newobj_buf_lock_);
    // The lock guards both variables: satb_buff_list_ and newobj_buffer_
    os::memory::Mutex satb_and_newobj_buf_lock_;
    UpdateRemsetThread<LanguageConfig> *update_remset_thread_ {nullptr};
    GCMarkingStackType concurrent_marking_stack_;
    std::atomic<bool> is_mixed_gc_required_ {false};
    // TODO(agrebenkin): Remove unnessesary is_young_full_gc_ field after refactoring Full GC
    bool is_young_full_gc_ {false};
    size_t number_of_mixed_tenured_regions_ {2};  //! number of tenured regions added at the young GC
    double region_garbage_rate_threshold_ {0.0};
    double g1_promotion_region_alive_rate_ {0.0};
    bool g1_track_freed_objects_ {false};
    CollectionSet collection_set_;
    // Max size of unique_refs_from_remsets_ buffer. It should be enough to store
    // almost all references to the collection set.
    // But any way there may be humongous arrays which contains a lot of references to the collection set.
    // For such objects GC don't store each reference. It just put the whole object into unique_objects_from_remsets_.
    static constexpr size_t MAX_REFS = 1024;
    // Storages for references from remsets to the collection set.
    // The list has the same number of elements as number of GC workers + GC thread.
    // Each vector is thread specific.
    // unique_refs_from_remsets_ contains an object from the remset and the offset of
    // the field which refers to the collection set.
    // Total number of collected references is limited by MAX_REFS and
    // divided between threads uniformly.
    PandaList<RefVector> unique_refs_from_remsets_;
    // unique_objects_from_remsets_ contains objects from remsets which have a reference to the collection set.
    // It is used when the limit of unique_refs_from_remsets_ is reached.
    PandaVector<ObjectHeader *> unique_objects_from_remsets_;
    os::memory::Mutex objects_from_remsets_lock_;
    PandaVector<RefVector *> unassigned_buffers_ GUARDED_BY(unassigned_buffers_lock_);
    os::memory::Mutex unassigned_buffers_lock_;
    GC::MarkPredicate calc_live_bytes_;
#ifndef NDEBUG
    bool unique_cards_initialized_ = false;
#endif                                                                     // NDEBUG
    GCG1BarrierSet::ThreadLocalCardQueues *updated_refs_queue_ {nullptr};  //! queue with updated refs info
    os::memory::Mutex queue_lock_;

    template <class LC>
    friend class RefCacheBuilder;
};

template <MTModeT MTMode>
class AllocConfig<GCType::G1_GC, MTMode> {
public:
    using ObjectAllocatorType = ObjectAllocatorG1<MTMode>;
    using CodeAllocatorType = CodeAllocator;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_G1_GC_H
