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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_H
#define COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_H

#include "common_interfaces/objects/string/base_string_declare.h"

namespace common {
/*
 +-----------------------------+ <-- offset 0
 |      BaseObject fields      |
 +-----------------------------+ <-- offset = BaseObjectSize()
 | LengthAndFlags (uint32_t)   |
 +-----------------------------+
 | RawHashcode (uint32_t)      |
 +-----------------------------+ <-- offset = BaseString::SIZE
 |     String Data (UTF8/16)   | <-- DATA_OFFSET = BaseString::SIZE
 |     ...                     |
 +-----------------------------+
 */
// The LineString abstract class captures sequential string values, only LineString can store chars data
class LineString : public BaseString {
public:
    BASE_CAST_CHECK(LineString, IsLineString);
    NO_MOVE_SEMANTIC_CC(LineString);
    NO_COPY_SEMANTIC_CC(LineString);

    static constexpr size_t ALIGNMENT_8_BYTES = 8;
    static constexpr uint32_t MAX_LENGTH = (1 << 28) - 16;
    static constexpr uint32_t INIT_LENGTH_TIMES = 4;
    // DATA_OFFSET: the string data stored after the string header.
    // Data can be stored in utf8 or utf16 form according to compressed bit.
    static constexpr size_t DATA_OFFSET = BaseString::SIZE; // DATA_OFFSET equal to Empty String size

    uint16_t *GetData() const;

    template <bool verify = true>
    uint16_t Get(int32_t index) const;

    void Set(uint32_t index, uint16_t src);

    void Trim(uint32_t newLength);

    static size_t ComputeSizeUtf8(uint32_t utf8Len);

    static size_t ComputeSizeUtf16(uint32_t utf16Len);

    static size_t ObjectSize(BaseString *str);

    static size_t DataSize(BaseString *str);
};
}
#endif //COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_H