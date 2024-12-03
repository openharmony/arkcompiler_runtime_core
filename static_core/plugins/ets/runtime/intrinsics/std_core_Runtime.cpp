/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "include/object_header.h"
#include "intrinsics.h"
#include "libpandabase/utils/logger.h"
#include "runtime/handle_scope-inl.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_stubs.h"

namespace ark::ets::intrinsics {

EtsBoolean StdCoreRuntimeGetPlatformIsLittleEndian()
{
    ASSERT(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}

uint8_t StdCoreRuntimeIsSameReference([[maybe_unused]] ObjectHeader *header, EtsObject *source, EtsObject *target)
{
    return (source == target) ? UINT8_C(1) : UINT8_C(0);
}

EtsInt StdCoreRuntimeGetHashCode([[maybe_unused]] ObjectHeader *header, EtsObject *source)
{
    ASSERT(source != nullptr);
    return bit_cast<EtsInt>(source->GetHashCode());
}

static char const *ReferenceTypeString(EtsCoroutine *coro, EtsObject *obj)
{
    if (obj == nullptr) {
        return "null";
    }
    if (obj == EtsObject::FromCoreType(coro->GetUndefinedObject())) {
        return "undefined";
    }
    return obj->GetClass()->GetDescriptor();
}

ObjectHeader *StdCoreRuntimeFailedTypeCastException(EtsObject *source, EtsString *target)
{
    auto coro = EtsCoroutine::GetCurrent();

    auto message = PandaString(ReferenceTypeString(coro, source)) + " cannot be cast to " + target->GetMutf8();

    return ets::SetupEtsException(coro, panda_file_items::class_descriptors::CLASS_CAST_ERROR.data(), message.data())
        ->GetCoreType();
}

EtsString *StdCoreRuntimeTypeof(EtsObject *obj)
{
    return EtsGetTypeof(EtsCoroutine::GetCurrent(), obj);
}

}  // namespace ark::ets::intrinsics
