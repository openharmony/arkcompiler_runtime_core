/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <array>
#include <vector>
#include <cstring>

#include "unicode/locid.h"
#include "unicode/unistr.h"
#include "unicode/datefmt.h"
#include "unicode/dtptngen.h"
#include "unicode/smpdtfmt.h"

#include "libpandabase/macros.h"
#include "plugins/ets/stdlib/native/core/IntlDateTimeFormat.h"

#include "ani/ani_checkers.h"

namespace ark::ets::stdlib::intl {
constexpr const char *FORMAT_HOUR_SYMBOLS = "hHkK";

constexpr const char *HOUR_CYCLE_11 = "h11";
constexpr const char *HOUR_CYCLE_12 = "h12";
constexpr const char *HOUR_CYCLE_23 = "h23";
constexpr const char *HOUR_CYCLE_24 = "h24";

constexpr const char *STYLE_FULL = "full";
constexpr const char *STYLE_LONG = "long";
constexpr const char *STYLE_MEDIUM = "medium";
constexpr const char *STYLE_SHORT = "short";

static ani_status ThrowError(ani_env *env, const char *errorClassName, const std::string &message)
{
    ani_status aniStatus = ANI_OK;

    ani_class errorClass = nullptr;
    aniStatus = env->FindClass(errorClassName, &errorClass);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_method errorCtor;
    aniStatus = env->Class_FindMethod(errorClass, "<ctor>", "Lstd/core/String;:V", &errorCtor);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_string errorMsg;
    aniStatus = env->String_NewUTF8(message.data(), message.size(), &errorMsg);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_object errorObj;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    aniStatus = env->Object_New(errorClass, errorCtor, &errorObj, errorMsg);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    return env->ThrowError(static_cast<ani_error>(errorObj));
}

static ani_status ThrowRangeError(ani_env *env, const std::string &message)
{
    return ThrowError(env, "Lstd/core/RangeError;", message);
}

static ani_status ThrowInternalError(ani_env *env, const std::string &message)
{
    return ThrowError(env, "Lstd/core/InternalError;", message);
}

static std::unique_ptr<icu::Locale> ToIcuLocale(ani_env *env, ani_object self)
{
    ani_ref localeRef = nullptr;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(self, "locale", &localeRef);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "Failed to get DateTimeFormat.locale field");
        return nullptr;
    }

    auto locale = static_cast<ani_string>(localeRef);

    ani_boolean localeIsUndefined = ANI_FALSE;
    aniStatus = env->Reference_IsUndefined(locale, &localeIsUndefined);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "DateTimeFormat.locale != undefined check failed");
        return nullptr;
    }

    if (localeIsUndefined == ANI_TRUE) {
        return std::make_unique<icu::Locale>(icu::Locale::getDefault());
    }

    ASSERT(locale != nullptr);
    ani_size localeStrSize = 0;
    aniStatus = env->String_GetUTF8Size(locale, &localeStrSize);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "Failed to get locale string size");
        return nullptr;
    }

    ani_size copiedCharsCount = 0;
    std::vector<char> localeStrBuf(localeStrSize + 1);

    aniStatus = env->String_GetUTF8(locale, localeStrBuf.data(), localeStrBuf.size(), &copiedCharsCount);
    ASSERT(localeStrSize == copiedCharsCount);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "Failed to get locale string");
        return nullptr;
    }

    return std::make_unique<icu::Locale>(localeStrBuf.data());
}

static icu::DateFormat::EStyle ToICUDateTimeStyle(ani_env *env, ani_string style, ani_status &aniStatus)
{
    ani_size styleStrSize = 0;
    aniStatus = env->String_GetUTF8Size(style, &styleStrSize);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to get format style string size");
        return icu::DateFormat::NONE;
    }

    ani_size copiedCharsCount = 0;
    std::vector<char> styleStrBuf(styleStrSize + 1);

    aniStatus = env->String_GetUTF8(style, styleStrBuf.data(), styleStrBuf.size(), &copiedCharsCount);
    ASSERT(copiedCharsCount == styleStrSize);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to get format style string");
        return icu::DateFormat::NONE;
    }

    if (strcmp(styleStrBuf.data(), STYLE_FULL) == 0) {
        return icu::DateFormat::FULL;
    }
    if (strcmp(styleStrBuf.data(), STYLE_LONG) == 0) {
        return icu::DateFormat::LONG;
    }
    if (strcmp(styleStrBuf.data(), STYLE_MEDIUM) == 0) {
        return icu::DateFormat::MEDIUM;
    }
    if (strcmp(styleStrBuf.data(), STYLE_SHORT) == 0) {
        return icu::DateFormat::SHORT;
    }

    return icu::DateFormat::NONE;
}

static ani_status DateFormatSetTimeZone(ani_env *env, icu::DateFormat *dateFormat, ani_object options)
{
    ani_ref timeZoneRef = nullptr;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(options, "timeZone", &timeZoneRef);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to get options.timeZone field");
        return ANI_PENDING_ERROR;
    }

    auto timeZone = static_cast<ani_string>(timeZoneRef);

    ani_boolean timeZoneUndefined = ANI_FALSE;
    aniStatus = env->Reference_IsUndefined(timeZone, &timeZoneUndefined);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    if (timeZoneUndefined == ANI_TRUE) {
        return ANI_OK;
    }

    ASSERT(timeZone != nullptr);
    ani_size timeZoneStrSize = 0;
    aniStatus = env->String_GetUTF16Size(timeZone, &timeZoneStrSize);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_size copiedCharsCount = 0;
    std::vector<uint16_t> timeZoneStr(timeZoneStrSize + 1);

    aniStatus = env->String_GetUTF16(timeZone, timeZoneStr.data(), timeZoneStr.size(), &copiedCharsCount);
    ASSERT(copiedCharsCount == timeZoneStrSize);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    icu::UnicodeString timeZoneId(timeZoneStr.data(), timeZoneStrSize);
    std::unique_ptr<icu::TimeZone> icuTimeZone(icu::TimeZone::createTimeZone(timeZoneId));

    if (*icuTimeZone == icu::TimeZone::getUnknown()) {
        std::string invalidTimeZoneId;
        timeZoneId.toUTF8String(invalidTimeZoneId);

        std::string errorMessage = "Invalid time zone specified: ";
        errorMessage += invalidTimeZoneId;

        aniStatus = ThrowRangeError(env, errorMessage);
        ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

        return ANI_PENDING_ERROR;
    }

    dateFormat->adoptTimeZone(icuTimeZone.release());

    return ANI_OK;
}

static ani_string StdCoreIntlDateTimeFormatGetLocaleHourCycle(ani_env *env, ani_object self)
{
    std::unique_ptr<icu::Locale> icuLocale = ToIcuLocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }

    UErrorCode icuStatus = U_ZERO_ERROR;
    std::unique_ptr<icu::DateTimePatternGenerator> hourCycleResolver(
        icu::DateTimePatternGenerator::createInstance(*icuLocale, icuStatus));

    if (U_FAILURE(icuStatus) != 0) {
        ThrowInternalError(env, "Locale hour cycle resolver instance creation failed");
        return nullptr;
    }

    UDateFormatHourCycle localeHourCycle = hourCycleResolver->getDefaultHourCycle(icuStatus);
    if (U_FAILURE(icuStatus) != 0) {
        ThrowInternalError(env, "Failed to resolve locale hour cycle");
        return nullptr;
    }

    const char *resolvedHourCycle = nullptr;
    switch (localeHourCycle) {
        case UDAT_HOUR_CYCLE_11:
            resolvedHourCycle = HOUR_CYCLE_11;
            break;
        case UDAT_HOUR_CYCLE_12:
            resolvedHourCycle = HOUR_CYCLE_12;
            break;
        case UDAT_HOUR_CYCLE_23:
            resolvedHourCycle = HOUR_CYCLE_23;
            break;
        case UDAT_HOUR_CYCLE_24:
            resolvedHourCycle = HOUR_CYCLE_24;
            break;
    }

    if (resolvedHourCycle == nullptr) {
        ThrowInternalError(env, "Failed to resolve locale hour cycle");
        return nullptr;
    }

    ani_string result = nullptr;
    ani_status aniStatus = env->String_NewUTF8(resolvedHourCycle, strlen(resolvedHourCycle), &result);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "Resolved hour cycle string creation failed");
        return nullptr;
    }

    return result;
}

static char GetFormatHourSymbol(const char *hourCycle)
{
    if (strcmp(hourCycle, HOUR_CYCLE_11) == 0) {
        return 'K';
    }
    if (strcmp(hourCycle, HOUR_CYCLE_12) == 0) {
        return 'h';
    }
    if (strcmp(hourCycle, HOUR_CYCLE_23) == 0) {
        return 'H';
    }
    if (strcmp(hourCycle, HOUR_CYCLE_24) == 0) {
        return 'k';
    }

    return 'H';
}

static ani_status AlignPatternWithHourCycle(ani_env *env, icu::UnicodeString *pattern, ani_string hourCycle)
{
    ani_size hourCycleStrSize = 0;
    ani_status aniStatus = env->String_GetUTF8Size(hourCycle, &hourCycleStrSize);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_size copiedCharsCount = 0;
    std::vector<char> hourCycleStrBuf(hourCycleStrSize + 1);

    aniStatus = env->String_GetUTF8(hourCycle, hourCycleStrBuf.data(), hourCycleStrBuf.size(), &copiedCharsCount);
    ASSERT(copiedCharsCount == hourCycleStrSize);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    std::string patternStr;
    pattern->toUTF8String(patternStr);

    auto hourFmtStartIdx = patternStr.find_first_of(FORMAT_HOUR_SYMBOLS);
    auto hourFmtEndIdx = patternStr.find_last_of(FORMAT_HOUR_SYMBOLS);

    if (hourFmtStartIdx != std::string::npos) {
        auto hourFmtSize = hourFmtEndIdx - hourFmtStartIdx + 1;
        char formatHourSymbol = GetFormatHourSymbol(hourCycleStrBuf.data());
        patternStr.replace(hourFmtStartIdx, hourFmtSize, hourFmtSize, formatHourSymbol);

        *pattern = icu::UnicodeString(patternStr.data());
    }

    return ANI_OK;
}

static std::unique_ptr<icu::DateFormat> CreateSkeletonBasedDateFormat(ani_env *env, const icu::Locale &locale,
                                                                      const icu::UnicodeString &skeleton,
                                                                      ani_object options)
{
    UErrorCode icuStatus = U_ZERO_ERROR;

    std::unique_ptr<icu::DateTimePatternGenerator> dateTimePatternGen(
        icu::DateTimePatternGenerator::createInstance(locale, icuStatus));

    if (U_FAILURE(icuStatus) != 0) {
        ThrowInternalError(env, "DateTimePatternGenerator instance creation failed");
        return nullptr;
    }

    icu::UnicodeString datePattern = dateTimePatternGen->getBestPattern(
        skeleton, UDateTimePatternMatchOptions::UDATPG_MATCH_HOUR_FIELD_LENGTH, icuStatus);

    if (U_FAILURE(icuStatus) != 0) {
        ThrowInternalError(env, "DateFormat pattern creation failed");
        return nullptr;
    }

    ani_ref hourCycleRef = nullptr;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(options, "hourCycle", &hourCycleRef);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to get options.hourCycle field");
        return nullptr;
    }

    auto hourCycle = static_cast<ani_string>(hourCycleRef);
    ani_boolean hourCycleUndefined = ANI_FALSE;
    aniStatus = env->Reference_IsUndefined(hourCycle, &hourCycleUndefined);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "hourCycle reference check failed");
        return nullptr;
    }

    if (hourCycleUndefined != ANI_TRUE) {
        ASSERT(hourCycle != nullptr);
        aniStatus = AlignPatternWithHourCycle(env, &datePattern, hourCycle);
        if (aniStatus != ANI_OK) {
            ThrowInternalError(env, "failed to align DateFormat pattern with hourCycle");
            return nullptr;
        }
    }

    auto icuDateFormat = std::make_unique<icu::SimpleDateFormat>(datePattern, locale, icuStatus);
    if (U_FAILURE(icuStatus) != 0) {
        ThrowInternalError(env, "DateFormat instance creation failed");
        return nullptr;
    }

    return icuDateFormat;
}

static std::unique_ptr<icu::DateFormat> CreateStyleBasedDateFormat(ani_env *env, const icu::Locale &locale,
                                                                   ani_object options)
{
    ani_ref dateStyleRef = nullptr;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(options, "dateStyle", &dateStyleRef);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to get options.dateStyle field");
        return nullptr;
    }

    ani_ref timeStyleRef = nullptr;
    aniStatus = env->Object_GetFieldByName_Ref(options, "timeStyle", &timeStyleRef);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to get options.timeStyle field");
        return nullptr;
    }

    auto dateStyle = static_cast<ani_string>(dateStyleRef);
    auto timeStyle = static_cast<ani_string>(timeStyleRef);

    ani_boolean dateStyleUndefined = ANI_FALSE;
    aniStatus = env->Reference_IsUndefined(dateStyle, &dateStyleUndefined);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "dateStyle reference check failed");
        return nullptr;
    }

    icu::DateFormat::EStyle icuDateStyle = icu::DateFormat::NONE;
    if (dateStyleUndefined != ANI_TRUE) {
        ASSERT(dateStyle != nullptr);
        icuDateStyle = ToICUDateTimeStyle(env, dateStyle, aniStatus);
        ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, nullptr);
    }

    ani_boolean timeStyleUndefined = ANI_FALSE;
    aniStatus = env->Reference_IsUndefined(timeStyle, &timeStyleUndefined);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "timeStyle reference check failed");
        return nullptr;
    }

    icu::DateFormat::EStyle icuTimeStyle = icu::DateFormat::NONE;
    if (timeStyleUndefined != ANI_TRUE) {
        ASSERT(timeStyle != nullptr);
        icuTimeStyle = ToICUDateTimeStyle(env, timeStyle, aniStatus);
        ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, nullptr);
    }

    return std::unique_ptr<icu::DateFormat>(
        icu::DateFormat::createDateTimeInstance(icuDateStyle, icuTimeStyle, locale));
}

static ani_object DateTimeFormatGetOptions(ani_env *env, ani_object self)
{
    ani_ref optionsRef = nullptr;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(self, "options", &optionsRef);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to DateTimeFormat.options field");
        return nullptr;
    }

    return static_cast<ani_object>(optionsRef);
}

static std::unique_ptr<icu::UnicodeString> DateTimeFormatGetPatternSkeleton(ani_env *env, ani_object self)
{
    ani_ref patternRef;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(self, "pattern", &patternRef);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "failed to get options.pattern field");
        return nullptr;
    }

    auto pattern = static_cast<ani_string>(patternRef);

    ani_size patternSize = 0;
    aniStatus = env->String_GetUTF16Size(pattern, &patternSize);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "Failed to get pattern skeleton string size");
        return nullptr;
    }

    ani_size copiedCharsCount = 0;
    std::vector<uint16_t> patternBuf(patternSize + 1);

    aniStatus = env->String_GetUTF16(pattern, patternBuf.data(), patternBuf.size(), &copiedCharsCount);
    ASSERT(copiedCharsCount == patternSize);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "Failed to get pattern skeleton string");
        return nullptr;
    }

    return std::make_unique<icu::UnicodeString>(patternBuf.data(), patternSize);
}

static ani_string StdCoreIntlDateTimeFormatFormatImpl(ani_env *env, ani_object self, ani_double timestamp)
{
    ani_object options = DateTimeFormatGetOptions(env, self);
    if (options == nullptr) {
        return nullptr;
    }

    std::unique_ptr<icu::Locale> icuLocale = ToIcuLocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }

    std::unique_ptr<icu::UnicodeString> patternSkeleton = DateTimeFormatGetPatternSkeleton(env, self);
    if (!patternSkeleton) {
        return nullptr;
    }

    std::unique_ptr<icu::DateFormat> icuDateFormat;
    if (patternSkeleton->isEmpty() == FALSE) {
        icuDateFormat = CreateSkeletonBasedDateFormat(env, *icuLocale, *patternSkeleton, options);
    } else {
        icuDateFormat = CreateStyleBasedDateFormat(env, *icuLocale, options);
    }

    if (!icuDateFormat) {
        // DateFormat creation failed
        return nullptr;
    }

    ani_status aniStatus = DateFormatSetTimeZone(env, icuDateFormat.get(), options);
    if (aniStatus != ANI_OK) {
        if (aniStatus != ANI_PENDING_ERROR) {
            ThrowInternalError(env, "DateFormat time zone initialization failed");
        }

        return nullptr;
    }

    icu::UnicodeString icuFormattedDate;
    icuDateFormat->format(timestamp, icuFormattedDate);

    ani_string formattedDate = nullptr;
    auto formattedDateChars = reinterpret_cast<const uint16_t *>(icuFormattedDate.getBuffer());
    aniStatus = env->String_NewUTF16(formattedDateChars, icuFormattedDate.length(), &formattedDate);
    if (aniStatus != ANI_OK) {
        ThrowInternalError(env, "Formatted date string instance creation failed");
        return nullptr;
    }

    return formattedDate;
}

ani_status RegisterIntlDateTimeFormatMethods(ani_env *env)
{
    ani_class dtfClass;
    ani_status status = env->FindClass("Lstd/core/Intl/DateTimeFormat;", &dtfClass);
    ANI_CHECK_RETURN_IF_NE(status, ANI_OK, status);

    std::array dtfMethods {
        ani_native_function {"formatImpl", "D:Lstd/core/String;",
                             reinterpret_cast<void *>(StdCoreIntlDateTimeFormatFormatImpl)},
        ani_native_function {"getLocaleHourCycle", ":Lstd/core/String;",
                             reinterpret_cast<void *>(StdCoreIntlDateTimeFormatGetLocaleHourCycle)},
    };

    status = env->Class_BindNativeMethods(dtfClass, dtfMethods.data(), dtfMethods.size());
    if (status != ANI_OK) {
        if (status == ANI_ALREADY_BINDED) {
            return ANI_OK;
        }
        return status;
    }

    return ANI_OK;
}
}  // namespace ark::ets::stdlib::intl
