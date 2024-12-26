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

#include "plugins/ets/stdlib/native/core/IntlLocaleMatch.h"
#include "libpandabase/macros.h"
#include "unicode/locid.h"
#include "unicode/localematcher.h"

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

ets_string StdCoreIntlBestFitLocale(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale)
{
    UErrorCode success = U_ZERO_ERROR;
    const char *localeCharString = env->GetStringUTFChars(locale, nullptr);
    ASSERT(localeCharString != nullptr);
    std::string locTag {localeCharString};
    env->ReleaseStringUTFChars(locale, localeCharString);
    icu::StringPiece str = icu::StringPiece(BuildLocaleMatcher(success)
                                                .getBestMatchForListString(locTag, success)
                                                ->toLanguageTag<std::string>(success)
                                                .c_str());
    if (UNLIKELY(U_FAILURE(success))) {
        std::string message = "Unable to find bestfit match for" + locTag;
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return nullptr;
    }
    auto res =
        env->NewString(reinterpret_cast<const uint16_t *>(icu::UnicodeString::fromUTF8(str).getBuffer()), str.length());
    ASSERT(res != nullptr);
    return res;
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

ets_string StdCoreIntlLookupLocale(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_objectArray locales)
{
    UErrorCode success = U_ZERO_ERROR;
    ets_string locale;
    int32_t availableCount;
    std::string bestLoc;
    const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(availableCount);
    if (UNLIKELY(U_FAILURE(success))) {
        return env->NewString(nullptr, 0);
    }
    int32_t len = env->GetArrayLength(locales);
    for (int32_t i = 0; i < len; i++) {
        locale = reinterpret_cast<ets_string>(env->GetObjectArrayElement(locales, i));
        const char *localeCharString = env->GetStringUTFChars(locale, nullptr);
        std::string locTag {localeCharString};
        env->ReleaseStringUTFChars(locale, localeCharString);
        bestLoc = LookupLocale(locTag, availableLocales, availableCount);
        if (!bestLoc.empty()) {
            auto str = icu::StringPiece(bestLoc.c_str());
            auto res = env->NewString(reinterpret_cast<const uint16_t *>(icu::UnicodeString::fromUTF8(str).getBuffer()),
                                      str.length());
            ASSERT(res != nullptr);
            return res;
        }
    }
    icu::StringPiece str = icu::StringPiece(icu::Locale::getDefault().toLanguageTag<std::string>(success).c_str());
    auto res =
        env->NewString(reinterpret_cast<const uint16_t *>(icu::UnicodeString::fromUTF8(str).getBuffer()), str.length());
    ASSERT(res != nullptr);
    return res;
}

ets_int RegisterIntlLocaleMatch(EtsEnv *env)
{
    const auto methods =
        std::array {EtsNativeMethod {"bestFitLocale", nullptr, reinterpret_cast<void *>(StdCoreIntlBestFitLocale)},
                    EtsNativeMethod {"lookupLocale", nullptr, reinterpret_cast<void *>(StdCoreIntlLookupLocale)}};

    ets_class localeMatchClass = env->FindClass("std/core/LocaleMatch");
    return env->RegisterNatives(localeMatchClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib