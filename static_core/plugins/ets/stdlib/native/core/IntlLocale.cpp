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
#include "plugins/ets/stdlib/native/core/IntlLocale.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include <unicode/locid.h>
#include <unicode/numsys.h>
#include <unicode/localpointer.h>
#include "libpandabase/macros.h"

#include <cassert>
#include <set>
#include <array>
#include <sstream>
#include <vector>
#include <algorithm>

namespace ark::ets::stdlib::intl {

enum LocaleInfo { LANG, SCRIPT, COUNTRY, U_CA, U_KF, U_CO, U_HC, U_NU, U_KN, COUNT };

struct LocaleCheckInfo {
    std::set<std::string> langs;
    std::set<std::string> regions;
    std::set<std::string> scripts;
};

static LocaleCheckInfo &GetLocaleCheckInfo()
{
    static LocaleCheckInfo gCheckInfo;
    return gCheckInfo;
}

static std::vector<std::string> &GetLocaleInfo()
{
    static std::vector<std::string> gLocaleInfo(LocaleInfo::COUNT, "");
    std::fill(gLocaleInfo.begin(), gLocaleInfo.end(), "");
    return gLocaleInfo;
}

void StdCoreIntlLocaleFillCheckInfo()
{
    int32_t availableCount;
    const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(availableCount);

    for (int i = 0; i < availableCount; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto &locale = availableLocales[i];
        const std::string lang = locale.getLanguage();
        const std::string region = locale.getCountry();
        const std::string script = locale.getScript();
        if (!lang.empty()) {
            GetLocaleCheckInfo().langs.insert(lang);
        }
        if (!region.empty()) {
            GetLocaleCheckInfo().regions.insert(region);
        }
        if (!script.empty()) {
            GetLocaleCheckInfo().scripts.insert(script);
        }
    }
}

std::string Set2String(std::set<std::string> &inSet)
{
    const int lastCommaLength = 1;
    std::string outList;
    for (const auto &item : inSet) {
        outList += item;
        outList += ",";
    }
    if (outList.length() > lastCommaLength) {
        outList.erase(outList.length() - lastCommaLength);
    }
    return outList;
}

ani_string StdCoreIntlLocaleDefaultLang(ani_env *env, [[maybe_unused]] ani_class klass)
{
    const std::string lang = icu::Locale::getDefault().getLanguage();
    return StdStrToAni(env, lang);
}

ani_string StdCoreIntlLocaleRegionList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    std::string regionsList = Set2String(GetLocaleCheckInfo().regions);
    return StdStrToAni(env, regionsList);
}

ani_string StdCoreIntlLocaleLangList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    std::string langsList = Set2String(GetLocaleCheckInfo().langs);
    return StdStrToAni(env, langsList);
}

ani_string StdCoreIntlLocaleScriptList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    std::string scriptList = Set2String(GetLocaleCheckInfo().scripts);
    return StdStrToAni(env, scriptList);
}

ani_string StdCoreIntlLocaleNumberingSystemList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    UErrorCode success = U_ZERO_ERROR;
    std::unique_ptr<icu::StringEnumeration> numberingSystemList(icu::NumberingSystem::getAvailableNames(success));
    if (UNLIKELY(U_FAILURE(success))) {
        ASSERT(true);
    }
    std::string nsList;
    while (const icu::UnicodeString *ns = numberingSystemList->snext(success)) {
        if (UNLIKELY(U_FAILURE(success))) {
            ASSERT(true);
        }
        std::string str;
        ns->toUTF8String(str);
        nsList += str;
    }

    return StdStrToAni(env, nsList);
}

ani_string StdCoreIntlLocaleInfo(ani_env *env, [[maybe_unused]] ani_class klass, ani_string lang)
{
    std::string langKey = ConvertFromAniString(env, lang);

    UErrorCode success = U_ZERO_ERROR;
    auto l = icu::Locale::forLanguageTag(langKey, success);
    if (UNLIKELY(U_FAILURE(success))) {
        ASSERT(true);
    }
    l.addLikelySubtags(success);
    if (UNLIKELY(U_FAILURE(success))) {
        ASSERT(true);
    }

    std::string result;
    result += l.getCountry();
    result += ",";
    result += l.getScript();

    return StdStrToAni(env, result);
}

ani_int StdCoreIntlLocaleIsTagValid(ani_env *env, [[maybe_unused]] ani_class klass, ani_string tag)
{
    std::string tagKey = ConvertFromAniString(env, tag);

    UErrorCode success = U_ZERO_ERROR;
    icu::Locale::forLanguageTag(tagKey, success);
    if (UNLIKELY(U_FAILURE(success))) {
        return 1;
    }
    return 0;
}

static int UnicodeKeyToLocaleInfo(const std::string &unicodeKey)
{
    int keyIndex = -1;
    if (unicodeKey == "ca") {
        keyIndex = LocaleInfo::U_CA;
    } else if (unicodeKey == "co") {
        keyIndex = LocaleInfo::U_CO;
    } else if (unicodeKey == "kf") {
        keyIndex = LocaleInfo::U_KF;
    } else if (unicodeKey == "hc") {
        keyIndex = LocaleInfo::U_HC;
    } else if (unicodeKey == "nu") {
        keyIndex = LocaleInfo::U_NU;
    } else if (unicodeKey == "kn") {
        keyIndex = LocaleInfo::U_KN;
    }
    return keyIndex;
}

ani_ref StdCoreIntlLocaleParseTag(ani_env *env, [[maybe_unused]] ani_class klass, ani_string tag)
{
    std::vector<std::string> data = GetLocaleInfo();
    std::string tagKey = ConvertFromAniString(env, tag);
    UErrorCode success = U_ZERO_ERROR;

    auto l = icu::Locale::forLanguageTag(tagKey, success);
    if (UNLIKELY(U_FAILURE(success))) {
        std::string message = "Failed to find locale from tag";
        ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
        return nullptr;
    }

    data[LocaleInfo::LANG] = l.getLanguage();
    data[LocaleInfo::SCRIPT] = l.getScript();
    data[LocaleInfo::COUNTRY] = l.getCountry();

    std::set<std::string> keyWordsSet;
    l.getUnicodeKeywords<std::string>(std::insert_iterator<decltype(keyWordsSet)>(keyWordsSet, keyWordsSet.begin()),
                                      success);

    for (auto &unicodeKey : keyWordsSet) {
        auto unicodeKeywordValue = l.getUnicodeKeywordValue<std::string>(unicodeKey, success);
        if (UNLIKELY(U_FAILURE(success))) {
            std::string message = "Failed to getUnicodeKeywordValue";
            ThrowNewError(env, "Lstd/core/RuntimeException;", message.c_str(), "Lstd/core/String;:V");
            return nullptr;
        }
        int keyIndex = UnicodeKeyToLocaleInfo(unicodeKey);
        if (keyIndex >= LocaleInfo::U_CA && keyIndex < LocaleInfo::COUNT) {
            data[keyIndex] = unicodeKeywordValue;
        }
    }

    std::stringstream s;
    for (int i = 0; i < LocaleInfo::COUNT; ++i) {
        s << (data[i]) << " ";
    }
    std::string res = s.str();
    if (res.length() > 1) {
        res.resize(res.length() - 1);
    }

    return StdStrToAni(env, res);
}

ani_status RegisterIntlLocaleNativeMethods(ani_env *env)
{
    const auto methods = std::array {
        ani_native_function {"initLocale", ":V", reinterpret_cast<void *>(StdCoreIntlLocaleFillCheckInfo)},
        ani_native_function {"maximizeInfo", "Lstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(StdCoreIntlLocaleInfo)},
        ani_native_function {"regionList", ":Lstd/core/String;", reinterpret_cast<void *>(StdCoreIntlLocaleRegionList)},
        ani_native_function {"langList", ":Lstd/core/String;", reinterpret_cast<void *>(StdCoreIntlLocaleLangList)},
        ani_native_function {"scriptList", ":Lstd/core/String;", reinterpret_cast<void *>(StdCoreIntlLocaleScriptList)},
        ani_native_function {"numberingSystemList", ":Lstd/core/String;",
                             reinterpret_cast<void *>(StdCoreIntlLocaleNumberingSystemList)},
        ani_native_function {"defaultLang", ":Lstd/core/String;",
                             reinterpret_cast<void *>(StdCoreIntlLocaleDefaultLang)},
        ani_native_function {"isTagValid", "Lstd/core/String;:I",
                             reinterpret_cast<void *>(StdCoreIntlLocaleIsTagValid)},
        ani_native_function {"parseTagImpl", "Lstd/core/String;:Lstd/core/String;",
                             reinterpret_cast<void *>(StdCoreIntlLocaleParseTag)}};

    ani_class localeClass;
    ANI_FATAL_IF_ERROR(env->FindClass("Lstd/core/Intl/Locale;", &localeClass));
    return env->Class_BindNativeMethods(localeClass, methods.data(), methods.size());
}

ani_string StdCoreIntlLocaleDefaultBaseName(ani_env *env, [[maybe_unused]] ani_class klass)
{
    const std::string name = icu::Locale::getDefault().getBaseName();
    return StdStrToAni(env, name);
}

}  // namespace ark::ets::stdlib::intl
