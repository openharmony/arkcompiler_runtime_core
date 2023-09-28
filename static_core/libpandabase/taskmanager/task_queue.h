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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H

#include "libpandabase/os/mutex.h"
#include "libpandabase/taskmanager/task.h"
#include <atomic>
#include <optional>
#include <queue>

namespace panda::taskmanager {

class TaskQueueTraits {
public:
    TaskQueueTraits(TaskType tt, VMType vt);
    TaskType GetTaskType() const;
    VMType GetVMType() const;
    friend bool operator<(const TaskQueueTraits &lv, const TaskQueueTraits &rv);

private:
    uint32_t val_;
};

/**
 * @brief TaskQueue is a thread-safe queue for tasks. Queues can be registered in TaskScheduler and used to execute
 * tasks on workers. Also, queues can notify other threads when a new task is pushed.
 */
class TaskQueue {
public:
    /**
     * @brief When you construct a queue, you should write @arg task_type and @arg vm_type of Tasks that will be
     * contained in it. Also, you should write @arg priority, which will be used in Task Manager for task selection.
     * High priority value means a high probability of selecting a task from this queue.
     */
    NO_COPY_SEMANTIC(TaskQueue);
    NO_MOVE_SEMANTIC(TaskQueue);

    PANDA_PUBLIC_API TaskQueue(TaskType task_type, VMType vm_type, size_t priority);
    PANDA_PUBLIC_API ~TaskQueue();

    /**
     * @brief Adds task in task queue. Operation is thread-safe.
     * @param task - task that will be added
     * @return the size of queue after @arg task was added to it.
     */
    PANDA_PUBLIC_API size_t AddTask(Task &&task);

    /**
     * @brief Pops task from task queue. Operation is thread-safe. The method will wait new task if queue is empty and
     * method WaitForQueueEmptyAndFinish has not been executed. Otherwise it will return std::nullopt.
     */
    [[nodiscard]] PANDA_PUBLIC_API std::optional<Task> PopTask();

    /**
     * @brief Pops task from task queue with specified execution mode. Operation is thread-safe. The method will wait
     * new task if queue with specified execution mode is empty and method WaitForQueueEmptyAndFinish has not been
     * executed. Otherwise it will return std::nullopt.
     * @param mode - execution mode of task that we want to pop.
     */
    [[nodiscard]] PANDA_PUBLIC_API std::optional<Task> PopTask(TaskExecutionMode mode);

    [[nodiscard]] PANDA_PUBLIC_API bool IsEmpty() const;
    [[nodiscard]] PANDA_PUBLIC_API size_t Size() const;

    /**
     * @brief Method @returns true if queue does not have queue with specified execution mode
     * @param mode - execution mode of tasks
     */
    PANDA_PUBLIC_API bool HasTaskWithExecutionMode(TaskExecutionMode mode) const;

    PANDA_PUBLIC_API TaskType GetTaskType() const;
    PANDA_PUBLIC_API VMType GetVMType() const;

    PANDA_PUBLIC_API size_t GetPriority() const;
    PANDA_PUBLIC_API void SetPriority(size_t priority);

    /**
     * @brief This method saves the @arg pointers on mutex and cond_var. After the AddTask method, TaskQueue will signal
     * cond_var. It's necessary for outside synchronization.
     * @param cond_var - condition variable that will be used to notify outside waiter
     * @param lock - mutex that that should gourd cond_var
     */
    void SubscribeCondVarToAddTask(os::memory::Mutex *lock, os::memory::ConditionVariable *cond_var);

    /// @brief Removes pointer on cond_var.
    void UnsubscribeCondVarFromAddTask();

    /**
     * @brief Method waits until internal queue will be empty and finalize using of TaskQueue
     * After this method PopTask will not wait for new tasks.
     */
    void WaitForQueueEmptyAndFinish();

private:
    bool AreInternalQueuesEmpty() const REQUIRES(task_queue_lock_);

    size_t SumSizeOfInternalQueues() const REQUIRES(task_queue_lock_);

    void PushTaskToInternalQueues(Task &&task) REQUIRES(task_queue_lock_);
    Task PopTaskFromInternalQueues() REQUIRES(task_queue_lock_);

    Task PopTaskFromQueue(std::queue<Task> &queue) REQUIRES(task_queue_lock_);

    std::atomic_size_t priority_;
    TaskType task_type_;
    VMType vm_type_;

    /**
     * foreground_task_queue_ is queue that contains task with ExecutionMode::FOREGROUND. If method PopTask() is used,
     * foreground_queue_ will be checked first and if it's not empty, Task will be gotten from it.
     */
    std::queue<Task> foreground_task_queue_ GUARDED_BY(task_queue_lock_);
    /**
     * background_task_queue_ is queue that contains task with ExecutionMode::BACKGROUND. If method PopTask() is used,
     * background_queue_ will be popped only if foreground_queue_ is empty.
     */
    std::queue<Task> background_task_queue_ GUARDED_BY(task_queue_lock_);

    /// task_queue_lock_ is used in case of interaction with internal queues
    mutable os::memory::Mutex task_queue_lock_;
    os::memory::ConditionVariable task_queue_cond_var_ GUARDED_BY(task_queue_lock_);
    os::memory::ConditionVariable finish_var_ GUARDED_BY(task_queue_lock_);

    /// subscriber_lock_ is used in case of interaction with outside_lock_ and outside_cond_var_ pointers
    os::memory::Mutex subscriber_lock_;
    /// outside_lock_ - pointer on lock that is used when we use outside variables
    os::memory::Mutex *outside_lock_ GUARDED_BY(subscriber_lock_) {nullptr};
    os::memory::ConditionVariable *outside_cond_var_ GUARDED_BY(subscriber_lock_) {nullptr};

    bool finish_ {false};
};

}  // namespace panda::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
