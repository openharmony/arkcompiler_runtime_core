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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_DFX_STATIC_ASYNC_STACK_SNAPSHOT_MANAGER_H
#define PANDA_PLUGINS_ETS_RUNTIME_DFX_STATIC_ASYNC_STACK_SNAPSHOT_MANAGER_H

#include <cstdint>

#include "runtime/execution/async_stack_snapshot_handle.h"

namespace ark::ets {

class EtsAsyncStackSnapshot;
class EtsExecutionContext;
class PandaEtsVM;

/**
 * Creates bounded immutable managed snapshots from the current synchronous stack.
 *
 * All allocations are performed with an EtsHandleScope, and the resulting snapshot is physically
 * bounded before it becomes visible to callers.
 */
class StaticAsyncStackSnapshotManager {
public:
    StaticAsyncStackSnapshotManager() = delete;

    static const char *ClassifyPromiseSubmission(EtsExecutionContext *executionCtx);
    static EtsAsyncStackSnapshot *Capture(EtsExecutionContext *executionCtx, const char *description,
                                          bool skipTopFrame);

    /**
     * Captures a snapshot and discards a new OOM exception raised by optional debugger diagnostics.
     * A pending exception that existed before the call is always preserved.
     */
    static EtsAsyncStackSnapshot *CaptureManaged(EtsExecutionContext *executionCtx, const char *description,
                                                 bool skipTopFrame);

    /** Creates a rooted handle and applies the same optional-diagnostics failure policy. */
    static AsyncStackSnapshotHandlePtr CreateHandle(EtsExecutionContext *executionCtx, EtsAsyncStackSnapshot *snapshot);

    /** Captures and roots a snapshot in one operation for native launch paths. */
    static AsyncStackSnapshotHandlePtr CaptureHandle(EtsExecutionContext *executionCtx, const char *description,
                                                     bool skipTopFrame);

    static void DiscardOptionalSnapshotFailure(EtsExecutionContext *executionCtx, bool hadPendingException);
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_DFX_STATIC_ASYNC_STACK_SNAPSHOT_MANAGER_H
