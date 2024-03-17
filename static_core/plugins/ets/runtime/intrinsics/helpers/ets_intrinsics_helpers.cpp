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

#include <cstdlib>
#include "ets_intrinsics_helpers.h"
#include "include/mem/panda_string.h"
#include "types/ets_field.h"
#include "types/ets_string.h"

namespace ark::ets::intrinsics::helpers {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RETURN_IF_CONVERSION_END(p, end, result) \
    if ((p) == (end)) {                          \
        return (result);                         \
    }

namespace parse_helpers {

template <typename ResultType>
struct ParseResult {
    ResultType value;
    uint8_t *pointerPosition = nullptr;
    bool isSuccess = false;
};

ParseResult<int32_t> ParseExponent(const uint8_t *start, const uint8_t *end, const uint8_t radix, const uint32_t flags)
{
    constexpr int32_t MAX_EXPONENT = INT32_MAX / 2;
    auto p = const_cast<uint8_t *>(start);
    if (radix == 0) {
        return {0, p, false};
    }

    char exponentSign = '+';
    int32_t additionalExponent = 0;
    bool undefinedExponent = false;
    ++p;
    if (p == end) {
        undefinedExponent = true;
    }

    if (!undefinedExponent && (*p == '+' || *p == '-')) {
        exponentSign = static_cast<char>(*p);
        ++p;
        if (p == end) {
            undefinedExponent = true;
        }
    }
    if (!undefinedExponent) {
        uint8_t digit;
        while ((digit = ToDigit(*p)) < radix) {
            if (additionalExponent > MAX_EXPONENT / radix) {
                additionalExponent = MAX_EXPONENT;
            } else {
                additionalExponent = additionalExponent * static_cast<int32_t>(radix) + static_cast<int32_t>(digit);
            }
            if (++p == end) {
                break;
            }
        }
    } else if ((flags & flags::ERROR_IN_EXPONENT_IS_NAN) != 0) {
        return {0, p, false};
    }
    if (exponentSign == '-') {
        return {-additionalExponent, p, true};
    }
    return {additionalExponent, p, true};
}

}  // namespace parse_helpers

double StringToDouble(const uint8_t *start, const uint8_t *end, uint8_t radix, uint32_t flags)
{
    // 1. skip space and line terminal
    if (IsEmptyString(start, end)) {
        if ((flags & flags::EMPTY_IS_ZERO) != 0) {
            return 0.0;
        }
        return NAN_VALUE;
    }

    radix = 0;
    auto p = const_cast<uint8_t *>(start);

    GotoNonspace(&p, end);

    // 2. get number sign
    Sign sign = Sign::NONE;
    if (*p == '+') {
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
        sign = Sign::POS;
    } else if (*p == '-') {
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
        sign = Sign::NEG;
    }
    bool ignoreTrailing = (flags & flags::IGNORE_TRAILING) != 0;

    // 3. judge Infinity
    static const char INF[] = "Infinity";  // NOLINT(modernize-avoid-c-arrays)
    if (*p == INF[0]) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (const char *i = &INF[1]; *i != '\0'; ++i) {
            if (++p == end || *p != *i) {
                return NAN_VALUE;
            }
        }
        ++p;
        if (!ignoreTrailing && GotoNonspace(&p, end)) {
            return NAN_VALUE;
        }
        return sign == Sign::NEG ? -POSITIVE_INFINITY : POSITIVE_INFINITY;
    }

    // 4. get number radix
    bool leadingZero = false;
    bool prefixRadix = false;
    if (*p == '0' && radix == 0) {
        RETURN_IF_CONVERSION_END(++p, end, SignedZero(sign));
        if (*p == 'x' || *p == 'X') {
            if ((flags & flags::ALLOW_HEX) == 0) {
                return ignoreTrailing ? SignedZero(sign) : NAN_VALUE;
            }
            RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
            if (sign != Sign::NONE) {
                return NAN_VALUE;
            }
            prefixRadix = true;
            radix = HEXADECIMAL;
        } else if (*p == 'o' || *p == 'O') {
            if ((flags & flags::ALLOW_OCTAL) == 0) {
                return ignoreTrailing ? SignedZero(sign) : NAN_VALUE;
            }
            RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
            if (sign != Sign::NONE) {
                return NAN_VALUE;
            }
            prefixRadix = true;
            radix = OCTAL;
        } else if (*p == 'b' || *p == 'B') {
            if ((flags & flags::ALLOW_BINARY) == 0) {
                return ignoreTrailing ? SignedZero(sign) : NAN_VALUE;
            }
            RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
            if (sign != Sign::NONE) {
                return NAN_VALUE;
            }
            prefixRadix = true;
            radix = BINARY;
        } else {
            leadingZero = true;
        }
    }

    if (radix == 0) {
        radix = DECIMAL;
    }
    auto pStart = p;
    // 5. skip leading '0'
    while (*p == '0') {
        RETURN_IF_CONVERSION_END(++p, end, SignedZero(sign));
        leadingZero = true;
    }
    // 6. parse to number
    uint64_t intNumber = 0;
    uint64_t numberMax = (UINT64_MAX - (radix - 1)) / radix;
    int digits = 0;
    int exponent = 0;
    do {
        uint8_t c = ToDigit(*p);
        if (c >= radix) {
            if (!prefixRadix || ignoreTrailing || (pStart != p && !GotoNonspace(&p, end))) {
                break;
            }
            // "0b" "0x1.2" "0b1e2" ...
            return NAN_VALUE;
        }
        ++digits;
        if (intNumber < numberMax) {
            intNumber = intNumber * radix + c;
        } else {
            ++exponent;
        }
    } while (++p != end);

    auto number = static_cast<double>(intNumber);
    if (sign == Sign::NEG) {
        if (number == 0) {
            number = -0.0;
        } else {
            number = -number;
        }
    }

    // 7. deal with other radix except DECIMAL
    if (p == end || radix != DECIMAL) {
        if ((digits == 0 && !leadingZero) || (p != end && !ignoreTrailing && GotoNonspace(&p, end))) {
            // no digits there, like "0x", "0xh", or error trailing of "0x3q"
            return NAN_VALUE;
        }
        return number * std::pow(radix, exponent);
    }

    // 8. parse '.'
    if (*p == '.') {
        RETURN_IF_CONVERSION_END(++p, end, (digits > 0) ? (number * std::pow(radix, exponent)) : NAN_VALUE);
        while (ToDigit(*p) < radix) {
            --exponent;
            ++digits;
            if (++p == end) {
                break;
            }
        }
    }
    if (digits == 0 && !leadingZero) {
        // no digits there, like ".", "sss", or ".e1"
        return NAN_VALUE;
    }
    auto pEnd = p;

    // 9. parse 'e/E' with '+/-'
    if (radix == DECIMAL && (p != end && (*p == 'e' || *p == 'E'))) {
        auto parseExponentResult = parse_helpers::ParseExponent(p, end, radix, flags);
        if (!parseExponentResult.isSuccess) {
            return NAN_VALUE;
        }
        p = parseExponentResult.pointerPosition;
        exponent += parseExponentResult.value;
    }
    if (!ignoreTrailing && GotoNonspace(&p, end)) {
        return NAN_VALUE;
    }

    // 10. build StringNumericLiteral string
    PandaString buffer;
    if (sign == Sign::NEG) {
        buffer += "-";
    }
    for (uint8_t *i = pStart; i < pEnd; ++i) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*i != static_cast<uint8_t>('.')) {
            buffer += *i;
        }
    }

    // 11. convert none-prefix radix string
    return Strtod(buffer.c_str(), exponent, radix);
}

double StringToDoubleWithRadix(const uint8_t *start, const uint8_t *end, int radix)
{
    auto p = const_cast<uint8_t *>(start);
    // 1. skip space and line terminal
    if (!GotoNonspace(&p, end)) {
        return NAN_VALUE;
    }

    // 2. sign bit
    bool negative = false;
    if (*p == '-') {
        negative = true;
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
    } else if (*p == '+') {
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
    }
    // 3. 0x or 0X
    bool stripPrefix = true;
    // 4. If R != 0, then
    //     a. If R < 2 or R > 36, return NaN.
    //     b. If R != 16, let stripPrefix be false.
    if (radix != 0) {
        if (radix < MIN_RADIX || radix > MAX_RADIX) {
            return NAN_VALUE;
        }
        if (radix != HEXADECIMAL) {
            stripPrefix = false;
        }
    } else {
        radix = DECIMAL;
    }
    int size = 0;
    if (stripPrefix) {
        if (*p == '0') {
            size++;
            if (++p != end && (*p == 'x' || *p == 'X')) {
                RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
                radix = HEXADECIMAL;
            }
        }
    }

    double result = 0;
    bool isDone = false;
    do {
        double part = 0;
        uint32_t multiplier = 1;
        for (; p != end; ++p) {
            // The maximum value to ensure that uint32_t will not overflow
            const uint32_t maxMultiper = 0xffffffffU / 36U;
            uint32_t m = multiplier * static_cast<uint32_t>(radix);
            if (m > maxMultiper) {
                break;
            }

            int currentBit = static_cast<int>(ToDigit(*p));
            if (currentBit >= radix) {
                isDone = true;
                break;
            }
            size++;
            part = part * radix + currentBit;
            multiplier = m;
        }
        result = result * multiplier + part;
        if (isDone) {
            break;
        }
    } while (p != end);

    if (size == 0) {
        return NAN_VALUE;
    }

    return negative ? -result : result;
}

EtsString *DoubleToExponential(double number, int digit)
{
    PandaStringStream ss;
    if (digit < 0) {
        ss << std::setiosflags(std::ios::scientific) << std::setprecision(MAX_PRECISION) << number;
    } else {
        ss << std::setiosflags(std::ios::scientific) << std::setprecision(digit) << number;
    }
    PandaString result = ss.str();
    size_t found = result.find_last_of('e');
    if (found != PandaString::npos && found < result.size() - 2U && result[found + 2U] == '0') {
        result.erase(found + 2U, 1);  // 2:offset of e
    }
    if (digit < 0) {
        size_t end = found;
        while (--found > 0) {
            if (result[found] != '0') {
                break;
            }
        }
        if (result[found] == '.') {
            found--;
        }
        if (found < end - 1) {
            result.erase(found + 1, end - found - 1);
        }
    }
    return EtsString::CreateFromMUtf8(result.c_str());
}

EtsString *DoubleToFixed(double number, int digit)
{
    PandaStringStream ss;
    ss << std::setiosflags(std::ios::fixed) << std::setprecision(digit) << number;
    return EtsString::CreateFromMUtf8(ss.str().c_str());
}

EtsString *DoubleToPrecision(double number, int digit)
{
    if (number == 0.0) {
        return DoubleToFixed(number, digit - 1);
    }
    PandaStringStream ss;
    double positiveNumber = number > 0 ? number : -number;
    int logDigit = std::floor(log10(positiveNumber));
    int radixDigit = digit - logDigit - 1;
    const int maxExponentDigit = 6;
    if ((logDigit >= 0 && radixDigit >= 0) || (logDigit < 0 && radixDigit <= maxExponentDigit)) {
        return DoubleToFixed(number, std::abs(radixDigit));
    }
    return DoubleToExponential(number, digit - 1);
}

double GetStdDoubleArgument(ObjectHeader *obj)
{
    auto *cls = obj->ClassAddr<Class>();

    // Assume std.core.Double has only one `double` field
    ASSERT(cls->GetInstanceFields().size() == 1);

    Field &fieldVal = cls->GetInstanceFields()[0];

    ASSERT(fieldVal.GetTypeId() == panda_file::Type::TypeId::F64);

    size_t offset = fieldVal.GetOffset();
    return obj->GetFieldPrimitive<double>(offset);
}

}  // namespace ark::ets::intrinsics::helpers
