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

#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace panda::mem {

void GCWorkersTaskPool::IncreaseSolvedTasks()
{
    solved_tasks_++;
    if (solved_tasks_ == sended_tasks_) {
        os::memory::LockHolder lock(all_solved_tasks_cond_var_lock_);
        all_solved_tasks_cond_var_.Signal();
    }
}

bool GCWorkersTaskPool::AddTask(GCWorkersTask &&task)
{
    sended_tasks_++;
    [[maybe_unused]] GCWorkersTaskTypes type = task.GetType();
    if (this->TryAddTask(std::forward<GCWorkersTask &&>(task))) {
        LOG(DEBUG, GC) << "Added a new " << GCWorkersTaskTypesToString(type) << " to gc task pool";
        return true;
    }
    // Couldn't add gc workers task to pool
    sended_tasks_--;
    return false;
}

void GCWorkersTaskPool::RunGCWorkersTask(GCWorkersTask *task, void *worker_data)
{
    gc_->WorkerTaskProcessing(task, worker_data);
    IncreaseSolvedTasks();
}

void GCWorkersTaskPool::WaitUntilTasksEnd()
{
    for (;;) {
        // Current thread can to help workers with gc task processing.
        // Try to get gc workers task from pool if it's possible and run the task immediately
        this->RunInCurrentThread();
        os::memory::LockHolder lock(all_solved_tasks_cond_var_lock_);
        // If all sended task were solved then break from wait loop
        if (solved_tasks_ == sended_tasks_) {
            break;
        }
        all_solved_tasks_cond_var_.TimedWait(&all_solved_tasks_cond_var_lock_, ALL_GC_TASKS_FINISH_WAIT_TIMEOUT);
    }
    ResetTasks();
}

}  // namespace panda::mem
