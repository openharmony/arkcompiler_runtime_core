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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_TASKPOOL_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_TASKPOOL_H

#include "libpandabase/os/mutex.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::ets {

/// @class Taskpool contains information about each common task passed to execution until the task will not be finished
class Taskpool final {
public:
    NO_COPY_SEMANTIC(Taskpool);
    NO_MOVE_SEMANTIC(Taskpool);

    Taskpool();
    ~Taskpool() = default;

    /**
     * @see taskpool.Task.constructor
     * @return new unique identifier for creating task
     */
    EtsLong GenerateTaskId();

    /**
     * @see taskpool.TaskGroup.constructor
     * @return new unique identifier for creating group
     */
    EtsLong GenerateTaskGroupId();

    /**
     * @see taskpool.SequenceRunner.constructor
     * @return new unique identifier for creating task sequence runner
     */
    EtsLong GenerateSeqRunnerId();

    /**
     * @brief Notify taskpool about execution request for task
     * @param taskId requested task identifier
     */
    void TaskSubmitted(EtsLong taskId);

    /**
     * @brief Notify taskpool about execution request for group of tasks
     * @param groupId requested group identifier
     * @param tasksCount count of tasks in the passed group
     */
    void GroupSubmitted(EtsLong groupId, size_t tasksCount);

    /**
     * @brief Notify taskpool that task is starting execution on a coroutine
     * @param coroutineId identifier of executing coroutine for requested task
     * @param taskId requsted task identifier
     * @param groupId group identifier of this task (0 means task is not group task)
     * @return true if task can be started, false - if task was cancled
     *
     * @see CancelTask
     */
    bool TaskStarted(uint32_t coroutineId, EtsLong taskId, EtsLong groupId);

    /**
     * @brief Notify taskpool that task is ending execution
     * @param coroutineId identifier of executing coroutine for requested task
     * @param taskId requsted task identifier
     * @param groupId group identifier of this task (0 means task is not group task)
     * @return true if task can be successfully finished, false - if task was cancled
     *
     * @see CancelTask
     */
    bool TaskFinished(uint32_t coroutineId, EtsLong taskId, EtsLong groupId);

    /**
     * @brief Try to mark task as cancel. Only waiting or running tasks are allowed to canceling
     * @param taskId identifier of task for canceling
     * @return true if task was marked as caneling, false - if task is not executed or finished
     */
    bool CancelTask(EtsLong taskId);

    /**
     * @brief Try to mark group as cancel. Only waiting or running groups are allowed to canceling
     * @param groupId identifier of task for canceling
     * @return true if the group was marked as caneling, false - if group is not executed or finished
     */
    bool CancelGroup(EtsLong groupId);

    /**
     * @param coroutineId corotine id with potentially executing task
     * @return true if coroutine with coroutineId is executing a task and this task is marked as canceling, false -
     * otherwise
     */
    bool IsTaskCanceled(uint32_t coroutineId) const;

private:
    /**
     * @brief Decrement count of tasks with associated identifier from passed collection. If count of task after
     * decrementing == 0, then remove this identifier from the collection
     * @param id associated identifier of decremented task counter, it can be task id or group id
     * @param tasks map collection, key is unique identifier, value is counter of tasks with the associated identifier
     * @return count tasks with associated identifier in passed collection after decrementing
     */
    size_t DecrementTaskCounter(EtsLong id, PandaUnorderedMap<EtsLong, size_t> &tasks) REQUIRES(taskpoolLock_);

    /**
     * @brief Decrement count of tasks with associated identifier in waiting tasks collection and if the id is not
     * marked as cancled then move to running tasks collection. If id is marked as canceled and no more tasks with the
     * associated id in the taskpool then clear the id from set of canceled identifiers
     * @param id associated identifier of decremented task counter, it can be task id or group id
     * @param waitingTasks map collection of waiting tasks, key is unique identifier, value is counter of tasks
     * @param runningTasks map collection of running tasks, key is unique identifier, value is counter of tasks
     * @param idsToBeCanceled set of identifiers marked as canceled
     * @return true if task is moved from waiting map to running map, false - if id is marked as canceled
     */
    bool MoveTaskFromWaitingToRunning(EtsLong id, PandaUnorderedMap<EtsLong, size_t> &waitingTasks,
                                      PandaUnorderedMap<EtsLong, size_t> &runningTasks,
                                      PandaUnorderedSet<EtsLong> &idsToBeCanceled) REQUIRES(taskpoolLock_);

    /**
     * @brief Decrement count of tasks with associated identifier in running tasks. If id is marked as canceled and no
     * more tasks with the associated id in the taskpool then clear the id from set of canceled identifiers
     * @param id associated identifier of decremented task counter, it can be task id or group id
     * @param waitingTasks map collection of waiting tasks, key is unique identifier, value is counter of tasks.
     * @param runningTasks map collection of running tasks, key is unique identifier, value is counter of tasks
     * @param idsToBeCanceled set of identifiers marked as canceled
     * @return true if task is moved from waiting map to running map, false - if id is marked as canceled
     */
    bool RemoveTaskFromRunning(EtsLong id, PandaUnorderedMap<EtsLong, size_t> &runningTasks,
                               const PandaUnorderedMap<EtsLong, size_t> &waitingTasks,
                               PandaUnorderedSet<EtsLong> &idsToBeCanceled) REQUIRES(taskpoolLock_);

    struct ExecutingTaskInfo {
        EtsLong taskId;
        EtsLong groupId;
    };

    std::atomic<EtsLong> taskId_ {1};
    std::atomic<EtsLong> taskGroupId_ {1};
    std::atomic<EtsLong> seqRunnerId_ {1};
    mutable os::memory::Mutex taskpoolLock_;
    // key is task id, value is count of associated tasks
    PandaUnorderedMap<EtsLong, size_t> waitingTasks_ GUARDED_BY(taskpoolLock_);
    PandaUnorderedMap<EtsLong, size_t> runningTasks_ GUARDED_BY(taskpoolLock_);
    PandaUnorderedSet<EtsLong> tasksToBeCanceled_ GUARDED_BY(taskpoolLock_);
    // key is group id, value is count of associated tasks
    PandaUnorderedMap<EtsLong, size_t> waitingGroupTasks_ GUARDED_BY(taskpoolLock_);
    PandaUnorderedMap<EtsLong, size_t> runningGroupTasks_ GUARDED_BY(taskpoolLock_);
    PandaUnorderedSet<EtsLong> groupsToBeCanceled_ GUARDED_BY(taskpoolLock_);
    // key is coroutine id, value is task information
    PandaUnorderedMap<uint32_t, ExecutingTaskInfo> executingTasks_ GUARDED_BY(taskpoolLock_);
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_TASKPOOL_H
