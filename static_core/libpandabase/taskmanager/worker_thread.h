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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H
#define PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H

#include "libpandabase/taskmanager/schedulable_task_queue_interface.h"
#include "libpandabase/taskmanager/utils/worker_thread_local_queue.h"
#include "libpandabase/taskmanager/utils/task_selector.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/os/thread.h"
#include <thread>

namespace ark::taskmanager {

using TaskPropertiesCounterMap = std::unordered_map<TaskProperties, size_t, TaskProperties::Hash>;

class TaskScheduler;

class WorkerThread {
public:
    NO_COPY_SEMANTIC(WorkerThread);
    NO_MOVE_SEMANTIC(WorkerThread);

    static constexpr size_t WORKER_QUEUE_SIZE = 4UL;

    /// @brief functor that should add task in worker
    using AddTaskToWorkerFunc = std::function<void(Task &&)>;

    explicit WorkerThread(const std::string &name);
    ~WorkerThread();

    /**
     * @brief Adds task in internal queues.
     * @param task - task that will be added in internal queues
     */
    void AddTask(Task &&task);

    /// @brief Returns true if all internal queues are empty
    bool IsEmpty() const;

    /// @brief Returns count of tasks in local queue
    size_t Size() const;

    /// @brief Returns count of task in local queue with specified properties
    size_t CountOfTasksWithProperties(TaskProperties properties) const;

    /// @brief Waits for worker finish
    void Join();

    /**
     * @brief register all workers in local queue
     * @param workers - ref to vector with all workers
     */
    void RegisterAllWorkersInLocalQueue(const std::vector<WorkerThread *> &workers);

    std::string GetWorkerName() const;

    /**
     * @brief method returns id of worker to pop tasks.
     * @param worker: pointer on WorkerThread which id you want to get. It should be added with
     * RegisterAllWorkersInLocalQueue(...) method
     */
    size_t GetLocalWorkerQueuePopId(WorkerThread *worker) const;

    /// @brief method returns id of TaskScheduler.
    size_t GetLocalWorkerQueueSchedulerPopId() const;

    /// @brief method starts WorkerLoop. All workers should be registered before Start executing
    void Start();

    void SetStolenTask(Task &&stolenTask);

    /**
     * @brief Fills with tasks other WorkerThread.
     * @tparam Properties: variadic template class of TaskProperties that represent using of properties to pop
     * @param addTaskFunc - functor that should add new task in other WorkerThread
     * @param prop - TaskProperties of task is wanted to be fill with
     * @param taskCount - Count of tasks wanted to pop
     * @param id: id worker got after registration
     * @returns count of tasks that was added
     */
    template <class... Properties>
    size_t PANDA_PUBLIC_API GiveTasksToAnotherWorker(const AddTaskToWorkerFunc &addTaskFunc, size_t taskCount,
                                                     size_t id, Properties... prop)
    {
        static_assert(sizeof...(prop) < 2UL, "it's possible to have only one prop arg or no one at all");

        size_t count = 0;
        for (; count != taskCount; count++) {
            // Try to pop task
            std::optional<Task> task;
            if constexpr (sizeof...(prop) == 0) {
                task = localQueue_.Pop(id);
            } else {
                static_assert(std::is_same<std::tuple_element_t<0U, std::tuple<Properties...>>, TaskProperties>::value);
                task = localQueue_.Pop(id, std::get<TaskProperties>(std::tuple(prop...)));
            }
            // If pop task returned nullopt need to finish execution
            if (UNLIKELY(!task.has_value())) {
                break;
            }
            addTaskFunc(std::move(task.value()));
        }
        return count;
    }

    void TryDeleteRetiredPtrs();

private:
    void ExecuteTask(Task *task);

    /// @brief Main workers algorithm
    void WorkerLoop();

    /**
     * @brief pops and executes all tasks from internal queue.
     * Also counts executed tasks in finishedTasksCounterMap_.
     */
    size_t ExecuteTasksFromLocalQueue();

    /// @brief method executes task from stolenTask_ field
    void ExecuteStolenTask();

    /**
     * @brief method wait for starting
     * @see Start
     */
    void WaitForStart();

    std::thread *thread_ = nullptr;
    TaskScheduler *scheduler_ = nullptr;
    std::string name_;

    bool start_ {false};
    os::memory::Mutex startWaitLock_;
    os::memory::ConditionVariable startWaitCondVar_ GUARDED_BY(startWaitLock_);

    /**
     * @brief Here should be saved task that was stolen from other workers local queue
     * @see SetStolenTask
     * @see ExecuteStolenTask
     */
    Task stolenTask_;

    /**
     * @brief finishedTasksCounterMap_: map that consider info about executed tasks in one WorkerLoop iteration. Is used
     * to notify TaskStatistics.
     * @see ExecuteTasksFromLocalQueue
     */
    TaskPropertiesCounterMap finishedTasksCounterMap_;

    /**
     * @brief localQueue_ is set of lock-free queues with registration of consumers. Registration uses for correct
     * memory free.
     * @see RegisterAllWorkersInLocalQueue
     * @see GetLocalWorkerQueuePopId
     * @see perWorkerPopId_
     */
    internal::WorkerThreadLocalQueue<WORKER_QUEUE_SIZE> localQueue_;

    /**
     * @brief perWorkerPopId_: is map that uses when one worker wants to pop task from other worker's local queue.
     * @see RegisterAllWorkersInLocalQueue
     * @see GiveTasksToAnotherWorker
     */
    std::unordered_map<WorkerThread *, size_t> perWorkerPopId_;

    /**
     * @brief schedulerPopId_: specific id for TaskScheduler. TaskScheduler uses it when Helper tries to steal task.
     * @see GetLocalWorkerQueueSchedulerPopId
     * @see TaskScheduler::StealTaskFromOtherWorker
     */
    size_t schedulerPopId_ {0};

    size_t countOfExecutedTask_ {0};
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H
