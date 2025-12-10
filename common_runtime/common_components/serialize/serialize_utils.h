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

#ifndef COMMON_COMPONENTS_SERIALIZE_UTILS_H
#define COMMON_COMPONENTS_SERIALIZE_UTILS_H

#include <cstddef>
#include <cstdint>

namespace common {

enum class SerializedBaseObjectSpace : uint8_t {
    REGULAR_SPACE,
    PIN_SPACE,
    LARGE_SPACE,
    READ_ONLY_SPACE,
    OTHER,
};

class SerializeUtils {
public:
    static SerializedBaseObjectSpace GetSerializeObjectSpace(uintptr_t obj);
};
}  // namespace common
#endif  // COMMON_COMPONENTS_SERIALIZE_UTILS_H
