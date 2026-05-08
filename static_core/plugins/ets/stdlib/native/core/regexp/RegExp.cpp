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

#include "regexp_common.h"
#include "regexp_matchall.h"
#include "regexp_replace.h"
#include "regexp_split.h"

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"
#include "plugins/ets/runtime/ani/ani_type_info.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_std_core_array.h"

#include "ani.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace ark::ets::stdlib {

namespace refs {
ani_class g_regexpClass;
ani_class g_regexpExecArrayClass;
ani_class g_regexpMatchArrayClass;
}  // namespace refs

namespace {

constexpr const char *REGEXP_CLASS_NAME = "std.core.RegExp";
constexpr const char *EXEC_ARRAY_CLASS_NAME = "std.core.RegExpExecArray";
constexpr const char *MATCH_ARRAY_CLASS_NAME = "std.core.RegExpMatchArray";

ani_object CreateObjectOrNullOnPendingError(ani_env *env, ani_class klass, ani_method ctor)
{
    ani_object object = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ani_status status = env->Object_New(klass, ctor, &object);
    if (status == ANI_PENDING_ERROR || status == ANI_OUT_OF_MEMORY) {
        return nullptr;
    }
    if (status != ANI_OK) {
        StdlibLogFatal("Object_New", status);
        UNREACHABLE();
    }
    return object;
}

}  // namespace

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_object DoExec(ani_env *env, ani_object regexp, ani_class resultClass, ani_object resultObject,
                         ExecData execData)
{
    EtsRegExp re(env);
    re.SetFlags(ConvertFromAniString(env, execData.flags));

    ani_field lastIndexField;
    ANI_FATAL_IF_ERROR(FindLastIndexField(env, &lastIndexField));

    auto lastIdx = execData.lastIndex;
    const bool globalOrSticky = re.IsGlobal() || re.IsSticky();
    if (!globalOrSticky || lastIdx < 0) {
        lastIdx = 0;
        execData.lastIndex = 0;
    }
    if (static_cast<size_t>(lastIdx) > execData.inputSize) {
        if (globalOrSticky) {
            ANI_FATAL_IF_ERROR(SetLastIndex(env, regexp, lastIndexField, 0));
        }
        ANI_FATAL_IF_ERROR(SetIsCorrectField(env, resultClass, resultObject, false));
        return resultObject;
    }
    auto executionKind = SelectExecutionKind(re, execData);
    auto execResult = PrepareInputAndRun<RegExpExecResult>(
        re, execData, env, executionKind,
        [&re, &execData](const uint8_t *inputData) -> RegExpExecResult {
            auto result = RegExp8::Execute(re.GetCompiledRe8(), re.GetMatchFlags(), inputData,
                                           static_cast<int>(execData.inputSize), execData.lastIndex);
            RegExp8::ApplyGroupMeta(re.GetCompiledRe8()->groupMeta, result);
            return result;
        },
        [&re, &execData](const uint16_t *inputData) -> RegExpExecResult {
            auto result = RegExp16::Execute(re.GetCompiledRe16(), re.GetMatchFlags(), inputData,
                                            static_cast<int>(execData.inputSize), execData.lastIndex);
            RegExp16::ApplyGroupMeta(re.GetCompiledRe16()->groupMeta, result);
            return result;
        });
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    if (!execResult.isSuccess) {
        if (globalOrSticky) {
            ANI_FATAL_IF_ERROR(SetLastIndex(env, regexp, lastIndexField, 0));
        }
        ANI_FATAL_IF_ERROR(SetIsCorrectField(env, resultClass, resultObject, false));
        return resultObject;
    }

    if (globalOrSticky) {
        ANI_FATAL_IF_ERROR(SetLastIndex(env, regexp, lastIndexField, execResult.endIndex));
    }
    ANI_FATAL_IF_ERROR(PopulateRegExpResultObject(env, resultClass, resultObject, execResult));

    return resultObject;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_object Exec(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                       ani_int patternSize, ani_int strSize, ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    ani_method regexpExecArrayCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpExecArrayClass, "<ctor>", ":", &regexpExecArrayCtor));

    ExecData execData = MakeExecData(env, pattern, str, flags, patternSize, strSize, lastIndex, requiresUtf16Execution);

    ani_object regexpExecArrayObject =
        CreateObjectOrNullOnPendingError(env, refs::g_regexpExecArrayClass, regexpExecArrayCtor);
    if (regexpExecArrayObject == nullptr) {
        return nullptr;
    }
    return DoExec(env, regexp, refs::g_regexpExecArrayClass, regexpExecArrayObject, execData);
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_object Match(ani_env *env, ani_object regexp, ani_string pattern, ani_string flags, ani_string str,
                        ani_int patternSize, ani_int strSize, ani_int lastIndex, ani_boolean requiresUtf16Execution)
{
    ani_method regexpMatchArrayCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(refs::g_regexpMatchArrayClass, "<ctor>", ":", &regexpMatchArrayCtor));

    ExecData execData = MakeExecData(env, pattern, str, flags, patternSize, strSize, lastIndex, requiresUtf16Execution);

    ani_object regexpMatchArrayObject =
        CreateObjectOrNullOnPendingError(env, refs::g_regexpMatchArrayClass, regexpMatchArrayCtor);
    if (regexpMatchArrayObject == nullptr) {
        return nullptr;
    }
    return DoExec(env, regexp, refs::g_regexpMatchArrayClass, regexpMatchArrayObject, execData);
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
static ani_boolean Test(ani_env *env, [[maybe_unused]] ani_object regexp, ani_string pattern, ani_string flags,
                        ani_string str, ani_int patternSize, ani_int strSize, ani_int lastIndex,
                        ani_boolean requiresUtf16Execution)
{
    ExecData execData = MakeExecData(env, pattern, str, flags, patternSize, strSize, lastIndex, requiresUtf16Execution);
    auto re = EtsRegExp(env);
    re.SetFlags(ConvertFromAniString(env, flags));

    int32_t endIndex = 0;
    auto executionKind = SelectExecutionKind(re, execData);
    auto testResult = PrepareInputAndRun<bool>(
        re, execData, env, executionKind,
        [&re, &execData, &endIndex](const uint8_t *inputData) -> bool {
            return RegExp8::TestMatch(re.GetCompiledRe8(), re.GetMatchFlags(), inputData,
                                      static_cast<int>(execData.inputSize), execData.lastIndex, endIndex);
        },
        [&re, &execData, &endIndex](const uint16_t *inputData) -> bool {
            return RegExp16::TestMatch(re.GetCompiledRe16(), re.GetMatchFlags(), inputData,
                                       static_cast<int>(execData.inputSize), execData.lastIndex, endIndex);
        });
    const bool globalOrSticky = re.IsGlobal() || re.IsSticky();
    if (re.HasCompiledRe()) {
        re.Destroy();
    }
    if (testResult && globalOrSticky) {
        ani_field lastIndexField;
        ANI_FATAL_IF_ERROR(FindLastIndexField(env, &lastIndexField));
        ANI_FATAL_IF_ERROR(SetLastIndex(env, regexp, lastIndexField, endIndex));
    }
    return testResult ? ANI_TRUE : ANI_FALSE;
}

ani_status RegisterRegExpNativeMethods(ani_env *env)
{
    const auto regExpImpls = std::array {
        ani_native_function {"execImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpExecArray}",
                             reinterpret_cast<void *>(Exec)},
        ani_native_function {"matchImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpMatchArray}",
                             reinterpret_cast<void *>(Match)},
        ani_native_function {"testImpl", "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:z",
                             reinterpret_cast<void *>(Test)},
        ani_native_function {"replaceNativeImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.Array}",
                             reinterpret_cast<void *>(ReplaceNativeImpl)},
        ani_native_function {"matchAllNativeImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.Array}",
                             reinterpret_cast<void *>(MatchAllNativeImpl)},
        ani_native_function {
            "replaceLiteralNativeImpl",
            "C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.String}",
            reinterpret_cast<void *>(ReplaceLiteralNativeImpl)},
        ani_native_function {"splitNativeImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iilzz:C{std.core.Array}",
                             reinterpret_cast<void *>(SplitNativeImpl)}};

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
    return ANI_OK;
}

}  // namespace ark::ets::stdlib
