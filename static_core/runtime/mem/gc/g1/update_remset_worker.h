/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_WORKER_H
#define PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_WORKER_H

#include "libpandabase/os/mutex.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/gc_barrier_set.h"

namespace panda::mem {
// Forward declarations for UpdateRemsetWorker:
template <class LanguageConfig>
class G1GC;
class Region;

template <class LanguageConfig>
class UpdateRemsetWorker {
public:
    UpdateRemsetWorker(G1GC<LanguageConfig> *gc, GCG1BarrierSet::ThreadLocalCardQueues *queue,
                       os::memory::Mutex *queue_lock, size_t region_size, bool update_concurrent,
                       size_t min_concurrent_cards_to_process);
    NO_COPY_SEMANTIC(UpdateRemsetWorker);
    NO_MOVE_SEMANTIC(UpdateRemsetWorker);

    virtual ~UpdateRemsetWorker();

    /// @brief Create necessary worker structures and start UpdateRemsetWorker
    void CreateWorker();

    /// @brief Stop UpdateRemsetWorker and destroy according worker structures
    void DestroyWorker();

    ALWAYS_INLINE void SetUpdateConcurrent(bool value)
    {
        os::memory::LockHolder holder(update_remset_lock_);
        update_concurrent_ = value;
    }

    /**
     * @brief add post-barrier buffer to process according cards in UpdateRemsetWorker
     * @param buffer post-barrier buffer
     */
    void AddPostBarrierBuffer(PandaVector<mem::CardTable::CardPtr> *buffer);

    /**
     * @brief Drain all cards to GC. Can be called only if UpdateRemsetWorker is suspended
     * @param cards pointer for saving all unprocessed cards
     */
    void DrainAllCards(PandaUnorderedSet<CardTable::CardPtr> *cards);

    /**
     * @brief Suspend UpdateRemsetWorker to reduce CPU usage during GC puase.
     * Pause cards processing in UpdateRemsetWorker
     */
    void SuspendWorkerForGCPause();

    /**
     * @brief Resume UpdateRemsetWorker execution loop.
     * Continue cards processing in UpdateRemsetWorker
     */
    virtual void ResumeWorkerAfterGCPause();

    /**
     * @brief Process all cards in the GC thread.
     * Can be called only if UpdateRemsetWorker is suspended
     */
    void GCProcessCards();

    using RegionVector = PandaVector<Region *>;

    /**
     * @brief Invalidate regions during non-pause GC phase.
     * Pause cards processing in UpdateRemsetWorker during regions invalidation
     * @param regions regions for invalidation
     */
    void InvalidateRegions(RegionVector *regions);

    /**
     * @brief Invalidate regions in the GC thread
     * Can be called only if UpdateRemsetWorker is suspended
     * @param regions regions for invalidation
     */
    void GCInvalidateRegions(RegionVector *regions);

#ifndef NDEBUG
    // only debug purpose
    ALWAYS_INLINE size_t GetQueueSize() const
    {
        os::memory::LockHolder holder(*queue_lock_);
        return queue_->size();
    }
#endif  // NDEBUG

protected:
    /// @brief Create implementation specific structures and start UpdateRemsetWorker in according with these
    virtual void CreateWorkerImpl() = 0;

    /// @brief Stop implementation specific UpdateRemsetWorker and destroy according structures
    virtual void DestroyWorkerImpl() = 0;

    /// @brief Try to process all unprocessed cards in one thread
    size_t ProcessAllCards() REQUIRES(update_remset_lock_);

    /**
     * @brief Notify UpdateRemsetWorker to continue process cards
     * @see SuspendWorkerForGCPause
     * @see ResumeWorkerAfterGCPause
     * @see InvalidateRegions
     */
    virtual void ContinueProcessCards() REQUIRES(update_remset_lock_) = 0;

    ALWAYS_INLINE G1GC<LanguageConfig> *GetGC() const
    {
        return gc_;
    }

    /// @enum UpdateRemsetWorkerFlags is special flags for indicating of process kind in UpdateRemsetWorker
    enum class UpdateRemsetWorkerFlags : uint32_t {
        IS_PROCESS_CARD = 0U,               ///< Special value for main work (process cards) in update remset worker
        IS_STOP_WORKER = 1U,                ///< Update remset worker is in destroying process
        IS_PAUSED_BY_GC_THREAD = 1U << 1U,  ///< Update remset worker is paused by GCThread
        IS_INVALIDATE_REGIONS = 1U << 2U,   ///< Update remset worker is invalidating regions
    };

    ALWAYS_INLINE bool IsFlag(UpdateRemsetWorkerFlags value) const
    {
        return iteration_flag_ == value;
    }

    ALWAYS_INLINE void SetFlag(UpdateRemsetWorkerFlags value)
    {
        iteration_flag_ = value;
    }

    ALWAYS_INLINE void RemoveFlag([[maybe_unused]] UpdateRemsetWorkerFlags value)
    {
        ASSERT(iteration_flag_ == value);
        iteration_flag_ = UpdateRemsetWorkerFlags::IS_PROCESS_CARD;
    }

    ALWAYS_INLINE UpdateRemsetWorkerFlags GetFlag() const
    {
        return iteration_flag_;
    }

    ALWAYS_INLINE size_t GetMinConcurrentCardsToProcess() const
    {
        return min_concurrent_cards_to_process_;
    }

    /**
     * We use this lock to synchronize UpdateRemsetWorker and external operations with it
     * (as SuspendWorkerForGCPause/ResumeWorkerAfterGCPause/DestroyWorker/etc), wait and notify this worker.
     */
    os::memory::Mutex update_remset_lock_;  // NOLINT(misc-non-private-member-variables-in-classes)

#ifndef NDEBUG
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<bool> paused_by_gc_thread_ GUARDED_BY(update_remset_lock_) {false};
#endif

private:
    void FillFromDefered(PandaUnorderedSet<CardTable::CardPtr> *cards) REQUIRES(update_remset_lock_);
    void FillFromQueue(PandaUnorderedSet<CardTable::CardPtr> *cards) REQUIRES(update_remset_lock_);
    void FillFromThreads(PandaUnorderedSet<CardTable::CardPtr> *cards) REQUIRES(update_remset_lock_);

    void FillFromPostBarrierBuffers(PandaUnorderedSet<CardTable::CardPtr> *cards);
    void FillFromPostBarrierBuffer(GCG1BarrierSet::G1PostBarrierRingBufferType *post_wrb,
                                   PandaUnorderedSet<CardTable::CardPtr> *cards);
    void FillFromPostBarrierBuffer(GCG1BarrierSet::ThreadLocalCardQueues *post_wrb,
                                   PandaUnorderedSet<CardTable::CardPtr> *cards);

    void DoInvalidateRegions(RegionVector *regions) REQUIRES(update_remset_lock_);

    G1GC<LanguageConfig> *gc_ {nullptr};
    size_t region_size_bits_;
    size_t min_concurrent_cards_to_process_;
    GCG1BarrierSet::ThreadLocalCardQueues *queue_ GUARDED_BY(queue_lock_) {nullptr};
    os::memory::Mutex *queue_lock_ {nullptr};
    bool update_concurrent_;  // used to process references in gc-thread between zygote phases

    PandaUnorderedSet<CardTable::CardPtr> cards_;
    PandaVector<GCG1BarrierSet::ThreadLocalCardQueues *> post_barrier_buffers_ GUARDED_BY(post_barrier_buffers_lock_);
    os::memory::Mutex post_barrier_buffers_lock_;

    std::atomic<UpdateRemsetWorkerFlags> iteration_flag_ {UpdateRemsetWorkerFlags::IS_STOP_WORKER};

    // We do not fully update remset during collection pause
    std::atomic<bool> defer_cards_ {false};
};

template <class LanguageConfig>
class SuspendUpdateRemsetWorkerScope {
public:
    explicit SuspendUpdateRemsetWorkerScope(UpdateRemsetWorker<LanguageConfig> *worker) : worker_(worker)
    {
        worker_->SuspendWorkerForGCPause();
    }
    ~SuspendUpdateRemsetWorkerScope()
    {
        worker_->ResumeWorkerAfterGCPause();
    }
    NO_COPY_SEMANTIC(SuspendUpdateRemsetWorkerScope);
    NO_MOVE_SEMANTIC(SuspendUpdateRemsetWorkerScope);

private:
    UpdateRemsetWorker<LanguageConfig> *worker_ {nullptr};
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_WORKER_H
