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

#include "regexp_16.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 16
#include "pcre2.h"

#include "plugins/ets/runtime/ets_exceptions.h"

#include <utility>

namespace ark::ets {

constexpr int PCRE2_MATCH_DATA_UNIT_WIDTH = 2;
constexpr int PCRE2_CHARACTER_WIDTH = 2;
constexpr int PCRE2_GROUPS_NAME_ENTRY_SHIFT = 4;

Pcre2Obj RegExp16::CreatePcre2Object(const uint16_t *patternStr, uint32_t flags, uint32_t extraFlags, const int len)
{
    int errorNumber;
    PCRE2_SPTR pattern = static_cast<PCRE2_SPTR>(patternStr);
    PCRE2_SIZE errorOffset;
    auto *compileContext = pcre2_compile_context_create(nullptr);
    pcre2_set_compile_extra_options(compileContext, extraFlags);
    auto re = pcre2_compile(pattern, len, flags, &errorNumber, &errorOffset, compileContext);
    pcre2_compile_context_free(compileContext);
    return reinterpret_cast<Pcre2Obj>(re);
}

RegExpExecResult RegExp16::Execute(Pcre2Obj re, const uint16_t *str, int len, const int startOffset)
{
    auto *expr = reinterpret_cast<pcre2_code *>(re);
    auto *matchData = pcre2_match_data_create_from_pattern(expr, nullptr);
    PandaVector<std::pair<bool, PandaString>> captures;
    PandaVector<std::pair<uint32_t, uint32_t>> indices;
    auto resultCount = pcre2_match(expr, str, len, startOffset, 0, matchData, nullptr);
    auto *ovector = pcre2_get_ovector_pointer(matchData);

    RegExpExecResult result;
    result.isWide = true;
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
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto res = PandaString(reinterpret_cast<const char *>(str + substringStart),
                               (substringEnd - substringStart) * PCRE2_CHARACTER_WIDTH);
        captures.push_back({true, res});
    }

    int nameCount;
    pcre2_pattern_info(expr, PCRE2_INFO_NAMECOUNT, &nameCount);

    if (nameCount > 0) {
        RegExp16::ExtractGroups(re, nameCount, result, reinterpret_cast<void *>(ovector));
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

void RegExp16::ExtractGroups(Pcre2Obj expression, int count, RegExpExecResult &result, void *data)
{
    PCRE2_SPTR nameTable;
    PCRE2_SPTR tabPtr;
    int nameEntrySize;

    auto *expr = reinterpret_cast<pcre2_code *>(expression);
    auto *ovector = reinterpret_cast<PCRE2_SIZE *>(data);

    pcre2_pattern_info(expr, PCRE2_INFO_NAMETABLE, &nameTable);
    pcre2_pattern_info(expr, PCRE2_INFO_NAMEENTRYSIZE, &nameEntrySize);

    tabPtr = nameTable;
    for (int currentNameId = 0; currentNameId < count; currentNameId++) {
        auto n = static_cast<int32_t>(tabPtr[0]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto index = static_cast<int32_t>(ovector[PCRE2_CHARACTER_WIDTH * n]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto endIndex = static_cast<int32_t>(ovector[PCRE2_CHARACTER_WIDTH * n + 1]);
        auto tabConstCharPtr = reinterpret_cast<const char *>(tabPtr + 1);
        size_t size = nameEntrySize * PCRE2_CHARACTER_WIDTH - PCRE2_GROUPS_NAME_ENTRY_SHIFT;
        while (size > 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (static_cast<uint8_t>(*(tabConstCharPtr + size - PCRE2_CHARACTER_WIDTH)) != 0) {
                break;
            }
            size -= PCRE2_CHARACTER_WIDTH;
        }
        auto key16 = PandaString(tabConstCharPtr, size);
        PandaString key;
        key.reserve(key16.size() / PCRE2_CHARACTER_WIDTH);
        for (size_t i = 0; i < key16.size(); i += PCRE2_CHARACTER_WIDTH) {
            key += key16[i];
        }
        result.namedGroups[key] = {index, endIndex};
        tabPtr += nameEntrySize;
    }
}

void RegExp16::FreePcre2Object(Pcre2Obj re)
{
    pcre2_code_free(reinterpret_cast<pcre2_code *>(re));
}

}  // namespace ark::ets
