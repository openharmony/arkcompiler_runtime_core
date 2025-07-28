/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "unit_test.h"
#include "panda_runner.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/jit/profiling_saver.h"

namespace ark::test {
class ProfilingRunnerTest : public testing::Test {};

static constexpr auto SOURCE = R"(
.function void main() <static> {
    movi v0, 0x2
    movi v1, 0x0
    jump_label_1: lda v1
    jge v0, jump_label_0
    call.short foo
    inci v1, 0x1
    jmp jump_label_1
    jump_label_0: return.void
}

.function i32 foo() <static> {
    movi v0, 0x64
    movi v1, 0x0
    mov v2, v1
    jump_label_3: lda v2
    jge v0, jump_label_0
    lda v2
    modi 0x3
    jnez jump_label_1
    lda v1
    addi 0x2
    sta v3
    mov v1, v3
    jmp jump_label_2
    jump_label_1: lda v1
    addi 0x3
    sta v3
    mov v1, v3
    jump_label_2: inci v2, 0x1
    jmp jump_label_3
    jump_label_0: lda v1
    return
}
)";

TEST_F(ProfilingRunnerTest, BranchStatisticsCpp)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    runner.GetRuntimeOptions().SetInterpreterType("cpp");
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod("foo");
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(132U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(199U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(67U, profilingData->GetBranchNotTakenCounter(0x10U));
    Runtime::Destroy();
}

TEST_F(ProfilingRunnerTest, ProfilingDataNullptTestCpp)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetInterpreterType("cpp");
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod("foo");
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(nullptr, profilingData);
    ProfilingSaver saver;
    pgo::AotProfilingData profData;
    saver.AddMethod(&profData, method, 0);
    Runtime::Destroy();
}

#ifndef PANDA_COMPILER_TARGET_AARCH32
TEST_F(ProfilingRunnerTest, BranchStatistics)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod("foo");
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(132U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(199U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(67U, profilingData->GetBranchNotTakenCounter(0x10U));
    Runtime::Destroy();
}

TEST_F(ProfilingRunnerTest, ProfilingDataNullptTest)
{
    PandaRunner runner;
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod("foo");
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(nullptr, profilingData);
    ProfilingSaver saver;
    pgo::AotProfilingData profData;
    saver.AddMethod(&profData, method, 0);
    Runtime::Destroy();
}
#endif
}  // namespace ark::test
