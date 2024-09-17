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
#ifndef PANDA_RUNTIME_ENTRYPOINTS_STRING_INDEX_OF_H_
#define PANDA_RUNTIME_ENTRYPOINTS_STRING_INDEX_OF_H_

#include <cstdint>
#include <type_traits>
#include "utils/bit_utils.h"
#include "runtime/include/coretypes/string.h"

namespace ark::intrinsics {

/* CC-OFFNXT(G.FUN.01-CPP) false positive */
#if !defined(PANDA_TARGET_ARM64) && !defined(PANDA_TARGET_ARM32) && !defined(PANDA_TARGET_AMD64) && \
    !defined(PANDA_TARGET_X86)
#error \
    "Unknown target architecture. IndexOf implementation assumes that target architecture is little-endian, \
check it and update the file."
#endif

namespace impl {
template <typename CharType, typename LoadType>
struct IndexOfConstants {
};

template <>
struct IndexOfConstants<uint8_t, uint64_t> {
    static constexpr uint64_t XOR_SUB_MASK = 0x0101010101010101ULL;
    static constexpr uint64_t NXOR_OR_MASK = 0x7F7F7F7F7F7F7F7FULL;
    static constexpr size_t VECTOR_SIZE = sizeof(uint64_t) / sizeof(uint8_t);
    static constexpr size_t BITS_PER_CHAR = sizeof(uint8_t) * BITS_PER_BYTE;
};

template <>
struct IndexOfConstants<uint16_t, uint64_t> {
    static constexpr uint64_t XOR_SUB_MASK = 0x0001000100010001ULL;
    static constexpr uint64_t NXOR_OR_MASK = 0x7FFF7FFF7FFF7FFFULL;
    static constexpr size_t VECTOR_SIZE = sizeof(uint64_t) / sizeof(uint16_t);
    static constexpr size_t BITS_PER_CHAR = sizeof(uint16_t) * BITS_PER_BYTE;
};

template <typename CharType, typename LoadType>
LoadType BuildSearchPattern(CharType character) = delete;

template <typename T>
size_t ClzInSwappedBytes(T val) = delete;

#if defined(PANDA_TARGET_64) && defined(__SIZEOF_INT128__)
__extension__ using PandaWideUintT = unsigned __int128;
#else
using PandaWideUintT = uint64_t;
#endif

template <typename CharType, typename LoadType>
int32_t StringIndexOfSwar(CharType *data, int32_t offset, int32_t length, CharType character)
{
    static_assert(std::is_same_v<uint8_t, CharType> || std::is_same_v<uint16_t, CharType>,
                  "Only uint8_t and uint16_t CharTypes are supported");
    auto dataTyped = reinterpret_cast<CharType *>(data);
    auto end = dataTyped + length;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    auto ptr = dataTyped + offset;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto prefixEnd = reinterpret_cast<CharType *>(
        std::min((reinterpret_cast<uintptr_t>(ptr) & ~(sizeof(LoadType) - 1)) + sizeof(LoadType),
                 reinterpret_cast<uintptr_t>(end)));
    ASSERT(prefixEnd >= ptr);
    ASSERT(prefixEnd <= end);
    while (ptr < prefixEnd) {
        if (*ptr == character) {
            return ptr - dataTyped;
        }
        ++ptr;
    }
    auto mainEnd = reinterpret_cast<CharType *>(reinterpret_cast<uintptr_t>(end) & ~(sizeof(LoadType) - 1));
    ASSERT(mainEnd <= end);
    LoadType searchPattern = BuildSearchPattern<CharType, LoadType>(character);
    while (ptr < mainEnd) {
        LoadType value = *reinterpret_cast<LoadType *>(ptr);
        LoadType xoredValue = searchPattern ^ value;
        LoadType eq = (xoredValue - IndexOfConstants<CharType, LoadType>::XOR_SUB_MASK) &
                      ~(xoredValue | IndexOfConstants<CharType, LoadType>::NXOR_OR_MASK);
        if (eq != 0) {
            size_t idx = ClzInSwappedBytes<LoadType>(eq) / IndexOfConstants<CharType, LoadType>::BITS_PER_CHAR;
            ASSERT(ptr[idx] == character);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            return ptr + idx - dataTyped;   // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ptr += IndexOfConstants<CharType, LoadType>::VECTOR_SIZE;
    }
    while (ptr < end) {
        if (*ptr == character) {
            return ptr - dataTyped;
        }
        ++ptr;
    }
    return -1;
}

template <>
inline uint64_t BuildSearchPattern<uint8_t, uint64_t>(uint8_t character)
{
    uint64_t mask = (static_cast<uint64_t>(character) << BITS_PER_BYTE) | character;
    mask = (mask << BITS_PER_UINT16) | mask;
    return (mask << BITS_PER_UINT32) | mask;
}

template <>
inline uint64_t BuildSearchPattern<uint16_t, uint64_t>(uint16_t character)
{
    uint64_t mask = (static_cast<uint64_t>(character) << BITS_PER_UINT16) | character;
    return (mask << BITS_PER_UINT32) | mask;
}

template <>
inline size_t ClzInSwappedBytes<uint64_t>(uint64_t val)
{
    // Don't use ReverseBytes as it may not be replaced with bswap
    return Clz(__builtin_bswap64(val));
}

#if defined(PANDA_TARGET_64) && defined(__SIZEOF_INT128__)
template <>
struct IndexOfConstants<uint16_t, PandaWideUintT> {
    static constexpr PandaWideUintT XOR_SUB_MASK =
        (static_cast<PandaWideUintT>(IndexOfConstants<uint16_t, uint64_t>::XOR_SUB_MASK) << BITS_PER_UINT64) |
        IndexOfConstants<uint16_t, uint64_t>::XOR_SUB_MASK;
    static constexpr PandaWideUintT NXOR_OR_MASK =
        (static_cast<PandaWideUintT>(IndexOfConstants<uint16_t, uint64_t>::NXOR_OR_MASK) << BITS_PER_UINT64) |
        IndexOfConstants<uint16_t, uint64_t>::NXOR_OR_MASK;
    static constexpr size_t VECTOR_SIZE = sizeof(PandaWideUintT) / sizeof(uint16_t);
    static constexpr size_t BITS_PER_CHAR = sizeof(uint16_t) * BITS_PER_BYTE;
};

template <>
inline PandaWideUintT BuildSearchPattern<uint16_t, PandaWideUintT>(uint16_t character)
{
    PandaWideUintT mask = (static_cast<uint64_t>(character) << BITS_PER_UINT16) | character;
    mask = (mask << BITS_PER_UINT32) | mask;
    return (mask << BITS_PER_UINT64) | mask;
}

template <>
inline size_t ClzInSwappedBytes<PandaWideUintT>(PandaWideUintT val)
{
    uint64_t hi = __builtin_bswap64(static_cast<uint64_t>(val >> BITS_PER_UINT64));
    uint64_t lo = __builtin_bswap64(static_cast<uint64_t>(val));
    if (lo == 0) {
        return BITS_PER_UINT64 + Clz(hi);
    }
    return Clz(lo);
}
#endif

inline ALWAYS_INLINE int32_t Utf8StringIndexOfChar(uint8_t *data, size_t offset, size_t length, uint8_t character)
{
    if ((length - offset) >= IndexOfConstants<uint8_t, uint64_t>::VECTOR_SIZE * 2) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto pos = reinterpret_cast<uint8_t *>(memchr(data + offset, character, length - offset));
        return pos == nullptr ? -1 : pos - data;
    }
    return StringIndexOfSwar<uint8_t, uint64_t>(data, offset, length, character);
}

inline ALWAYS_INLINE int32_t Utf16StringIndexOfChar(uint16_t *data, size_t offset, size_t length, uint16_t character)
{
    if constexpr (sizeof(PandaWideUintT) >= sizeof(uint64_t)) {
        if ((length - offset) >= IndexOfConstants<uint16_t, PandaWideUintT>::VECTOR_SIZE * 2) {
            return StringIndexOfSwar<uint16_t, PandaWideUintT>(data, offset, length, character);
        }
    }
    return StringIndexOfSwar<uint16_t, uint64_t>(data, offset, length, character);
}

}  // namespace impl

// CC-OFFNXT(G.FUD.06) perf critical
inline int32_t StringIndexOfU16(void *str, uint16_t character, int32_t offset)
{
    auto string = reinterpret_cast<ark::coretypes::String *>(str);
    ASSERT(string != nullptr);
    bool isUtf8 = string->IsMUtf8();
    auto length = string->GetLength();
    if (offset < 0) {
        offset = 0;
    }

    if (static_cast<uint32_t>(offset) >= length) {
        return -1;
    }
    if (isUtf8) {
        if (character > std::numeric_limits<uint8_t>::max()) {
            return -1;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return impl::Utf8StringIndexOfChar(string->GetDataMUtf8(), offset, length, character);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return impl::Utf16StringIndexOfChar(string->GetDataUtf16(), offset, length, character);
}

}  // namespace ark::intrinsics

#endif
