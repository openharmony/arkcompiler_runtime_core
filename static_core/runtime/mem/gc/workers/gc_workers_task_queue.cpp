/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "libpandabase/taskmanager/task_scheduler.h"
#include "runtime/mem/gc/workers/gc_workers_task_queue.h"

namespace ark::mem {

GCWorkersTaskQueue::GCWorkersTaskQueue(GC *gc) : GCWorkersTaskPool(gc) {}

GCWorkersTaskQueue::~GCWorkersTaskQueue() = default;

bool GCWorkersTaskQueue::TryAddTask(GCWorkersTask &&task)
{
    auto gcTaskRunner = [this, gcWorkerTask = std::move(task)]() mutable {  // NOLINT(performance-move-const-arg)
        this->RunGCWorkersTask(&gcWorkerTask);
    };
    auto gcTask = taskmanager::Task::Create(GC_TASK_PROPERTIES, gcTaskRunner);
    GetGC()->GetWorkersTaskQueue()->AddTask(std::move(gcTask));
    return true;
}

void GCWorkersTaskQueue::RunInCurrentThread()
{
    taskmanager::TaskScheduler::GetTaskScheduler()->HelpWorkersWithTasks(GC_TASK_PROPERTIES);
}

}  // namespace ark::mem
