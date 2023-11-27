/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

namespace panda::ets::intrinsics {

uint8_t StdCoreRuntimeEquals(ObjectHeader *header [[maybe_unused]], EtsObject *source, EtsObject *target)
{
    return (source == target) ? UINT8_C(1) : UINT8_C(0);
}

EtsInt StdCoreRuntimeGetHashCode([[maybe_unused]] ObjectHeader *header, EtsObject *source)
{
    ASSERT(source != nullptr);
    if (source->IsHashed()) {
        return source->GetInteropHash();
    }
    auto hash = ObjectHeader::GenerateHashCode();
    source->SetInteropHash(hash);
    return bit_cast<EtsInt>(hash);
}

}  // namespace panda::ets::intrinsics
