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

#include "libpandabase/taskmanager/task_statistics/simple_task_statistics_impl.h"
#include "libpandabase/taskmanager/task.h"

namespace ark::taskmanager {

SimpleTaskStatisticsImpl::SimpleTaskStatisticsImpl()
{
    for (const auto &status : ALL_TASK_STATUSES) {
        perStatusLock_[status];  // Here we use implicitly construction of mutex
        for (const auto &properties : allTaskProperties_) {
            taskPropertiesCounterMap_[status][properties] = 0;
        }
    }
}

void SimpleTaskStatisticsImpl::IncrementCount(TaskStatus status, TaskProperties properties, size_t count)
{
    os::memory::LockHolder lockHolder(perStatusLock_.at(status));
    taskPropertiesCounterMap_.at(status)[properties] += count;
}

size_t SimpleTaskStatisticsImpl::GetCount(TaskStatus status, TaskProperties properties) const
{
    os::memory::LockHolder lockHolder(perStatusLock_.at(status));
    return taskPropertiesCounterMap_.at(status).at(properties);
}

size_t SimpleTaskStatisticsImpl::GetCountOfTaskInSystem() const
{
    os::memory::LockHolder addedLockHolder(perStatusLock_.at(TaskStatus::ADDED));
    os::memory::LockHolder executedLockHolder(perStatusLock_.at(TaskStatus::EXECUTED));
    os::memory::LockHolder poppedLockHolder(perStatusLock_.at(TaskStatus::POPPED));

    size_t inSystemTasksCount = 0;
    for (const auto &properties : allTaskProperties_) {
        size_t addedTaskCountVal = taskPropertiesCounterMap_.at(TaskStatus::ADDED).at(properties);
        size_t executedTaskCountVal = taskPropertiesCounterMap_.at(TaskStatus::EXECUTED).at(properties);
        size_t poppedTaskCountVal = taskPropertiesCounterMap_.at(TaskStatus::POPPED).at(properties);

        ASSERT(addedTaskCountVal >= executedTaskCountVal + poppedTaskCountVal);
        inSystemTasksCount += addedTaskCountVal - executedTaskCountVal - poppedTaskCountVal;
    }
    return inSystemTasksCount;
}

size_t SimpleTaskStatisticsImpl::GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const
{
    os::memory::LockHolder addedLockHolder(perStatusLock_.at(TaskStatus::ADDED));
    os::memory::LockHolder executedLockHolder(perStatusLock_.at(TaskStatus::EXECUTED));
    os::memory::LockHolder poppedLockHolder(perStatusLock_.at(TaskStatus::POPPED));

    size_t addedTaskCountVal = taskPropertiesCounterMap_.at(TaskStatus::ADDED).at(properties);
    size_t executedTaskCountVal = taskPropertiesCounterMap_.at(TaskStatus::EXECUTED).at(properties);
    size_t poppedTaskCountVal = taskPropertiesCounterMap_.at(TaskStatus::POPPED).at(properties);

    ASSERT(addedTaskCountVal >= executedTaskCountVal + poppedTaskCountVal);
    return addedTaskCountVal - executedTaskCountVal - poppedTaskCountVal;
}

void SimpleTaskStatisticsImpl::ResetAllCounters()
{
    for (const auto &properties : allTaskProperties_) {
        ResetCountersWithTaskProperties(properties);
    }
}

void SimpleTaskStatisticsImpl::ResetCountersWithTaskProperties(TaskProperties properties)
{
    // Getting locks for every state counter with specified properties
    std::unordered_map<TaskStatus, os::memory::LockHolder<os::memory::Mutex>> lockHolderMap;
    for (const auto &status : ALL_TASK_STATUSES) {
        lockHolderMap.emplace(status, perStatusLock_.at(status));
    }
    for (const auto &status : ALL_TASK_STATUSES) {
        taskPropertiesCounterMap_[status][properties] = 0;
    }
}

}  // namespace ark::taskmanager
