/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "runtime/coroutines/coroutine_events.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_callback.h"

namespace ark::ets {

/*static*/
EtsPromise *EtsPromise::Create(EtsCoroutine *etsCoroutine)
{
    EtsClass *klass = etsCoroutine->GetPandaVM()->GetClassLinker()->GetPromiseClass();
    EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
    return reinterpret_cast<EtsPromise *>(etsObject);
}

void EtsPromise::OnPromiseCompletion(EtsCoroutine *coro)
{
    auto *event = GetEventPtr();
    // event can be equal to nullptr in case of GENERIC CoroutineEvent
    if (event != nullptr) {
        event->Happen();
        SetEventPtr(nullptr);
    }

    PandaUnorderedMap<EtsCoroutine *, PandaList<PandaUniquePtr<Callback>>> readyCallbacks;
    auto *cbQueue = GetCallbackQueue(coro);
    auto *coroPtrQueue = GetCoroPtrQueue(coro);
    auto queueSize = GetQueueSize();

    for (int idx = 0; idx < queueSize; ++idx) {
        auto *callbackCoro = reinterpret_cast<EtsCoroutine *>(coroPtrQueue->Get(idx));
        auto *thenCallback = cbQueue->Get(idx);
        auto callback = EtsCallback::Create(coro, thenCallback);
        readyCallbacks[callbackCoro].push_back(std::move(callback));
    }

    ClearQueues(coro);
    for (auto &[callbackCoro, callbacks] : readyCallbacks) {
        callbackCoro->AcceptAnnouncedCallbacks(std::move(callbacks));
    }
}

void EtsPromise::EnsureCapacity(EtsCoroutine *coro)
{
    int queueLength = GetCallbackQueue(coro) == nullptr ? 0 : GetCallbackQueue(coro)->GetLength();
    if (queueSize_ > queueLength) {
        return;
    }
    auto newQueueLength = queueLength * 2U + 1U;
    auto *objectClass = coro->GetPandaVM()->GetClassLinker()->GetObjectClass();
    auto *tmpCbQueue = EtsObjectArray::Create(objectClass, newQueueLength);
    if (queueSize_ != 0) {
        GetCallbackQueue(coro)->CopyDataTo(tmpCbQueue);
    }
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, callbackQueue_), tmpCbQueue->GetCoreType());
    auto *newCoroPtrQueue = EtsLongArray::Create(newQueueLength);
    if (queueSize_ != 0) {
        auto *coroPtrQueueData = GetCoroPtrQueue(coro)->GetData<EtsCoroutine *>();
        [[maybe_unused]] auto err =
            memcpy_s(newCoroPtrQueue->GetData<EtsCoroutine *>(), newQueueLength * sizeof(EtsLong), coroPtrQueueData,
                     queueLength * sizeof(EtsCoroutine *));
        ASSERT(err == EOK);
    }
    ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, coroPtrQueue_), newCoroPtrQueue->GetCoreType());
}

void EtsPromise::PromotePromiseRef(EtsCoroutine *coro)
{
    auto *event = GetEventPtr();
    // event can be equal to nullptr in case of GENERIC CoroutineEvent
    if (event == nullptr || event->GetType() != CoroutineEvent::Type::COMPLETION) {
        return;
    }
    auto *completionEvent = static_cast<CompletionEvent *>(event);
    os::memory::LockHolder lh(*completionEvent);
    auto *promiseRef = completionEvent->ReleasePromise();
    // promiseRef can be equal to nullptr in case of concurrent Promise.resolve
    if (promiseRef == nullptr || promiseRef->IsGlobal()) {
        completionEvent->SetPromise(promiseRef);
        return;
    }
    ASSERT(promiseRef->IsWeak());
    auto *storage = coro->GetPandaVM()->GetGlobalObjectStorage();
    auto *globalPromiseRef = storage->Add(this, mem::Reference::ObjectType::GLOBAL);
    completionEvent->SetPromise(globalPromiseRef);
    storage->Remove(promiseRef);
}

CoroutineEvent *EtsPromise::GetOrCreateEventPtr()
{
    ASSERT(IsLocked());
    auto *event = GetEventPtr();
    if (event != nullptr) {
        if (event->GetType() == CoroutineEvent::Type::GENERIC) {
            eventRefCounter_++;
        }
        return event;
    }
    ASSERT(eventRefCounter_ == 0);
    auto *coroManager = EtsCoroutine::GetCurrent()->GetCoroutineManager();
    event = Runtime::GetCurrent()->GetInternalAllocator()->New<GenericEvent>(coroManager);
    eventRefCounter_++;
    SetEventPtr(event);
    return event;
}

void EtsPromise::RetireEventPtr(CoroutineEvent *event)
{
    ASSERT(event != nullptr);
    os::memory::LockHolder lk(*this);
    ASSERT(event->GetType() == CoroutineEvent::Type::GENERIC);
    ASSERT(GetEventPtr() == event || GetEventPtr() == nullptr);
    if (eventRefCounter_-- == 1) {
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(event);
        SetEventPtr(nullptr);
    }
}

}  // namespace ark::ets
