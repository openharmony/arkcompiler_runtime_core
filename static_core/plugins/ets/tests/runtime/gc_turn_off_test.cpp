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

#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark::test {

class GCTurnOffTest : public testing::Test {
public:
    GCTurnOffTest()
    {
        RuntimeOptions options;
        options.SetCompilerEnableJit(false);
        options.SetShouldLoadBootPandaFiles(true);  // Need for weakrefs
        options.SetGcType("g1-gc");
        options.SetLoadRuntimes({"ets"});
        options.SetGcTriggerType("debug-never");

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }

        options.SetBootPandaFiles({stdlib});
        Runtime::Create(options);
    }
    ~GCTurnOffTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(GCTurnOffTest);
    NO_MOVE_SEMANTIC(GCTurnOffTest);
};

static size_t GetGCLimit(mem::GC *gc)
{
    auto limit = gc->AdjustStartupLimit(0);
    gc->AdjustStartupLimit(limit);  // Need to restore limit
    return limit;
}

TEST_F(GCTurnOffTest, GCTurnOffTest)
{
    auto *runtime = Runtime::GetCurrent();
    auto *gc = runtime->GetPandaVM()->GetGC();

    auto startLimit = GetGCLimit(gc);

    runtime->PreZygoteFork();
    runtime->PostZygoteFork();

    auto limitOnFork = GetGCLimit(gc);

    ASSERT_GT(limitOnFork, startLimit);

    auto begin = std::chrono::system_clock::now();
    runtime->GetPandaVM()->GetGC()->GetWorkersTaskQueue()->WaitBackgroundTasks();
    auto end = std::chrono::system_clock::now();

    // In milliseconds
    constexpr uint64_t TIME_TO_WAIT_IN_MS = 2ULL * 1000ULL;
    constexpr uint64_t THRESHOLD_PERCENT = 150ULL;  // If system is highly loaded we can get huge overtime

    auto waitedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    ASSERT_GE(waitedTime, TIME_TO_WAIT_IN_MS);
    ASSERT_LT(waitedTime, TIME_TO_WAIT_IN_MS * (100ULL + THRESHOLD_PERCENT) / 100ULL);

    auto endLimit = GetGCLimit(gc);
    ASSERT_EQ(startLimit, endLimit);
}

}  // namespace ark::test
