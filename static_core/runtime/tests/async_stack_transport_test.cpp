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

#include "runtime/execution/async_stack_snapshot_handle.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/execution/job.h"
#include "runtime/execution/job_launch.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <utility>

namespace {

struct HandleCounters {
    uint32_t clones = 0U;
    uint32_t destructors = 0U;
};

class TestAsyncStackSnapshotHandle final : public ark::AsyncStackSnapshotHandle {
public:
    explicit TestAsyncStackSnapshotHandle(HandleCounters *counters) : counters_(counters) {}

    ~TestAsyncStackSnapshotHandle() override
    {
        ++counters_->destructors;
    }

    ark::AsyncStackSnapshotHandle::Ptr Clone() const override
    {
        ++counters_->clones;
        return ark::AsyncStackSnapshotHandle::Ptr(new TestAsyncStackSnapshotHandle(counters_));
    }

private:
    HandleCounters *counters_;
};

ark::Job CreateJobWithoutEntrypoint()
{
    return ark::Job {ark::PandaString {},     1U,   ark::Job::EntrypointInfo {}, ark::JobPriority::DEFAULT_PRIORITY,
                     ark::Job::Type::MUTATOR, false};
}

}  // namespace

namespace ark::test {

TEST(AsyncStackTransportTest, HandleCloneCreatesIndependentOwner)
{
    HandleCounters counters;
    {
        auto handle = ark::AsyncStackSnapshotHandle::Ptr(new TestAsyncStackSnapshotHandle(&counters));
        auto clone = handle->Clone();

        ASSERT_NE(clone, nullptr);
        ASSERT_NE(clone.get(), handle.get());
    }

    ASSERT_EQ(counters.clones, 1U);
    ASSERT_EQ(counters.destructors, 2U);
}

TEST(AsyncStackTransportTest, LaunchParamsAndJobMoveOwnership)
{
    HandleCounters counters;
    auto handle = ark::AsyncStackSnapshotHandle::Ptr(new TestAsyncStackSnapshotHandle(&counters));
    auto *handlePointer = handle.get();

    ark::LaunchParams params(ark::JobPriority::DEFAULT_PRIORITY);
    params.asyncDebuggerStack = std::move(handle);
    ASSERT_EQ(params.asyncDebuggerStack.get(), handlePointer);

    auto job = CreateJobWithoutEntrypoint();
    job.SetAsyncDebuggerStack(params.TakeAsyncDebuggerStack());
    ASSERT_EQ(params.asyncDebuggerStack, nullptr);
    ASSERT_EQ(job.GetAsyncDebuggerStack(), handlePointer);

    auto clone = job.CloneAsyncDebuggerStack();
    ASSERT_NE(clone, nullptr);
    ASSERT_NE(clone.get(), handlePointer);

    ark::Job movedJob(std::move(job));
    ASSERT_EQ(job.GetAsyncDebuggerStack(), nullptr);
    ASSERT_EQ(movedJob.GetAsyncDebuggerStack(), handlePointer);

    auto takenHandle = movedJob.TakeAsyncDebuggerStack();
    ASSERT_EQ(takenHandle.get(), handlePointer);
    ASSERT_EQ(movedJob.GetAsyncDebuggerStack(), nullptr);
}

class AsyncStackCoreVmTest : public testing::Test {
public:
    AsyncStackCoreVmTest()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("epsilon");
        options.SetCompilerEnableJit(false);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        EXPECT_TRUE(Runtime::Create(options));
    }

    ~AsyncStackCoreVmTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(AsyncStackCoreVmTest);
    NO_MOVE_SEMANTIC(AsyncStackCoreVmTest);
};

TEST_F(AsyncStackCoreVmTest, CorePandaVmAsyncStackQueryIsUnsupported)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    ASSERT_NE(vm, nullptr);
    vm->SetAsyncDebuggerMaxAsyncDepth(4U);

    ASSERT_EQ(vm->CloneCurrentAsyncDebuggerStack(), nullptr);
}

TEST(AsyncStackTransportTest, ConfigGenerationChangesOnlyOnEnable)
{
    ark::AsyncDebuggerConfig config;

    auto snapshot = config.GetSnapshot();
    ASSERT_FALSE(snapshot.IsCaptureEnabled());
    ASSERT_EQ(snapshot.maxAsyncDepth, 0U);
    ASSERT_EQ(snapshot.maxFramesPerSegment, ark::AsyncDebuggerConfig::DEFAULT_MAX_FRAMES_PER_SEGMENT);
    ASSERT_EQ(snapshot.generation, 0U);

    config.SetMaxAsyncDepth(8U);
    snapshot = config.GetSnapshot();
    ASSERT_TRUE(snapshot.IsCaptureEnabled());
    ASSERT_EQ(snapshot.maxAsyncDepth, 8U);
    ASSERT_EQ(snapshot.generation, 1U);

    config.SetMaxAsyncDepth(4U);
    snapshot = config.GetSnapshot();
    ASSERT_EQ(snapshot.maxAsyncDepth, 4U);
    ASSERT_EQ(snapshot.generation, 1U);

    config.SetMaxAsyncDepth(0U);
    snapshot = config.GetSnapshot();
    ASSERT_FALSE(snapshot.IsCaptureEnabled());
    ASSERT_EQ(snapshot.maxAsyncDepth, 0U);
    ASSERT_EQ(snapshot.generation, 1U);

    config.SetMaxAsyncDepth(2U);
    snapshot = config.GetSnapshot();
    ASSERT_EQ(snapshot.maxAsyncDepth, 2U);
    ASSERT_EQ(snapshot.generation, 2U);

    ASSERT_FALSE(config.SetMaxFramesPerSegment(0U));
    ASSERT_TRUE(config.SetMaxFramesPerSegment(32U));
    snapshot = config.GetSnapshot();
    ASSERT_EQ(snapshot.maxFramesPerSegment, 32U);
}

TEST(AsyncStackTransportTest, EnableUsesStoredDepth)
{
    ark::AsyncDebuggerConfig config;
    config.SetMaxAsyncDepth(5U);
    config.SetEnabled(false);
    config.SetEnabled(true);

    auto snapshot = config.GetSnapshot();
    ASSERT_TRUE(snapshot.IsCaptureEnabled());
    ASSERT_EQ(snapshot.maxAsyncDepth, 5U);
    ASSERT_EQ(snapshot.generation, 2U);
}

}  // namespace ark::test
