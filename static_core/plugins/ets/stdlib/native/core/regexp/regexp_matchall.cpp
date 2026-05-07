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

#include "regexp_matchall.h"

#include <vector>

namespace ark::ets::stdlib {

namespace {

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array RunMatchAllLoop(EtsRegExp &re, uint32_t matchFlags, const CharT *input, int32_t inputSize, bool unicode,
                          ani_env *env, ani_class resultClass, ani_method resultCtor, int32_t lastIndex)
{
    std::vector<ani_object> results;
    if (lastIndex < 0) {
        lastIndex = 0;
    }
    if (lastIndex > inputSize) {
        ani_array arr = nullptr;
        ani_status status = CreateResultArray(env, results, &arr);
        if (status != ANI_OK) {
            return nullptr;
        }
        return arr;
    }

    while (lastIndex <= inputSize) {
        auto execResult = ExecuteOnce(re, matchFlags, input, inputSize, lastIndex);
        if (!execResult.isSuccess) {
            break;
        }

        ani_object resultObj = nullptr;
        ani_status status = CreateRegExpResultObject(env, resultClass, resultCtor, execResult, &resultObj);
        if (status != ANI_OK) {
            return nullptr;
        }
        results.push_back(resultObj);
        lastIndex = execResult.endIndex;
        if (execResult.index == execResult.endIndex) {
            lastIndex = AdvanceIndex(input, inputSize, lastIndex, unicode);
        }
    }

    ani_array arr = nullptr;
    ani_status status = CreateResultArray(env, results, &arr);
    if (status != ANI_OK) {
        return nullptr;
    }
    return arr;
}

}  // namespace

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_array MatchAllNativeImpl(ani_env *env, [[maybe_unused]] ani_object regexp, ani_string pattern, ani_string flags,
                             ani_string str, ani_int patternSize, ani_int strSize, ani_int lastIndex,
                             ani_boolean requiresUtf16Execution)
{
    ExecData execData = MakeExecData(env, pattern, str, flags, patternSize, strSize, lastIndex, requiresUtf16Execution);
    const auto inputSize = static_cast<int32_t>(strSize);

    EtsRegExp re(env);
    re.SetFlags(ConvertFromAniString(env, execData.flags));

    ani_method regexpMatchArrayCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpMatchArrayClass, "<ctor>", ":", &regexpMatchArrayCtor));

    const InputExecutionKind executionKind = SelectExecutionKind(re, execData);
    const uint32_t matchFlags = re.GetMatchFlags();
    auto result = PrepareInputAndRun<ani_array>(
        re, execData, env, executionKind,
        [&](const uint8_t *inputData) -> ani_array {
            return RunMatchAllLoop<uint8_t>(re, matchFlags, inputData, inputSize, false, env,
                                            refs::g_regexpMatchArrayClass, regexpMatchArrayCtor, execData.lastIndex);
        },
        [&](const uint16_t *inputData) -> ani_array {
            return RunMatchAllLoop<uint16_t>(re, matchFlags, inputData, inputSize, re.HasUnicodeOrUnicodeSetsFlag(),
                                             env, refs::g_regexpMatchArrayClass, regexpMatchArrayCtor,
                                             execData.lastIndex);
        });

    if (re.HasCompiledRe()) {
        re.Destroy();
    }

    return result;
}

}  // namespace ark::ets::stdlib
