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

#include "taskmanager/task_statistics/task_statistics.h"

namespace panda::taskmanager {

TaskStatistics::TaskStatistics()
{
    for (TaskType task_type : ALL_TASK_TYPES) {
        for (VMType vm_type : ALL_VM_TYPES) {
            for (TaskExecutionMode execution_mode : ALL_TASK_EXECUTION_MODES) {
                all_task_properties_.emplace_back(task_type, vm_type, execution_mode);
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
        case TaskStatisticsImplType::LOCK_FREE:
            os << "TaskStatisticsImplType::LOCK_FREE";
            break;
        case TaskStatisticsImplType::SIMPLE:
            os << "TaskStatisticsImplType::SIMPLE";
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
    UNREACHABLE();
}

}  // namespace panda::taskmanager
