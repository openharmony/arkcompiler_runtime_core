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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_LOCK_FREE_TASK_STATISTICS_IMPL_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_LOCK_FREE_TASK_STATISTICS_IMPL_H

#include "libpandabase/taskmanager/task_statistics/task_statistics.h"
#include <unordered_map>
#include <atomic>

namespace ark::taskmanager {

class LockFreeTaskStatisticsImpl : public TaskStatistics {
    using TaskPropertiesAtomicCounterMap = std::unordered_map<TaskProperties, std::atomic_size_t, TaskProperties::Hash>;

public:
    LockFreeTaskStatisticsImpl();
    ~LockFreeTaskStatisticsImpl() override = default;

    NO_COPY_SEMANTIC(LockFreeTaskStatisticsImpl);
    NO_MOVE_SEMANTIC(LockFreeTaskStatisticsImpl);

    void IncrementCount(TaskStatus status, TaskProperties properties, size_t count) override;
    size_t GetCount(TaskStatus status, TaskProperties properties) const override;

    size_t GetCountOfTaskInSystem() const override;
    size_t GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const override;

    void ResetAllCounters() override;
    void ResetCountersWithTaskProperties(TaskProperties properties) override;

private:
    TaskPropertiesAtomicCounterMap taskInSystemCounterMap_;
    std::unordered_map<TaskStatus, TaskPropertiesAtomicCounterMap> taskPropertiesCounterMap_;
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_LOCK_FREE_TASK_STATISTICS_IMPL_H
