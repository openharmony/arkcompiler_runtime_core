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

#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

#include "runtime/execution/job_launch.h"
#include "runtime/execution/job_execution_context.h"

namespace ark::ets::test {

class EtsNoSuitableWorkerTest : public ani::testing::AniTest {};

TEST_F(EtsNoSuitableWorkerTest, NoSuitableWorkerForNativeCoro)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] ScopedManagedCodeThread sj(executionCtx->GetMT());
    [[maybe_unused]] EtsHandleScope hs(executionCtx);
    auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto groupId = JobWorkerThreadGroup::FromDomain(jobMan, JobWorkerThreadDomain::EXACT_ID, {99});
    auto cb = []([[maybe_unused]] void *data) {};
    auto *job = jobMan->CreateJob("NoSuitableWorkerForNativeCoro", Job::NativeEntrypointInfo {cb, nullptr});
    auto launchRes = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId});
    ASSERT_EQ(launchRes, LaunchResult::NO_SUITABLE_WORKER);
    ASSERT_TRUE(executionCtx->GetMT()->HasPendingException());
}

}  // namespace ark::ets::test
