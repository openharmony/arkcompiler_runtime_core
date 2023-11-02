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

namespace panda::taskmanager {

FineGrainedTaskStatisticsImpl::FineGrainedTaskStatisticsImpl()
{
    for (const auto &status : ALL_TASK_STATES) {
        for (const auto &properties : all_task_properties_) {
            task_properties_counter_map_[status].emplace(properties, 0);
        }
    }
}

void FineGrainedTaskStatisticsImpl::IncrementCount(TaskStatus status, TaskProperties properties, size_t count)
{
    task_properties_counter_map_.at(status)[properties] += count;
}

size_t FineGrainedTaskStatisticsImpl::GetCount(TaskStatus status, TaskProperties properties) const
{
    return task_properties_counter_map_.at(status).at(properties).GetValue();
}

size_t FineGrainedTaskStatisticsImpl::GetCountOfTaskInSystem() const
{
    size_t in_system_tasks_count = 0;
    for (const auto &properties : all_task_properties_) {
        in_system_tasks_count +=
            CalcCountOfTasksInSystem(task_properties_counter_map_.at(TaskStatus::ADDED).at(properties),
                                     task_properties_counter_map_.at(TaskStatus::EXECUTED).at(properties),
                                     task_properties_counter_map_.at(TaskStatus::POPPED).at(properties));
    }
    return in_system_tasks_count;
}

size_t FineGrainedTaskStatisticsImpl::GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const
{
    return CalcCountOfTasksInSystem(task_properties_counter_map_.at(TaskStatus::ADDED).at(properties),
                                    task_properties_counter_map_.at(TaskStatus::EXECUTED).at(properties),
                                    task_properties_counter_map_.at(TaskStatus::POPPED).at(properties));
}

void FineGrainedTaskStatisticsImpl::ResetAllCounters()
{
    for (const auto &status : ALL_TASK_STATES) {
        for (const auto &properties : all_task_properties_) {
            task_properties_counter_map_[status][properties].SetValue(0);
        }
    }
}

void FineGrainedTaskStatisticsImpl::ResetCountersWithTaskProperties(TaskProperties properties)
{
    for (const auto &status : ALL_TASK_STATES) {
        task_properties_counter_map_[status][properties].SetValue(0);
    }
}

}  // namespace panda::taskmanager
