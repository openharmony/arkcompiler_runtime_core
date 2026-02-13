/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/finalreg/finalization_registry_manager.h"

namespace ark::ets::intrinsics {
extern "C" EtsInt StdFinalizationRegistryGetWorkerId()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    return static_cast<EtsInt>(executionCtx->GetWorker()->GetId());
}

extern "C" EtsInt StdFinalizationRegistryGetWorkerDomain()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return static_cast<EtsInt>(FinalizationRegistryManager::GetExecCtxDomain(executionCtx));
}

extern "C" void StdFinalizationRegistryFinishCleanup()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    executionCtx->GetPandaVM()->GetFinalizationRegistryManager()->CleanupJobFinished();
}
}  // namespace ark::ets::intrinsics
