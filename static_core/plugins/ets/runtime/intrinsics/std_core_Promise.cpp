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

#include <utility>

#include "intrinsics.h"
#include "intrinsics/helpers/intrinsic_promise_impl.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_promise_async_stack_snapshot_queue.h"
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
#include "runtime/execution/job_worker_thread.h"
#include "runtime/execution/stackless/suspendable_job.h"

namespace ark::ets::intrinsics::helpers {

namespace {

struct QueueGrowthDimensions {
    size_t newQueueLength;
    size_t oldQueueLength;
    size_t queueSize;
};

bool GrowBusinessQueues(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                        const QueueGrowthDimensions &dimensions)
{
    EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, promise->GetCallbackQueue(executionCtx));
    EtsHandle<EtsIntArray> workerDomainQueueHandle(executionCtx, promise->GetWorkerDomainQueue(executionCtx));

    auto *newCallbackQueue = EtsObjectArray::Create(PlatformTypes(executionCtx)->coreObject, dimensions.newQueueLength);
    if (newCallbackQueue == nullptr) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return false;
    }
    EtsHandle<EtsObjectArray> newCallbackQueueHandle(executionCtx, newCallbackQueue);

    auto *newWorkerDomainQueue = EtsIntArray::Create(dimensions.newQueueLength);
    if (newWorkerDomainQueue == nullptr) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return false;
    }
    EtsHandle<EtsIntArray> newWorkerDomainQueueHandle(executionCtx, newWorkerDomainQueue);

    if (dimensions.queueSize != 0) {
        callbackQueueHandle->CopyDataTo(newCallbackQueueHandle.GetPtr());
        auto *workerDomainQueueData = workerDomainQueueHandle->GetData<EtsInt *>();
        [[maybe_unused]] auto err = memcpy_s(newWorkerDomainQueueHandle->GetData<JobWorkerThreadDomain>(),
                                             dimensions.newQueueLength * sizeof(EtsInt), workerDomainQueueData,
                                             dimensions.oldQueueLength * sizeof(JobWorkerThreadDomain));
        ASSERT(err == EOK);
    }

    promise->SetCallbackQueue(executionCtx, newCallbackQueueHandle.GetPtr());
    promise->SetWorkerDomainQueue(executionCtx, newWorkerDomainQueueHandle.GetPtr());
    return true;
}

}  // namespace

void EnsurePromiseCapacity(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise)
{
    ASSERT(promise.GetPtr() != nullptr);
    ASSERT(promise->IsLocked());
    EtsHandle<EtsObjectArray> callbackQueueHandle(executionCtx, promise->GetCallbackQueue(executionCtx));

    const auto queueLength = callbackQueueHandle.GetPtr() == nullptr ? 0U : callbackQueueHandle->GetLength();
    const auto queueSize = static_cast<uint32_t>(promise->GetQueueSize());
    if (queueSize != queueLength) {
        EtsPromiseAsyncStackSnapshotQueue::EnsureCapacity(executionCtx, promise, queueLength);
        return;
    }

    const auto newQueueLength = queueLength * 2U + 1U;
    QueueGrowthDimensions dimensions {newQueueLength, queueLength, queueSize};
    if (!GrowBusinessQueues(executionCtx, promise, dimensions)) {
        return;
    }
    EtsPromiseAsyncStackSnapshotQueue::EnsureCapacity(executionCtx, promise, newQueueLength);
}

}  // namespace ark::ets::intrinsics::helpers

namespace ark::ets::intrinsics {

void EtsPromiseResolve(EtsPromise *promise, EtsObject *value, EtsBoolean wasLinked)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    if (wasLinked == 1 && !promise->TryChangeStateFromLinkedToPending()) {
        return;
    }
    helpers::EtsPromiseResolveImpl(executionCtx, promise, value);
}

void EtsPromiseReject(EtsPromise *promise, EtsObject *error, EtsBoolean wasLinked)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, executionCtx->GetMT());
        return;
    }
    if (wasLinked == 1 && !promise->TryChangeStateFromLinkedToPending()) {
        return;
    }
    helpers::EtsPromiseRejectImpl(executionCtx, promise, error);
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
    EtsHandle<EtsAsyncStackSnapshot> asyncStackSnapshotHandle(
        executionCtx, EtsPromiseAsyncStackSnapshotQueue::CaptureSubmissionSnapshot(executionCtx));

    EtsMutex::LockHolder lh(hpromise);
    hpromise->SetHandled();
    if (hpromise->IsPending() || hpromise->IsLinked()) {
        helpers::EnsurePromiseCapacity(executionCtx, hpromise);
        if (UNLIKELY(executionCtx->GetMT()->HasPendingException())) {
            return;
        }
        hpromise->SubmitCallback(executionCtx, hcallback.GetPtr(), workerDomain);
        auto callbackIndex = static_cast<uint32_t>(hpromise->GetQueueSize() - 1);
        auto callbackQueueLength = hpromise->GetCallbackQueue(executionCtx)->GetLength();
        EtsPromiseAsyncStackSnapshotQueue::Append(executionCtx, hpromise, asyncStackSnapshotHandle, callbackIndex,
                                                  callbackQueueLength);
        return;
    }
    executionCtx->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(hpromise.GetPtr(), executionCtx);
    ASSERT(hpromise->GetQueueSize() == 0);
    ASSERT(hpromise->GetCallbackQueue(executionCtx) == nullptr);
    ASSERT(hpromise->GetWorkerDomainQueue(executionCtx) == nullptr);
    auto groupId = workerDomain == JobWorkerThreadDomain::MAIN
                       ? JobWorkerThreadGroup::FromDomain(jobExecCtx->GetManager(), JobWorkerThreadDomain::MAIN)
                       : JobWorkerThreadGroup::AnyId();
    auto asyncDebuggerStack =
        EtsPromiseAsyncStackSnapshotQueue::CreateHandle(executionCtx, asyncStackSnapshotHandle.GetPtr());
    EtsPromise::LaunchCallback(executionCtx, hcallback, groupId, std::move(asyncDebuggerStack));
}

EtsObject *EtsAwaitPromise(EtsPromise *promise)
{
    return helpers::EtsAwaitPromiseImpl(promise, -1, -1, -1);
}

EtsObject *EtsAwaitPromiseSync(EtsPromise *promise)
{
    return helpers::EtsAwaitPromiseSyncImpl(promise);
}
}  // namespace ark::ets::intrinsics
