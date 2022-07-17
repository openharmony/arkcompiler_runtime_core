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

#include "update_remset_thread.h"
#include "libpandabase/utils/logger.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_phase.h"

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
void UpdateRemsetThread<LanguageConfig>::CreateThread(InternalAllocatorPtr internal_allocator)
{
    ASSERT(internal_allocator != nullptr);
    if (update_concurrent_) {
        LOG(DEBUG, GC) << "Start creating UpdateRemsetThread";

        os::memory::LockHolder holder(loop_lock);
        stop_thread_ = false;
        // dont reset pause_thread_ here WaitUntilTasksEnd does it, and we can reset pause_thread_ by accident here,
        // because we set it without lock
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
        stop_thread_ = true;
        LOG(DEBUG, GC) << "Starting destroy UpdateRemsetThread";
        {
            os::memory::LockHolder holder(loop_lock);
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
void UpdateRemsetThread<LanguageConfig>::StartThread()
{
    if (update_concurrent_) {
        LOG(DEBUG, GC) << "Start UpdateRemsetThread";
        {
            os::memory::LockHolder holder(loop_lock);
            ASSERT(update_thread_ != nullptr);
            pause_thread_ = false;
            thread_cond_var_.Signal();
        }
    }
}

// TODO(alovkov): GC-thread can help to update-thread to process all cards concurrently
template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::WaitUntilTasksEnd()
{
    pause_thread_ = true;  // either ThreadLoop should set it to false, or this function if we don't have a thread
    if (update_concurrent_ && !stop_thread_) {
        LOG(DEBUG, GC) << "Starting pause UpdateRemsetThread";

        os::memory::LockHolder holder(loop_lock);
        while (pause_thread_) {
            // runtime is destroying, handle all refs anyway for now
            if (stop_thread_ || update_thread_ == nullptr) {
                ProcessAllCards();  // Process all cards inside gc
                pause_thread_ = false;
                break;
            }
            thread_cond_var_.Signal();
            thread_cond_var_.Wait(&loop_lock);
        }
        ASSERT(GetQueueSize() == 0);
    } else {
        os::memory::LockHolder holder(loop_lock);
        // we will handle all remsets even when thread is stopped (we are trying to destroy Runtime, but it's the last
        // GC), try to eliminate it in the future for faster shutdown
        ProcessAllCards();  // Process all cards inside gc
        pause_thread_ = false;
    }
    thread_cond_var_.Signal();
    stats_.PrintStats();
    stats_.Reset();
    ASSERT(GetQueueSize() == 0);
    ASSERT(!pause_thread_);
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::ThreadLoop()
{
    LOG(DEBUG, GC) << "Entering UpdateRemsetThread ThreadLoop";

    loop_lock.Lock();
    while (true) {
        if (stop_thread_) {
            LOG(DEBUG, GC) << "exit UpdateRemsetThread loop, because thread was stopped";
            break;
        }
        if (pause_thread_) {
            // gc is waiting for us to handle all updates
            // possible improvements: let GC thread to help us to handle elements in queue in parallel, instead of
            // waiting
            ProcessAllCards();  // Process all cards inside gc
            pause_thread_ = false;
            thread_cond_var_.Signal();          // notify GC thread that we processed all updates
            thread_cond_var_.Wait(&loop_lock);  // let WaitUntilTasksEnd to finish
            continue;
        }
        if (invalidate_regions_ != nullptr) {
            for (const auto &region : *invalidate_regions_) {
                // don't need lock because only update_remset_thread changes remsets
                RemSet<>::template InvalidateRegion<false>(region);
            }
            invalidate_regions_ = nullptr;
            thread_cond_var_.Signal();
            Sleep();
        }
        if (need_invalidate_region_) {
            Sleep();
            continue;
        }
        auto processed_cards = ProcessAllCards();

        auto phase = gc_->GetGCPhase();
        if (phase != GCPhase::GC_PHASE_IDLE) {  // means GC is in progress now and we need to process more aggressively
            continue;
        } else if (processed_cards < min_concurrent_cards_to_process_) {  // NOLINT(readability-else-after-return)
            Sleep();
        }
    }
    loop_lock.Unlock();
    LOG(DEBUG, GC) << "Exiting UpdateRemsetThread ThreadLoop";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::HandleCard(CardTable::CardPtr card_ptr)
{
    LOG(DEBUG, GC) << "HandleCard card: " << ToVoidPtr(card_table_->GetCardStartAddress(card_ptr));
    // TODO(alovkov): eliminate atomics in cards, force FullMemoryBarrier here, all mutators will be faster

    // clear card before we process it, because parallel mutator thread can make a write and we would miss it
    card_ptr->Clear();

    auto rem_set_fill = [this](void *mem) {
        auto object_header = static_cast<ObjectHeader *>(mem);
        auto obj_ref_visitor = [this](ObjectHeader *from_obj, ObjectHeader *to_obj) {
            auto to_obj_space_type = PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(to_obj);

            switch (to_obj_space_type) {
                case SpaceType::SPACE_TYPE_OBJECT:
                case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
                case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
                    if (((ToUintPtr(from_obj) ^ ToUintPtr(to_obj)) >> region_size_bits_) != 0) {
                        // don't need lock because only update_remset_thread changes remsets
                        RemSet<>::AddRefWithAddr<false>(from_obj, to_obj);
                        LOG(DEBUG, GC) << "fill rem set " << std::hex << from_obj << " -> " << to_obj;
                    }
                    break;
                default:
                    LOG(FATAL, GC) << "Not suitable space for to_obj: " << std::hex << to_obj;
                    break;
            }
        };
        if (object_header->ClassAddr<BaseClass>() != nullptr) {
            // Class may be null when we are visiting a card and at the same time a new non-movable object
            // is allocated in the memory region covered by the card.
            ObjectHelpers<LanguageConfig::LANG_TYPE>::TraverseAllObjects(object_header, obj_ref_visitor);
        }
    };
    // process all cards
    auto mmap_mempool = PoolManager::GetMmapMemPool();
    auto space_type = mmap_mempool->GetSpaceTypeForAddr(ToVoidPtr(card_table_->GetCardStartAddress(card_ptr)));
    switch (space_type) {
        case SpaceType::SPACE_TYPE_OBJECT:
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT: {
            auto region = AddrToRegion(ToVoidPtr(card_table_->GetCardStartAddress(card_ptr)));
            ASSERT(region != nullptr);
            ASSERT(region->GetLiveBitmap() != nullptr);
            region->GetLiveBitmap()->IterateOverMarkedChunkInRange<true>(
                ToVoidPtr(card_table_->GetCardStartAddress(card_ptr)),
                ToVoidPtr(card_table_->GetCardEndAddress(card_ptr)), rem_set_fill);
            break;
        }
        default:
            LOG(FATAL, GC) << "Invalid space type for the " << ToVoidPtr(card_table_->GetCardStartAddress(card_ptr));
            break;
    };
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::BuildSetFromQueue()
{
    ASSERT(cards_.empty());
    os::memory::LockHolder holder(*queue_lock_);
    for (const auto &card : *queue_) {
        cards_.insert(card);
    }
    queue_->clear();
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::DrainThreads()
{
    ASSERT(vm_ != nullptr);
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (LanguageConfig::MT_MODE == MT_MODE_SINGLE) {
        auto thread = vm_->GetAssociatedThread();
        DrainThread(thread);
    } else if (LanguageConfig::MT_MODE == MT_MODE_MULTI) {  // NOLINT(readability-misleading-indentation)
        auto *thread_manager = vm_->GetThreadManager();
        ASSERT(thread_manager != nullptr);
        // TODO(alovkov) if !inside_gc + too many from thread -> go to next thread.
        thread_manager->EnumerateThreads(
            [this](ManagedThread *thread) {
                DrainThread(thread);
                return true;
            },
            static_cast<unsigned int>(EnumerationFlag::ALL));
    } else {
        UNREACHABLE();
    }
}

template <class LanguageConfig>

void UpdateRemsetThread<LanguageConfig>::DrainThread(ManagedThread *thread)
{
    if (thread == nullptr) {  // possible for ECMA
        return;
    }
    auto cards = thread->GetG1PostBarrierBuffer();
    if (cards == nullptr) {
        return;
    }
    bool has_element;
    while (true) {
        mem::CardTable::CardPtr card;
        has_element = cards->TryPop(&card);
        if (!has_element) {
            break;
        }
        cards_.insert(card);
    }
}

template <class LanguageConfig>
size_t UpdateRemsetThread<LanguageConfig>::ProcessAllCards()
{
    BuildSetFromQueue();
    DrainThreads();
    auto cards_size = cards_.size();
    if (!cards_.empty()) {
        LOG(DEBUG, GC) << "Remset thread started process: " << cards_size << " cards";
    }
    for (const auto &card : cards_) {
        HandleCard(card);
    }
    cards_.clear();
    return cards_size;
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetThread);
}  // namespace panda::mem
