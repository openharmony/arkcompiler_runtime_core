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

#ifndef PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H
#define PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H

#include "runtime/mem/gc/g1/update_remset_worker.h"

#include "libpandabase/macros.h"
#include "libpandabase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"

namespace panda::mem {

constexpr bool REMSET_THREAD_USE_STATS = false;

class RemsetThreadStats {
public:
    void IncAddedCardToQueue(size_t value = 1)
    {
        if (REMSET_THREAD_USE_STATS) {
            added_cards_to_queue_ += value;
        }
    }

    void IncProcessedConcurrentCards(const PandaUnorderedSet<CardTable::CardPtr> &cards)
    {
        if (REMSET_THREAD_USE_STATS) {
            processed_concurrent_cards_ += cards.size();
            for (const auto &card : cards) {
                unique_cards_.insert(card);
            }
        }
    }

    void IncProcessedAtSTWCards(const PandaUnorderedSet<CardTable::CardPtr> &cards)
    {
        if (REMSET_THREAD_USE_STATS) {
            processed_at_stw_cards_ += cards.size();
            for (const auto &card : cards) {
                unique_cards_.insert(card);
            }
        }
    }

    void Reset()
    {
        added_cards_to_queue_ = processed_concurrent_cards_ = processed_at_stw_cards_ = 0;
        unique_cards_.clear();
    }

    void PrintStats() const
    {
        if (REMSET_THREAD_USE_STATS) {
            LOG(DEBUG, GC) << "remset thread stats: "
                           << "added_cards_to_queue: " << added_cards_to_queue_
                           << " processed_concurrent_cards: " << processed_concurrent_cards_
                           << " processed_at_stw_cards: " << processed_at_stw_cards_
                           << " uniq_cards_processed: " << unique_cards_.size();
        }
    }

private:
    std::atomic<size_t> added_cards_to_queue_ {0};
    std::atomic<size_t> processed_concurrent_cards_ {0};
    std::atomic<size_t> processed_at_stw_cards_ {0};
    PandaUnorderedSet<CardTable::CardPtr> unique_cards_;
};

template <class LanguageConfig>
class UpdateRemsetThread final : public UpdateRemsetWorker<LanguageConfig> {
public:
    explicit UpdateRemsetThread(G1GC<LanguageConfig> *gc, GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                os::memory::Mutex *queue_lock, size_t region_size, bool update_concurrent,
                                size_t min_concurrent_cards_to_process);
    ~UpdateRemsetThread() final;
    NO_COPY_SEMANTIC(UpdateRemsetThread);
    NO_MOVE_SEMANTIC(UpdateRemsetThread);

private:
    void CreateWorkerImpl() final;
    void DestroyWorkerImpl() final;
    void ContinueProcessCards() REQUIRES(this->update_remset_lock_) final;

    void ThreadLoop();

    void Sleep() REQUIRES(this->update_remset_lock_)
    {
        static constexpr uint64_t SLEEP_MS = 1;
        thread_cond_var_.TimedWait(&this->update_remset_lock_, SLEEP_MS);
    }

    /* Thread specific variables */
    std::thread *update_thread_ {nullptr};
    os::memory::ConditionVariable thread_cond_var_ GUARDED_BY(this->update_remset_lock_);
    RemsetThreadStats stats_;
};

}  // namespace panda::mem
#endif  // PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_THREAD_H
