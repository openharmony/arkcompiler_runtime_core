/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_exceptions.h"

namespace ark::ets::intrinsics {

extern "C" EtsLong GenerateTaskId()
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return coroutine->GetPandaVM()->GetTaskpool()->GenerateTaskId();
}

extern "C" void TaskpoolTaskSubmitted(EtsLong taskId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    coroutine->GetPandaVM()->GetTaskpool()->TaskSubmitted(taskId);
}

extern "C" EtsBoolean TaskpoolTaskStarted(EtsLong taskId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return EtsBoolean(coroutine->GetPandaVM()->GetTaskpool()->TaskStarted(coroutine->GetCoroutineId(), taskId));
}

extern "C" EtsBoolean TaskpoolTaskFinished(EtsLong taskId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return EtsBoolean(coroutine->GetPandaVM()->GetTaskpool()->TaskFinished(coroutine->GetCoroutineId(), taskId));
}

extern "C" void TaskpoolCancelTask(EtsLong taskId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    if (!coroutine->GetPandaVM()->GetTaskpool()->CancelTask(taskId)) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ERROR,
                          "taskpool:: task is not executed or has been executed");
    }
}

extern "C" EtsBoolean TaskpoolIsTaskCanceled()
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return EtsBoolean(coroutine->GetPandaVM()->GetTaskpool()->IsTaskCanceled(coroutine->GetCoroutineId()));
}

}  // namespace ark::ets::intrinsics
