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

#include "libpandabase/taskmanager/task_scheduler.h"
#include <chrono>
#include <gtest/gtest.h>
#include <cmath>

namespace ark::taskmanager {

class TaskManagerGCCornerCaseTest : public ::testing::Test {
public:
    static constexpr size_t COUNT_OF_WORKERS = 4;

    static constexpr size_t REPETITION_COUNT = 1000;

    static constexpr size_t FIRST_GC_STAGE_TASK_COUNT = 500;
    static constexpr size_t SECOND_GC_STAGE_TASK_COUNT = 50;
    static constexpr size_t THIRD_GC_STAGE_TASK_COUNT = 30;
    static constexpr size_t ADDITIONAL_GC_STAGE_TASK_COUNT = 50;

    static constexpr std::chrono::nanoseconds FIRST_GC_STAGE_TASK_DURATION {1500};
    static constexpr std::chrono::microseconds SECOND_GC_STAGE_TASK_DURATION {1};
    static constexpr std::chrono::microseconds THIRD_GC_STAGE_TASK_DURATION {1};
    static constexpr std::chrono::nanoseconds ADDITIONAL_GC_STAGE_TASK_DURATION {1500};

    static constexpr size_t CALCULATION_BASE = 0x777777;

    static constexpr TaskProperties FOREGROUND_TASK = {TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::FOREGROUND};
    static constexpr TaskProperties BACKGROUND_TASK = {TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND};

    TaskManagerGCCornerCaseTest() : counter_(0) {}
    ~TaskManagerGCCornerCaseTest() override = default;

    NO_COPY_SEMANTIC(TaskManagerGCCornerCaseTest);
    NO_MOVE_SEMANTIC(TaskManagerGCCornerCaseTest);

    void SetQueue(TaskQueueInterface *queue)
    {
        queue_ = queue;
    }

    TaskQueueInterface *GetQueue()
    {
        return queue_;
    }

    void GetAndRunForegroundTasks()
    {
        while (true) {
            size_t count = TaskScheduler::GetTaskScheduler()->HelpWorkersWithTasks(FOREGROUND_TASK);
            if (count == 0) {
                break;
            }
        }
    }

    void BigGcTask(bool needAdditionalTasks)
    {
        LoadForegroundTasks(FIRST_GC_STAGE_TASK_COUNT, [this]() { LoadRunner(FIRST_GC_STAGE_TASK_DURATION); });
        GetAndRunForegroundTasks();
        TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(FOREGROUND_TASK);

        LoadForegroundTasks(SECOND_GC_STAGE_TASK_COUNT, [this]() { LoadRunner(SECOND_GC_STAGE_TASK_DURATION); });
        GetAndRunForegroundTasks();
        TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(FOREGROUND_TASK);

        LoadForegroundTasks(THIRD_GC_STAGE_TASK_COUNT, [this]() { LoadRunner(THIRD_GC_STAGE_TASK_DURATION); });
        GetAndRunForegroundTasks();
        TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(FOREGROUND_TASK);

        if (needAdditionalTasks) {
            LoadForegroundTasks(ADDITIONAL_GC_STAGE_TASK_COUNT,
                                [this]() { LoadRunner(ADDITIONAL_GC_STAGE_TASK_DURATION); });
            GetAndRunForegroundTasks();
            TaskScheduler::GetTaskScheduler()->WaitForFinishAllTasksWithProperties(FOREGROUND_TASK);
        }
    }

    void LoadForegroundTasks(size_t count, const std::function<void()> &runner)
    {
        for (size_t i = 0; i < count; i++) {
            queue_->AddTask(Task::Create(FOREGROUND_TASK, runner));
        }
    }

    void LoadRunner(std::chrono::nanoseconds time)
    {
        const auto startTime = std::chrono::system_clock::now();
        do {
            // just calculations for load
            counter_ += std::log(CALCULATION_BASE * time.count());
        } while ((std::chrono::system_clock::now() - startTime) < time);
    }

    void SetUp() override
    {
        TaskScheduler::Create(COUNT_OF_WORKERS, TaskTimeStatsType::NO_STATISTICS);
    }

    void TearDown() override
    {
        TaskScheduler::Destroy();
    }

private:
    TaskQueueInterface *queue_ {nullptr};
    std::atomic_size_t counter_;
};

TEST_F(TaskManagerGCCornerCaseTest, TriggerGcInMainThread)
{
    auto *tm = TaskScheduler::GetTaskScheduler();
    auto *queue = tm->CreateAndRegisterTaskQueue(TaskType::GC, VMType::STATIC_VM, TaskQueueInterface::DEFAULT_PRIORITY);
    SetQueue(queue);
    tm->Initialize();
    for (size_t i = 0; i < REPETITION_COUNT; i++) {
        BigGcTask(false);
    }
    tm->Finalize();
    tm->UnregisterAndDestroyTaskQueue(queue);
}

TEST_F(TaskManagerGCCornerCaseTest, TriggerGcInTaskManager)
{
    auto *tm = TaskScheduler::GetTaskScheduler();
    auto *queue = tm->CreateAndRegisterTaskQueue(TaskType::GC, VMType::STATIC_VM, TaskQueueInterface::DEFAULT_PRIORITY);
    SetQueue(queue);
    tm->Initialize();
    for (size_t i = 0; i < REPETITION_COUNT; i++) {
        queue->AddTask(Task::Create(BACKGROUND_TASK, [this]() { BigGcTask(true); }));
        tm->WaitForFinishAllTasksWithProperties(BACKGROUND_TASK);
    }
    tm->Finalize();
    tm->UnregisterAndDestroyTaskQueue(queue);
}

}  // namespace ark::taskmanager
