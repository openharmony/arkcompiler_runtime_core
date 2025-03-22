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

#include "plugins/ets/stdlib/native/core/IntlLocaleMatch.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "libpandabase/macros.h"
#include "unicode/locid.h"
#include "unicode/localematcher.h"
#include "stdlib_ani_helpers.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <array>

namespace ark::ets::stdlib {

icu::LocaleMatcher BuildLocaleMatcher(UErrorCode &success)
{
    int32_t count;
    const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(count);
    icu::Locale defaultLocale = icu::Locale::getDefault();
    icu::LocaleMatcher::Builder builder;
    builder.setDefaultLocale(&defaultLocale);
    for (int32_t i = 0; i < count; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        builder.addSupportedLocale(availableLocales[i]);
    }
    return builder.build(success);
}

ani_string StdCoreIntlBestFitLocale(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale)
{
    UErrorCode success = U_ZERO_ERROR;
    auto locTag = ConvertFromAniString(env, locale);
    auto str =
        BuildLocaleMatcher(success).getBestMatchForListString(locTag, success)->toLanguageTag<std::string>(success);
    if (UNLIKELY(U_FAILURE(success))) {
        std::string message = "Unable to find bestfit match for" + locTag;
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return nullptr;
    }
    return intl::StdStrToAni(env, str);
}

std::string LookupLocale(const std::string &locTag, const icu::Locale *availableLocales, const int32_t count)
{
    UErrorCode success = U_ZERO_ERROR;
    auto locP = icu::StringPiece(locTag.c_str());
    icu::Locale requestedLoc = icu::Locale::forLanguageTag(locP, success);
    if (UNLIKELY(U_FAILURE(success))) {
        return std::string();
    }
    for (int32_t i = 0; i < count; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (LIKELY(availableLocales[i] == requestedLoc)) {
            return requestedLoc.toLanguageTag<std::string>(success);
        }
    }
    return std::string();
}

ani_string StdCoreIntlLookupLocale(ani_env *env, [[maybe_unused]] ani_class klass, ani_array_ref locales)
{
    UErrorCode success = U_ZERO_ERROR;
    ani_ref locale;
    int32_t availableCount;
    std::string bestLoc;
    const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(availableCount);
    if (UNLIKELY(U_FAILURE(success))) {
        return CreateUtf8String(env, "", 0);
    }

    ani_size len;
    ANI_FATAL_IF_ERROR(env->Array_GetLength(locales, &len));

    for (ani_size i = 0; i < len; i++) {
        ANI_FATAL_IF_ERROR(env->Array_Get_Ref(locales, i, &locale));

        auto locTag = ConvertFromAniString(env, reinterpret_cast<ani_string>(locale));

        bestLoc = LookupLocale(locTag, availableLocales, availableCount);
        if (!bestLoc.empty()) {
            return intl::StdStrToAni(env, bestLoc);
        }
    }
    auto str = icu::Locale::getDefault().toLanguageTag<std::string>(success);
    return intl::StdStrToAni(env, str);
}

ani_status RegisterIntlLocaleMatch(ani_env *env)
{
    const auto methods = std::array {ani_native_function {"bestFitLocale", "Lstd/core/String;:Lstd/core/String;",
                                                          reinterpret_cast<void *>(StdCoreIntlBestFitLocale)},
                                     ani_native_function {"lookupLocale", "[Lstd/core/String;:Lstd/core/String;",
                                                          reinterpret_cast<void *>(StdCoreIntlLookupLocale)}};

    ani_class localeMatchClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/LocaleMatch;", &localeMatchClass));

    return env->Class_BindNativeMethods(localeMatchClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib
