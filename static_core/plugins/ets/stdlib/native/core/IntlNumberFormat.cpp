/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/IntlState.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "libpandabase/macros.h"
#include "unicode/numberformatter.h"
#include "unicode/numberrangeformatter.h"
#include "unicode/locid.h"
#include "unicode/unistr.h"
#include "IntlCommon.h"

#include <string>
#include <cstdlib>
#include <array>

namespace ark::ets::stdlib::intl {

icu::Locale LocTagToIcuLocale(ani_env *env, const std::string &locTag)
{
    icu::StringPiece sp {locTag.data(), static_cast<int32_t>(locTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale loc = icu::Locale::forLanguageTag(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Language tag '" + locTag + std::string("' is invalid or not supported");
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return icu::Locale::getDefault();
    }
    return loc;
}

ani_string IcuFormatDouble(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale, ani_double value)
{
    const std::string &locTag = ConvertFromAniString(env, locale);
    const icu::Locale &loc = LocTagToIcuLocale(env, locTag);

    ani_boolean unhandledExc;
    ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&unhandledExc));
    if (unhandledExc == ANI_TRUE) {
        return nullptr;
    }

    const icu::number::LocalizedNumberFormatter &formatter =
        g_intlState->fmtsCache.NumFmtsCacheInvalidation(locTag, loc);

    UErrorCode status = U_ZERO_ERROR;
    const icu::number::FormattedNumber &fmtNumber = formatter.formatDouble(value, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "IcuFormatDouble. Unable to format double " + std::to_string(value);
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return nullptr;
    }
    icu::UnicodeString ustr = fmtNumber.toString(status);
    return UnicodeToAniStr(env, ustr);
}

ani_string IcuFormatDecStr(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale, ani_string value)
{
    const std::string &locTag = ConvertFromAniString(env, locale);
    const icu::Locale &loc = LocTagToIcuLocale(env, locTag);

    ani_boolean unhandledExc;
    ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&unhandledExc));
    if (unhandledExc == ANI_TRUE) {
        return nullptr;
    }

    const icu::number::LocalizedNumberFormatter &formatter =
        g_intlState->fmtsCache.NumFmtsCacheInvalidation(locTag, loc);

    const std::string &valueString = ConvertFromAniString(env, value);
    const icu::StringPiece sp {valueString.data(), static_cast<int32_t>(valueString.size())};

    UErrorCode status = U_ZERO_ERROR;
    const icu::number::FormattedNumber &fmtNumber = formatter.formatDecimal(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "IcuFormatDecimal. Unable to format BigInt " + valueString;
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return nullptr;
    }
    icu::UnicodeString ustr = fmtNumber.toString(status);
    return UnicodeToAniStr(env, ustr);
}

ani_string IcuFormatRange(ani_env *env, ani_string locale, icu::Formattable startFrmtbl, icu::Formattable endFrmtbl)
{
    const std::string &locTag = ConvertFromAniString(env, locale);
    const icu::Locale &loc = LocTagToIcuLocale(env, locTag);

    ani_boolean unhandledExc;
    ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&unhandledExc));
    if (unhandledExc == ANI_TRUE) {
        return nullptr;
    }

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
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return nullptr;
    }
    icu::UnicodeString ustr = fmtRangeNumber.toString(status);
    return UnicodeToAniStr(env, ustr);
}

icu::Formattable DoubleToFormattable([[maybe_unused]] ani_env *env, double value)
{
    return icu::Formattable(value);
}

icu::Formattable EtsStrToFormattable(ani_env *env, ani_string value)
{
    UErrorCode status = U_ZERO_ERROR;
    const std::string &str = ConvertFromAniString(env, value);
    const icu::StringPiece sp {str.data(), static_cast<int32_t>(str.size())};
    icu::Formattable ret(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "EtsStrToToFormattable. Unable to create icu::Formattable from value " + str;
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return icu::Formattable();
    }
    return ret;
}

ani_string IcuFormatRangeDoubleDouble(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale,
                                      double startValue, double endValue)
{
    return IcuFormatRange(env, locale, DoubleToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDoubleDecStr(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale,
                                      double startValue, ani_string endValue)
{
    return IcuFormatRange(env, locale, DoubleToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDouble(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale,
                                      ani_string startValue, double endValue)
{
    return IcuFormatRange(env, locale, EtsStrToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDecStr(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale,
                                      ani_string startValue, ani_string endValue)
{
    return IcuFormatRange(env, locale, EtsStrToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ani_status RegisterIntlNumberFormatNativeMethods(ani_env *env)
{
    const auto methods = std::array {
        ani_native_function {"formatDouble", "Lstd/core/String;D:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatDouble)},
        ani_native_function {"formatDecStr", "Lstd/core/String;Lstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatDecStr)},
        ani_native_function {"formatRangeDoubleDouble", "Lstd/core/String;DD:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDoubleDouble)},
        ani_native_function {"formatRangeDoubleDecStr", "Lstd/core/String;DLstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDoubleDecStr)},
        ani_native_function {"formatRangeDecStrDouble", "Lstd/core/String;Lstd/core/String;D:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDecStrDouble)},
        ani_native_function {"formatRangeDecStrDecStr",
                             "Lstd/core/String;Lstd/core/String;Lstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDecStrDecStr)}};

    ani_class numberFormatClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/NumberFormat;", &numberFormatClass));

    return env->Class_BindNativeMethods(numberFormatClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
