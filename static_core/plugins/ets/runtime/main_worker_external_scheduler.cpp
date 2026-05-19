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

#include "plugins/ets/runtime/main_worker_external_scheduler.h"
#include "runtime/execution/job_manager.h"
#include "runtime/execution/job_execution_context.h"

namespace ark::ets {

static void ScheduleWrapper([[maybe_unused]] void *param)
{
    auto *jobMan = JobExecutionContext::GetCurrent()->GetManager();
    ASSERT(JobExecutionContext::GetCurrent() == jobMan->GetMainThread());
    jobMan->ExecuteJobs();
}

static void ResetTriggerAndScheduleWrapper(void *param)
{
    auto *needTriggerScheduler = static_cast<std::atomic<bool> *>(param);
    *needTriggerScheduler = true;
    ScheduleWrapper(nullptr);
}

MainWorkerExternalScheduler::MainWorkerExternalScheduler(TaskPosterFunc poster)
    : needTriggerScheduler_(true), poster_(poster)
{
}

void MainWorkerExternalScheduler::PostImpl(int64_t delayMs)
{
    if (delayMs <= 0) {
        if (needTriggerScheduler_.exchange(false)) {
            poster_(ResetTriggerAndScheduleWrapper, &needTriggerScheduler_, "ScheduleCoroutine", 0);
        }
    } else {
        poster_(ScheduleWrapper, nullptr, "ScheduleCoroutine", delayMs);
    }
}

}  // namespace ark::ets