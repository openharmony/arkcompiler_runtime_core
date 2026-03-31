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

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ani/verify/types/internal_ref.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vwref.h"

namespace ark::ets::ani::verify {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
InternalRef::InternalRef(ani_ref aniRef) : ref {aniRef} {}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
InternalRef::InternalRef(ani_wref aniWref) : wref {aniWref} {}

ani_ref InternalRef::GetRef()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return ref;
}

ani_wref InternalRef::GetWRef()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return wref;
}

VRef *InternalRef::CastToVRef(InternalRef *iref)
{
    ASSERT((ToUintPtr(iref) & VERIFY_HANDLE_TAG_MASK) == 0U);
    return reinterpret_cast<VRef *>((ToUintPtr(iref) | VERIFY_HANDLE_TAG_VREF));
}

InternalRef *InternalRef::CastFromVRef(VRef *vref)
{
    ASSERT((ToUintPtr(vref) & VERIFY_HANDLE_TAG_MASK) == VERIFY_HANDLE_TAG_VREF);
    return reinterpret_cast<InternalRef *>((ToUintPtr(vref) & ~VERIFY_HANDLE_TAG_MASK));
}

bool InternalRef::IsUndefinedStackRef(VRef *vref)
{
    return reinterpret_cast<EtsReference *>(vref) == EtsReference::GetUndefined();
}

bool InternalRef::IsStackVRef(VRef *vref)
{
    if (IsUndefinedStackRef(vref)) {
        return true;
    }
    return (ToUintPtr(vref) & VERIFY_HANDLE_TAG_MASK) != VERIFY_HANDLE_TAG_VREF;
}

VWRef *InternalRef::CastToVWRef(InternalRef *iref)
{
    ASSERT((ToUintPtr(iref) & VERIFY_HANDLE_TAG_MASK) == 0U);
    return reinterpret_cast<VWRef *>((ToUintPtr(iref) | VERIFY_HANDLE_TAG_VWREF));
}

InternalRef *InternalRef::CastFromVWRef(VWRef *vwref)
{
    ASSERT((ToUintPtr(vwref) & VERIFY_HANDLE_TAG_MASK) == VERIFY_HANDLE_TAG_VWREF);
    return reinterpret_cast<InternalRef *>((ToUintPtr(vwref) & ~VERIFY_HANDLE_TAG_MASK));
}

bool InternalRef::IsVWRef(VWRef *vwref)
{
    if (vwref == nullptr) {
        return false;
    }
    return (ToUintPtr(vwref) & VERIFY_HANDLE_TAG_MASK) == VERIFY_HANDLE_TAG_VWREF;
}

bool InternalRef::IsVRef(VRef *vref)
{
    if (vref == nullptr) {
        return false;
    }
    return (ToUintPtr(vref) & VERIFY_HANDLE_TAG_MASK) == VERIFY_HANDLE_TAG_VREF;
}

}  // namespace ark::ets::ani::verify
