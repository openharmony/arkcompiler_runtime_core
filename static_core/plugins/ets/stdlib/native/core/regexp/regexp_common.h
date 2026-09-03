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
                                  std::vector<uint16_t> &storage);
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

// Validates an ani_int size argument against the actual length of the managed string.
// ani_int is signed: a negative value cast to size_t becomes huge, and a mismatched positive
// value would cause heap over-read in vector::assign/resize or inside PCRE2. Trust boundary:
// sizes passed from the managed layer must not be used without verification.
inline ani_status ValidateStringSize(ani_env *env, ani_string str, ani_int size)
{
    ark::ets::ani::ScopedManagedCodeFix scope(env);
    const auto actualLength = static_cast<uint32_t>(scope.ToInternalType(str)->GetLength());
    if (size < 0 || static_cast<uint32_t>(size) != actualLength) {
        return ThrowNewError(env, "std.core.RuntimeError", "invalid string size argument");
    }
    return ANI_OK;
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
inline ani_status MakeExecData(ExecData *out, ani_env *env, ani_string pattern, ani_string str, ani_string flags,
                               ani_int patternSize, ani_int strSize, ani_int lastIndex,
                               ani_boolean requiresUtf16Execution)
{
    ani_status status = ValidateStringSize(env, pattern, patternSize);
    if (status != ANI_OK) {
        return status;
    }
    status = ValidateStringSize(env, str, strSize);
    if (status != ANI_OK) {
        return status;
    }
    *out = {pattern,
            str,
            flags,
            static_cast<int32_t>(lastIndex),
            static_cast<size_t>(patternSize),
            static_cast<size_t>(strSize),
            IsUtf16(env, pattern),
            IsUtf16(env, str),
            static_cast<bool>(requiresUtf16Execution)};
    return ANI_OK;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
inline ani_status MakeTestExecData(ExecData *out, ani_env *env, ani_string pattern, ani_string str, ani_int patternSize,
                                   ani_int strSize, ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    ani_status status = ValidateStringSize(env, pattern, patternSize);
    if (status != ANI_OK) {
        return status;
    }
    status = ValidateStringSize(env, str, strSize);
    if (status != ANI_OK) {
        return status;
    }
    *out = {pattern,
            str,
            {},
            static_cast<int32_t>(lastIndex),
            static_cast<size_t>(patternSize),
            static_cast<size_t>(strSize),
            IsUtf16(env, pattern),
            IsUtf16(env, str),
            static_cast<bool>(requiresUtf16Execution)};
    return ANI_OK;
}

// How the caller's fn is executed, i.e. how its raw input pointer is kept GC-safe:
//  - RUN_IN_MANAGED_SCOPE: fn must be pure PCRE2/native code (no ANI calls, no managed
//    allocations).
//  - MATERIALIZE: fn uses ANI APIs / managed allocations (e.g. replace/split result
//    construction). The input is copied to native memory and fn runs after the scope ends:
//    a nested scope from ANI calls is only allowed in NATIVE state.
enum class InputRunMode {
    RUN_IN_MANAGED_SCOPE,
    MATERIALIZE,
};

template <typename ResultT, typename Latin1Fn, typename Utf16Fn>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ResultT PrepareInputAndRun(EtsRegExp &re, const ExecData &execData, ani_env *env, InputExecutionKind executionKind,
                           InputRunMode runMode, Latin1Fn &&latin1Fn, Utf16Fn &&utf16Fn)
{
    switch (executionKind) {
        case InputExecutionKind::LATIN1_DIRECT: {
            std::vector<uint8_t> inputStorage;
            {
                ark::ets::ani::ScopedManagedCodeFix scope(env);
                auto *patternEtsStr = scope.ToInternalType(execData.pattern);
                RegExpStringAccessor patternAccessor(patternEtsStr);
                if (!re.Compile(patternAccessor.GetDataUtf8(), static_cast<int>(execData.patternSize))) {
                    return ResultT {};
                }

                auto *inputEtsStr = scope.ToInternalType(execData.input);
                RegExpStringAccessor inputAccessor(inputEtsStr);
                if (!inputAccessor.HasFlattenedData()) {
                    // OOM during flatten: pending exception is already raised, propagate the failure
                    return ResultT {};
                }
                if (runMode == InputRunMode::RUN_IN_MANAGED_SCOPE) {
                    // Zero-copy fast path: the pure-PCRE2 fn is invoked still inside the
                    // scope. The RUNNING state blocks STW GC and fn performs no managed
                    // allocations / ANI calls, so the raw pointer cannot dangle.
                    return std::forward<Latin1Fn>(latin1Fn)(inputAccessor.GetDataUtf8());
                }
                const uint8_t *inputData = inputAccessor.GetDataUtf8();
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                inputStorage.assign(inputData, inputData + execData.inputSize);
            }
            // MATERIALIZE: the fn makes ANI calls / managed allocations and must run in
            // NATIVE state with the materialized copy.
            return std::forward<Latin1Fn>(latin1Fn)(inputStorage.data());
        }
        case InputExecutionKind::UTF16_DIRECT:
        case InputExecutionKind::LATIN1_TO_UTF16: {
            std::vector<uint16_t> inputStorage;
            {
                ark::ets::ani::ScopedManagedCodeFix scope(env);
                auto &patternUtf16 = GetUtf16ScratchA();
                if (!CompileUtf16Pattern(re, scope, execData, patternUtf16)) {
                    return ResultT {};
                }
                // Pattern flattening above may have triggered GC; acquire the input only
                // after the last potential allocation point.
                if (runMode == InputRunMode::RUN_IN_MANAGED_SCOPE &&
                    executionKind == InputExecutionKind::UTF16_DIRECT) {
                    // Zero-copy fast path: raw UTF-16 input + pure-PCRE2 fn inside the scope.
                    RegExpStringAccessor inputAccessor(scope.ToInternalType(execData.input));
                    if (!inputAccessor.HasFlattenedData()) {
                        return ResultT {};
                    }
                    return std::forward<Utf16Fn>(utf16Fn)(inputAccessor.GetDataUtf16());
                }
                AcquireUtf16Input(scope, execData.input, static_cast<int32_t>(execData.inputSize), inputStorage);
                if (runMode == InputRunMode::RUN_IN_MANAGED_SCOPE) {
                    // LATIN1_TO_UTF16 transcoded input (native memory) + pure-PCRE2 fn:
                    // safe to invoke inside the scope as well.
                    return std::forward<Utf16Fn>(utf16Fn)(inputStorage.data());
                }
            }
            // MATERIALIZE: run in NATIVE state with the materialized copy.
            return std::forward<Utf16Fn>(utf16Fn)(inputStorage.data());
        }
        default:
            UNREACHABLE();
    }
}

inline InputExecutionKind SelectExecutionKind(bool forceUtf16, const ExecData &data)
{
    const bool canUseLatin1Direct = !data.isUtf16Input && !data.isUtf16Pattern && !forceUtf16;
    if (canUseLatin1Direct) {
        return InputExecutionKind::LATIN1_DIRECT;
    }
    if (data.isUtf16Input) {
        return InputExecutionKind::UTF16_DIRECT;
    }
    return InputExecutionKind::LATIN1_TO_UTF16;
}

template <typename CharT>
int32_t AdvanceIndex(const CharT *data, int32_t size, int32_t pos, bool unicode)
{
    if constexpr (std::is_same_v<CharT, uint8_t>) {
        (void)data;
        (void)size;
        (void)unicode;
        // Saturate: pos + 1 is unrepresentable when pos == INT32_MAX (maximal-length input)
        return pos < INT32_MAX ? pos + 1 : pos;
    } else {
        // Overflow-safe form of the previous "pos + 1 >= size" guard: with pos == size ==
        // INT32_MAX (a ~4GiB UTF-16 input), pos + 1 overflows int32, wraps negative and
        // bypasses the bounds check, causing out-of-range reads of data[] below.
        if (!unicode || pos < 0 || pos >= size - 1) {
            return pos < INT32_MAX ? pos + 1 : pos;
        }
        const auto indexAdv = pos + 1;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint16_t hi = data[pos];
        if (ark::mem::UtfUtils::IsUTF16HighSurrogate(hi)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            uint16_t lo = data[indexAdv];
            if (ark::mem::UtfUtils::IsUTF16LowSurrogate(lo)) {
                return indexAdv + SKIP_ONE_SYM;
            }
        }
        return indexAdv;
    }
}

template <typename CharT>
RegExpExecResult ExecuteOnce(EtsRegExp &re, uint32_t matchFlags, const CharT *input, int32_t inputSz, int32_t lastEnd)
{
    if constexpr (std::is_same_v<CharT, uint8_t>) {
        auto *compiled = re.GetCompiledRe8();
        auto result = RegExp8::Execute(compiled, matchFlags, input, inputSz, lastEnd);
        RegExp8::ApplyGroupMeta(compiled->groupMeta, result);
        return result;
    } else {
        auto *compiled = re.GetCompiledRe16();
        auto result = RegExp16::Execute(compiled, matchFlags, input, inputSz, lastEnd);
        RegExp16::ApplyGroupMeta(compiled->groupMeta, result);
        return result;
    }
}

template <typename ResultT, typename Latin1Fn, typename Utf16Fn>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ResultT PreparePatternAndInputAndRunTest(const ExecData &execData, ani_env *env, InputExecutionKind executionKind,
                                         Latin1Fn &&latin1Fn, Utf16Fn &&utf16Fn)
{
    switch (executionKind) {
        case InputExecutionKind::LATIN1_DIRECT: {
            ark::ets::ani::ScopedManagedCodeFix scope(env);
            auto *patternEtsStr = scope.ToInternalType(execData.pattern);
            RegExpStringAccessor patternAccessor(patternEtsStr);
            if (!patternAccessor.HasFlattenedData()) {
                // OOM during flatten: pending exception is already raised, propagate the failure
                return ResultT {};
            }
            const uint8_t *patternData = patternAccessor.GetDataUtf8();

            auto *inputEtsStr = scope.ToInternalType(execData.input);
            RegExpStringAccessor inputAccessor(inputEtsStr);
            if (!inputAccessor.HasFlattenedData()) {
                // OOM during flatten: pending exception is already raised, propagate the failure
                return ResultT {};
            }
            const uint8_t *inputData = inputAccessor.GetDataUtf8();
            // fn performs no managed allocations after the last potential GC point (the
            // accessors above), so both raw pointers stay valid throughout the call.
            return std::forward<Latin1Fn>(latin1Fn)(patternData, inputData);
        }
        case InputExecutionKind::UTF16_DIRECT:
        case InputExecutionKind::LATIN1_TO_UTF16: {
            std::vector<uint16_t> patternStorage;
            std::vector<uint16_t> inputStorage;
            ark::ets::ani::ScopedManagedCodeFix scope(env);
            // Patterns are always materialized (LATIN1 ones must be transcoded anyway);
            // matching itself only reads them, and pcre2_compile copies the pattern out.
            AcquireUtf16Input(scope, execData.pattern, static_cast<int32_t>(execData.patternSize), patternStorage);
            AcquireUtf16Input(scope, execData.input, static_cast<int32_t>(execData.inputSize), inputStorage);
            return std::forward<Utf16Fn>(utf16Fn)(patternStorage.data(), inputStorage.data());
        }
        default:
            UNREACHABLE();
    }
}

}  // namespace ark::ets::stdlib

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_REGEXP_COMMON_H
