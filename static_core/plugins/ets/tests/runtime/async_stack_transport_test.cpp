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

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "ets_coroutine.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/logger.h"
#include "runtime/execution/async_stack_snapshot_handle.h"
#include "runtime/execution/job.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_launch.h"
#include "runtime/execution/job_manager.h"
#include "runtime/include/runtime.h"

namespace {

class TestAsyncStackSnapshotHandle final : public ark::AsyncStackSnapshotHandle {
public:
    explicit TestAsyncStackSnapshotHandle(uint32_t id) : id_(id) {}

    ark::AsyncStackSnapshotHandle::Ptr Clone() const override
    {
        return ark::AsyncStackSnapshotHandle::Ptr(new TestAsyncStackSnapshotHandle(id_));
    }

    uint32_t GetId() const
    {
        return id_;
    }

private:
    uint32_t id_;
};

struct ObservationState {
    ark::os::memory::Mutex mutex;
    ark::os::memory::ConditionVariable condition;
    bool done = false;
    ark::Job *job = nullptr;
    uint32_t stackId = 0U;
};

void ObserveCurrentJob(void *param)
{
    auto *state = static_cast<ObservationState *>(param);
    auto *job = ark::Job::GetCurrent();
    auto *stack = ark::Job::GetCurrentAsyncDebuggerStack();
    auto stackId = stack == nullptr ? 0U : static_cast<const TestAsyncStackSnapshotHandle *>(stack)->GetId();

    {
        ark::os::memory::LockHolder lock(state->mutex);
        state->job = job;
        state->stackId = stackId;
        state->done = true;
    }
    state->condition.Signal();
}

}  // namespace

namespace ark::ets::test {

class AsyncStackTransportRuntimeTest : public testing::Test {
public:
    AsyncStackTransportRuntimeTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB environment variable is not set" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Logger::InitializeStdLogging(Logger::Level::ERROR, 0U);
        Runtime::Create(options);
    }

    ~AsyncStackTransportRuntimeTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(AsyncStackTransportRuntimeTest);
    NO_MOVE_SEMANTIC(AsyncStackTransportRuntimeTest);
};

TEST_F(AsyncStackTransportRuntimeTest, LaunchTransfersAndExposesCurrentStack)
{
    constexpr uint32_t stackId = 101U;  // NOLINT(readability-identifier-naming)
    ObservationState state;

    auto *manager = JobExecutionContext::GetCurrent()->GetManager();
    ASSERT_NE(manager, nullptr);
    auto *job = manager->CreateJob(PandaString {"async-stack-test-job"},
                                   Job::NativeEntrypointInfo {&ObserveCurrentJob, &state});
    ASSERT_NE(job, nullptr);

    LaunchParams params(JobPriority::DEFAULT_PRIORITY);
    params.asyncDebuggerStack = AsyncStackSnapshotHandlePtr(new TestAsyncStackSnapshotHandle(stackId));
    ASSERT_EQ(manager->Launch(job, params), LaunchResult::OK);

    ark::os::memory::LockHolder lock(state.mutex);
    while (!state.done) {
        state.condition.Wait(&state.mutex);
    }

    ASSERT_EQ(state.job, job);
    ASSERT_EQ(state.stackId, stackId);
}

}  // namespace ark::ets::test
