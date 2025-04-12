/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <limits>
#include "include/mem/panda_string.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/numberformatter.h"
#include "unicode/unistr.h"
#include "utils/utf.h"
#include "plugins/ets/runtime/intrinsics/helpers/dtoa_helper.h"
#include "plugins/ets/runtime/ets_exceptions.h"

namespace ark::ets::intrinsics {

namespace {

double ParseFloat(EtsString *s, const uint32_t flags)
{
    if (UNLIKELY(s->IsUtf16())) {
        size_t len = utf::Utf16ToUtf8Size(s->GetDataUtf16(), s->GetUtf16Length()) - 1;
        PandaVector<uint8_t> buf(len);
        len = utf::ConvertRegionUtf16ToUtf8(s->GetDataUtf16(), buf.data(), s->GetLength(), len, 0);

        Span<uint8_t> str = Span<uint8_t>(buf.data(), len);
        return helpers::StringToDouble(str.begin(), str.end(), 0, flags);
    }

    Span<uint8_t> str = Span<uint8_t>(s->GetDataMUtf8(), s->GetMUtf8Length() - 1);
    return helpers::StringToDouble(str.begin(), str.end(), 0, flags);
}

}  // namespace

EtsString *StdCoreDoubleToString(double number, int radix)
{
    if (UNLIKELY(radix != helpers::DECIMAL)) {
        return helpers::FpToString(number, radix);
    }
    auto *cache = PandaEtsVM::GetCurrent()->GetDoubleToStringCache();
    ASSERT(cache != nullptr);
    return cache->GetOrCache(EtsCoroutine::GetCurrent(), number);
}

bool IsNegativeNan(double x)
{
    return std::isnan(x) && std::signbit(x);
}

EtsString *StdCoreDoubleToLocaleString(ObjectHeader *obj, EtsString *locale)
{
    ASSERT(obj != nullptr && locale != nullptr);
    icu::Locale loc;
    UErrorCode status = U_ZERO_ERROR;
    PandaVector<uint8_t> buf;
    std::string_view locTag = locale->ConvertToStringView(&buf);
    icu::StringPiece sp {locTag.data(), static_cast<int32_t>(locTag.size())};
    loc = icu::Locale::forLanguageTag(sp, status);

    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Language tag '" + std::string(locTag) + "' is invalid or not supported";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }

    double objValue = helpers::GetStdDoubleArgument(obj);
    if (IsNegativeNan(objValue)) {
        objValue = std::numeric_limits<double>::quiet_NaN();
    }

    icu::number::LocalizedNumberFormatter locNumFmt = icu::number::NumberFormatter::withLocale(loc);
    icu::number::FormattedNumber fmtNum = locNumFmt.formatDouble(objValue, status);

    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Unable to convert " + std::to_string(objValue) + " to locale " + std::string(locTag);
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RUNTIME_EXCEPTION, message);
        return nullptr;
    }

    icu::UnicodeString uniStr = fmtNum.toString(status);
    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(uniStr.getBuffer()), uniStr.length());
}

double StdCoreDoubleParseFloat(EtsString *s)
{
    return ParseFloat(s, helpers::flags::IGNORE_TRAILING);
}

double StdCoreDoubleParseInt(EtsString *s, int32_t radix)
{
    bool isUtf16 = s->IsUtf16();
    size_t startIndex = 0;
    size_t length = s->GetLength();
    const int baseDec = 10;
    const int baseHex = 16;

    Span<uint16_t> utf16Span {};
    Span<uint8_t> mutf8Span {};
    if (isUtf16) {
        utf16Span = {s->GetDataUtf16(), s->GetUtf16Length()};
    } else {
        mutf8Span = {s->GetDataMUtf8(), s->GetMUtf8Length()};
    }

    if (length >= 1) {
        char firstChar = isUtf16 ? utf16Span[0] : mutf8Span[0];
        if (firstChar == '-') {
            startIndex = 1;
        }
    }

    if (radix == -1 || radix == 0) {
        radix = baseDec;
        if (length >= 2U + startIndex) {
            auto first = isUtf16 ? utf16Span[startIndex] : mutf8Span[startIndex];
            auto second = isUtf16 ? utf16Span[startIndex + 1] : mutf8Span[startIndex + 1];
            if (first == '0' && (second == 'x' || second == 'X')) {
                radix = baseHex;
            }
        }
    }

    if (isUtf16) {
        size_t utf16Length = s->GetUtf16Length();
        size_t utf8Size = utf::Utf16ToUtf8Size(s->GetDataUtf16(), utf16Length) - 1;
        PandaVector<uint8_t> buf(utf8Size);
        size_t convertedSize = utf::ConvertRegionUtf16ToUtf8(s->GetDataUtf16(), buf.data(), length, utf8Size, 0);
        Span<uint8_t> str = Span<uint8_t>(buf.data(), convertedSize);
        return std::trunc(helpers::StringToDoubleWithRadix(str.begin(), str.end(), radix));
    }
    Span<uint8_t> str = Span<uint8_t>(s->GetDataMUtf8(), s->GetMUtf8Length() - 1);
    return std::trunc(helpers::StringToDoubleWithRadix(str.begin(), str.end(), radix));
}

EtsString *StdCoreDoubleToExponential(ObjectHeader *obj, double d)
{
    double objValue = helpers::GetStdDoubleArgument(obj);
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    // truncate the arg val
    double digit = std::isnan(d) ? 0 : d;
    digit = (digit >= 0) ? std::floor(digit) : std::ceil(digit);
    // Check range
    if (UNLIKELY(digit > helpers::MAX_FRACTION || digit < helpers::MIN_FRACTION)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(),
                          panda_file_items::class_descriptors::ARGUMENT_OUT_OF_RANGE_EXCEPTION,
                          "toExponential argument must be between 0 and 100");
        return nullptr;
    }

    return helpers::DoubleToExponential(objValue, static_cast<int>(digit));
}

EtsString *StdCoreDoubleToExponentialWithNoDigit(ObjectHeader *obj)
{
    double objValue = helpers::GetStdDoubleArgument(obj);
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    if (objValue == 0.0) {
        return EtsString::CreateFromMUtf8("0e+0");
    }
    std::string res;
    if (objValue < 0) {
        res += "-";
        objValue = -objValue;
    }

    char tmpbuf[helpers::BUF_SIZE] = {0};
    helpers::DtoaHelper dtoa {tmpbuf};
    dtoa.Dtoa(objValue);
    int n = dtoa.GetPoint();
    int k = dtoa.GetDigits();

    std::string base = tmpbuf;
    base.erase(1, k - 1);
    if (k != 1) {
        base += std::string(".") + std::string(&tmpbuf[1]);
    }
    base += "e" + (n >= 1 ? std::string("+") : "") + std::to_string(n - 1);
    res += base;
    return EtsString::CreateFromMUtf8(res.c_str());
}

EtsString *StdCoreDoubleToPrecision(ObjectHeader *obj, double d)
{
    double objValue = helpers::GetStdDoubleArgument(obj);
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    // truncate the arg val
    double digitAbs = std::isnan(d) ? 0 : d;
    digitAbs = std::abs((digitAbs >= 0) ? std::floor(digitAbs) : std::ceil(digitAbs));
    // Check range
    if (UNLIKELY(digitAbs > helpers::MAX_FRACTION || digitAbs < helpers::MIN_FRACTION + 1)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(),
                          panda_file_items::class_descriptors::ARGUMENT_OUT_OF_RANGE_EXCEPTION,
                          "toPrecision argument must be between 1 and 100");
        return nullptr;
    }

    return helpers::DoubleToPrecision(objValue, static_cast<int>(digitAbs));
}

EtsString *StdCoreDoubleToFixed(ObjectHeader *obj, double d)
{
    // truncate the arg val
    double digitAbs = std::isnan(d) ? 0 : d;
    digitAbs = std::abs((digitAbs >= 0) ? std::floor(digitAbs) : std::ceil(digitAbs));
    // Check range
    if (UNLIKELY(digitAbs > helpers::MAX_FRACTION || digitAbs < helpers::MIN_FRACTION)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR,
                          "toFixed argument must be between 0 and 100");
        return nullptr;
    }

    double objValue = helpers::GetStdDoubleArgument(obj);
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    return helpers::DoubleToFixed(objValue, static_cast<int>(digitAbs));
}

extern "C" EtsBoolean StdCoreDoubleIsNan(double v)
{
    return ToEtsBoolean(v != v);
}

extern "C" EtsBoolean StdCoreDoubleIsFinite(double v)
{
    static const double POSITIVE_INFINITY = 1.0 / 0.0;
    static const double NEGATIVE_INFINITY = -1.0 / 0.0;

    return ToEtsBoolean(v == v && v != POSITIVE_INFINITY && v != NEGATIVE_INFINITY);
}

extern "C" EtsDouble StdCoreDoubleBitCastFromLong(EtsLong i)
{
    return bit_cast<EtsDouble>(i);
}

extern "C" EtsLong StdCoreDoubleBitCastToLong(EtsDouble f)
{
    return bit_cast<EtsLong>(f);
}

static inline bool IsInteger(double v)
{
    return std::isfinite(v) && (std::fabs(v - std::trunc(v)) <= std::numeric_limits<double>::epsilon());
}

extern "C" EtsBoolean StdCoreDoubleIsInteger(double v)
{
    return ToEtsBoolean(IsInteger(v));
}

/*
 * In ETS Double.isSafeInteger returns (Double.isInteger(v) && (abs(v) <= Double.MAX_SAFE_INTEGER)).
 * MAX_SAFE_INTEGER is a max integer value that can be used as a double without losing precision.
 */
extern "C" EtsBoolean StdCoreDoubleIsSafeInteger(double v)
{
    return ToEtsBoolean(IsInteger(v) && (std::fabs(v) <= helpers::MaxSafeInteger<double>()));
}

double StdCoreDoubleNumberFromString(EtsString *s)
{
    uint32_t flags = 0;
    flags |= helpers::flags::ALLOW_BINARY;
    flags |= helpers::flags::ALLOW_OCTAL;
    flags |= helpers::flags::ALLOW_HEX;
    flags |= helpers::flags::EMPTY_IS_ZERO;
    flags |= helpers::flags::ERROR_IN_EXPONENT_IS_NAN;
    return ParseFloat(s, flags);
}

}  // namespace ark::ets::intrinsics
