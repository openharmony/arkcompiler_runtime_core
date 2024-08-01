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
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::ets::intrinsics {

void SubscribePromiseOnResultObject(EtsPromise *outsidePromise, EtsPromise *internalPromise)
{
    PandaVector<Value> args {Value(outsidePromise), Value(internalPromise)};

    auto subscribeOnAnotherPromise = [&args]() {
        EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetSubscribeOnAnotherPromiseMethod()->Invoke(
            EtsCoroutine::GetCurrent(), args.data());
    };

    auto *mainT = EtsCoroutine::GetCurrent()->GetPandaVM()->GetCoroutineManager()->GetMainThread();
    Coroutine *mainCoro = Coroutine::CastFromThread(mainT);
    Coroutine *current = Coroutine::GetCurrent();
    if (current != mainCoro && mainCoro->GetId() == current->GetId()) {
        // Call ExecuteOnThisContext is possible only in the same thread.
        mainCoro->GetContext<StackfulCoroutineContext>()->ExecuteOnThisContext(
            &subscribeOnAnotherPromise, EtsCoroutine::GetCurrent()->GetContext<StackfulCoroutineContext>());
    } else {
        subscribeOnAnotherPromise();
    }
}

static void EnsureCapacity(EtsCoroutine *coro, EtsHandle<EtsPromise> &hpromise)
{
    ASSERT(hpromise->IsLocked());
    int queueLength = hpromise->GetCallbackQueue(coro) == nullptr ? 0 : hpromise->GetCallbackQueue(coro)->GetLength();
    if (hpromise->GetQueueSize() != queueLength) {
        return;
    }
    auto newQueueLength = queueLength * 2U + 1U;
    auto *objectClass = coro->GetPandaVM()->GetClassLinker()->GetObjectClass();
    auto *newCallbackQueue = EtsObjectArray::Create(objectClass, newQueueLength);
    if (hpromise->GetQueueSize() != 0) {
        hpromise->GetCallbackQueue(coro)->CopyDataTo(newCallbackQueue);
    }
    hpromise->SetCallbackQueue(coro, newCallbackQueue);
    auto *newLaunchModeQueue = EtsIntArray::Create(newQueueLength);
    if (hpromise->GetQueueSize() != 0) {
        auto *launchModeQueueData = hpromise->GetLaunchModeQueue(coro)->GetData<EtsCoroutine *>();
        [[maybe_unused]] auto err =
            memcpy_s(newLaunchModeQueue->GetData<CoroutineLaunchMode>(), newQueueLength * sizeof(EtsInt),
                     launchModeQueueData, queueLength * sizeof(CoroutineLaunchMode));
        ASSERT(err == EOK);
    }
    hpromise->SetLaunchModeQueue(coro, newLaunchModeQueue);
}

void EtsPromiseResolve(EtsPromise *promise, EtsObject *value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    EtsHandle<EtsObject> hvalue(coro, value);
    EtsMutex::LockHolder lh(hpromise);
    if (hpromise->GetState() != EtsPromise::STATE_PENDING) {
        return;
    }
    if (hvalue.GetPtr() != nullptr && hvalue->IsInstanceOf(coro->GetPandaVM()->GetClassLinker()->GetPromiseClass())) {
        auto internalPromise = EtsPromise::FromEtsObject(hvalue.GetPtr());
        EtsHandle<EtsPromise> hInternalPromise(coro, internalPromise);
        if (hInternalPromise->IsPending() || coro->GetCoroutineManager()->IsJsMode()) {
            SubscribePromiseOnResultObject(hpromise.GetPtr(), hInternalPromise.GetPtr());
            return;
        }
        if (hInternalPromise->IsRejected()) {
            hpromise->Reject(coro, hInternalPromise->GetValue(coro));
            return;
        }
        // We can use internal promise's value as return value
        hvalue = EtsHandle<EtsObject>(coro, hInternalPromise->GetValue(coro));
    }
    hpromise->Resolve(coro, hvalue.GetPtr());
}

void EtsPromiseReject(EtsPromise *promise, EtsObject *error)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    EtsHandle<EtsObject> herror(coro, error);
    EtsMutex::LockHolder lh(hpromise);
    if (hpromise->GetState() != EtsPromise::STATE_PENDING) {
        return;
    }
    hpromise->Reject(coro, herror.GetPtr());
}

void EtsPromiseSubmitCallback(EtsPromise *promise, EtsObject *callback)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *coroManager = coro->GetCoroutineManager();
    auto launchMode =
        coro == coroManager->GetMainThread() ? CoroutineLaunchMode::MAIN_WORKER : CoroutineLaunchMode::DEFAULT;
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    EtsHandle<EtsObject> hcallback(coro, callback);
    EtsMutex::LockHolder lh(hpromise);
    if (hpromise->GetState() == EtsPromise::STATE_PENDING) {
        EnsureCapacity(coro, hpromise);
        hpromise->SubmitCallback(coro, hcallback.GetPtr(), launchMode);
        return;
    }
    ASSERT(hpromise->GetQueueSize() == 0);
    ASSERT(hpromise->GetCallbackQueue(coro) == nullptr);
    ASSERT(hpromise->GetLaunchModeQueue(coro) == nullptr);
    EtsPromise::LaunchCallback(coro, hcallback.GetPtr(), launchMode);
}

void EtsPromiseCreateLink(EtsObject *source, EtsPromise *target)
{
    EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
    auto *jobQueue = currentCoro->GetPandaVM()->GetJobQueue();
    if (jobQueue != nullptr) {
        jobQueue->CreateLink(source, target->AsObject());
    }
}

static EtsObject *AwaitProxyPromise(EtsCoroutine *currentCoro, EtsHandle<EtsPromise> &promiseHandle)
{
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

    promiseHandle->Wait();
    ASSERT(!promiseHandle->IsPending());

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
        return AwaitProxyPromise(currentCoro, promiseHandle);
    }

    /* CASE 2. This is a native STS promise */
    LOG(DEBUG, COROUTINES) << "Promise::await: starting await() for a promise...";
    promiseHandle->Wait();
    ASSERT(!promiseHandle->IsPending());
    LOG(DEBUG, COROUTINES) << "Promise::await: await() finished.";

    /**
     * The promise is already resolved or rejected. Further actions:
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
}  // namespace ark::ets::intrinsics
