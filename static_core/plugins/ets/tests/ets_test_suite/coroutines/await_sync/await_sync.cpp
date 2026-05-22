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

#include <cstdlib>
#include <ani.h>
#include <gtest/gtest.h>

#include "execution/job.h"
#include "execution/job_execution_context.h"
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

namespace ark::ets::test {

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class EtsCoroutineAwaitSyncTest : public ani::testing::AniTest, public ::testing::WithParamInterface<const char *> {
private:
    std::string coroOptionStr_;  // Stores the constructed option string

protected:
    void SetUp() override
    {
        coroOptionStr_ = std::string("--ext:coroutine-impl=") + GetParam();
        ani::testing::AniTest::SetUp();
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        ani_option gcType = {"--ext:gc-type=epsilon", nullptr};
        ani_option jit = {"--ext:compiler-enable-jit=false", nullptr};
        ani_option coroImpl = {coroOptionStr_.c_str(), nullptr};
        ani_option coroutineWorkersCount = {"--ext:coroutine-workers-count=2", nullptr};
        return {gcType, jit, coroImpl, coroutineWorkersCount};
    }
};

static ani_boolean IsBlocked([[maybe_unused]] ani_env *env, ani_long jobId)
{
    auto cb = [jobId](Job *job) {
        if (jobId != job->GetId()) {
            return true;
        }
        auto jobStatus = job->GetStatus();

        return jobStatus == Job::Status::YIELDED || jobStatus == Job::Status::BLOCKED;
    };

    return static_cast<ani_boolean>(JobExecutionContext::GetCurrent()->GetManager()->EnumerateJobs(cb));
}

static const ani_native_function NATIVE_FUNC_CHECK_IF_BLOCKED = {"isBlocked", "l:z",
                                                                 reinterpret_cast<void *>(IsBlocked)};

TEST_P(EtsCoroutineAwaitSyncTest, AwaitSyncTest)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("await_sync", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    std::array functions = {
        NATIVE_FUNC_CHECK_IF_BLOCKED,
    };
    ASSERT_EQ(env_->Module_BindNativeFunctions(module, functions.data(), functions.size()), ANI_OK);

    auto result = CallEtsFunction<ani_int>("await_sync", "main");
    ASSERT_EQ(result, 0);
}

INSTANTIATE_TEST_SUITE_P(CoroutineImplementations, EtsCoroutineAwaitSyncTest,
                         ::testing::Values("stackful", "stackless"));

}  // namespace ark::ets::test
