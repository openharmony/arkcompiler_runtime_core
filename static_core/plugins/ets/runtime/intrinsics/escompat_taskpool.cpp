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

extern "C" EtsLong GenerateGroupId()
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return coroutine->GetPandaVM()->GetTaskpool()->GenerateTaskGroupId();
}

extern "C" EtsLong GenerateSeqRunnerId()
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return coroutine->GetPandaVM()->GetTaskpool()->GenerateSeqRunnerId();
}

extern "C" void TaskpoolTaskSubmitted(EtsLong taskId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    coroutine->GetPandaVM()->GetTaskpool()->TaskSubmitted(taskId);
}

extern "C" void TaskpoolGroupSubmitted(EtsLong groupId, EtsLong tasksCount)
{
    ASSERT(tasksCount > 0);
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    coroutine->GetPandaVM()->GetTaskpool()->GroupSubmitted(groupId, static_cast<size_t>(tasksCount));
}

extern "C" EtsBoolean TaskpoolTaskStarted(EtsLong taskId, EtsLong groupId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return EtsBoolean(
        coroutine->GetPandaVM()->GetTaskpool()->TaskStarted(coroutine->GetCoroutineId(), taskId, groupId));
}

extern "C" EtsBoolean TaskpoolTaskFinished(EtsLong taskId, EtsLong groupId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return EtsBoolean(
        coroutine->GetPandaVM()->GetTaskpool()->TaskFinished(coroutine->GetCoroutineId(), taskId, groupId));
}

extern "C" void TaskpoolCancelTask(EtsLong taskId, EtsLong seqId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    if (!coroutine->GetPandaVM()->GetTaskpool()->CancelTask(taskId)) {
        // seqId = 0 means task was not added to a task sequence runner
        const char *messageError = (seqId == 0) ? "taskpool:: task is not executed or has been executed"
                                                : "taskpool:: sequenceRunner task has been executed";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ERROR, messageError);
    }
}

extern "C" void TaskpoolCancelGroup(EtsLong groupId)
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    if (!coroutine->GetPandaVM()->GetTaskpool()->CancelGroup(groupId)) {
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ERROR,
                          "taskpool:: taskGroup is not executed or has been executed");
    }
}

extern "C" EtsBoolean TaskpoolIsTaskCanceled()
{
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    return EtsBoolean(coroutine->GetPandaVM()->GetTaskpool()->IsTaskCanceled(coroutine->GetCoroutineId()));
}

}  // namespace ark::ets::intrinsics
