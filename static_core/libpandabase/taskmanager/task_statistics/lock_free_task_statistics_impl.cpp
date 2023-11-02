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

namespace panda::taskmanager {

LockFreeTaskStatisticsImpl::LockFreeTaskStatisticsImpl()
{
    for (const auto &status : ALL_TASK_STATES) {
        for (const auto &properties : all_task_properties_) {
            // Atomic with release order reason: threads should see correct initialization
            status_task_properties_counter_map_[status][properties].store(0, std::memory_order_release);
        }
        // Atomic with release order reason: threads should see correct initialization
        status_counter_map_[status].store(0, std::memory_order_release);
    }
}

void LockFreeTaskStatisticsImpl::IncrementCount(TaskStatus status, TaskProperties properties, size_t count)
{
    // Atomic with relaxed order reason: synchronization of count increment.
    status_task_properties_counter_map_.at(status)[properties].fetch_add(count, std::memory_order_relaxed);
    // Atomic with relaxed order reason: synchronization of count increment.
    status_counter_map_.at(status).fetch_add(count, std::memory_order_relaxed);
}

size_t LockFreeTaskStatisticsImpl::GetCount(TaskStatus status, TaskProperties properties) const
{
    // Atomic with acquire order reason: load synchronization
    return status_task_properties_counter_map_.at(status).at(properties).load(std::memory_order_acquire);
}

size_t LockFreeTaskStatisticsImpl::GetCountOfTaskInSystem() const
{
    size_t added_task_count_val = 0;
    size_t executed_task_count_val = 0;
    size_t popped_task_count_val = 0;

    const auto &added_task_counter = status_counter_map_.at(TaskStatus::ADDED);
    const auto &executed_task_counter = status_counter_map_.at(TaskStatus::EXECUTED);
    const auto &popped_task_counter = status_counter_map_.at(TaskStatus::POPPED);

    do {
        // Atomic with acquire order reason: load synchronization
        added_task_count_val = added_task_counter.load(std::memory_order_acquire);
        // Atomic with acquire order reason: load synchronization
        executed_task_count_val = executed_task_counter.load(std::memory_order_acquire);
        // Atomic with acquire order reason: load synchronization
        popped_task_count_val = popped_task_counter.load(std::memory_order_acquire);
        // Atomic with acquire order reason: load synchronization
    } while (added_task_counter.load(std::memory_order_acquire) != added_task_count_val ||
             // Atomic with acquire order reason: load synchronization
             executed_task_counter.load(std::memory_order_acquire) != executed_task_count_val ||
             // Atomic with acquire order reason: load synchronization
             popped_task_counter.load(std::memory_order_acquire) != popped_task_count_val);
    ASSERT(added_task_count_val >= executed_task_count_val + popped_task_count_val);
    return added_task_count_val - executed_task_count_val - popped_task_count_val;
}

size_t LockFreeTaskStatisticsImpl::GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const
{
    size_t added_task_count_val = 0;
    size_t executed_task_count_val = 0;
    size_t popped_task_count_val = 0;

    const auto &added_task_counter = status_task_properties_counter_map_.at(TaskStatus::ADDED).at(properties);
    const auto &executed_task_counter = status_task_properties_counter_map_.at(TaskStatus::EXECUTED).at(properties);
    const auto &popped_task_counter = status_task_properties_counter_map_.at(TaskStatus::POPPED).at(properties);

    do {
        // Atomic with acquire order reason: load synchronization
        added_task_count_val = added_task_counter.load(std::memory_order_acquire);
        // Atomic with acquire order reason: load synchronization
        executed_task_count_val = executed_task_counter.load(std::memory_order_acquire);
        // Atomic with acquire order reason: load synchronization
        popped_task_count_val = popped_task_counter.load(std::memory_order_acquire);
        // Atomic with acquire order reason: load synchronization
    } while (added_task_counter.load(std::memory_order_acquire) != added_task_count_val ||
             // Atomic with acquire order reason: load synchronization
             executed_task_counter.load(std::memory_order_acquire) != executed_task_count_val ||
             // Atomic with acquire order reason: load synchronization
             popped_task_counter.load(std::memory_order_acquire) != popped_task_count_val);
    ASSERT(added_task_count_val >= executed_task_count_val + popped_task_count_val);
    return added_task_count_val - executed_task_count_val - popped_task_count_val;
}

void LockFreeTaskStatisticsImpl::ResetAllCounters()
{
    for (const auto &status : ALL_TASK_STATES) {
        for (const auto &properties : all_task_properties_) {
            // Atomic with release order reason: threads should see correct initialization
            status_task_properties_counter_map_[status][properties].store(0, std::memory_order_release);
        }
        // Atomic with release order reason: threads should see correct initialization
        status_counter_map_[status].store(0, std::memory_order_release);
    }
}

void LockFreeTaskStatisticsImpl::ResetCountersWithTaskProperties(TaskProperties properties)
{
    for (const auto &status : ALL_TASK_STATES) {
        // Atomic with acquire order reason: load synchronization
        size_t status_counter = status_task_properties_counter_map_[status][properties].load(std::memory_order_acquire);
        // Atomic with release order reason: threads should see correct initialization
        status_task_properties_counter_map_[status][properties].store(0, std::memory_order_release);
        // Atomic with acq_rel order reason: synchronization of count decrement.
        status_counter_map_[status].fetch_sub(status_counter, std::memory_order_acq_rel);
    }
}

}  // namespace panda::taskmanager
