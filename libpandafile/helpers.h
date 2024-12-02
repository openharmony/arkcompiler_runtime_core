/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef LIBPANDAFILE_HELPERS_H
#define LIBPANDAFILE_HELPERS_H

#include "file.h"
#include "file_error.h"
#include "macros.h"
#include "utils/bit_helpers.h"
#include "utils/leb128.h"
#include "utils/logger.h"
#include "utils/span.h"

#include <cstdint>
#include <exception>
#include <limits>
#include <optional>

namespace panda::panda_file::helpers {

constexpr size_t UINT_BYTE2_SHIFT = 8U;
constexpr size_t UINT_BYTE3_SHIFT = 16U;
constexpr size_t UINT_BYTE4_SHIFT = 24U;

class FileAccessException : public std::exception {
public:
    explicit FileAccessException(const char *msg) : msg_(msg) {}
    explicit FileAccessException(const std::string_view &msg) : msg_(msg) {}
    const char *what() const noexcept override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

#ifndef SUPPORT_KNOWN_EXCEPTION
#define THROW_IF(cond, msg)              \
    if (UNLIKELY(cond)) {                \
        LOG(FATAL, PANDAFILE) << (msg);  \
    }
#else
#define THROW_IF(cond, msg)                       \
    if (UNLIKELY(cond)) {                         \
        throw helpers::FileAccessException(msg);  \
    }
#endif

template <size_t width>
inline auto Read(Span<const uint8_t> *sp, const File *file = nullptr)
{
    constexpr size_t BYTE_WIDTH = std::numeric_limits<uint8_t>::digits;
    constexpr size_t BITWIDTH = BYTE_WIDTH * width;
    using unsigned_type = panda::helpers::TypeHelperT<BITWIDTH, false>;

    unsigned_type result = 0;
    if (file != nullptr) {
        file->ThrowIfWithCheck(sp->Size() < width, FileError::NOT_ENOUGH_SPAN_SIZE);
    } else {
        THROW_IF(sp->Size() < width, FileError::NOT_ENOUGH_SPAN_SIZE);
    }

    for (size_t i = 0; i < width; i++) {
        unsigned_type tmp = static_cast<unsigned_type>((*sp)[i]) << (i * BYTE_WIDTH);
        result |= tmp;
    }
    *sp = sp->SubSpan(width);
    return result;
}

template <>
inline auto Read<sizeof(uint16_t)>(Span<const uint8_t> *sp, const File *file)
{
    uint16_t result = 0;
    if (file != nullptr) {
        file->ThrowIfWithCheck(sp->Size() < sizeof(uint16_t), FileError::NOT_ENOUGH_SPAN_SIZE);
    } else {
        THROW_IF(sp->Size() < sizeof(uint16_t), FileError::NOT_ENOUGH_SPAN_SIZE);
    }
    
    auto *p = sp->data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result = *(p++);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, hicpp-signed-bitwise)
    result |= static_cast<uint16_t>(*p) << UINT_BYTE2_SHIFT;
    *sp = sp->SubSpan(sizeof(uint16_t));
    return result;
}

template <>
inline auto Read<sizeof(uint32_t)>(Span<const uint8_t> *sp, const File *file)
{
    uint32_t result = 0;
    if (file != nullptr) {
        file->ThrowIfWithCheck(sp->Size() < sizeof(uint32_t), FileError::NOT_ENOUGH_SPAN_SIZE);
    } else {
        THROW_IF(sp->Size() < sizeof(uint32_t), FileError::NOT_ENOUGH_SPAN_SIZE);
    }

    auto *p = sp->data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result = *(p++);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result |= static_cast<uint32_t>(*(p++)) << UINT_BYTE2_SHIFT;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result |= static_cast<uint32_t>(*(p++)) << UINT_BYTE3_SHIFT;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result |= static_cast<uint32_t>(*p) << UINT_BYTE4_SHIFT;
    *sp = sp->SubSpan(sizeof(uint32_t));
    return result;
}

template <size_t width>
inline auto Read(Span<const uint8_t> sp, const File *file = nullptr)
{
    return Read<width>(&sp, file);
}

inline uint32_t ReadULeb128(Span<const uint8_t> *sp, const File *file = nullptr,
                            std::optional<File::EntityId> code_id = std::nullopt)
{
    if (file != nullptr) {
        file->ThrowIfWithCheck(sp->Size() == 0U, FileError::SPAN_SIZE_IS_ZERO);
    } else {
        THROW_IF(sp->Size() == 0U, FileError::SPAN_SIZE_IS_ZERO);
    }

    uint32_t result;
    size_t n;
    bool is_full;
    /**
     * Decode an unsigned 32-bit integer using LEB128 variable-length encoding format
     * result: the decoded value
     * n: number of bytes used in decoding
     * is_full: Indicates whether the decoding is valid. For uint32_t type, the first 4 bytes
     *          use 7 bits each (28 bits in total), and the 5th byte can only use the remaining
     *          4 bits (value must be <16). If the fifth byte uses more than 4 bits, it indicates
     *          invalid decoding.
     */
    std::tie(result, n, is_full) = leb128::DecodeUnsigned<uint32_t>(sp->data());

    if (!is_full || sp->Size() < n) {
        LOG(ERROR, PANDAFILE) << "LEB128 bytes (invalid span): ";
        const uint8_t* data = sp->data();
        for (size_t i = 0; i < std::min(n, sp->Size()); i++) {
            LOG(ERROR, PANDAFILE) << std::hex << static_cast<int>(data[i]) << " ";
        }
        LOG(ERROR, PANDAFILE) << "\nDecoded value: " << std::dec << result
                              << ", Expected bytes: " << n
                              << ", Actual span size: " << sp->Size();
    }

    if (file != nullptr) {
        file->ThrowIfWithCheck(!is_full, FileError::INVALID_LEB128_ENCODING, FileError::READULEB128, code_id);
        file->ThrowIfWithCheck(sp->Size() < n, FileError::NOT_ENOUGH_SPAN_SIZE, FileError::READULEB128, code_id);
    } else {
        THROW_IF(!is_full, FileError::INVALID_LEB128_ENCODING);
        THROW_IF(sp->Size() < n, FileError::NOT_ENOUGH_SPAN_SIZE);
    }
    *sp = sp->SubSpan(n);
    return result;
}

inline void SkipULeb128(Span<const uint8_t> *sp)
{
    if (sp->Size() > 0U && (*sp)[0U] <= leb128::PAYLOAD_MASK) {
        *sp = sp->SubSpan(1U);
        return;
    }

    if (sp->Size() > 1U && (*sp)[1U] <= leb128::PAYLOAD_MASK) {
        *sp = sp->SubSpan(2U);
        return;
    }

    if (sp->Size() > 2U && (*sp)[2U] <= leb128::PAYLOAD_MASK) {
        *sp = sp->SubSpan(3U);
        return;
    }

    if (sp->Size() > 3U && (*sp)[3U] <= leb128::PAYLOAD_MASK) {
        *sp = sp->SubSpan(4U);
    }
}

inline int32_t ReadLeb128(Span<const uint8_t> *sp, const File *file = nullptr)
{
    uint32_t result;
    size_t n;
    bool is_full;
    std::tie(result, n, is_full) = leb128::DecodeSigned<int32_t>(sp->data());
    if (file != nullptr) {
        file->ThrowIfWithCheck(!is_full, FileError::INVALID_LEB128_ENCODING);
        file->ThrowIfWithCheck(sp->Size() < n, FileError::NOT_ENOUGH_SPAN_SIZE);
    } else {
        THROW_IF(!is_full, FileError::INVALID_LEB128_ENCODING);
        THROW_IF(sp->Size() < n, FileError::NOT_ENOUGH_SPAN_SIZE);
    }
    *sp = sp->SubSpan(n);
    return result;
}

template <size_t alignment>
inline const uint8_t *Align(const uint8_t *ptr)
{
    auto intptr = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t aligned = (intptr - 1U + alignment) & -alignment;
    return reinterpret_cast<const uint8_t *>(aligned);
}

template <size_t alignment, class T>
inline T Align(T n)
{
    return (n - 1U + alignment) & -alignment;
}

template <class T, class E>
inline std::optional<T> GetOptionalTaggedValue(Span<const uint8_t> sp, E tag, Span<const uint8_t> *next)
{
    // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
    // which fails Release builds for GCC 8 and 9.
    std::optional<T> novalue = {};
    if (UNLIKELY(sp.Size() == 0U)) {
        return novalue;
    }

    if (sp[0] == static_cast<uint8_t>(tag)) {
        sp = sp.SubSpan(1);
        T value = static_cast<T>(Read<sizeof(T)>(&sp));
        *next = sp;
        return value;
    }
    *next = sp;

    return novalue;
}

template <class T, class E, class Callback>
inline void EnumerateTaggedValues(Span<const uint8_t> sp, E tag, Callback cb, Span<const uint8_t> *next)
{
    while (sp.Size() > 0U && sp[0] == static_cast<uint8_t>(tag)) {
        sp = sp.SubSpan(1);
        T value(Read<sizeof(T)>(&sp));
        cb(value);
    }

    if (next == nullptr) {
        return;
    }

    *next = sp;
}

template <class T, class E, class Callback>
inline bool EnumerateTaggedValuesWithEarlyStop(Span<const uint8_t> sp, E tag, Callback cb)
{
    while (sp.Size() > 0U && sp[0] == static_cast<uint8_t>(tag)) {
        sp = sp.SubSpan(1);
        T value(Read<sizeof(T)>(&sp));
        if (cb(value)) {
            return true;
        }
    }

    return false;
}

}  // namespace panda::panda_file::helpers

#endif  // LIBPANDAFILE_HELPERS_H
