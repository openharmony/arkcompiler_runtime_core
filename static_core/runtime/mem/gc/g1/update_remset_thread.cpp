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

#include "runtime/mem/gc/g1/update_remset_thread.h"

#include "libpandabase/utils/logger.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/object_helpers-inl.h"

namespace panda::mem {

static constexpr size_t PREALLOCATED_SET_SIZE = 256;

template <class LanguageConfig>
UpdateRemsetThread<LanguageConfig>::UpdateRemsetThread(GC *gc, PandaVM *vm,
                                                       GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                       os::memory::Mutex *queue_lock, size_t region_size,
                                                       bool update_concurrent, size_t min_concurrent_cards_to_process,
                                                       CardTable *card_table)
    : gc_(gc),
      vm_(vm),
      card_table_(card_table),
      queue_(queue),
      queue_lock_(queue_lock),
      update_concurrent_(update_concurrent),
      region_size_bits_(panda::helpers::math::GetIntLog2(region_size)),
      min_concurrent_cards_to_process_(min_concurrent_cards_to_process)

{
    cards_.reserve(PREALLOCATED_SET_SIZE);
}

template <class LanguageConfig>
UpdateRemsetThread<LanguageConfig>::~UpdateRemsetThread()
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    // Take the lock to satisfy TSAN.
    // Actually at this moment all mutators should be destroyed and the lock is not needed.
    os::memory::LockHolder holder(post_barrier_buffers_lock_);
    for (auto *buffer : post_barrier_buffers_) {
        allocator->Delete(buffer);
    }
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::CreateThread(InternalAllocatorPtr internal_allocator)
{
    ASSERT(internal_allocator != nullptr);
    if (update_concurrent_) {
        LOG(DEBUG, GC) << "Start creating UpdateRemsetThread";

        os::memory::LockHolder holder(loop_lock_);
        if (IsFlag(UpdateRemsetThreadFlags::IS_STOP_WORKER)) {
            RemoveFlag(UpdateRemsetThreadFlags::IS_STOP_WORKER);
        }
        ASSERT(update_thread_ == nullptr);
        update_thread_ = internal_allocator->New<std::thread>(&UpdateRemsetThread::ThreadLoop, this);
        int res = os::thread::SetThreadName(update_thread_->native_handle(), "UpdateRemset");
        if (res != 0) {
            LOG(ERROR, RUNTIME) << "Failed to set a name for the UpdateRemset thread";
        }
    }
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::DestroyThread()
{
    if (update_concurrent_) {
        SetFlag(UpdateRemsetThreadFlags::IS_STOP_WORKER);
        LOG(DEBUG, GC) << "Starting destroy UpdateRemsetThread";
        {
            os::memory::LockHolder holder(loop_lock_);
            thread_cond_var_.SignalAll();  // wake up update_thread & pause method
        }
        ASSERT(update_thread_ != nullptr);
        update_thread_->join();
        auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
        ASSERT(allocator != nullptr);
        allocator->Delete(update_thread_);
        update_thread_ = nullptr;
        LOG(DEBUG, GC) << "UpdateRemsetThread was destroyed";
    }
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::ThreadLoop()
{
    LOG(DEBUG, GC) << "Entering UpdateRemsetThread ThreadLoop";

    loop_lock_.Lock();
    while (true) {
        // Do one atomic load before checks
        UpdateRemsetThreadFlags iteration_flag = iteration_flag_;
        if (iteration_flag == UpdateRemsetThreadFlags::IS_STOP_WORKER) {
            LOG(DEBUG, GC) << "exit UpdateRemsetThread loop, because thread was stopped";
            break;
        }
        if (iteration_flag == UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD) {
            // UpdateRemsetThread is paused by GC, wait until GC notifies to continue work
            thread_cond_var_.Wait(&loop_lock_);
            continue;
        }
        if (iteration_flag == UpdateRemsetThreadFlags::IS_INVALIDATE_REGIONS) {
            // wait until GC-Thread invalidates regions
            thread_cond_var_.Wait(&loop_lock_);
            continue;
        }
        ASSERT(!paused_by_gc_thread_);
        ASSERT(iteration_flag == UpdateRemsetThreadFlags::IS_PROCESS_CARD);
        auto processed_cards = ProcessAllCards();

        if (processed_cards < min_concurrent_cards_to_process_) {
            Sleep();
        }
    }
    loop_lock_.Unlock();
    LOG(DEBUG, GC) << "Exiting UpdateRemsetThread ThreadLoop";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::FillFromDefered(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    os::memory::LockHolder holder(*queue_lock_);
    std::copy(cards_.begin(), cards_.end(), std::inserter(*cards, cards->end()));
    cards_.clear();
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::FillFromQueue(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    os::memory::LockHolder holder(*queue_lock_);
    std::copy(queue_->begin(), queue_->end(), std::inserter(*cards, cards->end()));
    queue_->clear();
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::FillFromThreads(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    ASSERT(vm_ != nullptr);
    auto *thread_manager = vm_->GetThreadManager();
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
void UpdateRemsetThread<LanguageConfig>::FillFromPostBarrierBuffers(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    os::memory::LockHolder holder(post_barrier_buffers_lock_);
    while (!post_barrier_buffers_.empty()) {
        auto *buffer = post_barrier_buffers_.back();
        post_barrier_buffers_.pop_back();
        FillFromPostBarrierBuffer(buffer, cards);
        allocator->Delete(buffer);
    }
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::FillFromPostBarrierBuffer(
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
void UpdateRemsetThread<LanguageConfig>::FillFromPostBarrierBuffer(GCG1BarrierSet::ThreadLocalCardQueues *post_wrb,
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
size_t UpdateRemsetThread<LanguageConfig>::ProcessAllCards()
{
    FillFromQueue(&cards_);
    FillFromThreads(&cards_);
    FillFromPostBarrierBuffers(&cards_);
    if (!cards_.empty()) {
        LOG(DEBUG, GC) << "Remset thread started process: " << cards_.size() << " cards";
    }

    size_t cards_size = 0;
    RemsetCardHandler<LanguageConfig> card_handler(card_table_, region_size_bits_, defer_cards_);
    for (auto it = cards_.begin(); it != cards_.end();) {
        if (!card_handler.Handle(*it)) {
            break;
        }
        cards_size++;

        it = cards_.erase(it);
    }
    return cards_size;
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::DrainAllCards(PandaUnorderedSet<CardTable::CardPtr> *cards)
{
    ASSERT(IsFlag(UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(loop_lock_);
    FillFromDefered(cards);
    FillFromQueue(cards);
    FillFromThreads(cards);
    FillFromPostBarrierBuffers(cards);
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::SuspendThread()
{
    ASSERT(!IsFlag(UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD));
    SetFlag(UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD);
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(true, std::memory_order_relaxed);
    // Aquare lock to be sure that UpdateRemsetThread has been stopped
    os::memory::LockHolder holder(loop_lock_);
#ifndef NDEBUG
    paused_by_gc_thread_ = true;
#endif
    LOG(DEBUG, GC) << "Remset thread has been paused";
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(false, std::memory_order_relaxed);
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::GCProcessCards()
{
    ASSERT(IsFlag(UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(loop_lock_);
    ProcessAllCards();
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::DoInvalidateRegions(RegionVector *regions)
{
    for (auto *region : *regions) {
        // don't need lock because only one thread changes remsets
        RemSet<>::template InvalidateRegion<false>(region);
    }
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::InvalidateRegions(RegionVector *regions)
{
    // Do invalidate region during concurrent sweep
    ASSERT(IsFlag(UpdateRemsetThreadFlags::IS_PROCESS_CARD));
    SetFlag(UpdateRemsetThreadFlags::IS_INVALIDATE_REGIONS);
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(true, std::memory_order_relaxed);
    // Aquare lock to be sure that UpdateRemsetThread has been stopped
    os::memory::LockHolder holder(loop_lock_);
    DoInvalidateRegions(regions);
    // Atomic with relaxed order reason: memory order is not required
    defer_cards_.store(false, std::memory_order_relaxed);
    RemoveFlag(UpdateRemsetThreadFlags::IS_INVALIDATE_REGIONS);
    // Signal to continue UpdateRemsetThread
    thread_cond_var_.Signal();
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::GCInvalidateRegions(RegionVector *regions)
{
    // Do invalidate region on pause in GCThread
    ASSERT(IsFlag(UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(loop_lock_);
    DoInvalidateRegions(regions);
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::ResumeThread()
{
    ASSERT(IsFlag(UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD));
    os::memory::LockHolder holder(loop_lock_);
#ifndef NDEBUG
    paused_by_gc_thread_ = false;
#endif
    RemoveFlag(UpdateRemsetThreadFlags::IS_PAUSED_BY_GC_THREAD);
    thread_cond_var_.Signal();
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetThread);
}  // namespace panda::mem
