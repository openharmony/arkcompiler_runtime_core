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

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/intrinsics/gc_cause_helper.h"
#include "plugins/ets/runtime/intrinsics/gc_task_tracker.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/include/thread_scopes.h"

#include "libarkbase/utils/utils.h"
#include "libarkbase/utils/logger.h"

namespace ark::ets::intrinsics {

/**
 * The function triggers specific GC.
 * @param cause - integer denotes type of GC. Possible values are: YOUNG_CAUSE = 0, THRESHOLD_CAUSE = 1,
 *                FULL_CAUSE = 2
 * @param isRunGcInPlace - option to run GC in place
 * @return gc id. The id should be passed to waitForFinishGC to ensure the GC is finished.
 *  - The function may return 0 in case the GC is executed in-place. It means there is no need to wait such GC.
 *  - The function may return -1 in case the task is canceled.
 */
extern "C" EtsLong StdGCStartGC(EtsInt cause, EtsObject *callback, EtsBoolean isRunGcInPlace)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    bool runGcInPlace = (isRunGcInPlace == 1) ? true : Runtime::GetOptions().IsRunGcInPlace("ets");

    GCTaskCause reason = GCCauseFromInt(cause);
    auto *gc = executionCtx->GetPandaVM()->GetGC();
    if (!gc->CheckGCCause(reason)) {
        PandaStringStream eMsg;
        eMsg << mem::GCStringFromType(gc->GetType()) << " does not support " << reason << " cause";
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalArgumentError, eMsg.str());
        return -1;
    }
    auto &gcTaskTracker = GCTaskTracker::InitIfNeededAndGet(gc);
    auto task = MakePandaUnique<GCTask>(reason);
    uint32_t id = task->GetId();
    if (callback != nullptr) {
        auto *callbackRef = executionCtx->GetPandaVM()->GetGlobalObjectStorage()->Add(
            callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        gcTaskTracker.SetCallbackForTask(id, callbackRef);
        // Run GC in place, because need to run callback in managed part
        runGcInPlace = true;
    }

    // Young GC runs in place
    if (reason == GCTaskCause::YOUNG_GC_CAUSE) {
        runGcInPlace = true;
    }
    if (runGcInPlace) {
        return gc->WaitForGCInManaged(*task) ? 0 : -1;
    }
    // Run GC in GC-thread
    if ((reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE) && gc->IsPostponeEnabled()) {
        LOG(ERROR, GC) << "Threshold GC request rejected during postpone period";
        return -1;
    }
    gcTaskTracker.AddTaskId(id);
    if (!gc->Trigger(std::move(task))) {
        gcTaskTracker.RemoveId(id);
        return -1;
    }
    return static_cast<EtsLong>(id);
}

/**
 * The function returns when the specified GC gets finished.
 * @param gc_id - id of the GC which is returned by startGc.
 * If gc_id is 0 or -1 the function returns immediately.
 */
extern "C" void StdGCWaitForFinishGC(EtsLong gcId)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    if (gcId <= 0) {
        return;
    }
    auto id = static_cast<uint64_t>(gcId);
    ASSERT(GCTaskTracker::IsInitialized());
    ScopedNativeCodeThread s(executionCtx->GetMT());
    while (GCTaskTracker::InitIfNeededAndGet(executionCtx->GetPandaVM()->GetGC()).HasId(id)) {
        constexpr uint64_t WAIT_TIME_MS = 2U;
        os::thread::NativeSleep(WAIT_TIME_MS);
    }
}

}  // namespace ark::ets::intrinsics