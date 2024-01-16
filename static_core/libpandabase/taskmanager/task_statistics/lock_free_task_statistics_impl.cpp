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

#include "libpandabase/taskmanager/task_statistics/lock_free_task_statistics_impl.h"
#include <atomic>

namespace ark::taskmanager {

LockFreeTaskStatisticsImpl::LockFreeTaskStatisticsImpl()
{
    for (const auto &properties : allTaskProperties_) {
        taskInSystemCounterMap_[properties] = 0;
        for (const auto &status : ALL_TASK_STATUSES) {
            taskPropertiesCounterMap_[status][properties] = 0;
        }
    }
}

void LockFreeTaskStatisticsImpl::IncrementCount(TaskStatus status, TaskProperties properties, size_t count)
{
    // Atomic with acq_rel order reason: sync for counter
    taskPropertiesCounterMap_.at(status).at(properties).fetch_add(count, std::memory_order_acq_rel);

    switch (status) {
        case TaskStatus::ADDED:
            // Atomic with acq_rel order reason: sync for counter
            taskInSystemCounterMap_.at(properties).fetch_add(count, std::memory_order_acq_rel);
            break;
        case TaskStatus::EXECUTED:
            // Atomic with acquire order reason: sync for counter
            ASSERT(taskInSystemCounterMap_.at(properties).load(std::memory_order_acquire) >= count);
            // Atomic with acq_rel order reason: sync for counter
            taskInSystemCounterMap_.at(properties).fetch_sub(count, std::memory_order_acq_rel);
            break;
        case TaskStatus::POPPED:
            // Atomic with acquire order reason: sync for counter
            ASSERT(taskInSystemCounterMap_.at(properties).load(std::memory_order_acquire) >= count);
            // Atomic with acq_rel order reason: sync for counter
            taskInSystemCounterMap_.at(properties).fetch_sub(count, std::memory_order_acq_rel);
            break;
        default:
            UNREACHABLE();
    }
}

size_t LockFreeTaskStatisticsImpl::GetCount(TaskStatus status, TaskProperties properties) const
{
    // Atomic with acquire order reason: sync for counter
    return taskPropertiesCounterMap_.at(status).at(properties).load(std::memory_order_acquire);
}

size_t LockFreeTaskStatisticsImpl::GetCountOfTaskInSystem() const
{
    size_t taskInSystemCount = 0;
    for (const auto &properties : allTaskProperties_) {
        taskInSystemCount += GetCountOfTasksInSystemWithTaskProperties(properties);
    }
    return taskInSystemCount;
}

size_t LockFreeTaskStatisticsImpl::GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const
{
    // Atomic with acquire order reason: sync for counter
    return taskInSystemCounterMap_.at(properties).load(std::memory_order_acquire);
}

void LockFreeTaskStatisticsImpl::ResetAllCounters()
{
    for (const auto &properties : allTaskProperties_) {
        ResetCountersWithTaskProperties(properties);
    }
}

void LockFreeTaskStatisticsImpl::ResetCountersWithTaskProperties(TaskProperties properties)
{
    for (const auto &state : ALL_TASK_STATUSES) {
        // Atomic with release order reason: sync for counter
        taskPropertiesCounterMap_.at(state).at(properties).store(0, std::memory_order_release);
    }
}

}  // namespace ark::taskmanager
