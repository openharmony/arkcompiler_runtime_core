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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_SIMPLE_TASK_STATISTICS_IMPL_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_SIMPLE_TASK_STATISTICS_IMPL_H

#include "libpandabase/os/mutex.h"
#include "libpandabase/taskmanager/task_statistics/task_statistics.h"
#include <unordered_map>

namespace ark::taskmanager {

/// SimpleTaskStatisticsImpl use lock for every used status to protect counters
class SimpleTaskStatisticsImpl : public TaskStatistics {
    using TaskPropertiesCounterMap = std::unordered_map<TaskProperties, size_t, TaskProperties::Hash>;

public:
    SimpleTaskStatisticsImpl();
    ~SimpleTaskStatisticsImpl() override = default;

    NO_COPY_SEMANTIC(SimpleTaskStatisticsImpl);
    NO_MOVE_SEMANTIC(SimpleTaskStatisticsImpl);

    void IncrementCount(TaskStatus status, TaskProperties properties, size_t count) override;
    size_t GetCount(TaskStatus status, TaskProperties properties) const override;

    size_t GetCountOfTaskInSystem() const override;
    size_t GetCountOfTasksInSystemWithTaskProperties(TaskProperties properties) const override;

    void ResetAllCounters() override;
    void ResetCountersWithTaskProperties(TaskProperties properties) override;

private:
    mutable std::unordered_map<TaskStatus, os::memory::Mutex> perStatusLock_;
    std::unordered_map<TaskStatus, TaskPropertiesCounterMap> taskPropertiesCounterMap_;
};

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_STATISTICS_SIMPLE_TASK_STATISTICS_IMPL_H
