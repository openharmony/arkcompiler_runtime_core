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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_H
#define COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_H

#include "common_interfaces/objects/string/base_string_declare.h"

namespace panda {
/*
 +-----------------------------+ <-- offset 0
 |      BaseObject fields      |
 +-----------------------------+
 | LengthAndFlags (uint32_t)   |
 +-----------------------------+
 | RawHashcode (uint32_t)      |
 +-----------------------------+ <-- offset = BaseString::SIZE
 | Parent (BaseString *)       | <-- PARENT_OFFSET
 +-----------------------------+
 | StartIndexAndFlags (uint32_t) | <-- STARTINDEX_AND_FLAGS_OFFSET
 +-----------------------------+ <-- SIZE
*/
/*
 +-------------------------------+
 | StartIndexAndFlags (uint32_t) |
 +-------------------------------+
 Bit layout:
   [0]         : HasBackingStoreBit         (1 bit)
   [1]         : ReserveBit                 (1 bit)
   [2 - 31]    : StartIndexBits             (30 bits)
 */
// The substrings of another string use SlicedString to describe.
class SlicedString : public BaseString {
public:
    BASE_CAST_CHECK(SlicedString, IsSlicedString);
    NO_MOVE_SEMANTIC_CC(SlicedString);
    NO_COPY_SEMANTIC_CC(SlicedString);
    static constexpr uint32_t MIN_SLICED_STRING_LENGTH = 13;
    static constexpr size_t PARENT_OFFSET = BaseString::SIZE;
    static constexpr uint32_t START_INDEX_BITS_NUM = 30U;
    using HasBackingStoreBit = BitField<bool, 0>;                                 // 1
    using ReserveBit = HasBackingStoreBit::NextFlag;                              // 1
    using StartIndexBits = ReserveBit::NextField<uint32_t, START_INDEX_BITS_NUM>; // 30
    static_assert(StartIndexBits::START_BIT + StartIndexBits::SIZE == sizeof(uint32_t) * BITS_PER_BYTE,
                  "StartIndexBits does not match the field size");
    static_assert(StartIndexBits::SIZE == LengthBits::SIZE, "The size of startIndex should be same with Length");

    POINTER_FIELD(Parent, PARENT_OFFSET, STARTINDEX_AND_FLAGS_OFFSET);
    PRIMITIVE_FIELD(StartIndexAndFlags, uint32_t, STARTINDEX_AND_FLAGS_OFFSET, SIZE);

    uint32_t GetStartIndex() const;

    void SetStartIndex(uint32_t startIndex);

    bool GetHasBackingStore() const;

    void SetHasBackingStore(bool hasBackingStore);

    // Minimum length for a sliced string
    template <bool verify = true, typename ReadBarrier>
    uint16_t Get(ReadBarrier &&readBarrier, int32_t index) const;
};
}
#endif //COMMON_INTERFACES_OBJECTS_STRING_SLICED_STRING_H