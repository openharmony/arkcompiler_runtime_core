/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/job_queue.h"
#include "runtime/coroutines/stackful_coroutine.h"

namespace ark::ets::intrinsics {

// length parameter represents the current size of the queue as opposed to its length field which represents its
// capacity
static void OnPromiseCompletion(EtsCoroutine *coro, EtsHandle<EtsPromise> &promise, EtsHandle<EtsObjectArray> &queue,
                                EtsInt length)
{
    // Since handle cannot be created for nullptr use 'promise' object as a special marker of empty value
    VMMutableHandle<ObjectHeader> exception(coro, promise.GetPtr());
    for (EtsInt i = 0; i < length; ++i) {
        EtsObject *callback = queue->Get(i);
        if (callback != nullptr) {
            queue->Set(i, nullptr);
            LambdaUtils::InvokeVoid(coro, callback);
            if (coro->HasPendingException() && exception.GetPtr() == promise.GetPtr()) {
                exception.Update(coro->GetException());
                coro->ClearException();
            }
        }
    }
    promise->ClearQueues(coro);
    if (exception.GetPtr() != promise.GetPtr()) {
        coro->SetException(exception.GetPtr());
    }
    coro->GetPandaVM()->FirePromiseStateChanged(promise);
}

void SubscribePromiseOnResultObject(EtsPromise *outsidePromise, EtsPromise *internalPromise)
{
    PandaVector<Value> args {Value(outsidePromise), Value(internalPromise)};

    auto subscribeOnAnotherPromise = [&args]() {
        EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetSubscribeOnAnotherPromiseMethod()->Invoke(
            EtsCoroutine::GetCurrent(), args.data());
    };

    auto *mainT = EtsCoroutine::GetCurrent()->GetPandaVM()->GetCoroutineManager()->GetMainThread();
    Coroutine *mainCoro = Coroutine::CastFromThread(mainT);
    if (Coroutine::GetCurrent() != mainCoro) {
        mainCoro->GetContext<StackfulCoroutineContext>()->ExecuteOnThisContext(
            &subscribeOnAnotherPromise, EtsCoroutine::GetCurrent()->GetContext<StackfulCoroutineContext>());
    } else {
        subscribeOnAnotherPromise();
    }
}

void EtsPromiseResolve(EtsPromise *promise, EtsObject *value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    if (promise->GetState() != EtsPromise::STATE_PENDING) {
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);

    if (value != nullptr && value->IsInstanceOf(coro->GetPandaVM()->GetClassLinker()->GetPromiseClass())) {
        auto internalPromise = EtsPromise::FromEtsObject(value);
        EtsHandle<EtsPromise> hInternalPromise(coro, internalPromise);
        if (hInternalPromise->IsPending() || coro->GetCoroutineManager()->IsJsMode()) {
            SubscribePromiseOnResultObject(hpromise.GetPtr(), hInternalPromise.GetPtr());
            return;
        }
        if (hInternalPromise->IsRejected()) {
            EtsPromiseReject(hpromise.GetPtr(), hInternalPromise->GetValue(coro));
            return;
        }
        // We can use internal promise's value as return value
        value = hInternalPromise->GetValue(coro);
    }
    EtsHandle<EtsObjectArray> thenQueue(coro, hpromise->GetThenQueue(coro));
    hpromise->Resolve(coro, value);
    OnPromiseCompletion(coro, hpromise, thenQueue, hpromise->GetThenQueueSize());
}

void EtsPromiseReject(EtsPromise *promise, EtsObject *error)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    if (promise->GetState() != EtsPromise::STATE_PENDING) {
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    EtsHandle<EtsObjectArray> catchQueue(coro, hpromise->GetCatchQueue(coro));
    hpromise->Reject(coro, error);
    OnPromiseCompletion(coro, hpromise, catchQueue, hpromise->GetCatchQueueSize());
}

void EtsPromiseAddToJobQueue(EtsObject *callback)
{
    auto *jobQueue = EtsCoroutine::GetCurrent()->GetPandaVM()->GetJobQueue();
    if (jobQueue != nullptr) {
        jobQueue->AddJob(callback);
    }
}

void EtsPromiseCreateLink(EtsObject *source, EtsPromise *target)
{
    EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
    auto *jobQueue = currentCoro->GetPandaVM()->GetJobQueue();
    if (jobQueue != nullptr) {
        jobQueue->CreateLink(source, target->AsObject());
    }
}

EtsObject *EtsAwaitPromise(EtsPromise *promise)
{
    EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, currentCoro);
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    EtsHandle<EtsPromise> promiseHandle(currentCoro, promise);

    /* CASE 1. This is a converted JS promise */

    if (promiseHandle->IsProxy()) {
        /**
         * This is a backed by JS equivalent promise.
         * STS mode: error, no one can create such a promise!
         * JS mode:
         *      - add a callback to JQ, that will:
         *          - resolve the promise with some value OR reject it
         *          - unblock the coro via event
         *          - schedule();
         *      - create a blocker event and link it to the promise
         *      - block current coroutine on the event
         *      - Schedule();
         *          (the last two steps are actually the cm->await()'s job)
         *      - return promise.value() if resolved or throw() it if rejected
         */
        EtsPromiseCreateLink(promiseHandle->GetLinkedPromise(currentCoro), promiseHandle.GetPtr());

        PandaUniquePtr<CoroutineEvent> e = MakePandaUnique<GenericEvent>();
        e->Lock();
        promiseHandle->SetEventPtr(e.get());
        currentCoro->GetCoroutineManager()->Await(e.get());
        // will get here after the JS callback is called
        if (promiseHandle->IsResolved()) {
            LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been resolved.";
            return promiseHandle->GetValue(currentCoro);
        }
        // rejected
        LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been rejected.";
        auto *exc = promiseHandle->GetValue(currentCoro);
        currentCoro->SetException(exc->GetCoreType());
        return nullptr;
    }

    /* CASE 2. This is a native STS promise */
    promiseHandle->Lock();
    if (!promiseHandle->IsPending()) {
        // already settled!
        promiseHandle->Unlock();

        /**
         * The promise is already resolved of rejected. Further actions:
         *      STS mode:
         *          if resolved: return Promise.value
         *          if rejected: throw Promise.value
         *      JS mode: NOTE!
         *          - suspend coro, create resolved JS promise and put it to the Q, on callback resume the coro
         *            and possibly throw
         *          - JQ::put(current_coro, promise)
         *
         */
        if (promiseHandle->IsResolved()) {
            LOG(DEBUG, COROUTINES) << "Promise::await: promise is already resolved!";
            return promiseHandle->GetValue(currentCoro);
        }
        LOG(DEBUG, COROUTINES) << "Promise::await: promise is already rejected!";
        auto *exc = promiseHandle->GetValue(currentCoro);
        currentCoro->SetException(exc->GetCoreType());
        return nullptr;
    }

    // the promise is not resolved yet
    CoroutineEvent *e = promiseHandle->GetEventPtr();
    if (e != nullptr) {
        /**
         * The promise is linked to come coroutine return value.
         * Further actions:
         *      STS mode:
         *          if resolved: return P.value
         *          if rejected: throw P.value
         *      JS mode: ??? NOTE
         */
        LOG(DEBUG, COROUTINES) << "Promise::await: starting await() for a pending promise...";
        // NOTE(konstanting, #I67QXC): try to make the Promise/Event locking sequence easier for understanding
        e->Lock();
        promiseHandle->Unlock();
        currentCoro->GetCoroutineManager()->Await(e);  // will unlock the event

        // will get here once the promise is resolved
        if (promiseHandle->IsResolved()) {
            LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been resolved.";
            return promiseHandle->GetValue(currentCoro);
        }
        // rejected
        LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been rejected.";
        auto *exc = promiseHandle->GetValue(currentCoro);
        currentCoro->SetException(exc->GetCoreType());
        return nullptr;
    }

    LOG(DEBUG, COROUTINES) << "Promise::await: promise is not linked to an event (standalone)";
    /**
     * This promise is not linked to any coroutine return value (standalone promise).
     * Further actions:
     *      STS mode:
     *          create Event, connect it to promise
     *          CM::Await(event) // who will resolve P and P.event?
     *      JS mode: ??? NOTE
     */

    return nullptr;
}
}  // namespace ark::ets::intrinsics
