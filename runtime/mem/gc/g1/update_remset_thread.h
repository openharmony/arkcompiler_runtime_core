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

#ifndef PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H
#define PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H

#include "libpandabase/macros.h"
#include "libpandabase/os/mutex.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/card_table-inl.h"

namespace panda::mem {

// template <typename T>
class GC;

constexpr bool REMSET_THREAD_USE_STATS = false;

class RemsetThreadStats {
public:
    void IncAddedCardToQueue(size_t value = 1)
    {
        if (REMSET_THREAD_USE_STATS) {
            added_cards_to_queue += value;
        }
    }

    void IncProcessedConcurrentCards(const PandaUnorderedSet<CardTable::CardPtr> &cards)
    {
        if (REMSET_THREAD_USE_STATS) {
            processed_concurrent_cards += cards.size();
            for (const auto &card : cards) {
                unique_cards.insert(card);
            }
        }
    }

    void IncProcessedAtSTWCards(const PandaUnorderedSet<CardTable::CardPtr> &cards)
    {
        if (REMSET_THREAD_USE_STATS) {
            processed_at_stw_cards += cards.size();
            for (const auto &card : cards) {
                unique_cards.insert(card);
            }
        }
    }

    void Reset()
    {
        added_cards_to_queue = processed_concurrent_cards = processed_at_stw_cards = 0;
        unique_cards.clear();
    }

    void PrintStats() const
    {
        if (REMSET_THREAD_USE_STATS) {
            LOG(DEBUG, GC) << "remset thread stats: "
                           << "added_cards_to_queue: " << added_cards_to_queue
                           << " processed_concurrent_cards: " << processed_concurrent_cards
                           << " processed_at_stw_cards: " << processed_at_stw_cards
                           << " uniq_cards_processed: " << unique_cards.size();
        }
    }

private:
    std::atomic<size_t> added_cards_to_queue {0};
    std::atomic<size_t> processed_concurrent_cards {0};
    std::atomic<size_t> processed_at_stw_cards {0};
    PandaUnorderedSet<CardTable::CardPtr> unique_cards;
};

template <class LanguageConfig>
class UpdateRemsetThread {
public:
    explicit UpdateRemsetThread(GC *gc, PandaVM *vm, GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                os::memory::Mutex *queue_lock, size_t region_size, bool update_concurrent,
                                size_t min_concurrent_cards_to_process, CardTable *card_table);
    ~UpdateRemsetThread() = default;
    NO_COPY_SEMANTIC(UpdateRemsetThread);
    NO_MOVE_SEMANTIC(UpdateRemsetThread);

    void CreateThread(InternalAllocatorPtr internal_allocator);

    void DestroyThread();

    void StartThread();

    /**
     * Blocking call until all tasks are not processed
     */
    void WaitUntilTasksEnd();

    void ThreadLoop();

    // only debug purpose
    size_t GetQueueSize() const
    {
        os::memory::LockHolder holder(*queue_lock_);
        return queue_->size();
    }

    void SetUpdateConcurrent(bool value)
    {
        os::memory::LockHolder holder(loop_lock);
        update_concurrent_ = value;
    }

    void InvalidateRegions(PandaVector<Region *> *regions)
    {
        need_invalidate_region_ = true;
        {
            os::memory::LockHolder holder(loop_lock);
            invalidate_regions_ = regions;
        }
        thread_cond_var_.Signal();
        {
            os::memory::LockHolder holder(loop_lock);
            while (invalidate_regions_ != nullptr) {
                Sleep();
            }
        }
        need_invalidate_region_ = false;
        thread_cond_var_.Signal();
    }

private:
    void HandleCard(CardTable::CardPtr card_ptr) REQUIRES(loop_lock);

    void BuildSetFromQueue() REQUIRES(loop_lock);

    void DrainThreads() REQUIRES(loop_lock);
    void DrainThread(ManagedThread *thread);

    size_t ProcessAllCards() REQUIRES(loop_lock);

    void Sleep()
    {
        static constexpr uint64_t SLEEP_MS = 1;
        thread_cond_var_.TimedWait(&loop_lock, SLEEP_MS);
    }

    GC *gc_ {nullptr};
    PandaVM *vm_ {nullptr};
    CardTable *card_table_ {nullptr};
    GCG1BarrierSet::ThreadLocalCardQueues *queue_ GUARDED_BY(queue_lock_) {nullptr};
    os::memory::Mutex *queue_lock_ {nullptr};
    PandaUnorderedSet<CardTable::CardPtr> cards_;
    PandaVector<Region *> *invalidate_regions_ GUARDED_BY(loop_lock) {nullptr};
    /*
     * We use this lock to synchronize UpdateRemSet and external operations with it (as
     * WaitUntilTasksEnd/DestroyThread/etc), wait and notify this thread.
     */
    os::memory::Mutex loop_lock;
    bool update_concurrent_;  // used to process reference in gc-thread between zygote phases
    size_t region_size_bits_;
    size_t min_concurrent_cards_to_process_;
    std::thread *update_thread_ {nullptr};
    os::memory::ConditionVariable thread_cond_var_;
    std::atomic<bool> stop_thread_ {false};
    std::atomic<bool> pause_thread_ {false};
    std::atomic<bool> need_invalidate_region_ {false};

    RemsetThreadStats stats_;
};
}  // namespace panda::mem
#endif  // PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H
