/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "intrinsic_await_promise_impl.h"
#include "ets_handle.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_async_context.h"
#include "runtime/include/exceptions.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/stackless/stackless_job_manager.h"
#include "runtime/execution/stackless/suspendable_job.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"

namespace ark::ets::intrinsics::helpers {

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
    promise->GetEvent<CoroutineMode::STACKLESS>(etsCtx)->AddDependency(dependency);
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
    promiseHandle->GetEvent<CoroutineMode::STACKFUL>(etsCtx)->Wait();
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
    etsCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(promiseHandle.GetPtr(), etsCtx);
    LOG(DEBUG, COROUTINES) << "Promise::await: promise is already rejected!";
    auto *exc = promiseHandle->GetValue(etsCtx);
    etsCtx->GetMT()->SetException(exc->GetCoreType());
    return nullptr;
}

EtsObject *EtsAwaitPromiseImpl(EtsPromise *promise, int32_t refCount, int32_t primCount, int32_t pc)
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
        asyncCtx->SetAwaitee(etsCtx, promise);
        stacklessJobMan->AwaitAsynchronous(dependency);
        promise->GetEvent<CoroutineMode::STACKLESS>(etsCtx)->AddDependency(dependency);
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        return asyncCtx;
    }

    [[maybe_unused]] EtsHandleScope scope(etsCtx);
    EtsHandle<EtsPromise> promiseHandle(etsCtx, promise);
    asyncCtx = EtsAsyncContext::Create(etsCtx, refCount, primCount, pc);
    EtsHandle<EtsAsyncContext> asyncCtxHandle(etsCtx, asyncCtx);
    if UNLIKELY (asyncCtxHandle.GetPtr() == nullptr) {
        return nullptr;
    }
    auto refStor = EtsExecutionContext::FromMT(executionCtx)->GetPandaAniEnv()->GetEtsReferenceStorage();
    auto *aCtxRef = EtsReference::CastToReference(refStor->NewEtsRef(asyncCtx, EtsReference::EtsObjectType::LOCAL));
    auto *asyncMethod = StackWalker::Create(etsCtx->GetMT()).GetMethod();
    auto args = FillEntrypointArgs(asyncMethod);
    auto epInfo = Job::ManagedEntrypointInfo {nullptr, asyncMethod, std::move(args)};
    auto *job = executionCtx->GetManager()->CreateJob<SuspendableJob>(asyncMethod->GetFullName(), std::move(epInfo));
    job->SetSuspensionContext(aCtxRef);
    LaunchJobWithDependency(executionCtx, job, asyncCtxHandle.GetPtr(), promiseHandle.GetPtr());
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return asyncCtxHandle.GetPtr();
}

static EtsObject *HandleSettledPromise(EtsExecutionContext *etsCtx, EtsHandle<EtsPromise> &promiseHandle)
{
    ASSERT(!promiseHandle->IsPending() && !promiseHandle->IsLinked());
    if (promiseHandle->IsResolved()) {
        LOG(DEBUG, COROUTINES) << "Promise::awaitSync: promise is resolved!";
        return promiseHandle->GetValue(etsCtx);
    }
    etsCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(promiseHandle.GetPtr(), etsCtx);
    LOG(DEBUG, COROUTINES) << "Promise::awaitSync: promise is rejected!";
    auto *exc = promiseHandle->GetValue(etsCtx);
    etsCtx->GetMT()->SetException(exc->GetCoreType());
    return nullptr;
}

EtsObject *EtsAwaitPromiseSyncImpl(EtsPromise *promise)
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

    [[maybe_unused]] EtsHandleScope scope(etsCtx);
    EtsHandle<EtsPromise> promiseHandle(etsCtx, promise);

    LOG(DEBUG, COROUTINES) << "Promise::awaitSync: starting blocking await for a promise...";

    promiseHandle->GetEvent<CoroutineMode::STACKLESS>(etsCtx)->WaitBlocking();
    ASSERT(!promiseHandle->IsPending() && !promiseHandle->IsLinked());
    LOG(DEBUG, COROUTINES) << "Promise::awaitSync: blocking await finished.";

    return HandleSettledPromise(etsCtx, promiseHandle);
}

}  // namespace ark::ets::intrinsics::helpers
