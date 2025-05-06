/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <cstddef>
#include "cross_values.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"

namespace ark::ets::intrinsics {
EtsObject *EtsEscompatArrayGet(ObjectHeader *arrayHeader, int32_t index)
{
    ASSERT(arrayHeader != nullptr);
    auto *array = EtsArrayObject<EtsObject>::FromEtsObject(EtsObject::FromCoreType(arrayHeader));
    auto actualLength = array->GetActualLength();
    if (UNLIKELY(static_cast<uint32_t>(index) >= actualLength)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR,
                          "Out of bounds");
        return nullptr;
    }
    return array->GetData()->Get(index);
}

void EtsEscompatArraySet(ObjectHeader *arrayHeader, int32_t index, EtsObject *value)
{
    ASSERT(arrayHeader != nullptr);
    auto *array = EtsArrayObject<EtsObject>::FromEtsObject(EtsObject::FromCoreType(arrayHeader));
    auto actualLength = array->GetActualLength();
    if (UNLIKELY(static_cast<uint32_t>(index) >= actualLength)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR,
                          "Out of bounds");
        return;
    }
    array->GetData()->Set(index, value);
}
}  // namespace ark::ets::intrinsics
