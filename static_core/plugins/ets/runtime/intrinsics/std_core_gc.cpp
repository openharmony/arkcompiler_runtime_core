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

// Shared GC intrinsics (compiled for both Default GC and CMC GC)
// For Default (G1) GC only implementations see std_core_default_gc.cpp
// For CMC GC (common_runtime) implementations see std_core_common_gc.cpp

#include "libarkbase/utils/utils.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/intrinsics/gc_cause_helper.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics {

extern "C" EtsBoolean StdGCIsScheduledGCTriggered()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *vm = executionCtx->GetPandaVM();
    auto *trigger = vm->GetGCTrigger();

    if (trigger->GetType() != mem::GCTriggerType::ON_NTH_ALLOC) {
        return ToEtsBoolean(false);
    }
    auto schedTrigger = reinterpret_cast<mem::SchedGCOnNthAllocTrigger *>(vm->GetGCTrigger());
    return ToEtsBoolean(schedTrigger->IsTriggered());
}

extern "C" void StdGCPostponeGCStart()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *gc = executionCtx->GetPandaVM()->GetGC();
    if (!gc->IsPostponeGCSupported()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreUnsupportedOperationError,
                          "GC postpone is not supported for this GC type");
        return;
    }
    if (gc->IsPostponeEnabled()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalStateError,
                          "Calling postponeGCStart without calling postponeGCEnd");
        return;
    }
    gc->PostponeGCStart();
}

extern "C" void StdGCPostponeGCEnd()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *gc = executionCtx->GetPandaVM()->GetGC();
    if (!gc->IsPostponeGCSupported()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreUnsupportedOperationError,
                          "GC postpone is not supported for this GC type");
        return;
    }
    if (!gc->IsPostponeEnabled()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalStateError,
                          "Calling postponeGCEnd without calling postponeGCStart");
        return;
    }
    gc->PostponeGCEnd();
}

template <class ResArrayType>
[[nodiscard]] static ResArrayType *StdGCAllocatePinnedPrimitiveTypeArray(EtsLong length)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);

    if (length < 0) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreNegativeArraySizeError,
                          "The value must be non negative");
        return nullptr;
    }
    auto *vm = executionCtx->GetPandaVM();
    if (!vm->GetGC()->IsPinningSupported()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreUnsupportedOperationError,
                          "Object pinning does not support with current GC");
        return nullptr;
    }
    auto *array = ResArrayType::Create(length, SpaceType::SPACE_TYPE_OBJECT, true);

    if (array == nullptr) {
        PandaStringStream ss;
        ss << "Could not allocate array of " << length << " elements";
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreOutOfMemoryError, ss.str());
        return nullptr;
    }

    return array;
}

extern "C" EtsBooleanArray *StdGCAllocatePinnedBooleanArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsBooleanArray>(length);
}

extern "C" EtsByteArray *StdGCAllocatePinnedByteArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsByteArray>(length);
}

extern "C" EtsCharArray *StdGCAllocatePinnedCharArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsCharArray>(length);
}

extern "C" EtsShortArray *StdGCAllocatePinnedShortArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsShortArray>(length);
}

extern "C" EtsIntArray *StdGCAllocatePinnedIntArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsIntArray>(length);
}

extern "C" EtsLongArray *StdGCAllocatePinnedLongArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsLongArray>(length);
}

extern "C" EtsFloatArray *StdGCAllocatePinnedFloatArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsFloatArray>(length);
}

extern "C" EtsDoubleArray *StdGCAllocatePinnedDoubleArray(EtsLong length)
{
    return StdGCAllocatePinnedPrimitiveTypeArray<EtsDoubleArray>(length);
}

extern "C" EtsInt StdGCGetObjectSpaceType(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    auto *vm = ManagedThread::GetCurrent()->GetVM();
    SpaceType objSpaceType =
        PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(static_cast<void *>(obj->GetCoreType()));
    ASSERT_PRINT(IsHeapSpace(objSpaceType), SpaceTypeToString(objSpaceType));
    if (objSpaceType == SpaceType::SPACE_TYPE_OBJECT && vm->GetGC()->IsGenerational()) {
        if (vm->GetHeapManager()->IsObjectInYoungSpace(obj->GetCoreType())) {
            const EtsInt youngSpace = 4;
            return youngSpace;
        }
        const EtsInt tenuredSpace = 5;
        return tenuredSpace;
    }
    return SpaceTypeToIndex(objSpaceType);
}

extern "C" void StdGCPinObject(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);

    auto *vm = executionCtx->GetPandaVM();
    auto *gc = vm->GetGC();
    if (!gc->IsPinningSupported()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreUnsupportedOperationError,
                          "Object pinning does not support with current gc");
        return;
    }
    vm->GetHeapManager()->PinObject(obj->GetCoreType());
}

extern "C" void StdGCUnpinObject(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    auto *vm = ManagedThread::GetCurrent()->GetVM();
    vm->GetHeapManager()->UnpinObject(obj->GetCoreType());
}

extern "C" EtsLong StdGCGetObjectAddress(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    return reinterpret_cast<EtsLong>(obj);
}

extern "C" EtsLong StdGetObjectSize(EtsObject *obj)
{
    ASSERT(obj != nullptr);
    return static_cast<EtsLong>(obj->GetCoreType()->ObjectSize());
}

// Function schedules GC before n-th allocation by setting counter to the specific GC trigger.
// Another call may reset the counter.  In this case the last counter will be used to trigger the GC.
extern "C" void StdGCScheduleGCAfterNthAlloc(EtsInt counter, EtsInt cause)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);

    if (counter < 0) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalArgumentError,
                          "counter for allocation is negative");
        return;
    }
    GCTaskCause reason = GCCauseFromInt(cause);

    auto *vm = executionCtx->GetPandaVM();
    auto *gc = vm->GetGC();
    if (!gc->CheckGCCause(reason)) {
        PandaStringStream eMsg;
        eMsg << mem::GCStringFromType(gc->GetType()) << " does not support " << reason << " cause";
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalArgumentError, eMsg.str());
        return;
    }
    mem::GCTrigger *trigger = vm->GetGCTrigger();
    if (trigger->GetType() != mem::GCTriggerType::ON_NTH_ALLOC) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreUnsupportedOperationError,
                          "VM is running with unsupported GC trigger");
        return;
    }
    auto schedTrigger = reinterpret_cast<mem::SchedGCOnNthAllocTrigger *>(trigger);
    schedTrigger->ScheduleGc(reason, counter);
}

}  // namespace ark::ets::intrinsics
