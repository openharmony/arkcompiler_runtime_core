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

#include "intrinsics.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/include/object_header.h"
#include "types/ets_object.h"

namespace ark::ets::intrinsics {
extern "C" {
EtsObject *EtsAwaitJob(EtsJob *job)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return nullptr;
    }
    if (JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager()->IsJobSwitchDisabled()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreInvalidJobOperationError,
                          "Cannot await in the current context!");
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsJob> jobHandle(executionCtx, job);

    LOG(DEBUG, COROUTINES) << "Job::await: starting await() for a job...";
    jobHandle->Wait();
    ASSERT(!jobHandle->IsRunning());
    LOG(DEBUG, COROUTINES) << "Job::await: await() finished.";

    if (jobHandle->IsFinished()) {
        LOG(DEBUG, COROUTINES) << "Job::await: job is already successfully finished!";
        return jobHandle->GetValue(executionCtx);
    }
    LOG(DEBUG, COROUTINES) << "Job::await: job is failed!";

    executionCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveFailedJob(jobHandle.GetPtr());
    auto *exc = jobHandle->GetValue(executionCtx);
    executionCtx->GetMT()->SetException(exc->GetCoreType());
    return nullptr;
}

void EtsFinishJob(EtsJob *job, EtsObject *value)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsJob> hjob(executionCtx, job);
    EtsHandle<EtsObject> hvalue(executionCtx, value);
    EtsMutex::LockHolder lh(hjob);
    if (!hjob->IsRunning()) {
        return;
    }
    hjob->Finish(executionCtx, hvalue.GetPtr());
}

void EtsFailJob(EtsJob *job, EtsObject *error)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsJob> hjob(executionCtx, job);
    ASSERT(error != nullptr);
    EtsHandle<EtsObject> herror(executionCtx, error);
    EtsMutex::LockHolder lh(hjob);

    auto *errorClass = PlatformTypes(executionCtx)->coreError;
    if (!herror->IsInstanceOf(errorClass)) {
        ThrowRuntimeException("fail() argument is not an Error object");
        return;
    }

    if (!hjob->IsRunning()) {
        return;
    }
    hjob->Fail(executionCtx, herror.GetPtr());
}
}
}  // namespace ark::ets::intrinsics
