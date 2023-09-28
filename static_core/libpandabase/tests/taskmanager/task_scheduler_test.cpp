/**
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

#include "libpandabase/taskmanager/task_scheduler.h"
#include "libpandabase/taskmanager/task.h"
#include <gtest/gtest.h>

namespace panda::taskmanager {

constexpr size_t DEFAULT_SEED = 123456;
constexpr size_t TIMEOUT = 1;

class TaskSchedulerTest : public testing::Test {
public:
    static constexpr TaskProperties GC_STATIC_VM_BACKGROUND_PROPERTIES {TaskType::GC, VMType::STATIC_VM,
                                                                        TaskExecutionMode::BACKGROUND};
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

    std::thread *CreateTaskProducerThread(TaskType task_type, VMType vm_type, size_t priority, TaskExecutionMode mode)
    {
        return new std::thread(
            [task_type, vm_type, priority, mode](TaskSchedulerTest *test) {
                TaskQueue queue(task_type, vm_type, priority);
                TaskScheduler::GetTaskScheduler()->RegisterQueue(&queue);
                test->RegisteredOneQueue();
                for (size_t i = 0; i < THREADED_TASKS_COUNT; i++) {
                    queue.AddTask(
                        Task::Create({task_type, vm_type, mode}, [test]() { test->IncrementGlobalCounter(); }));
                }
                test->AddedSetOfTasks();
                test->WaitFinish();
            },
            this);
    }

    void IncrementGlobalCounter()
    {
        global_counter_++;
    }

    size_t GetGlobalCounter() const
    {
        return global_counter_;
    }

    void SetQueueCount(size_t queue_count)
    {
        queue_count_ = queue_count;
    }

    void SetTasksSetCount(size_t set_count)
    {
        tasks_count_ = set_count;
    }

    /// Wait for all queues will be registered
    void WaitAllQueue()
    {
        os::memory::LockHolder lock_holder(queues_mutex_);
        while (registered_queues_counter_ != queue_count_) {
            queues_cond_var_.TimedWait(&queues_mutex_, TIMEOUT);
        }
    }

    /// Wait for all tasks would be added in queues
    void WaitAllTask()
    {
        os::memory::LockHolder lock_holder(tasks_mutex_);
        while (tasks_set_added_ != tasks_count_) {
            tasks_cond_var_.TimedWait(&tasks_mutex_, TIMEOUT);
        }
    }

    void WaitFinish()
    {
        os::memory::LockHolder lock_holder(finish_mutex_);
        while (!finish_) {
            finish_cond_var_.TimedWait(&finish_mutex_, TIMEOUT);
        }
    }

    void SetFinish()
    {
        os::memory::LockHolder lock_holder(finish_mutex_);
        finish_ = true;
        finish_cond_var_.SignalAll();
    }

    void AddedSetOfTasks()
    {
        os::memory::LockHolder lock_holder(tasks_mutex_);
        tasks_set_added_++;
        tasks_cond_var_.SignalAll();
    }

    void RegisteredOneQueue()
    {
        os::memory::LockHolder lock_holder(queues_mutex_);
        registered_queues_counter_++;
        queues_cond_var_.SignalAll();
    }

    size_t GetSeed() const
    {
        return seed_;
    }

private:
    os::memory::Mutex lock_;
    os::memory::ConditionVariable cond_var_;
    std::atomic_size_t global_counter_ = 0;

    size_t queue_count_ = 0;
    std::atomic_size_t registered_queues_counter_ = 0;
    os::memory::Mutex queues_mutex_;
    os::memory::ConditionVariable queues_cond_var_;

    size_t tasks_count_ = 0;
    std::atomic_size_t tasks_set_added_ = 0;
    os::memory::Mutex tasks_mutex_;
    os::memory::ConditionVariable tasks_cond_var_;

    std::atomic_bool finish_ = false;
    os::memory::Mutex finish_mutex_;
    os::memory::ConditionVariable finish_cond_var_;

    size_t seed_ = 0;
};

TEST_F(TaskSchedulerTest, TaskQueuesFillingFromOwner)
{
    // Set and print seed
    srand(GetSeed());
    std::cout << "seed:" << GetSeed() << std::endl;
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 5;
    auto *tm = TaskScheduler::Create(THREADS_COUNT);
    // Create and register 2 queues
    constexpr size_t QUEUE_PRIORITY = 50;
    TaskQueue gc_queue(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueue jit_queue(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    tm->RegisterQueue(&gc_queue);
    tm->RegisterQueue(&jit_queue);
    // Initialize tm workers
    tm->Initialize();
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 10;
    std::array<std::atomic_size_t, 2> counters = {0, 0};
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gc_queue.AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            // Atomic with relaxed order reason: data race with counters[static_cast<size_t>(TaskType::GC)] with
            // no synchronization or ordering constraints
            counters[static_cast<size_t>(TaskType::GC)].fetch_add(1, std::memory_order_relaxed);
        }));
        jit_queue.AddTask(Task::Create(JIT_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            // Atomic with relaxed order reason: data race with counters[static_cast<size_t>(TaskType::JIT)]
            // with no synchronization or ordering constraints
            counters[static_cast<size_t>(TaskType::JIT)].fetch_add(1, std::memory_order_relaxed);
        }));
    }
    tm->Finalize();
    for (auto &counter : counters) {
        EXPECT_EQ(counter, COUNT_OF_TASK);
    }
    TaskScheduler::Destroy();
}

TEST_F(TaskSchedulerTest, TaskQueuesFillingFromTaskScheduler)
{
    // Set and print seed
    srand(GetSeed());
    std::cout << "seed:" << GetSeed() << std::endl;
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 5;
    auto *tm = TaskScheduler::Create(THREADS_COUNT);
    // Create and register 2 queues
    constexpr size_t QUEUE_PRIORITY = 50;
    TaskQueue gc_queue(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueue jit_queue(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    tm->RegisterQueue(&gc_queue);
    tm->RegisterQueue(&jit_queue);
    // Initialize tm workers
    tm->Initialize();
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 10;
    std::array<std::atomic_size_t, 2> counters = {0, 0};
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        tm->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            // Atomic with relaxed order reason: data race with counters[static_cast<size_t>(TaskType::GC)] with no
            // synchronization or ordering constraints
            counters[static_cast<size_t>(TaskType::GC)].fetch_add(1, std::memory_order_relaxed);
        }));
        tm->AddTask(Task::Create(JIT_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
            // Atomic with relaxed order reason: data race with counters[static_cast<size_t>(TaskType::JIT)] with no
            // synchronization or ordering constraints
            counters[static_cast<size_t>(TaskType::JIT)].fetch_add(1, std::memory_order_relaxed);
        }));
    }
    tm->Finalize();
    for (auto &counter : counters) {
        EXPECT_EQ(counter, COUNT_OF_TASK);
    }
    TaskScheduler::Destroy();
}

TEST_F(TaskSchedulerTest, ForegroundQueueTest)
{
    // Set and print seed
    srand(GetSeed());
    std::cout << "seed:" << GetSeed() << std::endl;
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 1;  // IMPORTANT: only one worker to see effect of using foreground execution mode
    auto *tm = TaskScheduler::Create(THREADS_COUNT);
    // Create and register 2 queues
    constexpr size_t QUEUE_PRIORITY = 50;
    TaskQueue gc_queue(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueue jit_queue(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    tm->RegisterQueue(&gc_queue);
    tm->RegisterQueue(&jit_queue);

    // Fill queues with tasks that push their TaskType to global queue.
    std::queue<TaskType> global_queue;
    tm->AddTask(Task::Create({TaskType::JIT, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND},
                             [&global_queue]() { global_queue.push(TaskType::JIT); }));
    tm->AddTask(Task::Create({TaskType::JIT, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND},
                             [&global_queue]() { global_queue.push(TaskType::JIT); }));
    tm->AddTask(Task::Create({TaskType::GC, VMType::STATIC_VM, TaskExecutionMode::FOREGROUND},
                             [&global_queue]() { global_queue.push(TaskType::GC); }));
    tm->AddTask(Task::Create({TaskType::JIT, VMType::STATIC_VM, TaskExecutionMode::BACKGROUND},
                             [&global_queue]() { global_queue.push(TaskType::JIT); }));
    // Initialize tm workers
    tm->Initialize();
    // Wait that do work
    tm->Finalize();

    EXPECT_EQ(global_queue.front(), TaskType::GC);
    global_queue.pop();
    EXPECT_EQ(global_queue.front(), TaskType::JIT);
    global_queue.pop();
    EXPECT_EQ(global_queue.front(), TaskType::JIT);
    global_queue.pop();
    EXPECT_EQ(global_queue.front(), TaskType::JIT);
    global_queue.pop();
    EXPECT_TRUE(global_queue.empty());
    TaskScheduler::Destroy();
}

TEST_F(TaskSchedulerTest, TaskCreateTask)
{
    // Set and print seed
    srand(GetSeed());
    std::cout << "seed:" << GetSeed() << std::endl;
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 5;
    auto *tm = TaskScheduler::Create(THREADS_COUNT);
    // Create and register 2 queues
    constexpr size_t QUEUE_PRIORITY = 50;
    TaskQueue gc_queue(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueue jit_queue(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    tm->RegisterQueue(&gc_queue);
    tm->RegisterQueue(&jit_queue);

    // Initialize tm workers
    tm->Initialize();
    // Fill queues with tasks that increment counter with its type. GC task will add JIT task in MT.
    std::array<std::atomic_size_t, 2> counters = {0, 0};
    constexpr size_t COUNT_OF_TASK = 10;
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        tm->AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&counters, &tm]() {
            // Atomic with relaxed order reason: data race with counters[static_cast<size_t>(TaskType::GC)] with
            // no synchronization or ordering constraints
            counters[static_cast<size_t>(TaskType::GC)].fetch_add(1, std::memory_order_relaxed);
            tm->AddTask(Task::Create(JIT_STATIC_VM_BACKGROUND_PROPERTIES, [&counters]() {
                // Atomic with relaxed order reason: data race with
                // counters[static_cast<size_t>(TaskType::JIT)] with no synchronization or ordering
                // constraints
                counters[static_cast<size_t>(TaskType::JIT)].fetch_add(1, std::memory_order_relaxed);
            }));
        }));
    }
    tm->Finalize();
    EXPECT_EQ(counters[static_cast<size_t>(TaskType::GC)], COUNT_OF_TASK);
    EXPECT_EQ(counters[static_cast<size_t>(TaskType::JIT)], COUNT_OF_TASK);
    TaskScheduler::Destroy();
}

TEST_F(TaskSchedulerTest, MultithreadingUsage)
{
    // Set and print seed
    srand(GetSeed());
    std::cout << "seed:" << GetSeed() << std::endl;
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 15;
    auto *tm = TaskScheduler::Create(THREADS_COUNT);
    // Create 4 thread. Each thread create, register and fill queues
    constexpr size_t PRODUCER_THREADS_COUNT = 4;
    SetQueueCount(PRODUCER_THREADS_COUNT);
    SetTasksSetCount(PRODUCER_THREADS_COUNT);
    constexpr size_t QUEUE_PRIORITY = 25;
    auto jit_static_thread =
        CreateTaskProducerThread(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY, TaskExecutionMode::BACKGROUND);
    auto jit_dynamic_thread =
        CreateTaskProducerThread(TaskType::JIT, VMType::DYNAMIC_VM, QUEUE_PRIORITY, TaskExecutionMode::BACKGROUND);
    auto gc_static_thread =
        CreateTaskProducerThread(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY, TaskExecutionMode::BACKGROUND);
    auto gc_dynamic_thread =
        CreateTaskProducerThread(TaskType::GC, VMType::DYNAMIC_VM, QUEUE_PRIORITY, TaskExecutionMode::BACKGROUND);

    /* Wait until all threads register their queues before tm->Initialize */
    WaitAllQueue();
    tm->Initialize();
    /* Wait for all tasks would be added before tm->Finalize */
    WaitAllTask();
    tm->Finalize();

    TaskScheduler::Destroy();
    EXPECT_EQ(GetGlobalCounter(), THREADED_TASKS_COUNT * PRODUCER_THREADS_COUNT);
    SetFinish();

    jit_static_thread->join();
    jit_dynamic_thread->join();
    gc_static_thread->join();
    gc_dynamic_thread->join();

    delete jit_static_thread;
    delete jit_dynamic_thread;
    delete gc_static_thread;
    delete gc_dynamic_thread;
}

TEST_F(TaskSchedulerTest, TaskSchedulerGetTask)
{
    // Set and print seed
    srand(GetSeed());
    std::cout << "seed:" << GetSeed() << std::endl;
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 1;  // Worker will not be used in this test
    auto *tm = TaskScheduler::Create(THREADS_COUNT);
    constexpr size_t PRIORITY = 100;
    auto queue = TaskQueue(TaskType::GC, VMType::STATIC_VM, PRIORITY);
    tm->RegisterQueue(&queue);
    std::queue<TaskType> global_queue;
    constexpr size_t COUNT_OF_TASKS = 100;
    for (size_t i = 0; i < COUNT_OF_TASKS; i++) {
        tm->AddTask(
            Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&global_queue]() { global_queue.push(TaskType::GC); }));
    }
    for (size_t i = 0; i < COUNT_OF_TASKS; i++) {
        tm->GetTaskByProperties(GC_STATIC_VM_BACKGROUND_PROPERTIES).value().RunTask();
    }
    EXPECT_FALSE(tm->GetTaskByProperties(GC_STATIC_VM_BACKGROUND_PROPERTIES).has_value());
    EXPECT_EQ(global_queue.size(), COUNT_OF_TASKS);
    tm->Initialize();
    tm->Finalize();
    tm->Destroy();
}

TEST_F(TaskSchedulerTest, TasksWithMutex)
{
    // Set and print seed
    srand(GetSeed());
    std::cout << "seed:" << GetSeed() << std::endl;
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 10;
    auto *tm = TaskScheduler::Create(THREADS_COUNT);
    // Create and register 2 queues
    constexpr size_t QUEUE_PRIORITY = 50;
    TaskQueue gc_task_queue(TaskType::GC, VMType::STATIC_VM, QUEUE_PRIORITY);
    TaskQueue jit_task_queue(TaskType::JIT, VMType::STATIC_VM, QUEUE_PRIORITY);
    tm->RegisterQueue(&gc_task_queue);
    tm->RegisterQueue(&jit_task_queue);
    // Initialize tm workers
    tm->Initialize();
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 1000;
    std::array<size_t, 2> counters = {0, 0};
    os::memory::Mutex main_mutex;
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gc_task_queue.AddTask(Task::Create(GC_STATIC_VM_BACKGROUND_PROPERTIES, [&main_mutex, &counters]() {
            os::memory::LockHolder lock_holder(main_mutex);
            counters[static_cast<size_t>(TaskType::GC)]++;
        }));
        jit_task_queue.AddTask(Task::Create(JIT_STATIC_VM_BACKGROUND_PROPERTIES, [&main_mutex, &counters]() {
            os::memory::LockHolder lock_holder(main_mutex);
            counters[static_cast<size_t>(TaskType::JIT)]++;
        }));
    }
    tm->Finalize();
    for (auto &counter : counters) {
        EXPECT_EQ(counter, COUNT_OF_TASK);
    }
    TaskScheduler::Destroy();
}

}  // namespace panda::taskmanager