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

#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/gc/g1/g1-marker.h"
#include "runtime/mem/gc/g1/collection_set.h"
#include "runtime/mem/gc/generational-gc-base.h"
#include "runtime/mem/heap_verifier.h"
#include "runtime/mem/gc/g1/g1_pause_tracker.h"
#include "runtime/mem/gc/g1/g1_analytics.h"
#include "runtime/mem/gc/g1/update_remset_worker.h"

namespace panda {
class ManagedThread;
}  // namespace panda
namespace panda::mem {

/// @brief Class for reference informantion collecting for rem-sets in G1 GC
class RefInfo {
public:
    RefInfo() = default;

    RefInfo(ObjectHeader *object, uint32_t ref_offset) : object_(object), ref_offset_(ref_offset) {}

    ~RefInfo() = default;

    ObjectHeader *GetObject() const
    {
        return object_;
    }

    uint32_t GetReferenceOffset() const
    {
        return ref_offset_;
    }

    DEFAULT_COPY_SEMANTIC(RefInfo);
    DEFAULT_MOVE_SEMANTIC(RefInfo);

private:
    ObjectHeader *object_;
    uint32_t ref_offset_;
};

/// @brief G1 alike GC
template <class LanguageConfig>
class G1GC : public GenerationalGC<LanguageConfig> {
    using RefVector = PandaVector<RefInfo>;
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;
    using MemRangeRefsChecker = std::function<bool(const MemRange &, Region *)>;
    template <bool VECTOR>
    using MovedObjectsContainer = std::conditional_t<VECTOR, PandaVector<PandaVector<ObjectHeader *> *>,
                                                     PandaVector<PandaDeque<ObjectHeader *> *>>;

public:
    explicit G1GC(ObjectAllocatorBase *object_allocator, const GCSettings &settings);

    ~G1GC() override;

    void StopGC() override
    {
        GC::StopGC();
        // GC is using update_remset_worker so we need to stop GC first before we destroy the worker
        update_remset_worker_->DestroyWorker();
    }

    NO_MOVE_SEMANTIC(G1GC);
    NO_COPY_SEMANTIC(G1GC);

    void InitGCBits(panda::ObjectHeader *obj_header) override;

    void InitGCBitsForAllocationInTLAB(panda::ObjectHeader *object) override;

    bool IsPinningSupported() const final
    {
        // G1 GC supports region pinning, so G1 can pin objects
        return true;
    }

    void WorkerTaskProcessing(GCWorkersTask *task, void *worker_data) override;

    void MarkReferences(GCMarkingStackType *references, GCPhase gc_phase) override;

    void MarkObject(ObjectHeader *object) override;

    bool MarkObjectIfNotMarked(ObjectHeader *object) override;

    bool InGCSweepRange(const ObjectHeader *object) const override;

    void OnThreadTerminate(ManagedThread *thread, mem::BuffersKeepingFlag keep_buffers) override;

    void PreZygoteFork() override;
    void PostZygoteFork() override;

    void OnWaitForIdleFail() override;

    void StartGC() override
    {
        update_remset_worker_->CreateWorker();
        GC::StartGC();
    }

    bool HasRefFromRemset(ObjectHeader *obj)
    {
        for (auto &ref_vector : unique_refs_from_remsets_) {
            auto it = std::find_if(ref_vector->cbegin(), ref_vector->cend(),
                                   [obj](auto ref) { return ref.GetObject() == obj; });
            if (it != ref_vector->cend()) {
                return true;
            }
        }
        return false;
    }

    void PostponeGCStart() override;
    void PostponeGCEnd() override;
    bool IsPostponeGCSupported() const override;

    void StartConcurrentScopeRoutine() const override;
    void EndConcurrentScopeRoutine() const override;

    void ComputeNewSize() override;

protected:
    ALWAYS_INLINE ObjectAllocatorG1<LanguageConfig::MT_MODE> *GetG1ObjectAllocator() const
    {
        return static_cast<ObjectAllocatorG1<LanguageConfig::MT_MODE> *>(this->GetObjectAllocator());
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    /// Queue with updated refs info
    GCG1BarrierSet::ThreadLocalCardQueues *updated_refs_queue_ {nullptr};
    os::memory::Mutex queue_lock_;
    os::memory::Mutex gc_worker_queue_lock_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    void CreateUpdateRemsetWorker();
    void ProcessDirtyCards();
    bool HaveGarbageRegions();

    bool HaveGarbageRegions(const PandaPriorityQueue<std::pair<uint32_t, Region *>> &regions);

    template <RegionFlag REGION_TYPE, bool FULL_GC>
    void DoRegionCompacting(Region *region, bool use_gc_workers,
                            PandaVector<PandaVector<ObjectHeader *> *> *moved_objects_vector);

    template <bool ATOMIC, bool CONCURRENTLY>
    void CollectNonRegularObjects();

    template <bool ATOMIC, bool CONCURRENTLY>
    void CollectEmptyRegions(GCTask &task, PandaVector<Region *> *empty_tenured_regions);

    template <bool ATOMIC, bool CONCURRENTLY>
    void ClearEmptyTenuredMovableRegions(PandaVector<Region *> *empty_tenured_regions);

    bool NeedToPromote(const Region *region) const;

    template <bool ATOMIC, RegionFlag REGION_TYPE>
    void RegionCompactingImpl(Region *region, const ObjectVisitor &moved_object_saver);

    template <bool ATOMIC>
    void RegionPromotionImpl(Region *region, const ObjectVisitor &moved_object_saver);

    // Return whether all cross region references were processed in mem_range
    template <typename Handler>
    void IterateOverRefsInMemRange(const MemRange &mem_range, Region *region, Handler &refs_handler);

    template <typename Visitor>
    void CacheRefsFromDirtyCards(Visitor visitor);

    void InitializeImpl() override;

    bool NeedFullGC(const panda::GCTask &task);

    bool NeedToRunGC(const panda::GCTask &task);

    void RunPhasesImpl(GCTask &task) override;

    void RunFullGC(panda::GCTask &task);

    void RunMixedGC(panda::GCTask &task, const CollectionSet &collection_set);

    /// Determine whether GC need to run concurrent mark or mixed GC
    bool ScheduleMixedGCAndConcurrentMark(panda::GCTask &task);

    /// Start concurrent mark
    void RunConcurrentMark(panda::GCTask &task);

    void RunPhasesForRegions([[maybe_unused]] panda::GCTask &task, const CollectionSet &collectible_regions);

    void PreStartupImp() override;

    void VisitCard(CardTable::CardPtr card, const ObjectVisitor &object_visitor, const CardVisitor &card_visitor);

    /// GC for young generation. Runs with STW.
    void RunGC(GCTask &task, const CollectionSet &collectible_regions);

    /// GC for tenured generation.
    void RunTenuredGC(const GCTask &task);

    /**
     * Mark predicate with calculation of live bytes in region
     * @see MarkStackCond
     *
     * @param object marked object from marking-stack
     *
     * @return true
     */
    static void CalcLiveBytesMarkPreprocess(const ObjectHeader *object, BaseClass *base_klass);

    /**
     * Mark predicate with calculation of live bytes in region, not atomically
     * @see ConcurentMarkImpl
     *
     * @param object marked object from marking-stack
     */
    static void CalcLiveBytesNotAtomicallyMarkPreprocess(const ObjectHeader *object, BaseClass *base_klass);

    /// Caches refs from remset and marks objects in collection set (young-generation + maybe some tenured regions).
    MemRange MixedMarkAndCacheRefs(const GCTask &task, const CollectionSet &collectible_regions);

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

    void MarkStackMixed(GCMarkingStackType *stack);

    void MarkStackFull(GCMarkingStackType *stack);

    bool IsInCollectionSet(ObjectHeader *object);

    /**
     * Collect dead objects in young generation and move survivors
     * @return true if moving was success, false otherwise
     */
    template <bool FULL_GC>
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

    template <bool FULL_GC, bool NEED_LOCK>
    std::conditional_t<FULL_GC, UpdateRemsetRefUpdater<LanguageConfig, NEED_LOCK>,
                       EnqueueRemsetRefUpdater<LanguageConfig>>
    CreateRefUpdater(GCG1BarrierSet::ThreadLocalCardQueues *updated_ref_queue) const;

    /// Update all refs to moved objects
    template <bool FULL_GC, bool USE_WORKERS>
    void UpdateRefsToMovedObjects(MovedObjectsContainer<FULL_GC> *moved_objects_container);

    void Sweep();

    bool IsMarked(const ObjectHeader *object) const override;

    /// Start process of on pause marking
    void FullMarking(panda::GCTask &task);

    /**
     * Marking all objects on pause
     * @param task gc task for current GC
     * @param objects_stack stack for marked objects
     * @param use_gc_workers whether do marking in parallel
     */
    void OnPauseMark(GCTask &task, GCMarkingStackType *objects_stack, bool use_gc_workers);

    /// Start process of concurrent marking
    void ConcurrentMarking(panda::GCTask &task);

    /// Iterate over roots and mark them concurrently
    NO_THREAD_SAFETY_ANALYSIS void ConcurentMarkImpl(GCMarkingStackType *objects_stack);

    void PauseTimeGoalDelay();

    void InitialMark(GCMarkingStackType &marking_stack);

    /*
     * Mark the heap in concurrent mode and calculate live bytes
     */
    void ConcurrentMark(GCMarkingStackType *objects_stack);

    /// ReMarks objects after Concurrent marking and actualize information about live bytes
    void Remark(panda::GCTask const &task);

    /// Sweep VM refs for non-regular (humongous + nonmovable) objects
    void SweepNonRegularVmRefs();

    void SweepRegularVmRefs();

    /// Return collectible regions
    CollectionSet GetCollectibleRegions(panda::GCTask const &task, bool is_mixed);
    void AddOldRegionsMaxAllowed(CollectionSet &collection_set);
    void AddOldRegionsAccordingPauseTimeGoal(CollectionSet &collection_set);

    CollectionSet GetFullCollectionSet();

    void UpdateCollectionSet(const CollectionSet &collectible_regions);

    /// Estimate space in tenured to objects from collectible regions
    bool HaveEnoughSpaceToMove(const CollectionSet &collectible_regions);

    /// Check if we have enough free regions in tenured space
    bool HaveEnoughRegionsToMove(size_t num);

    /**
     * Add data from SATB buffer to the object stack
     * @param object_stack - stack to add data to
     */
    void DrainSatb(GCAdaptiveStack *object_stack);

    void HandlePendingDirtyCards();

    void ReenqueueDirtyCards();

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

    template <class Visitor>
    void UpdateRefsFromRemSets(const Visitor &visitor);

    MemRange CacheRefsFromRemsets(const MemRangeRefsChecker &refs_checker);

    void ClearRefsFromRemsetsCache();

    void ActualizeRemSets();

    bool ShouldRunTenuredGC(const GCTask &task) override;

    void RestoreYoungCards(const CollectionSet &collection_set);

    void ClearYoungCards(const CollectionSet &collection_set);

    void ClearDirtyAndYoungCards(const MemRange &dirty_cards_range);

    size_t GetMaxMixedRegionsCount();

    void PrepareYoungRegionsForFullGC(const CollectionSet &collection_set);

    void RestoreYoungRegionsAfterFullGC(const CollectionSet &collection_set);

    template <typename Container>
    void BuildCrossYoungRemSets(const Container &young);

    size_t CalculateDesiredEdenLengthByPauseDelay();
    size_t CalculateDesiredEdenLengthByPauseDuration();

    G1GCPauseMarker<LanguageConfig> marker_;
    G1GCConcurrentMarker<LanguageConfig> conc_marker_;
    G1GCMixedMarker<LanguageConfig> mixed_marker_;
    /// Flag indicates if we currently in concurrent marking phase
    std::atomic<bool> concurrent_marking_flag_ {false};
    /// Flag indicates if we need to interrupt concurrent marking
    std::atomic<bool> interrupt_concurrent_flag_ {false};
    /// Function called in the post WRB
    std::function<void(const void *, const void *)> post_queue_func_ {nullptr};
    /**
     * After first process it stores humongous objects only, after marking them it's still store them for updating
     * pointers from Humongous
     */
    PandaList<PandaVector<ObjectHeader *> *> satb_buff_list_ GUARDED_BY(satb_and_newobj_buf_lock_) {};
    PandaVector<ObjectHeader *> newobj_buffer_ GUARDED_BY(satb_and_newobj_buf_lock_);
    // The lock guards both variables: satb_buff_list_ and newobj_buffer_
    os::memory::Mutex satb_and_newobj_buf_lock_;
    UpdateRemsetWorker<LanguageConfig> *update_remset_worker_ {nullptr};
    GCMarkingStackType concurrent_marking_stack_;
    GCMarkingStackType::MarkedObjects mixed_marked_objects_;
    std::atomic<bool> is_mixed_gc_required_ {false};
    /// Number of tenured regions added at the young GC
    size_t number_of_mixed_tenured_regions_ {2};
    double region_garbage_rate_threshold_ {0.0};
    double g1_promotion_region_alive_rate_ {0.0};
    bool g1_track_freed_objects_ {false};
    bool is_explicit_concurrent_gc_enabled_ {false};
    CollectionSet collection_set_;
    // Max size of unique_refs_from_remsets_ buffer. It should be enough to store
    // almost all references to the collection set.
    // But any way there may be humongous arrays which contains a lot of references to the collection set.
    // For such objects GC created a new RefVector, which will be cleared at the end of the collections.
    static constexpr size_t MAX_REFS = 1024;
    // Storages for references from remsets to the collection set.
    // List elements have RefVector inside, with double size compare to previous one (starts from MAX_REFS)
    // Each vector element contains an object from the remset and the offset of
    // the field which refers to the collection set.
    PandaList<RefVector *> unique_refs_from_remsets_;
    // Dirty cards which are not fully processed before collection.
    // These cards are processed later.
    PandaUnorderedSet<CardTable::CardPtr> dirty_cards_;
#ifndef NDEBUG
    bool unique_cards_initialized_ = false;
#endif  // NDEBUG
    size_t region_size_bits_;
    G1PauseTracker g1_pause_tracker_;
    os::memory::Mutex concurrent_mark_mutex_;
    os::memory::Mutex mixed_marked_objects_mutex_;
    os::memory::ConditionVariable concurrent_mark_cond_var_;
    G1Analytics analytics_;

    template <class>
    friend class RefCacheBuilder;
    friend class G1GCTest;
    friend class RemSetChecker;
};

template <MTModeT MT_MODE>
class AllocConfig<GCType::G1_GC, MT_MODE> {
public:
    using ObjectAllocatorType = ObjectAllocatorG1<MT_MODE>;
    using CodeAllocatorType = CodeAllocator;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_G1_GC_H
