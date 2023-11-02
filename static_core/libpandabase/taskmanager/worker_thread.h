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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H
#define PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H

#include "libpandabase/taskmanager/task.h"
#include <thread>
#include <queue>
#include <unordered_map>

namespace panda::taskmanager {

class TaskScheduler;

using TaskPropertiesCounterMap = std::unordered_map<TaskProperties, size_t, TaskProperties::Hash>;

class WorkerThread {
public:
    NO_COPY_SEMANTIC(WorkerThread);
    NO_MOVE_SEMANTIC(WorkerThread);

    /**
     * FinishedTasksCallback instance should be called after tasks finishing. As argument you should input count of
     * finished tasks.
     */
    using FinishedTasksCallback = std::function<void(TaskPropertiesCounterMap)>;

    static constexpr size_t WORKER_QUEUE_SIZE = 10;

    explicit WorkerThread(FinishedTasksCallback callback, size_t tasks_count = WORKER_QUEUE_SIZE);
    ~WorkerThread();

    /**
     * @brief Adds task in internal queues
     * @param task - task that will be added in internal queues
     */
    void AddTask(Task &&task);

    /// @brief Waits for worker finish
    void Join();

    /// @brief Returns true if all internal queues are empty
    bool IsEmpty() const;

private:
    [[nodiscard]] Task PopTask();

    void WorkerLoop(size_t tasks_count);

    void ExecuteTasks();

    std::thread *thread_;

    std::queue<Task> background_queue_;
    std::queue<Task> foreground_queue_;

    size_t size_ {0};

    TaskPropertiesCounterMap finished_tasks_counter_map_;

    FinishedTasksCallback finished_tasks_callback_;
};

}  // namespace panda::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H
