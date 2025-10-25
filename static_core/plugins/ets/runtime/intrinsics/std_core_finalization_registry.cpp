/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "runtime/include/thread.h"

namespace ark::ets::intrinsics {

/**
 * The function register FinalizationRegistry instance in ETS VM.
 * @param instance - FinalizationRegistry class instance needed to register for managing by GC.
 */
extern "C" void StdFinalizationRegistryRegisterInstance(EtsObject *instance)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    coro->GetPandaVM()->GetFinalizationRegistryManager()->RegisterInstance(instance);
}

extern "C" EtsInt StdFinalizationRegistryGetWorkerId()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    return static_cast<EtsInt>(coro->GetWorker()->GetId());
}

extern "C" EtsInt StdFinalizationRegistryGetWorkerDomain()
{
    auto *coro = EtsCoroutine::GetCurrent();
    return static_cast<EtsInt>(FinalizationRegistryManager::GetCoroDomain(coro));
}

extern "C" bool StdFinalizationRegistryCanCleanup(EtsInt expectedWorkerId, EtsInt expectedWorkerDomain)
{
    auto *coro = EtsCoroutine::GetCurrent();
    CoroutineWorkerDomain currentWorkerDomain = FinalizationRegistryManager::GetCoroDomain(coro);
    if (expectedWorkerDomain == static_cast<EtsInt>(CoroutineWorkerDomain::MAIN)) {
        return currentWorkerDomain == CoroutineWorkerDomain::MAIN;
    } else if (expectedWorkerDomain == static_cast<EtsInt>(CoroutineWorkerDomain::GENERAL)) {
        return currentWorkerDomain == CoroutineWorkerDomain::MAIN ||
               currentWorkerDomain == CoroutineWorkerDomain::GENERAL;
    } else if (expectedWorkerDomain == static_cast<EtsInt>(CoroutineWorkerDomain::EA)) {
        return (currentWorkerDomain == CoroutineWorkerDomain::EA) && (coro->GetWorker()->GetId() == expectedWorkerId);
    }
    return false;
}

extern "C" void StdFinalizationRegistryFinishCleanup()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    coro->GetPandaVM()->GetFinalizationRegistryManager()->CleanupCoroFinished();
}

}  // namespace ark::ets::intrinsics
