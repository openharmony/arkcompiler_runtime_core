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

// CMC GC (common_runtime) implementations of ETS GC intrinsics
// For shared intrinsics see std_core_gc.cpp
// For default GC (static_core) implementations see std_core_default_gc.cpp
#include "common_interfaces/base_runtime.h"
#include "common_interfaces/thread/mutator-inl.h"
#include "libarkbase/utils/utils.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "common_components/common/scoped_object_access.h"

namespace ark::ets::intrinsics {

static inline size_t ClampToSizeT(EtsLong n)
{
    if constexpr (sizeof(EtsLong) > sizeof(size_t)) {
        if (UNLIKELY(n > static_cast<EtsLong>(std::numeric_limits<size_t>::max()))) {
            return std::numeric_limits<size_t>::max();
        }
    }
    return n;
}

// Convert ETS cause int to (GCReason, GCType) pair
// ETS Cause enum: YOUNG=0, THRESHOLD=1, FULL=2
static std::pair<ark::common_vm::GCReason, ark::common_vm::GCType> GCParamsFromCause(EtsInt cause)
{
    if (cause == 0_I) {
        return {ark::common_vm::GC_REASON_YOUNG, ark::common_vm::GC_TYPE_YOUNG};
    } else if (cause == 1_I) {
        return {ark::common_vm::GC_REASON_USER, ark::common_vm::GC_TYPE_FULL};
    } else if (cause == 2_I) {
        return {ark::common_vm::GC_REASON_OOM, ark::common_vm::GC_TYPE_FULL};
    }
    return {ark::common_vm::GC_REASON_INVALID, ark::common_vm::GC_TYPE_END};
}

/**
 * The function triggers specific GC.
 * @param cause - integer denotes type of GC. Possible values are: YOUNG_CAUSE = 0, THRESHOLD_CAUSE = 1,
 *                OOM_CAUSE = 2
 * @param isRunGcInPlace - not used by CMC GC (common_runtime manages GC scheduling internally)
 * @return gc id as target value of the completed-GC counter. Pass it to waitForFinishGC to block
 *  until at least this many GC cycles have completed (i.e. at least one cycle since this call).
 *  - The function may return -1 if the request is rejected (invalid cause or non-null callback);
 *    an ETS exception is thrown in this case
 */
extern "C" EtsLong StdGCStartGC(EtsInt cause, EtsObject *callback, [[maybe_unused]] EtsBoolean isRunGcInPlace)
{
    if (callback != nullptr) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        ASSERT(executionCtx != nullptr);
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreUnsupportedOperationError,
                          "callback is not supported with current GC");
        return -1;
    }
    auto [reason, gcType] = GCParamsFromCause(cause);
    if (reason == ark::common_vm::GC_REASON_INVALID) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        ASSERT(executionCtx != nullptr);
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalArgumentError, "Invalid GC cause");
        return -1;
    }
    // target guarantees waitForFinishGC waits for at least +1 GC cycle
    uint64_t target = ark::common_vm::BaseRuntime::GetGcCompletedCount() + 1U;
    // false allows to call RequestGCAndWait.
    // Sync or async current gcTask will be detected
    // inside RequestGCAndWait by reason
    ark::common_vm::BaseRuntime::RequestGC(reason, false, gcType, true);
    return static_cast<EtsLong>(target);
}

/**
 * The function returns when the specified GC gets finished.
 * @param gcId - target counter value returned by startGc.
 * If gcId is 0 or -1 the function returns immediately.
 *
 * Uses WaitForGCCompletionCount to block on a condition variable until
 * gcCompletedCount reaches the required value — even if GC has not started yet
 * when the wait begins (request is queued, GC thread has not picked it up).
 * This avoids busy-spinning and premature exit.
 */
extern "C" void StdGCWaitForFinishGC(EtsLong gcId)
{
    if (gcId <= 0) {
        return;  // Nothing to wait
    }
    uint64_t target = static_cast<uint64_t>(gcId);
    common_vm::ScopedEnterSaferegion enterSaferegion(true);
    // Block on condition variable until gcCompletedCount >= target, or return early on
    // GC shutdown (TASK_INDEX_GC_EXIT sentinel) — no more cycles will fire
    ark::common_vm::BaseRuntime::WaitForGCCompletionCount(target);
}

extern "C" EtsLong StdGetFreeHeapSize()
{
    return static_cast<EtsLong>(ark::common_vm::BaseRuntime::GetFreeHeapSize());
}

extern "C" EtsLong StdGetUsedHeapSize()
{
    return static_cast<EtsLong>(ark::common_vm::BaseRuntime::GetUsedHeapSize());
}

extern "C" EtsLong StdGetReservedHeapSize()
{
    return static_cast<EtsLong>(ark::common_vm::BaseRuntime::GetReservedHeapSize());
}

extern "C" void StdGCRegisterNativeAllocation(EtsLong size)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    if (size < 0) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreNegativeArraySizeError,
                          "The value must be non negative");
        return;
    }
    ark::common_vm::BaseRuntime::NotifyNativeAllocation(ClampToSizeT(size));
}

extern "C" void StdGCRegisterNativeFree(EtsLong size)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    if (size < 0) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreNegativeArraySizeError,
                          "The value must be non negative");
        return;
    }
    ark::common_vm::BaseRuntime::NotifyNativeFree(ClampToSizeT(size));
}

}  // namespace ark::ets::intrinsics
