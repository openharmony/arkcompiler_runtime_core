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

#include "runtime/mem/gc/g1/update_remset_worker.h"

#include "runtime/include/language_context.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/g1/card_handler.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/rem_set-inl.h"

namespace panda::mem {

template <class LanguageConfig>
UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorker(G1GC<LanguageConfig> *gc,
                                                       GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                       os::memory::Mutex *queue_lock, size_t region_size,
                                                       bool update_concurrent, size_t min_concurrent_cards_to_process)
    : gc_(gc),
      region_size_bits_(panda::helpers::math::GetIntLog2(region_size)),
      min_concurrent_cards_to_process_(min_concurrent_cards_to_process),
      queue_(queue),
      queue_lock_(queue_lock),
      update_concurrent_(update_concurrent)
{
    static constexpr size_t PREALLOCATED_CARDS_SET_SIZE = 256;
    cards_.reserve(PREALLOCATED_CARDS_SET_SIZE);
}

template <class LanguageConfig>
UpdateRemsetWorker<LanguageConfig>::~UpdateRemsetWorker()
{
    auto allocator = gc_->GetInternalAllocator();
    // Take the lock to satisfy TSAN.
    // Actually at this moment all mutators should be destroyed and the lock is not needed.
    os::memory::LockHolder holder(post_barrier_buffers_lock_);
    for (auto *buffer : post_barrier_buffers_) {
        allocator->Delete(buffer);
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::CreateWorker()
{
    if (IsFlag(UpdateRemsetWorkerFlags::IS_STOP_WORKER)) {
        RemoveFlag(UpdateRemsetWorkerFlags::IS_STOP_WORKER);
    }
    if (update_concurrent_) {
        this->CreateWorkerImpl();
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::DestroyWorker()
{
    SetFlag(UpdateRemsetWorkerFlags::IS_STOP_WORKER);
    if (update_concurrent_) {
        this->DestroyWorkerImpl();
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::AddPostBarrierBuffer(PandaVector<mem::CardTable::CardPtr> *buffer)
{
    os::memory::LockHolder holder(post_barrier_buffers_lock_);
    post_barrier_buffers_.push_back(buffer);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromDefered(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    os::memory::LockHolder holder(*queue_lock_);
    std::copy(cards_.begin(), cards_.end(), std::inserter(*cards, cards->end()));
    cards_.clear();
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromQueue(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    os::memory::LockHolder holder(*queue_lock_);
    std::copy(queue_->begin(), queue_->end(), std::inserter(*cards, cards->end()));
    queue_->clear();
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromThreads(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    auto *vm = gc_->GetPandaVm();
    ASSERT(vm != nullptr);
    auto *thread_manager = vm->GetThreadManager();
    ASSERT(thread_manager != nullptr);
    thread_manager->EnumerateThreads([this, cards](ManagedThread *thread) {
        auto *buffer = thread->GetG1PostBarrierBuffer();
        if (buffer != nullptr) {
            FillFromPostBarrierBuffer(buffer, cards);
        }
        return true;
    });
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromPostBarrierBuffers(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    auto allocator = gc_->GetInternalAllocator();
    os::memory::LockHolder holder(post_barrier_buffers_lock_);
    while (!post_barrier_buffers_.empty()) {
        auto *buffer = post_barrier_buffers_.back();
        post_barrier_buffers_.pop_back();
        FillFromPostBarrierBuffer(buffer, cards);
        allocator->Delete(buffer);
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromPostBarrierBuffer(
    GCG1BarrierSet::G1PostBarrierRingBufferType *post_wrb, PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    if (post_wrb == nullptr) {
        return;
    }
    bool has_element;
    while (true) {
        mem::CardTable::CardPtr card;
        has_element = post_wrb->TryPop(&card);
        if (!has_element) {
            break;
        }
        cards->insert(card);
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::FillFromPostBarrierBuffer(GCG1BarrierSet::ThreadLocalCardQueues *post_wrb,
                                                                   PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    while (!post_wrb->empty()) {
        cards->insert(post_wrb->back());
        post_wrb->pop_back();
    }
}

template <class LanguageConfig>
class RemsetCardHandler : public CardHandler {
public:
    RemsetCardHandler(CardTable *card_table, size_t region_size_bits, const std::atomic<bool> &defer_cards)
        : CardHandler(card_table), region_size_bits_(region_size_bits), defer_cards_(defer_cards)
    {
    }

protected:
    bool HandleObject(ObjectHeader *object_header, void *begin, void *end) override
    {
        auto obj_ref_visitor = [this](ObjectHeader *from_obj, ObjectHeader *to_obj, uint32_t offset,
                                      [[maybe_unused]] bool is_volatile) {
            ASSERT_DO(IsHeapSpace(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(to_obj)),
                      std::cerr << "Not suitable space for to_obj: " << to_obj << std::endl);

            // don't need lock because only one thread changes remsets
            RemSet<>::AddRefWithAddr<false>(from_obj, offset, to_obj);
            LOG(DEBUG, GC) << "fill rem set " << from_obj << " -> " << to_obj;
            // Atomic with relaxed order reason: memory order is not required
            return !defer_cards_.load(std::memory_order_relaxed);
        };
        return ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<true>(
            object_header, obj_ref_visitor, begin, end);
    }

private:
    size_t region_size_bits_;
    const std::atomic<bool> &defer_cards_;
};

template <class LanguageConfig>
size_t UpdateRemsetWorker<LanguageConfig>::ProcessAllCards()
{
    FillFromQueue(&cards_);
    FillFromThreads(&cards_);
    FillFromPostBarrierBuffers(&cards_);
    LOG_IF(!cards_.empty(), DEBUG, GC) << "Started process: " << cards_.size() << " cards";

    size_t cards_size = 0;
    RemsetCardHandler<LanguageConfig> card_handler(gc_->GetCardTable(), region_size_bits_, defer_cards_);
    for (auto it = cards_.begin(); it != cards_.end();) {
        if (!card_handler.Handle(*it)) {
            break;
        }
        cards_size++;

        it = cards_.erase(it);
    }
    LOG_IF(!cards_.empty(), DEBUG, GC) << "Processed " << cards_size << " cards";
    return cards_size;
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::DrainAllCards(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(update_remset_lock_);
    FillFromDefered(cards);
    FillFromQueue(cards);
    FillFromThreads(cards);
    FillFromPostBarrierBuffers(cards);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::GCProcessCards()
{
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(update_remset_lock_);
    ProcessAllCards();
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::DoInvalidateRegions(RegionVector *regions)
{
    for (auto *region : *regions) {
        // don't need lock because only one thread changes remsets
        RemSet<>::template InvalidateRegion<false>(region);
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::GCInvalidateRegions(RegionVector *regions)
{
    // Do invalidate region on pause in GCThread
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(update_remset_lock_);
    DoInvalidateRegions(regions);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::InvalidateRegions(RegionVector *regions)
{
    // Do invalidate region during concurrent sweep
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PROCESS_CARD));
    SetFlag(UpdateRemsetWorkerFlags::IS_INVALIDATE_REGIONS);
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(true, std::memory_order_relaxed);
    // Aquare lock to be sure that UpdateRemsetWorker has been stopped
    os::memory::LockHolder holder(update_remset_lock_);
    LOG(DEBUG, GC) << "Remset worker has been paused to invalidate region on concurrent";
    DoInvalidateRegions(regions);
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(false, std::memory_order_relaxed);
    RemoveFlag(UpdateRemsetWorkerFlags::IS_INVALIDATE_REGIONS);
    if (update_concurrent_) {
        this->ContinueProcessCards();
    }
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::SuspendWorkerForGCPause()
{
    ASSERT(!IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    SetFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD);
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(true, std::memory_order_relaxed);
    // Aquare lock to be sure that UpdateRemsetWorker has been stopped
    os::memory::LockHolder holder(update_remset_lock_);
#ifndef NDEBUG
    paused_by_gc_thread_ = true;
#endif
    LOG(DEBUG, GC) << "Remset worker has been paused for GC";
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(false, std::memory_order_relaxed);
}

template <class LanguageConfig>
void UpdateRemsetWorker<LanguageConfig>::ResumeWorkerAfterGCPause()
{
    ASSERT(IsFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(update_remset_lock_);
#ifndef NDEBUG
    paused_by_gc_thread_ = false;
#endif
    RemoveFlag(UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD);
    if (update_concurrent_) {
        this->ContinueProcessCards();
    }
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetWorker);
}  // namespace panda::mem
