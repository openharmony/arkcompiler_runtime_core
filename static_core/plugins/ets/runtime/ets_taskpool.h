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
     * @brief Notify taskpool about execution request for task
     * @param taskId requsted task identifier
     */
    void TaskSubmitted(EtsLong taskId);

    /**
     * @brief Notify taskpool that task is starting execution on a coroutine
     * @param coroutineId identifier of executing coroutine for requested task
     * @param taskId requsted task identifier
     * @return true if task can be started, false - if task was cancled
     *
     * @see CancelTask
     */
    bool TaskStarted(uint32_t coroutineId, EtsLong taskId);

    /**
     * @brief Notify taskpool that task is ending execution
     * @param coroutineId identifier of executing coroutine for requested task
     * @param taskId requsted task identifier
     * @return true if task can be successfully finished, false - if task was cancled
     *
     * @see CancelTask
     */
    bool TaskFinished(uint32_t coroutineId, EtsLong taskId);

    /**
     * @brief Try to mark task as cancel. Only waiting or running tasks are allowed to canceling
     * @param taskId identifier of task for canceling
     * @return true if task was marked as caneling, false - if task is not executed or finished
     */
    bool CancelTask(EtsLong taskId);

    /**
     * @param coroutineId corotine id with potentially executing task
     * @return true if coroutine with coroutineId is executing a task and this task is marked as canceling, false -
     * otherwise
     */
    bool IsTaskCanceled(uint32_t coroutineId) const;

private:
    /**
     * @brief Decrease count of tasks with requested identifier from passed collection. If count of task after
     * decrementing == 0, then remove this task identifier from the collection
     * @param taskId identifier of decremented task counter
     * @param tasks map collection, key is unique task identifier, value is counter of tasks with this identifier
     * @return count tasks with requested identifier in passed collection after decrementing
     */
    size_t DecrementTaskCounter(EtsLong taskId, PandaUnorderedMap<EtsLong, size_t> &tasks) REQUIRES(taskpoolLock_);

    std::atomic<EtsLong> taskId_ {1};
    mutable os::memory::Mutex taskpoolLock_;
    PandaUnorderedMap<EtsLong, size_t> waitingTasks_ GUARDED_BY(taskpoolLock_);
    PandaUnorderedMap<EtsLong, size_t> runningTasks_ GUARDED_BY(taskpoolLock_);
    PandaUnorderedSet<EtsLong> tasksToBeCanceled_ GUARDED_BY(taskpoolLock_);
    // key is coroutine id, value is task id
    PandaUnorderedMap<uint32_t, EtsLong> executingTasks_ GUARDED_BY(taskpoolLock_);
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_TASKPOOL_H
