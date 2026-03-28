/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/job_queue.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_events.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::ets::intrinsics {

void SubscribePromiseOnResultObject(EtsPromise *outsidePromise, EtsPromise *internalPromise)
{
    PandaVector<Value> args {Value(outsidePromise), Value(internalPromise)};

    PlatformTypes()->corePromiseSubscribeOnAnotherPromise->GetPandaMethod()->Invoke(ManagedThread::GetCurrent(),
                                                                                    args.data());
}

static void EnsureCapacity(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &hpromise)
{
    ASSERT(hpromise.GetPtr() != nullptr);
    ASSERT(hpromise->IsLocked());
    int queueLength =
        hpromise->GetCallbackQueue(executionCtx) == nullptr ? 0 : hpromise->GetCallbackQueue(executionCtx)->GetLength();
    if (hpromise->GetQueueSize() != queueLength) {
        return;
    }
    auto newQueueLength = queueLength * 2U + 1U;
    auto *objectClass = PlatformTypes(executionCtx)->coreObject;
    auto *newCallbackQueue = EtsObjectArray::Create(objectClass, newQueueLength);
    if (hpromise->GetQueueSize() != 0) {
        hpromise->GetCallbackQueue(executionCtx)->CopyDataTo(newCallbackQueue);
    }
    hpromise->SetCallbackQueue(executionCtx, newCallbackQueue);
    auto *newWorkerDomainQueue = EtsIntArray::Create(newQueueLength);
    if (hpromise->GetQueueSize() != 0) {
        auto *workerDomainQueueData = hpromise->GetWorkerDomainQueue(executionCtx)->GetData<EtsInt *>();
        [[maybe_unused]] auto err =
            memcpy_s(newWorkerDomainQueue->GetData<JobWorkerThreadDomain>(), newQueueLength * sizeof(EtsInt),
                     workerDomainQueueData, queueLength * sizeof(JobWorkerThreadDomain));
        ASSERT(err == EOK);
    }
    hpromise->SetWorkerDomainQueue(executionCtx, newWorkerDomainQueue);
}

void EtsPromiseResolve(EtsPromise *promise, EtsObject *value, EtsBoolean wasLinked)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> hpromise(executionCtx, promise);
    EtsHandle<EtsObject> hvalue(executionCtx, value);

    if (wasLinked == 0) {
        /* When the value is still a Promise, the lock must be unlocked first. */
        hpromise->Lock();
        if (!hpromise->IsPending()) {
            hpromise->Unlock();
            return;
        }
        if (hvalue.GetPtr() != nullptr && hvalue->IsInstanceOf(PlatformTypes(executionCtx)->corePromise)) {
            auto internalPromise = EtsPromise::FromEtsObject(hvalue.GetPtr());
            EtsHandle<EtsPromise> hInternalPromise(executionCtx, internalPromise);
            hpromise->Unlock();
            SubscribePromiseOnResultObject(hpromise.GetPtr(), hInternalPromise.GetPtr());
            return;
        }
        hpromise->Resolve(executionCtx, hvalue.GetPtr());
        hpromise->Unlock();
    } else {
        /* When the value is still a Promise, the lock must be unlocked first. */
        hpromise->Lock();
        if (!hpromise->IsLinked()) {
            hpromise->Unlock();
            return;
        }
        if (hvalue.GetPtr() != nullptr && hvalue->IsInstanceOf(PlatformTypes(executionCtx)->corePromise)) {
            auto internalPromise = EtsPromise::FromEtsObject(hvalue.GetPtr());
            EtsHandle<EtsPromise> hInternalPromise(executionCtx, internalPromise);
            hpromise->ChangeStateToPendingFromLinked();
            hpromise->Unlock();
            SubscribePromiseOnResultObject(hpromise.GetPtr(), hInternalPromise.GetPtr());
            return;
        }
        hpromise->Resolve(executionCtx, hvalue.GetPtr());
        hpromise->Unlock();
    }
}

void EtsPromiseReject(EtsPromise *promise, EtsObject *error, EtsBoolean wasLinked)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> hpromise(executionCtx, promise);
    EtsHandle<EtsObject> herror(executionCtx, error);
    EtsMutex::LockHolder lh(hpromise);
    if ((!hpromise->IsPending() && wasLinked == 0) || (!hpromise->IsLinked() && wasLinked != 0)) {
        return;
    }
    hpromise->Reject(executionCtx, herror.GetPtr());
}

void EtsPromiseSubmitCallback(EtsPromise *promise, EtsObject *callback)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *jobExecCtx = JobExecutionContext::CastFromMutator(executionCtx->GetMT());
    ASSERT(jobExecCtx != nullptr);
    auto workerDomain =
        jobExecCtx->GetWorker()->IsMainWorker() ? JobWorkerThreadDomain::MAIN : JobWorkerThreadDomain::GENERAL;
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> hpromise(executionCtx, promise);
    EtsHandle<EtsObject> hcallback(executionCtx, callback);
    EtsMutex::LockHolder lh(hpromise);
    if (hpromise->IsPending() || hpromise->IsLinked()) {
        EnsureCapacity(executionCtx, hpromise);
        hpromise->SubmitCallback(executionCtx, hcallback.GetPtr(), workerDomain);
        return;
    }
    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        executionCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(hpromise.GetPtr(), executionCtx);
    }
    ASSERT(hpromise->GetQueueSize() == 0);
    ASSERT(hpromise->GetCallbackQueue(executionCtx) == nullptr);
    ASSERT(hpromise->GetWorkerDomainQueue(executionCtx) == nullptr);
    auto groupId = workerDomain == JobWorkerThreadDomain::MAIN
                       ? JobWorkerThreadGroup::FromDomain(jobExecCtx->GetManager(), JobWorkerThreadDomain::MAIN)
                       : JobWorkerThreadGroup::AnyId();
    EtsPromise::LaunchCallback(executionCtx, hcallback.GetPtr(), groupId);
}

static EtsObject *AwaitProxyPromise(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promiseHandle)
{
    /**
     * This is a backed by JS equivalent promise.
     * ETS mode: error, no one can create such a promise!
     * JS mode:
     *      - add a callback to JQ, that will:
     *          - resolve the promise with some value OR reject it
     *          - unblock the executionCtx via event
     *          - schedule();
     *      - create a blocker event and link it to the promise
     *      - block current coroutine on the event
     *      - Schedule();
     *          (the last two steps are actually the cm->await()'s job)
     *      - return promise.value() if resolved or throw() it if rejected
     */
    ASSERT(promiseHandle.GetPtr() != nullptr);
    promiseHandle->Wait();
    ASSERT(!promiseHandle->IsPending() && !promiseHandle->IsLinked());

    // will get here after the JS callback is called
    if (promiseHandle->IsResolved()) {
        LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been resolved.";
        return promiseHandle->GetValue(executionCtx);
    }
    // rejected
    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        executionCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(promiseHandle.GetPtr(),
                                                                                       executionCtx);
    }
    LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been rejected.";
    auto *exc = promiseHandle->GetValue(executionCtx);
    ASSERT(exc != nullptr);
    executionCtx->GetMT()->SetException(exc->GetCoreType());
    return nullptr;
}

EtsObject *EtsAwaitPromise(EtsPromise *promise)
{
    JobExecutionContext *executionCtx = JobExecutionContext::GetCurrent();
    EtsExecutionContext *etsCtx = EtsExecutionContext::FromMT(executionCtx);
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx);
        return nullptr;
    }
    if (executionCtx->GetManager()->IsJobSwitchDisabled()) {
        ThrowEtsException(etsCtx, PlatformTypes(executionCtx)->coreInvalidJobOperationError,
                          "Cannot await in the current context!");
        return nullptr;
    }

    [[maybe_unused]] EtsHandleScope scope(etsCtx);
    EtsHandle<EtsPromise> promiseHandle(etsCtx, promise);

    {
        ScopedNativeCodeThread n(executionCtx);
        executionCtx->GetManager()->ExecuteJobs();
    }

    /* CASE 1. This is a converted JS promise */
    if (promiseHandle->IsProxy()) {
        return AwaitProxyPromise(etsCtx, promiseHandle);
    }

    /* CASE 2. This is a native ETS promise */
    LOG(DEBUG, COROUTINES) << "Promise::await: starting await() for a promise...";
    promiseHandle->Wait();
    ASSERT(!promiseHandle->IsPending() && !promiseHandle->IsLinked());
    LOG(DEBUG, COROUTINES) << "Promise::await: await() finished.";

    /**
     * The promise is already resolved or rejected. Further actions:
     *      ETS mode:
     *          if resolved: return Promise.value
     *          if rejected: throw Promise.value
     *      JS mode: NOTE!
     *          - suspend executionCtx, create resolved JS promise and put it to the Q, on callback resume the
     * executionCtx and possibly throw
     *          - JQ::put(current_coro, promise)
     *
     */
    if (promiseHandle->IsResolved()) {
        LOG(DEBUG, COROUTINES) << "Promise::await: promise is already resolved!";
        return promiseHandle->GetValue(etsCtx);
    }
    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        etsCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(promiseHandle.GetPtr(), etsCtx);
    }
    LOG(DEBUG, COROUTINES) << "Promise::await: promise is already rejected!";
    auto *exc = promiseHandle->GetValue(etsCtx);
    executionCtx->SetException(exc->GetCoreType());
    return nullptr;
}
}  // namespace ark::ets::intrinsics
