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
#include "plugins/ets/runtime/types/ets_escompat_array.h"
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
constexpr const char *GROUPS_FIELD_NAME = "groupsRaw_";

constexpr size_t MAX_INT32_DIGITS = 11;
constexpr size_t AVG_KEY_LENGTH = 10;
constexpr size_t SEPARATOR_SIZE = 1;

constexpr auto FLAG_GLOBAL = (1U << 0U);
constexpr auto FLAG_IGNORECASE = (1U << 1U);
constexpr auto FLAG_MULTILINE = (1U << 2U);
constexpr auto FLAG_DOTALL = (1U << 3U);
constexpr auto FLAG_UTF16 = (1U << 4U);
constexpr auto FLAG_STICKY = (1U << 5U);
constexpr auto FLAG_HASINDICES = (1U << 6U);
constexpr auto FLAG_EXT_UNICODE = (1U << 7U);

uint32_t CastToBitMask(ani_env *env, ani_string checkStr)
{
    uint32_t flagsBits = 0;
    uint32_t flagsBitsTemp = 0;
    for (const char &c : ConvertFromAniString(env, checkStr)) {
        switch (c) {
            case 'd':
                flagsBitsTemp = FLAG_HASINDICES;
                break;
            case 'g':
                flagsBitsTemp = FLAG_GLOBAL;
                break;
            case 'i':
                flagsBitsTemp = FLAG_IGNORECASE;
                break;
            case 'm':
                flagsBitsTemp = FLAG_MULTILINE;
                break;
            case 's':
                flagsBitsTemp = FLAG_DOTALL;
                break;
            case 'u':
                flagsBitsTemp = FLAG_UTF16;
                break;
            case 'v':
                flagsBitsTemp = FLAG_EXT_UNICODE;
                break;
            case 'y':
                flagsBitsTemp = FLAG_STICKY;
                break;
            default: {
                ThrowNewError(env, "std.core.IllegalArgumentError", "invalid regular expression flags");
                return 0;
            }
        }
        if ((flagsBits & flagsBitsTemp) != 0) {
            ThrowNewError(env, "std.core.IllegalArgumentError", "invalid regular expression flags");
            return 0;
        }
        flagsBits |= flagsBitsTemp;
    }
    return flagsBits;
}

bool IsUtf16(ani_env *env, ani_string str)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    auto internalString = s.ToInternalType(str);
    return internalString->IsUtf16();
}

std::vector<uint16_t> ExpandLatin1ToUtf16(const uint8_t *data, size_t len)
{
    std::vector<uint16_t> result(len);
    for (size_t i = 0; i < len; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        result[i] = data[i];
    }
    return result;
}

std::vector<uint16_t> MaterializeAsUtf16(const RegExpStringAccessor &accessor)
{
    if (accessor.IsUtf16()) {
        const auto *data = accessor.GetDataUtf16();
        std::vector<uint16_t> result(accessor.GetLength());
        std::copy_n(data, accessor.GetLength(), result.begin());
        return result;
    }
    return ExpandLatin1ToUtf16(accessor.GetDataUtf8(), accessor.GetLength());
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
    RegExp8::EraseExtraGroups(patternAccessor.GetDataUtf8(), data.patternSize, result);
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
        RegExp16::EraseExtraGroups(patternAccessor.GetDataUtf16(), data.patternSize, result);
        return result;
    }

    auto *patternEtsStr = s.ToInternalType(data.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    auto patternUtf16 = MaterializeAsUtf16(patternAccessor);
    if (!re.Compile(patternUtf16.data(), static_cast<int>(data.patternSize))) {
        return MakeFailedResult();
    }

    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    auto result = RegExp16::Execute(re.GetCompiledRe16(), GetMatchFlags(re), inputAccessor.GetDataUtf16(),
                                    static_cast<int>(data.inputSize), data.lastIndex);
    RegExp16::EraseExtraGroups(patternUtf16.data(), data.patternSize, result);
    return result;
}

RegExpExecResult ExecuteLatin1ToUtf16(ani_env *env, EtsRegExp &re, const ExecData &data)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    const uint16_t *patternPtr = nullptr;
    std::vector<uint16_t> patternStorage;
    auto *inputEtsStr = s.ToInternalType(data.input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    auto inputUtf16 = MaterializeAsUtf16(inputAccessor);
    if (data.isUtf16Pattern) {
        auto *patternEtsStr = s.ToInternalType(data.pattern);
        RegExpStringAccessor patternAccessor(patternEtsStr);
        patternPtr = patternAccessor.GetDataUtf16();
        if (!re.Compile(patternPtr, static_cast<int>(data.patternSize))) {
            return MakeFailedResult();
        }

        auto result = RegExp16::Execute(re.GetCompiledRe16(), GetMatchFlags(re), inputUtf16.data(),
                                        static_cast<int>(data.inputSize), data.lastIndex);
        RegExp16::EraseExtraGroups(patternPtr, data.patternSize, result);
        return result;
    }

    auto *patternEtsStr = s.ToInternalType(data.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    patternStorage = MaterializeAsUtf16(patternAccessor);
    patternPtr = patternStorage.data();
    if (!re.Compile(patternPtr, static_cast<int>(data.patternSize))) {
        return MakeFailedResult();
    }

    auto result = RegExp16::Execute(re.GetCompiledRe16(), GetMatchFlags(re), inputUtf16.data(),
                                    static_cast<int>(data.inputSize), data.lastIndex);
    RegExp16::EraseExtraGroups(patternPtr, data.patternSize, result);
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

}  // namespace

static RegExpExecResult Execute(ani_env *env, const ExecData &data)
{
    auto re = EtsRegExp(env);
    re.SetFlags(ConvertFromAniString(env, data.flags));
    auto result = Execute(re, env, data);
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    return result;
}

static void SetResultField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                           const std::vector<RegExpExecResult::IndexPair> &matches)
{
    if (matches.empty()) {
        return;
    }
    ani_field resultField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, RESULT_FIELD_NAME, &resultField));

    size_t estimatedSize = matches.size() * (MAX_INT32_DIGITS + SEPARATOR_SIZE + MAX_INT32_DIGITS + SEPARATOR_SIZE);
    std::string data;
    data.reserve(estimatedSize);
    for (const auto &[index, endIndex] : matches) {
        data.append(std::to_string(index)).append(1, ',').append(std::to_string(endIndex)).append(1, ',');
    }
    if (!data.empty()) {
        data.pop_back();
    }

    ani_string resultStr;
    ANI_FATAL_IF_ERROR(env->String_NewUTF8(data.c_str(), data.size(), &resultStr));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, resultField, static_cast<ani_ref>(resultStr)));
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

static void SetGroupsField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                           const std::map<std::string, std::pair<int32_t, int32_t>> &groups)
{
    if (groups.empty()) {
        return;
    }
    ani_field groupsField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, GROUPS_FIELD_NAME, &groupsField));

    size_t estimatedSize = groups.size() * (AVG_KEY_LENGTH + SEPARATOR_SIZE + MAX_INT32_DIGITS + SEPARATOR_SIZE +
                                            MAX_INT32_DIGITS + SEPARATOR_SIZE);
    std::string data;
    data.reserve(estimatedSize);
    for (const auto &[key, value] : groups) {
        data.append(key)
            .append(1, ',')
            .append(std::to_string(value.first))
            .append(1, ',')
            .append(std::to_string(value.second))
            .append(1, ';');
    }
    if (!data.empty()) {
        data.pop_back();
    }

    ani_string groupsStr;
    ANI_FATAL_IF_ERROR(env->String_NewUTF8(data.c_str(), data.size(), &groupsStr));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, groupsField, static_cast<ani_ref>(groupsStr)));
}

static ani_object DoExec(ani_env *env, ani_object regexp, ani_class resultClass, ani_object resultObject,
                         ExecData execData)
{
    ani_field lastIndexField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_regexpClass, LAST_INDEX_FIELD_NAME, &lastIndexField));

    auto lastIdx = execData.lastIndex;
    const auto flagsBits = static_cast<uint8_t>(CastToBitMask(env, execData.flags));

    const bool global = (flagsBits & FLAG_GLOBAL) > 0;
    const bool sticky = (flagsBits & FLAG_STICKY) > 0;
    const bool globalOrSticky = global || sticky;
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
    auto execResult = Execute(env, execData);
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
    SetGroupsField(env, resultClass, resultObject, execResult.namedGroups);

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
    RegExpExecResult execResult =
        Execute(re, env,
                ExecData {pattern, str, flags, static_cast<int32_t>(lastIndex), static_cast<size_t>(patternSize),
                          static_cast<size_t>(strSize), isUtf16Pattern, isUtf16Input,
                          static_cast<bool>(requiresUtf16Execution)});
    if (re.HasCompiledRe()) {
        re.Destroy();
    }

    const auto flagsBits = static_cast<uint8_t>(CastToBitMask(env, flags));

    const bool global = (flagsBits & FLAG_GLOBAL) > 0;
    const bool sticky = (flagsBits & FLAG_STICKY) > 0;
    const bool globalOrSticky = global || sticky;
    if (execResult.isSuccess && globalOrSticky) {
        ani_field lastIndexField;
        ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_regexpClass, LAST_INDEX_FIELD_NAME, &lastIndexField));
        SetLastIndexField(env, regexp, lastIndexField, execResult.endIndex);
    }
    return execResult.isSuccess ? ANI_TRUE : ANI_FALSE;
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
