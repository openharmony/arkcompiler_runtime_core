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
#include "runtime/execution/job_execution_context.h"
#include "plugins/ets/runtime/intrinsics/helpers/intrinsic_timer_impl.h"

namespace ark::ets::intrinsics {

helpers::TimerId StdCoreRegisterTimer(EtsObject *callback, int32_t delay, uint8_t periodic)
{
    auto *refStorage = JobExecutionContext::GetCurrent()->GetVM()->GetGlobalObjectStorage();
    auto *callbackRef = refStorage->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    return helpers::CreateTimer(helpers::ManagedEntrypoint {callbackRef}, delay, periodic != 0);
}

void StdCoreClearTimer(helpers::TimerId timerId)
{
    helpers::StopTimer(timerId);
}

}  // namespace ark::ets::intrinsics
