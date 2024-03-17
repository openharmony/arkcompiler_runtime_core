/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_HELPERS_
#define PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_HELPERS_

#include <cmath>
#include <cstdint>
#include <type_traits>
#include "libpandabase/utils/bit_helpers.h"
#include "libpandabase/utils/utils.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

namespace ark::ets::intrinsics::helpers {

inline constexpr double MIN_RADIX = 2;
inline constexpr double MAX_RADIX = 36;
inline constexpr double MIN_FRACTION = 0;
inline constexpr double MAX_FRACTION = 100;

inline constexpr uint32_t MAX_PRECISION = 16;
inline constexpr uint8_t BINARY = 2;
inline constexpr uint8_t OCTAL = 8;
inline constexpr uint8_t DECIMAL = 10;
inline constexpr uint8_t HEXADECIMAL = 16;
inline constexpr double HALF = 0.5;
inline constexpr double EPSILON = std::numeric_limits<double>::epsilon();
inline constexpr double MAX_SAFE_INTEGER = 9007199254740991;
inline constexpr double MAX_VALUE = std::numeric_limits<double>::max();
inline constexpr double MIN_VALUE = std::numeric_limits<double>::min();
inline constexpr double POSITIVE_INFINITY = coretypes::TaggedValue::VALUE_INFINITY;
inline constexpr double NAN_VALUE = coretypes::TaggedValue::VALUE_NAN;

inline constexpr int DOUBLE_MAX_PRECISION = 17;
inline constexpr int FLOAT_MAX_PRECISION = 9;

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
inline constexpr uint16_t SPACE_OR_LINE_TERMINAL[] = {
    0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x0020, 0x00A0, 0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004,
    0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028, 0x2029, 0x202F, 0x205F, 0x3000, 0xFEFF,
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
inline constexpr char CHARS[] = "0123456789abcdefghijklmnopqrstuvwxyz";
inline constexpr uint64_t MAX_MANTISSA = 0x1ULL << 52U;
inline constexpr size_t BUF_SIZE = 128;
inline constexpr size_t TEN = 10;

enum class Sign { NONE, NEG, POS };

union DoubleValUnion {
    double fval;
    uint64_t uval;
    struct {
        uint64_t significand : coretypes::DOUBLE_SIGNIFICAND_SIZE;
        uint16_t exponent : coretypes::DOUBLE_EXPONENT_SIZE;
        int sign : 1;
    } bits __attribute__((packed));
} __attribute__((may_alias, packed));

template <typename T>
constexpr T MaxSafeInteger()
{
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>);
    if (std::is_same_v<T, float>) {
        return static_cast<float>((UINT32_C(1) << static_cast<uint32_t>(std::numeric_limits<float>::digits)) - 1);
    }
    return static_cast<double>((UINT64_C(1) << static_cast<uint32_t>(std::numeric_limits<double>::digits)) - 1);
}

inline double SignedZero(Sign sign)
{
    return sign == Sign::NEG ? -0.0 : 0.0;
}

inline uint8_t ToDigit(uint8_t c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + DECIMAL;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + DECIMAL;
    }
    return '$';
}

inline double PowHelper(uint64_t number, int16_t exponent, uint8_t radix)
{
    const double log2Radix {std::log2(radix)};

    double expRem = log2Radix * exponent;
    int expI = static_cast<int>(expRem);
    expRem = expRem - expI;

    // NOLINTNEXTLINE(readability-magic-numbers)
    DoubleValUnion u = {static_cast<double>(number) * std::pow(2.0, expRem)};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    expI = u.bits.exponent + expI;
    if (((expI & ~coretypes::DOUBLE_EXPONENT_MASK) != 0) || expI == 0) {  // NOLINT(hicpp-signed-bitwise)
        if (expI > 0) {
            return std::numeric_limits<double>::infinity();
        }
        if (expI < -static_cast<int>(coretypes::DOUBLE_SIGNIFICAND_SIZE)) {
            return -std::numeric_limits<double>::infinity();
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        u.bits.exponent = 0;
        // NOLINTNEXTLINE(hicpp-signed-bitwise, cppcoreguidelines-pro-type-union-access)
        u.bits.significand = (u.bits.significand | coretypes::DOUBLE_HIDDEN_BIT) >> (1 - expI);
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        u.bits.exponent = expI;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return u.fval;
}

inline bool IsNonspace(uint16_t c)
{
    int i;
    int len = sizeof(SPACE_OR_LINE_TERMINAL) / sizeof(SPACE_OR_LINE_TERMINAL[0]);
    for (i = 0; i < len; i++) {
        if (c == SPACE_OR_LINE_TERMINAL[i]) {
            return false;
        }
        if (c < SPACE_OR_LINE_TERMINAL[i]) {
            return true;
        }
    }
    return true;
}

inline bool GotoNonspace(uint8_t **ptr, const uint8_t *end)
{
    while (*ptr < end) {
        uint16_t c = **ptr;
        size_t size = 1;
        if (c > INT8_MAX) {
            size = 0;
            uint16_t utf8Bit = INT8_MAX + 1;  // equal 0b1000'0000
            while (utf8Bit > 0 && (c & utf8Bit) == utf8Bit) {
                ++size;
                utf8Bit >>= 1UL;
            }
            if (utf::ConvertRegionUtf8ToUtf16(*ptr, &c, end - *ptr, 1, 0) <= 0) {
                return true;
            }
        }
        if (IsNonspace(c)) {
            return true;
        }
        *ptr += size;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return false;
}

inline bool IsEmptyString(const uint8_t *start, const uint8_t *end)
{
    auto p = const_cast<uint8_t *>(start);
    return !GotoNonspace(&p, end);
}

inline double Strtod(const char *str, int exponent, uint8_t radix)
{
    ASSERT(str != nullptr);
    ASSERT(radix >= MIN_RADIX && radix <= MAX_RADIX);
    auto p = const_cast<char *>(str);
    Sign sign = Sign::NONE;
    uint64_t number = 0;
    uint64_t numberMax = (UINT64_MAX - (radix - 1)) / radix;
    double result = 0.0;
    if (*p == '-') {
        sign = Sign::NEG;
        ++p;
    }
    while (*p == '0') {
        ++p;
    }
    while (*p != '\0') {
        uint8_t digit = ToDigit(static_cast<uint8_t>(*p));
        if (digit >= radix) {
            break;
        }
        if (number < numberMax) {
            number = number * radix + digit;
        } else {
            ++exponent;
        }
        ++p;
    }
    if (exponent < 0) {
        result = number / std::pow(radix, -exponent);
    } else {
        result = number * std::pow(radix, exponent);
    }
    if (!std::isfinite(result)) {
        result = PowHelper(number, exponent, radix);
    }
    return sign == Sign::NEG ? -result : result;
}

[[maybe_unused]] inline char Carry(char current, int radix)
{
    int digit = static_cast<int>((current > '9') ? (current - 'a' + DECIMAL) : (current - '0'));
    digit = (digit == (radix - 1)) ? 0 : digit + 1;
    return CHARS[digit];
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
FpType TruncateFp(FpType number)
{
    // -0 to +0
    if (number == 0.0) {
        return 0;
    }
    return (number >= 0) ? std::floor(number) : std::ceil(number);
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
PandaString DecimalsToString(FpType *numberInteger, FpType fraction, int radix, FpType delta)
{
    PandaString result;
    while (fraction >= delta) {
        fraction *= radix;
        delta *= radix;
        int64_t integer = std::floor(fraction);
        fraction -= integer;
        result += CHARS[integer];
        if (fraction > HALF && fraction + delta > 1) {
            size_t fractionEnd = result.size() - 1;
            result[fractionEnd] = Carry(*result.rbegin(), radix);
            for (; fractionEnd > 0; fractionEnd--) {
                if (result[fractionEnd] == '0') {
                    result[fractionEnd - 1] = Carry(result[fractionEnd - 1], radix);
                } else {
                    break;
                }
            }
            if (fractionEnd == 0) {
                (*numberInteger)++;
            }
            break;
        }
    }
    // delete 0 in the end
    size_t found = result.find_last_not_of('0');
    if (found != PandaString::npos) {
        result.erase(found + 1);
    }

    return result;
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
PandaString IntegerToString(FpType number, int radix)
{
    ASSERT(radix >= MIN_RADIX && radix <= MAX_RADIX);
    ASSERT(number == std::round(number));
    PandaString result;
    while (TruncateFp(number / radix) > static_cast<FpType>(MAX_MANTISSA)) {
        number /= radix;
        TruncateFp(number);
        result = PandaString("0").append(result);
    }
    do {
        auto remainder = static_cast<uint8_t>(std::fmod(number, radix));
        result = CHARS[remainder] + result;
        number = TruncateFp((number - remainder) / radix);
    } while (number > 0);
    return result;
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
FpType StrToFp(char *str, char **strEnd)
{
    if constexpr (std::is_same_v<FpType, double>) {
        return std::strtod(str, strEnd);
    } else {
        return std::strtof(str, strEnd);
    }
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
void GetBase(FpType d, int digits, int *decpt, char *buf, char *bufTmp, int size)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    int result = snprintf_s(bufTmp, size, size - 1, "%+.*e", digits - 1, d);
    if (result == -1) {
        LOG(FATAL, ETS) << "snprintf_s failed";
        UNREACHABLE();
    }
    // mantissa
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    buf[0] = bufTmp[1];
    if (digits > 1) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (memcpy_s(buf + 1U, digits, bufTmp + 2U, digits) != EOK) {  // 2 means add the point char to buf
            LOG(FATAL, ETS) << "snprintf_s failed";
            UNREACHABLE();
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    buf[digits + 1] = '\0';
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    char *endBuf = bufTmp + size;
    const size_t positive = (digits > 1) ? 1 : 0;
    // exponent
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *decpt = std::strtol(bufTmp + digits + 2U + positive, &endBuf, TEN) + 1;  // 2 means ignore the integer and point
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
int GetMinimumDigits(FpType d, int *decpt, char *buf)
{
    int digits = 0;

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char bufTmp[BUF_SIZE] = {0};

    // find the minimum amount of digits
    int minDigits = 1;
    int maxDigits = std::is_same_v<FpType, double> ? DOUBLE_MAX_PRECISION : FLOAT_MAX_PRECISION;

    while (minDigits < maxDigits) {
        digits = (minDigits + maxDigits) / 2_I;
        GetBase(d, digits, decpt, buf, bufTmp, sizeof(bufTmp));

        bool same = StrToFp<FpType>(bufTmp, nullptr) == d;
        if (same) {
            // no need to keep the trailing zeros
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            while (digits >= 2_I && buf[digits] == '0') {  // 2 means ignore the integer and point
                digits--;
            }
            maxDigits = digits;
        } else {
            minDigits = digits + 1;
        }
    }
    digits = maxDigits;
    GetBase(d, digits, decpt, buf, bufTmp, sizeof(bufTmp));

    return digits;
}

double StringToDouble(const uint8_t *start, const uint8_t *end, uint8_t radix, uint32_t flags);
double StringToDoubleWithRadix(const uint8_t *start, const uint8_t *end, int radix);
EtsString *DoubleToExponential(double number, int digit);
EtsString *DoubleToFixed(double number, int digit);
EtsString *DoubleToPrecision(double number, int digit);
double GetStdDoubleArgument(ObjectHeader *obj);

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
inline const char *FpNonFiniteToString(FpType number)
{
    ASSERT(std::isnan(number) || !std::isfinite(number));
    if (std::isnan(number)) {
        return "NaN";
    }
    return std::signbit(number) ? "-Infinity" : "Infinity";
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
void FpToStringDecimalRadix(FpType number, PandaString &result)
{
    ASSERT(result.empty());
    using SignedInt = typename ark::helpers::TypeHelperT<sizeof(FpType) * CHAR_BIT, true>;
    static constexpr FpType MIN_BOUND = 0.1;

    // isfinite checks if the number is normal, subnormal or zero, but not infinite or NaN.
    if (!std::isfinite(number)) {
        result = FpNonFiniteToString(number);
        return;
    }

    if (number < 0) {
        result += "-";
        number = -number;
    }
    if (MIN_BOUND <= number && number < 1) {
        // Fast path. In this case, n==0, just need to calculate k and s.
        result += "0.";
        SignedInt power = TEN;
        SignedInt s = 0;
        for (int k = 1; k <= intrinsics::helpers::FLOAT_MAX_PRECISION; ++k) {
            int digit = static_cast<SignedInt>(number * power) % TEN;
            s = s * TEN + digit;
            result.append(ToPandaString(digit));
            if (s / static_cast<FpType>(power) == number) {  // s * (10 ** -k)
                break;
            }
            power *= TEN;
        }
        return;
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char buffer[BUF_SIZE] = {0};
    int n = 0;
    int k = intrinsics::helpers::GetMinimumDigits(number, &n, buffer);
    PandaString base(buffer);

    if (0 < n && n <= 21_I) {  // NOLINT(readability-magic-numbers)
        base.erase(1, 1);
        if (k <= n) {
            // 6. If k ≤ n ≤ 21, return the String consisting of the code units of the k digits of the decimal
            // representation of s (in order, with no leading zeroes), followed by n−k occurrences of the code unit
            // 0x0030 (DIGIT ZERO).
            base += PandaString(n - k, '0');
        } else {
            // 7. If 0 < n ≤ 21, return the String consisting of the code units of the most significant n digits of the
            // decimal representation of s, followed by the code unit 0x002E (FULL STOP), followed by the code units of
            // the remaining k−n digits of the decimal representation of s.
            base.insert(n, 1, '.');
        }
    } else if (-6_I < n && n <= 0) {  // NOLINT(readability-magic-numbers)
        // 8. If −6 < n ≤ 0, return the String consisting of the code unit 0x0030 (DIGIT ZERO), followed by the code
        // unit 0x002E (FULL STOP), followed by −n occurrences of the code unit 0x0030 (DIGIT ZERO), followed by the
        // code units of the k digits of the decimal representation of s.
        base.erase(1, 1);
        base = PandaString("0.") + PandaString(-n, '0') + base;
    } else {
        if (k == 1) {
            // 9. Otherwise, if k = 1, return the String consisting of the code unit of the single digit of s
            base.erase(1, 1);
        }
        // followed by code unit 0x0065 (LATIN SMALL LETTER E), followed by the code unit 0x002B (PLUS SIGN) or the code
        // unit 0x002D (HYPHEN-MINUS) according to whether n−1 is positive or negative, followed by the code units of
        // the decimal representation of the integer abs(n−1) (with no leading zeroes).
        base += (n >= 1 ? PandaString("e+") : "e") + ToPandaString(n - 1);
    }
    result += base;
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
inline float FpDelta(FpType number)
{
    using UnsignedIntType = ark::helpers::TypeHelperT<sizeof(FpType) * CHAR_BIT, false>;
    auto value = bit_cast<FpType>(bit_cast<UnsignedIntType>(number) + 1U);
    float delta = static_cast<FpType>(HALF) * (bit_cast<FpType>(value) - number);
    return delta == 0 ? number : delta;
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
EtsString *FpToString(FpType number, int radix)
{
    // check radix range
    if (UNLIKELY(radix > helpers::MAX_RADIX || radix < helpers::MIN_RADIX)) {
        constexpr size_t MAX_BUF_SIZE = 128;
        std::array<char, MAX_BUF_SIZE> buf {};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        snprintf_s(buf.data(), buf.size(), buf.size() - 1, "radix must be %.f to %.f", helpers::MIN_RADIX,
                   helpers::MAX_RADIX);
        ThrowEtsException(EtsCoroutine::GetCurrent(),
                          panda_file_items::class_descriptors::ARGUMENT_OUT_OF_RANGE_EXCEPTION, buf.data());
        return nullptr;
    }

    if (radix == helpers::DECIMAL) {
        PandaString result;
        helpers::FpToStringDecimalRadix(number, result);
        return EtsString::CreateFromMUtf8(result.data());
    }

    // isfinite checks if the number is normal, subnormal or zero, but not infinite or NaN.
    if (!std::isfinite(number)) {
        return EtsString::CreateFromMUtf8(FpNonFiniteToString(number));
    }

    PandaString result;
    if (number < 0.0) {
        result += "-";
        number = -number;
    }

    float delta = FpDelta(number);
    FpType integral;
    FpType fractional = std::modf(number, &integral);
    if (fractional != 0 && fractional >= delta) {
        PandaString fraction(DecimalsToString<FpType>(&integral, fractional, radix, delta));
        result += IntegerToString(integral, radix) + "." + fraction;
    } else {
        result += IntegerToString(integral, radix);
    }

    return EtsString::CreateFromMUtf8(result.c_str());
}

}  // namespace ark::ets::intrinsics::helpers

namespace ark::ets::intrinsics::helpers::flags {

inline constexpr uint32_t NO_FLAGS = 0U;
inline constexpr uint32_t ALLOW_BINARY = 1U << 0U;
inline constexpr uint32_t ALLOW_OCTAL = 1U << 1U;
inline constexpr uint32_t ALLOW_HEX = 1U << 2U;
inline constexpr uint32_t IGNORE_TRAILING = 1U << 3U;
inline constexpr uint32_t EMPTY_IS_ZERO = 1U << 4U;
inline constexpr uint32_t ERROR_IN_EXPONENT_IS_NAN = 1U << 5U;

}  // namespace ark::ets::intrinsics::helpers::flags

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_HELPERS_
