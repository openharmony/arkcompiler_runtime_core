/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_H_
#define PANDA_PLUGINS_ETS_TYPEAPI_H_

#include "types/ets_primitives.h"

namespace panda::ets {

enum class EtsTypeAPIKind : EtsByte {
    NONE = 0x0U,
    VOID = 0x1U,

    CHAR = 0x2U,
    BOOLEAN = 0x3U,
    BYTE = 0x4U,
    SHORT = 0x5U,
    INT = 0x6U,
    LONG = 0x7U,
    FLOAT = 0x8U,
    DOUBLE = 0x9U,

    CLASS = 0xAU,
    STRING = 0xBU,
    INTERFACE = 0xCU,
    ARRAY = 0xDU,
    FUNCTION = 0xEU,
    UNION = 0xFU,
    UNDEFINED = 0x10U,
    NUL = 0x11U,

    ENUM = 0x12U

};

constexpr EtsByte ETS_TYPE_KIND_VALUE_MASK = 1U << 6U;

}  // namespace panda::ets

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_H_