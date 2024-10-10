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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H

#include "libpandabase/taskmanager/task_queue.h"
#include "libpandabase/taskmanager/utils/wait_list.h"
#include "libpandabase/taskmanager/worker_thread.h"
#include "libpandabase/taskmanager/utils/task_time_stats.h"
#include <vector>
#include <map>
#include <queue>

namespace ark::taskmanager {
/**
 * Task Manager can register 3 queues with different type of tasks
 *  - GC queue(ECMA)
 *  - GC queue(ArkTS)
 *  - JIT queue
 */
class TaskScheduler {
    using TaskPropertiesCounterMap = std::unordered_map<TaskProperties, size_t, TaskProperties::Hash>;

public:
    NO_COPY_SEMANTIC(TaskScheduler);
    NO_MOVE_SEMANTIC(TaskScheduler);

    static constexpr uint64_t TASK_WAIT_TIMEOUT = 1U;

    using LocalTaskQueue = std::queue<Task>;

    /**
     * @brief Creates an instance of TaskScheduler.
     * @param threadsCount - number of worker that will be created be Task Manager
     * @param taskStatsType - type of TaskStatistics that will be used in TaskScheduler
     */
    PANDA_PUBLIC_API static TaskScheduler *Create(size_t threadsCount,
                                                  TaskTimeStatsType taskStatsType = TaskTimeStatsType::NO_STATISTICS);

    /**
     * @brief Returns the pointer to TaskScheduler. If you use it before the Create or after Destroy methods, it
     * will return nullptr.
     */
    [[nodiscard]] PANDA_PUBLIC_API static TaskScheduler *GetTaskScheduler();

    /// @brief Deletes the existed TaskScheduler. You should not use it if you didn't use Create before.
    PANDA_PUBLIC_API static void Destroy();

    /// @brief Returns true if TaskScheduler outputs log info
    PANDA_PUBLIC_API bool IsTaskLifetimeStatisticsUsed() const;

    TaskTimeStatsBase *GetTaskTimeStats() const;
    TaskTimeStatsType GetTaskTimeStatsType() const;

    /**
     * @brief Creates and starts workers with registered queues. After this method, you can not register new
     * queues.
     */
    PANDA_PUBLIC_API void Initialize();

    /**
     * @brief Method allocates, constructs and registers TaskQueue. If it already exists, method returns nullptr.
     * @param taskType - TaskType of future TaskQueue.
     * @param vmType - VMType of future TaskQueue.
     * @param priority - value of priority:
     * TaskQueueInterface::MIN_PRIORITY <= priority <= TaskQueueInterface::MIN_PRIORITY
     * @tparam Allocator - allocator of Task that will be used in internal queues. By default is used
     * std::allocator<Task>
     */
    template <class Allocator = std::allocator<Task>>
    PANDA_PUBLIC_API TaskQueueInterface *CreateAndRegisterTaskQueue(
        TaskType taskType, VMType vmType, uint8_t priority = TaskQueueInterface::DEFAULT_PRIORITY)
    {
        auto *queue = internal::TaskQueue<Allocator>::Create(taskType, vmType, priority);
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
     * @tparam Allocator - allocator of Task that will be used to deallocate TaskQueue. Use the same allocator as
     * you have used in TaskScheduler::CreateAndRegisterTaskQueue method.
     */
    template <class Allocator = std::allocator<Task>>
    PANDA_PUBLIC_API void UnregisterAndDestroyTaskQueue(TaskQueueInterface *queue)
    {
        TaskQueueId id(queue->GetTaskType(), queue->GetVMType());
        auto *schedulableQueue = taskQueues_[id];

        schedulableQueue->UnsetCallbacks();
        taskQueues_.erase(id);
        internal::TaskQueue<Allocator>::Destroy(schedulableQueue);
    }

    /**
     * @brief Fills @arg worker (local queues) with tasks. The number of tasks obtained depends on the max size of the
     * worker's local queue and the number of workers. The algorithm strives to give the same number of tasks to all
     * workers. If queues are empty and TaskScheduler is not destroying workers will wait.  If it's true, workers should
     * finish after the execution of tasks.
     * @param worker - pointer on worker that should be fill will tasks.
     * @returns true if Worker should finish loop execution, otherwise returns false
     */
    bool FillWithTasks(WorkerThread *worker);

    /**
     * @brief Method steal task from worker with the largest number of tasks and push it to gotten worker.
     * @param worker: pointer to WorkerThread that should be fill with stollen task
     */
    void StealTaskFromOtherWorker(WorkerThread *taskReceiver);

    /// @brief Checks if task queues are empty
    bool AreQueuesEmpty() const;

    /// @brief Checks if worker local queues are empty
    bool AreWorkersEmpty() const;

    /**
     * @brief Method increment counter of finished tasks and signal Finalize waiter
     * @param counterMap - map from id to count of finished tasks
     * @return count of executed tasks
     */
    size_t IncrementCounterOfExecutedTasks(const TaskPropertiesCounterMap &counterMap);

    /**
     * @brief Executes tasks with specific properties. It will get them from queue or steal from workers.
     * @param properties - TaskProperties of tasks needs to help
     * @returns real count of tasks that was executed
     */
    PANDA_PUBLIC_API size_t HelpWorkersWithTasks(TaskProperties properties);

    /**
     * @brief Method waits all tasks with specified properties. This method should be used only from Main Thread and
     * only for finalization!
     * @param properties - TaskProperties of tasks we will wait to be completed.
     */
    PANDA_PUBLIC_API void WaitForFinishAllTasksWithProperties(TaskProperties properties);

    /**
     * @brief Adds the task to the wait list with timeout. After the timeout expires, the task will be added
     * to its corresponding TaskQueue.
     * @param task: instance of task
     * @param time: waiting time in milliseconds
     * @returns unique waiter id. It can be used to signal wait list to add task to TaskQueue
     */
    PANDA_PUBLIC_API WaiterId AddTaskToWaitListWithTimeout(Task &&task, uint64_t time);

    /**
     * @brief Adds the task to the wait list.
     * @param task: instance of task
     * @returns unique waiter id. It can be used to signal wait list to add task to TaskQueue
     * @see TaskScheduler::SignalWaitList
     */
    PANDA_PUBLIC_API WaiterId AddTaskToWaitList(Task &&task);

    /**
     * @brief Signals wait list to add task in TaskQueue.
     * @param waiterId: unique waiter id
     * @see TaskScheduler::AddTaskToWaitListWithTimeout, TaskScheduler::AddTaskToWaitList
     */
    PANDA_PUBLIC_API void SignalWaitList(WaiterId waiterId);

    /// @brief This method indicates that workers can no longer wait for new tasks and be completed.
    PANDA_PUBLIC_API void Finalize();

    PANDA_PUBLIC_API ~TaskScheduler();

private:
    explicit TaskScheduler(size_t workersCount, TaskTimeStatsType taskTimeStatsType);

    /**
     * @brief Method get and execute tasks with specified properties. If there are no tasks with that properties method
     * will return nullopt.
     * @param properties - TaskProperties of task we want to get.
     * @returns real count of gotten tasks
     */
    size_t GetAndExecuteSetOfTasksFromQueue(TaskProperties properties);

    /**
     * @brief Method steal and execute one task from one Worker. Method will find worker the largest number of tasks,
     * steal one from it and execute.
     * @param properties - TaskProperties of tasks needs to help
     * @returns 1 if stealing was done successfully
     */
    size_t StealAndExecuteOneTaskFromWorkers(TaskProperties properties);

    /**
     * @brief Registers a queue that was created externally. It should be valid until all workers finish, and the
     * queue should have a unique set of TaskType and VMType fields. You can not use this method after Initialize()
     * method
     * @param queue: pointer to a valid TaskQueue.
     * @return TaskQueueId of queue that was added. If queue with same TaskType and VMType is already added, method
     * returns INVALID_TASKQUEUE_ID
     */
    PANDA_PUBLIC_API TaskQueueId RegisterQueue(internal::SchedulableTaskQueueInterface *queue);

    /**
     * @brief Method pops one task from internal queues based on priorities.
     * @return if queue are empty, returns nullopt, otherwise returns task.
     */
    [[nodiscard]] std::optional<Task> GetNextTask() REQUIRES(popFromTaskQueuesLock_);

    /// @brief Checks if there are no tasks in queues and workers
    bool AreNoMoreTasks() const;

    /**
     * @brief Method puts tasks to @arg worker. Queue and count of tasks depends on selectedQueues_. After
     * execution of the method selectedQueues_ will be empty.
     * @param worker - pointer on worker that should be fill with tasks
     * @param selectedQueue - count of selected tasks for all TaskQueueId
     * @return count of task that was gotten by worker.
     */
    size_t PutTasksInWorker(WorkerThread *worker, TaskQueueId selectedQueue);

    /**
     * @brief Method increment counter of new tasks and signal worker
     * @param properties - TaskProperties of task from queue that execute the callback
     * @param ivalue - the value by which the counter will be increased
     */
    void IncrementCounterOfAddedTasks(TaskProperties properties, size_t ivalue);

    /// @brief Method signals workers if it's needed
    void SignalWorkers();

    void IncrementCountOfTasksInSystem(TaskProperties prop, size_t count);

    void DecrementCountOfTasksInSystem(TaskProperties prop, size_t count);

    size_t GetCountOfTasksInSystemWithTaskProperties(TaskProperties prop) const;

    size_t GetCountOfTasksInSystem() const;

    internal::SchedulableTaskQueueInterface *GetQueue(TaskQueueId id) const;

    void PutWaitTaskInLocalQueue(LocalTaskQueue &queue) REQUIRES(taskSchedulerStateLock_);

    void PutTaskInTaskQueues(LocalTaskQueue &queue);

    /**
     * @brief Method waits until new tasks coming or finishing of Task Scheduler usage
     * @return true if TaskScheduler have tasks to manager
     */
    bool WaitUntilNewTasks() REQUIRES(taskSchedulerStateLock_);

    static TaskScheduler *instance_;

    size_t workersCount_;

    /// Pointers to Worker Threads.
    std::vector<WorkerThread *> workers_;

    /// Iterator for workers_ to balance stealing
    size_t workersIterator_ = {0UL};

    /// pop_from_task_queues_lock_ is used to guard popping from queues
    os::memory::Mutex popFromTaskQueuesLock_;

    /// Represents count of task that sleeps
    std::atomic_size_t waitWorkersCount_ {0UL};

    /**
     * Map from TaskType and VMType to queue.
     * Can be changed only before Initialize methods.
     * Since we can change the map only before creating the workers, we do not need to synchronize access after
     * Initialize method
     */
    std::map<TaskQueueId, internal::SchedulableTaskQueueInterface *> taskQueues_;

    /// task_scheduler_state_lock_ is used to check state of task
    os::memory::RecursiveMutex taskSchedulerStateLock_;

    /**
     * queues_wait_cond_var_ is used when all registered queues are empty to wait until one of them will have a
     * task
     */
    os::memory::ConditionVariable queuesWaitCondVar_ GUARDED_BY(taskSchedulerStateLock_);

    /**
     * This cond var uses to wait for all tasks will be done.
     * It is used in Finalize() method.
     */
    os::memory::ConditionVariable finishTasksCondVar_ GUARDED_BY(taskSchedulerStateLock_);

    /// start_ is true if we used Initialize method
    std::atomic_bool start_ {false};

    /// finish_ is true when TaskScheduler finish Workers and TaskQueues
    bool finish_ GUARDED_BY(taskSchedulerStateLock_) {false};

    /// newTasksCount_ represents count of new tasks
    TaskPropertiesCounterMap newTasksCount_ GUARDED_BY(taskSchedulerStateLock_);

    std::atomic_size_t waitToFinish_ {0UL};

    std::atomic_bool disableHelpers_ {false};

    /**
     * finishedTasksCount_ represents count of finished tasks;
     * Task is finished if:
     * - it was executed by Worker;
     * - it was gotten by main thread;
     */
    TaskPropertiesCounterMap finishedTasksCount_ GUARDED_BY(taskSchedulerStateLock_);

    /**
     * Represents count of tasks that exist in TaskScheduler system per TaskProperties. Task in system means that task
     * was added but wasn't executed or popped.
     */
    std::unordered_map<TaskProperties, std::atomic_size_t, TaskProperties::Hash> countOfTasksInSystem_;

    TaskTimeStatsBase *taskTimeStats_ = nullptr;
    TaskTimeStatsType taskTimeStatsType_;

    internal::TaskSelector selector_;

    WaitList<Task> waitList_ GUARDED_BY(taskSchedulerStateLock_);
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
