/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "libpandabase/utils/time.h"
#include "libpandabase/taskmanager/schedulable_task_queue_interface.h"

namespace ark::taskmanager::internal {

void TaskLifeTimeAggregator::GetAndStoreTimeOfTaskAddingToQueue()
{
    addingToQueueTime_ = time::GetCurrentTimeInMicros(false);
}

void TaskLifeTimeAggregator::GetAndStoreTimeOfTaskExecutionStart()
{
    startExecutionTime_ = time::GetCurrentTimeInMicros(false);
}

void TaskLifeTimeAggregator::GetTimeOfTaskExecutionFinishAndStoreTimeStats(TaskQueueInterface *queue)
{
    auto *taskTimeStats = static_cast<SchedulableTaskQueueInterface *>(queue)->GetTaskTimeStats();
    ASSERT(taskTimeStats != nullptr);
    auto endExecutionTime = time::GetCurrentTimeInMicros(false);
    taskTimeStats->CollectLifeAndExecutionTimes(queue->GetQueueId(), endExecutionTime - addingToQueueTime_,
                                                endExecutionTime - startExecutionTime_);
}

/* static */
Task::Ptr Task::Create(RunnerCallback runner, TaskQueueInterface *queue, OnDestructionCallback callback)
{
    // Task can be created only by TaskQueue so callback should be controled from outside.
    ASSERT(callback != nullptr);
    // CC-OFFNXT(G.RES.09-CPP): constructor of Task class is private
    return std::unique_ptr<Task>(new Task(std::move(runner), queue, callback));
}

Task::~Task()
{
    onDestructionCallback_(parentQueue_);
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
    runner_ = nullptr;
}

bool Task::IsInvalid() const
{
    return runner_ == nullptr;
}

void Task::EventOnTaskAdding()
{
    if (static_cast<SchedulableTaskQueueInterface *>(parentQueue_)->GetTaskTimeStats() != nullptr) {
        lifeTimeStorage_.GetAndStoreTimeOfTaskAddingToQueue();
    }
}

void Task::EventOnStartExecution()
{
    if (static_cast<SchedulableTaskQueueInterface *>(parentQueue_)->GetTaskTimeStats() != nullptr) {
        lifeTimeStorage_.GetAndStoreTimeOfTaskExecutionStart();
    }
}

void Task::EventOnEndExecution()
{
    if (static_cast<SchedulableTaskQueueInterface *>(parentQueue_)->GetTaskTimeStats() != nullptr) {
        lifeTimeStorage_.GetTimeOfTaskExecutionFinishAndStoreTimeStats(parentQueue_);
    }
}

}  // namespace ark::taskmanager::internal
