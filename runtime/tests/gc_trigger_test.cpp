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
#include "runtime/include/runtime.h"
#include "runtime/mem/gc/gc_trigger.h"

namespace panda::test {
class GCTriggerTest : public testing::Test {
public:
    GCTriggerTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcTriggerType("adaptive-heap-trigger");

        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~GCTriggerTest()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    [[nodiscard]] mem::GCAdaptiveTriggerHeap *CreateGCTriggerHeap() const
    {
        return new mem::GCAdaptiveTriggerHeap(nullptr, nullptr, MIN_HEAP_SIZE,
                                              mem::GCTriggerHeap::DEFAULT_PERCENTAGE_THRESHOLD,
                                              DEFAULT_INCREASE_MULTIPLIER, MIN_EXTRA_HEAP_SIZE, MAX_EXTRA_HEAP_SIZE);
    }

    static size_t GetTargetFootprint(const mem::GCAdaptiveTriggerHeap *trigger)
    {
        // Atomic with relaxed order reason: simple getter for test
        return trigger->target_footprint_.load(std::memory_order_relaxed);
    }

protected:
    static constexpr size_t MIN_HEAP_SIZE = 8_MB;
    static constexpr size_t MIN_EXTRA_HEAP_SIZE = 1_MB;
    static constexpr size_t MAX_EXTRA_HEAP_SIZE = 8_MB;
    static constexpr uint32_t DEFAULT_INCREASE_MULTIPLIER = 3U;

    MTManagedThread *thread_ = nullptr;
};

TEST_F(GCTriggerTest, ThresholdTest)
{
    static constexpr size_t BEFORE_HEAP_SIZE = 50_MB;
    static constexpr size_t CURRENT_HEAP_SIZE = MIN_HEAP_SIZE;
    static constexpr size_t FIRST_THRESHOLD = 2U * MIN_HEAP_SIZE;
    static constexpr size_t HEAP_SIZE_AFTER_BASE_TRIGGER = (BEFORE_HEAP_SIZE + CURRENT_HEAP_SIZE) / 2U;
    auto *trigger = CreateGCTriggerHeap();
    GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);

    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);

    ASSERT_EQ(GetTargetFootprint(trigger), FIRST_THRESHOLD);

    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);
    ASSERT_EQ(GetTargetFootprint(trigger), HEAP_SIZE_AFTER_BASE_TRIGGER);
    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);
    ASSERT_EQ(GetTargetFootprint(trigger), HEAP_SIZE_AFTER_BASE_TRIGGER);
    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);
    ASSERT_EQ(GetTargetFootprint(trigger), HEAP_SIZE_AFTER_BASE_TRIGGER);
    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);

    // Check that we could to avoid locale triggering
    ASSERT_EQ(GetTargetFootprint(trigger), FIRST_THRESHOLD);

    delete trigger;
}
}  // namespace panda::test
