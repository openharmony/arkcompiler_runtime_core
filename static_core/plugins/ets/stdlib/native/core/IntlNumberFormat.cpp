/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/stdlib/native/core/IntlNumberFormat.h"
#include "plugins/ets/stdlib/native/core/IntlState.h"
#include "libpandabase/macros.h"
#include "libpandabase/os/mutex.h"
#include "unicode/numberformatter.h"
#include "unicode/numberrangeformatter.h"
#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/unistr.h"

#include <string>
#include <unordered_map>
#include <iterator>
#include <cstdlib>
#include <array>

namespace ark::ets::stdlib {

std::string EtsToStdStr(EtsEnv *env, ets_string etsStr)
{
    const char *charString = env->GetStringUTFChars(etsStr, nullptr);
    std::string stdStr {charString};
    env->ReleaseStringUTFChars(etsStr, charString);
    return stdStr;
}

icu::Locale LocTagToIcuLocale(EtsEnv *env, const std::string &locTag)
{
    icu::StringPiece sp {locTag.data(), static_cast<int32_t>(locTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale loc = icu::Locale::forLanguageTag(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Language tag '" + locTag + std::string("' is invalid or not supported");
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return icu::Locale::getDefault();
    }
    return loc;
}

ets_string IcuFormatDouble(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale, ets_double value)
{
    const std::string &locTag = EtsToStdStr(env, locale);
    const icu::Locale &loc = LocTagToIcuLocale(env, locTag);
    const icu::number::LocalizedNumberFormatter &formatter =
        g_intlState->fmtsCache.NumFmtsCacheInvalidation(locTag, loc);

    UErrorCode status = U_ZERO_ERROR;
    const icu::number::FormattedNumber &fmtNumber = formatter.formatDouble(value, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "IcuFormatDouble. Unable to format double " + std::to_string(value);
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return nullptr;
    }
    icu::UnicodeString ustr = fmtNumber.toString(status);
    return env->NewString(reinterpret_cast<const uint16_t *>(ustr.getBuffer()), ustr.length());
}

ets_string IcuFormatDecStr(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale, ets_string value)
{
    const std::string &locTag = EtsToStdStr(env, locale);
    const icu::Locale &loc = LocTagToIcuLocale(env, locTag);
    const icu::number::LocalizedNumberFormatter &formatter =
        g_intlState->fmtsCache.NumFmtsCacheInvalidation(locTag, loc);

    const std::string &valueString = EtsToStdStr(env, value);
    const icu::StringPiece sp {valueString.data(), static_cast<int32_t>(valueString.size())};

    UErrorCode status = U_ZERO_ERROR;
    const icu::number::FormattedNumber &fmtNumber = formatter.formatDecimal(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "IcuFormatDecimal. Unable to format BigInt " + valueString;
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return nullptr;
    }
    icu::UnicodeString ustr = fmtNumber.toString(status);
    return env->NewString(reinterpret_cast<const uint16_t *>(ustr.getBuffer()), ustr.length());
}

ets_string IcuFormatRange(EtsEnv *env, ets_string locale, icu::Formattable startFrmtbl, icu::Formattable endFrmtbl)
{
    const std::string &locTag = EtsToStdStr(env, locale);
    const icu::Locale &loc = LocTagToIcuLocale(env, locTag);
    const icu::number::LocalizedNumberRangeFormatter &formatter =
        g_intlState->fmtsCache.NumRangeFmtsCacheInvalidation(locTag, loc);

    UErrorCode status = U_ZERO_ERROR;
    const icu::number::FormattedNumberRange &fmtRangeNumber =
        formatter.formatFormattableRange(startFrmtbl, endFrmtbl, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "IcuFormatRangeFormattable. Unable to rangeformat ";
        UErrorCode startStatus = U_ZERO_ERROR;
        std::string startStr {startFrmtbl.getDecimalNumber(startStatus).data()};
        UErrorCode endStatus = U_ZERO_ERROR;
        std::string endStr {endFrmtbl.getDecimalNumber(endStatus).data()};
        if (UNLIKELY(U_FAILURE(startStatus))) {
            message += ". Crashed conversion startFrmtbl into std::string";
        } else if (UNLIKELY(U_FAILURE(endStatus))) {
            message += ". Crashed conversion endFrmtbl into std::string";
        } else {
            message += std::string(" start ") + startStr + std::string(" end ") + endStr;
        }
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return nullptr;
    }
    icu::UnicodeString ustr = fmtRangeNumber.toString(status);
    return env->NewString(reinterpret_cast<const uint16_t *>(ustr.getBuffer()), ustr.length());
}

icu::Formattable DoubleToFormattable([[maybe_unused]] EtsEnv *env, double value)
{
    return icu::Formattable(value);
}

icu::Formattable EtsStrToFormattable(EtsEnv *env, ets_string value)
{
    UErrorCode status = U_ZERO_ERROR;
    const std::string &str = EtsToStdStr(env, value);
    const icu::StringPiece sp {str.data(), static_cast<int32_t>(str.size())};
    icu::Formattable ret(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "EtsStrToToFormattable. Unable to create icu::Formattable from value " + str;
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return icu::Formattable();
    }
    return ret;
}

ets_string IcuFormatRangeDoubleDouble(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale,
                                      double startValue, double endValue)
{
    return IcuFormatRange(env, locale, DoubleToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ets_string IcuFormatRangeDoubleDecStr(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale,
                                      double startValue, ets_string endValue)
{
    return IcuFormatRange(env, locale, DoubleToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ets_string IcuFormatRangeDecStrDouble(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale,
                                      ets_string startValue, double endValue)
{
    return IcuFormatRange(env, locale, EtsStrToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ets_string IcuFormatRangeDecStrDecStr(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale,
                                      ets_string startValue, ets_string endValue)
{
    return IcuFormatRange(env, locale, EtsStrToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ets_int RegisterIntlNumberFormatNativeMethods(EtsEnv *env)
{
    const auto methods =
        std::array {EtsNativeMethod {"formatDouble", nullptr, reinterpret_cast<ets_string *>(IcuFormatDouble)},
                    EtsNativeMethod {"formatDecStr", nullptr, reinterpret_cast<ets_string *>(IcuFormatDecStr)},
                    EtsNativeMethod {"formatRangeDoubleDouble", nullptr,
                                     reinterpret_cast<ets_string *>(IcuFormatRangeDoubleDouble)},
                    EtsNativeMethod {"formatRangeDoubleDecStr", nullptr,
                                     reinterpret_cast<ets_string *>(IcuFormatRangeDoubleDecStr)},
                    EtsNativeMethod {"formatRangeDecStrDouble", nullptr,
                                     reinterpret_cast<ets_string *>(IcuFormatRangeDecStrDouble)},
                    EtsNativeMethod {"formatRangeDecStrDecStr", nullptr,
                                     reinterpret_cast<ets_string *>(IcuFormatRangeDecStrDecStr)}};

    ets_class numberFormatClass = env->FindClass("std/core/NumberFormat");
    return env->RegisterNatives(numberFormatClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib
