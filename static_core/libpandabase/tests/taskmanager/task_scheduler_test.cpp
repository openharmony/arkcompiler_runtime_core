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
#include "libpandabase/taskmanager/task_queue.h"
#include "libpandabase/taskmanager/task.h"
#include <tuple>
#include <gtest/gtest.h>

namespace ark::taskmanager {

constexpr size_t DEFAULT_SEED = 123456;
constexpr size_t TIMEOUT = 1;

class TaskSchedulerTest : public testing::TestWithParam<TaskStatisticsImplType> {
public:
    static constexpr TaskProperties GC_STATIC_VM_BACKGROUND_PROPERTIES {TaskType::GC, VMType::STATIC_VM,
                                                                        TaskExecutionMode::BACKGROUND};
    static constexpr TaskProperties GC_STATIC_VM_FOREGROUND_PROPERTIES {TaskType::GC, VMType::STATIC_VM,
                                                                        TaskExecutionMode::FOREGROUND};
    static constexpr TaskProperties JIT_STATIC_VM_BACKGROUND_PROPERTIES {TaskType::JIT, VMType::STATIC_VM,
                                                                         TaskExecutionMode::BACKGROUND};
    TaskSchedulerTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        seed_ = DEFAULT_SEED;
#endif
    };
    ~TaskSchedulerTest() override = default;

    NO_COPY_SEMANTIC(TaskSchedulerTest);
    NO_MOVE_SEMANTIC(TaskSchedulerTest);

    static constexpr size_t THREADED_TASKS_COUNT = 100'000;

    std::thread *CreateTaskProducerThread(TaskQueueInterface *queue, TaskExecutionMode mode)
    {
        return new std::thread(
            [queue, mode](TaskSchedulerTest *test) {
                for (size_t i = 0; i < THREADED_TASKS_COUNT; i++) {
                    queue->AddTask(Task::Create({queue->GetTaskType(), queue->GetVMType(), mode},
                                                [test]() { test->IncrementGlobalCounter(); }));
                }
                test->AddedSetOfTasks();
            },
            this);
    }

    void IncrementGlobalCounter()
    {
        globalCounter_++;
    }

    size_t GetGlobalCounter() const
    {
        return globalCounter_;
    }

    void SetTasksSetCount(size_t setCount)
    {
        tasksCount_ = setCount;
    }

    /// Wait for all tasks would be added in queues
    void WaitAllTask()
    {
        os::memory::LockHolder lockHolder(tasksMutex_);
        while (tasksSetAdded_ != tasksCount_) {
            tasksCondVar_.TimedWait(&tasksMutex_, TIMEOUT);
        }
    }

    void AddedSetOfTasks()
    {
        os::memory::LockHolder lockHolder(tasksMutex_);
        tasksSetAdded_++;
        tasksCondVar_.SignalAll();
    }

    size_t GetSeed() const
    {
        return seed_;
    }

private:
    os::memory::Mutex lock_;
    os::memory::ConditionVariable condVar_;
    std::atomic_size_t globalCounter_ = 0;

    size_t tasksCount_ = 0;
    std::atomic_size_t tasksSetAdded_ = 0;
    os::memory::Mutex tasksMutex_;
    os::memory::ConditionVariable tasksCondVar_;

    size_t seed_ = 0;
};

TEST_P(TaskSchedulerTest, TaskQueueRegistration)
{
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    TaskQueueInterface *queue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    EXPECT_NE(queue, nullptr);
    EXPECT_EQ(tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY), nullptr);
    tm->UnregisterAndDestroyTaskQueue<>(queue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, TaskQueuesFillingFromOwner)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::DEFAULT_PRIORITY;
    TaskQueueInterface *gcQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    // Initialize tm workers
    tm->Initialize();
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 10;
    std::array<std::atomic_size_t, 2> counters = {0, 0};
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            constexpr size_t GC_COUNTER = 0;
            // Atomic with relaxed order reason: data race with counters[GC_COUNTER] with no synchronization or ordering
            // constraints
            counters[GC_COUNTER].fetch_add(1, std::memory_order_relaxed);
        }));
        jitQueue->AddTask(Task::Create(JIT_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            constexpr size_t JIT_COUNTER = 1;
            // Atomic with relaxed order reason: data race with counters[JIT_COUNTER] with no synchronization or
            // ordering constraints
            counters[JIT_COUNTER].fetch_add(1, std::memory_order_relaxed);
        }));
    }
    tm->Finalize();
    for (auto &counter : counters) {
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    tm->UnregisterAndDestroyTaskQueue<>(gcQueue);
    tm->UnregisterAndDestroyTaskQueue<>(jitQueue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, ForegroundQueueTest)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 1;  // IMPORTANT: only one worker to see effect of using foreground execution mode
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::DEFAULT_PRIORITY;
    TaskQueueInterface *gcQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);

    // Fill queues with tasks that push their TaskExecutionMode to global queue.
    std::queue<TaskExecutionMode> globalQueue;
    gcQueue->AddTask(Task::Create({TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND},
                                  [&globalQueue]() { globalQueue.push(TaskExecutionMode::BACKGROUND); }));
    gcQueue->AddTask(Task::Create({TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND},
                                  [&globalQueue]() { globalQueue.push(TaskExecutionMode::BACKGROUND); }));
    gcQueue->AddTask(Task::Create({TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::FOREGROUND},
                                  [&globalQueue]() { globalQueue.push(TaskExecutionMode::FOREGROUND); }));
    gcQueue->AddTask(Task::Create({TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND},
                                  [&globalQueue]() { globalQueue.push(TaskExecutionMode::BACKGROUND); }));
    // Initialize tm workers
    tm->Initialize();
    // Wait that do work
    tm->Finalize();

    ASSERT_EQ(globalQueue.front(), TaskExecutionMode::FOREGROUND) << "seed:" << GetSeed();
    globalQueue.pop();
    ASSERT_EQ(globalQueue.front(), TaskExecutionMode::BACKGROUND) << "seed:" << GetSeed();
    globalQueue.pop();
    ASSERT_EQ(globalQueue.front(), TaskExecutionMode::BACKGROUND) << "seed:" << GetSeed();
    globalQueue.pop();
    ASSERT_EQ(globalQueue.front(), TaskExecutionMode::BACKGROUND) << "seed:" << GetSeed();
    globalQueue.pop();
    ASSERT_TRUE(globalQueue.empty());
    tm->UnregisterAndDestroyTaskQueue<>(gcQueue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, TaskCreateTask)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::DEFAULT_PRIORITY;
    TaskQueueInterface *gcQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);

    // Initialize tm workers
    tm->Initialize();
    // Fill queues with tasks that increment counter with its type. GC task will add JIT task in MT.
    std::array<std::atomic_size_t, 2> counters = {0, 0};
    constexpr size_t COUNT_OF_TASK = 10;
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&counters, &jitQueue]() {
            constexpr size_t GC_COUNTER = 0;
            // Atomic with relaxed order reason: data race with counters[GC_COUNTER] with no synchronization or ordering
            // constraints
            counters[GC_COUNTER].fetch_add(1, std::memory_order_relaxed);
            jitQueue->AddTask(
                Task::Create({TaskType::JIT, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND}, [&counters]() {
                    constexpr size_t JIT_COUNTER = 1;
                    // Atomic with relaxed order reason: data race with counters[JIT_COUNTER] with no synchronization or
                    // ordering constraints
                    counters[JIT_COUNTER].fetch_add(1, std::memory_order_relaxed);
                }));
        }));
    }
    tm->Finalize();
    for (auto &counter : counters) {
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    tm->UnregisterAndDestroyTaskQueue<>(gcQueue);
    tm->UnregisterAndDestroyTaskQueue<>(jitQueue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, MultithreadingUsage)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create 4 thread. Each thread create, register and fill queues
    constexpr size_t PRODUCER_THREADS_COUNT = 3;
    SetTasksSetCount(PRODUCER_THREADS_COUNT);
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::DEFAULT_PRIORITY;
    auto *jitStaticQueue = tm->CreateAndRegisterTaskQueue(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    auto *gcStaticQueue = tm->CreateAndRegisterTaskQueue(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    auto *gcDynamicQueue = tm->CreateAndRegisterTaskQueue(TaskType::GC, VMType::DYNAMIC_VM, QUEUE_PRIORITY);

    auto jitStaticThread = CreateTaskProducerThread(jitStaticQueue, TaskExecutionMode::BACKGROUND);
    auto gcStaticThread = CreateTaskProducerThread(gcStaticQueue, TaskExecutionMode::BACKGROUND);
    auto gcDynamicThread = CreateTaskProducerThread(gcDynamicQueue, TaskExecutionMode::BACKGROUND);

    tm->Initialize();
    /* Wait for all tasks would be added before tm->Finalize */
    WaitAllTask();
    tm->Finalize();

    ASSERT_EQ(GetGlobalCounter(), THREADED_TASKS_COUNT * PRODUCER_THREADS_COUNT) << "seed:" << GetSeed();

    jitStaticThread->join();
    gcStaticThread->join();
    gcDynamicThread->join();

    delete jitStaticThread;
    delete gcStaticThread;
    delete gcDynamicThread;

    tm->UnregisterAndDestroyTaskQueue(jitStaticQueue);
    tm->UnregisterAndDestroyTaskQueue(gcStaticQueue);
    tm->UnregisterAndDestroyTaskQueue(gcDynamicQueue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, TaskSchedulerGetTask)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 1;  // Worker will not be used in this test
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    auto queue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    std::queue<TaskType> globalQueue;
    constexpr size_t COUNT_OF_TASKS = 100;
    for (size_t i = 0; i < COUNT_OF_TASKS; i++) {
        queue->AddTask(
            Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&globalQueue]() { globalQueue.push(TaskType::GC); }));
    }
    for (size_t i = 0; i < COUNT_OF_TASKS;) {
        i += tm->HelpWorkersWithTasks(GC_STATIC_VM_BACKGROUND_PROPERTIES);
    }
    ASSERT_EQ(tm->HelpWorkersWithTasks(GC_STATIC_VM_BACKGROUND_PROPERTIES), 0) << "seed:" << GetSeed();
    ASSERT_EQ(globalQueue.size(), COUNT_OF_TASKS) << "seed:" << GetSeed();
    tm->Initialize();
    tm->Finalize();
    ASSERT_EQ(tm->HelpWorkersWithTasks(GC_STATIC_VM_BACKGROUND_PROPERTIES), 0) << "seed:" << GetSeed();
    tm->UnregisterAndDestroyTaskQueue<>(queue);
    tm->Destroy();
}

TEST_P(TaskSchedulerTest, TasksWithMutex)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::DEFAULT_PRIORITY;
    TaskQueueInterface *gcQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    // Initialize tm workers
    tm->Initialize();
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 1000;
    std::array<size_t, 2> counters = {0, 0};
    os::memory::Mutex mainMutex;
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&mainMutex, &counters]() {
            constexpr size_t GC_COUNTER = 0;
            os::memory::LockHolder lockHolder(mainMutex);
            counters[GC_COUNTER]++;
        }));
        jitQueue->AddTask(Task::Create(JIT_STATIC_VM_BACKGROUND_PROPERTIES, [&mainMutex, &counters]() {
            constexpr size_t JIT_COUNTER = 1;
            os::memory::LockHolder lockHolder(mainMutex);
            counters[JIT_COUNTER]++;
        }));
    }
    tm->Finalize();
    for (auto &counter : counters) {
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    tm->UnregisterAndDestroyTaskQueue<>(gcQueue);
    tm->UnregisterAndDestroyTaskQueue<>(jitQueue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, TaskCreateTaskRecursively)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    TaskQueueInterface *gcQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);

    // Initialize tm workers
    tm->Initialize();
    std::atomic_size_t counter = 0;
    constexpr size_t COUNT_OF_TASK = 10;
    constexpr size_t COUNT_OF_REPLICAS = 6;
    constexpr size_t MAX_RECURSION_DEPTH = 5;
    std::function<void(size_t)> runner;
    runner = [&counter, &runner, &gcQueue](size_t recursionDepth) {
        if (recursionDepth < MAX_RECURSION_DEPTH) {
            for (size_t j = 0; j < COUNT_OF_REPLICAS; j++) {
                gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES,
                                              [runner, recursionDepth]() { runner(recursionDepth + 1); }));
            }
            // Atomic with relaxed order reason: data race with counter with no synchronization or ordering
            // constraints
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [runner]() { runner(0); }));
    }
    tm->Finalize();
    ASSERT_TRUE(gcQueue->IsEmpty());
    tm->UnregisterAndDestroyTaskQueue<>(gcQueue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, TaskSchedulerTaskGetTask)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::MAX_PRIORITY;
    TaskQueueInterface *gcQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);

    // Initialize tm workers
    tm->Initialize();
    std::atomic_size_t counter = 0;
    constexpr size_t COUNT_OF_TASK = 100'000;
    gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, []() {
        size_t executedTasksCount = 0;
        while (true) {  // wait for valid task;
            executedTasksCount =
                TaskScheduler::GetTaskScheduler()->HelpWorkersWithTasks(GC_STATIC_VM_BACKGROUND_PROPERTIES);
            if (executedTasksCount > 0) {
                break;
            }
        }
        for (size_t i = 0; i < COUNT_OF_TASK; i++) {
            executedTasksCount +=
                TaskScheduler::GetTaskScheduler()->HelpWorkersWithTasks(GC_STATIC_VM_BACKGROUND_PROPERTIES);
        }
    }));
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&counter]() {
            // Atomic with relaxed order reason: data race with counter with no synchronization or ordering
            // constraints
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }
    tm->Finalize();
    ASSERT_TRUE(gcQueue->IsEmpty());
    tm->UnregisterAndDestroyTaskQueue<>(gcQueue);
    TaskScheduler::Destroy();
}

TEST_P(TaskSchedulerTest, TaskSchedulerWaitForFinishAllTaskFromQueue)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4;
    auto taskStatisticsType = GetParam();
    auto *tm = TaskScheduler::Create(THREADS_COUNT, taskStatisticsType);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = TaskQueueInterface::DEFAULT_PRIORITY;
    TaskQueueInterface *gcQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = tm->CreateAndRegisterTaskQueue<>(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 10'000;
    std::array<std::atomic_size_t, 3U> counters = {0, 0, 0};
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gcQueue->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            constexpr size_t GC_BACKGROUND_COUNTER = 0;
            // Atomic with relaxed order reason: data race with counters[GC_BACKGROUND_COUNTER] with no synchronization
            // or ordering constraints
            counters[GC_BACKGROUND_COUNTER].fetch_add(1, std::memory_order_relaxed);
        }));
        gcQueue->AddTask(Task::Create(GC_STATIC_VM_FOREGROUND_PROPERTIES, [&counters]() {
            constexpr size_t GC_FOREGROUND_COUNTER = 1;
            // Atomic with relaxed order reason: data race with counters[GC_FOREGROUND_COUNTER] with no synchronization
            // or ordering constraints
            counters[GC_FOREGROUND_COUNTER].fetch_add(1, std::memory_order_relaxed);
        }));
        jitQueue->AddTask(Task::Create(JIT_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            constexpr size_t JIT_COUNTER = 2;
            // Atomic with relaxed order reason: data race with counters[JIT_COUNTER] with no synchronization or
            // ordering constraints
            counters[JIT_COUNTER].fetch_add(1, std::memory_order_relaxed);
        }));
    }
    // Initialize tm workers
    tm->Initialize();
    tm->WaitForFinishAllTasksWithProperties(GC_STATIC_VM_FOREGROUND_PROPERTIES);
    ASSERT_FALSE(gcQueue->HasTaskWithExecutionMode(TaskExecutionMode::FOREGROUND));
    tm->WaitForFinishAllTasksWithProperties(GC_STATIC_VM_BACKGROUND_PROPERTIES);
    ASSERT_TRUE(gcQueue->IsEmpty());
    tm->WaitForFinishAllTasksWithProperties(JIT_STATIC_VM_BACKGROUND_PROPERTIES);
    ASSERT_TRUE(jitQueue->IsEmpty());
    tm->Finalize();
    for (auto &counter : counters) {
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    tm->UnregisterAndDestroyTaskQueue<>(gcQueue);
    tm->UnregisterAndDestroyTaskQueue<>(jitQueue);
    TaskScheduler::Destroy();
}

INSTANTIATE_TEST_SUITE_P(TaskStatisticsTypeSet, TaskSchedulerTest,
                         ::testing::Values(TaskStatisticsImplType::SIMPLE, TaskStatisticsImplType::FINE_GRAINED,
                                           TaskStatisticsImplType::LOCK_FREE, TaskStatisticsImplType::NO_STAT));

}  // namespace ark::taskmanager
