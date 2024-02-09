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

namespace ark::taskmanager {

/// This version of TaskStatistics uses a separate lock for each type of task.
class FineGrainedTaskStatisticsImpl : public TaskStatistics {
    class GuardedCounter {
    public:
        explicit GuardedCounter(size_t startValue = 0) : counter_(startValue) {}

        GuardedCounter &operator+=(size_t value)
        {
            os::memory::LockHolder lockHolder(lock_);
            counter_ += value;
            return *this;
        }

        friend bool operator==(const GuardedCounter &lv, const GuardedCounter &rv)
        {
            os::memory::LockHolder lvLockHolder(lv.lock_);
            os::memory::LockHolder rvLockHolder(rv.lock_);
            return lv.counter_ == rv.counter_;
        }

        void NoGuardedSetValue(size_t value)
        {
            counter_ = value;
        }

        os::memory::Mutex &GetMutex()
        {
            return lock_;
        }

        void SetValue(size_t value)
        {
            os::memory::LockHolder lockHolder(lock_);
            counter_ = value;
        }

        size_t GetValue() const
        {
            os::memory::LockHolder lockHolder(lock_);
            return counter_;
        }

        friend size_t CalcCountOfTasksInSystem(const GuardedCounter &addedCount, const GuardedCounter &executedCount,
                                               const GuardedCounter &poppedCount)
        {
            os::memory::LockHolder addedLockHolder(addedCount.lock_);
            os::memory::LockHolder executedLockHolder(executedCount.lock_);
            os::memory::LockHolder poppedLockHolder(poppedCount.lock_);
            ASSERT(addedCount.counter_ >= executedCount.counter_ + poppedCount.counter_);
            return addedCount.counter_ - executedCount.counter_ - poppedCount.counter_;
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
    std::unordered_map<TaskStatus, TaskPropertiesGuardedCounter> taskPropertiesCounterMap_;
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_FINE_GRAINED_TASK_STATISTICS_IMPL_H
