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

#include "execution/stackless/stackless_job_manager.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/job_queue.h"
#include "runtime/execution/job_events.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_launch.h"
#include "runtime/execution/job_worker_group.h"
#include "runtime/execution/stackless/suspendable_job.h"

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

static void LaunchJobWithDependency(JobExecutionContext *executionCtx, Job *job, EtsAsyncContext *asyncCtx,
                                    EtsPromise *promise)
{
    auto *etsCtx = EtsExecutionContext::FromMT(executionCtx);
    asyncCtx->SetAwaitee(etsCtx, promise);
    auto *jobMan = executionCtx->GetManager();
    auto *dependency = Runtime::GetCurrent()->GetInternalAllocator()->New<GenericEvent>(jobMan);
    auto groupId = JobWorkerThreadGroup::GenerateExactWorkerId(executionCtx->GetWorker()->GetId());
    [[maybe_unused]] auto launchResult = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId, dependency});
    ASSERT(launchResult == LaunchResult::OK);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    promise->GetEvent(etsCtx)->AddDependency(dependency);
}

static PandaVector<Value> FillEntrypointArgs(Method *method)
{
    PandaVector<Value> args;
    panda_file::ShortyIterator argIt {method->GetShorty()};
    ++argIt;  // skip return value
    if (!method->IsStatic()) {
        args.push_back(Value(static_cast<ObjectHeader *>(nullptr)));
    }
    for (; argIt != panda_file::ShortyIterator(); ++argIt) {
        ASSERT((*argIt).IsValid());
        args.push_back((*argIt).IsReference() ? Value(static_cast<ObjectHeader *>(nullptr)) : Value(0));
    }
    return args;
}

static EtsObject *HandleAwaitStackful(EtsPromise *promise)
{
    auto *etsCtx = EtsExecutionContext::GetCurrent();
    auto *executionCtx = JobExecutionContext::CastFromMutator(etsCtx->GetMT());

    [[maybe_unused]] EtsHandleScope scope(etsCtx);
    EtsHandle<EtsPromise> promiseHandle(etsCtx, promise);

    {
        ScopedNativeCodeThread n(executionCtx);
        executionCtx->GetManager()->ExecuteJobs();
    }

    /* CASE 2. This is a native ETS promise */
    LOG(DEBUG, COROUTINES) << "Promise::await: starting await() for a promise...";
    promiseHandle->GetEvent(etsCtx)->Wait();
    ASSERT(!promiseHandle->IsPending() && !promiseHandle->IsLinked());
    LOG(DEBUG, COROUTINES) << "Promise::await: await() finished.";

    /**
     * The promise is already resolved or rejected. Further actions:
     *      ETS mode:
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
        return promiseHandle->GetValue(etsCtx);
    }
    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        etsCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(promiseHandle.GetPtr(), etsCtx);
    }
    LOG(DEBUG, COROUTINES) << "Promise::await: promise is already rejected!";
    auto *exc = promiseHandle->GetValue(etsCtx);
    etsCtx->GetMT()->SetException(exc->GetCoreType());
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

    if (Runtime::GetCurrent()->GetOptions().GetCoroutineImpl() == "stackful") {
        return HandleAwaitStackful(promise);
    }

    auto *asyncCtx = EtsAsyncContext::GetCurrent(etsCtx);
    if (asyncCtx != nullptr) {
        auto *stacklessJobMan = static_cast<StacklessJobManager *>(executionCtx->GetManager());
        auto *dependency = Runtime::GetCurrent()->GetInternalAllocator()->New<GenericEvent>(stacklessJobMan);
        stacklessJobMan->AwaitAsynchronous(dependency);
        promise->GetEvent(etsCtx)->AddDependency(dependency);
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        return asyncCtx;
    }

    [[maybe_unused]] EtsHandleScope scope(etsCtx);
    EtsHandle<EtsPromise> promiseHandle(etsCtx, promise);

    asyncCtx = EtsAsyncContext::Create(etsCtx);
    if UNLIKELY (asyncCtx == nullptr) {
        return nullptr;
    }
    auto refStor = EtsExecutionContext::FromMT(executionCtx)->GetPandaAniEnv()->GetEtsReferenceStorage();
    auto *aCtxRef = EtsReference::CastToReference(refStor->NewEtsRef(asyncCtx, EtsReference::EtsObjectType::LOCAL));
    auto *asyncMethod = executionCtx->GetCurrentFrame()->GetMethod();
    auto args = FillEntrypointArgs(asyncMethod);
    auto epInfo = Job::ManagedEntrypointInfo {nullptr, asyncMethod, std::move(args)};
    auto *job = executionCtx->GetManager()->CreateJob<SuspendableJob>(asyncMethod->GetFullName(), std::move(epInfo));
    job->SetSuspensionContext(aCtxRef);
    LaunchJobWithDependency(executionCtx, job, asyncCtx, promiseHandle.GetPtr());
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return asyncCtx;
}

}  // namespace ark::ets::intrinsics
