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

#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
#define ASYNC_STACK_DFX_AVAILABLE 1
#endif

namespace {

constexpr uint64_t TEST_STACK_ID = 0x1234U;

#if defined(ASYNC_STACK_DFX_AVAILABLE)
constexpr size_t EXPECTED_STACK_DEPTH = 16U;

uint64_t g_nextCollectedStackId = TEST_STACK_ID;
uint64_t g_lastCollectType = 0U;
size_t g_lastCollectDepth = 0U;
uint32_t g_collectCalls = 0U;
uint32_t g_setCalls = 0U;
uint32_t g_getCalls = 0U;
thread_local uint64_t g_submitterStackId = 0U;
#endif

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

#if defined(ASYNC_STACK_DFX_AVAILABLE)
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
#endif

}  // namespace

#if defined(ASYNC_STACK_DFX_AVAILABLE)
extern "C" uint64_t DfxCollectStackWithDepth(uint64_t type, size_t depth)
{
    g_lastCollectType = type;
    g_lastCollectDepth = depth;
    g_collectCalls++;
    return g_nextCollectedStackId;
}

extern "C" void DfxSetSubmitterStackId(uint64_t stackId)
{
    g_submitterStackId = stackId;
    g_setCalls++;
}

extern "C" uint64_t DfxGetSubmitterStackId()
{
    g_getCalls++;
    return g_submitterStackId;
}
#endif

namespace ark::test {

#if defined(ASYNC_STACK_DFX_AVAILABLE)
TEST(AsyncStackTest, HelperForwardsCollectTypeAndDepth)
{
    ResetDfxState();

    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.CheckLoadDfxAsyncStackFunc();
    auto stackId = asyncStackHelper.CollectAsyncStack(dfx::StackType::STACK_TYPE_LAUNCH);

    ASSERT_EQ(stackId, TEST_STACK_ID);
    ASSERT_EQ(g_collectCalls, 1U);
    ASSERT_EQ(g_lastCollectType, 1ULL << 26U);
    ASSERT_EQ(g_lastCollectDepth, EXPECTED_STACK_DEPTH);
}

TEST(AsyncStackTest, HelperForwardsSetAndGetStackId)
{
    ResetDfxState();

    constexpr uint64_t stackId = 0x5678U;
    ark::dfx::AsyncStackHelper asyncStackHelper;
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

    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.SetStackId(TEST_STACK_ID);
    ASSERT_EQ(asyncStackHelper.GetStackId(), TEST_STACK_ID);

    asyncStackHelper.SetStackId(0U);
    ASSERT_EQ(asyncStackHelper.GetStackId(), 0U);
}

TEST(AsyncStackTest, HelperUsesThreadLocalSubmitterStackId)
{
    ResetDfxState();

    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.SetStackId(TEST_STACK_ID);
    ASSERT_EQ(asyncStackHelper.GetStackId(), TEST_STACK_ID);

    std::thread thread([]() {
        ark::dfx::AsyncStackHelper asyncStackHelper;
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

    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    auto stackId = asyncStackHelper.CollectAsyncStack(dfx::StackType::STACK_TYPE_LAUNCH);
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
    ark::dfx::AsyncStackHelper asyncStackHelper;
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
    ark::dfx::AsyncStackHelper asyncStackHelper;
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
    ark::dfx::AsyncStackHelper asyncStackHelper;
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
    ark::dfx::AsyncStackHelper asyncStackHelper;
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
#else
TEST(AsyncStackTest, HelperStubIsNoop)
{
    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    asyncStackHelper.CheckLoadDfxAsyncStackFunc();

    ASSERT_EQ(asyncStackHelper.CollectAsyncStack(dfx::StackType::STACK_TYPE_LAUNCH), 0U);
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
#endif

TEST(AsyncStackTest, ScopeAcceptsEmptyInputs)
{
    ark::dfx::AsyncStackHelper asyncStackHelper;
    InitializeAsyncStackHelper(asyncStackHelper);
    {
        dfx::AsyncStackScope scope(static_cast<Job *>(nullptr), asyncStackHelper);
    }
}

}  // namespace ark::test
