/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_TASK_SELECTOR_H
#define LIBPANDABASE_TASKMANAGER_UTILS_TASK_SELECTOR_H

#include "libpandabase/taskmanager/schedulable_task_queue_interface.h"
#include <vector>
#include <map>

namespace ark::taskmanager::internal {

class TaskSelector {
    /// PQPair is pair of priority and queue id. It's used in heap.
    using PQPair = std::pair<size_t, TaskQueueId>;

public:
    explicit TaskSelector(const std::map<TaskQueueId, internal::SchedulableTaskQueueInterface *> &taskQueues);

    /// @brief inits selection after adding all queues
    void Init();

    /**
     * @brief Method next TaskQueue to pop tasks from it
     * @returns TaskQueueId to pop if it's exists, else returns INVALID_TASKQUEUE_ID
     */
    TaskQueueId SelectQueue();

private:
    /// taskQueues_ is ref on map from id to pointer on queue
    const std::map<TaskQueueId, internal::SchedulableTaskQueueInterface *> &taskQueues_;

    /**
     * priorityQueues is a heap with PQPair elements type.
     * It's used for priority searching of queues.
     */
    std::vector<PQPair> priorityQueue_;
};

}  // namespace ark::taskmanager::internal

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_TASK_SELECTOR_H
