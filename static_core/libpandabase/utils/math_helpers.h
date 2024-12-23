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
#ifndef PANDA_LIBBASE_UTILS_MATH_HELPERS_H_
#define PANDA_LIBBASE_UTILS_MATH_HELPERS_H_

#include "bit_utils.h"
#include "macros.h"

#include <array>
#include <climits>
#include <cstdint>
#include <cmath>

namespace ark::helpers::math {

/**
 * @brief returns log2 for argument
 * @param X - should be power of 2
 * @return log2(X) or undefined if X 0
 */
constexpr uint32_t GetIntLog2(const uint32_t x)
{
    ASSERT((x > 0) && !(x & (x - 1U)));
    return static_cast<uint32_t>(PANDA_BIT_UTILS_CTZ(x));
}

/**
 * @brief returns log2 for argument
 * @param X - of type uint64_t, should be power of 2
 * @return log2(X) or undefined if X 0
 */
constexpr uint64_t GetIntLog2(const uint64_t x)
{
    ASSERT((x > 0) && !(x & (x - 1U)));
    return static_cast<uint64_t>(PANDA_BIT_UTILS_CTZLL(x));
}

template <typename T>
inline constexpr T AbsOrMin(T v)
{
    static_assert(std::is_integral_v<T> && std::is_signed_v<T>);
    if (UNLIKELY(v == std::numeric_limits<T>::min())) {
        return v;
    }
    return std::abs(v);
}

/// return value is power of two
template <typename T>
inline constexpr bool IsPowerOfTwo(T value)
{
    static_assert(std::is_integral_v<T>);
    if constexpr (std::is_unsigned_v<T>) {
        return (value != 0U) && (value & (value - 1U)) == 0U;
    } else {
        using UT = std::make_unsigned_t<T>;
        auto absValue = bit_cast<UT>(AbsOrMin(value));
        return (absValue != 0U) && (absValue & (absValue - 1U)) == 0U;
    }
}

/// returns an integer power of two but not greater than value.
constexpr uint32_t GetPowerOfTwoValue32(uint32_t value)
{
    ASSERT(value < (1UL << 31UL));
    if (value > 0) {
        --value;
    }
    if (value == 0) {
        return 1;
    }
    constexpr uint32_t BIT = 32UL;
    return 1UL << (BIT - Clz(static_cast<uint32_t>(value)));
}

/// Count trailing zeroes
constexpr unsigned int Ctz(uint32_t value)
{
    constexpr std::array<uint32_t, 32> MULTIPLY_DE_BRUIJN_BIT_POSITION = {0,  1,  28, 2,  29, 14, 24, 3,  30, 22, 20,
                                                                          15, 25, 17, 4,  8,  31, 27, 13, 23, 21, 19,
                                                                          16, 7,  26, 12, 18, 6,  11, 5,  10, 9};
    constexpr size_t SHIFT = 27;
    constexpr size_t C = 0x077CB531;
    return MULTIPLY_DE_BRUIJN_BIT_POSITION[(static_cast<uint32_t>((value & static_cast<uint32_t>(-value)) * C)) >>
                                           SHIFT];
}

/// Count leading zeroes
constexpr uint32_t Clz(uint32_t value)
{
    constexpr std::array<uint32_t, 32> MULTIPLY_DE_BRUIJN_BIT_POSITION = {0,  9,  1,  10, 13, 21, 2,  29, 11, 14, 16,
                                                                          18, 22, 25, 3,  30, 8,  12, 20, 28, 15, 17,
                                                                          24, 7,  19, 27, 23, 6,  26, 5,  4,  31};
    constexpr size_t BIT32 = 32;
    constexpr size_t SHIFT = 27;
    value |= value >> 1U;
    value |= value >> 2U;
    value |= value >> 4U;
    value |= value >> 8U;   // NOLINT(readability-magic-numbers)
    value |= value >> 16U;  // NOLINT(readability-magic-numbers)
    return BIT32 - 1 - MULTIPLY_DE_BRUIJN_BIT_POSITION[static_cast<uint32_t>(value * 0x07C4ACDDU) >> SHIFT];  // NOLINT
}

template <typename T>
T Min(T a, T b)
{
    static_assert(std::is_floating_point_v<T>);
    if (std::isnan(a)) {
        return a;
    }
    if (a == 0.0 && b == 0.0 && std::signbit(b)) {
        return b;
    }
    return a <= b ? a : b;
}

template <typename T>
T Max(T a, T b)
{
    static_assert(std::is_floating_point_v<T>);
    if (std::isnan(a)) {
        return a;
    }
    if (a == 0.0 && b == 0.0 && std::signbit(a)) {
        return b;
    }
    return a >= b ? a : b;
}

/// Combine lhash and rhash
inline size_t MergeHashes(size_t lhash, size_t rhash)
{
    constexpr const size_t MAGIC_CONSTANT = 0x9e3779b9;
    size_t shl = lhash << 6U;
    size_t shr = lhash >> 2U;
    return lhash ^ (rhash + MAGIC_CONSTANT + shl + shr);
}

template <typename T>
inline T PowerOfTwoTableSlot(T key, T tableSize, uint32_t skippedLowestBits = 0)
{
    ASSERT(IsPowerOfTwo(tableSize));
    return (key >> skippedLowestBits) & (tableSize - 1);
}

}  // namespace ark::helpers::math

#endif  // PANDA_LIBBASE_UTILS_MATH_HELPERS_H_
