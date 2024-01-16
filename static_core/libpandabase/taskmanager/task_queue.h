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

namespace ark::taskmanager::internal {

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
    static PANDA_PUBLIC_API SchedulableTaskQueueInterface *Create(TaskType taskType, VMType vmType, uint8_t priority)
    {
        TaskQueueAllocatorType allocator;
        auto *mem = allocator.allocate(sizeof(TaskQueue<TaskAllocatorType>));
        return new (mem) TaskQueue<TaskAllocatorType>(taskType, vmType, priority);
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
        os::memory::LockHolder pushLockHolder(pushPopLock_);
        auto properties = task.GetTaskProperties();
        size_t size = 0;
        {
            os::memory::LockHolder taskQueueStateLockHolder(taskQueueStateLock_);
            PushTaskToInternalQueues(std::move(task));
            pushWaitCondVar_.Signal();
            size = SumSizeOfInternalQueues();
        }
        os::memory::LockHolder subscriberLockHolder(subscriberLock_);
        // Notify subscriber about new task
        if (newTasksCallback_ != nullptr) {
            newTasksCallback_(properties, 1UL, size == 1UL);
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
        os::memory::LockHolder popLockHolder(pushPopLock_);
        while (IsEmpty()) {
            if (finish_) {
                return std::nullopt;
            }
            pushWaitCondVar_.Wait(&pushPopLock_);
        }
        os::memory::LockHolder taskQueueStateLockHolder(taskQueueStateLock_);
        auto task = PopTaskFromInternalQueues();
        finishCondVar_.Signal();
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
        os::memory::LockHolder popLockHolder(pushPopLock_);
        auto *queue = &foregroundTaskQueue_;
        if (mode != TaskExecutionMode::FOREGROUND) {
            queue = &backgroundTaskQueue_;
        }
        while (!HasTaskWithExecutionMode(mode)) {
            if (finish_) {
                return std::nullopt;
            }
            pushWaitCondVar_.Wait(&pushPopLock_);
        }
        os::memory::LockHolder taskQueueStateLockHolder(taskQueueStateLock_);
        auto task = PopTaskFromQueue(*queue);
        finishCondVar_.Signal();
        return std::make_optional(std::move(task));
    }

    /**
     * @brief Method pops several tasks to worker.
     * @param add_task_func - Functor that will be used to add popped tasks to worker
     * @param size - Count of tasks you want to pop. If it is greater then count of tasks that are stored in queue,
     * method will not wait and will pop all stored tasks.
     * @return count of task that was added to worker
     */
    size_t PopTasksToWorker(AddTaskToWorkerFunc addTaskFunc, size_t size) override
    {
        os::memory::LockHolder popLockHolder(pushPopLock_);
        os::memory::LockHolder taskQueueStateLockHolder(taskQueueStateLock_);
        size = (SumSizeOfInternalQueues() < size) ? (SumSizeOfInternalQueues()) : (size);
        for (size_t i = 0; i < size; i++) {
            addTaskFunc(PopTaskFromInternalQueues());
        }
        finishCondVar_.Signal();
        return size;
    }

    [[nodiscard]] PANDA_PUBLIC_API bool IsEmpty() const override
    {
        os::memory::LockHolder lockHolder(taskQueueStateLock_);
        return AreInternalQueuesEmpty();
    }

    [[nodiscard]] PANDA_PUBLIC_API size_t Size() const override
    {
        os::memory::LockHolder lockHolder(taskQueueStateLock_);
        return SumSizeOfInternalQueues();
    }

    /**
     * @brief Method @returns true if queue does not have queue with specified execution mode
     * @param mode - execution mode of tasks
     */
    [[nodiscard]] PANDA_PUBLIC_API bool HasTaskWithExecutionMode(TaskExecutionMode mode) const override
    {
        os::memory::LockHolder lockHolder(taskQueueStateLock_);
        if (mode == TaskExecutionMode::FOREGROUND) {
            return !foregroundTaskQueue_.empty();
        }
        return !backgroundTaskQueue_.empty();
    }

    /**
     * @brief This method saves the @arg callback. It will be called after adding new task in AddTask method.
     * This method should be used only in TaskScheduler!
     * @param callback - function that get count of inputted tasks.
     */
    void SetNewTasksCallback(NewTasksCallback callback) override
    {
        os::memory::LockHolder subscriberLockHolder(subscriberLock_);
        newTasksCallback_ = std::move(callback);
    }

    /// @brief Removes callback function. This method should be used only in TaskScheduler!
    void UnsetNewTasksCallback() override
    {
        os::memory::LockHolder lockHolder(subscriberLock_);
        newTasksCallback_ = nullptr;
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
        {
            os::memory::LockHolder lockHolder(taskQueueStateLock_);
            while (!AreInternalQueuesEmpty()) {
                finishCondVar_.Wait(&taskQueueStateLock_);
            }
        }
        os::memory::LockHolder pushPopLockHolder(pushPopLock_);
        finish_ = true;
        pushWaitCondVar_.SignalAll();
    }

    using InternalTaskQueue = std::queue<Task, std::deque<Task, TaskAllocatorType>>;

    TaskQueue(TaskType taskType, VMType vmType, uint8_t priority)
        : SchedulableTaskQueueInterface(taskType, vmType, priority),
          foregroundTaskQueue_(TaskAllocatorType()),
          backgroundTaskQueue_(TaskAllocatorType())
    {
    }

    bool AreInternalQueuesEmpty() const REQUIRES(taskQueueStateLock_)
    {
        return foregroundTaskQueue_.empty() && backgroundTaskQueue_.empty();
    }

    size_t SumSizeOfInternalQueues() const REQUIRES(taskQueueStateLock_)
    {
        return foregroundTaskQueue_.size() + backgroundTaskQueue_.size();
    }

    void PushTaskToInternalQueues(Task &&task) REQUIRES(taskQueueStateLock_)
    {
        if (task.GetTaskProperties().GetTaskExecutionMode() == TaskExecutionMode::FOREGROUND) {
            foregroundTaskQueue_.push(std::move(task));
        } else {
            backgroundTaskQueue_.push(std::move(task));
        }
    }

    Task PopTaskFromInternalQueues() REQUIRES(taskQueueStateLock_)
    {
        if (!foregroundTaskQueue_.empty()) {
            return PopTaskFromQueue(foregroundTaskQueue_);
        }
        return PopTaskFromQueue(backgroundTaskQueue_);
    }

    Task PopTaskFromQueue(InternalTaskQueue &queue) REQUIRES(taskQueueStateLock_)
    {
        auto task = std::move(queue.front());
        queue.pop();
        return task;
    }

    /// push_pop_lock_ is used in push and pop operations as first guarder
    mutable os::memory::Mutex pushPopLock_;

    /// task_queue_state_lock_ is used in case of interaction with internal queues.
    mutable os::memory::Mutex taskQueueStateLock_;

    os::memory::ConditionVariable pushWaitCondVar_ GUARDED_BY(pushPopLock_);
    os::memory::ConditionVariable finishCondVar_ GUARDED_BY(taskQueueStateLock_);

    /// subscriber_lock_ is used in case of calling new_tasks_callback_
    os::memory::Mutex subscriberLock_;
    NewTasksCallback newTasksCallback_ GUARDED_BY(subscriberLock_);

    bool finish_ GUARDED_BY(pushPopLock_) {false};

    /**
     * foreground_task_queue_ is queue that contains task with ExecutionMode::FOREGROUND. If method PopTask() is used,
     * foreground_task_queue_ will be checked first and if it's not empty, Task will be gotten from it.
     */
    InternalTaskQueue foregroundTaskQueue_ GUARDED_BY(taskQueueStateLock_);
    /**
     * background_task_queue_ is queue that contains task with ExecutionMode::BACKGROUND. If method PopTask() is used,
     * background_task_queue_ will be popped only if foreground_task_queue_ is empty.
     */
    InternalTaskQueue backgroundTaskQueue_ GUARDED_BY(taskQueueStateLock_);
};

}  // namespace ark::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
