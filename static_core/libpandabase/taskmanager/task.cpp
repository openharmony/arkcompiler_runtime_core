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

#include "libpandabase/taskmanager/task.h"

namespace ark::taskmanager {

/* static */
Task Task::Create(TaskProperties properties, RunnerCallback runner)
{
    Task task(properties, std::move(runner));
    return task;
}

TaskProperties Task::GetTaskProperties() const
{
    return properties_;
}

void Task::RunTask()
{
    ASSERT(!IsInvalid());
    lifeTimeScope_.IndicateTaskExecutionStart();
    runner_();
    lifeTimeScope_.IndicateTaskExecutionEnd();
}

void Task::MakeInvalid()
{
    properties_ = INVALID_TASK_PROPERTIES;
    runner_ = nullptr;
}

bool Task::IsInvalid() const
{
    return properties_ == INVALID_TASK_PROPERTIES;
}

std::ostream &operator<<(std::ostream &os, TaskType type)
{
    switch (type) {
        case TaskType::GC:
            os << "TaskType::GC";
            break;
        case TaskType::JIT:
            os << "TaskType::JIT";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, VMType type)
{
    switch (type) {
        case VMType::DYNAMIC_VM:
            os << "VMType::DYNAMIC_VM";
            break;
        case VMType::STATIC_VM:
            os << "VMType::STATIC_VM";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, TaskExecutionMode mode)
{
    switch (mode) {
        case TaskExecutionMode::FOREGROUND:
            os << "TaskExecutionMode::FOREGROUND";
            break;
        case TaskExecutionMode::BACKGROUND:
            os << "TaskExecutionMode::BACKGROUND";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, TaskProperties prop)
{
    os << "{" << prop.GetTaskType() << "," << prop.GetVMType() << "," << prop.GetTaskExecutionMode() << "}";
    return os;
}

}  // namespace ark::taskmanager
