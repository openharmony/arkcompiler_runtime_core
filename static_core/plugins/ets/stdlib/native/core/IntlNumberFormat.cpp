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
#include <unicode/formattedvalue.h>
#include "ani.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/IntlState.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "libpandabase/macros.h"
#include "unicode/numberformatter.h"
#include "unicode/numberrangeformatter.h"
#include "unicode/locid.h"
#include "unicode/unistr.h"
#include "IntlCommon.h"

#include <algorithm>
#include <optional>
#include <string>
#include <cstdlib>
#include <array>
#include <utility>

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

constexpr std::string_view INTEGER_FIELD = "integer";
constexpr std::string_view FRACTION_FIELD = "fraction";
constexpr std::string_view DECIMAL_SEPARATOR_FIELD = "decimal";
constexpr std::string_view EXPONENT_SYMBOL_FIELD = "exponentSymbol";
constexpr std::string_view GROUPING_SEPARATOR_FIELD = "group";
constexpr std::string_view CURRENCY_FIELD = "currency";
constexpr std::string_view PERCENT_FIELD = "percentSign";
constexpr std::string_view PERMILL_FIELD = "perMille";
constexpr std::string_view PLUS_SIGN = "plusSign";
constexpr std::string_view MINUS_SIGN = "minusSign";
constexpr std::string_view EXPONENT_MINUS_SIGN = "exponentMinusSign";
constexpr std::string_view EXPONENT_INTEGER = "exponentInteger";
constexpr std::string_view COMPACT_FIELD = "compact";
constexpr std::string_view MEASURE_UNIT_FIELD = "unit";
constexpr std::string_view APPROXIMATELY_SIGN_FIELD = "approximatelySign";

/// Fallback
constexpr std::string_view LITERAL_FIELD = "literal";

/// Sources
constexpr std::string_view START_RANGE_SOURCE = "startRange";
constexpr std::string_view END_RANGE_SOURCE = "endRange";
constexpr std::string_view SHARED_SOURCE = "shared";

constexpr int32_t INTL_NUMBER_RANGE_START = 0;
constexpr int32_t INTL_NUMBER_RANGE_END = 1;

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

class NumberFormatPart {
public:
    std::string type;
    std::string value;
    std::string source;
};

/*
 * FieldSpan class, used in formatToParts/formatRangeToParts
 *
 * Example:
 * ------------------------------------------------
 * Consider the formatted string: "-5678.9"
    0: "sign",     "-"
    1: "integer",  "5"
    2: "group",    ","
    3: "integer",  "678"
    4: "decimal",  "."
    5: "fraction", "9"
 */
class FieldSpan {
public:
    FieldSpan(int32_t fieldType, int32_t startIndex, int32_t endIndex)
        : fieldType_(fieldType), startIndex_(startIndex), endIndex_(endIndex)
    {
    }

    // Accessors
    int32_t GetFieldType() const
    {
        return fieldType_;
    }
    int32_t GetStartIndex() const
    {
        return startIndex_;
    }
    int32_t GetEndIndex() const
    {
        return endIndex_;
    }
    std::tuple<int32_t, int32_t> GetStartEndPositionsTuple() const
    {
        return std::make_tuple(startIndex_, endIndex_);
    }

private:
    /// Indicates the type of formatting (e.g., integer, fraction, etc.).
    int32_t fieldType_;
    /// The starting index (inclusive) of the span in the formatted string.
    int32_t startIndex_;
    /// The ending index (exclusive) of the span.
    int32_t endIndex_;
};

class SpanFolder {
public:
    explicit SpanFolder(std::vector<FieldSpan> spans) : spans_ {std::move(spans)}, currentActiveSpan_ {spans_.at(0)}
    {
        activeSpanIndexStack_.push_back(0);
    }
    std::vector<FieldSpan> Fold()
    {
        if (spans_.size() <= 1) {
            return spans_;
        }
        std::sort(spans_.begin(), spans_.end(), [](const auto &a, const auto &b) {
            const auto [aStart, aEnd] = a.GetStartEndPositionsTuple();
            const auto [bStart, bEnd] = b.GetStartEndPositionsTuple();
            if (aStart < bStart) {
                return true;
            }
            if (aStart > bStart) {
                return false;
            }
            if (aEnd < bEnd) {
                return false;
            }
            if (aEnd > bEnd) {
                return true;
            }
            return a.GetFieldType() < b.GetFieldType();
        });
        std::vector<FieldSpan> nonOverlappingSegments;
        // Stack to track the indexes of spans that are currently active/overlapping
        size_t nextSpanIndex = 1;
        // Total length to process (end of the first span, which should cover the entire string)
        const int32_t totalStringLength = spans_.at(0).GetEndIndex();
        while (currentPosition_ < totalStringLength) {
            // Determine where the next segment starts
            const int32_t nextSegmentStartPosition =
                (nextSpanIndex < spans_.size()) ? spans_.at(nextSpanIndex).GetStartIndex() : totalStringLength;
            if (currentPosition_ < nextSegmentStartPosition) {
                if (ProcessSpansUntilNextSegment(nextSegmentStartPosition, nonOverlappingSegments)) {
                    return nonOverlappingSegments;
                }
            }
            if (nextSpanIndex < spans_.size()) {
                activeSpanIndexStack_.push_back(nextSpanIndex++);
                currentActiveSpan_ = spans_.at(activeSpanIndexStack_.back());
            }
        }
        return nonOverlappingSegments;
    }

private:
    /// The spans to process
    std::vector<FieldSpan> spans_;
    /// The current active span
    FieldSpan currentActiveSpan_;
    /// The stack of active span indexes
    std::vector<size_t> activeSpanIndexStack_;
    /// The current position
    int32_t currentPosition_ {0};

    // Adds a segment to the result vector
    static void AddSegment(std::vector<FieldSpan> &segments, int32_t fieldType, int32_t start, int32_t end)
    {
        segments.emplace_back(fieldType, start, end);
    }

    bool ProcessSpansUntilNextSegment(int32_t nextSegmentStartPosition, std::vector<FieldSpan> &nonOverlappingSegments)
    {
        while (currentActiveSpan_.GetEndIndex() < nextSegmentStartPosition) {
            if (currentPosition_ < currentActiveSpan_.GetEndIndex()) {
                AddSegment(nonOverlappingSegments, currentActiveSpan_.GetFieldType(), currentPosition_,
                           currentActiveSpan_.GetEndIndex());

                currentPosition_ = currentActiveSpan_.GetEndIndex();
            }
            activeSpanIndexStack_.pop_back();
            if (activeSpanIndexStack_.empty()) {
                return true;
            }
            currentActiveSpan_ = spans_.at(activeSpanIndexStack_.back());
        }
        if (currentPosition_ < nextSegmentStartPosition) {
            AddSegment(nonOverlappingSegments, currentActiveSpan_.GetFieldType(), currentPosition_,
                       nextSegmentStartPosition);
            currentPosition_ = nextSegmentStartPosition;
        }
        return false;
    }
};

std::string_view GetFieldTypeString(const FieldSpan &field, const icu::UnicodeString &text)
{
    switch (static_cast<UNumberFormatFields>(field.GetFieldType())) {
        case UNUM_INTEGER_FIELD:
            return INTEGER_FIELD;
        case UNUM_FRACTION_FIELD:
            return FRACTION_FIELD;
        case UNUM_DECIMAL_SEPARATOR_FIELD:
            return DECIMAL_SEPARATOR_FIELD;
        case UNUM_EXPONENT_SYMBOL_FIELD:
            return EXPONENT_SYMBOL_FIELD;
        case UNUM_EXPONENT_SIGN_FIELD:
            return EXPONENT_MINUS_SIGN;
        case UNUM_EXPONENT_FIELD:
            return EXPONENT_INTEGER;
        case UNUM_GROUPING_SEPARATOR_FIELD:
            return GROUPING_SEPARATOR_FIELD;
        case UNUM_CURRENCY_FIELD:
            return CURRENCY_FIELD;
        case UNUM_PERCENT_FIELD:
            return PERCENT_FIELD;
        case UNUM_SIGN_FIELD:
            return (text.charAt(field.GetStartIndex()) == '+') ? PLUS_SIGN : MINUS_SIGN;
        case UNUM_PERMILL_FIELD:
            return PERMILL_FIELD;
        case UNUM_COMPACT_FIELD:
            return COMPACT_FIELD;
        case UNUM_MEASURE_UNIT_FIELD:
            return MEASURE_UNIT_FIELD;
        case UNUM_APPROXIMATELY_SIGN_FIELD:
            return APPROXIMATELY_SIGN_FIELD;
        default:
            UNREACHABLE();
    }
}

icu::Locale LocTagToIcuLocale(ani_env *env, const std::string &locTag)
{
    icu::StringPiece sp {locTag.data(), static_cast<int32_t>(locTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Language tag '" + locTag + std::string("' is invalid or not supported");
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

static ani_double NormalizeIfNaN(ani_double value)
{
    return std::isnan(value) ? std::numeric_limits<ani_double>::quiet_NaN() : value;
}

ani_string IcuFormatDouble(ani_env *env, ani_object self, ani_double value)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    icu::number::LocalizedNumberFormatter formatter;
    if (CreateNumFormatter(env, options, formatter) == ANI_OK) {
        UErrorCode status = U_ZERO_ERROR;
        auto fmtNumber = formatter.formatDouble(NormalizeIfNaN(value), status);
        if (UNLIKELY(U_FAILURE(status) != 0)) {
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
        if (UNLIKELY(U_FAILURE(status) != 0)) {
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

template <typename T>
struct GetICUFormattedNumber {
    static std::optional<icu::number::FormattedNumber> GetFormattedNumber(
        const icu::number::LocalizedNumberFormatter &formatter, T value);
};

template <>
struct GetICUFormattedNumber<ani_double> {
    static std::optional<icu::number::FormattedNumber> GetFormattedNumber(
        const icu::number::LocalizedNumberFormatter &formatter, ani_double value)
    {
        UErrorCode status = U_ZERO_ERROR;
        auto formatted = formatter.formatDouble(value, status);
        if (U_FAILURE(status) != 0) {
            return std::nullopt;
        }
        return formatted;
    }
};

template <>
struct GetICUFormattedNumber<std::string> {
    static std::optional<icu::number::FormattedNumber> GetFormattedNumber(
        const icu::number::LocalizedNumberFormatter &formatter, const std::string &value)
    {
        UErrorCode status = U_ZERO_ERROR;
        const icu::StringPiece sp {value.data(), static_cast<int32_t>(value.size())};
        auto formatted = formatter.formatDecimal(sp, status);
        if (U_FAILURE(status) != 0) {
            return std::nullopt;
        }
        return formatted;
    }
};

std::vector<NumberFormatPart> ProcessSpansToFormatParts(const std::vector<FieldSpan> &flatParts,
                                                        const icu::UnicodeString &formattedString,
                                                        const std::pair<int32_t, int32_t> &startRangeSource,
                                                        const std::pair<int32_t, int32_t> &endRangeSource,
                                                        bool calculateSources)
{
    std::vector<NumberFormatPart> parts;
    auto contains = [](std::pair<int32_t, int32_t> gold, std::pair<int32_t, int32_t> request) {
        return gold.first <= request.first && gold.second >= request.second;
    };

    for (const auto &part : flatParts) {
        std::string_view ty = LITERAL_FIELD;
        std::string_view source = SHARED_SOURCE;

        icu::UnicodeString sub;
        formattedString.extractBetween(part.GetStartIndex(), part.GetEndIndex(), sub);
        std::string value;
        sub.toUTF8String(value);

        if (part.GetFieldType() != -1) {
            ty = GetFieldTypeString(part, formattedString);
        }

        if (calculateSources) {
            auto request = std::make_pair(part.GetStartIndex(), part.GetEndIndex());
            if (contains(startRangeSource, request)) {
                source = START_RANGE_SOURCE;
            } else if (contains(endRangeSource, request)) {
                source = END_RANGE_SOURCE;
            }
        }
        parts.push_back({std::string(ty), value, std::string(source)});
    }
    return parts;
}

std::vector<NumberFormatPart> ExtractParts(ani_env *env, const icu::FormattedValue &formatted, bool calculateSources)
{
    UErrorCode status = U_ZERO_ERROR;
    std::vector<NumberFormatPart> parts;
    icu::UnicodeString formattedString = formatted.toString(status);

    if (UNLIKELY(U_FAILURE(status) != 0)) {
        std::string message = "ExtractParts. Unable to convert formatted value to string";
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return parts;
    }

    std::pair<int32_t, int32_t> startRangeSource = std::make_pair(0, 0);
    std::pair<int32_t, int32_t> endRangeSource = std::make_pair(0, 0);

    std::vector<FieldSpan> spans;

    // Add a default span for the entire string
    spans.emplace_back(-1, 0, formattedString.length());
    // Source (sep like "-", "~") for range to parts formatting looks like
    // span is a representation of semi-open interval [start, end)
    // For example "start: 0.123, end: 0.124"
    // formatted: "-0.123-0.124"
    //  - 0 . 1 2 3 - 0 . 1 2 4
    // |<--------->|
    //             |-|
    //             |<--------->|
    //  ^^^^^^^^^^^
    //  startRange               <------------------ [0, 6)
    //              ^
    //           shared          <------------------ [6, 7)
    //
    //              ^^^^^^^^^^^^
    //              endRange     <------------------ [7, 12)
    icu::ConstrainedFieldPosition cfpos;
    while (formatted.nextPosition(cfpos, status) != 0) {
        if (UNLIKELY(U_FAILURE(status) != 0)) {
            std::string message = "ExtractParts. Error during field iteration";
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return parts;
        }
        int32_t cat = cfpos.getCategory();
        int32_t field = cfpos.getField();
        int32_t start = cfpos.getStart();
        int32_t limit = cfpos.getLimit();
        if (cat == UFIELD_CATEGORY_NUMBER_RANGE_SPAN) {
            switch (field) {
                case INTL_NUMBER_RANGE_START:
                    startRangeSource.first = start;
                    startRangeSource.second = limit;
                    break;
                case INTL_NUMBER_RANGE_END:
                    endRangeSource.first = start;
                    endRangeSource.second = limit;
                    break;
                default:
                    UNREACHABLE();
            }
        } else {
            spans.emplace_back(field, start, limit);
        }
    }
    return ProcessSpansToFormatParts(SpanFolder(std::move(spans)).Fold(), formattedString, startRangeSource,
                                     endRangeSource, calculateSources);
}

ani_object GetNumberFormatPart(ani_env *env, ani_string ty, ani_string value)
{
    ani_class numberFormatPartClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/NumberFormatPart;", &numberFormatPartClass));

    ani_method constructorMethod;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(numberFormatPartClass, "<ctor>", ":V", &constructorMethod));

    ani_object numberFormatPartObj;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(numberFormatPartClass, constructorMethod, &numberFormatPartObj));

    ani_field typeField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberFormatPartClass, "type", &typeField));

    ani_field valueField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberFormatPartClass, "value", &valueField));

    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberFormatPartObj, typeField, ty));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberFormatPartObj, valueField, value));

    return numberFormatPartObj;
}

ani_object GetNumberFormatRangePart(ani_env *env, ani_string ty, ani_string value, ani_string source)
{
    ani_class numberRangeFormatPartClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/NumberRangeFormatPart;", &numberRangeFormatPartClass));

    ani_method constructorMethod;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(numberRangeFormatPartClass, "<ctor>", ":V", &constructorMethod));

    ani_object numberRangeFormatPartObj;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(numberRangeFormatPartClass, constructorMethod, &numberRangeFormatPartObj));

    ani_field typeField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberRangeFormatPartClass, "type", &typeField));

    ani_field valueField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberRangeFormatPartClass, "value", &valueField));

    ani_field sourceField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberRangeFormatPartClass, "source", &sourceField));

    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberRangeFormatPartObj, typeField, ty));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberRangeFormatPartObj, valueField, value));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberRangeFormatPartObj, sourceField, source));
    return numberRangeFormatPartObj;
}

template <typename T>
ani_array_ref IcuFormatToParts(ani_env *env, [[maybe_unused]] ani_object self, [[maybe_unused]] T value)
{
    ani_array_ref resultArray;
    ParsedOptions options;
    ParseOptions(env, self, options);

    icu::number::LocalizedNumberFormatter formatter;
    if (CreateNumFormatter(env, options, formatter) != ANI_OK) {
        return nullptr;
    }

    auto fmtNumber = GetICUFormattedNumber<T>::GetFormattedNumber(formatter, value);
    if (!fmtNumber) {
        return nullptr;
    }

    std::vector<ani_object> resultParts;
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString ustr = fmtNumber->toString(status);

    auto parts = ExtractParts(env, static_cast<const icu::FormattedValue &>(*fmtNumber), false);
    for (const auto &part : parts) {
        auto utype = CreateUtf8String(env, part.type.c_str(), part.type.size());
        auto uvalue = CreateUtf8String(env, part.value.c_str(), part.value.size());
        auto partObj = GetNumberFormatPart(env, utype, uvalue);
        resultParts.push_back(partObj);
    }

    ani_class numberFormatPartClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/NumberFormatPart;", &numberFormatPartClass));
    ANI_FATAL_IF_ERROR(env->Array_New_Ref(numberFormatPartClass, resultParts.size(), nullptr, &resultArray));

    for (size_t i = 0; i < resultParts.size(); ++i) {
        ANI_FATAL_IF_ERROR(env->Array_Set_Ref(resultArray, i, resultParts[i]));
    }

    return resultArray;
}

ani_array_ref IcuFormatToRangeParts(ani_env *env, [[maybe_unused]] ani_object self,
                                    [[maybe_unused]] const icu::Formattable &startFrmtbl,
                                    [[maybe_unused]] const icu::Formattable &endFrmtbl)
{
    ani_array_ref resultArray;

    ParsedOptions options;
    ParseOptions(env, self, options);

    icu::number::LocalizedNumberRangeFormatter formatter;
    if (CreateNumRangeFormatter(env, options, formatter) != ANI_OK) {
        return nullptr;
    }

    UErrorCode status = U_ZERO_ERROR;
    icu::number::FormattedNumberRange formatted = formatter.formatFormattableRange(startFrmtbl, endFrmtbl, status);

    if (U_FAILURE(status) != 0) {
        return nullptr;
    }

    std::vector<NumberFormatPart> parts = ExtractParts(env, formatted, true);

    ani_class numberRangeFormatPartClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/NumberRangeFormatPart;", &numberRangeFormatPartClass));
    ANI_FATAL_IF_ERROR(env->Array_New_Ref(numberRangeFormatPartClass, parts.size(), nullptr, &resultArray));

    for (size_t i = 0; i < parts.size(); ++i) {
        auto typeString = CreateUtf8String(env, parts[i].type.c_str(), parts[i].type.size());
        auto valueString = CreateUtf8String(env, parts[i].value.c_str(), parts[i].value.size());
        auto sourceString = CreateUtf8String(env, parts[i].source.c_str(), parts[i].source.size());
        auto part = GetNumberFormatRangePart(env, typeString, valueString, sourceString);
        ANI_FATAL_IF_ERROR(env->Array_Set_Ref(resultArray, i, part));
    }

    return resultArray;
}

ani_string IcuFormatRangeDoubleDouble(ani_env *env, ani_object self, ani_double startValue, ani_double endValue)
{
    return IcuFormatRange(env, self, DoubleToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDoubleDecStr(ani_env *env, ani_object self, ani_double startValue, ani_string endValue)
{
    return IcuFormatRange(env, self, DoubleToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDouble(ani_env *env, ani_object self, ani_string startValue, ani_double endValue)
{
    return IcuFormatRange(env, self, EtsStrToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDecStr(ani_env *env, ani_object self, ani_string startValue, ani_string endValue)
{
    return IcuFormatRange(env, self, EtsStrToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ani_array_ref IcuFormatToPartsDouble(ani_env *env, [[maybe_unused]] ani_object self, [[maybe_unused]] ani_double value)
{
    return IcuFormatToParts<ani_double>(env, self, value);
}

ani_array_ref IcuFormatToPartsDecStr(ani_env *env, [[maybe_unused]] ani_object self, [[maybe_unused]] ani_string value)
{
    return IcuFormatToParts<std::string>(env, self, ConvertFromAniString(env, value));
}

ani_array_ref IcuFormatToRangePartsDoubleDouble(ani_env *env, [[maybe_unused]] ani_object self,
                                                [[maybe_unused]] ani_double startValue,
                                                [[maybe_unused]] ani_double endValue)
{
    return IcuFormatToRangeParts(env, self, DoubleToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_array_ref IcuFormatToRangePartsDoubleDecStr(ani_env *env, [[maybe_unused]] ani_object self,
                                                [[maybe_unused]] ani_double startValue,
                                                [[maybe_unused]] ani_string endValue)
{
    return IcuFormatToRangeParts(env, self, DoubleToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
}

ani_array_ref IcuFormatToRangePartsDecStrDouble(ani_env *env, [[maybe_unused]] ani_object self,
                                                [[maybe_unused]] ani_string startValue,
                                                [[maybe_unused]] ani_double endValue)
{
    return IcuFormatToRangeParts(env, self, EtsStrToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_array_ref IcuFormatToRangePartsDecStrDecStr(ani_env *env, [[maybe_unused]] ani_object self,
                                                [[maybe_unused]] ani_string startValue,
                                                [[maybe_unused]] ani_string endValue)
{
    return IcuFormatToRangeParts(env, self, EtsStrToFormattable(env, startValue), EtsStrToFormattable(env, endValue));
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
                             reinterpret_cast<void *>(IcuFormatRangeDecStrDecStr)},
        ani_native_function {"formatToPartsDouble", "D:[Lstd/core/Intl/NumberFormatPart;",
                             reinterpret_cast<void *>(IcuFormatToPartsDouble)},
        ani_native_function {"formatToPartsDecStr", "Lstd/core/String;:[Lstd/core/Intl/NumberFormatPart;",
                             reinterpret_cast<void *>(IcuFormatToPartsDecStr)},
        ani_native_function {"formatToRangePartsDoubleDouble", "DD:[Lstd/core/Intl/NumberRangeFormatPart;",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDoubleDouble)},
        ani_native_function {"formatToRangePartsDoubleDecStr",
                             "DLstd/core/String;:[Lstd/core/Intl/NumberRangeFormatPart;",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDoubleDecStr)},
        ani_native_function {"formatToRangePartsDecStrDouble",
                             "Lstd/core/String;D:[Lstd/core/Intl/NumberRangeFormatPart;",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDecStrDouble)},
        ani_native_function {"formatToRangePartsDecStrDecStr",
                             "Lstd/core/String;Lstd/core/String;:[Lstd/core/Intl/NumberRangeFormatPart;",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDecStrDecStr)},
    };

    ani_status status = env->Class_BindNativeMethods(numberFormatClass, methods.data(), methods.size());
    if (!(status == ANI_OK || status == ANI_ALREADY_BINDED)) {
        return status;
    }

    return ANI_OK;
}

}  // namespace ark::ets::stdlib::intl
