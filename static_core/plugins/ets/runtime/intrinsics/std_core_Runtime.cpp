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

#include "plugins/ets/runtime/ets_execution_context.h"
#include "include/object_header.h"
#include "intrinsics.h"
#include "intrinsics/helpers/ets_intrinsics_helpers.h"
#include "libarkbase/utils/logger.h"
#include "runtime/handle_scope-inl.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_map.h"
#include "plugins/ets/runtime/ets_stubs.h"

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#endif
namespace ark::ets::intrinsics {

EtsBoolean StdCoreRuntimeIsLittleEndianPlatform()
{
    ASSERT(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}

uint8_t StdCoreRuntimeIsSameReference(EtsObject *source, EtsObject *target)
{
    return (source == target) ? UINT8_C(1) : UINT8_C(0);
}

EtsInt StdCoreRuntimeGetHashCode(EtsObject *source)
{
    ASSERT(source != nullptr);
    return bit_cast<EtsInt>(source->GetHashCode());
}

EtsLong StdRuntimeGetHashCodeByValue(EtsObject *source)
{
    return static_cast<EtsInt>(EtsEscompatMap::GetHashCode(source));
}

EtsBoolean StdRuntimeSameValueZero(EtsObject *a, EtsObject *b)
{
    return ToEtsBoolean(ark::ets::intrinsics::helpers::SameValueZero(EtsExecutionContext::GetCurrent(), a, b));
}

ObjectHeader *StdCoreRuntimeAllocSameTypeArray(EtsClass *cls, int32_t length)
{
    if (UNLIKELY(!cls->IsArrayClass())) {
        // should not appear for the optimized version of intrinsic, which is always inlined
        ThrowEtsException(EtsExecutionContext::GetCurrent(), PlatformTypes()->coreError, "class is not an array");
        return nullptr;
    }
    return coretypes::Array::Create(cls->GetRuntimeClass(), length);
}

EtsString *StdCoreRuntimeAnyToString([[maybe_unused]] EtsObject *anyObj)
{
    ASSERT(anyObj->GetClass()->GetRuntimeClass()->IsXRefClass());
#ifdef PANDA_ETS_INTEROP_JS
    return interop::js::JSValueToString(interop::js::JSValue::FromEtsObject(anyObj));
#endif
    UNREACHABLE();
}

}  // namespace ark::ets::intrinsics
