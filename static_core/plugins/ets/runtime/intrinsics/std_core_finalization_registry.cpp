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
extern "C" EtsInt StdFinalizationRegistryRegisterInstance(EtsObject *instance)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    coro->GetPandaVM()->RegisterFinalizationRegistryInstance(instance);
    auto launchMode = coro->GetCoroutineManager()->IsMainWorker(coro) ? CoroutineLaunchMode::MAIN_WORKER
                                                                      : CoroutineLaunchMode::DEFAULT;
    return static_cast<EtsInt>(launchMode);
}

extern "C" void StdFinalizationRegistryFinishCleanup()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    coro->GetPandaVM()->FinalizationRegistryCoroutineExecuted();
}

}  // namespace ark::ets::intrinsics
