/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "runtime/execution/dfx/async_stack_helper.h"
#include "runtime/execution/dfx/async_stack_scope.h"
#include "runtime/execution/job.h"

#include <gtest/gtest.h>
#include <cstddef>
#include <thread>

namespace {

constexpr uint64_t TEST_STACK_ID = 0x1234U;
constexpr size_t EXPECTED_STACK_DEPTH = 16U;

uint64_t g_nextCollectedStackId = TEST_STACK_ID;
uint64_t g_lastCollectType = 0U;
size_t g_lastCollectDepth = 0U;
uint32_t g_collectCalls = 0U;
uint32_t g_setCalls = 0U;
uint32_t g_getCalls = 0U;
thread_local uint64_t g_submitterStackId = 0U;

ark::Job CreateTestJob(uint64_t asyncStackId)
{
    ark::Job job(ark::PandaString {}, 1U, ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                 ark::Job::Type::MUTATOR, false);
    job.SetAsyncStackID(asyncStackId);
    return job;
}

void InitializeAsyncStackHelper(ark::dfx::AsyncStackHelper &asyncStackHelper)
{
    asyncStackHelper.Initialize();
}

void ResetDfxState()
{
    g_nextCollectedStackId = TEST_STACK_ID;
    g_lastCollectType = 0U;
    g_lastCollectDepth = 0U;
    g_collectCalls = 0U;
    g_setCalls = 0U;
    g_getCalls = 0U;
    g_submitterStackId = 0U;
}

class TestAsyncStackHelper final : public ark::dfx::AsyncStackHelper {
public:
    void Initialize() override {}

    bool CheckLoadDfxAsyncStackFunc() const override
    {
        return true;
    }

    uint64_t CollectAsyncStack([[maybe_unused]] ark::dfx::StackType stackType, size_t depth) const override
    {
        ASSERT(stackType == ark::dfx::StackType::STACK_TYPE_LAUNCH);
        g_lastCollectType = 1ULL << 26U;
        g_lastCollectDepth = depth;
        g_collectCalls++;
        return g_nextCollectedStackId;
    }

    void SetStackId(uint64_t stackId) const override
    {
        g_submitterStackId = stackId;
        g_setCalls++;
    }

    uint64_t GetStackId() const override
    {
        g_getCalls++;
        return g_submitterStackId;
    }
};

}  // namespace

namespace ark::test {

TEST(AsyncStackTest, HelperForwardsCollectTypeAndDepth)
{
    ResetDfxState();

    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.CheckLoadDfxAsyncStackFunc();
    auto stackId = asyncStackHelper.CollectAsyncStack(dfx::StackType::STACK_TYPE_LAUNCH,
                                                      dfx::AsyncStackHelper::DEFAULT_STACK_DEPTH);

    ASSERT_EQ(stackId, TEST_STACK_ID);
    ASSERT_EQ(g_collectCalls, 1U);
    ASSERT_EQ(g_lastCollectType, 1ULL << 26U);
    ASSERT_EQ(g_lastCollectDepth, EXPECTED_STACK_DEPTH);
}

TEST(AsyncStackTest, HelperForwardsSetAndGetStackId)
{
    ResetDfxState();

    constexpr uint64_t stackId = 0x5678U;
    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.SetStackId(stackId);
    auto currentId = asyncStackHelper.GetStackId();

    ASSERT_EQ(currentId, stackId);
    ASSERT_EQ(g_setCalls, 1U);
    ASSERT_EQ(g_getCalls, 1U);
}

TEST(AsyncStackTest, HelperCanResetStackIdToZero)
{
    ResetDfxState();

    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.SetStackId(TEST_STACK_ID);
    ASSERT_EQ(asyncStackHelper.GetStackId(), TEST_STACK_ID);

    asyncStackHelper.SetStackId(0U);
    ASSERT_EQ(asyncStackHelper.GetStackId(), 0U);
}

TEST(AsyncStackTest, HelperUsesThreadLocalSubmitterStackId)
{
    ResetDfxState();

    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.SetStackId(TEST_STACK_ID);
    ASSERT_EQ(asyncStackHelper.GetStackId(), TEST_STACK_ID);

    std::thread thread([]() {
        TestAsyncStackHelper asyncStackHelper;
        InitializeAsyncStackHelper(asyncStackHelper);
        ASSERT_EQ(asyncStackHelper.GetStackId(), 0U);
        asyncStackHelper.SetStackId(0x9999U);
        ASSERT_EQ(asyncStackHelper.GetStackId(), 0x9999U);
    });
    thread.join();

    ASSERT_EQ(asyncStackHelper.GetStackId(), TEST_STACK_ID);
}

TEST(AsyncStackTest, HelperAcceptsZeroCollectedStackId)
{
    ResetDfxState();
    g_nextCollectedStackId = 0U;

    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    auto stackId = asyncStackHelper.CollectAsyncStack(dfx::StackType::STACK_TYPE_LAUNCH,
                                                      dfx::AsyncStackHelper::DEFAULT_STACK_DEPTH);
    asyncStackHelper.SetStackId(stackId);

    ASSERT_EQ(stackId, 0U);
    ASSERT_EQ(asyncStackHelper.GetStackId(), 0U);
}

TEST(AsyncStackTest, ScopeForJobSetsAndRestoresStackId)
{
    ResetDfxState();
    constexpr uint64_t previousStackId = 0x1111U;
    constexpr uint64_t jobStackId = 0x2222U;
    g_submitterStackId = previousStackId;
    auto job = CreateTestJob(jobStackId);
    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);

    {
        dfx::AsyncStackScope scope(&job, asyncStackHelper);
        ASSERT_EQ(g_submitterStackId, jobStackId);
        ASSERT_EQ(g_getCalls, 1U);
        ASSERT_EQ(g_setCalls, 1U);
    }

    ASSERT_EQ(g_submitterStackId, previousStackId);
    ASSERT_EQ(g_setCalls, 2U);
}

TEST(AsyncStackTest, ScopeForJobIgnoresZeroStackId)
{
    ResetDfxState();
    constexpr uint64_t previousStackId = 0x3333U;
    g_submitterStackId = previousStackId;
    auto job = CreateTestJob(0U);
    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);

    {
        dfx::AsyncStackScope scope(&job, asyncStackHelper);
        ASSERT_EQ(g_submitterStackId, previousStackId);
    }

    ASSERT_EQ(g_getCalls, 0U);
    ASSERT_EQ(g_setCalls, 0U);
}

TEST(AsyncStackTest, ScopeForJobKeepsCurrentIdWhenItAlreadyMatches)
{
    ResetDfxState();
    constexpr uint64_t jobStackId = 0x4444U;
    g_submitterStackId = jobStackId;
    auto job = CreateTestJob(jobStackId);
    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);

    {
        dfx::AsyncStackScope scope(&job, asyncStackHelper);
        ASSERT_EQ(g_submitterStackId, jobStackId);
        ASSERT_EQ(g_getCalls, 1U);
        ASSERT_EQ(g_setCalls, 0U);
    }

    ASSERT_EQ(g_submitterStackId, jobStackId);
    ASSERT_EQ(g_setCalls, 0U);
}

TEST(AsyncStackTest, NestedJobScopesRestoreStackIdsInOrder)
{
    ResetDfxState();
    constexpr uint64_t outerPreviousStackId = 0x5555U;
    constexpr uint64_t outerJobStackId = 0x6666U;
    constexpr uint64_t innerJobStackId = 0x7777U;
    g_submitterStackId = outerPreviousStackId;
    auto outerJob = CreateTestJob(outerJobStackId);
    auto innerJob = CreateTestJob(innerJobStackId);
    TestAsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);

    {
        dfx::AsyncStackScope outerScope(&outerJob, asyncStackHelper);
        ASSERT_EQ(g_submitterStackId, outerJobStackId);
        {
            dfx::AsyncStackScope innerScope(&innerJob, asyncStackHelper);
            ASSERT_EQ(g_submitterStackId, innerJobStackId);
        }
        ASSERT_EQ(g_submitterStackId, outerJobStackId);
    }

    ASSERT_EQ(g_submitterStackId, outerPreviousStackId);
    ASSERT_EQ(g_setCalls, 4U);
}

TEST(AsyncStackTest, HelperStubIsNoop)
{
    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.CheckLoadDfxAsyncStackFunc();

    ASSERT_EQ(asyncStackHelper.CollectAsyncStack(dfx::StackType::STACK_TYPE_LAUNCH,
                                                 dfx::AsyncStackHelper::DEFAULT_STACK_DEPTH),
              0U);
    asyncStackHelper.SetStackId(TEST_STACK_ID);
    ASSERT_EQ(asyncStackHelper.GetStackId(), 0U);
}

TEST(AsyncStackTest, ScopeWithStubIsNoop)
{
    auto job = CreateTestJob(TEST_STACK_ID);
    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);

    {
        dfx::AsyncStackScope scope(&job, asyncStackHelper);
        ASSERT_EQ(asyncStackHelper.GetStackId(), 0U);
    }

    ASSERT_EQ(asyncStackHelper.GetStackId(), 0U);
}

TEST(AsyncStackTest, ScopeAcceptsEmptyInputs)
{
    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    {
        dfx::AsyncStackScope scope(static_cast<Job *>(nullptr), asyncStackHelper);
    }
}

}  // namespace ark::test
