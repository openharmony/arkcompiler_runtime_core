/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_OBJECTS_BASE_TYPE_H
#define COMMON_INTERFACES_OBJECTS_BASE_TYPE_H

#include <vector>
#include <variant>
#include <cstdint>
#include <memory>

#include "objects/base_object.h"
#include "objects/string/base_string_declare.h"

namespace panda::ecmascript {
class JSTaggedValue;
}

using JSTaggedValue = panda::ecmascript::JSTaggedValue;
namespace panda {

struct BaseUndefined {};
struct BaseNull {};
struct BaseBigInt {
    uint32_t length;
    bool sign;
    std::vector<uint32_t> data;
};

// The common consensus type between static and dynamic
using PandaType = std::variant<std::monostate, bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float,
                               double, int64_t, uint64_t, BaseUndefined, BaseNull, BaseBigInt, BaseObject*,
                               BaseString*>;

// base type for static vm
using BoxedValue = BaseObject *;

}  // namespace panda
#endif  // COMMON_INTERFACES_OBJECTS_BASE_TYPE_H