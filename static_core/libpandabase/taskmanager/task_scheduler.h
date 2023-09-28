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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H

#include "libpandabase/taskmanager/worker_thread.h"
#include "libpandabase/taskmanager/task_queue.h"
#include <optional>
#include <vector>
#include <map>

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
     */
    PANDA_PUBLIC_API static TaskScheduler *Create(size_t threads_count);

    /**
     * @brief Returns the pointer to TaskScheduler. If you use it before the Create or after Destroy methods, it will
     * return nullptr.
     */
    PANDA_PUBLIC_API static TaskScheduler *GetTaskScheduler();

    /// @brief Deletes the existed TaskScheduler. You should not use it if you didn't use Create before.
    PANDA_PUBLIC_API static void Destroy();

    /**
     * @brief Registers a queue that was created outside. It should be valid until all workers finish, and the queue
     * should have a unique set of TaskType and VMType fields. You can not use this method after Initialize() method
     * @param queue: pointer to a valid TaskQueue.
     * @return true if queue was successfully registered
     */
    PANDA_PUBLIC_API bool RegisterQueue(TaskQueue *queue);

    /// @brief Creates and starts workers with registered queues. After this method, you can not register new queues.
    PANDA_PUBLIC_API void Initialize();

    /**
     * @brief Adds new task in the queue specified for the inserted for it. Should be used by other task that want to
     * add new ones. @arg task should have a queue inside Task Managed to be added in it. @return the size of the queue
     * after the task was added to it.
     * @param task - task that will be added in one queue with the same TaskType and VMType.
     */
    PANDA_PUBLIC_API size_t AddTask(Task &&task);

    /**
     * @brief Fills @arg worker (local queues) with tasks. It will stop if the size of the queue is equal to @arg
     * tasks_count.  Method @return bool value that indicates worker end. If it's true, workers should finish after the
     * execution of tasks.
     * @param worker - pointer on worker that should be fill will tasks
     * @param tasks_count - max number of tasks for filling
     */
    bool FillWithTasks(WorkerThread *worker, size_t tasks_count);

    /**
     * @brief Method returns Task from specific queue with specific execution mode using @arg properties. If it has no
     * tasks method will return nullopt.
     * @param properties - properties of task that we want to get.
     */
    [[nodiscard]] PANDA_PUBLIC_API std::optional<Task> GetTaskByProperties(TaskProperties properties);

    /// @brief This method indicates that workers can no longer wait for new tasks and be completed.
    PANDA_PUBLIC_API void Finalize();

    PANDA_PUBLIC_API ~TaskScheduler();

private:
    explicit TaskScheduler(size_t workers_count);

    /// @brief Method pops one task from internal queues based on priorities.
    [[nodiscard]] Task GetNextTask() REQUIRES(task_manager_lock_);

    /**
     * @brief Method puts one @arg task to @arg worker.
     * Also, it makes is_worker_queue_filling_ = false to indicate that task transporting is finished.
     * @param worker - pointer on worker that should be fill will tasks
     * @param task - task that will be putted in worker
     */
    void PutTaskInWorker(WorkerThread *worker, Task &&task) REQUIRES(task_manager_lock_);

    /**
     * @brief This method @returns map from kinetic sum of non-empty queues to queues pointer
     * in the same order as they place in task_queues_. Use this method to choose next thread
     */
    std::map<size_t, TaskQueue *> GetKineticPriorities() const REQUIRES(task_manager_lock_);

    /// @brief Checks if task queues are empty
    bool AreQueuesEmpty() const REQUIRES(task_manager_lock_);

    /// @brief Checks if there are no tasks in queues and workers
    bool AreNoMoreTasks() const REQUIRES(task_manager_lock_);

    static TaskScheduler *instance_;

    size_t workers_count_;

    /// Pointers to Worker Threads.
    std::vector<WorkerThread *> workers_;

    /// workers_lock_ is used to synchronize a lot of workers in FillWithTasks method
    os::memory::Mutex workers_lock_;

    /**
     * Map from TaskType and VMType to queue.
     * Can be changed only before Initialize methods.
     * Since we can change the map only before creating the workers, we do not need to synchronize access after
     * Initialize method
     */
    std::map<TaskQueueTraits, TaskQueue *> task_queues_;

    /**
     * workers_lock_ is used in case of access to shared resources operated by the task manager:
     * - in RegisterQueue to synchronize modification of task_queues_ before Initialize method;
     * - in FillWithMethods to synchronize access for multiple workers;
     */
    os::memory::Mutex task_manager_lock_;

    /// queues_wait_cond_var_ is used when all registered queues are empty to wait until one of them will have a task
    os::memory::ConditionVariable queues_wait_cond_var_ GUARDED_BY(task_manager_lock_);

    /**
     * This cond var uses to wait for all tasks will be done.
     * It is used in Finalize() method.
     */
    os::memory::ConditionVariable finish_tasks_cond_var_ GUARDED_BY(task_manager_lock_);

    /// start_ is true if we used Initialize method
    std::atomic_bool start_ {false};

    /// finish_ is true when TaskScheduler finish Workers and TaskQueues
    bool finish_ GUARDED_BY(task_manager_lock_) {false};

    /**
     * This bool is true only in time period when:
     *  - Task is popped from TaskQueue;
     *  - Task still is not in WorkThread queue;
     * If we don't use this atomic, Finalize() method can finish when one task still exist.
     */
    bool is_worker_queue_filling_ GUARDED_BY(task_manager_lock_) {false};
};

}  // namespace panda::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H