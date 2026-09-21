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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_ASYNC_STACK_SNAPSHOT_QUEUE_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_ASYNC_STACK_SNAPSHOT_QUEUE_H

#include <cstdint>

#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/types/ets_async_stack_snapshot.h"
#include "runtime/execution/async_stack_snapshot_handle.h"

namespace ark::ets {

class EtsExecutionContext;
class EtsPromise;

/** Isolates debugger snapshot storage from the core Promise callback queue implementation. */
class EtsPromiseAsyncStackSnapshotQueue {
public:
    EtsPromiseAsyncStackSnapshotQueue() = delete;

    /** Captures a snapshot for a Promise callback submission on the current thread. */
    static EtsAsyncStackSnapshot *CaptureSubmissionSnapshot(EtsExecutionContext *executionCtx);

    /** Creates a rooted debugger handle for a managed snapshot. */
    static AsyncStackSnapshotHandlePtr CreateHandle(EtsExecutionContext *executionCtx, EtsAsyncStackSnapshot *snapshot);

    /** Grows an already existing snapshot queue to the length of the callback queue. */
    static void EnsureCapacity(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                               uint32_t callbackQueueLength);

    /** Appends a callback snapshot, lazily allocating diagnostic storage when it is required. */
    static void Append(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                       EtsHandle<EtsAsyncStackSnapshot> &snapshot, uint32_t index, uint32_t callbackQueueLength);

    /** Creates a rooted handle for the snapshot associated with the callback at index. */
    static AsyncStackSnapshotHandlePtr CreateHandleAt(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                                                      uint32_t index);

    /** Clears diagnostic storage attached to a Promise. */
    static void Clear(EtsExecutionContext *executionCtx, EtsPromise *promise);

private:
    static EtsHandle<EtsObjectArray> Grow(EtsExecutionContext *executionCtx, EtsHandle<EtsPromise> &promise,
                                          uint32_t newQueueLength);
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_ASYNC_STACK_SNAPSHOT_QUEUE_H
