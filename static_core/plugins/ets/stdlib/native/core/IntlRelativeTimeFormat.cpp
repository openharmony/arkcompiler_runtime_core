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

#include <unicode/unistr.h>
#include <unicode/locid.h>
#include <unicode/reldatefmt.h>
#include <unicode/udat.h>
#include <unicode/localematcher.h>
#include <set>
#include <algorithm>
#include <array>
#include "libpandabase/macros.h"
#include "plugins/ets/stdlib/native/core/IntlRelativeTimeFormat.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/IntlLocaleMatch.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "ani/ani_checkers.h"

namespace ark::ets::stdlib::intl {

static std::string CallOptionGetter(ani_env *env, ani_object options, const char *getterName)
{
    ani_class optionsClass = nullptr;
    if (env->FindClass("Lstd/core/Intl/RelativeTimeFormatOptionsImpl;", &optionsClass) != ANI_OK ||
        optionsClass == nullptr) {
        return "";
    }

    ani_method getter = nullptr;
    if (env->Class_FindGetter(optionsClass, getterName, &getter) != ANI_OK) {
        return "";
    }

    ani_ref resultRef = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (env->Object_CallMethod_Ref(options, getter, &resultRef) != ANI_OK || resultRef == nullptr) {
        return "";
    }

    auto resultStr = static_cast<ani_string>(resultRef);
    ani_size len = 0;
    env->String_GetUTF8Size(resultStr, &len);
    std::vector<char> buf(len + 1);
    ani_size copied = 0;
    env->String_GetUTF8(resultStr, buf.data(), buf.size(), &copied);
    return std::string(buf.data());
}

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
    ani_class errorClass = nullptr;
    if (env->FindClass("Lstd/core/RangeError;", &errorClass) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    ani_method ctor;
    if (env->Class_FindMethod(errorClass, "<ctor>", "Lstd/core/String;:V", &ctor) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    ani_string msgStr;
    if (env->String_NewUTF8(message.c_str(), message.length(), &msgStr) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    ani_object err;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (env->Object_New(errorClass, ctor, &err, msgStr) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    return env->ThrowError(static_cast<ani_error>(err));
}

static ani_status ThrowInternalError(ani_env *env, const std::string &message)
{
    return ThrowError(env, "Lstd/core/InternalError;", message);
}

static std::unique_ptr<icu::Locale> ToIcuLocale(ani_env *env, ani_object self)
{
    ani_ref localeRef = nullptr;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(self, "locale", &localeRef);
    if (aniStatus != ANI_OK || localeRef == nullptr) {
        return std::make_unique<icu::Locale>(icu::Locale::getDefault());
    }

    auto localeStr = static_cast<ani_string>(localeRef);

    ani_boolean isUndefined = ANI_FALSE;
    aniStatus = env->Reference_IsUndefined(localeStr, &isUndefined);
    if (aniStatus != ANI_OK || isUndefined == ANI_TRUE) {
        return std::make_unique<icu::Locale>(icu::Locale::getDefault());
    }

    ani_size strSize = 0;
    env->String_GetUTF8Size(localeStr, &strSize);
    std::vector<char> buf(strSize + 1);
    ani_size copied = 0;
    env->String_GetUTF8(localeStr, buf.data(), buf.size(), &copied);

    icu::Locale locale(buf.data());
    if (locale.isBogus() != 0) {
        ThrowRangeError(env, "Invalid locale: " + std::string(buf.data()));
        return nullptr;
    }

    return std::make_unique<icu::Locale>(locale);
}

static URelativeDateTimeUnit ToICUUnitOrThrow(ani_env *env, const std::string &unitStr)
{
    std::string unit = unitStr;
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);

    if (!unit.empty() && unit.back() == 's') {
        unit.pop_back();
    }

    if (unit == "second") {
        return UDAT_REL_UNIT_SECOND;
    }
    if (unit == "minute") {
        return UDAT_REL_UNIT_MINUTE;
    }
    if (unit == "hour") {
        return UDAT_REL_UNIT_HOUR;
    }
    if (unit == "day") {
        return UDAT_REL_UNIT_DAY;
    }
    if (unit == "week") {
        return UDAT_REL_UNIT_WEEK;
    }
    if (unit == "month") {
        return UDAT_REL_UNIT_MONTH;
    }
    if (unit == "quarter") {
        return UDAT_REL_UNIT_QUARTER;
    }
    if (unit == "year") {
        return UDAT_REL_UNIT_YEAR;
    }
    ThrowNewError(env, "Lstd/core/RuntimeException;", ("Invalid unit: " + unitStr).c_str(), "Lstd/core/String;:V");
    return static_cast<URelativeDateTimeUnit>(-1);
}

static ani_object StdCoreIntlRelativeTimeFormatResolvedOptionsImpl(ani_env *env, ani_object self)
{
    std::unique_ptr<icu::Locale> icuLocale = ToIcuLocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }
    UErrorCode status = U_ZERO_ERROR;
    auto langTagStr = icuLocale->toLanguageTag<std::string>(status);
    if (U_FAILURE(status) != 0) {
        ThrowInternalError(env, "failed to get locale lang tag: " + std::string(u_errorName(status)));
        return nullptr;
    }

    std::string numberingSystemStr = "latn";
    std::string numericStr = "always";
    std::string styleStr = "long";

    ani_ref optionsRef = nullptr;
    if (env->Object_GetFieldByName_Ref(self, "options", &optionsRef) == ANI_OK && optionsRef != nullptr) {
        auto options = static_cast<ani_object>(optionsRef);

        std::string maybeNumeric = CallOptionGetter(env, options, "numeric");
        std::string maybeStyle = CallOptionGetter(env, options, "style");

        if (!maybeNumeric.empty()) {
            numericStr = maybeNumeric;
        }
        if (!maybeStyle.empty()) {
            styleStr = maybeStyle;
        }
    }

    ani_string locale = CreateUtf8String(env, langTagStr.c_str(), langTagStr.length());
    ani_string numberingSystem = CreateUtf8String(env, numberingSystemStr.c_str(), numberingSystemStr.length());
    ani_string numeric = CreateUtf8String(env, numericStr.c_str(), numericStr.length());
    ani_string style = CreateUtf8String(env, styleStr.c_str(), styleStr.length());

    ani_class optsCls = nullptr;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/ResolvedRelativeTimeFormatOptionsImpl;", &optsCls));
    ani_method ctor = nullptr;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(optsCls, "<ctor>", "Lstd/core/String;Lstd/core/String;:V", &ctor));

    ani_object resolvedOpts = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(optsCls, ctor, &resolvedOpts, locale, numberingSystem));

    ani_method numericSetter = nullptr;
    ANI_FATAL_IF_ERROR(env->Class_FindSetter(optsCls, "numeric", &numericSetter));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_CallMethod_Void(resolvedOpts, numericSetter, numeric));

    ani_method styleSetter = nullptr;
    ANI_FATAL_IF_ERROR(env->Class_FindSetter(optsCls, "style", &styleSetter));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_CallMethod_Void(resolvedOpts, styleSetter, style));

    return resolvedOpts;
}

static ani_string StdCoreIntlRelativeTimeFormatFormatImpl(ani_env *env, ani_object self, ani_double value,
                                                          ani_string unit)
{
    std::unique_ptr<icu::Locale> icuLocale = ToIcuLocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::RelativeDateTimeFormatter formatter(*icuLocale, status);
    if (U_FAILURE(status) != 0) {
        return nullptr;
    }
    ani_size unitSize = 0;
    env->String_GetUTF8Size(unit, &unitSize);
    std::vector<char> unitBuf(unitSize + 1);
    ani_size copiedChars = 0;
    env->String_GetUTF8(unit, unitBuf.data(), unitBuf.size(), &copiedChars);

    URelativeDateTimeUnit icuUnit = ToICUUnitOrThrow(env, std::string(unitBuf.data()));

    icu::UnicodeString result;
    formatter.format(value, icuUnit, result, status);
    if (U_FAILURE(status) != 0) {
        return nullptr;
    }
    ani_string output;
    env->String_NewUTF16(reinterpret_cast<const uint16_t *>(result.getBuffer()), result.length(), &output);
    return output;
}

ani_status RegisterIntlRelativeTimeFormatMethods(ani_env *env)
{
    std::array methods = {
        ani_native_function {"formatImpl", "DLstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(StdCoreIntlRelativeTimeFormatFormatImpl)},
        ani_native_function {"resolvedOptionsImpl", ":Lstd/core/Intl/ResolvedRelativeTimeFormatOptions;",
                             reinterpret_cast<void *>(StdCoreIntlRelativeTimeFormatResolvedOptionsImpl)}};
    ani_class rtfClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/RelativeTimeFormat;", &rtfClass));

    return env->Class_BindNativeMethods(rtfClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
