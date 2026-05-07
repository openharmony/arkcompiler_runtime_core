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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_REGEXP_COMMON_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_REGEXP_COMMON_H

#include "plugins/ets/stdlib/native/core/regexp/regexp_exec_result.h"
#include "plugins/ets/stdlib/native/core/regexp/regexp_16.h"
#include "plugins/ets/stdlib/native/core/regexp/regexp_8.h"
#include "plugins/ets/stdlib/native/core/regexp/regexp_executor.h"
#include "plugins/ets/stdlib/native/core/regexp/regexp_string_accessor.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "common_interfaces/objects/utils/utf_utils.h"

#include <ani.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace ark::ets::stdlib {

namespace refs {
extern ani_class g_regexpClass;
extern ani_class g_regexpExecArrayClass;
extern ani_class g_regexpMatchArrayClass;
}  // namespace refs

constexpr const char *LAST_INDEX_FIELD_NAME = "lastIndex";
constexpr const char *IS_CORRECT_FIELD_NAME = "isCorrect";
constexpr int32_t SKIP_ONE_SYM = 1;

struct ExecData {
    ani_string pattern;
    ani_string input;
    ani_string flags;
    int32_t lastIndex;
    size_t patternSize;
    size_t inputSize;
    bool isUtf16Pattern;
    bool isUtf16Input;
    bool requiresUtf16Execution;
};

enum class InputExecutionKind {
    LATIN1_DIRECT,
    UTF16_DIRECT,
    LATIN1_TO_UTF16,
};

void MaterializeAsUtf16InPlace(const RegExpStringAccessor &accessor, std::vector<uint16_t> &out);
const uint16_t *AcquireUtf16Input(ark::ets::ani::ScopedManagedCodeFix &scope, ani_string input, int32_t inputSize,
                                  bool needsMaterialization, std::vector<uint16_t> &storage);
const uint16_t *AcquireUtf16Replacement(ark::ets::ani::ScopedManagedCodeFix &scope, ani_string replaceValue,
                                        std::vector<uint16_t> &storage, ani_int &length);
bool CompileUtf16Pattern(EtsRegExp &re, ark::ets::ani::ScopedManagedCodeFix &scope, const ExecData &execData,
                         std::vector<uint16_t> &storage);

template <typename CodeT>
bool HasCapturingGroupsImpl(const void *pcre2Code);

InputExecutionKind SelectExecutionKind(const EtsRegExp &re, const ExecData &data);
ani_status CreateResultArray(ani_env *env, const std::vector<ani_object> &results, ani_array *out);
ani_status PopulateRegExpResultObject(ani_env *env, ani_class regexpResultClass, ani_object regexpResultObject,
                                      const RegExpExecResult &execResult);
ani_status CreateRegExpResultObject(ani_env *env, ani_class regexpResultClass, ani_method regexpResultCtor,
                                    const RegExpExecResult &execResult, ani_object *out);

inline bool IsUtf16(ani_env *env, ani_string str)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    auto internalString = s.ToInternalType(str);
    return internalString->IsUtf16();
}

inline std::vector<uint16_t> &GetUtf16ScratchA()
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local std::vector<uint16_t> sBufA;
    return sBufA;
}
inline std::vector<uint16_t> &GetUtf16ScratchB()
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local std::vector<uint16_t> sBufB;
    return sBufB;
}

inline ani_status SetIsCorrectField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArrayObj,
                                    bool value)
{
    ani_field resultCorrectField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, IS_CORRECT_FIELD_NAME, &resultCorrectField));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Boolean(regexpExecArrayObj, resultCorrectField, value));
    return ANI_OK;
}

inline ani_status SetLastIndex(ani_env *env, ani_object regexp, ani_field lastIndexField, ani_int value)
{
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(regexp, lastIndexField, value));
    return ANI_OK;
}

inline ani_status FindLastIndexField(ani_env *env, ani_field *out)
{
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_regexpClass, LAST_INDEX_FIELD_NAME, out));
    return ANI_OK;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
inline ExecData MakeExecData(ani_env *env, ani_string pattern, ani_string str, ani_string flags, ani_int patternSize,
                             ani_int strSize, ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    return {pattern,
            str,
            flags,
            static_cast<int32_t>(lastIndex),
            static_cast<size_t>(patternSize),
            static_cast<size_t>(strSize),
            IsUtf16(env, pattern),
            IsUtf16(env, str),
            static_cast<bool>(requiresUtf16Execution)};
}

template <typename ResultT, typename Latin1Fn, typename Utf16Fn>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ResultT PrepareInputAndRun(EtsRegExp &re, const ExecData &execData, ani_env *env, InputExecutionKind executionKind,
                           Latin1Fn &&latin1Fn, Utf16Fn &&utf16Fn)
{
    switch (executionKind) {
        case InputExecutionKind::LATIN1_DIRECT: {
            const uint8_t *inputData = nullptr;
            {
                ark::ets::ani::ScopedManagedCodeFix scope(env);
                auto *patternEtsStr = scope.ToInternalType(execData.pattern);
                RegExpStringAccessor patternAccessor(patternEtsStr);
                if (!re.Compile(patternAccessor.GetDataUtf8(), static_cast<int>(execData.patternSize))) {
                    return ResultT {};
                }

                auto *inputEtsStr = scope.ToInternalType(execData.input);
                RegExpStringAccessor inputAccessor(inputEtsStr);
                inputData = inputAccessor.GetDataUtf8();
            }
            return std::forward<Latin1Fn>(latin1Fn)(inputData);
        }
        case InputExecutionKind::UTF16_DIRECT:
        case InputExecutionKind::LATIN1_TO_UTF16: {
            const uint16_t *inputData = nullptr;
            std::vector<uint16_t> inputStorage;
            {
                ark::ets::ani::ScopedManagedCodeFix scope(env);
                const bool needsMaterialization = executionKind == InputExecutionKind::LATIN1_TO_UTF16;
                inputData = AcquireUtf16Input(scope, execData.input, static_cast<int32_t>(execData.inputSize),
                                              needsMaterialization, inputStorage);
                auto &patternUtf16 = GetUtf16ScratchA();
                if (!CompileUtf16Pattern(re, scope, execData, patternUtf16)) {
                    return ResultT {};
                }
            }
            return std::forward<Utf16Fn>(utf16Fn)(inputData);
        }
        default:
            UNREACHABLE();
    }
}

template <typename CharT>
int32_t AdvanceIndex(const CharT *data, int32_t size, int32_t pos, bool unicode)
{
    if constexpr (std::is_same_v<CharT, uint8_t>) {
        (void)data;
        (void)size;
        (void)unicode;
        return pos + 1;
    } else {
        if (!unicode || pos + 1 >= size) {
            return pos + 1;
        }
        const auto indexAdv = pos + 1;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint16_t hi = data[pos];
        if (common_vm::UtfUtils::IsUTF16HighSurrogate(hi)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            uint16_t lo = data[indexAdv];
            if (common_vm::UtfUtils::IsUTF16LowSurrogate(lo)) {
                return indexAdv + SKIP_ONE_SYM;
            }
        }
        return indexAdv;
    }
}

template <typename CharT>
RegExpExecResult ExecuteOnce(EtsRegExp &re, uint32_t matchFlags, const CharT *input, int32_t inputSize, int32_t lastEnd)
{
    if constexpr (std::is_same_v<CharT, uint8_t>) {
        auto *compiled = re.GetCompiledRe8();
        auto result = RegExp8::Execute(compiled, matchFlags, input, inputSize, lastEnd);
        RegExp8::ApplyGroupMeta(compiled->groupMeta, result);
        return result;
    } else {
        auto *compiled = re.GetCompiledRe16();
        auto result = RegExp16::Execute(compiled, matchFlags, input, inputSize, lastEnd);
        RegExp16::ApplyGroupMeta(compiled->groupMeta, result);
        return result;
    }
}

}  // namespace ark::ets::stdlib

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_REGEXP_COMMON_H
