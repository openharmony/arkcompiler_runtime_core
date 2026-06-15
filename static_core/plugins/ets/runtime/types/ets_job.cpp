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

#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/managed_thread.h"

namespace ark::ets {

/*static*/
EtsJob *EtsJob::Create(EtsExecutionContext *executionCtx)
{
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->coreJob;
    auto hJobObject = EtsObject::Create(executionCtx, klass);
    ASSERT(hJobObject != nullptr);
    auto hJob = EtsHandle<EtsJob>(executionCtx, EtsJob::FromEtsObject(hJobObject));
    auto *mutex = EtsMutex::Create(executionCtx);
    hJob->SetMutex(executionCtx, mutex);
    auto *event = EtsEvent::Create(executionCtx);
    hJob->SetEvent(executionCtx, event);
    hJob->state_ = STATE_RUNNING;
    return hJob.GetPtr();
}

/*static*/
void EtsJob::EtsJobFinish(EtsJob *job, EtsObject *value)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsJob> hJob(executionCtx, job);
    ASSERT(hJob.GetPtr() != nullptr);
    EtsHandle<EtsObject> hvalue(executionCtx, value);
    EtsMutex::LockHolder lh(hJob);
    if (!hJob->IsRunning()) {
        return;
    }
    hJob->Finish(executionCtx, hvalue.GetPtr());
}

/*static*/
void EtsJob::EtsJobFail(EtsJob *job, EtsObject *error)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsJob> hJob(executionCtx, job);
    ASSERT(hJob.GetPtr() != nullptr);
    EtsHandle<EtsObject> herror(executionCtx, error);
    EtsMutex::LockHolder lh(hJob);
    if (!hJob->IsRunning()) {
        return;
    }
    hJob->Fail(executionCtx, herror.GetPtr());
}

}  // namespace ark::ets
