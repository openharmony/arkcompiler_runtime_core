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

class TaskQueueId {
public:
    constexpr TaskQueueId(TaskType tt, VMType vt)
        : val_(static_cast<uint16_t>(tt) |
               static_cast<uint16_t>(static_cast<uint16_t>(vt) << (BITS_PER_BYTE * sizeof(TaskQueueId) / 2)))
    {
        static_assert(sizeof(TaskType) == sizeof(TaskQueueId) / 2);
        static_assert(sizeof(VMType) == sizeof(TaskQueueId) / 2);
    }

    friend constexpr bool operator==(const TaskQueueId &lv, const TaskQueueId &rv)
    {
        return lv.val_ == rv.val_;
    }

    friend constexpr bool operator!=(const TaskQueueId &lv, const TaskQueueId &rv)
    {
        return lv.val_ != rv.val_;
    }

    friend constexpr bool operator<(const TaskQueueId &lv, const TaskQueueId &rv)
    {
        return lv.val_ < rv.val_;
    }

private:
    uint16_t val_;
};

constexpr TaskQueueId INVALID_TASKQUEUE_ID = TaskQueueId(TaskType::UNKNOWN, VMType::UNKNOWN);

/**
 * @brief TaskQueue is a thread-safe queue for tasks. Queues can be registered in TaskScheduler and used to execute
 * tasks on workers. Also, queues can notify other threads when a new task is pushed.
 */
class TaskQueue {
public:
    NO_COPY_SEMANTIC(TaskQueue);
    NO_MOVE_SEMANTIC(TaskQueue);

    /**
     * NewTasksCallback instance should be called after tasks adding. As argument you should input count of added
     * tasks.
     */
    using NewTasksCallback = std::function<void(TaskProperties, size_t)>;

    static constexpr uint8_t MAX_PRIORITY = 10;
    static constexpr uint8_t MIN_PRIORITY = 1;
    static constexpr uint8_t DEFAULT_PRIORITY = 5;

    /**
     * @brief When you construct a queue, you should write @arg task_type and @arg vm_type of Tasks that will be
     * contained in it. Also, you should write @arg priority, which will be used in Task Manager for task selection.
     * It's a number from 1 to 10 and determines the weight of the queue for the Task Manager.
     * MAX_PRIORITY = 10, MIN_PRIORITY = 1, DEFAULT_PRIORITY = 5;
     */
    PANDA_PUBLIC_API TaskQueue(TaskType task_type, VMType vm_type, uint8_t priority);
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
    [[nodiscard]] std::optional<Task> PopTask();

    /**
     * @brief Pops task from task queue with specified execution mode. Operation is thread-safe. The method will wait
     * new task if queue with specified execution mode is empty and method WaitForQueueEmptyAndFinish has not been
     * executed. Otherwise it will return std::nullopt.
     * @param mode - execution mode of task that we want to pop.
     */
    [[nodiscard]] std::optional<Task> PopTask(TaskExecutionMode mode);

    [[nodiscard]] PANDA_PUBLIC_API bool IsEmpty() const;
    [[nodiscard]] PANDA_PUBLIC_API size_t Size() const;

    /**
     * @brief Method @returns true if queue does not have queue with specified execution mode
     * @param mode - execution mode of tasks
     */
    [[nodiscard]] PANDA_PUBLIC_API bool HasTaskWithExecutionMode(TaskExecutionMode mode) const;

    PANDA_PUBLIC_API TaskType GetTaskType() const;
    PANDA_PUBLIC_API VMType GetVMType() const;

    PANDA_PUBLIC_API uint8_t GetPriority() const;
    PANDA_PUBLIC_API void SetPriority(uint8_t priority);

    /**
     * @brief This method saves the @arg callback. It will be called after adding new task in AddTask method.
     * @param callback - function that get count of inputted tasks.
     */
    void SubscribeCallbackToAddTask(NewTasksCallback callback);

    /// @brief Removes callback function.
    void UnsubscribeCallback();

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

    std::atomic_uint8_t priority_;
    TaskType task_type_;
    VMType vm_type_;

    /**
     * foreground_task_queue_ is queue that contains task with ExecutionMode::FOREGROUND. If method PopTask() is used,
     * foreground_task_queue_ will be checked first and if it's not empty, Task will be gotten from it.
     */
    std::queue<Task> foreground_task_queue_ GUARDED_BY(task_queue_lock_);
    /**
     * background_task_queue_ is queue that contains task with ExecutionMode::BACKGROUND. If method PopTask() is used,
     * background_task_queue_ will be popped only if foreground_task_queue_ is empty.
     */
    std::queue<Task> background_task_queue_ GUARDED_BY(task_queue_lock_);

    /// task_queue_lock_ is used in case of interaction with internal queues
    mutable os::memory::Mutex task_queue_lock_;
    os::memory::ConditionVariable task_queue_cond_var_ GUARDED_BY(task_queue_lock_);
    os::memory::ConditionVariable finish_var_ GUARDED_BY(task_queue_lock_);

    /// subscriber_lock_ is used in case of calling new_tasks_callback_
    os::memory::Mutex subscriber_lock_;
    NewTasksCallback new_tasks_callback_ GUARDED_BY(subscriber_lock_);

    bool finish_ {false};
};

}  // namespace panda::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
