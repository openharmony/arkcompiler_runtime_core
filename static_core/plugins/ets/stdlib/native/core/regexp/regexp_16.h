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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_STD_CORE_REGEXP_REGEXP_16_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_STD_CORE_REGEXP_REGEXP_16_H

#include "plugins/ets/stdlib/native/core/regexp/regexp_group_meta.h"
#include "plugins/ets/stdlib/native/core/regexp/regexp_exec_result.h"

#include <cstdint>

namespace ark::ets::stdlib {

class RegExp16 {
public:
    static Pcre2Obj CreatePcre2Object(const uint16_t *pattern, uint32_t flags, uint32_t extraFlags, const int len);
    static bool CompileAndTest(const uint16_t *pattern, int patternLen, uint32_t compileFlags, uint32_t extraFlags,
                               const uint16_t *input, int inputLen, int startOffset, uint32_t matchFlags,
                               int32_t &endIndex);
    static RegExpExecResult Execute(Pcre2Obj re, uint32_t matchFlags, const uint16_t *str, int len,
                                    const int startOffset);
    // CC-OFFNXT(G.FUN.01, huge_method) solid logic
    static bool TestMatch(Pcre2Obj re, uint32_t matchFlags, const uint16_t *str, int len, const int startOffset,
                          int32_t &endIndex);
    static void ExtractGroups(Pcre2Obj expression, int count, RegExpExecResult &result, void *data);
    static void FreePcre2Object(Pcre2Obj re);
    static bool HasCapturingGroups(Pcre2Obj re);
    static void ApplyGroupMeta(const PatternGroupMeta &groupMeta, RegExpExecResult &result);
    static void SanitizeGroupCaptureResults(const std::vector<bool> &countableGroups,
                                            const std::map<size_t, size_t> &parentGroups, RegExpExecResult &result);
};

}  // namespace ark::ets::stdlib
#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_STD_CORE_REGEXP_REGEXP_16_H
