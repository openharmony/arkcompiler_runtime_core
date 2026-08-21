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

#include "test_common.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/thread.h"

#include <array>
#include <fcntl.h>
#include <set>
#include <tuple>
#include <unistd.h>
#if defined(__linux__)
#include <poll.h>
#endif

namespace ark::tooling::hprof::test {

using common::dump::DumpIdentity;
using common::dump::DumpReason;
#if defined(PANDA_JS_ETS_HYBRID_MODE)
using common::dump::DumpScope;
#endif

constexpr int FORK_MARKER_TIMEOUT_MS = 5000;
constexpr pid_t TEST_DUMP_PID = 1234;
constexpr pid_t TEST_DUMP_TID = 5678;
constexpr uint64_t TEST_DUMP_TIMESTAMP_MS = 1234567890;

class HeapDumpCoordinatorTest : public ::testing::Test {
protected:
    void TearDown() override
    {
        HeapDumpCoordinator::ResetOOMDumpStateForTest();
        HeapDumpCoordinator::GetInstance().GetObjectIdMap().Reset();
    }
};

TEST_F(HeapDumpCoordinatorTest, GetInstanceReturnsSameProfiler)
{
    EXPECT_EQ(&HeapDumpCoordinator::GetInstance(), &HeapDumpCoordinator::GetInstance());
}

TEST_F(HeapDumpCoordinatorTest, TryBeginOOMDumpAllowsOnlyOneCaller)
{
    EXPECT_TRUE(HeapDumpCoordinator::TryBeginOOMDump());
    EXPECT_FALSE(HeapDumpCoordinator::TryBeginOOMDump());
}

TEST_F(HeapDumpCoordinatorTest, DumpBinarySeparateRejectsEmptyParticipants)
{
    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    EXPECT_FALSE(HeapDumpCoordinator::GetInstance().DumpBinarySeparate(request, {}));
}

TEST_F(HeapDumpCoordinatorTest, InProcessDynamicDumpExecutesSessionLifecycle)
{
    FactoryState state;
    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    request.policy.triggerGC = true;

    DumpParticipants participants;
    participants.dynamicDumper = std::make_unique<MockDynamicDumper>(&state);

    EXPECT_TRUE(HeapDumpCoordinator::GetInstance().DumpBinarySeparate(request, std::move(participants)));
    EXPECT_FALSE(state.completeCrossRuntimeGCCalled);
    EXPECT_TRUE(state.triggerGCCalled);
    EXPECT_TRUE(state.prepareSessionCalled);
    EXPECT_TRUE(state.dumpCalled);
}

TEST_F(HeapDumpCoordinatorTest, SuccessfulDumpDoesNotWriteToStderr)
{
    FactoryState state;
    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    request.policy.triggerGC = true;

    DumpParticipants participants;
    participants.dynamicDumper = std::make_unique<MockDynamicDumper>(&state);

    testing::internal::CaptureStderr();
    bool result = HeapDumpCoordinator::GetInstance().DumpBinarySeparate(request, std::move(participants));
    std::string output = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(result);
    EXPECT_TRUE(output.empty());
}

#if defined(PANDA_JS_ETS_HYBRID_MODE)
constexpr uint64_t XREF_JS_ADDRESS_ONE = 0x1000U;
constexpr uint64_t XREF_JS_ADDRESS_TWO = 0x2000U;
constexpr uint64_t XREF_JS_ADDRESS_THREE = 0x3000U;
constexpr uint64_t XREF_ETS_ADDRESS_ONE = 0x4000U;
constexpr uint64_t XREF_ETS_ADDRESS_TWO = 0x5000U;
constexpr uint32_t XREF_JS_NODE_ID_ONE = 101U;
constexpr uint32_t XREF_JS_NODE_ID_TWO = 103U;
constexpr uint32_t XREF_JS_NODE_ID_THREE = 105U;

using SerializedXRef = std::tuple<uint32_t, uint32_t, uint8_t>;

static std::set<SerializedXRef> ParseXRefEdges(const std::vector<uint8_t> &data, const RecordInfo &record)
{
    std::set<SerializedXRef> edges;
    for (uint32_t index = 0; index < record.count; ++index) {
        size_t itemOffset = record.bodyStart + index * XREF_EDGE_BODY_SIZE;
        edges.emplace(ReadU32LE(data, itemOffset + XREF_FROM_OFF), ReadU32LE(data, itemOffset + XREF_TO_OFF),
                      data.at(itemOffset + XREF_DIR_OFF));
    }
    return edges;
}

TEST_F(HeapDumpCoordinatorTest, InProcessHybridDumpRunsXGCAndBothParticipants)
{
    FactoryState dynamicState;
    FactoryState staticState;
    MockSTSVMInterface stsInterface;
    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    request.policy.triggerGC = true;
    request.policy.scope = DumpScope::PROCESS;
    stsInterface.onXRefsCollected_ = [&dynamicState, &staticState]() {
        EXPECT_TRUE(dynamicState.prepareSessionCalled);
        EXPECT_TRUE(staticState.prepareSessionCalled);
        EXPECT_FALSE(dynamicState.dumpCalled);
        EXPECT_FALSE(staticState.dumpCalled);
    };

    DumpParticipants participants;
    participants.dynamicDumper = std::make_unique<MockDynamicDumper>(&dynamicState);
    participants.staticDumper = std::make_unique<MockStaticDumper>(&staticState);
    participants.stsInterface = &stsInterface;

    EXPECT_TRUE(HeapDumpCoordinator::GetInstance().DumpBinarySeparate(request, std::move(participants)));
    EXPECT_TRUE(stsInterface.xgcTriggered_);
    EXPECT_TRUE(dynamicState.completeCrossRuntimeGCCalled);
    EXPECT_FALSE(staticState.completeCrossRuntimeGCCalled);
    EXPECT_TRUE(stsInterface.xrefsCollected_);
    EXPECT_TRUE(dynamicState.dumpCalled);
    EXPECT_TRUE(staticState.dumpCalled);
}

TEST_F(HeapDumpCoordinatorTest, FailedXGCDoesNotRunDynamicCompletion)
{
    FactoryState dynamicState;
    FactoryState staticState;
    MockSTSVMInterface stsInterface;
    stsInterface.xgcResult_ = false;
    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    request.policy.triggerGC = true;

    DumpParticipants participants;
    participants.dynamicDumper = std::make_unique<MockDynamicDumper>(&dynamicState);
    participants.staticDumper = std::make_unique<MockStaticDumper>(&staticState);
    participants.stsInterface = &stsInterface;

    EXPECT_TRUE(HeapDumpCoordinator::GetInstance().DumpBinarySeparate(request, std::move(participants)));
    EXPECT_TRUE(stsInterface.xgcTriggered_);
    EXPECT_FALSE(dynamicState.completeCrossRuntimeGCCalled);
    EXPECT_TRUE(dynamicState.triggerGCCalled);
    EXPECT_TRUE(staticState.triggerGCCalled);
}

class XRefDynamicDumper final : public MockDynamicDumper {
public:
    explicit XRefDynamicDumper(std::unordered_map<uint64_t, uint32_t> nodeIds) : nodeIds_(std::move(nodeIds)) {}

    uint32_t GetNodeId(uint64_t address) const override
    {
        auto it = nodeIds_.find(address);
        return it == nodeIds_.end() ? 0 : it->second;
    }

private:
    std::unordered_map<uint64_t, uint32_t> nodeIds_;
};

class XRefStaticDumper final : public MockStaticDumper {
public:
    explicit XRefStaticDumper(const std::string &path) : output_(path) {}

    common::dump::DumpOutput *GetOutput() override
    {
        return &output_;
    }

private:
    OutputStream output_;
};

TEST_F(HeapDumpCoordinatorTest, ProcessDumpPreservesOneToManyXRefs)
{
    auto &coordinator = HeapDumpCoordinator::GetInstance();
    auto &objectIdMap = coordinator.GetObjectIdMap();
    objectIdMap.Reset();
    auto etsNodeIdOne = objectIdMap.FindOrInsert<Language::STATIC>(XREF_ETS_ADDRESS_ONE);
    auto etsNodeIdTwo = objectIdMap.FindOrInsert<Language::STATIC>(XREF_ETS_ADDRESS_TWO);
    ASSERT_NE(etsNodeIdOne, 0U);
    ASSERT_NE(etsNodeIdTwo, 0U);

    MockSTSVMInterface stsInterface;
    stsInterface.jsToEtsMappings_.emplace(XREF_JS_ADDRESS_ONE, XREF_ETS_ADDRESS_ONE);
    stsInterface.jsToEtsMappings_.emplace(XREF_JS_ADDRESS_TWO, XREF_ETS_ADDRESS_ONE);
    stsInterface.jsToEtsMappings_.emplace(XREF_JS_ADDRESS_ONE, XREF_ETS_ADDRESS_TWO);
    stsInterface.etsToJsMappings_.emplace(XREF_ETS_ADDRESS_ONE, XREF_JS_ADDRESS_ONE);
    stsInterface.etsToJsMappings_.emplace(XREF_ETS_ADDRESS_ONE, XREF_JS_ADDRESS_THREE);
    stsInterface.etsToJsMappings_.emplace(XREF_ETS_ADDRESS_TWO, XREF_JS_ADDRESS_TWO);

    std::string outputPath = CreateTempPath();
    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    request.policy.scope = DumpScope::PROCESS;
    request.policy.triggerGC = false;
    DumpParticipants participants;
    participants.dynamicDumper = std::make_unique<XRefDynamicDumper>(
        std::unordered_map<uint64_t, uint32_t> {{XREF_JS_ADDRESS_ONE, XREF_JS_NODE_ID_ONE},
                                                {XREF_JS_ADDRESS_TWO, XREF_JS_NODE_ID_TWO},
                                                {XREF_JS_ADDRESS_THREE, XREF_JS_NODE_ID_THREE}});
    participants.staticDumper = std::make_unique<XRefStaticDumper>(outputPath);
    participants.stsInterface = &stsInterface;

    EXPECT_TRUE(coordinator.DumpBinarySeparate(request, std::move(participants)));
    auto data = ReadFileBack(outputPath);
    RemoveTempFile(outputPath);
    RecordInfo xrefRecord = ParseRecord(data, 0);
    EXPECT_EQ(xrefRecord.tag, TAG_XREF_EDGE);
    EXPECT_EQ(xrefRecord.count, 5U);
    std::set<SerializedXRef> expected {
        {XREF_JS_NODE_ID_ONE, etsNodeIdOne, XREF_DIR_BIDIR},
        {XREF_JS_NODE_ID_TWO, etsNodeIdOne, XREF_DIR_DYN_TO_STA},
        {XREF_JS_NODE_ID_ONE, etsNodeIdTwo, XREF_DIR_DYN_TO_STA},
        {XREF_JS_NODE_ID_THREE, etsNodeIdOne, XREF_DIR_STA_TO_DYN},
        {XREF_JS_NODE_ID_TWO, etsNodeIdTwo, XREF_DIR_STA_TO_DYN},
    };
    EXPECT_EQ(ParseXRefEdges(data, xrefRecord), expected);
}
#endif

struct BlockingDumpState {
    os::memory::Mutex mutex;
    os::memory::ConditionVariable condition;
    bool executeEntered = false;
    bool releaseExecute = false;
};

class BlockingDumper final : public MockDynamicDumper {
public:
    explicit BlockingDumper(BlockingDumpState *state) : state_(state) {}

    DumpResult Dump() override
    {
        os::memory::LockHolder lock(state_->mutex);
        state_->executeEntered = true;
        state_->condition.SignalAll();
        while (!state_->releaseExecute) {
            state_->condition.Wait(&state_->mutex);
        }
        return MockDynamicDumper::Dump();
    }

private:
    BlockingDumpState *state_;
};

class PrepareSignalDumper final : public MockDynamicDumper {
public:
    explicit PrepareSignalDumper(std::atomic<bool> *prepared) : prepared_(prepared) {}

    void PrepareSession() override
    {
        // Atomic with release order reason: publishes preparation before the observing thread continues.
        prepared_->store(true, std::memory_order_release);
        MockDynamicDumper::PrepareSession();
    }

private:
    std::atomic<bool> *prepared_;
};

TEST_F(HeapDumpCoordinatorTest, ConcurrentDumpsAreSerialized)
{
    BlockingDumpState blockingState;
    bool firstResult = false;
    bool secondResult = false;
    std::atomic<bool> secondStarted {false};
    std::atomic<bool> secondPrepared {false};

    DumpRequest firstRequest;
    firstRequest.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    firstRequest.policy.triggerGC = false;
    DumpParticipants firstParticipants;
    firstParticipants.dynamicDumper = std::make_unique<BlockingDumper>(&blockingState);

    std::thread firstThread([&firstRequest, &firstParticipants, &firstResult]() mutable {
        firstResult = HeapDumpCoordinator::GetInstance().DumpBinarySeparate(firstRequest, std::move(firstParticipants));
    });

    {
        os::memory::LockHolder lock(blockingState.mutex);
        while (!blockingState.executeEntered) {
            blockingState.condition.Wait(&blockingState.mutex);
        }
    }

    DumpRequest secondRequest;
    secondRequest.reason = DumpReason::STATIC_OOM;
    secondRequest.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    secondRequest.policy.triggerGC = false;
    DumpParticipants secondParticipants;
    secondParticipants.dynamicDumper = std::make_unique<PrepareSignalDumper>(&secondPrepared);

    std::thread secondThread([&secondRequest, &secondParticipants, &secondResult, &secondStarted]() mutable {
        // Atomic with release order reason: publishes that the second worker reached the coordinator call.
        secondStarted.store(true, std::memory_order_release);
        secondResult =
            HeapDumpCoordinator::GetInstance().DumpBinarySeparate(secondRequest, std::move(secondParticipants));
    });
    // Atomic with acquire order reason: pairs with the worker's release store before checking serialization.
    while (!secondStarted.load(std::memory_order_acquire)) {
        os::thread::Yield();
    }
    os::thread::NativeSleep(MOCK_DUMPER_DELAY_MS);
    // Atomic with acquire order reason: observes any preparation published by the second worker.
    EXPECT_FALSE(secondPrepared.load(std::memory_order_acquire));

    {
        os::memory::LockHolder lock(blockingState.mutex);
        blockingState.releaseExecute = true;
    }
    blockingState.condition.SignalAll();

    firstThread.join();
    secondThread.join();
    EXPECT_TRUE(firstResult);
    EXPECT_TRUE(secondResult);
    // Atomic with acquire order reason: observes preparation after the worker has completed.
    EXPECT_TRUE(secondPrepared.load(std::memory_order_acquire));
}

#if defined(__linux__)
class FailingStreamDumper final : public MockStaticDumper {
public:
    FailingStreamDumper() : file_(OpenWriteOnlyFile("/dev/full"))
    {
        EXPECT_NE(file_, nullptr);
        stream_ = std::make_unique<OutputStream>(fileno(file_.get()), 1);
    }

    common::dump::DumpOutput *GetOutput() override
    {
        return stream_.get();
    }

private:
    ScopedFile file_;
    std::unique_ptr<OutputStream> stream_;
};

TEST_F(HeapDumpCoordinatorTest, InProcessDumpReportsStaticStreamFailure)
{
    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
    request.policy.triggerGC = false;
    DumpParticipants participants;
    participants.staticDumper = std::make_unique<FailingStreamDumper>();

    EXPECT_FALSE(HeapDumpCoordinator::GetInstance().DumpBinarySeparate(request, std::move(participants)));
}

struct ForkLifecycleMarker {
    bool prepareForkChildCalled = false;
    pid_t outputAcquireProcessId = 0;
    pid_t processId = 0;
    pid_t threadId = 0;
    DumpIdentity outputIdentity {};
};

class ForkMarkerDumper final : public MockDynamicDumper {
public:
    ForkMarkerDumper(int markerFd, DumpIdentity identity) : markerFd_(markerFd), outputIdentity_(identity) {}

    ForkMarkerDumper(const ForkMarkerDumper &) = delete;
    ForkMarkerDumper &operator=(const ForkMarkerDumper &) = delete;
    ForkMarkerDumper(ForkMarkerDumper &&) = delete;
    ForkMarkerDumper &operator=(ForkMarkerDumper &&) = delete;

    ~ForkMarkerDumper() override
    {
        close(markerFd_);
    }

    bool AcquireOutput() override
    {
        outputAcquireProcessId_ = getpid();
        return true;
    }

    void PrepareForkChild() override
    {
        prepareForkChildCalled_ = true;
    }

    DumpResult Dump() override
    {
        DumpResult result = MockDynamicDumper::Dump();
        ForkLifecycleMarker marker;
        marker.prepareForkChildCalled = prepareForkChildCalled_;
        marker.outputAcquireProcessId = outputAcquireProcessId_;
        marker.processId = getpid();
        marker.threadId = static_cast<pid_t>(os::thread::GetCurrentThreadId());
        marker.outputIdentity = outputIdentity_;
        result.success =
            write(markerFd_, &marker, sizeof(marker)) == static_cast<ssize_t>(sizeof(marker)) && result.success;
        return result;
    }

private:
    int markerFd_;
    bool prepareForkChildCalled_ = false;
    pid_t outputAcquireProcessId_ = 0;
    DumpIdentity outputIdentity_ {};
};

TEST_F(HeapDumpCoordinatorTest, ForkModeRunsLifecycleOnForkingChildThread)
{
    std::array<int, 2U> pipeFds = {-1, -1};
    ASSERT_EQ(pipe2(pipeFds.data(), O_CLOEXEC), 0);
    pid_t parentPid = getpid();

    DumpRequest request;
    request.policy.executionMode = DumpExecutionMode::FORK_ONCE;
    request.policy.triggerGC = false;
    request.identity = {TEST_DUMP_PID, TEST_DUMP_TID, TEST_DUMP_TIMESTAMP_MS};
    DumpParticipants participants;
    participants.dynamicDumper = std::make_unique<ForkMarkerDumper>(pipeFds[1], request.identity);

    ASSERT_TRUE(HeapDumpCoordinator::GetInstance().DumpBinarySeparate(request, std::move(participants)));
    pollfd descriptor {pipeFds[0], POLLIN, 0};
    ASSERT_EQ(poll(&descriptor, 1, FORK_MARKER_TIMEOUT_MS), 1);
    ForkLifecycleMarker marker;
    ASSERT_EQ(read(pipeFds[0], &marker, sizeof(marker)), static_cast<ssize_t>(sizeof(marker)));
    EXPECT_TRUE(marker.prepareForkChildCalled);
    EXPECT_EQ(marker.outputAcquireProcessId, parentPid);
    EXPECT_NE(marker.processId, parentPid);
    EXPECT_EQ(marker.processId, marker.threadId);
    EXPECT_EQ(marker.outputIdentity.GetPid(), request.identity.GetPid());
    EXPECT_EQ(marker.outputIdentity.GetTid(), request.identity.GetTid());
    uint8_t trailingByte = 0;
    EXPECT_EQ(read(pipeFds[0], &trailingByte, sizeof(trailingByte)), 0);
    close(pipeFds[0]);
}

#endif

}  // namespace ark::tooling::hprof::test
