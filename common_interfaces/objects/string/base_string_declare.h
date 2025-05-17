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

#ifndef COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_DECLARE_H
#define COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_DECLARE_H

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/utils/field_macro.h"
#include "common_interfaces/objects/utils/objects_traits.h"
#include "common_interfaces/objects/readonly_handle.h"
#include "libpandabase/utils/bit_field.h"
#include "libpandabase/utils/span.h"

#include <type_traits>

namespace panda {
class LineString;
class TreeString;
class SlicedString;

/*
 +-----------------------------+ <-- offset 0
 |      BaseObject fields      |
 +-----------------------------+ <-- offset = BaseObjectSize()
 | LengthAndFlags (uint32_t)   | <-- LENGTH_AND_FLAGS_OFFSET
 +-----------------------------+
 | RawHashcode (uint32_t)      | <-- RAW_HASHCODE_OFFSET
 +-----------------------------+ <-- SIZE (== BaseString::SIZE)
 */
/*
 +-----------------------------+
 |   LengthAndFlags (uint32_t) |
 +-----------------------------+
 Bit layout:
   [0]         : CompressedStatusBit         (1 bit)
   [1]         : IsInternBit                 (1 bit)
   [2 - 31]    : LengthBits                  (30 bits)
 */
class BaseString : public BaseObject {
public:
    BASE_CAST_CHECK(BaseString, IsString);
    NO_MOVE_SEMANTIC_CC(BaseString);
    NO_COPY_SEMANTIC_CC(BaseString);
    static constexpr size_t MAX_STRING_LENGTH = 0x40000000U; // 30 bits for string length, 2 bits for special meaning
    static constexpr uint32_t MAX_ELEMENT_INDEX_LEN = 10;
    static constexpr size_t HASH_SHIFT = 5;
    static constexpr size_t LENGTH_AND_FLAGS_OFFSET = BaseObjectSize();
    static constexpr uint32_t STRING_LENGTH_BITS_NUM = 30;
    static constexpr uint32_t BITS_PER_BYTE = 8;

    enum CompressedStatus {
        STRING_COMPRESSED,
        STRING_UNCOMPRESSED,
    };

    enum TrimMode : uint8_t {
        TRIM,
        TRIM_START,
        TRIM_END,
    };

    enum ConcatOptStatus {
        BEGIN_STRING_ADD = 1,
        IN_STRING_ADD,
        CONFIRMED_IN_STRING_ADD,
        END_STRING_ADD,
        INVALID_STRING_ADD,
    };

    using CompressedStatusBit = BitField<CompressedStatus, 0>;                   // 1
    using IsInternBit = CompressedStatusBit::NextFlag;                           // 1
    using LengthBits = IsInternBit::NextField<uint32_t, STRING_LENGTH_BITS_NUM>; // 30
    static_assert(LengthBits::START_BIT + LengthBits::SIZE == sizeof(uint32_t) * BITS_PER_BYTE,
                  "LengthBits does not match the field size");

    PRIMITIVE_FIELD(LengthAndFlags, uint32_t, LENGTH_AND_FLAGS_OFFSET, RAW_HASHCODE_OFFSET)
    // In last bit of mix_hash we store if this string is small-integer number or not.
    PRIMITIVE_FIELD(RawHashcode, uint32_t, RAW_HASHCODE_OFFSET, SIZE)

    bool IsString() const
    {
        return GetBaseClass()->IsString();
    }

    bool IsLineString() const
    {
        return GetBaseClass()->IsLineString();
    }

    bool IsTreeString() const
    {
        return GetBaseClass()->IsTreeString();
    }

    bool IsSlicedString() const
    {
        return GetBaseClass()->IsSlicedString();
    }

    bool IsUtf8() const;

    bool IsUtf16() const;

    // require is LineString
    uint16_t *GetData() const;
    const uint8_t *GetDataUtf8() const;
    const uint16_t *GetDataUtf16() const;

    // require is LineString
    uint8_t *GetDataUtf8Writable();
    uint16_t *GetDataUtf16Writable();

    uint32_t GetLength() const;

    void InitLengthAndFlags(uint32_t length, bool compressed = false, bool isIntern = false);

    template <typename ReadBarrier>
    size_t GetUtf8Length(ReadBarrier &&readBarrier, bool modify = true, bool isGetBufferSize = false) const;

    void SetIsInternString();

    bool IsInternString() const;

    void ClearInternStringFlag();

    bool TryGetHashCode(uint32_t *hash) const;

    // not change this data structure.
    // if string is not flat, this func has low efficiency.
    template <typename ReadBarrier>
    uint32_t PUBLIC_API GetHashcode(ReadBarrier &&readBarrier);

    template <typename ReadBarrier>
    uint32_t PUBLIC_API ComputeRawHashcode(ReadBarrier &&readBarrier) const;

    template <bool verify = true, typename ReadBarrier>
    uint16_t At(ReadBarrier &&readBarrier, int32_t index) const;

    // require is LineString
    void WriteData(uint32_t index, uint16_t src);

    // Compares string1 + string2 by bytes, It doesn't check canonical unicode equivalence.
    template <typename ReadBarrier>
    bool EqualToSplicedString(ReadBarrier &&readBarrier, const BaseString *str1, const BaseString *str2);

    // It allows user to copy into buffer even if maxLength < length
    template <typename ReadBarrier>
    size_t WriteUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength,
                     bool isWriteBuffer = false) const;
    template <typename ReadBarrier>
    size_t CopyDataToUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t length, uint32_t bufLength) const;

    // It allows user to copy into buffer even if maxLength < length
    template <typename ReadBarrier>
    size_t WriteUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t targetLength, uint32_t bufLength) const;

    template <typename ReadBarrier>
    size_t WriteOneByte(ReadBarrier &&readBarrier, uint8_t *buf, size_t maxLength) const;
    template <typename ReadBarrier>
    size_t CopyDataRegionUtf8(ReadBarrier &&readBarrier, uint8_t *buf, size_t start, size_t length,
                              size_t maxLength,
                              bool modify = true, bool isWriteBuffer = false) const;
    template <typename ReadBarrier>
    uint32_t CopyDataUtf16(ReadBarrier &&readBarrier, uint16_t *buf, uint32_t maxLength) const;
    template <typename ReadBarrier>
    std::u16string ToU16String(ReadBarrier &&readBarrier, uint32_t len = 0);

    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>  = 0>
    Span<const uint8_t> ToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify = true, bool cesu8 = false);

    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>  = 0>
    Span<const uint8_t> DebuggerToUtf8Span(ReadBarrier &&readBarrier, Vec &buf, bool modify = true);

    template <typename ReadBarrier>
    void WriteData(ReadBarrier &&readBarrier, BaseString *src, uint32_t start, uint32_t destSize,
                   uint32_t length);

    template <typename ReadBarrier>
    bool IsFlat(ReadBarrier &&readBarrier) const;

    bool NotTreeString() const;

    CommonType GetStringType() const;

    template <class T1, class T2>
    static uint32_t CalculateDataConcatHashCode(const T1 *dataFirst, size_t sizeFirst, const T2 *dataSecond,
                                                size_t sizeSecond);

    static uint32_t ComputeHashcodeUtf8(const uint8_t *utf8Data, size_t utf8Len, bool canBeCompress);
    static uint32_t ComputeHashcodeUtf16(const uint16_t *utf16Data, uint32_t length);

    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>  = 0>
    static BaseString *CreateFromUtf8(Allocator &&allocate, const uint8_t *utf8Data, uint32_t utf8Len,
                                      bool canBeCompress);

    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>  = 0>
    static BaseString *CreateFromUtf8CompressedSubString(Allocator &&allocate,
                                                         const ReadOnlyHandle<BaseString> string,
                                                         uint32_t offset, uint32_t utf8Len);

    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>  = 0>
    static LineString *CreateLineString(Allocator &&allocator, size_t length, bool compressed);

    template <typename Allocator, objects_traits::enable_if_is_allocate<Allocator, BaseObject *>  = 0>
    static BaseString *CreateFromUtf16(Allocator &&allocator, const uint16_t *utf16Data, uint32_t utf16Len,
                                       bool canBeCompress);

    template <typename Allocator, typename WriteBarrier,
              objects_traits::enable_if_is_allocate<Allocator, BaseObject *>  = 0,
              objects_traits::enable_if_is_write_barrier<WriteBarrier>  = 0>
    static SlicedString *CreateSlicedString(Allocator &&allocator, WriteBarrier &&writeBarrier,
                                            ReadOnlyHandle<BaseString> parent);

    template <typename Allocator, typename WriteBarrier,
              objects_traits::enable_if_is_allocate<Allocator, BaseObject *>  = 0,
              objects_traits::enable_if_is_write_barrier<WriteBarrier>  = 0>
    static TreeString *CreateTreeString(Allocator &&allocator, WriteBarrier &&writeBarrier,
                                        ReadOnlyHandle<BaseString> left, ReadOnlyHandle<BaseString> right,
                                        uint32_t length, bool compressed);

    // Check that two spans are equal. Should have the same length.
    /* static */
    template <typename T, typename T1>
    static bool StringsAreEquals(Span<const T> &str1, Span<const T1> &str2);

    // Converts utf8Data to utf16 and compare it with given utf16_data.
    static bool IsUtf8EqualsUtf16(const uint8_t *utf8Data, size_t utf8Len, const uint16_t *utf16Data,
                                  uint32_t utf16Len);

    // Compares strings by bytes, It doesn't check canonical unicode equivalence.
    template <typename ReadBarrier>
    static PUBLIC_API bool StringsAreEqual(ReadBarrier &&readBarrier, BaseString *str1, BaseString *str2);
    // Two strings have the same type of utf encoding format.
    template <typename ReadBarrier>
    static bool StringsAreEqualDiffUtfEncoding(ReadBarrier &&readBarrier, BaseString *str1, BaseString *str2);

    // Compares strings by bytes, It doesn't check canonical unicode equivalence.
    // not change str1 data structure.
    // if str1 is not flat, this func has low efficiency.
    template <typename ReadBarrier>
    static bool StringIsEqualUint8Data(ReadBarrier &&readBarrier, const BaseString *str1,
                                       const uint8_t *dataAddr, uint32_t dataLen,
                                       bool canBeCompress);
    // Compares strings by bytes, It doesn't check canonical unicode equivalence.
    // not change str1 data structure.
    // if str1 is not flat, this func has low efficiency.
    template <typename ReadBarrier>
    static bool StringsAreEqualUtf16(ReadBarrier &&readBarrier, const BaseString *str1,
                                     const uint16_t *utf16Data, uint32_t utf16Len);

    static bool CanBeCompressed(const uint8_t *utf8Data, uint32_t utf8Len);

    static bool CanBeCompressed(const uint16_t *utf16Data, uint32_t utf16Len);

    static bool CanBeCompressed(const BaseString *string);

    // single char copy for loop
    template <typename DstType, typename SrcType>
    static void CopyChars(DstType *dst, SrcType *src, uint32_t count);

    // To change the hash algorithm of BaseString, please modify BaseString::CalculateConcatHashCode
    // and BaseStringHashHelper::ComputeHashForDataPlatform simultaneously!!
    template <typename T>
    static uint32_t ComputeHashForData(const T *data, size_t size, uint32_t hashSeed);

    static bool IsASCIICharacter(uint16_t data);

    template <typename T1, typename T2>
    static int32_t IndexOf(Span<const T1> &lhsSp, Span<const T2> &rhsSp, int32_t pos, int32_t max);

    template <typename T1, typename T2>
    static int32_t LastIndexOf(Span<const T1> &lhsSp, Span<const T2> &rhsSp, int32_t pos);

    template <typename Char, typename ReadBarrier>
    static void WriteToFlat(ReadBarrier &&readBarrier, BaseString *src, Char *buf, uint32_t maxLength);

    template <typename Char, typename ReadBarrier>
    static void WriteToFlatWithPos(ReadBarrier &&readBarrier, BaseString *src, Char *buf, uint32_t length,
                                   uint32_t pos);

    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint8_t>, int>  = 0>
    static const uint8_t *PUBLIC_API GetUtf8DataFlat(ReadBarrier &&readBarrier, const BaseString *src, Vec &buf);

    template <typename ReadBarrier>
    static const uint8_t *PUBLIC_API GetNonTreeUtf8Data(ReadBarrier &&readBarrier, const BaseString *src);

    template <typename ReadBarrier, typename Vec,
              std::enable_if_t<objects_traits::is_std_vector_of_v<std::decay_t<Vec>, uint16_t>, int>  = 0>
    static const uint16_t *PUBLIC_API GetUtf16DataFlat(ReadBarrier &&readBarrier, const BaseString *src,
                                                       Vec &buf);

    template <typename ReadBarrier>
    static const uint16_t *PUBLIC_API GetNonTreeUtf16Data(ReadBarrier &&readBarrier, const BaseString *src);
};
} // namespace panda
#endif // COMMON_INTERFACES_OBJECTS_STRING_BASE_STRING_DECLARE_H