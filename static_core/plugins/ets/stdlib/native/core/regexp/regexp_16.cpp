/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "regexp_common.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 16
#include "pcre2.h"

#include "plugins/ets/runtime/ets_exceptions.h"

#include <memory>
#include <utility>

namespace ark::ets::stdlib {

namespace {

constexpr uint32_t EXTRA_FLAGS_MASK = PCRE2_EXTRA_ALT_BSUX;
constexpr uint32_t PREALLOC_MDATA_GROUPS = 32U;

pcre2_compile_context *GetOrCreateCompileCtx16()
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local pcre2_compile_context *sCCtx = nullptr;
    if (sCCtx == nullptr) {
        sCCtx = pcre2_compile_context_create(nullptr);
        if (sCCtx != nullptr) {
            pcre2_set_newline(sCCtx, PCRE2_NEWLINE_ANY);
        }
    }
    return sCCtx;
}

struct TlsMatchData16 {
    pcre2_match_data *data = nullptr;
    bool inUse = false;
};

TlsMatchData16 &GetTlsMatchData16()
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local TlsMatchData16 sMdata;
    if (sMdata.data == nullptr) {
        sMdata.data = pcre2_match_data_create(PREALLOC_MDATA_GROUPS + 1U, nullptr);
    }
    return sMdata;
}

class MatchDataScope16 {
public:
    MatchDataScope16(TlsMatchData16 *tls, pcre2_match_data *data, bool usePrealloc)
        : tls_(tls), data_(data), usePrealloc_(usePrealloc)
    {
    }

    MatchDataScope16(const MatchDataScope16 &) = delete;
    MatchDataScope16 &operator=(const MatchDataScope16 &) = delete;
    MatchDataScope16(MatchDataScope16 &&) = delete;
    MatchDataScope16 &operator=(MatchDataScope16 &&) = delete;

    ~MatchDataScope16()
    {
        if (usePrealloc_ && tls_ != nullptr) {
            tls_->inUse = false;
        } else if (data_ != nullptr) {
            pcre2_match_data_free(data_);
        }
    }

    pcre2_match_data *Get() const
    {
        return data_;
    }

private:
    TlsMatchData16 *tls_;
    pcre2_match_data *data_;
    bool usePrealloc_;
};

struct MatchDataHandle16 {
    TlsMatchData16 *tls = nullptr;
    pcre2_match_data *data = nullptr;
    bool usePrealloc = false;
};

MatchDataHandle16 AcquireMatchData16(pcre2_code *expr)
{
    auto &tlsMdata = GetTlsMatchData16();
    uint32_t captureCount = 0U;
    if (pcre2_pattern_info(expr, PCRE2_INFO_CAPTURECOUNT, &captureCount) == 0 && tlsMdata.data != nullptr &&
        !tlsMdata.inUse && captureCount <= PREALLOC_MDATA_GROUPS) {
        tlsMdata.inUse = true;
        return {&tlsMdata, tlsMdata.data, true};
    }

    return {&tlsMdata, pcre2_match_data_create_from_pattern(expr, nullptr), false};
}

MatchDataHandle16 AcquireTestMatchData16()
{
    auto &tlsMdata = GetTlsMatchData16();
    if (!tlsMdata.inUse && tlsMdata.data != nullptr) {
        tlsMdata.inUse = true;
        return {&tlsMdata, tlsMdata.data, true};
    }

    return {&tlsMdata, pcre2_match_data_create(1U, nullptr), false};
}

}  // namespace

constexpr int PCRE2_MATCH_DATA_UNIT_WIDTH = 2;
constexpr int PCRE2_CHARACTER_WIDTH = 2;
constexpr int PCRE2_GROUPS_NAME_ENTRY_SHIFT = 4;

Pcre2Obj RegExp16::CreatePcre2Object(const uint16_t *pattern, uint32_t flags, uint32_t extraFlags, const int len)
{
    int errorNumber;
    PCRE2_SPTR patternStr = static_cast<PCRE2_SPTR>(pattern);
    PCRE2_SIZE errorOffset;
    auto *compileContext = GetOrCreateCompileCtx16();
    if (compileContext == nullptr) {
        return nullptr;
    }
    pcre2_set_compile_extra_options(compileContext, extraFlags | EXTRA_FLAGS_MASK);
    auto re = pcre2_compile(patternStr, len, flags, &errorNumber, &errorOffset, compileContext);
    if (re == nullptr) {
        return nullptr;
    }
    auto *compiled = new CompiledPattern();
    compiled->pcre2Code = re;
    compiled->groupMeta = BuildPatternGroupMeta(pattern, static_cast<size_t>(len));
    return compiled;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
bool RegExp16::CompileAndTest(const uint16_t *pattern, int patternLen, uint32_t compileFlags, uint32_t extraFlags,
                              const uint16_t *input, int inputLen, int startOffset, uint32_t matchFlags,
                              int32_t &endIndex)
{
    int errorNumber;
    PCRE2_SIZE errorOffset;
    auto *compileContext = GetOrCreateCompileCtx16();
    if (compileContext == nullptr) {
        return false;
    }
    pcre2_set_compile_extra_options(compileContext, extraFlags | EXTRA_FLAGS_MASK);
    auto *code = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(pattern), patternLen, compileFlags, &errorNumber,
                               &errorOffset, compileContext);
    if (code == nullptr) {
        return false;
    }

    std::unique_ptr<pcre2_code, decltype(&pcre2_code_free)> codeGuard(code, &pcre2_code_free);

    auto matchDataHandle = AcquireTestMatchData16();
    auto *matchData = matchDataHandle.data;
    if (matchData == nullptr) {
        return false;
    }
    MatchDataScope16 matchDataScope(matchDataHandle.tls, matchData, matchDataHandle.usePrealloc);
    const auto resultCount = pcre2_match(code, input, inputLen, startOffset, matchFlags, matchData, nullptr);
    if (resultCount < 0) {
        return false;
    }

    auto *ovector = pcre2_get_ovector_pointer(matchData);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    endIndex = static_cast<int32_t>(ovector[1]);
    return true;
}

RegExpExecResult RegExp16::Execute(Pcre2Obj re, uint32_t matchFlags, const uint16_t *str, int len,
                                   const int startOffset)
{
    auto *expr = reinterpret_cast<pcre2_code *>(re->pcre2Code);
    auto matchDataHandle = AcquireMatchData16(expr);
    auto *matchData = matchDataHandle.data;
    if (matchData == nullptr) {
        RegExpExecResult result;
        result.isWide = true;
        result.isSuccess = false;
        return result;
    }
    MatchDataScope16 matchDataScope(matchDataHandle.tls, matchData, matchDataHandle.usePrealloc);
    std::vector<std::pair<bool, std::string>> captures;
    std::vector<std::pair<int32_t, int32_t>> indices;
    auto resultCount = pcre2_match(expr, str, len, startOffset, matchFlags, matchData, nullptr);

    RegExpExecResult result;
    result.isWide = true;
    if (resultCount < 0) {
        result.isSuccess = false;
        return result;
    }
    auto *ovector = pcre2_get_ovector_pointer(matchData);
    const auto lastIndex = resultCount * PCRE2_MATCH_DATA_UNIT_WIDTH;
    for (decltype(resultCount) i = 0; i < lastIndex; i += PCRE2_MATCH_DATA_UNIT_WIDTH) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto substringStart = ovector[i];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto substringEnd = ovector[i + 1];
        indices.emplace_back(std::make_pair(substringStart, substringEnd));
    }

    int nameCount;
    pcre2_pattern_info(expr, PCRE2_INFO_NAMECOUNT, &nameCount);

    if (nameCount > 0) {
        RegExp16::ExtractGroups(re, nameCount, result, reinterpret_cast<void *>(ovector));
    }

    result.isSuccess = true;
    result.indices = std::move(indices);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result.index = ovector[0];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result.endIndex = ovector[1];
    auto groupCount = static_cast<size_t>(pcre2_get_ovector_count(matchData));
    while (result.indices.size() < groupCount) {
        result.indices.emplace_back(std::make_pair(-1, -1));
    }
    return result;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
bool RegExp16::TestMatch(Pcre2Obj re, uint32_t matchFlags, const uint16_t *str, int len, const int startOffset,
                         int32_t &endIndex)
{
    auto *expr = reinterpret_cast<pcre2_code *>(re->pcre2Code);
    auto matchDataHandle = AcquireTestMatchData16();
    auto *matchData = matchDataHandle.data;
    if (matchData == nullptr) {
        return false;
    }
    MatchDataScope16 matchDataScope(matchDataHandle.tls, matchData, matchDataHandle.usePrealloc);
    auto resultCount = pcre2_match(expr, str, len, startOffset, matchFlags, matchData, nullptr);
    if (resultCount < 0) {
        return false;
    }

    auto *ovector = pcre2_get_ovector_pointer(matchData);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    endIndex = static_cast<int32_t>(ovector[1]);
    return true;
}

void RegExp16::ExtractGroups(Pcre2Obj expression, int count, RegExpExecResult &result, void *data)
{
    PCRE2_SPTR nameTable;
    PCRE2_SPTR tabPtr;
    int nameEntrySize;

    auto *expr = reinterpret_cast<pcre2_code *>(expression->pcre2Code);
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
        auto key16 = std::string(tabConstCharPtr, size);
        std::string key;
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
    if (re == nullptr) {
        return;
    }
    pcre2_code_free(reinterpret_cast<pcre2_code *>(re->pcre2Code));
    delete re;
}

bool RegExp16::HasCapturingGroups(Pcre2Obj re)
{
    ASSERT(re != nullptr);
    ASSERT(re->pcre2Code != nullptr);
    return HasCapturingGroupsImpl<pcre2_code_16>(re->pcre2Code);
}

void RegExp16::SanitizeGroupCaptureResults(const std::vector<bool> &countableGroups,
                                           const std::map<size_t, size_t> &parentGroups, RegExpExecResult &result)
{
    auto groupId = 0U;
    auto countedGroupId = groupId;
    std::map<size_t, size_t> groupToCountableGroup;
    for (size_t i = 1U; i < countableGroups.size(); i++) {
        groupId++;
        if (countableGroups[i]) {
            countedGroupId++;
            groupToCountableGroup[groupId] = countedGroupId;
        }
    }

    for (const auto [childIndex, parentIndex] : parentGroups) {
        if (childIndex >= countableGroups.size() || parentIndex >= countableGroups.size()) {
            continue;
        }
        if (!countableGroups[childIndex] || !countableGroups[parentIndex]) {
            continue;
        }
        const auto countedChildIndex = groupToCountableGroup[childIndex];
        const auto countedResultIndex = groupToCountableGroup[parentIndex];
        if (countedChildIndex >= result.indices.size() || countedResultIndex >= result.indices.size()) {
            continue;
        }
        const auto &child = result.indices[countedChildIndex];
        const auto &parent = result.indices[countedResultIndex];
        if (child.first < parent.first || child.second > parent.second) {
            result.indices[countedChildIndex].first = -1;
            result.indices[countedChildIndex].second = -1;
        }
    }
}

void RegExp16::ApplyGroupMeta(const PatternGroupMeta &groupMeta, RegExpExecResult &result)
{
    if (result.indices.empty()) {
        return;
    }
    size_t ecmaCaptures = 0U;
    const auto &countableGroups = groupMeta.CountableGroups();
    for (size_t i = 1U; i < countableGroups.size(); ++i) {
        ecmaCaptures += static_cast<size_t>(countableGroups[i]);
    }
    const auto expectedIndicesSize = ecmaCaptures + 1U;
    if (result.indices.size() > expectedIndicesSize) {
        result.indices.resize(expectedIndicesSize);
    } else if (result.indices.size() < expectedIndicesSize) {
        result.indices.resize(expectedIndicesSize, std::make_pair(-1, -1));
    }
    if (result.indices.size() < 2U || groupMeta.IsEmpty()) {
        return;
    }
    SanitizeGroupCaptureResults(countableGroups, groupMeta.ParentGroups(), result);
}

}  // namespace ark::ets::stdlib
