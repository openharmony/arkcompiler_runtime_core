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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_INL2_H
#define COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_INL2_H

#include "common_interfaces/objects/string/base_string_declare.h"
#include "common_interfaces/objects/string/line_string.h"
#include "common_interfaces/objects/string/sliced_string.h"
#include "common_interfaces/objects/string/tree_string.h"
#include "common_interfaces/objects/utils/utf_utils.h"

namespace panda {
template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
BaseString *BaseString::CreateFromUtf8(Allocator &&allocator, const uint8_t *utf8Data, uint32_t utf8Len,
                                       bool canBeCompress)
{
    BaseString *string = nullptr;
    if (canBeCompress) {
        string = CreateLineString(allocator, utf8Len, true);
        ASSERT(string != nullptr);
        std::copy(utf8Data, utf8Data + utf8Len, string->GetDataUtf8Writable());
    } else {
        auto utf16Len = panda::utf_utils::Utf8ToUtf16Size(utf8Data, utf8Len);
        string = CreateLineString(allocator, utf16Len, false);
        ASSERT(string != nullptr);
        [[maybe_unused]] auto len =
            panda::utf_utils::ConvertRegionUtf8ToUtf16(utf8Data, string->GetDataUtf16Writable(), utf8Len, utf16Len);
        ASSERT(len == utf16Len);
    }

    ASSERT_PRINT(canBeCompress == CanBeCompressed(string), "Bad input canBeCompress!");
    return string;
}

template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
BaseString *BaseString::CreateFromUtf8CompressedSubString(Allocator &&allocator,
                                                          const ReadOnlyHandle<BaseString> string,
                                                          uint32_t offset, uint32_t utf8Len)
{
    BaseString *subString = CreateLineString(allocator, utf8Len, true);
    ASSERT(subString != nullptr);

    auto *utf8Data = string->GetDataUtf8() + offset;
    std::copy(utf8Data, utf8Data + utf8Len, subString->GetDataUtf8Writable());
    ASSERT_PRINT(CanBeCompressed(subString), "String cannot be compressed!");
    return subString;
}

template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
BaseString *BaseString::CreateFromUtf16(Allocator &&allocator, const uint16_t *utf16Data, uint32_t utf16Len,
                                        bool canBeCompress)
{
    auto string = CreateLineString(allocator, utf16Len, canBeCompress);
    ASSERT(string != nullptr);

    if (canBeCompress) {
        CopyChars(string->GetDataUtf8Writable(), utf16Data, utf16Len);
    } else {
        uint32_t len = utf16Len * (sizeof(uint16_t) / sizeof(uint8_t));
        if (memcpy_s(string->GetDataUtf16Writable(), len, utf16Data, len) != EOK) {
            UNREACHABLE();
        }
    }

    ASSERT_PRINT(canBeCompress == CanBeCompressed(string), "Bad input canBeCompress!");
    return string;
}

template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>>
LineString *BaseString::CreateLineString(Allocator &&allocator, size_t length, bool compressed)
{
    size_t size = compressed ? LineString::ComputeSizeUtf8(length) : LineString::ComputeSizeUtf16(length);
    BaseObject *obj = std::invoke(allocator, size, CommonType::LINE_STRING);
    LineString *string = LineString::Cast(obj);
    string->InitLengthAndFlags(length, compressed);
    string->SetRawHashcode(0);
    return string;
}

template <typename Allocator, typename WriteBarrier,
          objects_traits::enable_if_is_allocate<Allocator, BaseObject *>,
          objects_traits::enable_if_is_write_barrier<WriteBarrier>>
SlicedString *BaseString::CreateSlicedString(Allocator &&allocate, WriteBarrier &&writeBarrier,
                                             ReadOnlyHandle<BaseString> parent)
{
    SlicedString *slicedString = SlicedString::Cast(
        std::invoke(allocate, SlicedString::SIZE, CommonType::SLICED_STRING));
    slicedString->SetRawHashcode(0);
    slicedString->SetParent(std::forward<WriteBarrier>(writeBarrier), parent.GetBaseObject());
    return slicedString;
}


template <typename Allocator, typename WriteBarrier,
          objects_traits::enable_if_is_allocate<Allocator, BaseObject *>,
          objects_traits::enable_if_is_write_barrier<WriteBarrier>>
TreeString *BaseString::CreateTreeString(Allocator &&allocator, WriteBarrier &&writeBarrier,
                                         ReadOnlyHandle<BaseString> left, ReadOnlyHandle<BaseString> right,
                                         uint32_t length, bool compressed)
{
    auto string = TreeString::Cast(
        std::invoke(allocator, TreeString::SIZE, CommonType::TREE_STRING));
    string->InitLengthAndFlags(length, compressed);
    string->SetRawHashcode(0);
    string->SetFirst(writeBarrier, left.GetBaseObject());
    string->SetSecond(writeBarrier, right.GetBaseObject());
    return string;
}

template <typename ReadBarrier>
size_t BaseString::GetUtf8Length(ReadBarrier &&readBarrier, bool modify, bool isGetBufferSize) const
{
    if (!IsUtf16()) {
        return GetLength() + 1; // add place for zero in the end
    }
    std::vector<uint16_t> tmpBuf;
    const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    return panda::utf_utils::Utf16ToUtf8Size(data, GetLength(), modify, isGetBufferSize);
}

template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint16_t>, int>>
const uint16_t *BaseString::GetUtf16DataFlat(ReadBarrier &&readBarrier, const BaseString *src, Vec &buf)
{
    ASSERT(src->IsUtf16());
    uint32_t length = src->GetLength();
    BaseString *string = const_cast<BaseString *>(src);
    if (string->IsTreeString()) {
        if (string->IsFlat(std::forward<ReadBarrier>(readBarrier))) {
            string = BaseString::Cast(
                TreeString::Cast(string)->GetFirst<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
        } else {
            buf.reserve(length);
            WriteToFlat(std::forward<ReadBarrier>(readBarrier), string, buf.data(), length);
            return buf.data();
        }
    } else if (string->IsSlicedString()) {
        SlicedString *str = SlicedString::Cast(string);
        return BaseString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf16() +
               str->GetStartIndex();
    }
    return string->GetDataUtf16();
}

inline CommonType BaseString::GetStringType() const
{
    CommonType type = GetBaseClass()->GetObjectType();
    ASSERT_PRINT(type >= CommonType::STRING_FIRST && type <= CommonType::STRING_LAST,
                 "Invalid CommonType: " + std::to_string(static_cast<uint8_t>(type)));
    return type;
}

template <bool verify, typename ReadBarrier>
uint16_t BaseString::At(ReadBarrier &&readBarrier, int32_t index) const
{
    int32_t length = static_cast<int32_t>(GetLength());
    if constexpr (verify) {
        if ((index < 0) || (index >= length)) {
            return 0;
        }
    }
    switch (GetStringType()) {
        case CommonType::LINE_STRING:
            return LineString::ConstCast(this)->Get<verify>(index);
        case CommonType::SLICED_STRING:
            return SlicedString::ConstCast(this)->Get<verify>(std::forward<ReadBarrier>(readBarrier), index);
        case CommonType::TREE_STRING:
            return TreeString::ConstCast(this)->Get<verify>(std::forward<ReadBarrier>(readBarrier), index);
        default:
            UNREACHABLE();
    }
}

inline uint16_t *BaseString::GetData() const
{
    ASSERT_PRINT(IsLineString(), "BaseString: Read data from not LineString");
    return LineString::ConstCast(this)->GetData();
}

inline uint16_t *LineString::GetData() const
{
    return reinterpret_cast<uint16_t *>(ToUintPtr(this) + DATA_OFFSET);
}

inline const uint8_t *BaseString::GetDataUtf8() const
{
    ASSERT_PRINT(IsUtf8(), "BaseString: Read data as utf8 for utf16 string");
    return reinterpret_cast<uint8_t *>(GetData());
}

inline const uint16_t *BaseString::GetDataUtf16() const
{
    ASSERT_PRINT(IsUtf16(), "BaseString: Read data as utf16 for utf8 string");
    return GetData();
}

inline uint8_t *BaseString::GetDataUtf8Writable()
{
    ASSERT_PRINT(IsUtf8(), "BaseString: Read data as utf8 for utf16 string");
    return reinterpret_cast<uint8_t *>(GetData());
}

inline uint16_t *BaseString::GetDataUtf16Writable()
{
    ASSERT_PRINT(IsUtf16(), "BaseString: Read data as utf16 for utf8 string");
    return GetData();
}


inline void BaseString::WriteData(uint32_t index, uint16_t src)
{
    ASSERT(index < GetLength());
    ASSERT(IsLineString());
    LineString::Cast(this)->Set(index, src);
}


template <typename ReadBarrier>
bool BaseString::IsFlat(ReadBarrier &&readBarrier) const
{
    if (!this->IsTreeString()) {
        return true;
    }
    return TreeString::ConstCast(this)->IsFlat(std::forward<ReadBarrier>(readBarrier));
}

template <typename Char, typename ReadBarrier>
// CC-OFFNXT(huge_depth, huge_method, G.FUN.01-CPP) solid logic
void BaseString::WriteToFlat(ReadBarrier &&readBarrier, BaseString *src, Char *buf, uint32_t maxLength)
{
    // DISALLOW_GARBAGE_COLLECTION;
    uint32_t length = src->GetLength();
    if (length == 0) {
        return;
    }
    while (true) {
        ASSERT(length <= maxLength && length > 0);
        ASSERT(length <= src->GetLength());
        switch (src->GetStringType()) {
            case CommonType::LINE_STRING: {
                if (src->IsUtf8()) {
                    CopyChars(buf, src->GetDataUtf8(), length);
                } else {
                    CopyChars(buf, src->GetDataUtf16(), length);
                }
                return;
            }
            case CommonType::TREE_STRING: {
                TreeString *treeSrc = TreeString::Cast(src);
                BaseString *first = BaseString::Cast(
                    treeSrc->GetFirst<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                BaseString *second = BaseString::Cast(
                    treeSrc->GetSecond<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                uint32_t firstLength = first->GetLength();
                uint32_t secondLength = second->GetLength();
                // NOLINTNEXTLINE(C_RULE_ID_FUNCTION_NESTING_LEVEL)
                if (secondLength >= firstLength) {
                    // second string is longer. So recurse over first.
                    WriteToFlat(std::forward<ReadBarrier>(readBarrier), first, buf, maxLength);
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (first == second) {
                        CopyChars(buf + firstLength, buf, firstLength);
                        return;
                    }
                    buf += firstLength;
                    maxLength -= firstLength;
                    src = second;
                    length -= firstLength;
                } else {
                    // first string is longer.  So recurse over second.
                    // if src{first:A,second:B} is half flat to {first:A+B,second:empty} by another thread
                    // but the other thread didn't end, and this thread get  {first:A+B,second:B}
                    // it may cause write buffer overflower in line 424, buf + firstLength is overflower.
                    // so use 'length > firstLength' instead of 'secondLength > 0'
                    // CC-OFFNXT(G.FUN.01-CPP) solid logic
                    if (length > firstLength) {
                        if (secondLength == 1) {
                            buf[firstLength] = static_cast<Char>(second->At<false>(
                                std::forward<ReadBarrier>(readBarrier), 0));
                        } else if ((second->IsLineString()) && second->IsUtf8()) {
                            CopyChars(buf + firstLength, second->GetDataUtf8(), secondLength);
                        } else {
                            WriteToFlat(std::forward<ReadBarrier>(readBarrier), second, buf + firstLength,
                                        maxLength - firstLength);
                        }
                        length -= secondLength;
                    }
                    maxLength = firstLength;
                    src = first;
                }
                continue;
            }
            case CommonType::SLICED_STRING: {
                BaseString *parent = BaseString::Cast(
                    SlicedString::Cast(src)->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                if (src->IsUtf8()) {
                    CopyChars(buf, parent->GetDataUtf8() + SlicedString::Cast(src)->GetStartIndex(), length);
                } else {
                    CopyChars(buf, parent->GetDataUtf16() + SlicedString::Cast(src)->GetStartIndex(), length);
                }
                return;
            }
            default:
                UNREACHABLE();
        }
    }
}


template <typename Char, typename ReadBarrier>
void BaseString::WriteToFlatWithPos(ReadBarrier &&readBarrier, BaseString *src, Char *buf, uint32_t length,
                                    uint32_t pos)
{
    // DISALLOW_GARBAGE_COLLECTION;
    [[ maybe_unused ]] uint32_t maxLength = src->GetLength();
    if (length == 0) {
        return;
    }
    while (true) {
        ASSERT(length + pos <= maxLength && length > 0);
        ASSERT(length <= src->GetLength());
        ASSERT(pos >= 0);
        switch (src->GetStringType()) {
            case CommonType::LINE_STRING: {
                if (src->IsUtf8()) {
                    CopyChars(buf, src->GetDataUtf8() + pos, length);
                } else {
                    CopyChars(buf, src->GetDataUtf16() + pos, length);
                }
                return;
            }
            case CommonType::TREE_STRING: {
                TreeString *treeSrc = TreeString::Cast(src);
                BaseString *first = BaseString::Cast(
                    treeSrc->GetFirst<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                ASSERT(first->IsLineString());
                src = first;
                continue;
            }
            case CommonType::SLICED_STRING: {
                BaseString *parent = BaseString::Cast(
                    SlicedString::Cast(src)->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
                if (src->IsUtf8()) {
                    CopyChars(buf, parent->GetDataUtf8() + SlicedString::Cast(src)->GetStartIndex() + pos, length);
                } else {
                    CopyChars(buf, parent->GetDataUtf16() + SlicedString::Cast(src)->GetStartIndex() + pos, length);
                }
                return;
            }
            default:
                UNREACHABLE();
        }
    }
}

// It allows user to copy into buffer even if maxLength < length
template <typename ReadBarrier>
size_t BaseString::WriteUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength, bool isWriteBuffer) const
{
    if (maxLength == 0) {
        return 1; // maxLength was -1 at napi
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    buf[maxLength - 1] = '\0';
    return CopyDataRegionUtf8(std::forward<ReadBarrier>(readBarrier), buf, 0, GetLength(), maxLength, true,
                              isWriteBuffer) + 1;
}

// It allows user to copy into buffer even if maxLength < length
template <typename ReadBarrier>
size_t BaseString::WriteUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t targetLength,
                              uint32_t bufLength) const
{
    if (bufLength == 0) {
        return 0;
    }
    // Returns a number representing a valid backrest length.
    return CopyDataToUtf16(std::forward<ReadBarrier>(readBarrier), buf, targetLength, bufLength);
}


template <typename ReadBarrier>
size_t BaseString::WriteOneByte(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength) const
{
    if (maxLength == 0) {
        return 0;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    buf[maxLength - 1] = '\0';
    uint32_t length = GetLength();
    if (!IsUtf16()) {
        std::vector<uint8_t> tmpBuf;
        const uint8_t *data = GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        if (length > maxLength) {
            length = maxLength;
        }
        if (memcpy_s(buf, maxLength, data, length) != EOK) {
            UNREACHABLE();
        }
        return length;
    }

    std::vector<uint16_t> tmpBuf;
    const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    if (length > maxLength) {
        return panda::utf_utils::ConvertRegionUtf16ToLatin1(data, buf, maxLength, maxLength);
    }
    return panda::utf_utils::ConvertRegionUtf16ToLatin1(data, buf, length, maxLength);
}


template <typename ReadBarrier>
uint32_t BaseString::CopyDataUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t maxLength) const
{
    uint32_t length = GetLength();
    if (length > maxLength) {
        return 0;
    }
    if (IsUtf16()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::vector<uint16_t> tmpBuf;
        const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        if (memcpy_s(buf, maxLength * sizeof(uint16_t), data, length * sizeof(uint16_t)) != EOK) {
            UNREACHABLE();
        }
        return length;
    }
    std::vector<uint8_t> tmpBuf;
    const uint8_t *data = GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    return panda::utf_utils::ConvertRegionUtf8ToUtf16(data, buf, length, maxLength);
}


template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>>
Span<const uint8_t> BaseString::ToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify, bool cesu8)
{
    Span<const uint8_t> str;
    uint32_t strLen = GetLength();
    if (UNLIKELY(IsUtf16())) {
        using U16Vec = objects_traits::vector_with_same_alloc_t<Vec, uint16_t>;
        U16Vec tmpBuf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        ASSERT(panda::utf_utils::Utf16ToUtf8Size(data, strLen, modify, false, cesu8) > 0);
        size_t len = panda::utf_utils::Utf16ToUtf8Size(data, strLen, modify, false, cesu8) - 1;
        buf.reserve(len);
        len = panda::utf_utils::ConvertRegionUtf16ToUtf8(data, buf.data(), strLen, len, 0, modify, false, cesu8);
        str = Span<const uint8_t>(buf.data(), len);
    } else {
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        str = Span<const uint8_t>(data, strLen);
    }
    return str;
}


template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>>
Span<const uint8_t> BaseString::DebuggerToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify)
{
    Span<const uint8_t> str;
    uint32_t strLen = GetLength();
    if (UNLIKELY(IsUtf16())) {
        using U16Vec = objects_traits::vector_with_same_alloc_t<Vec, uint16_t>;
        U16Vec tmpBuf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        size_t len = panda::utf_utils::Utf16ToUtf8Size(data, strLen, modify) - 1;
        buf.reserve(len);
        len = panda::utf_utils::DebuggerConvertRegionUtf16ToUtf8(data, buf.data(), strLen, len, 0, modify);
        str = Span<const uint8_t>(buf.data(), len);
    } else {
        const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, buf);
        str = Span<const uint8_t>(data, strLen);
    }
    return str;
}

// single char copy for loop
template <typename DstType, typename SrcType>
void BaseString::CopyChars(DstType *dst, SrcType *src, uint32_t count)
{
    Span<SrcType> srcSp(src, count);
    Span<DstType> dstSp(dst, count);
    for (uint32_t i = 0; i < count; i++) {
        dstSp[i] = srcSp[i];
    }
}

template <typename ReadBarrier, typename Vec,
          std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>>
const uint8_t *BaseString::GetUtf8DataFlat(ReadBarrier &&readBarrier, const BaseString *src, Vec &buf)
{
    ASSERT(src->IsUtf8());
    uint32_t length = src->GetLength();
    BaseString *string = const_cast<BaseString *>(src);
    if (string->IsTreeString()) {
        if (string->IsFlat(std::forward<ReadBarrier>(readBarrier))) {
            string = BaseString::Cast(
                TreeString::Cast(string)->GetFirst<BaseObject *>(std::forward<ReadBarrier>(readBarrier)));
        } else {
            buf.reserve(length);
            WriteToFlat(std::forward<ReadBarrier>(readBarrier), string, buf.data(), length);
            return buf.data();
        }
    } else if (string->IsSlicedString()) {
        SlicedString *str = SlicedString::Cast(string);
        return BaseString::Cast(str->GetParent<BaseObject *>(std::forward<ReadBarrier>(readBarrier)))->GetDataUtf8() +
               str->GetStartIndex();
    }
    return string->GetDataUtf8();
}


template <typename ReadBarrier>
size_t BaseString::CopyDataRegionUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t start, size_t length,
                                      size_t maxLength, bool modify, bool isWriteBuffer) const
{
    uint32_t len = GetLength();
    if (start + length > len) {
        return 0;
    }
    constexpr size_t TWO_TIMES = 2;
    if (!IsUtf16()) {
        if (length > (std::numeric_limits<size_t>::max() / TWO_TIMES - 1)) {
            // 2: half
            UNREACHABLE();
        }
        std::vector<uint8_t> tmpBuf;
        const uint8_t *data = GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf) + start;
        // Only copy maxLength number of chars into buffer if length > maxLength
        auto dataLen = std::min(length, maxLength);
        std::copy(data, data + dataLen, buf);
        return dataLen;
    }
    std::vector<uint16_t> tmpBuf;
    const uint16_t *data = GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    if (length > maxLength) {
        return panda::utf_utils::ConvertRegionUtf16ToUtf8(data, buf, maxLength, maxLength, start,
                                                          modify, isWriteBuffer);
    }
    return panda::utf_utils::ConvertRegionUtf16ToUtf8(data, buf, length, maxLength, start,
                                                      modify, isWriteBuffer);
}

template <typename ReadBarrier>
size_t BaseString::CopyDataToUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t length,
                                   uint32_t bufLength) const
{
    if (IsUtf16()) {
        std::vector<uint16_t> tmpBuf;
        const uint16_t *data = BaseString::GetUtf16DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
        if (length > bufLength) {
            if (memcpy_s(buf, bufLength * sizeof(uint16_t), data, bufLength * sizeof(uint16_t)) != EOK) {
                UNREACHABLE();
            }
            return bufLength;
        }
        if (memcpy_s(buf, bufLength * sizeof(uint16_t), data, length * sizeof(uint16_t)) != EOK) {
            UNREACHABLE();
        }
        return length;
    }
    std::vector<uint8_t> tmpBuf;
    const uint8_t *data = BaseString::GetUtf8DataFlat(std::forward<ReadBarrier>(readBarrier), this, tmpBuf);
    if (length > bufLength) {
        return panda::utf_utils::ConvertRegionUtf8ToUtf16(data, buf, bufLength, bufLength);
    }
    return panda::utf_utils::ConvertRegionUtf8ToUtf16(data, buf, length, bufLength);
}
} // namespace panda::ecmascript

#endif //COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_INL2_H