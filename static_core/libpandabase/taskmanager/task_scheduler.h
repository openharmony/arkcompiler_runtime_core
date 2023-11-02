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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H

#include "libpandabase/taskmanager/worker_thread.h"
#include "libpandabase/taskmanager/task_queue.h"
#include "libpandabase/taskmanager/task_statistics/task_statistics.h"
#include <vector>
#include <map>
#include <random>

namespace panda::taskmanager {

/**
 * Task Manager can register 3 queues with different type of tasks
 *  - GC queue(ECMA)
 *  - GC queue(ArkTS)
 *  - JIT queue
 */
class TaskScheduler {
public:
    NO_COPY_SEMANTIC(TaskScheduler);
    NO_MOVE_SEMANTIC(TaskScheduler);

    /**
     * @brief Creates an instance of TaskScheduler.
     * @param threads_count - number of worker that will be created be Task Manager
     * @param task_statistics_type - type of TaskStatistics that will be used in TaskScheduler
     */
    PANDA_PUBLIC_API static TaskScheduler *Create(
        size_t threads_count, TaskStatisticsImplType task_statistics_type = TaskStatisticsImplType::SIMPLE);

    /**
     * @brief Returns the pointer to TaskScheduler. If you use it before the Create or after Destroy methods, it will
     * return nullptr.
     */
    [[nodiscard]] PANDA_PUBLIC_API static TaskScheduler *GetTaskScheduler();

    /// @brief Deletes the existed TaskScheduler. You should not use it if you didn't use Create before.
    PANDA_PUBLIC_API static void Destroy();

    /// @brief Creates and starts workers with registered queues. After this method, you can not register new queues.
    PANDA_PUBLIC_API void Initialize();

    /**
     * @brief Method allocates, constructs and registers TaskQueue. If it already exists, method returns nullptr.
     * @param task_type - TaskType of future TaskQueue.
     * @param vm_type - VMType of future TaskQueue.
     * @param priority - value of priority:
     * TaskQueueInterface::MIN_PRIORITY <= priority <= TaskQueueInterface::MIN_PRIORITY
     * @tparam Allocator - allocator of Task that will be used in internal queues. By default is used
     * std::allocator<Task>
     */
    template <class Allocator = std::allocator<Task>>
    PANDA_PUBLIC_API TaskQueueInterface *CreateAndRegisterTaskQueue(
        TaskType task_type, VMType vm_type, uint8_t priority = TaskQueueInterface::DEFAULT_PRIORITY)
    {
        auto *queue = internal::TaskQueue<Allocator>::Create(task_type, vm_type, priority);
        if (UNLIKELY(queue == nullptr)) {
            return nullptr;
        }
        auto id = RegisterQueue(queue);
        if (UNLIKELY(id == INVALID_TASKQUEUE_ID)) {
            internal::TaskQueue<Allocator>::Destroy(queue);
            return nullptr;
        }
        return queue;
    }

    /**
     * @brief Method Destroy and Unregister TaskQueue
     * @param queue - TaskQueueInterface* of TaskQueue.
     * @tparam Allocator - allocator of Task that will be used to deallocate TaskQueue. Use the same allocator as you
     * have used in TaskScheduler::CreateAndRegisterTaskQueue method.
     */
    template <class Allocator = std::allocator<Task>>
    PANDA_PUBLIC_API void UnregisterAndDestroyTaskQueue(TaskQueueInterface *queue)
    {
        TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
        auto *schedulable_queue = task_queues_[id];

        schedulable_queue->UnsetNewTasksCallback();
        task_queues_.erase(id);
        internal::TaskQueue<Allocator>::Destroy(schedulable_queue);
    }

    /**
     * @brief Fills @arg worker (local queues) with tasks. It will stop if the size of the queue is equal to @arg
     * tasks_count.  Method @return bool value that indicates worker end. If it's true, workers should finish after the
     * execution of tasks.
     * @param worker - pointer on worker that should be fill will tasks
     * @param tasks_count - max number of tasks for filling
     */
    bool FillWithTasks(WorkerThread *worker, size_t tasks_count);

    /**
     * @brief Method returns Task with specified properties. If there are no tasks with that properties method will
     * return nullopt.
     * @param properties - TaskProperties of task we want to get.
     */
    [[nodiscard]] PANDA_PUBLIC_API std::optional<Task> GetTaskFromQueue(TaskProperties properties);

    /**
     * @brief Method waits all tasks with specified properties. This method should be used only from Main Thread and
     * only for finalization!
     * @param properties - TaskProperties of tasks we will wait to be completed.
     */
    PANDA_PUBLIC_API void WaitForFinishAllTasksWithProperties(TaskProperties properties);

    /// @brief This method indicates that workers can no longer wait for new tasks and be completed.
    PANDA_PUBLIC_API void Finalize();

    PANDA_PUBLIC_API ~TaskScheduler();

private:
    explicit TaskScheduler(size_t workers_count, TaskStatisticsImplType task_statistics_type);

    /**
     * @brief Registers a queue that was created externally. It should be valid until all workers finish, and the queue
     * should have a unique set of TaskType and VMType fields. You can not use this method after Initialize() method
     * @param queue: pointer to a valid TaskQueue.
     * @return TaskQueueId of queue that was added. If queue with same TaskType and VMType is already added, method
     * returns INVALID_TASKQUEUE_ID
     */
    PANDA_PUBLIC_API TaskQueueId RegisterQueue(internal::SchedulableTaskQueueInterface *queue);

    /**
     * @brief Method pops one task from internal queues based on priorities.
     * @return if queue are empty, returns nullopt, otherwise returns task.
     */
    [[nodiscard]] std::optional<Task> GetNextTask() REQUIRES(pop_from_task_queues_lock_);

    /**
     * @brief Method puts one @arg task to @arg worker.
     * @param worker - pointer on worker that should be fill will tasks
     * @param task - task that will be putted in worker
     */
    void PutTaskInWorker(WorkerThread *worker, Task &&task) REQUIRES(workers_lock_);

    /**
     * @brief This method @returns map from kinetic sum of non-empty queues to queues pointer
     * in the same order as they place in task_queues_. Use this method to choose next thread
     */
    std::map<size_t, internal::SchedulableTaskQueueInterface *> GetKineticPriorities() const
        REQUIRES(pop_from_task_queues_lock_);

    /// @brief Checks if task queues are empty
    bool AreQueuesEmpty() const;

    /// @brief Checks if there are no tasks in queues and workers
    bool AreNoMoreTasks() const;

    /**
     * @brief Method increment counter of new tasks and signal worker
     * @param properties - TaskProperties of task from queue that execute the callback
     * @param ivalue - the value by which the counter will be increased
     * @param was_empty - flage that should be true only, if queue was empty before last AddTask
     */
    void IncrementCounterOfAddedTasks(TaskProperties properties, size_t ivalue, bool was_empty);

    /**
     * @brief Method increment counter of finished tasks and signal Finalize waiter
     * @param counter_map - map from id to count of finished tasks
     */
    void IncrementCounterOfExecutedTasks(const TaskPropertiesCounterMap &counter_map);

    static TaskScheduler *instance_;

    size_t workers_count_;

    /// Pointers to Worker Threads.
    std::vector<WorkerThread *> workers_;

    /// workers_lock_ is used to synchronize a lot of workers in FillWithTasks method
    os::memory::Mutex workers_lock_;

    /// pop_from_task_queues_lock_ is used to guard popping from queues
    os::memory::Mutex pop_from_task_queues_lock_;

    /**
     * Map from TaskType and VMType to queue.
     * Can be changed only before Initialize methods.
     * Since we can change the map only before creating the workers, we do not need to synchronize access after
     * Initialize method
     */
    std::map<TaskQueueId, internal::SchedulableTaskQueueInterface *> task_queues_;

    /// task_scheduler_state_lock_ is used to check state of task
    os::memory::RecursiveMutex task_scheduler_state_lock_;

    /// queues_wait_cond_var_ is used when all registered queues are empty to wait until one of them will have a task
    os::memory::ConditionVariable queues_wait_cond_var_ GUARDED_BY(task_scheduler_state_lock_);

    /**
     * This cond var uses to wait for all tasks will be done.
     * It is used in Finalize() method.
     */
    os::memory::ConditionVariable finish_tasks_cond_var_ GUARDED_BY(task_scheduler_state_lock_);

    /// start_ is true if we used Initialize method
    std::atomic_bool start_ {false};

    /// finish_ is true when TaskScheduler finish Workers and TaskQueues
    bool finish_ GUARDED_BY(task_scheduler_state_lock_) {false};

    /// new_tasks_count_ represents count of new tasks
    TaskPropertiesCounterMap new_tasks_count_ GUARDED_BY(task_scheduler_state_lock_);

    /**
     * finished_tasks_count_ represents count of finished tasks;
     * Task is finished if:
     * - it was executed by Worker;
     * - it was gotten by main thread;
     */
    TaskPropertiesCounterMap finished_tasks_count_ GUARDED_BY(task_scheduler_state_lock_);

    std::mt19937 gen_;

    TaskStatistics *task_statistics_;
};

}  // namespace panda::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
