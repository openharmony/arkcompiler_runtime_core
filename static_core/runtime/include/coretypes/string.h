/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_RUNTIME_CORETYPES_STRING_H_
#define PANDA_RUNTIME_CORETYPES_STRING_H_

#include <securec.h>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "libpandabase/utils/utf.h"
#include "runtime/include/language_context.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/vm_handle.h"

namespace ark::coretypes {

class Array;
class String : public ObjectHeader {
public:
    static String *Cast(ObjectHeader *object)
    {
        // NOTE(linxiang) to do assert
        return static_cast<String *>(object);
    }

    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, size_t mutf8Length, uint32_t utf16Length,
                                                    bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable = true, bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                    bool canBeCompressed, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable = true, bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t utf16Length,
                                                    const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                    bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromMUtf8(const uint8_t *mutf8Data, const LanguageContext &ctx, PandaVM *vm,
                                                    bool movable = true, bool pinned = false);

    static String *CreateFromMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length, uint32_t utf16Length,
                                   const LanguageContext &ctx, PandaVM *vm, bool movable, bool pinned);

    PANDA_PUBLIC_API static String *CreateFromUtf8(const uint8_t *utf8Data, uint32_t utf8Length,
                                                   const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                   bool pinned = false);

    PANDA_PUBLIC_API static String *CreateFromUtf16(const uint16_t *utf16Data, uint32_t utf16Length,
                                                    const LanguageContext &ctx, PandaVM *vm, bool movable = true,
                                                    bool pinned = false);

    PANDA_PUBLIC_API static String *CreateEmptyString(const LanguageContext &ctx, PandaVM *vm);

    PANDA_PUBLIC_API static String *CreateFromString(String *str, const LanguageContext &ctx, PandaVM *vm);

    PANDA_PUBLIC_API static String *Concat(String *jstring1, String *jstring2, const LanguageContext &ctx, PandaVM *vm);

    PANDA_PUBLIC_API static String *CreateNewStringFromChars(uint32_t offset, uint32_t length, Array *chararray,
                                                             const LanguageContext &ctx, PandaVM *vm);

    static String *CreateNewStringFromBytes(uint32_t offset, uint32_t length, uint32_t highByte, Array *bytearray,
                                            const LanguageContext &ctx, PandaVM *vm);

    template <bool VERIFY = true>
    uint16_t At(int32_t index);

    PANDA_PUBLIC_API int32_t Compare(String *rstr);

    PANDA_PUBLIC_API Array *ToCharArray(const LanguageContext &ctx);

    PANDA_PUBLIC_API static Array *GetChars(String *src, uint32_t start, uint32_t utf16Length,
                                            const LanguageContext &ctx);

    bool IsUtf16() const
    {
        return compressedStringsEnabled_ ? ((length_ & STRING_COMPRESSED_BIT) == STRING_UNCOMPRESSED) : true;
    }

    bool IsMUtf8() const
    {
        return !IsUtf16();
    }

    static size_t ComputeDataSizeUtf16(uint32_t length)
    {
        return length * sizeof(dataUtf16_[0]);
    }

    /// Methods for uncompressed strings (UTF16)
    static size_t ComputeSizeUtf16(uint32_t utf16Length)
    {
        return sizeof(String) + ComputeDataSizeUtf16(utf16Length);
    }

    uint16_t *GetDataUtf16()
    {
        ASSERT_PRINT(IsUtf16(), "String: Read data as utf16 for mutf8 string");
        return dataUtf16_;
    }

    /// Methods for compresses strings (MUTF8 or LATIN1)
    static size_t ComputeSizeMUtf8(uint32_t mutf8Length)
    {
        return sizeof(String) + mutf8Length;
    }

    /// It's MUtf8 format, but without 0 in the end.
    uint8_t *GetDataMUtf8()
    {
        ASSERT_PRINT(!IsUtf16(), "String: Read data as mutf8 for utf16 string");
        return reinterpret_cast<uint8_t *>(dataUtf16_);
    }

    size_t GetMUtf8Length()
    {
        if (!IsUtf16()) {
            return GetLength() + 1;  // add place for zero at the end
        }
        return ark::utf::Utf16ToMUtf8Size(dataUtf16_, GetLength());
    }

    size_t GetUtf16Length()
    {
        return GetLength();
    }

    inline size_t CopyDataMUtf8(uint8_t *buf, size_t maxLength, bool isCString)
    {
        if (isCString) {
            ASSERT(maxLength != 0);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            buf[maxLength - 1] = '\0';
            return CopyDataRegionMUtf8(buf, 0, GetLength(), maxLength) + 1;  // add place for zero at the end
        }

        return CopyDataRegionMUtf8(buf, 0, GetLength(), maxLength);
    }

    size_t CopyDataRegionMUtf8(uint8_t *buf, size_t start, size_t length, size_t maxLength)
    {
        if (length > maxLength) {
            return 0;
        }
        uint32_t len = GetLength();
        if (start + length > len) {
            return 0;
        }
        if (!IsUtf16()) {
            constexpr size_t MAX_LEN = std::numeric_limits<size_t>::max() / 2 - 1;
            if (length > MAX_LEN) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than half of size_t::max";
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buf, sizeof(uint8_t) * (maxLength + 1), GetDataMUtf8() + start, length) != EOK) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than buf size";
            }
            return length;
        }
        return ark::utf::ConvertRegionUtf16ToMUtf8(GetDataUtf16(), buf, length, maxLength - 1, start);
    }

    inline uint32_t CopyDataUtf16(uint16_t *buf, uint32_t maxLength)
    {
        return CopyDataRegionUtf16(buf, 0, GetLength(), maxLength);
    }

    uint32_t CopyDataRegionUtf16(uint16_t *buf, uint32_t start, uint32_t length, uint32_t maxLength)
    {
        if (length > maxLength) {
            return 0;
        }
        uint32_t len = GetLength();
        if (start + length > len) {
            return 0;
        }
        if (IsUtf16()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buf, sizeof(uint16_t) * maxLength, GetDataUtf16() + start, ComputeDataSizeUtf16(length)) !=
                EOK) {
                LOG(FATAL, RUNTIME) << __func__ << " length is higher than buf size";
            }
        } else {
            uint8_t *src8 = GetDataMUtf8() + start;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            for (uint32_t i = 0; i < length; ++i) {
                buf[i] = src8[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            }
        }
        return length;
    }

    uint32_t GetLength() const
    {
        uint32_t length;
        if (compressedStringsEnabled_) {
            length = length_ >> 1U;
        } else {
            length = length_;
        }
        return length;
    }

    bool IsEmpty() const
    {
        // do not shift right length because it is always zero for empty string
        return length_ == 0;
    }

    size_t ObjectSize() const
    {
        uint32_t length = GetLength();
        return IsUtf16() ? ComputeSizeUtf16(length) : ComputeSizeMUtf8(length);
    }

    uint32_t GetHashcode()
    {
        if (hashcode_ == 0) {
            hashcode_ = ComputeHashcode();
        }
        return hashcode_;
    }

    int32_t IndexOf(String *rhs, int pos = 0);
    int32_t LastIndexOf(String *rhs, int pos = INT32_MAX);

    static constexpr uint32_t GetLengthOffset()
    {
        return MEMBER_OFFSET(String, length_);
    }

    static constexpr uint32_t GetDataOffset()
    {
        return MEMBER_OFFSET(String, dataUtf16_);
    }

    static constexpr uint32_t GetHashcodeOffset()
    {
        return MEMBER_OFFSET(String, hashcode_);
    }

    static constexpr uint32_t GetStringCompressionMask()
    {
        return STRING_COMPRESSED_BIT;
    }

    /// Compares strings by bytes, It doesn't check canonical unicode equivalence.
    PANDA_PUBLIC_API static bool StringsAreEqual(String *str1, String *str2);
    /// Compares strings by bytes, It doesn't check canonical unicode equivalence.
    PANDA_PUBLIC_API static bool StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8Data, uint32_t utf16Length);
    static bool StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8Data, uint32_t utf16Length,
                                     bool canBeCompressed);
    /// Compares strings by bytes, It doesn't check canonical unicode equivalence.
    PANDA_PUBLIC_API static bool StringsAreEqualUtf16(String *str1, const uint16_t *utf16Data,
                                                      uint32_t utf16DataLength);
    static String *DoReplace(String *src, uint16_t oldC, uint16_t newC, const LanguageContext &ctx, PandaVM *vm);
    static uint32_t ComputeHashcodeMutf8(const uint8_t *mutf8Data, uint32_t length);
    static uint32_t ComputeHashcodeMutf8(const uint8_t *mutf8Data, uint32_t utf16Length, bool canBeCompressed);
    static uint32_t ComputeHashcodeUtf16(const uint16_t *utf16Data, uint32_t length);

    static void SetCompressedStringsEnabled(bool val)
    {
        compressedStringsEnabled_ = val;
    }

    static bool GetCompressedStringsEnabled()
    {
        return compressedStringsEnabled_;
    }

    static std::pair<int32_t, int32_t> NormalizeSubStringIndexes(int32_t beginIndex, int32_t endIndex,
                                                                 const coretypes::String *str)
    {
        auto strLen = str->GetLength();
        std::pair<int32_t, int32_t> normIndexes = {beginIndex, endIndex};

        // If begin_index < 0, then it is assumed to be equal to zero.
        if (normIndexes.first < 0) {
            normIndexes.first = 0;
        } else if (static_cast<decltype(strLen)>(normIndexes.first) > strLen) {
            // If begin_index > str_len, then it is assumed to be equal to str_len.
            normIndexes.first = static_cast<int32_t>(strLen);
        }
        // If end_index < 0, then it is assumed to be equal to zero.
        if (normIndexes.second < 0) {
            normIndexes.second = 0;
        } else if (static_cast<decltype(strLen)>(normIndexes.second) > strLen) {
            // If end_index > str_len, then it is assumed to be equal to str_len.
            normIndexes.second = static_cast<int32_t>(strLen);
        }
        // If begin_index > end_index, then these are swapped.
        if (normIndexes.first > normIndexes.second) {
            std::swap(normIndexes.first, normIndexes.second);
        }
        ASSERT((normIndexes.second - normIndexes.first) >= 0);
        return normIndexes;
    }

    static String *FastSubString(String *src, uint32_t start, uint32_t utf16Length, const LanguageContext &ctx,
                                 PandaVM *vm = nullptr);

    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data);

    static bool IsASCIICharacter(uint16_t data)
    {
        // \0 is not considered ASCII in Modified-UTF8
        return data - 1U < utf::UTF8_1B_MAX;
    }

protected:
    void SetLength(uint32_t length, bool compressed = false)
    {
        if (compressedStringsEnabled_) {
            ASSERT(length < 0x80000000U);
            // Use 0u for compressed/utf8 expression
            length_ = (length << 1U) | (compressed ? STRING_COMPRESSED : STRING_UNCOMPRESSED);
        } else {
            length_ = length;
        }
    }

    void SetHashcode(uint32_t hashcode)
    {
        hashcode_ = hashcode;
    }

    uint32_t ComputeHashcode();
    static bool CanBeCompressed(const uint16_t *utf16Data, uint32_t utf16Length);
    static void CopyUtf16AsMUtf8(const uint16_t *utf16From, uint8_t *mutf8To, uint32_t utf16Length);
    static String *AllocStringObject(size_t length, bool compressed, const LanguageContext &ctx, PandaVM *vm = nullptr,
                                     bool movable = true, bool pinned = false);

private:
    PANDA_PUBLIC_API static bool compressedStringsEnabled_;
    static constexpr uint32_t STRING_COMPRESSED_BIT = 0x1;
    enum CompressedStatus {
        STRING_COMPRESSED,
        STRING_UNCOMPRESSED,
    };

    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length);
    static bool CanBeCompressedUtf16(const uint16_t *utf16Data, uint32_t utf16Length, uint16_t non);
    static bool CanBeCompressedMUtf8(const uint8_t *mutf8Data, uint32_t mutf8Length, uint16_t non);

    /**
     * str1 should have the same length as mutf16_data.
     * Converts mutf8_data to mutf16 and compare it with given mutf16_data.
     */
    // NOTE(alovkov): move to utils/utf.h without allocation a temporary buffer
    static bool IsMutf8EqualsUtf16(const uint8_t *utf8Data, uint32_t utf8DataLength, const uint16_t *utf16Data,
                                   uint32_t utf16DataLength);

    static bool IsMutf8EqualsUtf16(const uint8_t *utf8Data, const uint16_t *utf16Data, uint32_t utf16DataLength);

    template <typename T>
    /// Check that two spans are equal. Should have the same length.
    static bool StringsAreEquals(Span<const T> &str1, Span<const T> &str2);

    // In last bit of length_ we store if this string is compressed or not.
    uint32_t length_;
    uint32_t hashcode_;
    // A pointer to the string data stored after the string header.
    // Data can be stored in mutf8 or utf16 form according to compressed bit.
    __extension__ uint16_t dataUtf16_[0];  // NOLINT(modernize-avoid-c-arrays)
};

constexpr uint32_t STRING_LENGTH_OFFSET = 8U;
static_assert(STRING_LENGTH_OFFSET == ark::coretypes::String::GetLengthOffset());
constexpr uint32_t STRING_HASHCODE_OFFSET = 12U;
static_assert(STRING_HASHCODE_OFFSET == ark::coretypes::String::GetHashcodeOffset());
constexpr uint32_t STRING_DATA_OFFSET = 16U;
static_assert(STRING_DATA_OFFSET == ark::coretypes::String::GetDataOffset());

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_STRING_H_
