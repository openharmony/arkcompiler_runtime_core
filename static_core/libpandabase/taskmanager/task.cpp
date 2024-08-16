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
#include "libpandabase/taskmanager/task_scheduler.h"

namespace ark::taskmanager {

void TaskLifeTimeAggregator::GetAndStoreTimeOfTaskAddingToQueue()
{
    addingToQueueTime_ = time::GetCurrentTimeInMicros(false);
}

void TaskLifeTimeAggregator::GetAndStoreTimeOfTaskExecutionStart()
{
    startExecutionTime_ = time::GetCurrentTimeInMicros(false);
}

void TaskLifeTimeAggregator::GetTimeOfTaskExecutionFinishAndStoreTimeStats(TaskProperties prop)
{
    auto *scheduler = TaskScheduler::GetTaskScheduler();
    ASSERT(scheduler != nullptr);
    auto *taskTimeStats = scheduler->GetTaskTimeStats();
    ASSERT(taskTimeStats != nullptr);
    auto endExecutionTime = time::GetCurrentTimeInMicros(false);
    taskTimeStats->CollectLifeAndExecutionTimes(prop, endExecutionTime - addingToQueueTime_,
                                                endExecutionTime - startExecutionTime_);
}

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
    EventOnStartExecution();
    runner_();
    EventOnEndExecution();
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

void Task::EventOnTaskAdding()
{
    auto *scheduler = TaskScheduler::GetTaskScheduler();
    if UNLIKELY (scheduler == nullptr) {
        return;
    }
    if (scheduler->IsTaskLifetimeStatisticsUsed()) {
        lifeTimeStorage_.GetAndStoreTimeOfTaskAddingToQueue();
    }
}

void Task::EventOnStartExecution()
{
    auto *scheduler = TaskScheduler::GetTaskScheduler();
    if UNLIKELY (scheduler == nullptr) {
        return;
    }
    if (scheduler->IsTaskLifetimeStatisticsUsed()) {
        lifeTimeStorage_.GetAndStoreTimeOfTaskExecutionStart();
    }
}

void Task::EventOnEndExecution()
{
    auto *scheduler = TaskScheduler::GetTaskScheduler();
    if UNLIKELY (scheduler == nullptr) {
        return;
    }
    if (scheduler->IsTaskLifetimeStatisticsUsed()) {
        lifeTimeStorage_.GetTimeOfTaskExecutionFinishAndStoreTimeStats(properties_);
    }
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
