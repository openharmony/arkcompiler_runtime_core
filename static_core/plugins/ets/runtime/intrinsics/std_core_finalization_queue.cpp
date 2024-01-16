/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/types/ets_void.h"

namespace ark::ets::intrinsics {

/**
 * The function register FinalizationQueue instance in ETS VM.
 * @param instance - FinalizationQueue class instance needed to register for managing by GC.
 */
extern "C" EtsVoid *StdFinalizationQueueRegisterInstance(EtsObject *instance)
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    static_cast<PandaEtsVM *>(thread->GetVM())->RegisterFinalizationQueueInstance(instance);
    return EtsVoid::GetInstance();
}

}  // namespace ark::ets::intrinsics
