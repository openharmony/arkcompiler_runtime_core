/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "types/ets_array.h"

namespace ark::ets::intrinsics {

extern "C" EtsObject *StdCoreReflectInternalsCreateFixedArray(EtsClass *component, EtsInt length)
{
    if (UNLIKELY(component == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    if (UNLIKELY(length < 0)) {
        ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreIllegalArgumentError,
                               "Array length can't be negative");
        return nullptr;
    }

    auto *result = EtsObjectArray::Create(component, length);
    if (UNLIKELY(result == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    return result->AsObject();
}

}  // namespace ark::ets::intrinsics
