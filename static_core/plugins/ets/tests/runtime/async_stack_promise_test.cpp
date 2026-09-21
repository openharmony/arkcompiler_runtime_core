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

#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <string>
#include <utility>
#include <vector>

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/dfx/ets_async_stack_snapshot_handle.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/types/ets_async_stack_snapshot.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_promise_async_stack_snapshot_queue.h"
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"
#include "runtime/execution/job.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/include/runtime.h"

namespace {

struct Observation {
    ani_int value = 0;
    bool hasStack = false;
    bool hasSnapshotType = false;
    bool hasCorrectOwner = false;
    bool hasSnapshot = false;
    std::string description;
    std::vector<std::string> descriptions;
    std::vector<std::string> frameFunctionNames;
    std::vector<std::vector<std::string>> segmentFrameFunctionNames;
    uint64_t generation = 0U;
    uint32_t segmentCount = 0U;
    uint32_t frameCount = 0U;
    int32_t workerId = -1;
    bool isMainWorker = true;
    uint32_t jobId = 0U;
};

struct ObservationQueue {
    ark::os::memory::Mutex mutex;
    ark::os::memory::ConditionVariable condition;
    std::deque<Observation> observations;
};

ObservationQueue *g_observationQueue = nullptr;
std::atomic<int32_t> g_submissionWorkerId {-1};

void FillJobObservation(Observation &observation, ark::JobExecutionContext *executionCtx)
{
    auto *currentJob = ark::Job::GetCurrent();
    observation.jobId = currentJob == nullptr ? 0U : currentJob->GetId();
    if (executionCtx == nullptr || executionCtx->GetWorker() == nullptr) {
        return;
    }
    observation.workerId = executionCtx->GetWorker()->GetId();
    observation.isMainWorker = executionCtx->GetWorker()->IsMainWorker();
}

void FillSegmentDescription(Observation &observation, ark::ets::EtsExecutionContext *executionCtx,
                            ark::ets::EtsAsyncStackSegment *segment, uint32_t segmentIndex)
{
    auto *description = segment->GetDescription(executionCtx);
    if (description != nullptr) {
        std::string descriptionText = description->GetUtf8().c_str();  // NOLINT(readability-redundant-string-cstr)
        observation.descriptions.push_back(descriptionText);
        if (segmentIndex == 0U) {
            observation.description = std::move(descriptionText);
        }
    }
}

void FillSegmentFrames(Observation &observation, ark::ets::EtsExecutionContext *executionCtx,
                       ark::ets::EtsAsyncStackSegment *segment, uint32_t segmentIndex)
{
    observation.segmentFrameFunctionNames.resize(static_cast<size_t>(segmentIndex) + 1U);
    auto *frames = segment->GetFrames(executionCtx);
    if (frames == nullptr) {
        return;
    }
    const uint32_t frameCount = frames->GetLength();
    observation.frameCount += frameCount;
    for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
        auto *frame = ark::ets::EtsAsyncStackFrame::FromEtsObject(frames->Get(frameIndex));
        auto *functionName = frame->GetFunctionName(executionCtx);
        if (functionName != nullptr) {
            observation.segmentFrameFunctionNames[segmentIndex].push_back(functionName->GetUtf8().c_str());
            observation.frameFunctionNames.emplace_back(functionName->GetUtf8().c_str());
        }
    }
}

void FillSegmentObservation(Observation &observation, ark::ets::EtsExecutionContext *executionCtx,
                            ark::ets::EtsAsyncStackSegment *segment, uint32_t segmentIndex)
{
    observation.generation = static_cast<uint64_t>(segment->GetGeneration());
    FillSegmentDescription(observation, executionCtx, segment, segmentIndex);
    FillSegmentFrames(observation, executionCtx, segment, segmentIndex);
}

void FillSnapshotObservation(Observation &observation, ark::ets::EtsExecutionContext *executionCtx,
                             ark::ets::EtsAsyncStackSnapshot *snapshot)
{
    observation.generation = static_cast<uint64_t>(snapshot->GetGeneration());
    auto *segments = snapshot->GetSegments(executionCtx);
    if (segments == nullptr) {
        return;
    }

    observation.segmentCount = segments->GetLength();
    for (uint32_t idx = 0; idx < observation.segmentCount; ++idx) {
        auto *segment = ark::ets::EtsAsyncStackSegment::FromEtsObject(segments->Get(idx));
        FillSegmentObservation(observation, executionCtx, segment, idx);
    }
}

ani_int ObserveAsyncStack([[maybe_unused]] ani_env *env, ani_int value)
{
    ark::ets::ani::ScopedManagedCodeFix scope(env);
    auto *coroutine = ark::ets::EtsCoroutine::GetCurrent();
    auto *executionCtx = ark::ets::EtsExecutionContext::FromMT(coroutine);
    auto *vm = coroutine->GetPandaVM();
    auto *stack = ark::Job::GetCurrentAsyncDebuggerStack();

    Observation observation;
    observation.value = value;
    auto *jobExecutionContext = ark::JobExecutionContext::GetCurrent();
    FillJobObservation(observation, jobExecutionContext);
    observation.hasStack = stack != nullptr;
    if (stack != nullptr) {
        observation.hasSnapshotType =
            stack->GetSnapshotTypeId() == ark::ets::EtsAsyncStackSnapshotHandle::SnapshotTypeId();
        observation.hasCorrectOwner = stack->GetOpaqueOwner() == vm;
        if (observation.hasSnapshotType && observation.hasCorrectOwner) {
            auto *snapshot = static_cast<ark::ets::EtsAsyncStackSnapshot *>(stack->GetOpaqueSnapshot());
            observation.hasSnapshot = snapshot != nullptr;
            if (snapshot != nullptr) {
                FillSnapshotObservation(observation, executionCtx, snapshot);
            }
        }
    }

    auto *queue = g_observationQueue;
    if (queue != nullptr) {
        ark::os::memory::LockHolder lock(queue->mutex);
        queue->observations.push_back(std::move(observation));
        queue->condition.SignalAll();
    }
    return value;
}

ani_int InspectPromiseQueue(ani_env *env, ani_object promiseObject)
{
    ark::ets::ani::ScopedManagedCodeFix scope(env);
    auto *promise = ark::ets::EtsPromise::FromEtsObject(scope.ToInternalType(promiseObject));
    auto *executionCtx = ark::ets::EtsExecutionContext::GetCurrent();
    ark::ets::EtsHandleScope handleScope(executionCtx);
    ark::ets::EtsHandle<ark::ets::EtsPromise> promiseHandle(executionCtx, promise);
    uint32_t result = 0;
    if (promise->GetQueueSize() == 1) {
        result |= 1U;
    }
    if (promise->GetCallbackQueue(executionCtx) != nullptr &&
        promise->GetCallbackQueue(executionCtx)->Get(0) != nullptr) {
        result |= 2U;
    }
    if (promise->GetWorkerDomainQueue(executionCtx) != nullptr) {
        result |= 4U;
    }
    auto snapshotHandle = ark::ets::EtsPromiseAsyncStackSnapshotQueue::CreateHandleAt(executionCtx, promiseHandle, 0U);
    if (snapshotHandle != nullptr) {
        result |= 8U;
    }
    return static_cast<ani_int>(result);
}

ani_int RecordSubmissionWorker([[maybe_unused]] ani_env *env)
{
    ark::ets::ani::ScopedManagedCodeFix scope(env);
    auto *jobExecutionContext = ark::JobExecutionContext::GetCurrent();
    const auto workerId = jobExecutionContext == nullptr || jobExecutionContext->GetWorker() == nullptr
                              ? -1
                              : jobExecutionContext->GetWorker()->GetId();
    // Atomic with relaxed order reason: only the worker id value is transferred; ordering constraints are not required.
    g_submissionWorkerId.store(workerId, std::memory_order_relaxed);
    return workerId;
}

}  // namespace

namespace ark::ets::test {

class AsyncStackPromiseTest : public ani::testing::AniTest {
protected:
    void SetUp() override
    {
        ani::testing::AniTest::SetUp();
        BindNativeFunctions();
        vm_ = EtsCoroutine::GetCurrent()->GetPandaVM();
        Runtime::GetCurrent()->SetDebugMode(true);
        constexpr uint32_t maxAsyncDepth = 4U;  // NOLINT(readability-identifier-naming)
        vm_->SetAsyncDebuggerMaxAsyncDepth(maxAsyncDepth);
        config_ = vm_->GetAsyncDebuggerConfig();
        ASSERT_TRUE(config_.IsCaptureEnabled());
        g_observationQueue = &observationQueue_;
        // Atomic with relaxed order reason: test setup resets the value before concurrent work starts.
        g_submissionWorkerId.store(-1, std::memory_order_relaxed);
    }

    void TearDown() override
    {
        {
            ark::os::memory::LockHolder lock(observationQueue_.mutex);
            ASSERT_TRUE(observationQueue_.observations.empty());
        }
        g_observationQueue = nullptr;
        ani::testing::AniTest::TearDown();
    }

    Observation WaitObservation()
    {
        constexpr uint64_t waitTimeoutMs = 10000U;  // NOLINT(readability-identifier-naming)
        ark::os::memory::LockHolder lock(observationQueue_.mutex);
        while (observationQueue_.observations.empty()) {
            if (observationQueue_.condition.TimedWait(&observationQueue_.mutex, waitTimeoutMs)) {
                return Observation {};
            }
        }
        auto observation = std::move(observationQueue_.observations.front());
        observationQueue_.observations.pop_front();
        return observation;
    }

    void AssertObservation(const Observation &observation, ani_int expectedValue,
                           const std::vector<std::string> &expectedDescriptions)
    {
        AssertObservation(observation, expectedValue, expectedDescriptions, config_.generation);
    }

    void AssertObservation(const Observation &observation, ani_int expectedValue,
                           const std::vector<std::string> &expectedDescriptions, uint64_t expectedGeneration)
    {
        ASSERT_EQ(observation.value, expectedValue);
        ASSERT_TRUE(observation.hasStack);
        ASSERT_TRUE(observation.hasSnapshotType);
        ASSERT_TRUE(observation.hasCorrectOwner);
        ASSERT_TRUE(observation.hasSnapshot);
        ASSERT_EQ(observation.descriptions, expectedDescriptions);
        if (!expectedDescriptions.empty()) {
            ASSERT_EQ(observation.description, expectedDescriptions.front());
        }
        ASSERT_EQ(observation.generation, expectedGeneration);
        ASSERT_EQ(observation.segmentCount, expectedDescriptions.size());
        ASSERT_EQ(observation.segmentFrameFunctionNames.size(), observation.segmentCount);
        ASSERT_EQ(observation.frameFunctionNames.size(), observation.frameCount);
    }

    void AssertFramesContain(const Observation &observation, const std::string &expectedFunctionName)
    {
        ASSERT_FALSE(observation.frameFunctionNames.empty());
        bool found = false;
        for (const auto &functionName : observation.frameFunctionNames) {
            found |= functionName.find(expectedFunctionName) != std::string::npos;
        }
        ASSERT_TRUE(found) << observation.frameFunctionNames.front();
    }

    void AssertSegmentFramesContain(const Observation &observation, uint32_t segmentIndex,
                                    const std::string &expectedFunctionName)
    {
        ASSERT_LT(static_cast<size_t>(segmentIndex), observation.segmentFrameFunctionNames.size());
        const auto &functionNames = observation.segmentFrameFunctionNames[segmentIndex];
        ASSERT_FALSE(functionNames.empty()) << "segment " << segmentIndex << " has no frames";
        bool found = false;
        for (const auto &functionName : functionNames) {
            found |= functionName.find(expectedFunctionName) != std::string::npos;
        }
        ASSERT_TRUE(found) << "segment " << segmentIndex << " does not contain " << expectedFunctionName
                           << ", actual: " << functionNames.front();
    }

    void AssertSegmentFramesInOrder(const Observation &observation, uint32_t segmentIndex,
                                    const std::vector<std::string> &expectedFunctionNames)
    {
        ASSERT_LT(static_cast<size_t>(segmentIndex), observation.segmentFrameFunctionNames.size());
        const auto &functionNames = observation.segmentFrameFunctionNames[segmentIndex];
        ASSERT_GE(functionNames.size(), expectedFunctionNames.size());
        size_t expectedIdx = 0U;
        for (size_t idx = 0; idx < functionNames.size() && expectedIdx < expectedFunctionNames.size(); ++idx) {
            if (functionNames[idx].find(expectedFunctionNames[expectedIdx]) != std::string::npos) {
                ++expectedIdx;
            }
        }
        ASSERT_EQ(expectedIdx, expectedFunctionNames.size())
            << "segment " << segmentIndex
            << " does not contain frames in order, actual first frame: " << functionNames.front();
    }

    void AssertFramesDoNotContain(const Observation &observation, const std::string &unexpectedFunctionName)
    {
        for (const auto &functionName : observation.frameFunctionNames) {
            ASSERT_TRUE(functionName.find(unexpectedFunctionName) == std::string::npos)
                << "unexpected function " << unexpectedFunctionName << " found as " << functionName;
        }
    }

    static constexpr const char *moduleName = "AsyncStackPromiseTest";  // NOLINT(readability-identifier-naming)
    static constexpr ani_int ALREADY_SETTLED_VALUE = 1;
    static constexpr ani_int ALREADY_REJECTED_CATCH_VALUE = 14;
    static constexpr ani_int ALREADY_SETTLED_FINALLY_VALUE = 15;
    static constexpr ani_int PENDING_THEN_VALUE = 2;
    static constexpr ani_int PENDING_CATCH_VALUE = 16;
    static constexpr ani_int PENDING_FINALLY_VALUE = 17;
    static constexpr ani_int FIRST_AWAIT_VALUE = 3;
    static constexpr ani_int FIRST_CHAIN_AWAIT_VALUE = 4;
    static constexpr ani_int SECOND_CHAIN_AWAIT_VALUE = 5;
    static constexpr ani_int FIRST_GENERATION_AWAIT_VALUE = 6;
    static constexpr ani_int SECOND_GENERATION_AWAIT_VALUE = 7;
    static constexpr ani_int THIRD_GENERATION_AWAIT_VALUE = 8;
    static constexpr ani_int WORKER_AWAIT_VALUE = 9;
    static constexpr ani_int WORKER_THEN_VALUE = 13;
    static constexpr ani_int NESTED_INNER_AWAIT_VALUE = 10;
    static constexpr ani_int NESTED_OUTER_FIRST_AWAIT_VALUE = 11;
    static constexpr ani_int NESTED_OUTER_AFTER_AWAIT_VALUE = 12;
    static constexpr ani_int BEFORE_FIRST_AWAIT_VALUE = 18;
    static constexpr ani_int AFTER_FIRST_AWAIT_VALUE = 19;
    static constexpr ani_int BEFORE_PRE_AWAIT_INNER_VALUE = 20;
    static constexpr ani_int AFTER_PRE_AWAIT_INNER_VALUE = 21;
    static constexpr ani_int BEFORE_DEEP_CHAIN_C_VALUE = 22;
    static constexpr ani_int AFTER_DEEP_CHAIN_C_VALUE = 23;
    static constexpr ani_int MAX_FRAMES_AWAIT_VALUE = 24;

    ObservationQueue observationQueue_;      // NOLINT(misc-non-private-member-variables-in-classes)
    PandaEtsVM *vm_ = nullptr;               // NOLINT(misc-non-private-member-variables-in-classes)
    AsyncDebuggerConfigSnapshot config_ {};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    void BindNativeFunctions()
    {
        ani_module module {};
        ASSERT_EQ(env_->FindModule(moduleName, &module), ANI_OK);
        std::array functions = {
            ani_native_function {"observeAsyncStack", "i:i", reinterpret_cast<void *>(ObserveAsyncStack)},
            ani_native_function {"inspectPromiseQueue", "C{std.core.Object}:i",
                                 reinterpret_cast<void *>(InspectPromiseQueue)},
            ani_native_function {"recordSubmissionWorker", ":i", reinterpret_cast<void *>(RecordSubmissionWorker)},
        };
        ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_OK);
    }
};

class AsyncStackPromiseStackfulTest : public AsyncStackPromiseTest {
private:
    std::vector<ani_option> GetExtraAniOptions() override
    {
        ani_option coroutineImpl = {"--ext:coroutine-impl=stackful", nullptr};
        return std::vector<ani_option> {coroutineImpl};
    }
};

class AsyncStackPromiseWorkerHandoffTest : public AsyncStackPromiseTest {
private:
    std::vector<ani_option> GetExtraAniOptions() override
    {
        ani_option coroutineWorkersCount = {"--ext:coroutine-workers-count=4", nullptr};
        return std::vector<ani_option> {coroutineWorkersCount};
    }
};

TEST_F(AsyncStackPromiseTest, AlreadySettledThenTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "alreadySettled"), 0);
    auto observation = WaitObservation();

    AssertObservation(observation, ALREADY_SETTLED_VALUE, {"promise.then"});
    AssertSegmentFramesInOrder(observation, 0U, {"alreadySettled"});
}

TEST_F(AsyncStackPromiseTest, AlreadyRejectedCatchTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "alreadyRejectedCatch"), 0);
    auto observation = WaitObservation();

    AssertObservation(observation, ALREADY_REJECTED_CATCH_VALUE, {"promise.catch"});
    AssertSegmentFramesInOrder(observation, 0U, {"alreadyRejectedCatch"});
}

TEST_F(AsyncStackPromiseTest, AlreadySettledFinallyTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "alreadySettledFinally"), 0);
    auto observation = WaitObservation();

    AssertObservation(observation, ALREADY_SETTLED_FINALLY_VALUE, {"promise.finally"});
    AssertSegmentFramesInOrder(observation, 0U, {"alreadySettledFinally"});
}

TEST_F(AsyncStackPromiseTest, PendingThenQueuesAndTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitPending"), 0);
    ASSERT_TRUE(CallEtsFunction<ani_boolean>(moduleName, "pendingQueueHasSnapshot"));

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePending"), 0);
    auto observation = WaitObservation();

    AssertObservation(observation, PENDING_THEN_VALUE, {"promise.then"});
    AssertSegmentFramesInOrder(observation, 0U, {"submitPending"});
}

TEST_F(AsyncStackPromiseTest, PendingCatchQueuesAndTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitPendingCatch"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "rejectPendingCatch"), 0);

    auto observation = WaitObservation();
    AssertObservation(observation, PENDING_CATCH_VALUE, {"promise.catch"});
    AssertSegmentFramesInOrder(observation, 0U, {"submitPendingCatch"});
}

TEST_F(AsyncStackPromiseTest, PendingFinallyQueuesAndTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitPendingFinally"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePendingFinally"), 0);

    auto observation = WaitObservation();
    AssertObservation(observation, PENDING_FINALLY_VALUE, {"promise.finally"});
    AssertSegmentFramesInOrder(observation, 0U, {"submitPendingFinally"});
}

TEST_F(AsyncStackPromiseTest, FirstAwaitTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitFirst"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitFirst"), 0);

    auto observation = WaitObservation();
    AssertObservation(observation, FIRST_AWAIT_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(observation, 1U, {"submitAwaitFirst"});
}

TEST_F(AsyncStackPromiseTest, MultipleAwaitsInheritSnapshotChain)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitChain"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainFirst"), 0);

    auto firstObservation = WaitObservation();
    AssertObservation(firstObservation, FIRST_CHAIN_AWAIT_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(firstObservation, 1U, {"submitAwaitChain"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainSecond"), 0);
    auto secondObservation = WaitObservation();
    AssertObservation(secondObservation, SECOND_CHAIN_AWAIT_VALUE, {"await", "await", "await"});
    AssertSegmentFramesInOrder(secondObservation, 2U, {"submitAwaitChain"});
}

TEST_F(AsyncStackPromiseStackfulTest, FirstAwaitTransfersSnapshot)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitFirst"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitFirst"), 0);

    auto observation = WaitObservation();
    AssertObservation(observation, FIRST_AWAIT_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(observation, 1U, {"submitAwaitFirst"});
}

TEST_F(AsyncStackPromiseStackfulTest, MultipleAwaitsInheritSnapshotChain)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitChain"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainFirst"), 0);

    auto firstObservation = WaitObservation();
    AssertObservation(firstObservation, FIRST_CHAIN_AWAIT_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(firstObservation, 1U, {"submitAwaitChain"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainSecond"), 0);
    auto secondObservation = WaitObservation();
    AssertObservation(secondObservation, SECOND_CHAIN_AWAIT_VALUE, {"await", "await", "await"});
    AssertSegmentFramesInOrder(secondObservation, 2U, {"submitAwaitChain"});
}

TEST_F(AsyncStackPromiseTest, GenerationMismatchRestartsAwaitChain)
{
    const auto oldConfig = config_;

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitGenerationMismatch"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainFirst"), 0);
    auto firstObservation = WaitObservation();
    AssertObservation(firstObservation, FIRST_GENERATION_AWAIT_VALUE, {"await", "await"}, oldConfig.generation);

    vm_->SetAsyncDebuggerEnabled(false);
    vm_->SetAsyncDebuggerEnabled(true);
    config_ = vm_->GetAsyncDebuggerConfig();
    ASSERT_NE(config_.generation, oldConfig.generation);

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationSecond"), 0);
    auto secondObservation = WaitObservation();
    AssertObservation(secondObservation, SECOND_GENERATION_AWAIT_VALUE, {"await", "await", "await"},
                      oldConfig.generation);

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationThird"), 0);
    auto thirdObservation = WaitObservation();
    AssertObservation(thirdObservation, THIRD_GENERATION_AWAIT_VALUE, {"await"}, config_.generation);
}

TEST_F(AsyncStackPromiseStackfulTest, GenerationMismatchRestartsAwaitChain)
{
    const auto oldConfig = config_;

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitGenerationMismatch"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainFirst"), 0);
    auto firstObservation = WaitObservation();
    AssertObservation(firstObservation, FIRST_GENERATION_AWAIT_VALUE, {"await", "await"}, oldConfig.generation);

    vm_->SetAsyncDebuggerEnabled(false);
    vm_->SetAsyncDebuggerEnabled(true);
    config_ = vm_->GetAsyncDebuggerConfig();
    ASSERT_NE(config_.generation, oldConfig.generation);

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationSecond"), 0);
    auto secondObservation = WaitObservation();
    AssertObservation(secondObservation, SECOND_GENERATION_AWAIT_VALUE, {"await", "await", "await"},
                      oldConfig.generation);

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationThird"), 0);
    auto thirdObservation = WaitObservation();
    AssertObservation(thirdObservation, THIRD_GENERATION_AWAIT_VALUE, {"await"}, config_.generation);
}

TEST_F(AsyncStackPromiseTest, DisabledCaptureClearsCurrentAwaitStack)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitGenerationMismatch"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainFirst"), 0);
    auto firstObservation = WaitObservation();
    AssertObservation(firstObservation, FIRST_GENERATION_AWAIT_VALUE, {"await", "await"});

    vm_->SetAsyncDebuggerEnabled(false);
    ASSERT_FALSE(vm_->GetAsyncDebuggerConfig().IsCaptureEnabled());

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationSecond"), 0);
    auto secondObservation = WaitObservation();
    AssertObservation(secondObservation, SECOND_GENERATION_AWAIT_VALUE, {"await", "await", "await"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationThird"), 0);
    auto thirdObservation = WaitObservation();
    ASSERT_EQ(thirdObservation.value, THIRD_GENERATION_AWAIT_VALUE);
    ASSERT_FALSE(thirdObservation.hasStack);
    ASSERT_FALSE(thirdObservation.hasSnapshotType);
    ASSERT_FALSE(thirdObservation.hasCorrectOwner);
    ASSERT_FALSE(thirdObservation.hasSnapshot);
}

TEST_F(AsyncStackPromiseStackfulTest, DisabledCaptureClearsCurrentAwaitStack)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitGenerationMismatch"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitChainFirst"), 0);
    auto firstObservation = WaitObservation();
    AssertObservation(firstObservation, FIRST_GENERATION_AWAIT_VALUE, {"await", "await"});

    vm_->SetAsyncDebuggerEnabled(false);
    ASSERT_FALSE(vm_->GetAsyncDebuggerConfig().IsCaptureEnabled());

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationSecond"), 0);
    auto secondObservation = WaitObservation();
    AssertObservation(secondObservation, SECOND_GENERATION_AWAIT_VALUE, {"await", "await", "await"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitGenerationThird"), 0);
    auto thirdObservation = WaitObservation();
    ASSERT_EQ(thirdObservation.value, THIRD_GENERATION_AWAIT_VALUE);
    ASSERT_FALSE(thirdObservation.hasStack);
    ASSERT_FALSE(thirdObservation.hasSnapshotType);
    ASSERT_FALSE(thirdObservation.hasCorrectOwner);
    ASSERT_FALSE(thirdObservation.hasSnapshot);
}

TEST_F(AsyncStackPromiseWorkerHandoffTest, AwaitSurvivesWorkerHandoff)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitOnWorker"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitOnWorker"), 0);

    auto observation = WaitObservation();
    AssertObservation(observation, WORKER_AWAIT_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(observation, 1U, {"startAwaitOnWorker"});
    ASSERT_FALSE(observation.isMainWorker);
    ASSERT_GE(observation.workerId, 0);
}

TEST_F(AsyncStackPromiseWorkerHandoffTest, PromiseThenSnapshotSurvivesWorkerHandoff)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitThenOnWorker"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveThenOnWorker"), 0);

    // Atomic with relaxed order reason: only the worker id value is read; ordering constraints are not required.
    const auto submissionWorkerId = g_submissionWorkerId.load(std::memory_order_relaxed);
    auto observation = WaitObservation();
    AssertObservation(observation, WORKER_THEN_VALUE, {"promise.then"});
    AssertSegmentFramesInOrder(observation, 0U, {"startThenOnWorker"});
    ASSERT_GE(submissionWorkerId, 0);
    ASSERT_FALSE(observation.isMainWorker);
    ASSERT_NE(observation.workerId, submissionWorkerId);
}

TEST_F(AsyncStackPromiseTest, NestedAwaitRestoresCurrentJobAndParent)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitNestedAwait"), 0);
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveNestedOuterAwait"), 0);

    auto outerFirstObservation = WaitObservation();
    AssertObservation(outerFirstObservation, NESTED_OUTER_FIRST_AWAIT_VALUE, {"await", "await"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveNestedInnerAwait"), 0);
    auto innerObservation = WaitObservation();
    ASSERT_NE(innerObservation.jobId, outerFirstObservation.jobId);
    AssertObservation(innerObservation, NESTED_INNER_AWAIT_VALUE, {"await", "await", "await", "await"});
    AssertSegmentFramesInOrder(innerObservation, 1U, {"nestedOuterAwait"});
    AssertSegmentFramesInOrder(innerObservation, 3U, {"submitNestedAwait"});

    auto outerAfterNestedObservation = WaitObservation();
    AssertObservation(outerAfterNestedObservation, NESTED_OUTER_AFTER_AWAIT_VALUE, {"await", "await", "await"});
    ASSERT_EQ(outerAfterNestedObservation.jobId, outerFirstObservation.jobId);
}

TEST_F(AsyncStackPromiseTest, AsyncCallSiteVisibleBeforeFirstAwait)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitObservationBeforeFirstAwait"), 0);
    auto beforeObservation = WaitObservation();
    AssertObservation(beforeObservation, BEFORE_FIRST_AWAIT_VALUE, {"await"});
    AssertSegmentFramesInOrder(beforeObservation, 0U, {"submitAwaitObservationBeforeFirstAwait"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitObservationBeforeFirstAwait"), 0);
    auto afterObservation = WaitObservation();
    AssertObservation(afterObservation, AFTER_FIRST_AWAIT_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(afterObservation, 1U, {"submitAwaitObservationBeforeFirstAwait"});
}

TEST_F(AsyncStackPromiseStackfulTest, AsyncCallSiteVisibleBeforeFirstAwait)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitAwaitObservationBeforeFirstAwait"), 0);
    auto beforeObservation = WaitObservation();
    AssertObservation(beforeObservation, BEFORE_FIRST_AWAIT_VALUE, {"await"});
    AssertSegmentFramesInOrder(beforeObservation, 0U, {"submitAwaitObservationBeforeFirstAwait"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolveAwaitObservationBeforeFirstAwait"), 0);
    auto afterObservation = WaitObservation();
    AssertObservation(afterObservation, AFTER_FIRST_AWAIT_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(afterObservation, 1U, {"submitAwaitObservationBeforeFirstAwait"});
}

TEST_F(AsyncStackPromiseTest, PreAwaitAsyncCallPreservesDeepAncestorChain)
{
    vm_->SetAsyncDebuggerMaxAsyncDepth(6U);
    config_ = vm_->GetAsyncDebuggerConfig();

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitPreAwaitDeepChain"), 0);
    auto innerBeforeAwait = WaitObservation();
    AssertObservation(innerBeforeAwait, BEFORE_PRE_AWAIT_INNER_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(innerBeforeAwait, 0U, {"preAwaitOuter"});
    AssertSegmentFramesInOrder(innerBeforeAwait, 1U, {"submitPreAwaitDeepChain"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePreAwaitInner"), 0);
    auto innerAfterAwait = WaitObservation();
    AssertObservation(innerAfterAwait, AFTER_PRE_AWAIT_INNER_VALUE, {"await", "await", "await"});
    AssertSegmentFramesInOrder(innerAfterAwait, 1U, {"preAwaitOuter"});
    AssertSegmentFramesInOrder(innerAfterAwait, 2U, {"submitPreAwaitDeepChain"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePreAwaitThen"), 0);
    auto callbackBeforeAwait = WaitObservation();
    AssertObservation(callbackBeforeAwait, BEFORE_DEEP_CHAIN_C_VALUE,
                      {"await", "promise.then", "await", "await", "await"});
    AssertSegmentFramesInOrder(callbackBeforeAwait, 0U, {"preAwaitThenCallback"});
    AssertSegmentFramesInOrder(callbackBeforeAwait, 1U, {"preAwaitInner"});
    AssertSegmentFramesInOrder(callbackBeforeAwait, 3U, {"preAwaitOuter"});
    AssertSegmentFramesInOrder(callbackBeforeAwait, 4U, {"submitPreAwaitDeepChain"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePreAwaitC"), 0);
    auto callbackAfterAwait = WaitObservation();
    AssertObservation(callbackAfterAwait, AFTER_DEEP_CHAIN_C_VALUE,
                      {"await", "await", "promise.then", "await", "await", "await"});
    AssertSegmentFramesInOrder(callbackAfterAwait, 1U, {"preAwaitThenCallback"});
    AssertSegmentFramesInOrder(callbackAfterAwait, 2U, {"preAwaitInner"});
    AssertSegmentFramesInOrder(callbackAfterAwait, 4U, {"preAwaitOuter"});
    AssertSegmentFramesInOrder(callbackAfterAwait, 5U, {"submitPreAwaitDeepChain"});
}

TEST_F(AsyncStackPromiseTest, MaxAsyncDepthBoundsInheritedChain)
{
    vm_->SetAsyncDebuggerMaxAsyncDepth(2U);
    config_ = vm_->GetAsyncDebuggerConfig();

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitPreAwaitDeepChain"), 0);
    auto beforeAwait = WaitObservation();
    AssertObservation(beforeAwait, BEFORE_PRE_AWAIT_INNER_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(beforeAwait, 0U, {"preAwaitOuter"});
    AssertSegmentFramesInOrder(beforeAwait, 1U, {"submitPreAwaitDeepChain"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePreAwaitInner"), 0);
    auto afterAwait = WaitObservation();
    AssertObservation(afterAwait, AFTER_PRE_AWAIT_INNER_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(afterAwait, 1U, {"preAwaitOuter"});
    AssertFramesDoNotContain(afterAwait, "submitPreAwaitDeepChain");

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePreAwaitThen"), 0);
    auto callbackBeforeAwait = WaitObservation();
    AssertObservation(callbackBeforeAwait, BEFORE_DEEP_CHAIN_C_VALUE, {"await", "promise.then"});
    AssertSegmentFramesInOrder(callbackBeforeAwait, 0U, {"preAwaitThenCallback"});
    AssertSegmentFramesInOrder(callbackBeforeAwait, 1U, {"preAwaitInner"});
    AssertFramesDoNotContain(callbackBeforeAwait, "preAwaitOuter");

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "resolvePreAwaitC"), 0);
    auto callbackAfterAwait = WaitObservation();
    AssertObservation(callbackAfterAwait, AFTER_DEEP_CHAIN_C_VALUE, {"await", "await"});
    AssertSegmentFramesInOrder(callbackAfterAwait, 1U, {"preAwaitThenCallback"});
    AssertFramesDoNotContain(callbackAfterAwait, "preAwaitInner");
}

TEST_F(AsyncStackPromiseTest, MaxFramesPerSegmentBoundsCallSiteCapture)
{
    ASSERT_TRUE(vm_->SetAsyncDebuggerMaxFramesPerSegment(1U));
    config_ = vm_->GetAsyncDebuggerConfig();

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "submitMaxFramesChain"), 0);
    auto observation = WaitObservation();
    AssertObservation(observation, MAX_FRAMES_AWAIT_VALUE, {"await"});
    ASSERT_EQ(observation.segmentFrameFunctionNames[0U].size(), 1U);
    AssertSegmentFramesInOrder(observation, 0U, {"maxFramesCaller"});

    ASSERT_EQ(CallEtsFunction<ani_int>(moduleName, "waitMaxFramesChain"), 0);
}

}  // namespace ark::ets::test
