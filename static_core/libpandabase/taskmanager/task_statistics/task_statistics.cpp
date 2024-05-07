/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "taskmanager/task_statistics/task_statistics.h"

namespace ark::taskmanager {

TaskStatistics::TaskStatistics()
{
    for (TaskType taskType : ALL_TASK_TYPES) {
        for (VMType vmType : ALL_VM_TYPES) {
            for (TaskExecutionMode executionMode : ALL_TASK_EXECUTION_MODES) {
                allTaskProperties_.emplace_back(taskType, vmType, executionMode);
            }
        }
    }
}

std::ostream &operator<<(std::ostream &os, TaskStatisticsImplType type)
{
    switch (type) {
        case TaskStatisticsImplType::FINE_GRAINED:
            os << "TaskStatisticsImplType::FINE_GRAINED";
            break;
        case TaskStatisticsImplType::SIMPLE:
            os << "TaskStatisticsImplType::SIMPLE";
            break;
        case TaskStatisticsImplType::LOCK_FREE:
            os << "TaskStatisticsImplType::LOCK_FREE";
            break;
        case TaskStatisticsImplType::NO_STAT:
            os << "TaskStatisticsImplType::NO_STAT";
            break;
        default:
            UNREACHABLE();
    }
    return os;
}

TaskStatisticsImplType TaskStatisticsImplTypeFromString(std::string_view string)
{
    if (string == "simple") {
        return TaskStatisticsImplType::SIMPLE;
    }
    if (string == "fine-grained") {
        return TaskStatisticsImplType::FINE_GRAINED;
    }
    if (string == "lock-free") {
        return TaskStatisticsImplType::LOCK_FREE;
    }
    if (string == "no-stat") {
        return TaskStatisticsImplType::NO_STAT;
    }
    UNREACHABLE();
}

}  // namespace ark::taskmanager
