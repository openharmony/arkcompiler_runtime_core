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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_FINE_GRAINED_TASK_STATISTICS_IMPL_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_FINE_GRAINED_TASK_STATISTICS_IMPL_H

#include <atomic>
#include <unordered_map>
#include "libpandabase/os/mutex.h"
#include "libpandabase/taskmanager/task_statistics/task_statistics.h"

namespace panda::taskmanager {

/// This version of TaskStatistics uses a separate lock for each type of task.
class FineGrainedTaskStatisticsImpl : public TaskStatistics {
    class GuardedCounter {
    public:
        explicit GuardedCounter(size_t start_value = 0) : counter_(start_value) {}

        GuardedCounter &operator+=(size_t value)
        {
            os::memory::LockHolder lock_holder(lock_);
            counter_ += value;
            return *this;
        }

        friend bool operator==(const GuardedCounter &lv, const GuardedCounter &rv)
        {
            os::memory::LockHolder lv_lock_holder(lv.lock_);
            os::memory::LockHolder rv_lock_holder(rv.lock_);
            return lv.counter_ == rv.counter_;
        }

        void SetValue(size_t value)
        {
            os::memory::LockHolder lock_holder(lock_);
            counter_ = value;
        }

        size_t GetValue() const
        {
            os::memory::LockHolder lock_holder(lock_);
            return counter_;
        }

        friend size_t CalcCountOfTasksInSystem(const GuardedCounter &added_count, const GuardedCounter &executed_count,
                                               const GuardedCounter &popped_count)
        {
            os::memory::LockHolder added_lock_holder(added_count.lock_);
            os::memory::LockHolder executed_lock_holder(executed_count.lock_);
            os::memory::LockHolder popped_lock_holder(popped_count.lock_);
            ASSERT(added_count.counter_ >= executed_count.counter_ + popped_count.counter_);
            return added_count.counter_ - executed_count.counter_ - popped_count.counter_;
        }

    private:
        mutable os::memory::Mutex lock_;
        size_t counter_;
    };

    using TaskPropertiesGuardedCounter = std::unordered_map<TaskProperties, GuardedCounter, TaskProperties::Hash>;

public:
    FineGrainedTaskStatisticsImpl();
    ~FineGrainedTaskStatisticsImpl() override = default;

    NO_COPY_SEMANTIC(FineGrainedTaskStatisticsImpl);
    NO_MOVE_SEMANTIC(FineGrainedTaskStatisticsImpl);

    void IncrementCount(TaskStatus status, TaskProperties properties, size_t count) override;
    size_t GetCount(TaskStatus status, TaskProperties properties) const override;

    size_t GetCountOfTaskInSystem() const override;
    size_t GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const override;

    void ResetAllCounters() override;
    void ResetCountersWithTaskProperties(TaskProperties properties) override;

private:
    std::unordered_map<TaskStatus, TaskPropertiesGuardedCounter> task_properties_counter_map_;
};

}  // namespace panda::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_FINE_GRAINED_TASK_STATISTICS_IMPL_H
