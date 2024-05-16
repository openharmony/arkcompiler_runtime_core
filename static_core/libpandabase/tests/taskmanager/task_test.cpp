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

#include "libpandabase/taskmanager/task_queue.h"
#include <gtest/gtest.h>
#include <thread>

namespace ark::taskmanager::internal {

class TaskTest : public testing::Test {
public:
    static constexpr TaskProperties TASK_PROPERTIES {TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND};

    TaskTest() = default;
    ~TaskTest() override = default;

    NO_COPY_SEMANTIC(TaskTest);
    NO_MOVE_SEMANTIC(TaskTest);

private:
};

TEST_F(TaskTest, TaskPropertiesTest)
{
    for (auto taskType : ALL_TASK_TYPES) {
        for (auto vmType : ALL_VM_TYPES) {
            for (auto executionMode : ALL_TASK_EXECUTION_MODES) {
                TaskProperties prop(taskType, vmType, executionMode);
                ASSERT_EQ(prop.GetTaskType(), taskType);
                ASSERT_EQ(prop.GetVMType(), vmType);
                ASSERT_EQ(prop.GetTaskExecutionMode(), executionMode);
            }
        }
    }
}

TEST_F(TaskTest, TaskSimpleTest)
{
    std::string message = "task:";
    Task task = Task::Create({TaskType::GC, VMType::DYNAMIC_VM, TaskExecutionMode::BACKGROUND},
                             [&message]() { message += "done"; });
    EXPECT_EQ(message, "task:");
    EXPECT_EQ(task.GetTaskProperties().GetTaskType(), TaskType::GC);
    EXPECT_EQ(task.GetTaskProperties().GetVMType(), VMType::DYNAMIC_VM);
    EXPECT_EQ(task.GetTaskProperties().GetTaskExecutionMode(), TaskExecutionMode::BACKGROUND);
    task.RunTask();
    EXPECT_EQ(message, "task:done");
    EXPECT_EQ(task.GetTaskProperties().GetTaskType(), TaskType::GC);
    EXPECT_EQ(task.GetTaskProperties().GetVMType(), VMType::DYNAMIC_VM);
    EXPECT_EQ(task.GetTaskProperties().GetTaskExecutionMode(), TaskExecutionMode::BACKGROUND);
}

TEST_F(TaskTest, TaskQueueSimpleTest)
{
    size_t counter = 0;
    // Creation of TaskQueue
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(TaskType::GC, VMType::DYNAMIC_VM, QUEUE_PRIORITY);
    EXPECT_EQ(queue->GetTaskType(), TaskType::GC);
    EXPECT_TRUE(queue->IsEmpty());
    EXPECT_EQ(queue->Size(), 0);
    EXPECT_EQ(queue->GetPriority(), QUEUE_PRIORITY);
    // Add COUNT_OF_TASKS tasks in queue-> Each task increment counter.
    constexpr size_t COUNT_OF_TASKS = 10;
    for (size_t i = 0; i < COUNT_OF_TASKS; i++) {
        queue->AddTask(Task::Create({TaskType::GC, VMType::DYNAMIC_VM, TaskExecutionMode::BACKGROUND},
                                    [&counter]() { counter++; }));
    }
    EXPECT_EQ(queue->GetTaskType(), TaskType::GC);
    EXPECT_FALSE(queue->IsEmpty());
    EXPECT_EQ(queue->Size(), COUNT_OF_TASKS);
    // Pop count_of_done_task tasks from queue and execute them.
    constexpr size_t COUNT_OF_DONE_TASKS = 6;
    ASSERT(COUNT_OF_DONE_TASKS < COUNT_OF_TASKS);
    for (size_t i = 0; i < COUNT_OF_DONE_TASKS; i++) {
        auto popTask = static_cast<SchedulableTaskQueueInterface *>(queue)->PopTask();
        EXPECT_EQ(popTask.value().GetTaskProperties().GetTaskType(), TaskType::GC);
        popTask.value().RunTask();
        EXPECT_EQ(counter, i + 1);
    }
    // Now in queue counter_of_tasks - COUNT_OF_DONE_TASKS objects.
    EXPECT_EQ(queue->GetTaskType(), TaskType::GC);
    EXPECT_EQ(queue->IsEmpty(), COUNT_OF_TASKS == COUNT_OF_DONE_TASKS);
    EXPECT_EQ(queue->Size(), COUNT_OF_TASKS - COUNT_OF_DONE_TASKS);
    // Change priority of this queue
    constexpr size_t NEW_QUEUE_PRIORITY = TaskQueueInterface::MIN_PRIORITY;
    queue->SetPriority(NEW_QUEUE_PRIORITY);
    EXPECT_EQ(queue->GetTaskType(), TaskType::GC);
    EXPECT_EQ(queue->IsEmpty(), COUNT_OF_TASKS == COUNT_OF_DONE_TASKS);
    EXPECT_EQ(queue->Size(), COUNT_OF_TASKS - COUNT_OF_DONE_TASKS);
    EXPECT_EQ(queue->GetPriority(), NEW_QUEUE_PRIORITY);
    // Add in queue counter_of_tasks new tasks-> Each add 2 to counter
    for (size_t i = 0; i < COUNT_OF_TASKS; i++) {
        queue->AddTask(Task::Create({TaskType::GC, VMType::DYNAMIC_VM, TaskExecutionMode::BACKGROUND},
                                    [&counter]() { counter += 2U; }));
    }
    // After we have 2 * counter_of_tasks - counter_of_done_tasks objects in queue
    EXPECT_EQ(queue->GetTaskType(), TaskType::GC);
    EXPECT_FALSE(queue->IsEmpty());
    EXPECT_EQ(queue->Size(), 2U * COUNT_OF_TASKS - COUNT_OF_DONE_TASKS);
    // Pop and execute all tasks in queue->
    while (!queue->IsEmpty()) {
        auto nextTask = static_cast<SchedulableTaskQueueInterface *>(queue)->PopTask();
        nextTask.value().RunTask();
    }
    // After all task is done, counter = 3 * COUNT_OF_TASKS
    EXPECT_EQ(counter, 3 * COUNT_OF_TASKS);
    EXPECT_EQ(queue->GetTaskType(), TaskType::GC);
    EXPECT_EQ(queue->Size(), 0);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueMultithreadingOnePushOnePop)
{
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    std::atomic_size_t counter = 0;
    constexpr size_t RESULT_COUNT = 10'000;
    auto pusher = [&queue, &counter]() {
        for (size_t i = 0; i < RESULT_COUNT; i++) {
            queue->AddTask(Task::Create(TASK_PROPERTIES, [&counter]() { counter++; }));
        }
    };
    auto popper = [&queue]() {
        for (size_t i = 0; i < RESULT_COUNT;) {
            auto task = queue->PopTask();
            if (!task.has_value()) {
                continue;
            }
            task->RunTask();
            i++;
        }
    };
    auto *worker1 = new std::thread(pusher);
    auto *worker2 = new std::thread(popper);
    worker1->join();
    worker2->join();
    delete worker1;
    delete worker2;
    EXPECT_EQ(counter, RESULT_COUNT);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueMultithreadingNPushNPop)
{
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    std::atomic_size_t counter = 0;
    constexpr size_t RESULT_COUNT = 10'000;
    auto pusher = [&queue, &counter]() {
        for (size_t i = 0; i < RESULT_COUNT; i++) {
            queue->AddTask(Task::Create(TASK_PROPERTIES, [&counter]() { counter++; }));
        }
    };
    os::memory::Mutex popLock;
    auto popper = [&queue, &popLock]() {
        std::optional<Task> task;
        for (size_t i = 0; i < RESULT_COUNT;) {
            {
                os::memory::LockHolder popLockGourd(popLock);
                task = queue->PopTask();
            }
            if (!task.has_value()) {
                continue;
            }
            task->RunTask();
            i++;
        }
    };
    std::vector<std::thread *> pushers;
    std::vector<std::thread *> poppers;
    constexpr size_t COUNT_OF_WORKERS = 10;
    for (size_t i = 0; i < COUNT_OF_WORKERS; i++) {
        pushers.push_back(new std::thread(pusher));
        poppers.push_back(new std::thread(popper));
    }
    for (size_t i = 0; i < COUNT_OF_WORKERS; i++) {
        pushers[i]->join();
        poppers[i]->join();
        delete pushers[i];
        delete poppers[i];
    }
    EXPECT_EQ(counter, RESULT_COUNT * COUNT_OF_WORKERS);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueWaitForQueueEmptyAndFinish)
{
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    std::atomic_size_t counter = 0;
    constexpr size_t TASK_COUNT = 100'000;
    for (size_t i = 0; i < TASK_COUNT; i++) {
        queue->AddTask(Task::Create(TASK_PROPERTIES, [&counter]() { counter++; }));
    }

    constexpr size_t THREAD_COUNTER = 10;
    os::memory::Mutex popLock;
    std::vector<std::thread> poppers;
    for (size_t i = 0; i < THREAD_COUNTER; i++) {
        poppers.emplace_back([&queue, &popLock]() {
            while (true) {
                os::memory::LockHolder popLockGourd(popLock);
                auto task = queue->PopTask();
                if (!task.has_value()) {
                    break;
                }
                task.value().RunTask();
            }
        });
    }

    for (auto &popper : poppers) {
        popper.join();
    }
    EXPECT_EQ(counter, TASK_COUNT);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueForegroundAndBackgroundTasks)
{
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    std::queue<TaskExecutionMode> modeQueue;
    constexpr TaskProperties FOREGROUND_PROPERTIES(TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::FOREGROUND);
    constexpr TaskProperties BACKGROUND_PROPERTIES(TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND);
    constexpr size_t TASKS_COUNT = 100;

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddTask(
            Task::Create(BACKGROUND_PROPERTIES, [&modeQueue]() { modeQueue.push(TaskExecutionMode::BACKGROUND); }));
    }
    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddTask(
            Task::Create(FOREGROUND_PROPERTIES, [&modeQueue]() { modeQueue.push(TaskExecutionMode::FOREGROUND); }));
    }

    for (size_t i = 0; i < 2U * TASKS_COUNT; i++) {
        auto task = queue->PopTask();
        ASSERT_TRUE(task.has_value());
        task.value().RunTask();
    }

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, TaskExecutionMode::FOREGROUND);
    }
    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, TaskExecutionMode::BACKGROUND);
    }
    EXPECT_TRUE(modeQueue.empty());
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, PopTaskWithExecutionMode)
{
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    std::queue<TaskExecutionMode> modeQueue;
    constexpr TaskProperties FOREGROUND_PROPERTIES(TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::FOREGROUND);
    constexpr TaskProperties BACKGROUND_PROPERTIES(TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND);
    constexpr size_t TASKS_COUNT = 100;

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddTask(
            Task::Create(BACKGROUND_PROPERTIES, [&modeQueue]() { modeQueue.push(TaskExecutionMode::BACKGROUND); }));
    }
    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddTask(
            Task::Create(FOREGROUND_PROPERTIES, [&modeQueue]() { modeQueue.push(TaskExecutionMode::FOREGROUND); }));
    }

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto task = queue->PopTask(TaskExecutionMode::FOREGROUND);
        ASSERT_TRUE(task.has_value());
        task.value().RunTask();
        task = queue->PopTask(TaskExecutionMode::BACKGROUND);
        ASSERT_TRUE(task.has_value());
        task.value().RunTask();
    }

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, TaskExecutionMode::FOREGROUND);
        mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, TaskExecutionMode::BACKGROUND);
    }
    EXPECT_TRUE(modeQueue.empty());
    TaskQueue<>::Destroy(queue);
}

}  // namespace ark::taskmanager::internal
