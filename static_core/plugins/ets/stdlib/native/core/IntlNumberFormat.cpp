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

constexpr std::string_view COMPACT_DISPLAY_SHORT = "short";
constexpr std::string_view COMPACT_DISPLAY_LONG = "long";
constexpr std::string_view CURRENCY_SIGN_ACCOUNTING = "accounting";
constexpr std::string_view CURRENCY_DISPLAY_CODE = "code";
constexpr std::string_view CURRENCY_DISPLAY_SYMBOL = "symbol";
constexpr std::string_view CURRENCY_DISPLAY_NARROWSYMBOL = "narrowSymbol";
constexpr std::string_view CURRENCY_DISPLAY_NAME = "name";
constexpr std::string_view NOTATION_STANDARD = "standard";
constexpr std::string_view NOTATION_SCIENTIFIC = "scientific";
constexpr std::string_view NOTATION_ENGINEERING = "engineering";
constexpr std::string_view NOTATION_COMPACT = "compact";
constexpr std::string_view SIGN_DISPLAY_AUTO = "auto";
constexpr std::string_view SIGN_DISPLAY_NEVER = "never";
constexpr std::string_view SIGN_DISPLAY_ALWAYS = "always";
constexpr std::string_view SIGN_DISPLAY_EXCEPTZERO = "exceptZero";
constexpr std::string_view STYLE_DECIMAL = "decimal";
constexpr std::string_view STYLE_PERCENT = "percent";
constexpr std::string_view STYLE_CURRENCY = "currency";
constexpr std::string_view STYLE_UNIT = "unit";
constexpr std::string_view UNIT_DISPLAY_SHORT = "short";
constexpr std::string_view UNIT_DISPLAY_LONG = "long";
constexpr std::string_view UNIT_DISPLAY_NARROW = "narrow";
constexpr std::string_view USE_GROUPING_TRUE = "true";
constexpr std::string_view USE_GROUPING_FALSE = "false";

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct ParsedOptions {
    std::string compactDisplay;
    std::string currencySign;
    std::string currency;
    std::string currencyDisplay;
    std::string locale;
    std::string maxFractionDigits;
    std::string maxSignificantDigits;
    std::string minFractionDigits;
    std::string minIntegerDigits;
    std::string minSignificantDigits;
    std::string notation;
    std::string numberingSystem;
    std::string signDisplay;
    std::string style;
    std::string unit;
    std::string unitDisplay;
    std::string useGrouping;

    [[maybe_unused]] std::string ToString()
    {
        std::string str;
        str.append(compactDisplay);
        str.append(currencySign);
        str.append(currency);
        str.append(currencyDisplay);
        str.append(locale);
        str.append(maxFractionDigits);
        str.append(maxSignificantDigits);
        str.append(minFractionDigits);
        str.append(minIntegerDigits);
        str.append(minSignificantDigits);
        str.append(notation);
        str.append(numberingSystem);
        str.append(signDisplay);
        str.append(style);
        str.append(unit);
        str.append(unitDisplay);
        str.append(useGrouping);
        return str;
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

icu::Locale LocTagToIcuLocale(ani_env *env, const std::string &localeTag)
{
    icu::StringPiece sp {localeTag.data(), static_cast<int32_t>(localeTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Language tag '" + localeTag + std::string("' is invalid or not supported");
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return icu::Locale::getDefault();
    }
    return locale;
}

std::string GetFieldStrUndefined(ani_env *env, ani_object obj, const char *name)
{
    ani_ref ref = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(obj, name, &ref));
    ani_boolean isUndefined = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(ref, &isUndefined));
    if (isUndefined == ANI_TRUE) {
        return std::string();
    }
    auto aniStr = static_cast<ani_string>(ref);
    ASSERT(aniStr != nullptr);
    return ConvertFromAniString(env, aniStr);
}

std::string GetFieldStr(ani_env *env, ani_object obj, const char *name)
{
    ani_ref ref = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(obj, name, &ref));
    auto aniStr = static_cast<ani_string>(ref);
    ASSERT(aniStr != nullptr);
    return ConvertFromAniString(env, aniStr);
}

void ParseOptions(ani_env *env, ani_object self, ParsedOptions &options)
{
    ani_ref optionsRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(self, "options", &optionsRef));
    auto optionsObj = static_cast<ani_object>(optionsRef);
    ASSERT(optionsObj != nullptr);

    options.locale = GetFieldStr(env, optionsObj, "locale");
    options.compactDisplay = GetFieldStrUndefined(env, optionsObj, "compactDisplay");
    options.currencySign = GetFieldStrUndefined(env, optionsObj, "currencySign");
    options.currency = GetFieldStrUndefined(env, optionsObj, "currency");
    options.currencyDisplay = GetFieldStrUndefined(env, optionsObj, "currencyDisplay");
    options.minFractionDigits = GetFieldStr(env, self, "minFracStr");
    options.maxFractionDigits = GetFieldStr(env, self, "maxFracStr");
    options.minSignificantDigits = GetFieldStrUndefined(env, self, "minSignStr");
    options.maxSignificantDigits = GetFieldStrUndefined(env, self, "maxSignStr");
    options.minIntegerDigits = GetFieldStr(env, self, "minIntStr");
    options.notation = GetFieldStrUndefined(env, optionsObj, "notation");
    options.numberingSystem = GetFieldStr(env, optionsObj, "numberingSystem");
    options.signDisplay = GetFieldStrUndefined(env, optionsObj, "signDisplay");
    options.style = GetFieldStr(env, optionsObj, "style");
    options.signDisplay = GetFieldStrUndefined(env, optionsObj, "signDisplay");
    options.useGrouping = GetFieldStrUndefined(env, self, "useGroupingStr");
}

// Configure minFractionDigits, maxFractionDigits, minSignificantDigits, maxSignificantDigits, minIntegerDigits
ani_status ConfigurePrecision(const ParsedOptions &options, icu::number::LocalizedNumberFormatter &fmt)
{
    fmt = fmt.precision(icu::number::Precision::minFraction(stoi(options.minFractionDigits)));
    fmt = fmt.precision(icu::number::Precision::maxFraction(stoi(options.maxFractionDigits)));
    if (!options.minSignificantDigits.empty()) {
        fmt = fmt.precision(icu::number::Precision::minSignificantDigits(stoi(options.minSignificantDigits)));
    }
    if (!options.maxSignificantDigits.empty()) {
        fmt = fmt.precision(icu::number::Precision::maxSignificantDigits(stoi(options.maxSignificantDigits)));
    }
    fmt = fmt.integerWidth(icu::number::IntegerWidth::zeroFillTo(stoi(options.minIntegerDigits)));
    return ANI_OK;
}

ani_status ConfigureNotation(ani_env *env, const ParsedOptions &options, icu::number::LocalizedNumberFormatter &fmt)
{
    if (options.notation == NOTATION_STANDARD) {
        fmt = fmt.notation(icu::number::Notation::simple());
    } else if (options.notation == NOTATION_SCIENTIFIC) {
        fmt = fmt.notation(icu::number::Notation::scientific());
    } else if (options.notation == NOTATION_ENGINEERING) {
        fmt = fmt.notation(icu::number::Notation::engineering());
    } else if (options.notation == NOTATION_COMPACT) {
        if (options.compactDisplay == COMPACT_DISPLAY_SHORT) {
            fmt = fmt.notation(icu::number::Notation::compactShort());
        } else if (options.compactDisplay == COMPACT_DISPLAY_LONG) {
            fmt = fmt.notation(icu::number::Notation::compactLong());
        } else {
            std::string message = "Invalid compactDisplay: " + options.compactDisplay;
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    } else {
        if (!options.notation.empty()) {
            std::string message = "Invalid compactDisplay: " + options.notation;
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status SetNumberingSystemIntoLocale(ani_env *env, const ParsedOptions &options, icu::Locale &locale)
{
    if (!options.numberingSystem.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        locale.setKeywordValue("nu", options.numberingSystem.c_str(), status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "Invalid numbering system: " + options.numberingSystem;
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status ConfigureNumberingSystem([[maybe_unused]] ani_env *env, const ParsedOptions &options,
                                    const icu::Locale &localeWithNumSystem, icu::number::LocalizedNumberFormatter &fmt)
{
    if (!options.numberingSystem.empty()) {
        fmt = icu::number::NumberFormatter::withLocale(localeWithNumSystem);
    }
    return ANI_OK;
}

ani_status ConfigureStyleUnit(ani_env *env, const ParsedOptions &options, icu::number::LocalizedNumberFormatter &fmt)
{
    // Handle MeasureUnit
    UErrorCode status = U_ZERO_ERROR;
    icu::MeasureUnit unit = icu::MeasureUnit::forIdentifier(icu::StringPiece(options.unit.c_str()), status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Invalid unit: " + options.unit;
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return ANI_PENDING_ERROR;
    }
    fmt = fmt.unit(unit);
    // Set unit display
    if (options.unitDisplay == UNIT_DISPLAY_SHORT) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_SHORT);
    } else if (options.unitDisplay == UNIT_DISPLAY_LONG) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_FULL_NAME);
    } else if (options.unitDisplay == UNIT_DISPLAY_NARROW) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_NARROW);
    } else {
        if (!options.unitDisplay.empty()) {
            std::string message = "Invalid unitDisplay: " + options.unitDisplay;
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status ConfigureStyleCurrency(ani_env *env, const ParsedOptions &options,
                                  icu::number::LocalizedNumberFormatter &fmt)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::CurrencyUnit currency(icu::StringPiece(options.currency.c_str()), status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Invalid currency: " + options.currency;
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return ANI_PENDING_ERROR;
    }
    fmt = fmt.unit(currency);
    // Set currency display
    if (options.currencyDisplay == CURRENCY_DISPLAY_CODE) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_ISO_CODE);
    } else if (options.currencyDisplay == CURRENCY_DISPLAY_SYMBOL) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_SHORT);
    } else if (options.currencyDisplay == CURRENCY_DISPLAY_NARROWSYMBOL) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_NARROW);
    } else if (options.currencyDisplay == CURRENCY_DISPLAY_NAME) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_FULL_NAME);
    } else {
        // Default style, no need to set anything
        if (!options.currencyDisplay.empty()) {
            std::string message = "Invalid currencyDisplay: " + options.currencyDisplay;
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status ConfigureStyle(ani_env *env, const ParsedOptions &options, icu::number::LocalizedNumberFormatter &fmt)
{
    if (options.style == STYLE_PERCENT) {
        // Use percent unit
        fmt = fmt.unit(icu::MeasureUnit::getPercent());
    } else if (options.style == STYLE_CURRENCY && !options.currency.empty()) {
        ConfigureStyleCurrency(env, options, fmt);
    } else if (options.style == STYLE_UNIT) {
        ConfigureStyleUnit(env, options, fmt);
    } else {
        // Default style, no need to set anything
        if (!options.style.empty() && options.style != STYLE_DECIMAL) {
            std::string message = "Invalid style: " + options.style;
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status ConfigureUseGrouping(ani_env *env, const ParsedOptions &options, icu::number::LocalizedNumberFormatter &fmt)
{
    if (options.useGrouping == USE_GROUPING_TRUE) {
        fmt = fmt.grouping(UNumberGroupingStrategy::UNUM_GROUPING_ON_ALIGNED);
    } else if (options.useGrouping == USE_GROUPING_FALSE) {
        fmt = fmt.grouping(UNumberGroupingStrategy::UNUM_GROUPING_OFF);
    } else {
        // Default is UNumberGroupingStrategy::UNUM_GROUPING_AUTO
        if (!options.useGrouping.empty()) {
            std::string message = "Invalid useGrouping: " + options.useGrouping;
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status ConfigureSignDisplay(ani_env *env, const ParsedOptions &options, icu::number::LocalizedNumberFormatter &fmt)
{
    // Configure signDisplay
    if (options.signDisplay == SIGN_DISPLAY_AUTO) {
        // Handle currency sign
        if (options.currencySign == CURRENCY_SIGN_ACCOUNTING) {
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ACCOUNTING);
        } else {
            // CURRENCY_SIGN_STANDARD
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_AUTO);
        }
    } else if (options.signDisplay == SIGN_DISPLAY_NEVER) {
        fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_NEVER);
    } else if (options.signDisplay == SIGN_DISPLAY_ALWAYS) {
        // Handle currency sign
        if (options.currencySign == CURRENCY_SIGN_ACCOUNTING) {
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_ALWAYS);
        } else {
            // CURRENCY_SIGN_STANDARD
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ALWAYS);
        }
        fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ALWAYS);
    } else if (options.signDisplay == SIGN_DISPLAY_EXCEPTZERO) {
        // Handle currency sign
        if (options.currencySign == CURRENCY_SIGN_ACCOUNTING) {
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_EXCEPT_ZERO);
        } else {
            // CURRENCY_SIGN_STANDARD
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ALWAYS);
        }
    } else {
        if (!options.signDisplay.empty()) {
            ThrowNewError(env, "Lstd/core/RuntimeException;", "Invalid signDisplay", "Lstd/core/String;:V");
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status CreateNumFormatter(ani_env *env, const ParsedOptions &options, icu::number::LocalizedNumberFormatter &fmt)
{
    // Create formatter
    auto locale = LocTagToIcuLocale(env, options.locale);
    ani_status err = SetNumberingSystemIntoLocale(env, options, locale);
    if (err == ANI_OK) {
        auto str = options.locale + options.numberingSystem;
        fmt = g_intlState->fmtsCache.NumFmtsCacheInvalidation(str, locale);

        // Configure
        err = ConfigureNumberingSystem(env, options, locale, fmt);
        err = err == ANI_OK ? ConfigureNotation(env, options, fmt) : err;
        err = err == ANI_OK ? ConfigurePrecision(options, fmt) : err;
        err = err == ANI_OK ? ConfigureStyle(env, options, fmt) : err;
        err = err == ANI_OK ? ConfigureUseGrouping(env, options, fmt) : err;
        err = err == ANI_OK ? ConfigureSignDisplay(env, options, fmt) : err;
    }
    return err;
}

ani_status CreateNumRangeFormatter(ani_env *env, const ParsedOptions &options,
                                   icu::number::LocalizedNumberRangeFormatter &fmt)
{
    // Create
    const icu::Locale &loc = LocTagToIcuLocale(env, options.locale);
    fmt = g_intlState->fmtsCache.NumRangeFmtsCacheInvalidation(options.locale, loc);
    return ANI_OK;
}

ani_string IcuFormatDouble(ani_env *env, ani_object self, ani_double value)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    icu::number::LocalizedNumberFormatter formatter;
    if (CreateNumFormatter(env, options, formatter) == ANI_OK) {
        UErrorCode status = U_ZERO_ERROR;
        auto fmtNumber = formatter.formatDouble(value, status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "IcuFormatDouble. Unable to format double " + std::to_string(value);
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return nullptr;
        }
        icu::UnicodeString ustr = fmtNumber.toString(status);
        return UnicodeToAniStr(env, ustr);
    }
    return nullptr;
}

ani_string IcuFormatDecStr(ani_env *env, ani_object self, ani_string value)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    icu::number::LocalizedNumberFormatter formatter;
    if (CreateNumFormatter(env, options, formatter) == ANI_OK) {
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
    return nullptr;
}

ani_string IcuFormatRange(ani_env *env, ani_object self, icu::Formattable startFrmtbl, icu::Formattable endFrmtbl)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    icu::number::LocalizedNumberRangeFormatter formatter;
    if (CreateNumRangeFormatter(env, options, formatter) == ANI_OK) {
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
    return nullptr;
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

ani_string IcuFormatRangeDoubleDouble(ani_env *env, ani_object self, double startValue, double endValue)
{
    return IcuFormatRange(env, self, DoubleToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDoubleDecStr(ani_env *env, ani_object self, double startValue, ani_string endValue)
{
    return IcuFormatRange(env, self, DoubleToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDouble(ani_env *env, ani_object self, ani_string startValue, double endValue)
{
    return IcuFormatRange(env, self, EtsStrToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDecStr(ani_env *env, ani_object self, ani_string startValue, ani_string endValue)
{
    return IcuFormatRange(env, self, EtsStrToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ani_status RegisterIntlNumberFormatNativeMethods(ani_env *env)
{
    ani_class numberFormatClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/NumberFormat;", &numberFormatClass));

    const auto methods = std::array {
        ani_native_function {"formatDouble", "D:Lstd/core/String;", reinterpret_cast<void *>(IcuFormatDouble)},
        ani_native_function {"formatDecStr", "Lstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatDecStr)},
        ani_native_function {"formatRangeDoubleDouble", "DD:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDoubleDouble)},
        ani_native_function {"formatRangeDoubleDecStr", "DLstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDoubleDecStr)},
        ani_native_function {"formatRangeDecStrDouble", "Lstd/core/String;D:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDecStrDouble)},
        ani_native_function {"formatRangeDecStrDecStr", "Lstd/core/String;Lstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(IcuFormatRangeDecStrDecStr)}};

    ani_status status = env->Class_BindNativeMethods(numberFormatClass, methods.data(), methods.size());
    if (!(status == ANI_OK || status == ANI_ALREADY_BINDED)) {
        return status;
    }

    return ANI_OK;
}

}  // namespace ark::ets::stdlib::intl
