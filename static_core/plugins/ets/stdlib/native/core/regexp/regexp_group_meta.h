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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_GROUP_META_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_GROUP_META_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

namespace ark::ets::stdlib {

class PatternGroupMeta {
public:
    std::vector<bool> &CountableGroups()
    {
        return countableGroups_;
    }

    const std::vector<bool> &CountableGroups() const
    {
        return countableGroups_;
    }

    std::map<size_t, size_t> &ParentGroups()
    {
        return parentGroups_;
    }

    const std::map<size_t, size_t> &ParentGroups() const
    {
        return parentGroups_;
    }

    bool IsEmpty() const
    {
        return countableGroups_.size() <= 1U;
    }

private:
    std::vector<bool> countableGroups_;
    std::map<size_t, size_t> parentGroups_;
};

struct CompiledPattern {
    void *pcre2Code = nullptr;
    PatternGroupMeta groupMeta;
};

using Pcre2Obj = CompiledPattern *;

template <typename CharT>
bool IsUncountableGroupStart(const CharT *pattern, size_t len, size_t index)
{
    uint8_t next = '\0';
    if (index + 1U < len) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        next = static_cast<uint8_t>(pattern[index + 1U]);
    }
    uint8_t next2 = '\0';
    if (index + 2U < len) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        next2 = static_cast<uint8_t>(pattern[index + 2U]);
    }
    uint8_t next3 = '\0';
    if (index + 3U < len) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        next3 = static_cast<uint8_t>(pattern[index + 3U]);
    }
    const bool isLookahead = next == '?' && (next2 == '=' || next2 == '!');
    const bool isLookbehind = next == '?' && next2 == '<' && (next3 == '=' || next3 == '!');
    const bool isAtomicOrNonCapturing = next == '?' && (next2 == ':' || next2 == '>');
    return isLookahead || isLookbehind || isAtomicOrNonCapturing;
}

template <typename CharT>
PatternGroupMeta BuildPatternGroupMeta(const CharT *pattern, size_t len)
{
    PatternGroupMeta meta;
    meta.CountableGroups().push_back(false);
    size_t groupId = 0;
    std::vector<size_t> currentGroups;
    uint8_t prev = '\0';
    uint8_t prev2 = '\0';
    bool inClass = false;
    for (size_t i = 0U; i < len; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto cur = static_cast<uint8_t>(pattern[i]);
        const bool notSupressed = prev != '\\' || prev2 == '\\';
        if (cur == '(' && notSupressed && !inClass) {
            groupId++;
            meta.CountableGroups().push_back(!IsUncountableGroupStart(pattern, len, i));
            currentGroups.push_back(groupId);
            if (currentGroups.size() > 1U) {
                meta.ParentGroups()[groupId] = currentGroups[currentGroups.size() - 2U];
            }
        } else if (cur == ')' && notSupressed && !inClass) {
            if (!currentGroups.empty()) {
                currentGroups.pop_back();
            }
        } else if (cur == '[' && notSupressed) {
            inClass = true;
        } else if (cur == ']' && notSupressed) {
            inClass = false;
        }
        prev2 = prev;
        prev = cur;
    }
    return meta;
}

}  // namespace ark::ets::stdlib

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_ESCOMPAT_REGEXP_GROUP_META_H
