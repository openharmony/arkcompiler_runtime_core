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

namespace ark::mem {

template <class LanguageConfig>
UpdateRemsetTaskQueue<LanguageConfig>::UpdateRemsetTaskQueue(G1GC<LanguageConfig> *gc,
                                                             GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                             os::memory::Mutex *queueLock, size_t regionSize,
                                                             bool updateConcurrent, size_t minConcurrentCardsToProcess)
    : UpdateRemsetWorker<LanguageConfig>(gc, queue, queueLock, regionSize, updateConcurrent,
                                         minConcurrentCardsToProcess)
{
}

template <class LanguageConfig>
UpdateRemsetTaskQueue<LanguageConfig>::~UpdateRemsetTaskQueue() = default;

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::StartProcessCards()
{
    // If we already have process cards task in task manager then no need to add a new task
    if (hasTaskInTaskmanager_) {
        return;
    }
    auto updateRemsetRunner = [this]() {
        os::memory::LockHolder lh(this->updateRemsetLock_);
        ASSERT(this->hasTaskInTaskmanager_);
        // GC notified us to pause cards processing, or we are destroing runtime and notifiy STOP_WORKER
        if (!this->IsFlag(UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_PROCESS_CARD)) {
            this->hasTaskInTaskmanager_ = false;
            return;
        }
        ASSERT(!this->pausedByGcThread_);
        this->ProcessAllCards();
        // After cards processing add new cards processing task to task manager
        this->hasTaskInTaskmanager_ = false;
        this->StartProcessCards();
    };
    auto processCardsTask = taskmanager::Task::Create(UPDATE_REMSET_TASK_PROPERTIES, updateRemsetRunner);
    hasTaskInTaskmanager_ = true;
    this->GetGC()->GetWorkersTaskQueue()->AddTask(std::move(processCardsTask));
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::ContinueProcessCards()
{
    StartProcessCards();
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::CreateWorkerImpl()
{
    os::memory::LockHolder lh(this->updateRemsetLock_);
    ASSERT(!hasTaskInTaskmanager_);
    StartProcessCards();
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::DestroyWorkerImpl()
{
    taskmanager::TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(UPDATE_REMSET_TASK_PROPERTIES);
    [[maybe_unused]] os::memory::LockHolder lh(this->updateRemsetLock_);
    ASSERT(!hasTaskInTaskmanager_);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetTaskQueue);
}  // namespace ark::mem
