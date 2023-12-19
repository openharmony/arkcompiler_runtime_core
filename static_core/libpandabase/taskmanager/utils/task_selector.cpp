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

#include "libpandabase/taskmanager/utils/task_selector.h"
#include <algorithm>

namespace ark::taskmanager::internal {

TaskSelector::TaskSelector(const std::map<TaskQueueId, internal::SchedulableTaskQueueInterface *> &taskQueues)
    : taskQueues_(taskQueues)
{
}

void TaskSelector::Init()
{
    priorityQueue_.reserve(taskQueues_.size());
    for (auto &[id, queue] : taskQueues_) {
        size_t priority = queue->GetPriority();
        priorityQueue_.emplace_back(priority, id);
    }
    std::make_heap(priorityQueue_.begin(), priorityQueue_.end());
}

TaskQueueId TaskSelector::SelectQueue()
{
    size_t popCount = 0;
    TaskQueueId resultId = INVALID_TASKQUEUE_ID;
    // First we need to find queue that have tasks
    while (popCount < priorityQueue_.size()) {
        // Getting max priority from priorityQueue_
        size_t frontPriority = 0;
        std::tie(frontPriority, std::ignore) = priorityQueue_.front();
        // if it's zero it means that queue should be rebuild with new priorities from queue
        if (frontPriority == 0) {
            for (auto &[priority, id] : priorityQueue_) {
                priority = taskQueues_.at(id)->GetPriority();
            }
        }
        // Select queue with highest priority
        std::pop_heap(priorityQueue_.begin(), priorityQueue_.end() - popCount);
        popCount++;
        auto &[priority, id] = priorityQueue_[priorityQueue_.size() - popCount];
        // Decrement it's priority
        priority--;
        // If queue isn't empty, we select it
        if (LIKELY(!taskQueues_.at(id)->IsEmpty())) {
            resultId = id;
            break;
        }
    }
    // Finally push all queues back
    while (popCount != 0) {
        popCount--;
        std::push_heap(priorityQueue_.begin(), priorityQueue_.end() - popCount);
    }
    return resultId;
}

}  // namespace ark::taskmanager::internal
