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
#include "napi/ets_napi.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "libpandabase/macros.h"
#include <unicode/coll.h>
#include <unicode/locid.h>

#include <unicode/translit.h>

#include <algorithm>
#include <string>
#include <array>
#include <sstream>

namespace ark::ets::stdlib::intl {

enum InputData { COLLATION, LANG, STR1, STR2, COUNT };

// https://stackoverflow.com/questions/2992066/code-to-strip-diacritical-marks-using-icu
std::string RemoveAccents(EtsEnv *env, const std::string &str)
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
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"),
                           "Removing accents failed, transliterate failed");
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

ets_string StdCoreIntlCollatorRemoveAccents(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string etsText)
{
    std::string text = EtsToStdStr(env, etsText);
    text = RemoveAccents(env, text);
    auto str = icu::StringPiece(text.c_str());
    return env->NewString(reinterpret_cast<const uint16_t *>(icu::UnicodeString::fromUTF8(str).getBuffer()),
                          str.length());
}

ets_string StdCoreIntlCollatorRemovePunctuation(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string etsText)
{
    std::string text = EtsToStdStr(env, etsText);
    RemovePunctuation(text);
    auto str = icu::StringPiece(text.c_str());
    return env->NewString(reinterpret_cast<const uint16_t *>(icu::UnicodeString::fromUTF8(str).getBuffer()),
                          str.length());
}

ets_double StdCoreIntlCollatorLocaleCmp([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                        std::array<ets_string, InputData::COUNT> data)
{
    ets_string collation = data[InputData::COLLATION];
    ets_string langTag = data[InputData::LANG];
    ets_string str1 = data[InputData::STR1];
    ets_string str2 = data[InputData::STR2];

    std::string lang = EtsToStdStr(env, langTag);
    UErrorCode status;
    icu::Locale locale = icu::Locale::forLanguageTag(lang, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::stringstream s;
        s << "Language tag '" << lang << "' is invalid or not supported";
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), s.str().c_str());
        return 0;
    }

    const auto collationStr = EtsToStdStr(env, collation);
    icu::StringPiece collationName = "collation";
    icu::StringPiece collationValue = collationStr.c_str();
    locale.setUnicodeKeywordValue(collationName, collationValue, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::stringstream s;
        s << "Collation '" << collationStr << "' is invalid or not supported";
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), s.str().c_str());
        return 0;
    }

    icu::UnicodeString source = EtsToUnicodeStr(env, str1);
    icu::UnicodeString target = EtsToUnicodeStr(env, str2);

    status = U_ZERO_ERROR;
    std::unique_ptr<icu::Collator> collator(icu::Collator::createInstance(locale, status));
    if (UNLIKELY(U_FAILURE(status))) {
        icu::UnicodeString dispName;
        locale.getDisplayName(dispName);
        std::string localeName;
        dispName.toUTF8String(localeName);
        std::stringstream s;
        s << "Failed to create the collator for " << localeName;
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), s.str().c_str());
    }
    return collator->compare(source, target);
}

ets_int RegisterIntlCollator(EtsEnv *env)
{
    const auto methods = std::array {
        EtsNativeMethod {"removePunctuation", nullptr,
                         reinterpret_cast<ets_string *>(StdCoreIntlCollatorRemovePunctuation)},
        EtsNativeMethod {"removeAccents", nullptr, reinterpret_cast<ets_string *>(StdCoreIntlCollatorRemoveAccents)},
        EtsNativeMethod {"compareByCollation", nullptr, reinterpret_cast<ets_double *>(StdCoreIntlCollatorLocaleCmp)},
    };

    ets_class collatorClass = env->FindClass("std/core/Intl/Collator");
    return env->RegisterNatives(collatorClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl