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

#include "IntlCollator.h"
#include "IntlCommon.h"
#include "ani.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "libpandabase/macros.h"
#include "stdlib_ani_helpers.h"
#include <unicode/coll.h>
#include <unicode/locid.h>
#include <unicode/stringpiece.h>
#include <unicode/translit.h>

#include <algorithm>
#include <string>
#include <array>
#include "IntlLocaleMatch.h"

namespace ark::ets::stdlib::intl {

// https://stackoverflow.com/questions/2992066/code-to-strip-diacritical-marks-using-icu
std::string RemoveAccents(ani_env *env, const std::string &str)
{
    // UTF-8 std::string -> UTF-16 UnicodeString
    icu::UnicodeString source = icu::UnicodeString::fromUTF8(icu::StringPiece(str));

    // Transliterate UTF-16 UnicodeString
    UErrorCode status = U_ZERO_ERROR;
    icu::Transliterator *accentsConverter =
        icu::Transliterator::createInstance("NFD; [:M:] Remove; NFC", UTRANS_FORWARD, status);
    accentsConverter->transliterate(source);
    delete accentsConverter;
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowNewError(env, "Lstd/core/RuntimeException;", "Removing accents failed, transliterate failed",
                      "Lstd/core/String;:V");
        return std::string();
    }

    // UTF-16 UnicodeString -> UTF-8 std::string
    std::string result;
    source.toUTF8String(result);

    return result;
}

void RemovePunctuation(std::string &text)
{
    text.erase(std::remove_if(text.begin(), text.end(), ispunct), text.end());
}

ani_string StdCoreIntlCollatorRemoveAccents(ani_env *env, [[maybe_unused]] ani_class klass, ani_string etsText)
{
    std::string text = ConvertFromAniString(env, etsText);
    text = RemoveAccents(env, text);

    ani_boolean unhandledExc;
    ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&unhandledExc));
    if (unhandledExc != 0U) {
        return nullptr;
    }

    return StdStrToAni(env, text);
}

ani_string StdCoreIntlCollatorRemovePunctuation(ani_env *env, [[maybe_unused]] ani_class klass, ani_string etsText)
{
    std::string text = ConvertFromAniString(env, etsText);
    RemovePunctuation(text);
    return StdStrToAni(env, text);
}

ani_double StdCoreIntlCollatorLocaleCmp(ani_env *env, [[maybe_unused]] ani_class klass, ani_string collationIn,
                                        ani_string langIn, ani_string firstStr, ani_string secondStr)
{
    auto collation = ConvertFromAniString(env, collationIn);
    auto lang = ConvertFromAniString(env, langIn);
    auto str1 = ConvertFromAniString(env, firstStr);
    auto str2 = ConvertFromAniString(env, secondStr);

    auto locale = GetLocale(env, lang);
    UErrorCode status = U_ZERO_ERROR;
    icu::StringPiece collationName = "collation";
    icu::StringPiece collationValue = collation.c_str();
    locale.setUnicodeKeywordValue(collationName, collationValue, status);
    if (UNLIKELY(U_FAILURE(status))) {
        const auto errorMessage = std::string("Collation '").append(collation).append("' is invalid or not supported");
        ThrowNewError(env, "Lstd/core/RuntimeException;", errorMessage.c_str(), "Lstd/core/String;:V");
        return 0;
    }

    status = U_ZERO_ERROR;
    std::unique_ptr<icu::Collator> collator(icu::Collator::createInstance(locale, status));
    if (UNLIKELY(U_FAILURE(status))) {
        icu::UnicodeString dispName;
        locale.getDisplayName(dispName);
        std::string localeName;
        dispName.toUTF8String(localeName);
        const auto errorMessage = std::string("Failed to create the collator for ").append(localeName);
        ThrowNewError(env, "Lstd/core/RuntimeException;", errorMessage.c_str(), "Lstd/core/String;:V");
    }

    auto strPiece1 = icu::StringPiece(str1.c_str());
    auto strPiece2 = icu::StringPiece(str2.c_str());
    if ((strPiece1.empty() != 0) && (strPiece2.empty() != 0)) {
        auto res = collator->compareUTF8(strPiece1, strPiece2, status);
        if (UNLIKELY(U_FAILURE(status))) {
            ThrowNewError(env, "Lstd/core/RuntimeException;", "Comparison failed", "Lstd/core/String;:V");
        }
        return res;
    }

    icu::UnicodeString source = StdStrToUnicode(str1);
    icu::UnicodeString target = StdStrToUnicode(str2);
    auto res = collator->compare(source, target, status);
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowNewError(env, "Lstd/core/RuntimeException;", "Comparison failed", "Lstd/core/String;:V");
    }
    return res;
}

ani_status RegisterIntlCollator(ani_env *env)
{
    const auto methods =
        std::array {ani_native_function {"removePunctuation", "Lstd/core/String;:Lstd/core/String;",
                                         reinterpret_cast<void *>(StdCoreIntlCollatorRemovePunctuation)},
                    ani_native_function {"removeAccents", "Lstd/core/String;:Lstd/core/String;",
                                         reinterpret_cast<void *>(StdCoreIntlCollatorRemoveAccents)},
                    ani_native_function {"compareByCollation",
                                         "Lstd/core/String;Lstd/core/String;Lstd/core/String;Lstd/core/String;:D",
                                         reinterpret_cast<void *>(StdCoreIntlCollatorLocaleCmp)}};

    ani_class collatorClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/Collator;", &collatorClass));
    return env->Class_BindNativeMethods(collatorClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
