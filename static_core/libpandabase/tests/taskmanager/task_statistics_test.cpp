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

#include "libpandabase/taskmanager/task_statistics/simple_task_statistics_impl.h"
#include "libpandabase/taskmanager/task_statistics/fine_grained_task_statistics_impl.h"
#include "libpandabase/taskmanager/task_statistics/lock_free_task_statistics_impl.h"
#include <gtest/gtest.h>
#include <thread>

namespace ark::taskmanager {

class TaskStatisticsTest : public testing::TestWithParam<TaskStatisticsImplType> {
public:
    static constexpr TaskProperties GC_STATIC_VM_BACKGROUND_PROPERTIES = {TaskType::GC, VMType::STATIC_VM,
                                                                          TaskExecutionMode::BACKGROUND};
    static constexpr TaskProperties GC_STATIC_VM_FOREGROUND_PROPERTIES = {TaskType::GC, VMType::STATIC_VM,
                                                                          TaskExecutionMode::FOREGROUND};
    static constexpr TaskProperties GC_DYNAMIC_VM_BACKGROUND_PROPERTIES = {TaskType::GC, VMType::DYNAMIC_VM,
                                                                           TaskExecutionMode::BACKGROUND};
    static constexpr TaskProperties GC_DYNAMIC_VM_FOREGROUND_PROPERTIES = {TaskType::GC, VMType::DYNAMIC_VM,
                                                                           TaskExecutionMode::FOREGROUND};

    TaskStatistics *GetStatistics()
    {
        switch (GetParam()) {
            case TaskStatisticsImplType::SIMPLE:
                return &simpleTaskStatistics_;
            case TaskStatisticsImplType::FINE_GRAINED:
                return &fineGrainedTaskStatistics_;
            case TaskStatisticsImplType::LOCK_FREE:
                return &lockFreeTaskStatistics_;
            default:
                UNREACHABLE();
        }
    }

private:
    SimpleTaskStatisticsImpl simpleTaskStatistics_;
    FineGrainedTaskStatisticsImpl fineGrainedTaskStatistics_;
    LockFreeTaskStatisticsImpl lockFreeTaskStatistics_;
};

TEST_P(TaskStatisticsTest, MultithreadedUsageTest)
{
    auto *taskStatistics = GetStatistics();
    constexpr size_t COUNT_OF_TASKS = 10'000U;
    auto addedAndExecuteTaskWorker = [taskStatistics](TaskProperties properties) {
        for (size_t i = 0U; i < COUNT_OF_TASKS; i++) {
            taskStatistics->IncrementCount(TaskStatus::ADDED, properties, 1U);
        }
        for (size_t i = 0U; i < COUNT_OF_TASKS; i++) {
            taskStatistics->IncrementCount(TaskStatus::EXECUTED, properties, 1U);
        }
    };

    constexpr size_t COUNT_OF_THREADS = 10U;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < COUNT_OF_THREADS; i++) {
        threads.emplace_back(addedAndExecuteTaskWorker, GC_STATIC_VM_BACKGROUND_PROPERTIES);
        threads.emplace_back(addedAndExecuteTaskWorker, GC_STATIC_VM_FOREGROUND_PROPERTIES);
        threads.emplace_back(addedAndExecuteTaskWorker, GC_DYNAMIC_VM_BACKGROUND_PROPERTIES);
        threads.emplace_back(addedAndExecuteTaskWorker, GC_DYNAMIC_VM_FOREGROUND_PROPERTIES);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    ASSERT_EQ(taskStatistics->GetCountOfTasksInSystemWithTaskProperties(GC_DYNAMIC_VM_BACKGROUND_PROPERTIES), 0U);
    ASSERT_EQ(taskStatistics->GetCountOfTasksInSystemWithTaskProperties(GC_DYNAMIC_VM_FOREGROUND_PROPERTIES), 0U);
    ASSERT_EQ(taskStatistics->GetCountOfTasksInSystemWithTaskProperties(GC_STATIC_VM_BACKGROUND_PROPERTIES), 0U);
    ASSERT_EQ(taskStatistics->GetCountOfTasksInSystemWithTaskProperties(GC_STATIC_VM_FOREGROUND_PROPERTIES), 0U);
    ASSERT_EQ(taskStatistics->GetCountOfTaskInSystem(), 0U);
}

INSTANTIATE_TEST_SUITE_P(TaskStatisticsImplSet, TaskStatisticsTest,
                         testing::Values(TaskStatisticsImplType::SIMPLE, TaskStatisticsImplType::FINE_GRAINED,
                                         TaskStatisticsImplType::LOCK_FREE));

}  // namespace ark::taskmanager
