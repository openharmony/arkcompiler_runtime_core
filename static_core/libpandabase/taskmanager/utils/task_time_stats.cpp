/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "libpandabase/taskmanager/utils/task_time_stats.h"
#include "libpandabase/utils/time.h"

#include <sstream>
#include <numeric>
#include <cmath>
#include <ostream>

namespace ark::taskmanager {

TaskTimeStatsType StringToTaskTimeStats(std::string_view str)
{
    if (str == "no-task-stats") {
        return TaskTimeStatsType::NO_STATISTICS;
    }
    if (str == "light-task-stats") {
        return TaskTimeStatsType::LIGHT_STATISTICS;
    }
    UNREACHABLE();
}

std::ostream &operator<<(std::ostream &os, TaskTimeStatsType type)
{
    switch (type) {
        case TaskTimeStatsType::NO_STATISTICS:
            os << "TaskTimeStatsType::NO_STATISTICS";
            break;
        case TaskTimeStatsType::LIGHT_STATISTICS:
            os << "TaskTimeStatsType::LIGHT_STATISTICS";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

thread_local TaskTimeStatsBase::ContainerId TaskTimeStatsBase::containerId_ = TaskTimeStatsBase::DEFAULT_CONTAINER_ID;

void TaskTimeStatsBase::RegisterWorkerThread()
{
    // Atomic with acq_rel order reason: fetch_add operation should be sync with calls from other threads
    size_t registerNumber = countOfRegisteredWorkers_.fetch_add(1, std::memory_order_acq_rel);
    containerId_ = registerNumber + 1U;
}

namespace internal {

LightTaskTimeTimeStats::LightTaskTimeTimeStats(size_t countOfWorkers)
    : statisticsContainerPerThread_(countOfWorkers + 1U)
{
    for (auto &statisticsContainer : statisticsContainerPerThread_) {
        for (size_t i = 0; i < STATISTICS_CONTAINER_SIZE; i++) {
            statisticsContainer[i] = MeanTimeStats();
        }
    }
}

void LightTaskTimeTimeStats::CollectLifeAndExecutionTimes(TaskProperties prop, uint64_t lifeTime,
                                                          uint64_t executionTime)
{
    auto propHash = TaskProperties::Hash()(prop);
    auto &meanTimeStats = statisticsContainerPerThread_[containerId_][propHash];

    meanTimeStats.sumExecutionTime += executionTime;
    meanTimeStats.sumLifeTime += lifeTime;

    meanTimeStats.maxExecutionTime = std::max(executionTime, meanTimeStats.maxExecutionTime);
    meanTimeStats.maxLifeTime = std::max(lifeTime, meanTimeStats.maxLifeTime);

    ASSERT(meanTimeStats.sumLifeTime >= meanTimeStats.sumExecutionTime);

    meanTimeStats.countOfTasks++;
}

std::vector<std::string> LightTaskTimeTimeStats::GetTaskStatistics()
{
    std::vector<std::string> resultData;
    for (auto prop : GetAllTaskProperties()) {
        if (GetCountOfTasksWithProperties(prop) == 0) {
            continue;
        }
        resultData.push_back(GetStatisticsForProperties(prop));
    }
    return resultData;
}

std::vector<TaskProperties> LightTaskTimeTimeStats::GetAllTaskProperties()
{
    std::vector<TaskProperties> allTaskProperties;
    for (TaskType taskType : ALL_TASK_TYPES) {
        for (VMType vmType : ALL_VM_TYPES) {
            for (TaskExecutionMode executionMode : ALL_TASK_EXECUTION_MODES) {
                allTaskProperties.emplace_back(taskType, vmType, executionMode);
            }
        }
    }
    return allTaskProperties;
}

std::string LightTaskTimeTimeStats::GetStatisticsForProperties(TaskProperties prop)
{
    std::stringstream stream;
    auto propHash = TaskProperties::Hash()(prop);
    uint64_t sumLifeTime = 0;
    uint64_t sumExecutionTime = 0;
    uint64_t maxLifeTime = 0;
    uint64_t maxExecutionTime = 0;
    size_t sumTaskCount = 0;
    for (const auto &statisticsContainer : statisticsContainerPerThread_) {
        const auto &meanStatistics = statisticsContainer[propHash];
        sumLifeTime += meanStatistics.sumLifeTime;
        sumExecutionTime += meanStatistics.sumExecutionTime;

        maxLifeTime = std::max(meanStatistics.maxLifeTime, maxLifeTime);
        maxExecutionTime = std::max(meanStatistics.maxExecutionTime, maxExecutionTime);

        sumTaskCount += meanStatistics.countOfTasks;
    }
    auto meanLifeTime = static_cast<double>(sumLifeTime) / sumTaskCount;
    auto meanExecutionTime = static_cast<double>(sumExecutionTime) / sumTaskCount;
    stream << "Task " << prop << ": mean life time: " << meanLifeTime << "us; max life time: " << maxLifeTime
           << "us; mean execution time: " << meanExecutionTime << "us; "
           << "max execution time: " << maxExecutionTime << "us; count of tasks: " << sumTaskCount << ";";
    return stream.str();
}

size_t LightTaskTimeTimeStats::GetCountOfTasksWithProperties(TaskProperties prop)
{
    auto propHash = TaskProperties::Hash()(prop);
    size_t sumTaskCount = 0;
    for (const auto &statisticsContainer : statisticsContainerPerThread_) {
        const auto &meanStatistics = statisticsContainer[propHash];
        sumTaskCount += meanStatistics.countOfTasks;
    }
    return sumTaskCount;
}

}  // namespace internal

}  // namespace ark::taskmanager