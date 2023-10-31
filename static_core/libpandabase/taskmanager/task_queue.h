/*
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
#include "libpandabase/taskmanager/schedulable_task_queue_interface.h"

namespace panda::taskmanager::internal {

/**
 * @brief TaskQueue is a thread-safe queue for tasks. Queues can be registered in TaskScheduler and used to execute
 * tasks on workers. Also, queues can notify other threads when a new task is pushed.
 * @tparam Allocator - allocator of Task that will be used in internal queues. By default is used
 * std::allocator<Task>
 */
template <class Allocator = std::allocator<Task>>
class TaskQueue : public SchedulableTaskQueueInterface {
    using TaskAllocatorType = typename Allocator::template rebind<Task>::other;
    using TaskQueueAllocatorType = typename Allocator::template rebind<TaskQueue<TaskAllocatorType>>::other;
    template <class OtherAllocator>
    friend class TaskQueue;

public:
    NO_COPY_SEMANTIC(TaskQueue);
    NO_MOVE_SEMANTIC(TaskQueue);

    /**
     * @brief The TaskQueue factory. Intended to be used by the TaskScheduler's CreateAndRegister method.
     * @param task_type: TaskType of queue.
     * @param vm_type: VMType of queue.
     * @param priority: A number from 1 to 10 that determines the weight of the queue during the task selection process
     * @return a pointer to the created queue.
     */
    static PANDA_PUBLIC_API SchedulableTaskQueueInterface *Create(TaskType task_type, VMType vm_type, uint8_t priority)
    {
        TaskQueueAllocatorType allocator;
        auto *mem = allocator.allocate(sizeof(TaskQueue<TaskAllocatorType>));
        return new (mem) TaskQueue<TaskAllocatorType>(task_type, vm_type, priority);
    }

    static PANDA_PUBLIC_API void Destroy(SchedulableTaskQueueInterface *queue)
    {
        TaskQueueAllocatorType allocator;
        std::allocator_traits<TaskQueueAllocatorType>::destroy(allocator, queue);
        allocator.deallocate(static_cast<TaskQueue<TaskAllocatorType> *>(queue), sizeof(TaskQueue<TaskAllocatorType>));
    }

    PANDA_PUBLIC_API ~TaskQueue() override
    {
        WaitForEmpty();
    }

    /**
     * @brief Adds task in task queue. Operation is thread-safe.
     * @param task - task that will be added
     * @return the size of queue after @arg task was added to it.
     */
    PANDA_PUBLIC_API size_t AddTask(Task &&task) override
    {
        ASSERT(task.GetTaskProperties().GetTaskType() == GetTaskType());
        ASSERT(task.GetTaskProperties().GetVMType() == GetVMType());
        auto properties = task.GetTaskProperties();
        size_t size = 0;
        {
            os::memory::LockHolder lock_holder(task_queue_lock_);
            PushTaskToInternalQueues(std::move(task));
            task_queue_cond_var_.Signal();
            size = SumSizeOfInternalQueues();
        }
        {
            os::memory::LockHolder lock_holder(subscriber_lock_);
            // Notify subscriber about new task
            if (new_tasks_callback_ != nullptr) {
                new_tasks_callback_(properties, 1);
            }
        }
        return size;
    }

    /**
     * @brief Pops task from task queue. Operation is thread-safe. The method will wait new task if queue is empty and
     * method WaitForQueueEmptyAndFinish has not been executed. Otherwise it will return std::nullopt.
     * This method should be used only in TaskScheduler
     */
    [[nodiscard]] std::optional<Task> PopTask() override
    {
        os::memory::LockHolder lock_holder(task_queue_lock_);
        while (AreInternalQueuesEmpty()) {
            if (finish_) {
                return std::nullopt;
            }
            task_queue_cond_var_.Wait(&task_queue_lock_);
        }
        auto task = PopTaskFromInternalQueues();
        finish_var_.Signal();
        return std::make_optional(std::move(task));
    }

    /**
     * @brief Pops task from task queue with specified execution mode. Operation is thread-safe. The method will wait
     * new task if queue with specified execution mode is empty and method WaitForQueueEmptyAndFinish has not been
     * executed. Otherwise it will return std::nullopt.
     * This method should be used only in TaskScheduler!
     * @param mode - execution mode of task that we want to pop.
     */
    [[nodiscard]] std::optional<Task> PopTask(TaskExecutionMode mode) override
    {
        os::memory::LockHolder lock_holder(task_queue_lock_);
        auto *queue = &foreground_task_queue_;
        if (mode != TaskExecutionMode::FOREGROUND) {
            queue = &background_task_queue_;
        }
        while (queue->empty()) {
            if (finish_) {
                return std::nullopt;
            }
            task_queue_cond_var_.Wait(&task_queue_lock_);
        }
        auto task = PopTaskFromQueue(*queue);
        finish_var_.Signal();
        return std::make_optional(std::move(task));
    }

    [[nodiscard]] PANDA_PUBLIC_API bool IsEmpty() const override
    {
        os::memory::LockHolder lock_holder(task_queue_lock_);
        return AreInternalQueuesEmpty();
    }

    [[nodiscard]] PANDA_PUBLIC_API size_t Size() const override
    {
        os::memory::LockHolder lock_holder(task_queue_lock_);
        return SumSizeOfInternalQueues();
    }

    /**
     * @brief Method @returns true if queue does not have queue with specified execution mode
     * @param mode - execution mode of tasks
     */
    [[nodiscard]] PANDA_PUBLIC_API bool HasTaskWithExecutionMode(TaskExecutionMode mode) const override
    {
        os::memory::LockHolder lock_holder(task_queue_lock_);
        if (mode == TaskExecutionMode::FOREGROUND) {
            return !foreground_task_queue_.empty();
        }
        return !background_task_queue_.empty();
    }

    /**
     * @brief This method saves the @arg callback. It will be called after adding new task in AddTask method.
     * This method should be used only in TaskScheduler!
     * @param callback - function that get count of inputted tasks.
     */
    void SetNewTasksCallback(NewTasksCallback callback) override
    {
        os::memory::LockHolder subscriber_lock_holder(subscriber_lock_);
        new_tasks_callback_ = std::move(callback);
    }

    /// @brief Removes callback function. This method should be used only in TaskScheduler!
    void UnsetNewTasksCallback() override
    {
        os::memory::LockHolder lock_holder(subscriber_lock_);
        new_tasks_callback_ = nullptr;
    }

    /**
     * @brief Method waits until internal queue will be empty and finalize using of TaskQueue
     * After this method PopTask will not wait for new tasks.
     */
    void WaitForQueueEmptyAndFinish() override
    {
        WaitForEmpty();
    }

private:
    void WaitForEmpty()
    {
        os::memory::LockHolder lock_holder(task_queue_lock_);
        while (!AreInternalQueuesEmpty()) {
            finish_var_.Wait(&task_queue_lock_);
        }
        finish_ = true;
        task_queue_cond_var_.SignalAll();
    }

    using InternalTaskQueue = std::queue<Task, std::deque<Task, TaskAllocatorType>>;

    TaskQueue(TaskType task_type, VMType vm_type, uint8_t priority)
        : SchedulableTaskQueueInterface(task_type, vm_type, priority),
          foreground_task_queue_(TaskAllocatorType()),
          background_task_queue_(TaskAllocatorType())
    {
    }

    bool AreInternalQueuesEmpty() const REQUIRES(task_queue_lock_)
    {
        return foreground_task_queue_.empty() && background_task_queue_.empty();
    }

    size_t SumSizeOfInternalQueues() const REQUIRES(task_queue_lock_)
    {
        return foreground_task_queue_.size() + background_task_queue_.size();
    }

    void PushTaskToInternalQueues(Task &&task) REQUIRES(task_queue_lock_)
    {
        if (task.GetTaskProperties().GetTaskExecutionMode() == TaskExecutionMode::FOREGROUND) {
            foreground_task_queue_.push(std::move(task));
        } else {
            background_task_queue_.push(std::move(task));
        }
    }

    Task PopTaskFromInternalQueues() REQUIRES(task_queue_lock_)
    {
        if (!foreground_task_queue_.empty()) {
            return PopTaskFromQueue(foreground_task_queue_);
        }
        return PopTaskFromQueue(background_task_queue_);
    }

    Task PopTaskFromQueue(InternalTaskQueue &queue) REQUIRES(task_queue_lock_)
    {
        auto task = std::move(queue.front());
        queue.pop();
        return task;
    }

    /// task_queue_lock_ is used in case of interaction with internal queues
    mutable os::memory::Mutex task_queue_lock_;
    os::memory::ConditionVariable task_queue_cond_var_ GUARDED_BY(task_queue_lock_);
    os::memory::ConditionVariable finish_var_ GUARDED_BY(task_queue_lock_);

    /// subscriber_lock_ is used in case of calling new_tasks_callback_
    os::memory::Mutex subscriber_lock_;
    NewTasksCallback new_tasks_callback_ GUARDED_BY(subscriber_lock_);

    bool finish_ {false};

    /**
     * foreground_task_queue_ is queue that contains task with ExecutionMode::FOREGROUND. If method PopTask() is used,
     * foreground_task_queue_ will be checked first and if it's not empty, Task will be gotten from it.
     */
    InternalTaskQueue foreground_task_queue_ GUARDED_BY(task_queue_lock_);
    /**
     * background_task_queue_ is queue that contains task with ExecutionMode::BACKGROUND. If method PopTask() is used,
     * background_task_queue_ will be popped only if foreground_task_queue_ is empty.
     */
    InternalTaskQueue background_task_queue_ GUARDED_BY(task_queue_lock_);
};

}  // namespace panda::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
