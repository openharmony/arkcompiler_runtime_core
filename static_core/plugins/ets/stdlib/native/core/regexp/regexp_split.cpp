/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "regexp_split.h"
#include "plugins/ets/stdlib/native/core/regexp/regexp_common.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"

#include <algorithm>
#include <type_traits>

namespace ark::ets::stdlib {
namespace {

constexpr size_t BOUNDARY_PAIR_SIZE = 2U;
using Boundaries = std::vector<int32_t>;  // Flat array: [start0, end0, start1, end1, ...]

template <typename T>
ani_status CreateSplitSubstring(ani_env *env, const T *inputData, int32_t start, int32_t end, ani_string *out)
{
    const auto len = static_cast<ani_size>(end - start);
    if constexpr (std::is_same_v<T, uint8_t>) {
        *out = CreateUtf8String(env, reinterpret_cast<const char *>(std::next(inputData, start)), len);
    } else {
        *out = CreateUtf16String(env, std::next(inputData, start), len);
    }
    return (*out == nullptr) ? ANI_OUT_OF_MEMORY : ANI_OK;
}

template <typename T>
ani_status CreateSplitResultArray(ani_env *env, const Boundaries &boundaries, const T *inputData, int32_t inputSize,
                                  ani_array *out)
{
    ani_ref undefined {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ANI_RETURN_ON_PENDING_ERROR(
        env->Array_New(static_cast<ani_size>(boundaries.size() / BOUNDARY_PAIR_SIZE), undefined, out));
    for (ani_size i = 0; i < boundaries.size(); i += BOUNDARY_PAIR_SIZE) {
        const int32_t start = boundaries[i];
        const int32_t end = boundaries[i + 1];
        if (start == -1 && end == -1) {
            continue;
        }
        if (start < 0 || end < 0 || start > end || end > inputSize) {
            return ANI_OK;
        }
        ani_string substr = nullptr;
        ani_status status = CreateSplitSubstring(env, inputData, start, end, &substr);
        if (status != ANI_OK) {
            return status;
        }
        ANI_FATAL_IF_ERROR(env->Array_Set(*out, i / 2U, substr));
    }
    return ANI_OK;
}

bool AppendBoundary(Boundaries &boundaries, ani_long limit, int32_t start, int32_t end)
{
    boundaries.push_back(start);
    boundaries.push_back(end);
    return limit > 0 && static_cast<int64_t>(boundaries.size() / BOUNDARY_PAIR_SIZE) >= limit;
}

bool AppendCapturedBoundaries(Boundaries &boundaries, ani_long limit, const RegExpExecResult &execResult)
{
    for (size_t i = 1; i < execResult.indices.size(); ++i) {
        const auto &[first, second] = execResult.indices[i];
        if (AppendBoundary(boundaries, limit, first, second)) {
            return true;
        }
    }
    return false;
}

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
bool AppendSplitBoundary(const CharT *input, int32_t inputSize, bool unicode, ani_long limit, Boundaries &boundaries,
                         int32_t &lastStart, int32_t &lastEnd, const RegExpExecResult &execResult, bool hasCaptures)
{
    const int32_t separatorRight = std::min<int32_t>(execResult.endIndex, inputSize);
    if (separatorRight == lastStart) {
        lastEnd = AdvanceIndex(input, inputSize, lastEnd, unicode);
        return false;
    }
    if (AppendBoundary(boundaries, limit, lastStart, lastEnd) ||
        (hasCaptures && AppendCapturedBoundaries(boundaries, limit, execResult))) {
        return true;
    }
    lastStart = separatorRight;
    lastEnd = lastStart;
    return false;
}

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
void RunSplitLoop(EtsRegExp &re, uint32_t matchFlags, const CharT *input, int32_t inputSize, bool unicode,
                  ani_long limit, Boundaries &boundaries)
{
    const bool hasCaptures = [&re] {
        if constexpr (std::is_same_v<CharT, uint8_t>) {
            return RegExp8::HasCapturingGroups(re.GetCompiledRe8());
        } else {
            return RegExp16::HasCapturingGroups(re.GetCompiledRe16());
        }
    }();
    int32_t lastStart = 0;
    int32_t lastEnd = 0;
    while (lastEnd < inputSize) {
        auto execResult = ExecuteOnce(re, matchFlags, input, inputSize, lastEnd);
        if (!execResult.isSuccess) {
            lastEnd = AdvanceIndex(input, inputSize, lastEnd, unicode);
            continue;
        }
        if (AppendSplitBoundary(input, inputSize, unicode, limit, boundaries, lastStart, lastEnd, execResult,
                                hasCaptures)) {
            return;
        }
    }
    (void)AppendBoundary(boundaries, limit, lastStart, inputSize);
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array RunSplitNativeImpl(EtsRegExp &re, const ExecData &execData, int32_t inputSize, bool unicode, ani_long limit,
                             ani_env *env, uint32_t matchFlags, InputExecutionKind executionKind)
{
    Boundaries boundaries;
    auto runner = [&re, matchFlags, inputSize, unicode, limit, &boundaries, env](const auto *inputData) -> ani_array {
        using CharT = std::remove_const_t<std::remove_pointer_t<decltype(inputData)>>;
        RunSplitLoop<CharT>(re, matchFlags, inputData, inputSize, std::is_same_v<CharT, uint16_t> && unicode, limit,
                            boundaries);
        ani_array arr = nullptr;
        ani_status status = CreateSplitResultArray(env, boundaries, inputData, inputSize, &arr);
        if (status != ANI_OK) {
            return nullptr;
        }
        return arr;
    };
    auto result = PrepareInputAndRun<ani_array>(re, execData, env, executionKind, runner, runner);
    return result;
}

}  // namespace

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array SplitNativeImpl(ani_env *env, [[maybe_unused]] ani_object regexp, ani_string pattern, ani_string flags,
                          ani_string str, ani_int patternSize, ani_int strSize, ani_long limit,
                          ani_boolean unicodeMatching, ani_boolean requiresUtf16Execution)
{
    ExecData execData = MakeExecData(env, pattern, str, flags, patternSize, strSize, 0, requiresUtf16Execution);
    EtsRegExp re(env);
    re.SetFlags(ConvertFromAniString(env, execData.flags));
    const auto executionKind = SelectExecutionKind(re, execData);
    const uint32_t matchFlags = re.GetMatchFlags();
    ani_array result = RunSplitNativeImpl(re, execData, static_cast<int32_t>(strSize),
                                          static_cast<bool>(unicodeMatching), limit, env, matchFlags, executionKind);
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    return result;
}

}  // namespace ark::ets::stdlib
