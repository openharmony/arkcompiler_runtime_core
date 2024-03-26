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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_TASK_STATISTICS_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_TASK_STATISTICS_H

#include "libpandabase/taskmanager/task.h"
#include <ostream>
#include <vector>

namespace ark::taskmanager {

enum class TaskStatisticsImplType : uint8_t { SIMPLE, FINE_GRAINED, LOCK_FREE, NO_STAT };
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskStatisticsImplType type);

PANDA_PUBLIC_API TaskStatisticsImplType TaskStatisticsImplTypeFromString(std::string_view string);

enum class TaskStatus : size_t {
    /// Status of tasks that were added in TaskQueue
    ADDED,

    /// Status of tasks that were executed on WorkerThread
    EXECUTED,

    /// Status of tasks that were popped from TaskQueue by calling method TaskScheduler::GetTaskFromQueue
    POPPED
};

constexpr auto ALL_TASK_STATUSES = std::array {TaskStatus::ADDED, TaskStatus::EXECUTED, TaskStatus::POPPED};

/**
 * TaskStatistics is a structure that counts the number of tasks that have received a certain
 * status. All statuses that are taken into account are listed in the TaskStatus enum.
 */
class TaskStatistics {
public:
    TaskStatistics();
    virtual ~TaskStatistics() = default;
    NO_COPY_SEMANTIC(TaskStatistics);
    NO_MOVE_SEMANTIC(TaskStatistics);

    /**
     * @brief Method increments status counter for tasks with specified TaskProperties.
     * @param status - status of task which counter you want to increment.
     * @param properties - TaskProperties of tasks which counter will be incremented.
     * @param count - value by which the counter will be increased.
     */
    virtual void IncrementCount(TaskStatus status, TaskProperties properties, size_t count) = 0;

    /**
     * @brief Method returns status counter for tasks with specified TaskProperties
     * @param status - status of task which counter you want to return.
     * @param properties - TaskProperties of tasks which counter will be return.
     * @return status counter.
     */
    virtual size_t GetCount(TaskStatus status, TaskProperties properties) const = 0;

    /**
     * @brief Method returns count of tasks that are in system. Task in system means that task was added but wasn't
     * executed or popped.
     * @return count of tasks in system for all properties.
     */
    virtual size_t GetCountOfTaskInSystem() const = 0;

    /**
     * @brief Method returns count of tasks that are in system for specified TaskProperties. Task in system means
     * that task was added but wasn't executed or popped.
     * @param properties - TaskProperties of tasks which counter will be return.
     * @return count of tasks in system for specified properties.
     */
    virtual size_t GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const = 0;

    /**
     * @brief Method resets internal counters for all stats and all properties. It should be used only if it's only
     * if there is a guarantee that other threads do not change states of all counters
     */
    virtual void ResetAllCounters() = 0;

    /**
     * @brief Method resets internal counters for all stats but with specified properties. It should be used only if
     * there is a guarantee that other threads do not change state of counters that you want to reset.
     * @param properties - TaskProperties of tasks which counters will be reset.
     */
    virtual void ResetCountersWithTaskProperties(TaskProperties properties) = 0;

protected:
    std::vector<TaskProperties> allTaskProperties_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_TASK_STATISTICS_H
