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

// Default (G1) GC implementations of ETS GC intrinsics
// For shared intrinsics see std_core_gc.cpp
// For CMC GC (common_runtime) implementations see std_core_common_gc.cpp
// StdGCStartGC and StdGCWaitForFinishGC in std_core_gc_lifecycle.cpp

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/thread_scopes.h"

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

extern "C" EtsLong StdGetFreeHeapSize()
{
    const auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    return static_cast<EtsLong>(executionCtx->GetPandaVM()->GetHeapManager()->GetFreeMemory());
}

extern "C" EtsLong StdGetUsedHeapSize()
{
    const auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    return static_cast<EtsLong>(executionCtx->GetPandaVM()->GetMemStats()->GetFootprintHeap());
}

extern "C" EtsLong StdGetReservedHeapSize()
{
    const auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    return static_cast<EtsLong>(executionCtx->GetPandaVM()->GetHeapManager()->GetMaxMemory());
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
    ScopedNativeCodeThread s(ManagedThread::GetCurrent());
    executionCtx->GetPandaVM()->GetGC()->RegisterNativeAllocation(ClampToSizeT(size));
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
    ScopedNativeCodeThread s(ManagedThread::GetCurrent());
    executionCtx->GetPandaVM()->GetGC()->RegisterNativeFree(ClampToSizeT(size));
}

}  // namespace ark::ets::intrinsics
