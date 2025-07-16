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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_INL_H
#define COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_INL_H

#include "common_components/base/globals.h"
#include "common_components/common_runtime/hooks.h"
#include "common_interfaces/objects/string/base_string_declare.h"
#include "common_interfaces/objects/string/line_string.h"

namespace common {
inline size_t LineString::ComputeSizeUtf8(uint32_t utf8Len)
{
    return DATA_OFFSET + utf8Len;
}

inline size_t LineString::ComputeSizeUtf16(uint32_t utf16Len)
{
    return DATA_OFFSET + utf16Len * sizeof(uint16_t);
}

inline size_t LineString::ObjectSize(BaseString *str)
{
    uint32_t length = str->GetLength();
    return str->IsUtf16() ? ComputeSizeUtf16(length) : ComputeSizeUtf8(length);
}

inline size_t LineString::DataSize(BaseString *str)
{
    uint32_t length = str->GetLength();
    return str->IsUtf16() ? length * sizeof(uint16_t) : length;
}

template <bool verify>
uint16_t LineString::Get(int32_t index) const
{
    int32_t length = static_cast<int32_t>(GetLength());
    if (verify) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }
    if (!IsUtf16()) {
        Span<const uint8_t> sp(GetDataUtf8(), length);
        return sp[index];
    }
    Span<const uint16_t> sp(GetDataUtf16(), length);
    return sp[index];
}

inline void LineString::Set(uint32_t index, uint16_t src)
{
    DCHECK_CC(index < GetLength());
    if (IsUtf8()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *(reinterpret_cast<uint8_t *>(GetData()) + index) = static_cast<uint8_t>(src);
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *(GetData() + index) = src;
    }
}

inline void LineString::Trim(uint32_t newLength)
{
    ASSERT(IsLineString());
    uint32_t oldLength = GetLength();
    ASSERT(oldLength >= newLength);
    if (newLength == oldLength) return;
    bool isUtf8 = IsUtf8();
    size_t sizeNew = isUtf8 ?
        LineString::ComputeSizeUtf8(newLength): LineString::ComputeSizeUtf16(newLength);
    size_t sizeOld = isUtf8 ?
        LineString::ComputeSizeUtf8(oldLength): LineString::ComputeSizeUtf16(oldLength);
    sizeNew = common::AlignUp(sizeNew, ALIGNMENT_8_BYTES);
    sizeOld = common::AlignUp(sizeOld, ALIGNMENT_8_BYTES);
    size_t trimBytes = sizeOld - sizeNew;
    if (trimBytes > 0) {
        uintptr_t newEndAddr = ToUintPtr(this) + sizeNew;
        ASSERT_PRINT((newEndAddr % ALIGNMENT_8_BYTES) == 0, "Alignment failed");
        FillFreeObject(reinterpret_cast<void*>(newEndAddr), trimBytes);
    }
    InitLengthAndFlags(newLength, isUtf8);
}
}
#endif //COMMON_INTERFACES_OBJECTS_STRING_LINE_STRING_INL_H