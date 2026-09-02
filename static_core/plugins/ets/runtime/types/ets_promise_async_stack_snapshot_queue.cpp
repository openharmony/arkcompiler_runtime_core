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
#include "plugins/ets/runtime/types/ets_promise_async_stack_snapshot_queue.h"

#include "plugins/ets/runtime/dfx/static_async_stack_snapshot_manager.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_async_stack_snapshot.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/include/runtime.h"

namespace ark::ets {

EtsAsyncStackSnapshot *EtsPromiseAsyncStackSnapshotQueue::CaptureSubmissionSnapshot(EtsExecutionContext *executionCtx)
{
    auto *runtime = Runtime::GetCurrent();
    if (runtime == nullptr || !runtime->IsDebugMode() ||
        !executionCtx->GetPandaVM()->GetAsyncDebuggerConfig().IsCaptureEnabled()) {
        return nullptr;
    }

    auto *description = StaticAsyncStackSnapshotManager::ClassifyPromiseSubmission(executionCtx);
    if (description == nullptr) {
        return nullptr;
    }
    return StaticAsyncStackSnapshotManager::CaptureManaged(executionCtx, description, false);
}

AsyncStackSnapshotHandlePtr EtsPromiseAsyncStackSnapshotQueue::CreateHandle(EtsExecutionContext *executionCtx,
                                                                            EtsAsyncStackSnapshot *snapshot)
{
    return StaticAsyncStackSnapshotManager::CreateHandle(executionCtx, snapshot);
}

void EtsPromiseAsyncStackSnapshotQueue::EnsureCapacity(EtsExecutionContext *executionCtx,
                                                       EtsHandle<EtsPromise> &promise, uint32_t callbackQueueLength)
{
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObjectArray> snapshotQueueHandle(executionCtx, promise->GetAsyncStackSnapshotQueue(executionCtx));
    if (snapshotQueueHandle.GetPtr() == nullptr || snapshotQueueHandle->GetLength() >= callbackQueueLength) {
        return;
    }
    (void)Grow(executionCtx, promise, callbackQueueLength);
}

void EtsPromiseAsyncStackSnapshotQueue::Append(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                                               EtsHandle<EtsAsyncStackSnapshot> &snapshot, uint32_t index,
                                               uint32_t callbackQueueLength)
{
    if (snapshot.GetPtr() == nullptr) {
        return;
    }

    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObjectArray> snapshotQueueHandle(executionCtx, promise->GetAsyncStackSnapshotQueue(executionCtx));
    if (snapshotQueueHandle.GetPtr() == nullptr || index >= snapshotQueueHandle->GetLength()) {
        snapshotQueueHandle = Grow(executionCtx, promise, callbackQueueLength);
    }
    if (snapshotQueueHandle.GetPtr() != nullptr && index < snapshotQueueHandle->GetLength()) {
        snapshotQueueHandle->Set(index, snapshot.GetPtr());
    }
}

AsyncStackSnapshotHandlePtr EtsPromiseAsyncStackSnapshotQueue::CreateHandleAt(EtsExecutionContext *executionCtx,
                                                                              EtsHandle<EtsPromise> &promise,
                                                                              uint32_t index)
{
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObjectArray> snapshotQueueHandle(executionCtx, promise->GetAsyncStackSnapshotQueue(executionCtx));
    if (snapshotQueueHandle.GetPtr() == nullptr || index >= snapshotQueueHandle->GetLength()) {
        return AsyncStackSnapshotHandlePtr {};
    }

    auto *snapshotObject = snapshotQueueHandle->Get(index);
    auto *snapshot = snapshotObject == nullptr ? nullptr : EtsAsyncStackSnapshot::FromEtsObject(snapshotObject);
    EtsHandle<EtsAsyncStackSnapshot> snapshotHandle(executionCtx, snapshot);
    return CreateHandle(executionCtx, snapshotHandle.GetPtr());
}

void EtsPromiseAsyncStackSnapshotQueue::Clear(EtsExecutionContext *executionCtx, EtsPromise *promise)
{
    promise->ClearAsyncStackSnapshotQueue(executionCtx);
}

EtsHandle<EtsObjectArray> EtsPromiseAsyncStackSnapshotQueue::Grow(EtsExecutionContext *executionCtx,
                                                                  EtsHandle<EtsPromise> &promise,
                                                                  uint32_t newQueueLength)
{
    auto *mt = executionCtx->GetMT();
    const bool hadPendingException = mt->HasPendingException();
    auto *newSnapshotQueue =
        EtsObjectArray::Create(PlatformTypes(executionCtx)->coreAsyncStackSnapshot, newQueueLength);
    if (newSnapshotQueue == nullptr) {
        ASSERT(mt->HasPendingException());
        StaticAsyncStackSnapshotManager::DiscardOptionalSnapshotFailure(executionCtx, hadPendingException);
        promise->ClearAsyncStackSnapshotQueue(executionCtx);
        return EtsHandle<EtsObjectArray> {};
    }

    EtsHandle<EtsObjectArray> newSnapshotQueueHandle(executionCtx, newSnapshotQueue);
    auto *oldSnapshotQueue = promise->GetAsyncStackSnapshotQueue(executionCtx);
    if (oldSnapshotQueue != nullptr) {
        oldSnapshotQueue->CopyDataTo(newSnapshotQueueHandle.GetPtr());
    }
    promise->SetAsyncStackSnapshotQueue(executionCtx, newSnapshotQueueHandle.GetPtr());
    return newSnapshotQueueHandle;
}

}  // namespace ark::ets
