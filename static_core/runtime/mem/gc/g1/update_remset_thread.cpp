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
#include "runtime/include/language_config.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/g1/update_remset_worker.h"

namespace panda::mem {

template <class LanguageConfig>
UpdateRemsetThread<LanguageConfig>::UpdateRemsetThread(G1GC<LanguageConfig> *gc,
                                                       GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                       os::memory::Mutex *queue_lock, size_t region_size,
                                                       bool update_concurrent, size_t min_concurrent_cards_to_process)
    : UpdateRemsetWorker<LanguageConfig>(gc, queue, queue_lock, region_size, update_concurrent,
                                         min_concurrent_cards_to_process)
{
}

template <class LanguageConfig>
UpdateRemsetThread<LanguageConfig>::~UpdateRemsetThread() = default;

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::CreateWorkerImpl()
{
    LOG(DEBUG, GC) << "Start creating UpdateRemsetThread";
    os::memory::LockHolder lk(this->update_remset_lock_);
    auto internal_allocator = this->GetGC()->GetInternalAllocator();
    ASSERT(update_thread_ == nullptr);
    update_thread_ = internal_allocator->template New<std::thread>(&UpdateRemsetThread::ThreadLoop, this);
    int res = os::thread::SetThreadName(update_thread_->native_handle(), "UpdateRemset");

    LOG_IF(res != 0, ERROR, RUNTIME) << "Failed to set a name for the UpdateRemset thread";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::DestroyWorkerImpl()
{
    LOG(DEBUG, GC) << "Starting destroy UpdateRemsetThread";
    {
        os::memory::LockHolder holder(this->update_remset_lock_);
        thread_cond_var_.SignalAll();  // wake up updateremset worker & pause method
    }
    ASSERT(update_thread_ != nullptr);
    update_thread_->join();
    auto allocator = this->GetGC()->GetInternalAllocator();
    ASSERT(allocator != nullptr);
    allocator->Delete(update_thread_);
    update_thread_ = nullptr;
    LOG(DEBUG, GC) << "UpdateRemsetThread was destroyed";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::ThreadLoop()
{
    LOG(DEBUG, GC) << "Entering UpdateRemsetThread ThreadLoop";

    this->update_remset_lock_.Lock();
    while (true) {
        // Do one atomic load before checks
        auto iteration_flag = this->GetFlag();
        if (iteration_flag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_STOP_WORKER) {
            LOG(DEBUG, GC) << "exit UpdateRemsetThread loop, because thread was stopped";
            break;
        }
        if (iteration_flag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD) {
            // UpdateRemsetThread is paused by GC, wait until GC notifies to continue work
            thread_cond_var_.Wait(&this->update_remset_lock_);
            continue;
        }
        if (iteration_flag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_INVALIDATE_REGIONS) {
            // wait until GC-Thread invalidates regions
            thread_cond_var_.Wait(&this->update_remset_lock_);
            continue;
        }
        ASSERT(!this->paused_by_gc_thread_);
        ASSERT(iteration_flag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_PROCESS_CARD);
        auto processed_cards = this->ProcessAllCards();

        if (processed_cards < this->GetMinConcurrentCardsToProcess()) {
            Sleep();
        }
    }
    this->update_remset_lock_.Unlock();
    LOG(DEBUG, GC) << "Exiting UpdateRemsetThread ThreadLoop";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::ContinueProcessCards()
{
    // Signal to continue UpdateRemsetThread
    thread_cond_var_.Signal();
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetThread);
}  // namespace panda::mem
