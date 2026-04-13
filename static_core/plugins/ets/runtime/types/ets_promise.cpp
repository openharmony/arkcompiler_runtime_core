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
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
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
    hPromise->SetEvent(executionCtx, event);
    return hPromise.GetPtr();
}

void EtsPromise::OnPromiseCompletion(EtsExecutionContext *executionCtx)
{
    auto *cbQueue = GetCallbackQueue(executionCtx);
    auto *workerDomainQueue = GetWorkerDomainQueue(executionCtx);
    auto queueSize = GetQueueSize();
    ASSERT(queueSize == 0 || cbQueue != nullptr);
    ASSERT(queueSize == 0 || workerDomainQueue != nullptr);

    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)) &&
        state_ == STATE_REJECTED && queueSize == 0) {
        executionCtx->GetPandaVM()->GetUnhandledObjectManager()->AddRejectedPromise(this, executionCtx);
    }

    // Unblock awaitee jobs
    if (Runtime::GetCurrent()->GetOptions().GetCoroutineImpl() == "stackful") {
        GetEvent<CoroutineMode::STACKFUL>(executionCtx)->Fire();
    } else {
        GetEvent<CoroutineMode::STACKLESS>(executionCtx)->ResolveDependencies();
    }

    for (int idx = 0; idx < queueSize; ++idx) {
        auto *thenCallback = cbQueue->Get(idx);
        auto workerDomain = static_cast<JobWorkerThreadDomain>(workerDomainQueue->Get(idx));
        auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
        ASSERT(workerDomain == JobWorkerThreadDomain::MAIN || workerDomain == JobWorkerThreadDomain::GENERAL);
        auto groupId = workerDomain == JobWorkerThreadDomain::MAIN
                           ? JobWorkerThreadGroup::FromDomain(jobMan, JobWorkerThreadDomain::MAIN)
                           : JobWorkerThreadGroup::AnyId();
        EtsPromise::LaunchCallback(executionCtx, thenCallback, groupId);
    }
    ClearQueues(executionCtx);
}

/* static */
void EtsPromise::LaunchCallback(EtsExecutionContext *executionCtx, EtsObject *callback,
                                const JobWorkerThreadGroup::Id &groupId)
{
    // Launch callback in its own coroutine
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    auto handledCb = EtsHandle<EtsObject>(executionCtx, callback);

    if (!handledCb->GetClass()->IsFunction()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreTypeError,
                          "Method have to be instance of function");
    }

    EtsMethod *etsmethod = PlatformTypes(executionCtx)->coreFunctionUnsafeCall;
    auto *method = EtsMethod::ToRuntimeMethod(handledCb->GetClass()->ResolveVirtualMethod(etsmethod));

    auto argArray = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, 0U);
    if (UNLIKELY(argArray == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }

    auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    auto *event = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, jobMan);
    auto args = PandaVector<Value> {Value(handledCb->GetCoreType()), Value(argArray->GetCoreType())};
    auto epInfo = Job::ManagedEntrypointInfo {event, method, std::move(args)};
    auto job = jobMan->CreateJob(method->GetFullName(), std::move(epInfo), EtsCoroutine::PROMISE_CALLBACK);
    auto launchResult = jobMan->Launch(job, LaunchParams {job->GetPriority(), groupId});
    if (UNLIKELY(launchResult == LaunchResult::RESOURCE_LIMIT_EXCEED)) {
        jobMan->DestroyJob(job);
    }
}

}  // namespace ark::ets
