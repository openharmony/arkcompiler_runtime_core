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

Taskpool::Taskpool() : taskId_(1), taskGroupId_(1) {}

EtsLong Taskpool::GenerateTaskId()
{
    return taskId_++;
}

EtsLong Taskpool::GenerateTaskGroupId()
{
    return taskGroupId_++;
}

EtsLong Taskpool::GenerateSeqRunnerId()
{
    return seqRunnerId_++;
}

void Taskpool::TaskSubmitted(EtsLong taskId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    waitingTasks_[taskId]++;
}

void Taskpool::GroupSubmitted(EtsLong groupId, size_t tasksCount)
{
    ASSERT(tasksCount != 0);
    os::memory::LockHolder lh(taskpoolLock_);
    waitingGroupTasks_[groupId] += tasksCount;
}

size_t Taskpool::DecrementTaskCounter(EtsLong id, PandaUnorderedMap<EtsLong, size_t> &tasks)
{
    auto taskIter = tasks.find(id);
    ASSERT(taskIter != tasks.end());
    auto tasksCount = --(taskIter->second);
    if (tasksCount == 0) {
        tasks.erase(taskIter);
    }
    return tasksCount;
}

bool Taskpool::MoveTaskFromWaitingToRunning(EtsLong id, PandaUnorderedMap<EtsLong, size_t> &waitingTasks,
                                            PandaUnorderedMap<EtsLong, size_t> &runningTasks,
                                            PandaUnorderedSet<EtsLong> &idsToBeCanceled)
{
    auto waitingTasksAfterDecrementation = DecrementTaskCounter(id, waitingTasks);
    if (idsToBeCanceled.find(id) != idsToBeCanceled.end()) {
        if ((waitingTasksAfterDecrementation == 0) && (runningTasks.find(id) == runningTasks.end())) {
            idsToBeCanceled.erase(id);
        }
        return false;
    }
    runningTasks[id]++;
    return true;
}

bool Taskpool::TaskStarted(uint32_t coroutineId, EtsLong taskId, EtsLong groupId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    auto isNotCanceled =
        (groupId == 0)  // 0 means task is not group task
            ? MoveTaskFromWaitingToRunning(taskId, waitingTasks_, runningTasks_, tasksToBeCanceled_)
            : MoveTaskFromWaitingToRunning(groupId, waitingGroupTasks_, runningGroupTasks_, groupsToBeCanceled_);
    if (isNotCanceled) {
        executingTasks_.insert({coroutineId, {taskId, groupId}});
        return true;
    }
    return false;
}

bool Taskpool::RemoveTaskFromRunning(EtsLong id, PandaUnorderedMap<EtsLong, size_t> &runningTasks,
                                     const PandaUnorderedMap<EtsLong, size_t> &waitingTasks,
                                     PandaUnorderedSet<EtsLong> &idsToBeCanceled)
{
    auto runningTasksAfterDecrementation = DecrementTaskCounter(id, runningTasks);
    if (idsToBeCanceled.find(id) != idsToBeCanceled.end()) {
        if ((runningTasksAfterDecrementation == 0) && (waitingTasks.find(id) == waitingTasks.end())) {
            idsToBeCanceled.erase(id);
        }
        return false;
    }
    return true;
}

bool Taskpool::TaskFinished(uint32_t coroutineId, EtsLong taskId, EtsLong groupId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    ASSERT(executingTasks_[coroutineId].taskId == taskId);
    executingTasks_.erase(coroutineId);
    return (groupId == 0) ? RemoveTaskFromRunning(taskId, runningTasks_, waitingTasks_, tasksToBeCanceled_)
                          : RemoveTaskFromRunning(groupId, runningGroupTasks_, waitingGroupTasks_, groupsToBeCanceled_);
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

bool Taskpool::CancelGroup(EtsLong groupId)
{
    os::memory::LockHolder lh(taskpoolLock_);
    if ((waitingGroupTasks_.find(groupId) == waitingGroupTasks_.end()) &&
        (runningGroupTasks_.find(groupId) == runningTasks_.end())) {
        // No tasks of the group with this id in the taskpool
        return false;
    }
    groupsToBeCanceled_.insert(groupId);
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
    // 0 means non-group tasks
    if (it->second.groupId == 0) {
        return tasksToBeCanceled_.find(it->second.taskId) != tasksToBeCanceled_.end();
    }
    // The executing task is group task, so check the group status
    return groupsToBeCanceled_.find(it->second.groupId) != groupsToBeCanceled_.end();
}

}  // namespace ark::ets
