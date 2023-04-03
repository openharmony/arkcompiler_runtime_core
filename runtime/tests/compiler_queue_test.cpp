/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "assembly-parser.h"
#include "runtime/compiler_queue_aged_counter_priority.h"
#include "runtime/compiler_queue_counter_priority.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"

namespace panda::test {

class CompilerQueueTest : public testing::Test {
public:
    CompilerQueueTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~CompilerQueueTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

protected:
    panda::MTManagedThread *thread_;
};

static Class *TestClassPrepare()
{
    auto source = R"(
        .function i32 g() {
            ldai 0
            return
        }

        .function i32 f() {
            ldai 0
            return
        }

        .function void main() {
            call f
            return.void
        }
    )";
    pandasm::Parser p;

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)->GetClass(
        ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    return klass;
}

static void GetAndCheckMethodIfExists(CompilerQueueInterface *queue, Method *target)
{
    auto method = queue->GetTask().GetMethod();
    // The element may expire and may be deleted
    if (method != nullptr) {
        ASSERT_EQ(method, target);
    }
}

static void WaitForExpire(uint millis)
{
    constexpr uint delta = 10;
    uint64_t startTime = time::GetCurrentTimeInMillis();
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    // sleep_for() works nondeterministically
    // use an additional check for more confidence
    // Note, the queue implementation uses GetCurrentTimeInMillis
    // to update aged counter
    while (time::GetCurrentTimeInMillis() < startTime + millis) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delta));
    }
}

// Testing of CounterQueue

TEST_F(CompilerQueueTest, AddGet)
{
    Class *klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    // Manual range
    main_method->SetHotnessCounter(1);
    f_method->SetHotnessCounter(2);
    g_method->SetHotnessCounter(3);

    RuntimeOptions options;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan());
    queue.AddTask(CompilerTask {main_method, 0});
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {g_method, 0});

    GetAndCheckMethodIfExists(&queue, g_method);
    GetAndCheckMethodIfExists(&queue, f_method);
    GetAndCheckMethodIfExists(&queue, main_method);
}

TEST_F(CompilerQueueTest, EqualCounters)
{
    Class *klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    // Manual range
    main_method->SetHotnessCounter(3);
    f_method->SetHotnessCounter(3);
    g_method->SetHotnessCounter(3);

    RuntimeOptions options;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan());

    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {g_method, 0});
    queue.AddTask(CompilerTask {main_method, 0});

    GetAndCheckMethodIfExists(&queue, f_method);
    GetAndCheckMethodIfExists(&queue, g_method);
    GetAndCheckMethodIfExists(&queue, main_method);
}

TEST_F(CompilerQueueTest, Expire)
{
    auto klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    RuntimeOptions options;
    constexpr int CompilerTaskLifeSpan1 = 500;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), CompilerTaskLifeSpan1);
    queue.AddTask(CompilerTask {main_method, 0});
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {g_method, 0});

    WaitForExpire(1000);

    // All tasks should expire after sleep
    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    constexpr int CompilerTaskLifeSpan2 = 0;
    CompilerPriorityCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                        options.GetCompilerQueueMaxLength(), CompilerTaskLifeSpan2);
    queue2.AddTask(CompilerTask {main_method, 0});
    queue2.AddTask(CompilerTask {f_method, 0});
    queue2.AddTask(CompilerTask {g_method, 0});

    // All tasks should expire without sleep
    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

TEST_F(CompilerQueueTest, Reorder)
{
    auto klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    main_method->SetHotnessCounter(1);
    f_method->SetHotnessCounter(2);
    g_method->SetHotnessCounter(3);

    RuntimeOptions options;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan());

    // It is possible, that the first added method is expired, and others are not
    // So, add to queue in reversed order to be sure, that the first method is present anyway
    queue.AddTask(CompilerTask {g_method, 0});
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {main_method, 0});

    // Change the order
    main_method->SetHotnessCounter(6);
    f_method->SetHotnessCounter(5);
    g_method->SetHotnessCounter(4);

    GetAndCheckMethodIfExists(&queue, main_method);
    GetAndCheckMethodIfExists(&queue, f_method);
    GetAndCheckMethodIfExists(&queue, g_method);
}

TEST_F(CompilerQueueTest, MaxLimit)
{
    auto klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    main_method->SetHotnessCounter(1);
    f_method->SetHotnessCounter(2);
    g_method->SetHotnessCounter(3);

    RuntimeOptions options;
    constexpr int CompilerQueueMaxLength1 = 100;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       CompilerQueueMaxLength1, options.GetCompilerTaskLifeSpan());

    for (int i = 0; i < 40; i++) {
        queue.AddTask(CompilerTask {main_method, 0});
        queue.AddTask(CompilerTask {f_method, 0});
        queue.AddTask(CompilerTask {g_method, 0});
    }

    // 100 as Max_Limit
    for (int i = 0; i < 100; i++) {
        queue.GetTask();
        // Can not check as the task may expire on an overloaded machine
    }

    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    // check an option
    constexpr int CompilerQueueMaxLength2 = 1;
    CompilerPriorityCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                        CompilerQueueMaxLength2, options.GetCompilerTaskLifeSpan());

    queue2.AddTask(CompilerTask {main_method, 0});
    queue2.AddTask(CompilerTask {f_method, 0});
    queue2.AddTask(CompilerTask {g_method, 0});

    method = queue2.GetTask().GetMethod();
    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

// Testing of AgedCounterQueue

TEST_F(CompilerQueueTest, AgedAddGet)
{
    Class *klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    // Manual range
    main_method->SetHotnessCounter(1000);
    f_method->SetHotnessCounter(1200);
    g_method->SetHotnessCounter(1300);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());
    queue.AddTask(CompilerTask {main_method, 0});
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {g_method, 0});

    GetAndCheckMethodIfExists(&queue, g_method);
    GetAndCheckMethodIfExists(&queue, f_method);
    GetAndCheckMethodIfExists(&queue, main_method);
}

TEST_F(CompilerQueueTest, AgedEqualCounters)
{
    Class *klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    // Manual range
    main_method->SetHotnessCounter(3000);
    g_method->SetHotnessCounter(3000);
    f_method->SetHotnessCounter(3000);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());
    // Add in reversed order, as methods with equal counters will be ordered by timestamp
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {g_method, 0});
    queue.AddTask(CompilerTask {main_method, 0});

    GetAndCheckMethodIfExists(&queue, f_method);
    GetAndCheckMethodIfExists(&queue, g_method);
    GetAndCheckMethodIfExists(&queue, main_method);
}

TEST_F(CompilerQueueTest, AgedExpire)
{
    auto klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    main_method->SetHotnessCounter(1000);
    f_method->SetHotnessCounter(1000);
    g_method->SetHotnessCounter(1000);

    RuntimeOptions options;
    constexpr int CompilerEpochDuration1 = 500;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           CompilerEpochDuration1);
    queue.AddTask(CompilerTask {main_method, 0});
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {g_method, 0});

    WaitForExpire(1600);

    // All tasks should expire after sleep
    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    constexpr int CompilerEpochDuration2 = 1;
    CompilerPriorityAgedCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                            options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                            CompilerEpochDuration2);

    queue2.AddTask(CompilerTask {main_method, 0});
    queue2.AddTask(CompilerTask {f_method, 0});
    queue2.AddTask(CompilerTask {g_method, 0});

    WaitForExpire(5);

    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

TEST_F(CompilerQueueTest, AgedReorder)
{
    auto klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    main_method->SetHotnessCounter(1500);
    f_method->SetHotnessCounter(2000);
    g_method->SetHotnessCounter(3000);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());
    // It is possible, that the first added method is expired, and others are not
    // So, add to queue in reversed order to be sure, that the first method is present anyway
    queue.AddTask(CompilerTask {g_method, 0});
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {main_method, 0});

    // Change the order
    main_method->SetHotnessCounter(6000);
    f_method->SetHotnessCounter(5000);
    g_method->SetHotnessCounter(4000);

    GetAndCheckMethodIfExists(&queue, main_method);
    GetAndCheckMethodIfExists(&queue, f_method);
    GetAndCheckMethodIfExists(&queue, g_method);
}

TEST_F(CompilerQueueTest, AgedMaxLimit)
{
    auto klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    main_method->SetHotnessCounter(1000);
    f_method->SetHotnessCounter(2000);
    g_method->SetHotnessCounter(3000);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());

    for (int i = 0; i < 40; i++) {
        queue.AddTask(CompilerTask {main_method, 0});
        queue.AddTask(CompilerTask {f_method, 0});
        queue.AddTask(CompilerTask {g_method, 0});
    }

    // 100 as Max_Limit
    for (int i = 0; i < 100; i++) {
        queue.GetTask();
        // Can not check as the task may expire on an overloaded machine
    }

    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    // check an option
    constexpr int CompilerQueueMaxLength = 1;
    CompilerPriorityAgedCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                            CompilerQueueMaxLength, options.GetCompilerDeathCounterValue(),
                                            options.GetCompilerEpochDuration());

    queue2.AddTask(CompilerTask {main_method, 0});
    queue2.AddTask(CompilerTask {f_method, 0});
    queue2.AddTask(CompilerTask {g_method, 0});

    method = queue2.GetTask().GetMethod();
    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

TEST_F(CompilerQueueTest, AgedDeathCounter)
{
    auto klass = TestClassPrepare();

    Method *main_method = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(main_method, nullptr);

    Method *f_method = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(f_method, nullptr);

    Method *g_method = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(g_method, nullptr);

    main_method->SetHotnessCounter(10);
    f_method->SetHotnessCounter(20);
    g_method->SetHotnessCounter(30000);

    RuntimeOptions options;
    constexpr int CompilerDeathCounterValue = 50;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), CompilerDeathCounterValue,
                                           options.GetCompilerEpochDuration());

    queue.AddTask(CompilerTask {main_method, 0});
    queue.AddTask(CompilerTask {f_method, 0});
    queue.AddTask(CompilerTask {g_method, 0});

    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, g_method);
    method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}
}  // namespace panda::test
