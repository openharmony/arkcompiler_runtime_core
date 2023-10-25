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

#include "runtime/mem/gc/g1/update_remset_task_queue.h"

#include "libpandabase/taskmanager/task_scheduler.h"
#include "runtime/include/language_config.h"
#include "runtime/mem/gc/g1/g1-gc.h"

namespace panda::mem {

template <class LanguageConfig>
UpdateRemsetTaskQueue<LanguageConfig>::UpdateRemsetTaskQueue(G1GC<LanguageConfig> *gc,
                                                             GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                             os::memory::Mutex *queue_lock, size_t region_size,
                                                             bool update_concurrent,
                                                             size_t min_concurrent_cards_to_process)
    : UpdateRemsetWorker<LanguageConfig>(gc, queue, queue_lock, region_size, update_concurrent,
                                         min_concurrent_cards_to_process)
{
}

template <class LanguageConfig>
UpdateRemsetTaskQueue<LanguageConfig>::~UpdateRemsetTaskQueue() = default;

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::StartProcessCards()
{
    // If we already have process cards task in task manager then no need to add a new task
    if (has_task_in_taskmanager_) {
        return;
    }
    auto update_remset_runner = [this]() {
        os::memory::LockHolder lh(this->update_remset_lock_);
        ASSERT(this->has_task_in_taskmanager_);
        // GC notified us to pause cards processing, or we are destroing runtime and notifiy STOP_WORKER
        if (!this->IsFlag(UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_PROCESS_CARD)) {
            this->has_task_in_taskmanager_ = false;
            return;
        }
        ASSERT(!this->paused_by_gc_thread_);
        this->ProcessAllCards();
        // After cards processing add new cards processing task to task manager
        this->has_task_in_taskmanager_ = false;
        this->StartProcessCards();
    };
    auto process_cards_task = taskmanager::Task::Create(UPDATE_REMSET_TASK_PROPERTIES, update_remset_runner);
    has_task_in_taskmanager_ = true;
    this->GetGC()->GetWorkersTaskQueue()->AddTask(std::move(process_cards_task));
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::ContinueProcessCards()
{
    StartProcessCards();
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::CreateWorkerImpl()
{
    os::memory::LockHolder lh(this->update_remset_lock_);
    ASSERT(!has_task_in_taskmanager_);
    StartProcessCards();
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::DestroyWorkerImpl()
{
    taskmanager::TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(UPDATE_REMSET_TASK_PROPERTIES);
    [[maybe_unused]] os::memory::LockHolder lh(this->update_remset_lock_);
    ASSERT(!has_task_in_taskmanager_);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetTaskQueue);
}  // namespace panda::mem
