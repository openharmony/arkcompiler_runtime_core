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

#include "RegExp.h"

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "regexp_16.h"
#include "regexp_8.h"
#include "regexp_executor.h"
#include "regexp_string_accessor.h"

#include "runtime/execution/coroutines/coroutine_scopes.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ani/ani_type_info.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_std_core_array.h"
#include "plugins/ets/runtime/ets_handle_scope.h"

#include "ani.h"

#include <algorithm>
#include <array>
#include <cstdint>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

namespace ark::ets::stdlib {

namespace refs {
ani_class g_regexpClass;
ani_class g_regexpExecArrayClass;
ani_class g_regexpMatchArrayClass;
}  // namespace refs

namespace {

constexpr const char *LAST_INDEX_FIELD_NAME = "lastIndex";

constexpr const char *REGEXP_CLASS_NAME = "std.core.RegExp";
constexpr const char *EXEC_ARRAY_CLASS_NAME = "std.core.RegExpExecArray";
constexpr const char *MATCH_ARRAY_CLASS_NAME = "std.core.RegExpMatchArray";
constexpr const char *INDEX_FIELD_NAME = "index_";
constexpr const char *RESULT_FIELD_NAME = "resultRaw_";
constexpr const char *IS_CORRECT_FIELD_NAME = "isCorrect";
constexpr const char *GROUP_KEYS_FIELD_NAME = "groupKeysRaw_";
constexpr const char *GROUP_VALS_FIELD_NAME = "groupValsRaw_";

bool IsUtf16(ani_env *env, ani_string str)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    auto internalString = s.ToInternalType(str);
    return internalString->IsUtf16();
}

std::vector<uint16_t> &GetUtf16ScratchA()
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local std::vector<uint16_t> sBuf;
    return sBuf;
}

std::vector<uint16_t> &GetUtf16ScratchB()
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local std::vector<uint16_t> sBuf;
    return sBuf;
}

void MaterializeAsUtf16InPlace(const RegExpStringAccessor &accessor, std::vector<uint16_t> &out)
{
    const size_t len = accessor.GetLength();
    out.resize(len);
    if (accessor.IsUtf16()) {
        std::copy_n(accessor.GetDataUtf16(), len, out.data());
    } else {
        const auto *src = accessor.GetDataUtf8();
        for (size_t i = 0; i < len; ++i) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            out[i] = src[i];
        }
    }
}

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

RegExpExecResult MakeFailedResult()
{
    RegExpExecResult result;
    result.isSuccess = false;
    return result;
}

uint32_t GetMatchFlags(const EtsRegExp &re)
{
    return re.HasUnicodeFlag() ? PCRE2_NO_UTF_CHECK : 0U;
}

InputExecutionKind SelectExecutionKind(const EtsRegExp &re, const ExecData &data)
{
    const bool forceUtf16 = data.requiresUtf16Execution || re.HasUnicodeFlag();
    const bool canUseLatin1Direct = !data.isUtf16Input && !data.isUtf16Pattern && !forceUtf16;
    if (canUseLatin1Direct) {
        return InputExecutionKind::LATIN1_DIRECT;
    }
    if (data.isUtf16Input) {
        return InputExecutionKind::UTF16_DIRECT;
    }
    return InputExecutionKind::LATIN1_TO_UTF16;
}

RegExpExecResult ExecuteLatin1Direct(ani_env *env, EtsRegExp &re, const ExecData &data)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    auto *patternEtsStr = s.ToInternalType(data.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    if (!re.Compile(patternAccessor.GetDataUtf8(), static_cast<int>(data.patternSize))) {
        return MakeFailedResult();
    }

    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    auto result = RegExp8::Execute(re.GetCompiledRe8(), GetMatchFlags(re), inputAccessor.GetDataUtf8(),
                                   static_cast<int>(data.inputSize), data.lastIndex);
    RegExp8::ApplyGroupMeta(re.GetCompiledRe8()->groupMeta, result);
    return result;
}

RegExpExecResult ExecuteUtf16Direct(ani_env *env, EtsRegExp &re, const ExecData &data)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    if (data.isUtf16Pattern) {
        auto *patternEtsStr = s.ToInternalType(data.pattern);
        RegExpStringAccessor patternAccessor(patternEtsStr);
        if (!re.Compile(patternAccessor.GetDataUtf16(), static_cast<int>(data.patternSize))) {
            return MakeFailedResult();
        }

        auto *inputEtsStr = s.ToInternalType(data.input);
        RegExpStringAccessor inputAccessor(inputEtsStr);
        auto result = RegExp16::Execute(re.GetCompiledRe16(), GetMatchFlags(re), inputAccessor.GetDataUtf16(),
                                        static_cast<int>(data.inputSize), data.lastIndex);
        RegExp16::ApplyGroupMeta(re.GetCompiledRe16()->groupMeta, result);
        return result;
    }

    auto *patternEtsStr = s.ToInternalType(data.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    auto &patternUtf16 = GetUtf16ScratchA();
    MaterializeAsUtf16InPlace(patternAccessor, patternUtf16);
    if (!re.Compile(patternUtf16.data(), static_cast<int>(data.patternSize))) {
        return MakeFailedResult();
    }

    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    auto result = RegExp16::Execute(re.GetCompiledRe16(), GetMatchFlags(re), inputAccessor.GetDataUtf16(),
                                    static_cast<int>(data.inputSize), data.lastIndex);
    RegExp16::ApplyGroupMeta(re.GetCompiledRe16()->groupMeta, result);
    return result;
}

RegExpExecResult ExecuteLatin1ToUtf16(ani_env *env, EtsRegExp &re, const ExecData &data)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    const uint16_t *patternPtr = nullptr;
    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    auto &inputUtf16 = GetUtf16ScratchA();
    MaterializeAsUtf16InPlace(inputAccessor, inputUtf16);
    if (data.isUtf16Pattern) {
        auto *patternEtsStr = s.ToInternalType(data.pattern);
        RegExpStringAccessor patternAccessor(patternEtsStr);
        patternPtr = patternAccessor.GetDataUtf16();
        if (!re.Compile(patternPtr, static_cast<int>(data.patternSize))) {
            return MakeFailedResult();
        }

        auto result = RegExp16::Execute(re.GetCompiledRe16(), GetMatchFlags(re), inputUtf16.data(),
                                        static_cast<int>(data.inputSize), data.lastIndex);
        RegExp16::ApplyGroupMeta(re.GetCompiledRe16()->groupMeta, result);
        return result;
    }

    auto *patternEtsStr = s.ToInternalType(data.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    auto &patternStorage = GetUtf16ScratchB();
    MaterializeAsUtf16InPlace(patternAccessor, patternStorage);
    patternPtr = patternStorage.data();
    if (!re.Compile(patternPtr, static_cast<int>(data.patternSize))) {
        return MakeFailedResult();
    }

    auto result = RegExp16::Execute(re.GetCompiledRe16(), GetMatchFlags(re), inputUtf16.data(),
                                    static_cast<int>(data.inputSize), data.lastIndex);
    RegExp16::ApplyGroupMeta(re.GetCompiledRe16()->groupMeta, result);
    return result;
}

RegExpExecResult Execute(EtsRegExp &re, ani_env *env, const ExecData &data)
{
    switch (SelectExecutionKind(re, data)) {
        case InputExecutionKind::LATIN1_DIRECT:
            return ExecuteLatin1Direct(env, re, data);
        case InputExecutionKind::UTF16_DIRECT:
            return ExecuteUtf16Direct(env, re, data);
        case InputExecutionKind::LATIN1_TO_UTF16:
            return ExecuteLatin1ToUtf16(env, re, data);
        default:
            UNREACHABLE();
    }
}

bool TestLatin1Direct(ani_env *env, EtsRegExp &re, const ExecData &data, int32_t &endIndex)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    auto *patternEtsStr = s.ToInternalType(data.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    if (!re.Compile(patternAccessor.GetDataUtf8(), static_cast<int>(data.patternSize))) {
        return false;
    }

    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    return RegExp8::TestMatch(re.GetCompiledRe8(), GetMatchFlags(re), inputAccessor.GetDataUtf8(),
                              static_cast<int>(data.inputSize), data.lastIndex, endIndex);
}

bool TestUtf16Direct(ani_env *env, EtsRegExp &re, const ExecData &data, int32_t &endIndex)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    if (data.isUtf16Pattern) {
        auto *patternEtsStr = s.ToInternalType(data.pattern);
        RegExpStringAccessor patternAccessor(patternEtsStr);
        if (!re.Compile(patternAccessor.GetDataUtf16(), static_cast<int>(data.patternSize))) {
            return false;
        }

        auto *inputEtsStr = s.ToInternalType(data.input);
        RegExpStringAccessor inputAccessor(inputEtsStr);
        return RegExp16::TestMatch(re.GetCompiledRe16(), GetMatchFlags(re), inputAccessor.GetDataUtf16(),
                                   static_cast<int>(data.inputSize), data.lastIndex, endIndex);
    }

    auto *patternEtsStr = s.ToInternalType(data.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    auto &patternUtf16 = GetUtf16ScratchA();
    MaterializeAsUtf16InPlace(patternAccessor, patternUtf16);
    if (!re.Compile(patternUtf16.data(), static_cast<int>(data.patternSize))) {
        return false;
    }

    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    return RegExp16::TestMatch(re.GetCompiledRe16(), GetMatchFlags(re), inputAccessor.GetDataUtf16(),
                               static_cast<int>(data.inputSize), data.lastIndex, endIndex);
}

bool TestLatin1ToUtf16(ani_env *env, EtsRegExp &re, const ExecData &data, int32_t &endIndex)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    const uint16_t *patternPtr = nullptr;
    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    auto &inputUtf16 = GetUtf16ScratchA();
    MaterializeAsUtf16InPlace(inputAccessor, inputUtf16);
    if (data.isUtf16Pattern) {
        auto *patternEtsStr = s.ToInternalType(data.pattern);
        RegExpStringAccessor patternAccessor(patternEtsStr);
        patternPtr = patternAccessor.GetDataUtf16();
        if (!re.Compile(patternPtr, static_cast<int>(data.patternSize))) {
            return false;
        }
    } else {
        auto *patternEtsStr = s.ToInternalType(data.pattern);
        RegExpStringAccessor patternAccessor(patternEtsStr);
        auto &patternStorage = GetUtf16ScratchB();
        MaterializeAsUtf16InPlace(patternAccessor, patternStorage);
        patternPtr = patternStorage.data();
        if (!re.Compile(patternPtr, static_cast<int>(data.patternSize))) {
            return false;
        }
    }

    return RegExp16::TestMatch(re.GetCompiledRe16(), GetMatchFlags(re), inputUtf16.data(),
                               static_cast<int>(data.inputSize), data.lastIndex, endIndex);
}

bool ExecuteTest(EtsRegExp &re, ani_env *env, const ExecData &data, int32_t &endIndex)
{
    switch (SelectExecutionKind(re, data)) {
        case InputExecutionKind::LATIN1_DIRECT:
            return TestLatin1Direct(env, re, data, endIndex);
        case InputExecutionKind::UTF16_DIRECT:
            return TestUtf16Direct(env, re, data, endIndex);
        case InputExecutionKind::LATIN1_TO_UTF16:
            return TestLatin1ToUtf16(env, re, data, endIndex);
        default:
            UNREACHABLE();
    }
}

}  // namespace

static void SetResultField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                           const std::vector<RegExpExecResult::IndexPair> &matches)
{
    if (matches.empty()) {
        return;
    }
    ani_field resultField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, RESULT_FIELD_NAME, &resultField));

    const auto count = static_cast<ani_size>(matches.size() * 2U);
    ani_fixedarray_int intArray;
    ANI_FATAL_IF_ERROR(env->FixedArray_New_Int(count, &intArray));

    std::vector<ani_int> flat;
    flat.reserve(count);
    for (const auto &[index, endIndex] : matches) {
        flat.push_back(index);
        flat.push_back(endIndex);
    }
    ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Int(intArray, 0, count, flat.data()));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, resultField, static_cast<ani_ref>(intArray)));
}

static void SetIsCorrectField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArrayObj, bool value)
{
    ani_field resultCorrectField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, IS_CORRECT_FIELD_NAME, &resultCorrectField));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Boolean(regexpExecArrayObj, resultCorrectField, value));
}

static void SetIndexField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArrayObj, uint32_t index)
{
    ani_field indexField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, INDEX_FIELD_NAME, &indexField));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(regexpExecArrayObj, indexField, static_cast<double>(index)));
}

static void SetLastIndexField(ani_env *env, ani_object regexp, ani_field lastIndexField, ani_int value)
{
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(regexp, lastIndexField, value));
}

static void SetGroupKeysField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                              const std::map<std::string, std::pair<int32_t, int32_t>> &groups)
{
    if (groups.empty()) {
        return;
    }
    ani_field groupKeysField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, GROUP_KEYS_FIELD_NAME, &groupKeysField));

    ani_class stringClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.String", &stringClass));

    auto it = groups.begin();
    ani_string firstKey;
    ANI_FATAL_IF_ERROR(env->String_NewUTF8(it->first.c_str(), it->first.size(), &firstKey));

    ani_fixedarray_ref keysArray;
    ANI_FATAL_IF_ERROR(env->FixedArray_New_Ref(stringClass, groups.size(), firstKey, &keysArray));

    ani_size index = 1;
    ++it;
    for (; it != groups.end(); ++it, ++index) {
        ani_string key;
        ANI_FATAL_IF_ERROR(env->String_NewUTF8(it->first.c_str(), it->first.size(), &key));
        ANI_FATAL_IF_ERROR(env->FixedArray_Set_Ref(keysArray, index, key));
    }
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, groupKeysField, static_cast<ani_ref>(keysArray)));
}

static void SetGroupValsField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                              const std::map<std::string, std::pair<int32_t, int32_t>> &groups)
{
    if (groups.empty()) {
        return;
    }
    ani_field groupValsField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, GROUP_VALS_FIELD_NAME, &groupValsField));

    const auto count = static_cast<ani_size>(groups.size() * 2U);
    ani_fixedarray_int valsArray;
    ANI_FATAL_IF_ERROR(env->FixedArray_New_Int(count, &valsArray));

    std::vector<ani_int> flat;
    flat.reserve(count);
    for (const auto &entry : groups) {
        flat.push_back(entry.second.first);
        flat.push_back(entry.second.second);
    }
    ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Int(valsArray, 0, count, flat.data()));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, groupValsField, static_cast<ani_ref>(valsArray)));
}

static ani_object DoExec(ani_env *env, ani_object regexp, ani_class resultClass, ani_object resultObject,
                         ExecData execData)
{
    EtsRegExp re(env);
    re.SetFlags(ConvertFromAniString(env, execData.flags));

    ani_field lastIndexField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_regexpClass, LAST_INDEX_FIELD_NAME, &lastIndexField));

    auto lastIdx = execData.lastIndex;
    const bool globalOrSticky = re.IsGlobal() || re.IsSticky();
    if (!globalOrSticky || lastIdx < 0) {
        lastIdx = 0;
        execData.lastIndex = 0;
    }
    if (static_cast<size_t>(lastIdx) > execData.inputSize) {
        if (globalOrSticky) {
            SetLastIndexField(env, regexp, lastIndexField, 0);
        }
        SetIsCorrectField(env, resultClass, resultObject, false);
        return resultObject;
    }
    auto execResult = Execute(re, env, execData);
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    if (!execResult.isSuccess) {
        if (globalOrSticky) {
            SetLastIndexField(env, regexp, lastIndexField, 0);
        }
        SetIsCorrectField(env, resultClass, resultObject, false);
        return resultObject;
    }

    if (globalOrSticky) {
        SetLastIndexField(env, regexp, lastIndexField, execResult.endIndex);
    }
    SetIsCorrectField(env, resultClass, resultObject, true);
    SetIndexField(env, resultClass, resultObject, execResult.index);
    SetResultField(env, resultClass, resultObject, execResult.indices);
    SetGroupKeysField(env, resultClass, resultObject, execResult.namedGroups);
    SetGroupValsField(env, resultClass, resultObject, execResult.namedGroups);

    return resultObject;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_object Exec(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                       ani_int patternSize, ani_int strSize, ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    ani_method regexpExecArrayCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpExecArrayClass, "<ctor>", ":", &regexpExecArrayCtor));
    ani_object regexpExecArrayObject;

    const bool isUtf16Pattern = IsUtf16(env, pattern);
    const bool isUtf16Input = IsUtf16(env, str);
    ExecData execData {pattern,
                       str,
                       flags,
                       static_cast<int32_t>(lastIndex),
                       static_cast<size_t>(patternSize),
                       static_cast<size_t>(strSize),
                       isUtf16Pattern,
                       isUtf16Input,
                       static_cast<bool>(requiresUtf16Execution)};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(refs::g_regexpExecArrayClass, regexpExecArrayCtor, &regexpExecArrayObject));
    return DoExec(env, regexp, refs::g_regexpExecArrayClass, regexpExecArrayObject, execData);
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_object Match(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                        ani_int patternSize, ani_int strSize, ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    ani_method regexpMatchArrayCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpMatchArrayClass, "<ctor>", ":", &regexpMatchArrayCtor));
    ani_object regexpMatchArrayObject;

    const bool isUtf16Pattern = IsUtf16(env, pattern);
    const bool isUtf16Input = IsUtf16(env, str);
    ExecData execData {pattern,
                       str,
                       flags,
                       static_cast<int32_t>(lastIndex),
                       static_cast<size_t>(patternSize),
                       static_cast<size_t>(strSize),
                       isUtf16Pattern,
                       isUtf16Input,
                       static_cast<bool>(requiresUtf16Execution)};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(refs::g_regexpMatchArrayClass, regexpMatchArrayCtor, &regexpMatchArrayObject));
    return DoExec(env, regexp, refs::g_regexpMatchArrayClass, regexpMatchArrayObject, execData);
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_boolean Test(ani_env *env, [[maybe_unused]] ani_object regexp, ani_string pattern, ani_string flags,
                        ani_string str, ani_int patternSize, ani_int strSize, ani_int lastIndex,
                        ani_boolean requiresUtf16Execution)
{
    const bool isUtf16Pattern = IsUtf16(env, pattern);
    const bool isUtf16Input = IsUtf16(env, str);
    auto re = EtsRegExp(env);
    re.SetFlags(ConvertFromAniString(env, flags));
    ExecData execData {pattern,
                       str,
                       flags,
                       static_cast<int32_t>(lastIndex),
                       static_cast<size_t>(patternSize),
                       static_cast<size_t>(strSize),
                       isUtf16Pattern,
                       isUtf16Input,
                       static_cast<bool>(requiresUtf16Execution)};
    int32_t endIndex = 0;
    const bool execResult = ExecuteTest(re, env, execData, endIndex);
    const bool globalOrSticky = re.IsGlobal() || re.IsSticky();
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    if (execResult && globalOrSticky) {
        ani_field lastIndexField;
        ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_regexpClass, LAST_INDEX_FIELD_NAME, &lastIndexField));
        SetLastIndexField(env, regexp, lastIndexField, endIndex);
    }
    return execResult ? ANI_TRUE : ANI_FALSE;
}

void RegisterRegExpNativeMethods(ani_env *env)
{
    const auto regExpImpls = std::array {
        ani_native_function {"execImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpExecArray}",
                             reinterpret_cast<void *>(Exec)},
        ani_native_function {"matchImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpMatchArray}",
                             reinterpret_cast<void *>(Match)},
        ani_native_function {"testImpl", "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:z",
                             reinterpret_cast<void *>(Test)}};

    ani_class regexpClassLocal;
    ANI_FATAL_IF_ERROR(env->FindClass(REGEXP_CLASS_NAME, &regexpClassLocal));
    ANI_FATAL_IF_ERROR(
        env->GlobalReference_Create(regexpClassLocal, reinterpret_cast<ani_ref *>(&refs::g_regexpClass)));
    ANI_FATAL_IF_ERROR(env->Class_BindNativeMethods(refs::g_regexpClass, regExpImpls.data(), regExpImpls.size()));

    ani_class regexpExecArrayClassLocal;
    ANI_FATAL_IF_ERROR(env->FindClass(EXEC_ARRAY_CLASS_NAME, &regexpExecArrayClassLocal));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(regexpExecArrayClassLocal,
                                                   reinterpret_cast<ani_ref *>(&refs::g_regexpExecArrayClass)));

    ani_class regexpMatchArrayClassLocal;
    ANI_FATAL_IF_ERROR(env->FindClass(MATCH_ARRAY_CLASS_NAME, &regexpMatchArrayClassLocal));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(regexpMatchArrayClassLocal,
                                                   reinterpret_cast<ani_ref *>(&refs::g_regexpMatchArrayClass)));
}

}  // namespace ark::ets::stdlib
