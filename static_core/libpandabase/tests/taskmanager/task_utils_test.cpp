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

#include "libpandabase/taskmanager/utils/sp_mc_lock_free_queue.h"
#include <gtest/gtest.h>
#include <thread>

namespace ark::taskmanager {

class TaskUtilityTest : public testing::Test {
public:
    static constexpr size_t ELEMENTS_MAX_COUNT = 10'000UL;
    TaskUtilityTest() = default;
    ~TaskUtilityTest() override = default;

    NO_COPY_SEMANTIC(TaskUtilityTest);
    NO_MOVE_SEMANTIC(TaskUtilityTest);
};

TEST_F(TaskUtilityTest, SPMCLockFreeQueueSPSCTest)
{
    internal::SPMCLockFreeQueue<int> queue;
    std::atomic_size_t consumerCounter = {0};
    std::atomic_size_t producerCounter = {0};

    std::thread producer([&queue, &producerCounter]() {
        for (size_t i = 0; i < ELEMENTS_MAX_COUNT; i++) {
            queue.Push(i);
            producerCounter += i;
        }
    });

    std::thread consumer([&queue, &consumerCounter]() {
        size_t id = queue.RegisterConsumer();
        for (size_t i = 0; i < ELEMENTS_MAX_COUNT;) {
            auto optVal = queue.Pop(id);
            if (optVal.has_value()) {
                i++;
                consumerCounter += optVal.value();
            }
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(producerCounter, consumerCounter);
}

TEST_F(TaskUtilityTest, SPMCLockFreeQueueSPMCTest)
{
    internal::SPMCLockFreeQueue<int> queue;
    std::atomic_size_t consumerCounter = {0};
    std::atomic_size_t producerCounter = {0};

    std::thread producer([&queue, &producerCounter]() {
        for (size_t i = 1; i < ELEMENTS_MAX_COUNT; i++) {
            queue.Push(i);
            producerCounter += i;
        }
    });

    std::vector<std::thread> consumers;
    constexpr size_t CONSUMER_COUNT = 5;
    std::atomic_size_t countOfPoppedTasks = 0;

    auto consumerFunc = [&queue, &consumerCounter, &countOfPoppedTasks]() {
        size_t id = queue.RegisterConsumer();

        while (countOfPoppedTasks < ELEMENTS_MAX_COUNT - 1) {
            auto optVal = queue.Pop(id);
            if (optVal.has_value()) {
                countOfPoppedTasks++;
                consumerCounter += optVal.value();
            }
        }
    };

    for (size_t i = 0; i < CONSUMER_COUNT; i++) {
        consumers.emplace_back(consumerFunc);
    }

    producer.join();
    for (auto &consumer : consumers) {
        consumer.join();
    }
    ASSERT_EQ(producerCounter, consumerCounter);
}

}  // namespace ark::taskmanager
