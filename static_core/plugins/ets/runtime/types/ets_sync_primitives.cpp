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
#include "plugins/ets/runtime/types/ets_sync_primitives.h"

#include <atomic>

namespace ark::ets {

/*static*/
EtsMutex *EtsMutex::Create(EtsCoroutine *coro)
{
    auto *klass = coro->GetPandaVM()->GetClassLinker()->GetMutexClass();
    auto hMutex = EtsHandle<EtsMutex>(coro, EtsMutex::FromEtsObject(EtsObject::Create(coro, klass)));
    auto *waitersList = EtsWaitersList::Create(coro);
    hMutex->SetWaitersList(coro, waitersList);
    return hMutex.GetPtr();
}

void EtsMutex::Lock()
{
    // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    if (waiters_.fetch_add(1, std::memory_order_acq_rel) == 0) {
        return;
    }
    auto *coroManager = EtsCoroutine::GetCurrent()->GetCoroutineManager();
    auto awaitee = EtsWaitersList::Node(coroManager);
    SuspendCoroutine(&awaitee);
}

void EtsMutex::Unlock()
{
    // Atomic with acq_rel order reason: sync Lock/Unlock in other threads
    if (waiters_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        return;
    }
    ResumeCoroutine();
}

bool EtsMutex::IsHeld()
{
    // Atomic with relaxed order reason: sync is not needed here
    // because it is expected that method is not called concurrently with Lock/Unlock
    return waiters_.load(std::memory_order_relaxed) != 0;
}

/*static*/
EtsEvent *EtsEvent::Create(EtsCoroutine *coro)
{
    auto *klass = coro->GetPandaVM()->GetClassLinker()->GetEventClass();
    auto hEvent = EtsHandle<EtsEvent>(coro, EtsEvent::FromEtsObject(EtsObject::Create(coro, klass)));
    auto *waitersList = EtsWaitersList::Create(coro);
    hEvent->SetWaitersList(coro, waitersList);
    return hEvent.GetPtr();
}

void EtsEvent::Wait()
{
    // Atomic with acq_rel order reason: sync Wait/Fire in other threads
    auto state = state_.fetch_add(ONE_WAITER, std::memory_order_acq_rel);
    if (IsFireState(state)) {
        return;
    }
    auto *coroManager = EtsCoroutine::GetCurrent()->GetCoroutineManager();
    auto awaitee = EtsWaitersList::Node(coroManager);
    SuspendCoroutine(&awaitee);
}

void EtsEvent::Fire()
{
    // Atomic with acq_rel order reason: sync Wait/Fire in other threads
    auto state = state_.exchange(FIRE_STATE, std::memory_order_acq_rel);
    if (IsFireState(state)) {
        return;
    }
    for (auto waiters = GetNumberOfWaiters(state); waiters > 0; --waiters) {
        ResumeCoroutine();
    }
}

/* static */
EtsCondVar *EtsCondVar::Create(EtsCoroutine *coro)
{
    auto *classLinker = coro->GetPandaVM()->GetClassLinker();
    auto *klass = classLinker->GetCondVarClass();
    auto hCondVar = EtsHandle<EtsCondVar>(coro, EtsCondVar::FromEtsObject(EtsObject::Create(klass)));
    auto *waitersList = EtsWaitersList::Create(coro);
    hCondVar->SetWaitersList(coro, waitersList);
    return hCondVar.GetPtr();
}

void EtsCondVar::Wait(EtsHandle<EtsMutex> &mutex)
{
    ASSERT(mutex->IsHeld());
    waiters_++;
    mutex->Unlock();
    auto *coroManager = EtsCoroutine::GetCurrent()->GetCoroutineManager();
    auto awaitee = EtsWaitersList::Node(coroManager);
    SuspendCoroutine(&awaitee);
    mutex->Lock();
}

void EtsCondVar::NotifyOne([[maybe_unused]] EtsMutex *mutex)
{
    ASSERT(mutex->IsHeld());
    if (waiters_ != 0) {
        ResumeCoroutine();
        waiters_--;
    }
}

void EtsCondVar::NotifyAll([[maybe_unused]] EtsMutex *mutex)
{
    ASSERT(mutex->IsHeld());
    while (waiters_ != 0) {
        ResumeCoroutine();
        waiters_--;
    }
}

}  // namespace ark::ets
