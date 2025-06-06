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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_IMPL_H
#define COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_IMPL_H

#include "common_interfaces/objects/string/base_string_declare.h"
#include "common_interfaces/objects/string/line_string.h"
#include "common_interfaces/objects/string/sliced_string.h"
#include "common_interfaces/objects/string/tree_string.h"

namespace common {
std::u16string Utf16ToU16String(const uint16_t *utf16Data, uint32_t dataLen);
std::u16string Utf8ToU16String(const uint8_t *utf8Data, uint32_t dataLen);

template <typename T1, typename T2>
int32_t CompareStringSpan(Span<T1> &lhsSp, Span<T2> &rhsSp, int32_t count);
template <typename T1, typename T2>
bool IsSubStringAtSpan(Span<T1> &lhsSp, Span<T2> &rhsSp, uint32_t offset);

template <typename ReadBarrier>
uint32_t BaseString::ComputeRawHashcode(ReadBarrier &&readBarrier) const
{
    uint32_t length = GetLength();
    if (length == 0) {
        return 0;
    }

    if (IsUtf8()) {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        // String can not convert to integer number, using normal hashcode computing algorithm.
        return ComputeHashForData(data, length, 0);
    } else {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        // If rawSeed has certain value, and second string uses UTF16 encoding,
        // then merged string can not be small integer number.
        return ComputeHashForData(data, length, 0);
    }
}


template <typename ReadBarrier>
bool BaseString::EqualToSplicedString(ReadBarrier &&readBarrier, const BaseString *str1, const BaseString *str2)
{
    DCHECK_CC(NotTreeString());
    DCHECK_CC(str1->NotTreeString() && str2->NotTreeString());
    if (GetLength() != str1->GetLength() + str2->GetLength()) {
        return false;
    }
    if (IsUtf16()) {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        if (BaseString::StringsAreEqualUtf16(std::forward<ReadBarrier>(readBarrier), str1, data, str1->GetLength())) {
            return BaseString::StringsAreEqualUtf16(std::forward<ReadBarrier>(readBarrier), str2,
                                                    data + str1->GetLength(), str2->GetLength());
        }
    } else {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        if (BaseString::StringIsEqualUint8Data(std::forward<ReadBarrier>(readBarrier), str1, data, str1->GetLength(),
                                               this->IsUtf8())) {
            return BaseString::StringIsEqualUint8Data(std::forward<ReadBarrier>(readBarrier), str2,
                                                      data + str1->GetLength(),
                                                      str2->GetLength(), this->IsUtf8());
        }
    }
    return false;
}

template <typename ReadBarrier>
std::u16string BaseString::ToU16String(ReadBarrier &&readBarrier, uint32_t len)
{
    uint32_t length = len > 0 ? len : GetLength();
    std::u16string result;
    if (IsUtf16()) {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        result = Utf16ToU16String(data, length);
    } else {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        result = Utf8ToU16String(data, length);
    }
    return result;
}


template <typename ReadBarrier>
void BaseString::WriteData(ReadBarrier &&readBarrier, BaseString *src, uint32_t start, uint32_t destSize,
                           uint32_t length)
{
    DCHECK_CC(IsLineString());
    if (IsUtf8()) {
        DCHECK_CC(src->IsUtf8());
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), src, buf);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (length != 0 && memcpy_s(GetDataUtf8Writable() + start, destSize, data, length) != EOK) {
            UNREACHABLE();
        }
    } else if (src->IsUtf8()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), src, buf);
        Span<uint16_t> to(GetDataUtf16Writable() + start, length);
        Span<const uint8_t> from(data, length);
        for (uint32_t i = 0; i < length; i++) {
            to[i] = from[i];
        }
    } else {
        std::vector<uint16_t> buf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), src, buf);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (length != 0 && memcpy_s(GetDataUtf16Writable() + start,
                                    destSize * sizeof(uint16_t), data, length * sizeof(uint16_t)) != EOK) {
            UNREACHABLE();
        }
    }
}

inline bool BaseString::NotTreeString() const
{
    return !IsTreeString();
}

template <typename ReadBarrier>
const uint8_t *BaseString::GetNonTreeUtf8Data(ReadBarrier &&readBarrier, const BaseString *src)
{
    DCHECK_CC(src->IsUtf8());
    DCHECK_CC(!src->IsTreeString());
    BaseString *string = const_cast<BaseString *>(src);
    if (string->IsSlicedString()) {
        SlicedString *str = SlicedString::Cast(string);
        return BaseString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf8() +
               str->GetStartIndex();
    }
    DCHECK_CC(src->IsLineString());
    return string->GetDataUtf8();
}


template <typename ReadBarrier>
const uint16_t *BaseString::GetNonTreeUtf16Data(ReadBarrier &&readBarrier, const BaseString *src)
{
    DCHECK_CC(src->IsUtf16());
    DCHECK_CC(!src->IsTreeString());
    BaseString *string = const_cast<BaseString *>(src);
    if (string->IsSlicedString()) {
        SlicedString *str = SlicedString::Cast(string);
        return BaseString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf16() +
               str->GetStartIndex();
    }
    DCHECK_CC(src->IsLineString());
    return string->GetDataUtf16();
}


/* static */
template <typename ReadBarrier>
bool BaseString::StringsAreEqualDiffUtfEncoding(ReadBarrier &&readBarrier, BaseString *left, BaseString *right)
{
    std::vector<uint16_t> bufLeftUft16;
    std::vector<uint16_t> bufRightUft16;
    std::vector<uint8_t> bufLeftUft8;
    std::vector<uint8_t> bufRightUft8;
    int32_t lhsCount = static_cast<int32_t>(left->GetLength());
    int32_t rhsCount = static_cast<int32_t>(right->GetLength());
    if (!left->IsUtf16() && !right->IsUtf16()) {
        const uint8_t *data1 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft8);
        const uint8_t *data2 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), right, bufRightUft8);
        Span<const uint8_t> lhsSp(data1, lhsCount);
        Span<const uint8_t> rhsSp(data2, rhsCount);
        return BaseString::StringsAreEquals(lhsSp, rhsSp);
    } else if (!left->IsUtf16()) {
        const uint8_t *data1 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft8);
        const uint16_t *data2 = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), right,
                                                             bufRightUft16);
        Span<const uint8_t> lhsSp(data1, lhsCount);
        Span<const uint16_t> rhsSp(data2, rhsCount);
        return BaseString::StringsAreEquals(lhsSp, rhsSp);
    } else if (!right->IsUtf16()) {
        const uint16_t *data1 =
            BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft16);
        const uint8_t *data2 = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), right, bufRightUft8);
        Span<const uint16_t> lhsSp(data1, lhsCount);
        Span<const uint8_t> rhsSp(data2, rhsCount);
        return BaseString::StringsAreEquals(lhsSp, rhsSp);
    } else {
        const uint16_t *data1 =
            BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), left, bufLeftUft16);
        const uint16_t *data2 = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), right,
                                                             bufRightUft16);
        Span<const uint16_t> lhsSp(data1, lhsCount);
        Span<const uint16_t> rhsSp(data2, rhsCount);
        return BaseString::StringsAreEquals(lhsSp, rhsSp);
    }
}

/* static */
template <typename ReadBarrier>
bool BaseString::StringsAreEqual(ReadBarrier &&readBarrier, BaseString *str1, BaseString *str2)
{
    DCHECK_CC(str1 != nullptr);
    DCHECK_CC(str2 != nullptr);
    if (str1 == str2) {
        return true;
    }
    uint32_t str1Len = str1->GetLength();
    if (str1Len != str2->GetLength()) {
        return false;
    }
    if (str1Len == 0) {
        return true;
    }

    uint32_t str1Hash;
    uint32_t str2Hash;
    if (str1->TryGetHashCode(&str1Hash) && str2->TryGetHashCode(&str2Hash)) {
        if (str1Hash != str2Hash) {
            return false;
        }
    }
    return StringsAreEqualDiffUtfEncoding(std::forward<ReadBarrier>(readBarrier), str1, str2);
}


/* static */
template <typename ReadBarrier>
bool BaseString::StringIsEqualUint8Data(ReadBarrier &&readBarrier, const BaseString *str1, const uint8_t *dataAddr,
                                        uint32_t dataLen,
                                        bool canBeCompressToUtf8)
{
    if (!str1->IsSlicedString() && canBeCompressToUtf8 != str1->IsUtf8()) {
        return false;
    }
    if (canBeCompressToUtf8 && str1->GetLength() != dataLen) {
        return false;
    }
    if (str1->IsUtf8()) {
        std::vector<uint8_t> buf;
        Span<const uint8_t> data1(BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf),
                                  dataLen);
        Span<const uint8_t> data2(dataAddr, dataLen);
        return BaseString::StringsAreEquals(data1, data2);
    }
    std::vector<uint16_t> buf;
    uint32_t length = str1->GetLength();
    const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf);
    return IsUtf8EqualsUtf16(dataAddr, dataLen, data, length);
}

/* static */
template <typename ReadBarrier>
bool BaseString::StringsAreEqualUtf16(ReadBarrier &&readBarrier, const BaseString *str1, const uint16_t *utf16Data,
                                      uint32_t utf16Len)
{
    uint32_t length = str1->GetLength();
    if (length != utf16Len) {
        return false;
    }
    if (str1->IsUtf8()) {
        std::vector<uint8_t> buf;
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf);
        return IsUtf8EqualsUtf16(data, length, utf16Data, utf16Len);
    } else {
        std::vector<uint16_t> buf;
        Span<const uint16_t> data1(BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), str1, buf),
                                   length);
        Span<const uint16_t> data2(utf16Data, utf16Len);
        return BaseString::StringsAreEquals(data1, data2);
    }
}
} // namespace common

#endif //COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_IMPL_H