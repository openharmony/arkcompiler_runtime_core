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

#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine_events.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"

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

    auto *cbQueue = GetCallbackQueue(coro);
    auto *launchModeQueue = GetLaunchModeQueue(coro);
    auto queueSize = GetQueueSize();

    for (int idx = 0; idx < queueSize; ++idx) {
        auto *thenCallback = cbQueue->Get(idx);
        auto launchMode = static_cast<CoroutineLaunchMode>(launchModeQueue->Get(idx));
        EtsPromise::LaunchCallback(coro, thenCallback, launchMode);
    }
    ClearQueues(coro);
}

/* static */
void EtsPromise::LaunchCallback(EtsCoroutine *coro, EtsObject *callback, CoroutineLaunchMode launchMode)
{
    // Post callback to js env
    auto *jobQueue = coro->GetPandaVM()->GetJobQueue();
    if (launchMode == CoroutineLaunchMode::MAIN_WORKER && jobQueue != nullptr) {
        jobQueue->Post(callback);
        return;
    }
    // Launch callback in its own coroutine
    auto *coroManager = coro->GetCoroutineManager();
    auto *event = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, coroManager);
    auto *method = EtsMethod::ToRuntimeMethod(callback->GetClass()->GetMethod("invoke"));
    ASSERT(method != nullptr);
    auto args = PandaVector<Value> {Value(callback->GetCoreType())};
    [[maybe_unused]] auto *launchedCoro = coroManager->Launch(event, method, std::move(args), launchMode);
    ASSERT(launchedCoro != nullptr);
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
