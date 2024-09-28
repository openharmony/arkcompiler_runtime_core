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

#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"

namespace ark::ets::intrinsics {

EtsObject *EtsMutexCreate()
{
    return EtsMutex::Create(EtsCoroutine::GetCurrent());
}

void EtsMutexLock(EtsObject *mutex)
{
    ASSERT(mutex->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetMutexClass());
    EtsMutex::FromEtsObject(mutex)->Lock();
}

void EtsMutexUnlock(EtsObject *mutex)
{
    ASSERT(mutex->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetMutexClass());
    EtsMutex::FromEtsObject(mutex)->Unlock();
}

EtsObject *EtsEventCreate()
{
    return EtsEvent::Create(EtsCoroutine::GetCurrent());
}

void EtsEventWait(EtsObject *event)
{
    ASSERT(event->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetEventClass());
    EtsEvent::FromEtsObject(event)->Wait();
}

void EtsEventFire(EtsObject *event)
{
    ASSERT(event->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetEventClass());
    EtsEvent::FromEtsObject(event)->Fire();
}

EtsObject *EtsCondVarCreate()
{
    return EtsCondVar::Create(EtsCoroutine::GetCurrent());
}

void EtsCondVarWait(EtsObject *condVar, EtsObject *mutex)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(condVar->GetClass() == coro->GetPandaVM()->GetClassLinker()->GetCondVarClass());
    ASSERT(mutex->GetClass() == coro->GetPandaVM()->GetClassLinker()->GetMutexClass());
    EtsHandleScope scope(coro);
    EtsHandle<EtsMutex> hMutex(coro, EtsMutex::FromEtsObject(mutex));
    EtsCondVar::FromEtsObject(condVar)->Wait(hMutex);
}

void EtsCondVarNotifyOne(EtsObject *condVar, EtsObject *mutex)
{
    ASSERT(condVar->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetCondVarClass());
    ASSERT(mutex->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetMutexClass());
    EtsCondVar::FromEtsObject(condVar)->NotifyOne(EtsMutex::FromEtsObject(mutex));
}

void EtsCondVarNotifyAll(EtsObject *condVar, EtsObject *mutex)
{
    ASSERT(condVar->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetCondVarClass());
    ASSERT(mutex->GetClass() == EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetMutexClass());
    EtsCondVar::FromEtsObject(condVar)->NotifyAll(EtsMutex::FromEtsObject(mutex));
}

}  // namespace ark::ets::intrinsics
