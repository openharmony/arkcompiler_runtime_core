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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_INL1_H
#define COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_INL1_H

#include <cstring>

#include "common_interfaces/objects/string/base_string_declare.h"

#include "common_interfaces/objects/string/line_string-inl.h"
#include "common_interfaces/objects/string/sliced_string-inl.h"
#include "common_interfaces/objects/string/tree_string-inl.h"
#include "common_interfaces/objects/utils/utf_utils.h"

namespace common {
inline bool BaseString::IsUtf8() const
{
    uint32_t bits = GetLengthAndFlags();
    return CompressedStatusBit::Decode(bits) == STRING_COMPRESSED;
}

inline bool BaseString::IsUtf16() const
{
    uint32_t bits = GetLengthAndFlags();
    return CompressedStatusBit::Decode(bits) == STRING_UNCOMPRESSED;
}

inline uint32_t BaseString::GetLength() const
{
    uint32_t bits = GetLengthAndFlags();
    return LengthBits::Decode(bits);
}

inline void BaseString::InitLengthAndFlags(uint32_t length, bool compressed, bool isIntern)
{
    DCHECK_CC(length < BaseString::MAX_STRING_LENGTH);
    uint32_t newVal = 0;
    newVal = IsInternBit::Update(newVal, isIntern);
    newVal = CompressedStatusBit::Update(newVal, (compressed ? STRING_COMPRESSED : STRING_UNCOMPRESSED));
    newVal = LengthBits::Update(newVal, length);
    SetLengthAndFlags(newVal);
}

inline void BaseString::SetIsInternString()
{
    uint32_t bits = GetLengthAndFlags();
    uint32_t newVal = IsInternBit::Update(bits, true);
    SetLengthAndFlags(newVal);
}

inline bool BaseString::IsInternString() const
{
    uint32_t bits = GetLengthAndFlags();
    return IsInternBit::Decode(bits);
}

inline void BaseString::ClearInternStringFlag()
{
    uint32_t bits = GetLengthAndFlags();
    uint32_t newVal = IsInternBit::Update(bits, false);
    SetLengthAndFlags(newVal);
}

inline bool BaseString::TryGetHashCode(uint32_t *hash) const
{
    uint32_t hashcode = GetRawHashcode();
    if (hashcode == 0 && GetLength() != 0) {
        return false;
    }
    *hash = hashcode;
    return true;
}

// not change this data structure.
// if string is not flat, this func has low efficiency.
template <typename ReadBarrier>
uint32_t PUBLIC_API BaseString::GetHashcode(ReadBarrier &&readBarrier)
{
    uint32_t hashcode = GetRawHashcode();
    // GetLength() == 0 means it's an empty array.No need to computeHashCode again when hashseed is 0.
    if (hashcode == 0 && GetLength() != 0) {
        hashcode = ComputeRawHashcode(std::forward<ReadBarrier>(readBarrier));
        SetRawHashcode(hashcode);
    }
    return hashcode;
}

// Check that two spans are equal. Should have the same length.
/* static */
template <typename T, typename T1>
bool BaseString::StringsAreEquals(Span<const T> &str1, Span<const T1> &str2)
{
    DCHECK_CC(str1.Size() <= str2.Size());
    size_t size = str1.Size();
    if constexpr (std::is_same_v<T, T1>) {
        return !memcmp(str1.data(), str2.data(), size * sizeof(T));
    } else {
        for (size_t i = 0; i < size; i++) {
            auto left = static_cast<uint16_t>(str1[i]);
            auto right = static_cast<uint16_t>(str2[i]);
            if (left != right) {
                return false;
            }
        }
        return true;
    }
}
} // namespace common

#endif //COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_INL1_H