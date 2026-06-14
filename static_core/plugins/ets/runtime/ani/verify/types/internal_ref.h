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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_INTERNAL_REF_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_INTERNAL_REF_H

#include "plugins/ets/runtime/ani/verify/types/vref.h"

namespace ark::ets::ani::verify {

class VWRef;

inline constexpr uintptr_t VERIFY_HANDLE_TAG_MASK = 3U;
inline constexpr uintptr_t VERIFY_HANDLE_TAG_VREF = 2U;
inline constexpr uintptr_t VERIFY_HANDLE_TAG_VWREF = 3U;

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class InternalRef final {
public:
    explicit InternalRef(ani_ref aniRef);
    explicit InternalRef(ani_wref aniWref);

    static VRef *CastToVRef(InternalRef *iref);
    static InternalRef *CastFromVRef(VRef *vref);

    static VWRef *CastToVWRef(InternalRef *iref);
    static InternalRef *CastFromVWRef(VWRef *vwref);

    static bool IsStackVRef(VRef *vref);
    static bool IsUndefinedStackRef(VRef *vref);
    static bool IsVWRef(VWRef *vwref);
    static bool IsVRef(VRef *vref);

    ani_ref GetRef();
    ani_wref GetWRef();

    NO_COPY_SEMANTIC(InternalRef);
    NO_MOVE_SEMANTIC(InternalRef);

    ~InternalRef() = default;

private:
    union {
        ani_ref ref;
        ani_wref wref;
    };
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_TYPES_INTERNAL_REF_H
