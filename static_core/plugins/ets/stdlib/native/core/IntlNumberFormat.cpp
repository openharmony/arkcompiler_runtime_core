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

#include "plugins/ets/stdlib/native/core/IntlNumberFormat.h"
#include "libpandabase/macros.h"
#include "libpandabase/os/mutex.h"
#include "unicode/numberformatter.h"
#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/unistr.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <array>

namespace ark::ets::stdlib {

using FormattersCache = std::unordered_map<std::string, icu::number::LocalizedNumberFormatter>;
using FormattersCacheIterator = FormattersCache::iterator;

struct IntlState {
    os::memory::Mutex mtx;
    FormattersCache fmtsCache;
};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::unique_ptr<IntlState> g_intlState {nullptr};

const icu::number::LocalizedNumberFormatter &FormattersCacheInvalidation(EtsEnv *env, ets_string locale)
{
    static auto defaultNumFmt {icu::number::NumberFormatter::withLocale(icu::Locale("en", "US"))};

    const char *localeCharString = env->GetStringUTFChars(locale, nullptr);
    std::string locTag {localeCharString};
    env->ReleaseStringUTFChars(locale, localeCharString);

    icu::StringPiece sp {locTag.data(), static_cast<int32_t>(locTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale loc = icu::Locale::forLanguageTag(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Language tag '" + locTag + std::string("' is invalid or not supported");
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return defaultNumFmt;
    }

    FormattersCacheIterator it;
    {
        os::memory::LockHolder lh(g_intlState->mtx);
        it = g_intlState->fmtsCache.find(locTag);
        if (it == g_intlState->fmtsCache.end()) {
            std::pair<FormattersCacheIterator, bool> newPair =
                g_intlState->fmtsCache.insert({locTag, icu::number::NumberFormatter::withLocale(loc)});
            it = newPair.first;  // newPair has iterator that pointed at new inserted LocalizedNumberFormatter
        }
    }
    return it->second;
}

ets_string IcuFormatDouble(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale, ets_double value)
{
    const icu::number::LocalizedNumberFormatter &formatter = FormattersCacheInvalidation(env, locale);
    UErrorCode status = U_ZERO_ERROR;
    const icu::number::FormattedNumber &fmtNumber = formatter.formatDouble(value, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Unable to format double " + std::to_string(value);
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return nullptr;
    }
    icu::UnicodeString ustr = fmtNumber.toString(status);
    return env->NewString(reinterpret_cast<const uint16_t *>(ustr.getBuffer()), ustr.length());
}

ets_string IcuFormatDecimal(EtsEnv *env, [[maybe_unused]] ets_class klass, ets_string locale, ets_string value)
{
    const char *valueCharString = env->GetStringUTFChars(value, nullptr);
    std::string valueString {valueCharString};
    env->ReleaseStringUTFChars(value, valueCharString);

    const icu::number::LocalizedNumberFormatter &formatter = FormattersCacheInvalidation(env, locale);
    UErrorCode status = U_ZERO_ERROR;
    icu::StringPiece sp {valueString.data(), static_cast<int32_t>(valueString.size())};
    const icu::number::FormattedNumber &fmtNumber = formatter.formatDecimal(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Unable to format BigInt " + valueString;
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), message.c_str());
        return nullptr;
    }
    icu::UnicodeString ustr = fmtNumber.toString(status);
    return env->NewString(reinterpret_cast<const uint16_t *>(ustr.getBuffer()), ustr.length());
}

void CreateDataCoreIntlNumberFormat()
{
    if (g_intlState == nullptr) {
        g_intlState = std::make_unique<IntlState>();
        ASSERT(g_intlState != nullptr);
    }
}

ets_int InitCoreIntlNumberFormat(EtsEnv *env)
{
    // Create internal data
    CreateDataCoreIntlNumberFormat();

    const auto methods =
        std::array {EtsNativeMethod {"formatDouble", nullptr, reinterpret_cast<ets_string *>(IcuFormatDouble)},
                    EtsNativeMethod {"formatDecimal", nullptr, reinterpret_cast<ets_string *>(IcuFormatDecimal)}};

    ets_class numberFormatClass = env->FindClass("std/core/NumberFormat");
    return env->RegisterNatives(numberFormatClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib
