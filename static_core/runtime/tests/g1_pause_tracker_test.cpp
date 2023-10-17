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

#include <gtest/gtest.h>
#include "libpandabase/utils/time.h"
#include "runtime/mem/gc/g1/g1_pause_tracker.h"

namespace panda::mem {
class G1PauseTrackerTest : public testing::Test {
public:
    G1PauseTrackerTest() : now_us_(panda::time::GetCurrentTimeInMillis()) {}

    int64_t Now()
    {
        return now_us_;
    }

    bool AddPauseTime(G1PauseTracker &pause_tracker, int64_t pause_time_us)
    {
        auto result = pause_tracker.AddPause(now_us_, now_us_ + pause_time_us);
        AddTime(pause_time_us);
        return result;
    }

    void AddTime(int64_t time_us)
    {
        now_us_ += time_us;
    }

private:
    int64_t now_us_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(G1PauseTrackerTest, MaxGcPauseRequiresNewTimeSlice)
{
    G1PauseTracker pause_tracker(20, 10);
    AddPauseTime(pause_tracker, 1'000);
    ASSERT_EQ(10'000, pause_tracker.MinDelayBeforePauseInMicros(Now(), 10'000));
    ASSERT_EQ(10'000, pause_tracker.MinDelayBeforeMaxPauseInMicros(Now()));
    AddTime(4'000);
    ASSERT_EQ(6'000, pause_tracker.MinDelayBeforePauseInMicros(Now(), 10'000));
    ASSERT_EQ(6'000, pause_tracker.MinDelayBeforeMaxPauseInMicros(Now()));
}

TEST_F(G1PauseTrackerTest, MinDelayBeforePauseInMicrosQueueIsFull)
{
    G1PauseTracker pause_tracker(20, 10);
    AddPauseTime(pause_tracker, 1'000);
    AddTime(2'000);
    AddPauseTime(pause_tracker, 2'000);
    AddTime(5'000);
    AddPauseTime(pause_tracker, 1'000);
    AddTime(5'000);
    AddPauseTime(pause_tracker, 3'000);
    AddTime(1'000);
    ASSERT_EQ(0, pause_tracker.MinDelayBeforePauseInMicros(Now(), 6'000));
    ASSERT_EQ(4'000, pause_tracker.MinDelayBeforePauseInMicros(Now(), 7'000));
}

TEST_F(G1PauseTrackerTest, MinDelayBeforePauseInMicrosQueueIsNotFull)
{
    G1PauseTracker pause_tracker(20, 10);
    for (int i = 0; i < 16; i++) {
        ASSERT_TRUE(AddPauseTime(pause_tracker, 1'000));
        AddTime(1'000);
    }
    ASSERT_EQ(1'000, pause_tracker.MinDelayBeforePauseInMicros(Now(), 2'000));
    ASSERT_EQ(2'000, pause_tracker.MinDelayBeforePauseInMicros(Now(), 3'000));
    ASSERT_EQ(9'000, pause_tracker.MinDelayBeforePauseInMicros(Now(), 10'000));
    ASSERT_EQ(4'000, pause_tracker.MinDelayBeforePauseInMicros(Now() + 5'000, 10'000));
    ASSERT_EQ(9'000, pause_tracker.MinDelayBeforeMaxPauseInMicros(Now()));
    ASSERT_EQ(4'000, pause_tracker.MinDelayBeforeMaxPauseInMicros(Now() + 5'000));
}

TEST_F(G1PauseTrackerTest, ExceedMaxGcTime)
{
    G1PauseTracker pause_tracker(20, 10);
    ASSERT_TRUE(AddPauseTime(pause_tracker, 3'000));
    AddTime(3'000);
    ASSERT_TRUE(AddPauseTime(pause_tracker, 3'000));
    AddTime(3'000);
    ASSERT_TRUE(AddPauseTime(pause_tracker, 4'000));
    AddTime(3'999);
    ASSERT_FALSE(AddPauseTime(pause_tracker, 1));
}

TEST_F(G1PauseTrackerTest, NotExceedMaxGcTime)
{
    G1PauseTracker pause_tracker(20, 10);
    ASSERT_TRUE(AddPauseTime(pause_tracker, 3'000));
    AddTime(3'000);
    ASSERT_TRUE(AddPauseTime(pause_tracker, 3'000));
    AddTime(3'000);
    ASSERT_TRUE(AddPauseTime(pause_tracker, 4'000));
    AddTime(4'000);
    ASSERT_TRUE(AddPauseTime(pause_tracker, 1));
}
// NOLINTEND(readability-magic-numbers)
}  // namespace panda::mem
