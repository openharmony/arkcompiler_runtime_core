/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "libpandabase/taskmanager/utils/sp_sc_lock_free_queue.h"

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
        auto *mem = allocator.allocate(1U);
        return new (mem) TaskQueue<TaskAllocatorType>(taskType, vmType, priority);
    }

    static PANDA_PUBLIC_API void Destroy(SchedulableTaskQueueInterface *queue)
    {
        TaskQueueAllocatorType allocator;
        std::allocator_traits<TaskQueueAllocatorType>::destroy(allocator, queue);
        allocator.deallocate(static_cast<TaskQueue<TaskAllocatorType> *>(queue), 1U);
    }

    PANDA_PUBLIC_API ~TaskQueue() override
    {
        ASSERT(AreInternalQueuesEmpty());
    }

    /**
     * @brief Adds task in task queue. Operation is thread-safe.
     * @param task - task that will be added
     * @return the size of queue after @arg task was added to it.
     */
    PANDA_PUBLIC_API size_t AddTask(Task &&task) override
    {
        auto properties = task.GetTaskProperties();
        ASSERT(properties.GetTaskType() == GetTaskType());
        ASSERT(properties.GetVMType() == GetVMType());
        // Send info about new added task
        if (LIKELY(newTasksCallback_ != nullptr)) {
            newTasksCallback_(properties, 1UL);
        }
        AddTaskWithoutNewTaskCallbackExecution(std::move(task));
        return Size();
    }

    /**
     * @brief The method adds a task to the queue without execution the new task callback. This method should only be
     * used with tasks that have already triggered this callback.
     * @param task: instance of Task
     */
    void AddTaskWithoutNewTaskCallbackExecution(Task &&task) override
    {
        EventOnTaskAdding(&task);
        // Push task in one of internal queues based on its TaskExecutionMode
        PushTaskToInternalQueues(std::move(task));
        // Signal workers that should execute new task
        if (signalWorkersCallback_ != nullptr) {
            signalWorkersCallback_();
        }
    }

    /**
     * @brief Pops task from task queue. Operation is thread-safe. The method will wait a new task if queue is empty
     * and method WaitForQueueEmptyAndFinish has not been executed. Otherwise it will return std::nullopt.
     * This method should be used only in TaskScheduler
     */
    [[nodiscard]] std::optional<Task> PopTask() override
    {
        return PopTaskFromInternalQueues();
    }

    /**
     * @brief Pops task from task queue with specified execution mode. Operation is thread-safe. The method will wait
     * a new task if queue with specified execution mode is empty and method WaitForQueueEmptyAndFinish has not been
     * executed. Otherwise it will return std::nullopt.
     * This method should be used only in TaskScheduler!
     * @param mode - execution mode of task that we want to pop.
     */
    [[nodiscard]] std::optional<Task> PopTask(TaskExecutionMode mode) override
    {
        if (UNLIKELY(!HasTaskWithExecutionMode(mode))) {
            return std::nullopt;
        }
        auto *queue = &foregroundTaskQueue_;
        if (UNLIKELY(mode != TaskExecutionMode::FOREGROUND)) {
            queue = &backgroundTaskQueue_;
        }
        auto task = queue->Pop();
        return task;
    }

    /**
     * @brief Method pops several tasks to worker.
     * @param addTaskFunc - Functor that will be used to add popped tasks to worker
     * @param size - Count of tasks you want to pop. If it is greater then count of tasks that are stored in queue,
     * method will not wait and will pop all stored tasks.
     * @return count of task that was added to worker
     */
    size_t PopTasksToWorker(const AddTaskToWorkerFunc &addTaskFunc, size_t size) override
    {
        if (UNLIKELY(AreInternalQueuesEmpty())) {
            return 0;
        }
        size_t returnSize = 0;
        for (; !AreInternalQueuesEmpty() && returnSize < size; returnSize++) {
            addTaskFunc(PopTaskFromInternalQueues().value());
        }
        return returnSize;
    }

    /**
     * @brief Method pops several tasks to helper thread. Helper thread in TaskScheduler is the thread that uses
     * HelpWorkersWithTasks method.
     * @param addTaskFunc - Functor that will be used to add popped tasks to helper
     * @param size - Count of tasks you want to pop. If it is greater then count of tasks that are stored in queue,
     * method will not wait and will pop all stored tasks.
     * @param mode - Execution mode of task you wast to pop
     * @return count of task that was added to helper
     */
    size_t PopTasksToHelperThread(const AddTaskToHelperFunc &addTaskFunc, size_t size, TaskExecutionMode mode) override
    {
        if (!HasTaskWithExecutionMode(mode)) {
            return 0;
        }
        auto *queue = &foregroundTaskQueue_;
        if (mode != TaskExecutionMode::FOREGROUND) {
            queue = &backgroundTaskQueue_;
        }
        size_t returnSize = 0;
        for (; HasTaskWithExecutionMode(mode) && returnSize < size; returnSize++) {
            addTaskFunc(queue->Pop().value());
        }
        return returnSize;
    }

    [[nodiscard]] PANDA_PUBLIC_API bool IsEmpty() const override
    {
        return AreInternalQueuesEmpty();
    }

    [[nodiscard]] PANDA_PUBLIC_API size_t Size() const override
    {
        return SumSizeOfInternalQueues();
    }

    /**
     * @brief Method @returns true if queue does not have queue with specified execution mode
     * @param mode - execution mode of tasks
     */
    [[nodiscard]] PANDA_PUBLIC_API bool HasTaskWithExecutionMode(TaskExecutionMode mode) const override
    {
        if (mode == TaskExecutionMode::FOREGROUND) {
            return !foregroundTaskQueue_.IsEmpty();
        }
        return !backgroundTaskQueue_.IsEmpty();
    }

    [[nodiscard]] PANDA_PUBLIC_API size_t CountOfTasksWithExecutionMode(TaskExecutionMode mode) const override
    {
        if (mode == TaskExecutionMode::FOREGROUND) {
            return foregroundTaskQueue_.Size();
        }
        return backgroundTaskQueue_.Size();
    }

    /**
     * @brief This method saves the @arg callback.
     * @param newTaskCallback - function that get count of inputted tasks and uses in AddTask method.
     * @param signalWorkersCallback - function that should signal workers to return to work if it's needed
     */
    void SetCallbacks(NewTasksCallback newTaskCallback, SignalWorkersCallback signalWorkersCallback) override
    {
        newTasksCallback_ = std::move(newTaskCallback);
        signalWorkersCallback_ = std::move(signalWorkersCallback);
    }

    /// @brief Removes callback function. This method should be used only in TaskScheduler!
    void UnsetCallbacks() override
    {
        newTasksCallback_ = nullptr;
        signalWorkersCallback_ = nullptr;
    }

private:
    using InternalTaskQueue = SPSCLockFreeQueue<Task, TaskAllocatorType>;

    TaskQueue(TaskType taskType, VMType vmType, uint8_t priority)
        : SchedulableTaskQueueInterface(taskType, vmType, priority)
    {
    }

    bool AreInternalQueuesEmpty() const
    {
        return foregroundTaskQueue_.IsEmpty() && backgroundTaskQueue_.IsEmpty();
    }

    size_t SumSizeOfInternalQueues() const
    {
        return foregroundTaskQueue_.Size() + backgroundTaskQueue_.Size();
    }

    void PushTaskToInternalQueues(Task &&task)
    {
        if (task.GetTaskProperties().GetTaskExecutionMode() == TaskExecutionMode::FOREGROUND) {
            os::memory::LockHolder lockGuard(pushForegroundLock_);
            foregroundTaskQueue_.Push(std::move(task));
        } else {
            os::memory::LockHolder lockGuard(pushBackgroundLock_);
            backgroundTaskQueue_.Push(std::move(task));
        }
    }

    std::optional<Task> PopTaskFromInternalQueues()
    {
        auto task = foregroundTaskQueue_.Pop();
        if (task.has_value()) {
            return task;
        }
        return backgroundTaskQueue_.Pop();
    }

    void EventOnTaskAdding(Task *task)
    {
        ASSERT(task != nullptr);
        task->EventOnTaskAdding();
    }

    /// subscriber_lock_ is used in case of calling new_tasks_callback_
    NewTasksCallback newTasksCallback_;
    SignalWorkersCallback signalWorkersCallback_;

    /// foreground part of TaskQueue
    mutable os::memory::Mutex pushForegroundLock_;
    InternalTaskQueue foregroundTaskQueue_;

    /// background part of TaskQueue
    mutable os::memory::Mutex pushBackgroundLock_;
    InternalTaskQueue backgroundTaskQueue_;
};

}  // namespace ark::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
