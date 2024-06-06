/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_taskpool.h"

namespace ark::ets {

Taskpool::Taskpool() : taskId_(1) {}

EtsLong Taskpool::GenerateTaskId()
{
    return taskId_++;
}

void Taskpool::TaskSubmitted(EtsLong taskId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    waitingTasks_[taskId]++;
}

size_t Taskpool::DecrementTaskCounter(EtsLong taskId, PandaUnorderedMap<EtsLong, size_t> &tasks)
{
    auto taskIter = tasks.find(taskId);
    ASSERT(taskIter != tasks.end());
    auto tasksCount = --(taskIter->second);
    if (tasksCount == 0) {
        tasks.erase(taskIter);
    }
    return tasksCount;
}

bool Taskpool::TaskStarted(uint32_t coroutineId, EtsLong taskId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    auto waitingTasksCountWithId = DecrementTaskCounter(taskId, waitingTasks_);
    if (tasksToBeCanceled_.find(taskId) != tasksToBeCanceled_.end()) {
        if ((waitingTasksCountWithId == 0) && (runningTasks_.find(taskId) == runningTasks_.end())) {
            // No tasks with this id in the taskpool
            tasksToBeCanceled_.erase(taskId);
        }
        // escompat.Error will be occurred for the current task
        return false;
    }
    executingTasks_.insert({coroutineId, taskId});
    runningTasks_[taskId]++;
    return true;
}

bool Taskpool::TaskFinished(uint32_t coroutineId, EtsLong taskId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    ASSERT(executingTasks_[coroutineId] == taskId);
    executingTasks_.erase(coroutineId);
    auto runningTasksCountWithId = DecrementTaskCounter(taskId, runningTasks_);
    if (tasksToBeCanceled_.find(taskId) != tasksToBeCanceled_.end()) {
        if ((runningTasksCountWithId == 0) && (waitingTasks_.find(taskId) == waitingTasks_.end())) {
            // No tasks with this id in the taskpool
            tasksToBeCanceled_.erase(taskId);
        }
        // escompat.Error will be occurred for the current task
        return false;
    }
    return true;
}

bool Taskpool::CancelTask(EtsLong taskId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    if ((waitingTasks_.find(taskId) == waitingTasks_.end()) && (runningTasks_.find(taskId) == runningTasks_.end())) {
        // No tasks with this id in the taskpool, escompat.Error will be occurred by taskpool.cancel
        return false;
    }
    tasksToBeCanceled_.insert(taskId);
    return true;
}

bool Taskpool::IsTaskCanceled(uint32_t coroutineId) const
{
    os::memory::LockHolder lh(taskpoolLock_);
    auto it = executingTasks_.find(coroutineId);
    if (it == executingTasks_.end()) {
        // Current coroutine is not executing a task
        return false;
    }
    auto taskId = it->second;
    return tasksToBeCanceled_.find(taskId) != tasksToBeCanceled_.end();
}

}  // namespace ark::ets
