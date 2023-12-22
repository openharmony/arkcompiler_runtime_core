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

namespace panda::taskmanager {

SimpleTaskStatisticsImpl::SimpleTaskStatisticsImpl()
{
    for (const auto &status : ALL_TASK_STATUSES) {
        per_status_lock_[status];  // Here we use implicitly construction of mutex
        for (const auto &properties : all_task_properties_) {
            task_properties_counter_map_[status][properties] = 0;
        }
    }
}

void SimpleTaskStatisticsImpl::IncrementCount(TaskStatus status, TaskProperties properties, size_t count)
{
    os::memory::LockHolder lock_holder(per_status_lock_.at(status));
    task_properties_counter_map_.at(status)[properties] += count;
}

size_t SimpleTaskStatisticsImpl::GetCount(TaskStatus status, TaskProperties properties) const
{
    os::memory::LockHolder lock_holder(per_status_lock_.at(status));
    return task_properties_counter_map_.at(status).at(properties);
}

size_t SimpleTaskStatisticsImpl::GetCountOfTaskInSystem() const
{
    os::memory::LockHolder added_lock_holder(per_status_lock_.at(TaskStatus::ADDED));
    os::memory::LockHolder executed_lock_holder(per_status_lock_.at(TaskStatus::EXECUTED));
    os::memory::LockHolder popped_lock_holder(per_status_lock_.at(TaskStatus::POPPED));

    size_t in_system_tasks_count = 0;
    for (const auto &properties : all_task_properties_) {
        size_t added_task_count_val = task_properties_counter_map_.at(TaskStatus::ADDED).at(properties);
        size_t executed_task_count_val = task_properties_counter_map_.at(TaskStatus::EXECUTED).at(properties);
        size_t popped_task_count_val = task_properties_counter_map_.at(TaskStatus::POPPED).at(properties);

        ASSERT(added_task_count_val >= executed_task_count_val + popped_task_count_val);
        in_system_tasks_count += added_task_count_val - executed_task_count_val - popped_task_count_val;
    }
    return in_system_tasks_count;
}

size_t SimpleTaskStatisticsImpl::GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const
{
    os::memory::LockHolder added_lock_holder(per_status_lock_.at(TaskStatus::ADDED));
    os::memory::LockHolder executed_lock_holder(per_status_lock_.at(TaskStatus::EXECUTED));
    os::memory::LockHolder popped_lock_holder(per_status_lock_.at(TaskStatus::POPPED));

    size_t added_task_count_val = task_properties_counter_map_.at(TaskStatus::ADDED).at(properties);
    size_t executed_task_count_val = task_properties_counter_map_.at(TaskStatus::EXECUTED).at(properties);
    size_t popped_task_count_val = task_properties_counter_map_.at(TaskStatus::POPPED).at(properties);

    ASSERT(added_task_count_val >= executed_task_count_val + popped_task_count_val);
    return added_task_count_val - executed_task_count_val - popped_task_count_val;
}

void SimpleTaskStatisticsImpl::ResetAllCounters()
{
    for (const auto &properties : all_task_properties_) {
        ResetCountersWithTaskProperties(properties);
    }
}

void SimpleTaskStatisticsImpl::ResetCountersWithTaskProperties(TaskProperties properties)
{
    // Getting locks for every state counter with specified properties
    std::unordered_map<TaskStatus, os::memory::LockHolder<os::memory::Mutex>> lock_holder_map;
    for (const auto &status : ALL_TASK_STATUSES) {
        lock_holder_map.emplace(status, per_status_lock_.at(status));
    }
    for (const auto &status : ALL_TASK_STATUSES) {
        task_properties_counter_map_[status][properties] = 0;
    }
}

}  // namespace panda::taskmanager
