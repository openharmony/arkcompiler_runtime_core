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

#include "libpandabase/taskmanager/task_statistics/fine_grained_task_statistics_impl.h"
#include "libpandabase/taskmanager/task.h"

namespace ark::taskmanager {

FineGrainedTaskStatisticsImpl::FineGrainedTaskStatisticsImpl()
{
    for (const auto &status : ALL_TASK_STATUSES) {
        for (const auto &properties : allTaskProperties_) {
            taskPropertiesCounterMap_[status].emplace(properties, 0);
        }
    }
}

void FineGrainedTaskStatisticsImpl::IncrementCount(TaskStatus status, TaskProperties properties, size_t count)
{
    taskPropertiesCounterMap_.at(status)[properties] += count;
}

size_t FineGrainedTaskStatisticsImpl::GetCount(TaskStatus status, TaskProperties properties) const
{
    return taskPropertiesCounterMap_.at(status).at(properties).GetValue();
}

size_t FineGrainedTaskStatisticsImpl::GetCountOfTaskInSystem() const
{
    size_t inSystemTasksCount = 0;
    for (const auto &properties : allTaskProperties_) {
        inSystemTasksCount +=
            CalcCountOfTasksInSystem(taskPropertiesCounterMap_.at(TaskStatus::ADDED).at(properties),
                                     taskPropertiesCounterMap_.at(TaskStatus::EXECUTED).at(properties),
                                     taskPropertiesCounterMap_.at(TaskStatus::POPPED).at(properties));
    }
    return inSystemTasksCount;
}

size_t FineGrainedTaskStatisticsImpl::GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const
{
    return CalcCountOfTasksInSystem(taskPropertiesCounterMap_.at(TaskStatus::ADDED).at(properties),
                                    taskPropertiesCounterMap_.at(TaskStatus::EXECUTED).at(properties),
                                    taskPropertiesCounterMap_.at(TaskStatus::POPPED).at(properties));
}

void FineGrainedTaskStatisticsImpl::ResetAllCounters()
{
    for (const auto &properties : allTaskProperties_) {
        ResetCountersWithTaskProperties(properties);
    }
}

void FineGrainedTaskStatisticsImpl::ResetCountersWithTaskProperties(TaskProperties properties)
{
    // Getting locks for every state counter with specified properties
    std::unordered_map<TaskStatus, os::memory::LockHolder<os::memory::Mutex>> lockHolderMap;
    for (const auto &status : ALL_TASK_STATUSES) {
        lockHolderMap.emplace(status, taskPropertiesCounterMap_[status][properties].GetMutex());
    }
    for (const auto &status : ALL_TASK_STATUSES) {
        taskPropertiesCounterMap_[status][properties].NoGuardedSetValue(0);
    }
}

}  // namespace ark::taskmanager
