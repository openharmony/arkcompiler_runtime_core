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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_H
#define COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_H

#include "common_interfaces/objects/string/base_string_declare.h"

namespace common {
/*
 +-----------------------------+ <-- offset 0
 |      BaseObject fields      |
 +-----------------------------+
 | LengthAndFlags (uint32_t)   |
 +-----------------------------+
 | RawHashcode (uint32_t)      |
 +-----------------------------+ <-- offset = BaseString::SIZE
 | First (BaseString *)        | <-- FIRST_OFFSET
 +-----------------------------+
 | Second (BaseString *)       | <-- SECOND_OFFSET
 +-----------------------------+ <-- SIZE
*/
class TreeString : public BaseString {
public:
    BASE_CAST_CHECK(TreeString, IsTreeString);

    NO_MOVE_SEMANTIC_CC(TreeString);
    NO_COPY_SEMANTIC_CC(TreeString);
    // Minimum length for a tree string
    static constexpr uint32_t MIN_TREE_STRING_LENGTH = 13;

    static constexpr size_t FIRST_OFFSET = BaseString::SIZE;
    POINTER_FIELD(First, FIRST_OFFSET, SECOND_OFFSET);
    POINTER_FIELD(Second, SECOND_OFFSET, SIZE);

    template <typename ReadBarrier>
    bool IsFlat(ReadBarrier &&readBarrier) const;

    template <bool verify = true, typename ReadBarrier>
    uint16_t Get(ReadBarrier &&readBarrier, int32_t index) const;
};
}
#endif //COMMON_INTERFACES_OBJECTS_STRING_TREE_STRING_H