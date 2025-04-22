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

#include "regexp_8.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

#include <utility>

namespace ark::ets {

constexpr int PCRE2_MATCH_DATA_UNIT_WIDTH = 2;

Pcre2Obj RegExp8::CreatePcre2Object(const uint8_t *patternStr, uint32_t flags, const int len)
{
    int errorNumber;
    PCRE2_SPTR pattern = static_cast<PCRE2_SPTR>(patternStr);
    PCRE2_SIZE errorOffset;
    auto re = pcre2_compile(pattern, len, flags, &errorNumber, &errorOffset, nullptr);
    return reinterpret_cast<Pcre2Obj>(re);
}

ark::RegExpMatchResult<PandaString> RegExp8::Execute(Pcre2Obj re, const uint8_t *str, const int len,
                                                     const int startOffset)
{
    auto *expr = reinterpret_cast<pcre2_code *>(re);
    auto *matchData = pcre2_match_data_create_from_pattern(expr, nullptr);
    PandaVector<std::pair<bool, PandaString>> captures;
    PandaVector<std::pair<uint32_t, uint32_t>> indices;
    auto resultCount = pcre2_match(expr, str, len, startOffset, 0, matchData, nullptr);
    auto *ovector = pcre2_get_ovector_pointer(matchData);
    RegExpMatchResult<PandaString> result;
    result.isWide = false;
    if (resultCount < 0) {
        result.isSuccess = false;
        pcre2_match_data_free(matchData);
        return result;
    }

    const auto lastIndex = resultCount * PCRE2_MATCH_DATA_UNIT_WIDTH;
    for (decltype(resultCount) i = 0; i < lastIndex; i += PCRE2_MATCH_DATA_UNIT_WIDTH) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto substringStart = ovector[i];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto substringEnd = ovector[i + 1];
        indices.emplace_back(
            std::make_pair(static_cast<uint32_t>(substringStart), static_cast<uint32_t>(substringEnd)));
        PandaString res;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        res = PandaString(reinterpret_cast<const char *>(str + substringStart), substringEnd - substringStart);
        captures.push_back({true, res});
    }

    result.isSuccess = true;
    result.captures = std::move(captures);
    result.indices = std::move(indices);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result.index = ovector[0];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result.endIndex = ovector[1];
    int groupCount = pcre2_get_ovector_count(matchData);
    while (static_cast<int>(result.captures.size()) < groupCount) {
        result.captures.push_back({false, PandaString()});
        result.indices.push_back({0, 0});
    }
    pcre2_match_data_free(matchData);
    return result;
}

void RegExp8::FreePcre2Object(Pcre2Obj re)
{
    pcre2_code_free(reinterpret_cast<pcre2_code *>(re));
}

}  // namespace ark::ets
