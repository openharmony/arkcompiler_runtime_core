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

#include "libpandabase/taskmanager/task_scheduler.h"
#include "runtime/mem/gc/workers/gc_workers_task_queue.h"

namespace panda::mem {

GCWorkersTaskQueue::GCWorkersTaskQueue(GC *gc) : GCWorkersTaskPool(gc)
{
    gc_task_queue_ = GetGC()->GetInternalAllocator()->New<taskmanager::TaskQueue>(
        taskmanager::TaskType::GC, taskmanager::VMType::STATIC_VM, GC_TASK_QUEUE_PRIORITY);
    ASSERT(gc_task_queue_ != nullptr);
    taskmanager::TaskScheduler::GetTaskScheduler()->RegisterQueue(gc_task_queue_);
}

GCWorkersTaskQueue::~GCWorkersTaskQueue()
{
    GetGC()->GetInternalAllocator()->Delete(gc_task_queue_);
}

bool GCWorkersTaskQueue::TryAddTask(GCWorkersTask &&task)
{
    auto gc_task_runner = [this, gc_worker_task = std::move(task)]() mutable {  // NOLINT(performance-move-const-arg)
        this->RunGCWorkersTask(&gc_worker_task);
    };
    auto gc_task = taskmanager::Task::Create(GC_TASK_PROPERTIES, gc_task_runner);
    gc_task_queue_->AddTask(std::move(gc_task));
    return true;
}

void GCWorkersTaskQueue::RunInCurrentThread()
{
    auto possible_gc_task = taskmanager::TaskScheduler::GetTaskScheduler()->GetTaskByProperties(GC_TASK_PROPERTIES);
    if (!possible_gc_task.has_value()) {
        // No available gc task for execution
        return;
    }
    // Execute gc workers task in the current thread for help to workers
    possible_gc_task->RunTask();
}

}  // namespace panda::mem
