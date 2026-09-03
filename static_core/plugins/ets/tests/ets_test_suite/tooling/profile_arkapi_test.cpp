/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "runtime_helpers.h"
#include "tooling/sampler/samples_record.h"
#include "tooling/sampler/sampling_profiler.h"
#include "ets_vm.h"
#include "include/runtime.h"
#include "include/runtime_options.h"
#include "libarkbase/test_utilities.h"
#include "libarkbase/os/filesystem.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/file.h"
#include "libarkfile/method_data_accessor.h"
#include "inspector/debugger_arkapi.h"

namespace ark::test {
// Fills the frame with the first method of the given class so it is resolvable by
// SamplesRecord::BuildStaticFrameInfo (real File* + real EntityId).
static bool FillFrameWithFirstMethodOfClass(const panda_file::File &pf, panda_file::File::EntityId classIdx,
                                            tooling::sampler::SampleInfo::ManagedStackFrameId *frame)
{
    bool filled = false;
    panda_file::ClassDataAccessor cda(pf, classIdx);
    cda.EnumerateMethods([&filled, &pf, frame](const panda_file::MethodDataAccessor &mda) {
        if (!filled) {
            frame->pandaFilePtr = reinterpret_cast<uintptr_t>(&pf);
            frame->fileId = mda.GetMethodId().GetOffset();
            filled = true;
        }
    });
    return filled;
}

// Fills the frame with the first method of the first class of the first loaded panda file, so the
// frame is resolvable by SamplesRecord::BuildStaticFrameInfo (real File* + real EntityId).
static bool FillFrameWithFirstRealMethod(tooling::sampler::SampleInfo::ManagedStackFrameId *frame)
{
    bool filled = false;
    Runtime::GetCurrent()->GetClassLinker()->EnumeratePandaFiles(
        [&filled, frame](const panda_file::File &pf) {
            for (auto classIdx : pf.GetClasses()) {
                if (FillFrameWithFirstMethodOfClass(pf, panda_file::File::EntityId(classIdx), frame)) {
                    filled = true;
                    break;
                }
            }
            return !filled;
        },
        false);
    return filled;
}

class ArkDebugNativeAPITest : public testing::Test {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetLoadRuntimes({"ets"});
        options.SetSamplingProfilerCreate(true);
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "profile_arkapi_test.abc"});
        bool success = Runtime::Create(options);
        ASSERT_TRUE(success) << "Cannot create Runtime";
        std::ofstream file(OUTPUT);
        if (!file.is_open()) {
            std::cerr << "Cannot create file" << std::endl;
            std::abort();
        }
        file.close();
    }

    void TearDown() override
    {
        // Runtime::Destroy stops and destroys the sampler and resets the session buffer
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
        std::filesystem::remove(OUTPUT);
    }
    static constexpr const char *MAIN_FUNC = "profile_arkapi_test.ETSGLOBAL::main";
    static constexpr const char *FILENAME = "profile_arkapi_test.abc";
    static constexpr const char *OUTPUT = "test.cpuprofiler";
    static constexpr uint32_t INTERVAL = 100;
    static constexpr ProfilerOption OPTION {ProfilerType::CPU_PROFILER, INTERVAL};
};

TEST_F(ArkDebugNativeAPITest, StartProfilingWithOutStop)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_TRUE(result);
}

TEST_F(ArkDebugNativeAPITest, StartProfilingRepeated)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT);
    EXPECT_TRUE(result);
    result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_FALSE(result);
}

TEST_F(ArkDebugNativeAPITest, StopProfilingWithOutStartAndExePanda)
{
    bool result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_FALSE(result);
}

TEST_F(ArkDebugNativeAPITest, StopProfilingWithOutStart)
{
    Runtime::GetCurrent()->ExecutePandaFile(FILENAME, MAIN_FUNC, {});
    bool result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_FALSE(result);
}

TEST_F(ArkDebugNativeAPITest, StartAndStopProfilingWithOutExePanda)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_TRUE(result);
    result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_TRUE(result);
    std::ifstream file(OUTPUT);
    EXPECT_TRUE(file.is_open());
    EXPECT_TRUE(file.peek() == std::ifstream::traits_type::eof());
}

TEST_F(ArkDebugNativeAPITest, StopProfilingDoesNotKillSessionMode)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    ASSERT_TRUE(ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false));

    // File-mode stop must refuse a session-mode profiling instead of stopping its sampler,
    // consuming the buffer and dropping the data.
    EXPECT_FALSE(ArkDebugNativeAPI::StopProfiling());
    EXPECT_TRUE(ArkDebugNativeAPI::IsProfilerRunning());
}

TEST_F(ArkDebugNativeAPITest, ArkDebugNativeAPINormalTest)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_TRUE(result);
    Runtime::GetCurrent()->ExecutePandaFile(FILENAME, MAIN_FUNC, {});
    result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_TRUE(result);
}

TEST_F(ArkDebugNativeAPITest, CheckDebugModeTest)
{
    ark::Runtime::GetCurrent()->SetDebugMode(false);
    EXPECT_FALSE(ArkDebugNativeAPI::IsDebugModeEnabled());
    ark::Runtime::GetCurrent()->SetDebugMode(true);
    EXPECT_TRUE(ArkDebugNativeAPI::IsDebugModeEnabled());
}

TEST_F(ArkDebugNativeAPITest, StartProfilerWithOutStop)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    bool result = ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false);
    EXPECT_TRUE(result);
}

TEST_F(ArkDebugNativeAPITest, StartProfilerRepeated)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    bool result = ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false);
    EXPECT_TRUE(result);
    result = ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false);
    EXPECT_FALSE(result);
}

TEST_F(ArkDebugNativeAPITest, StartProfilerDebugAppFalseNonBlocking)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    constexpr int64_t NON_BLOCKING_LIMIT_MS = 1000;
    auto start = std::chrono::steady_clock::now();
    bool result = ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false);
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_TRUE(result);
    EXPECT_LE(elapsed, NON_BLOCKING_LIMIT_MS);
}

TEST_F(ArkDebugNativeAPITest, StartProfilerWithExecutePanda)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    bool result = ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false);
    EXPECT_TRUE(result);
    Runtime::GetCurrent()->ExecutePandaFile(FILENAME, MAIN_FUNC, {});
}

TEST_F(ArkDebugNativeAPITest, StartProfilerSamplerActive)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    bool result = ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false);
    EXPECT_TRUE(result);
    auto *sampler = Runtime::GetCurrent()->GetTools().GetSamplingProfiler();
    ASSERT_NE(sampler, nullptr);
    sampler->Stop();
}

TEST_F(ArkDebugNativeAPITest, StartProfilerIsProfilerRunningTrue)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    ASSERT_TRUE(ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false));
    EXPECT_TRUE(ArkDebugNativeAPI::IsProfilerRunning());
    auto *sampler = Runtime::GetCurrent()->GetTools().GetSamplingProfiler();
    ASSERT_NE(sampler, nullptr);
    sampler->Stop();
    ArkDebugNativeAPI::ResetProfileInfoBuffer();
    EXPECT_FALSE(ArkDebugNativeAPI::IsProfilerRunning());
}

TEST_F(ArkDebugNativeAPITest, StartProfilerGetProfileInfoBufferNotNull)
{
    DebuggerPostTask postTask = [](std::function<void()> &&) {};
    ASSERT_TRUE(ArkDebugNativeAPI::StartProfiler(nullptr, OPTION, postTask, false));
    auto buffer = ArkDebugNativeAPI::GetProfileInfoBuffer();
    EXPECT_NE(buffer, nullptr);
    auto *sampler = Runtime::GetCurrent()->GetTools().GetSamplingProfiler();
    ASSERT_NE(sampler, nullptr);
    sampler->Stop();
    ArkDebugNativeAPI::ResetProfileInfoBuffer();
    EXPECT_EQ(ArkDebugNativeAPI::GetProfileInfoBuffer(), nullptr);
}

TEST_F(ArkDebugNativeAPITest, SamplesRecordMultiThreadProfileInfos)
{
    // Simulate two threads' samples sharing the same SamplesRecord, verify
    // GetAllThreadsProfileInfos returns one ProfileInfo per thread.
    constexpr uint32_t THREAD_COUNT = 2U;
    constexpr uint32_t SAMPLES_PER_THREAD = 3U;
    constexpr uint64_t BASE_TIMESTAMP = 1000U;
    constexpr uint64_t TIMESTAMP_STEP = 100U;
    constexpr uint32_t BASE_OS_TID = 100U;

    tooling::sampler::SamplesRecord record;
    record.SetThreadStartTime(BASE_TIMESTAMP);

    for (uint32_t tid = 1U; tid <= THREAD_COUNT; tid++) {
        for (uint32_t i = 0; i < SAMPLES_PER_THREAD; i++) {
            tooling::sampler::SampleInfo sample;
            sample.threadInfo.threadId = tid;
            sample.threadInfo.osTid = BASE_OS_TID + tid;
            sample.timeStamp = BASE_TIMESTAMP + TIMESTAMP_STEP * (i + 1U);
            sample.stackInfo.managedStackSize = 0;
            record.AddSampleInfo(tid, std::make_unique<tooling::sampler::OwnedSampleInfo>(sample));
        }
    }

    auto profiles = record.GetAllThreadsProfileInfos();
    ASSERT_NE(profiles, nullptr);
    EXPECT_EQ(profiles->size(), THREAD_COUNT);
    for (const auto &p : *profiles) {
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(p->samples.size(), SAMPLES_PER_THREAD);
        EXPECT_EQ(p->timeDeltas.size(), SAMPLES_PER_THREAD);
    }
    auto &first = (*profiles)[0];
    auto &second = (*profiles)[1];
    EXPECT_NE(first->tid, second->tid);
    EXPECT_EQ(first->osTid, BASE_OS_TID + first->tid);
    EXPECT_EQ(second->osTid, BASE_OS_TID + second->tid);
    for (int i = 0; i < first->nodeCount; i++) {
        EXPECT_EQ(first->nodes[i].id, i + 1);
    }
    for (int i = 0; i < second->nodeCount; i++) {
        EXPECT_EQ(second->nodes[i].id, i + 1);
    }
}

TEST_F(ArkDebugNativeAPITest, SamplesRecordSamplesAndDeltasPaired)
{
    // Every sample must produce exactly one timeDelta, including samples whose
    // stack is empty or whose topmost frame is invalid (skipped by continue).
    constexpr uint64_t START_TIME = 1000U;
    constexpr uint64_t EMPTY_STACK_TIMESTAMP = 1100U;
    constexpr uint64_t INVALID_STACK_TIMESTAMP = 1200U;
    constexpr uint64_t MIXED_STACK_TIMESTAMP = 1300U;
    constexpr size_t INVALID_STACK_SIZE = 2U;
    constexpr uint32_t SINGLE_THREAD_BUCKET = 1U;
    constexpr size_t TOTAL_SAMPLES = 3U;

    tooling::sampler::SamplesRecord record;
    record.SetThreadStartTime(START_TIME);

    // Case 1: empty stack.
    tooling::sampler::SampleInfo emptySample;
    emptySample.threadInfo.threadId = SINGLE_THREAD_BUCKET;
    emptySample.timeStamp = EMPTY_STACK_TIMESTAMP;
    emptySample.stackInfo.managedStackSize = 0;
    record.AddSampleInfo(SINGLE_THREAD_BUCKET, std::make_unique<tooling::sampler::OwnedSampleInfo>(emptySample));

    // Case 2: two frames, both with zero pandaFilePtr (invalid, skipped by BuildStackInfoMap).
    tooling::sampler::SampleInfo invalidSample;
    invalidSample.threadInfo.threadId = SINGLE_THREAD_BUCKET;
    invalidSample.timeStamp = INVALID_STACK_TIMESTAMP;
    invalidSample.stackInfo.managedStackSize = INVALID_STACK_SIZE;
    invalidSample.stackInfo.managedStack[0] = {};
    invalidSample.stackInfo.managedStack[1] = {};
    record.AddSampleInfo(SINGLE_THREAD_BUCKET, std::make_unique<tooling::sampler::OwnedSampleInfo>(invalidSample));

    // Case 3: one invalid frame (topmost) followed by one valid frame (base).
    tooling::sampler::SampleInfo mixedSample;
    mixedSample.threadInfo.threadId = SINGLE_THREAD_BUCKET;
    mixedSample.timeStamp = MIXED_STACK_TIMESTAMP;
    mixedSample.stackInfo.managedStackSize = INVALID_STACK_SIZE;
    mixedSample.stackInfo.managedStack[0] = {};
    ASSERT_TRUE(FillFrameWithFirstRealMethod(&mixedSample.stackInfo.managedStack[1]));
    record.AddSampleInfo(SINGLE_THREAD_BUCKET, std::make_unique<tooling::sampler::OwnedSampleInfo>(mixedSample));

    auto profiles = record.GetAllThreadsProfileInfos();
    ASSERT_NE(profiles, nullptr);
    ASSERT_EQ(profiles->size(), 1U);
    auto &p = (*profiles)[0];
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->samples.size(), TOTAL_SAMPLES);
    EXPECT_EQ(p->timeDeltas.size(), TOTAL_SAMPLES);
    for (int nodeId : p->samples) {
        EXPECT_GT(nodeId, 0);
    }
}

}  // namespace ark::test
