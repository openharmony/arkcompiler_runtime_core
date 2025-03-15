/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

/*static*/
EtsJob *EtsJob::Create(EtsCoroutine *coro)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *klass = PlatformTypes(coro)->coreJob;
    auto hJob = EtsHandle<EtsJob>(coro, EtsJob::FromEtsObject(EtsObject::Create(coro, klass)));
    auto *mutex = EtsMutex::Create(coro);
    hJob->SetMutex(coro, mutex);
    auto *event = EtsEvent::Create(coro);
    hJob->SetEvent(coro, event);
    hJob->state_ = STATE_RUNNING;
    return hJob.GetPtr();
}

/*static*/
void EtsJob::EtsJobFinish(EtsJob *job, EtsObject *value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsJob> hjob(coro, job);
    EtsHandle<EtsObject> hvalue(coro, value);
    EtsMutex::LockHolder lh(hjob);
    if (!hjob->IsRunning()) {
        return;
    }
    hjob->Finish(coro, hvalue.GetPtr());
}

/*static*/
void EtsJob::EtsJobFail(EtsJob *job, EtsObject *error)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsJob> hjob(coro, job);
    EtsHandle<EtsObject> herror(coro, error);
    EtsMutex::LockHolder lh(hjob);
    if (!hjob->IsRunning()) {
        return;
    }
    hjob->Fail(coro, herror.GetPtr());
}

}  // namespace ark::ets
