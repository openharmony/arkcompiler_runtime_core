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

#include "os/thread.h"
#include "utils/logger.h"
#include "utils/string_helpers.h"
#include "libpandabase/taskmanager/task_scheduler.h"
#include "libpandabase/test_utilities.h"

#include <cstdio>

#include <fstream>
#include <regex>
#include <streambuf>

#include <gtest/gtest.h>

namespace ark::taskmanager {

TEST(TaskSchedulerMetrics, InfoLoggingTest)
{
    constexpr size_t THREADS_COUNT = 4;
    auto *tm = TaskScheduler::Create(THREADS_COUNT, TaskTimeStatsType::LIGHT_STATISTICS);
    ASSERT_TRUE(tm->IsTaskLifetimeStatisticsUsed());
    auto *queue = tm->CreateAndRegisterTaskQueue(TaskType::GC, VMType::STATIC_VM);
    TaskProperties prop(TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::FOREGROUND);
    tm->Initialize();

    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);

    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    queue->AddTask(Task::Create(prop, []() {}));
    queue->AddTask(Task::Create(prop, []() {}));

    tm->Finalize();

    std::string err = testing::internal::GetCapturedStderr();
    auto correctMessage = std::string(
        "\\[TID [0-9a-f]{6}\\] I/task_manager: Task "
        "\\{TaskType::GC,VMType::STATIC_VM,TaskExecutionMode::FOREGROUND\\}: "
        "mean life time: [\\.0-9]+us; max life time: [\\.0-9]+us; mean execution time: "
        "[\\.0-9]+us; max execution time: [\\.0-9]+us; count of tasks: [0-9]+;\n");
    std::regex e(correctMessage);
    EXPECT_TRUE(std::regex_match(err, e)) << "real message: " << err << "expected to find: " << correctMessage;

    tm->UnregisterAndDestroyTaskQueue(queue);
    TaskScheduler::Destroy();
}

TEST(TaskSchedulerMetrics, NoInfoLoggingTest)
{
    constexpr size_t THREADS_COUNT = 4;
    auto *tm = TaskScheduler::Create(THREADS_COUNT, TaskTimeStatsType::NO_STATISTICS);
    ASSERT_FALSE(tm->IsTaskLifetimeStatisticsUsed());
    auto *queue = tm->CreateAndRegisterTaskQueue(TaskType::GC, VMType::STATIC_VM);
    TaskProperties prop(TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::FOREGROUND);
    tm->Initialize();

    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);

    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));

    testing::internal::CaptureStderr();

    queue->AddTask(Task::Create(prop, []() {}));
    tm->WaitForFinishAllTasksWithProperties(prop);
    tm->Finalize();

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(err.empty()) << "it should be no message in output, but it's equal to " << err;

    tm->UnregisterAndDestroyTaskQueue(queue);
    TaskScheduler::Destroy();
}

}  // namespace ark::taskmanager