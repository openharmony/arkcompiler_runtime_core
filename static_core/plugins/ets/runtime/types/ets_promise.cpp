/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "runtime/execution/job_execution_context.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/execution/job_events.h"
#include <utility>

#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_promise_async_stack_snapshot_queue.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_exceptions.h"

namespace ark::ets {

/*static*/
EtsPromise *EtsPromise::Create(EtsExecutionContext *executionCtx)
{
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    auto *klass = PlatformTypes(executionCtx)->corePromise;
    auto hPromise =
        EtsHandle<EtsPromise>(executionCtx, EtsPromise::FromEtsObject(EtsObject::Create(executionCtx, klass)));
    ASSERT(hPromise.GetPtr() != nullptr);
    auto *mutex = EtsMutex::Create(executionCtx);
    hPromise->SetMutex(executionCtx, mutex);
    auto *event = EtsEventWithDependencies::Create(executionCtx);
    hPromise->SetEvent<CoroutineMode::STACKLESS>(executionCtx, event);
    return hPromise.GetPtr();
}

void EtsPromise::SubmitCallback(EtsExecutionContext *executionCtx, EtsObject *callback,
                                JobWorkerThreadDomain workerDomain)
{
    ASSERT(IsLocked());
    ASSERT(queueSize_ < static_cast<int>(GetCallbackQueue(executionCtx)->GetLength()));
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> promiseHandle(executionCtx, this);
    EtsHandle<EtsObject> callbackHandle(executionCtx, callback);
    EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, GetCallbackQueue(executionCtx));
    EtsHandle<EtsIntArray> workerDomainQueueHandle(executionCtx, GetWorkerDomainQueue(executionCtx));

    workerDomainQueueHandle->Set(queueSize_, static_cast<int>(workerDomain));
    callbackQueueHandle->Set(queueSize_, callbackHandle.GetPtr());
    promiseHandle->queueSize_++;
}

/* static */
void EtsPromise::OnPromiseCompletion(EtsExecutionContext *executionCtx, EtsPromise *promise)
{
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> promiseHandle(executionCtx, promise);
    EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, promiseHandle->GetCallbackQueue(executionCtx));
    EtsHandle<EtsIntArray> workerDomainQueueHandle(executionCtx, promiseHandle->GetWorkerDomainQueue(executionCtx));
    auto queueSize = promiseHandle->GetQueueSize();
    ASSERT(queueSize == 0 || callbackQueueHandle.GetPtr() != nullptr);
    ASSERT(queueSize == 0 || workerDomainQueueHandle.GetPtr() != nullptr);

    if (promiseHandle->GetState() == STATE_REJECTED && queueSize == 0 && !promiseHandle->IsHandled()) {
        executionCtx->GetPandaVM()->GetUnhandledObjectManager()->AddRejectedPromise(promiseHandle.GetPtr(),
                                                                                    executionCtx);
    }

    // Unblock awaitee jobs
    if (Runtime::GetCurrent()->GetOptions().GetCoroutineImpl() == "stackful") {
        promiseHandle->GetEvent<CoroutineMode::STACKFUL>(executionCtx)->Fire();
    } else {
        promiseHandle->GetEvent<CoroutineMode::STACKLESS>(executionCtx)->ResolveDependencies();
    }

    if (queueSize == 0) {
        promiseHandle->ClearQueues(executionCtx);
        return;
    }

    for (int idx = 0; idx < queueSize; ++idx) {
        auto *thenCallback = callbackQueueHandle->Get(idx);
        EtsHandle<EtsObject> hThenCallback(executionCtx, thenCallback);
        auto asyncDebuggerStack =
            EtsPromiseAsyncStackSnapshotQueue::CreateHandleAt(executionCtx, promiseHandle, static_cast<uint32_t>(idx));
        auto workerDomain = static_cast<JobWorkerThreadDomain>(workerDomainQueueHandle->Get(idx));
        auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
        ASSERT(workerDomain == JobWorkerThreadDomain::MAIN || workerDomain == JobWorkerThreadDomain::GENERAL);
        auto groupId = workerDomain == JobWorkerThreadDomain::MAIN
                           ? JobWorkerThreadGroup::FromDomain(jobMan, JobWorkerThreadDomain::MAIN)
                           : JobWorkerThreadGroup::AnyId();
        EtsPromise::LaunchCallback(executionCtx, hThenCallback, groupId, std::move(asyncDebuggerStack));
    }
    promiseHandle->ClearQueues(executionCtx);
}

void EtsPromise::ClearQueues(EtsExecutionContext *executionCtx)
{
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsPromise, callbackQueue_), nullptr);
    ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsPromise, workerDomainQueue_), nullptr);
    EtsPromiseAsyncStackSnapshotQueue::Clear(executionCtx, this);
    queueSize_ = 0;
}

/* static */
void EtsPromise::LaunchCallback(EtsExecutionContext *executionCtx, EtsHandle<EtsObject> &handledCb,
                                const JobWorkerThreadGroup::Id &groupId, AsyncStackSnapshotHandlePtr asyncDebuggerStack)
{
    // Launch callback in its own coroutine
    if (!handledCb->GetClass()->IsFunction()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreTypeError,
                          "Method have to be instance of function");
    }

    EtsMethod *etsmethod = PlatformTypes(executionCtx)->coreFunctionUnsafeCall;
    auto *method = EtsMethod::ToRuntimeMethod(handledCb->GetClass()->ResolveVirtualMethod(etsmethod));

    auto *argArray = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, 0U);
    if (UNLIKELY(argArray == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }

    auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    auto *event = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, jobMan);
    auto args = PandaVector<Value> {Value(handledCb->GetCoreType()), Value(argArray->GetCoreType())};
    auto epInfo = Job::ManagedEntrypointInfo {event, method, std::move(args)};
    auto *job = jobMan->CreateJob(method->GetFullName(), std::move(epInfo), EtsCoroutine::PROMISE_CALLBACK);
    LaunchParams launchParams {job->GetPriority(), groupId};
    launchParams.asyncDebuggerStack = std::move(asyncDebuggerStack);
    auto launchResult = jobMan->Launch(job, launchParams);
    if UNLIKELY (launchResult != LaunchResult::OK) {
        jobMan->HandleLaunchResultManaged(launchResult);
        jobMan->DestroyJob(job);
    }
}

}  // namespace ark::ets
