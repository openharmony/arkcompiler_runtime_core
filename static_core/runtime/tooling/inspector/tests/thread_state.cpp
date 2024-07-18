/**
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

#include "gtest/gtest.h"
#include "thread_state.h"
#include "runtime_options.h"
#include "runtime.h"
#include "file.h"

// NOLINTBEGIN

namespace ark::tooling::inspector::test {
class ThreadStateTest : public testing::Test {
    void SetUp()
    {
        state.Reset();
    }

protected:
    ThreadState state;
    PtLocation fake = PtLocation("", {}, 0);
    PtLocation location0 = PtLocation("test.abc", panda_file::File::EntityId(1), 0);
    PtLocation location1 = PtLocation("test.abc", panda_file::File::EntityId(1), 1);
    PtLocation location2 = PtLocation("test.abc", panda_file::File::EntityId(1), 2);
    PtLocation location3 = PtLocation("test.abc", panda_file::File::EntityId(1), 3);
};

TEST_F(ThreadStateTest, BreakOnStart)
{
    ASSERT_FALSE(state.IsPaused());
    state.OnSingleStep(fake);
    ASSERT_FALSE(state.IsPaused());

    state.BreakOnStart();
    state.OnSingleStep(fake);
    ASSERT_TRUE(state.IsPaused());
}

TEST_F(ThreadStateTest, PauseAndContinue)
{
    state.Pause();
    state.OnSingleStep(fake);
    ASSERT_TRUE(state.IsPaused());
    state.Continue();
    state.OnSingleStep(fake);
    ASSERT_FALSE(state.IsPaused());

    state.Pause();
    ASSERT_FALSE(state.IsPaused());
    state.Continue();
    state.OnSingleStep(fake);
    ASSERT_FALSE(state.IsPaused());
}

TEST_F(ThreadStateTest, StepInto)
{
    state.Pause();
    state.OnSingleStep(fake);

    std::unordered_set<PtLocation, HashLocation> locs;
    locs.insert(location1);
    locs.insert(location2);

    ASSERT_TRUE(state.IsPaused());
    state.StepInto(locs);
    state.OnSingleStep(location2);
    ASSERT_FALSE(state.IsPaused());
    state.OnSingleStep(location3);
    ASSERT_TRUE(state.IsPaused());
}

TEST_F(ThreadStateTest, ContinueTo)
{
    state.Pause();
    state.OnSingleStep(fake);

    std::unordered_set<PtLocation, HashLocation> locs;
    locs.insert(location1);
    locs.insert(location2);

    ASSERT_TRUE(state.IsPaused());
    state.ContinueTo(locs);
    state.OnSingleStep(location0);
    ASSERT_FALSE(state.IsPaused());
    state.OnSingleStep(location1);
    ASSERT_TRUE(state.IsPaused());
}

TEST_F(ThreadStateTest, StepOut)
{
    state.Pause();
    state.OnSingleStep(fake);

    ASSERT_TRUE(state.IsPaused());
    state.StepOut();
    state.OnSingleStep(location0);
    ASSERT_FALSE(state.IsPaused());
    state.OnSingleStep(location1);
    ASSERT_FALSE(state.IsPaused());
    state.OnSingleStep(location2);
    ASSERT_FALSE(state.IsPaused());
    state.OnFramePop();
    state.OnSingleStep(fake);
    ASSERT_TRUE(state.IsPaused());
}

TEST_F(ThreadStateTest, StepOver)
{
    state.Pause();
    state.OnSingleStep(fake);

    std::unordered_set<PtLocation, HashLocation> locs;
    locs.insert(location1);
    locs.insert(location2);

    ASSERT_TRUE(state.IsPaused());
    state.StepOver(locs);
    state.OnSingleStep(location1);
    ASSERT_FALSE(state.IsPaused());
    state.OnMethodEntry();
    state.OnSingleStep(fake);
    ASSERT_FALSE(state.IsPaused());
    state.OnFramePop();
    state.OnSingleStep(location2);
    ASSERT_FALSE(state.IsPaused());
    state.OnFramePop();
    state.OnSingleStep(fake);
    ASSERT_TRUE(state.IsPaused());
}

TEST_F(ThreadStateTest, Breakpoint)
{
    std::vector<PtLocation> breaks = {location1, location2};
    auto break_id = state.SetBreakpoint(breaks);

    auto hit_breaks = state.GetBreakpointsByLocation(location3);
    ASSERT_TRUE(hit_breaks.empty());
    hit_breaks = state.GetBreakpointsByLocation(location2);
    ASSERT_FALSE(hit_breaks.empty());
    ASSERT_EQ(hit_breaks[0], break_id);

    state.Continue();
    state.OnSingleStep(location0);
    ASSERT_FALSE(state.IsPaused());
    state.OnSingleStep(location2);
    ASSERT_TRUE(state.IsPaused());

    state.SetBreakpointsActive(false);
    state.Continue();
    state.OnSingleStep(location2);
    ASSERT_FALSE(state.IsPaused());
    state.SetBreakpointsActive(true);

    state.Pause();
    state.OnSingleStep(fake);

    state.RemoveBreakpoint(break_id);
    state.Continue();
    state.OnSingleStep(location2);
    ASSERT_FALSE(state.IsPaused());
}

TEST_F(ThreadStateTest, OnException)
{
    ASSERT_FALSE(state.IsPaused());

    state.SetPauseOnExceptions(PauseOnExceptionsState::NONE);
    state.OnException(false);
    ASSERT_FALSE(state.IsPaused());
    state.OnException(true);
    ASSERT_FALSE(state.IsPaused());

    state.SetPauseOnExceptions(PauseOnExceptionsState::CAUGHT);
    state.OnException(true);
    ASSERT_FALSE(state.IsPaused());
    state.OnException(false);
    ASSERT_TRUE(state.IsPaused());
    state.Continue();
    ASSERT_FALSE(state.IsPaused());

    state.SetPauseOnExceptions(PauseOnExceptionsState::UNCAUGHT);
    state.OnException(false);
    ASSERT_FALSE(state.IsPaused());
    state.OnException(true);
    ASSERT_TRUE(state.IsPaused());
    state.Continue();
    ASSERT_FALSE(state.IsPaused());

    state.SetPauseOnExceptions(PauseOnExceptionsState::ALL);
    state.OnException(false);
    ASSERT_TRUE(state.IsPaused());
    state.Continue();
    ASSERT_FALSE(state.IsPaused());
    state.OnException(true);
    ASSERT_TRUE(state.IsPaused());
}

}  // namespace ark::tooling::inspector::test

// NOLINTEND