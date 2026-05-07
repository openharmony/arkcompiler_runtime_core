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

#include "regexp_common.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 0
#include "pcre2.h"

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

#include <algorithm>
#include <map>
#include <string>

namespace ark::ets::stdlib {

namespace {

constexpr const char *INDEX_FIELD_NAME = "index_";
constexpr const char *RESULT_FIELD_NAME = "resultRaw_";
constexpr const char *GROUP_KEYS_FIELD_NAME = "groupKeysRaw_";
constexpr const char *GROUP_VALS_FIELD_NAME = "groupValsRaw_";

template <typename Container, typename PairAccessor>
std::vector<ani_int> FlattenPairs(const Container &items, PairAccessor &&getPair)
{
    std::vector<ani_int> flat;
    if (items.empty()) {
        return flat;
    }

    const auto count = static_cast<ani_size>(items.size() * 2U);
    flat.reserve(count);
    for (const auto &item : items) {
        const auto pair = getPair(item);
        flat.push_back(pair.first);
        flat.push_back(pair.second);
    }
    return flat;
}

ani_status SetResultField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                          const std::vector<RegExpExecResult::IndexPair> &matches)
{
    if (matches.empty()) {
        return ANI_OK;
    }
    ani_field resultField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, RESULT_FIELD_NAME, &resultField));

    const auto count = static_cast<ani_size>(matches.size() * 2U);
    ani_fixedarray_int intArray;
    ANI_RETURN_ON_PENDING_ERROR(env->FixedArray_New_Int(count, &intArray));

    const auto flat = FlattenPairs(matches, [](const auto &match) { return match; });
    ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Int(intArray, 0, count, flat.data()));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, resultField, static_cast<ani_ref>(intArray)));
    return ANI_OK;
}

ani_status SetIndexField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArrayObj, uint32_t index)
{
    ani_field indexField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, INDEX_FIELD_NAME, &indexField));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(regexpExecArrayObj, indexField, static_cast<double>(index)));
    return ANI_OK;
}

ani_status SetGroupKeysField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                             const std::map<std::string, std::pair<int32_t, int32_t>> &groups)
{
    if (groups.empty()) {
        return ANI_OK;
    }
    ani_field groupKeysField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, GROUP_KEYS_FIELD_NAME, &groupKeysField));

    ani_class stringClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.String", &stringClass));

    auto it = groups.begin();
    ani_string firstKey;
    ANI_RETURN_ON_PENDING_ERROR(env->String_NewUTF8(it->first.c_str(), it->first.size(), &firstKey));

    ani_fixedarray_ref keysArray;
    ANI_RETURN_ON_PENDING_ERROR(env->FixedArray_New_Ref(stringClass, groups.size(), firstKey, &keysArray));

    ani_size index = 1;
    ++it;
    for (; it != groups.end(); ++it, ++index) {
        ani_string key;
        ANI_RETURN_ON_PENDING_ERROR(env->String_NewUTF8(it->first.c_str(), it->first.size(), &key));
        ANI_FATAL_IF_ERROR(env->FixedArray_Set_Ref(keysArray, index, key));
    }
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, groupKeysField, static_cast<ani_ref>(keysArray)));
    return ANI_OK;
}

ani_status SetGroupValsField(ani_env *env, ani_class regexpResultClass, ani_object regexpExecArray,
                             const std::map<std::string, std::pair<int32_t, int32_t>> &groups)
{
    if (groups.empty()) {
        return ANI_OK;
    }
    ani_field groupValsField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(regexpResultClass, GROUP_VALS_FIELD_NAME, &groupValsField));

    const auto count = static_cast<ani_size>(groups.size() * 2U);
    ani_fixedarray_int valsArray;
    ANI_RETURN_ON_PENDING_ERROR(env->FixedArray_New_Int(count, &valsArray));

    const auto flat = FlattenPairs(groups, [](const auto &entry) { return entry.second; });
    ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Int(valsArray, 0, count, flat.data()));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(regexpExecArray, groupValsField, static_cast<ani_ref>(valsArray)));
    return ANI_OK;
}

}  // namespace

void MaterializeAsUtf16InPlace(const RegExpStringAccessor &accessor, std::vector<uint16_t> &out)
{
    const size_t len = accessor.GetLength();
    out.resize(len);

    auto copy = [&out, len](const auto *src) { std::copy_n(src, len, out.data()); };

    accessor.IsUtf16() ? copy(accessor.GetDataUtf16()) : copy(accessor.GetDataUtf8());
}

const uint16_t *AcquireUtf16Input(ark::ets::ani::ScopedManagedCodeFix &scope, ani_string input, int32_t inputSize,
                                  bool needsMaterialization, std::vector<uint16_t> &storage)
{
    auto *inputEtsStr = scope.ToInternalType(input);
    RegExpStringAccessor inputAccessor(inputEtsStr);
    if (!needsMaterialization) {
        return inputAccessor.GetDataUtf16();
    }
    storage.resize(inputSize);
    MaterializeAsUtf16InPlace(inputAccessor, storage);
    return storage.data();
}

const uint16_t *AcquireUtf16Replacement(ark::ets::ani::ScopedManagedCodeFix &scope, ani_string replaceValue,
                                        std::vector<uint16_t> &storage, ani_int &length)
{
    auto *replaceEtsStr = scope.ToInternalType(replaceValue);
    RegExpStringAccessor replaceAccessor(replaceEtsStr);
    length = static_cast<ani_int>(replaceAccessor.GetLength());
    if (replaceAccessor.IsUtf16()) {
        return replaceAccessor.GetDataUtf16();
    }
    MaterializeAsUtf16InPlace(replaceAccessor, storage);
    return storage.data();
}

bool CompileUtf16Pattern(EtsRegExp &re, ark::ets::ani::ScopedManagedCodeFix &scope, const ExecData &execData,
                         std::vector<uint16_t> &storage)
{
    auto *patternEtsStr = scope.ToInternalType(execData.pattern);
    RegExpStringAccessor patternAccessor(patternEtsStr);
    const auto patternSize = static_cast<int>(execData.patternSize);
    if (execData.isUtf16Pattern) {
        return re.Compile(patternAccessor.GetDataUtf16(), patternSize);
    }
    MaterializeAsUtf16InPlace(patternAccessor, storage);
    return re.Compile(storage.data(), patternSize);
}

template <typename CodeT>
bool HasCapturingGroupsImpl(const void *pcre2Code)
{
    ASSERT(pcre2Code != nullptr);
    auto *code = static_cast<const CodeT *>(pcre2Code);
    uint32_t captureCount = 0U;
    if constexpr (std::is_same_v<CodeT, pcre2_code_8>) {
        if (pcre2_pattern_info_8(code, PCRE2_INFO_CAPTURECOUNT, &captureCount) != 0) {
            return true;
        }
    } else {
        if (pcre2_pattern_info_16(code, PCRE2_INFO_CAPTURECOUNT, &captureCount) != 0) {
            return true;
        }
    }
    return captureCount > 0U;
}

template bool HasCapturingGroupsImpl<pcre2_code_8>(const void *pcre2Code);
template bool HasCapturingGroupsImpl<pcre2_code_16>(const void *pcre2Code);

InputExecutionKind SelectExecutionKind(const EtsRegExp &re, const ExecData &data)
{
    const bool forceUtf16 = data.requiresUtf16Execution || re.HasUnicodeOrUnicodeSetsFlag();
    const bool canUseLatin1Direct = !data.isUtf16Input && !data.isUtf16Pattern && !forceUtf16;
    if (canUseLatin1Direct) {
        return InputExecutionKind::LATIN1_DIRECT;
    }
    if (data.isUtf16Input) {
        return InputExecutionKind::UTF16_DIRECT;
    }
    return InputExecutionKind::LATIN1_TO_UTF16;
}

ani_status CreateResultArray(ani_env *env, const std::vector<ani_object> &results, ani_array *out)
{
    ani_ref undefined {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));

    ANI_RETURN_ON_PENDING_ERROR(env->Array_New(static_cast<ani_size>(results.size()), undefined, out));
    for (ani_size i = 0; i < results.size(); ++i) {
        ANI_FATAL_IF_ERROR(env->Array_Set(*out, i, results[i]));
    }
    return ANI_OK;
}

ani_status PopulateRegExpResultObject(ani_env *env, ani_class regexpResultClass, ani_object regexpResultObject,
                                      const RegExpExecResult &execResult)
{
    ANI_FATAL_IF_ERROR(SetIsCorrectField(env, regexpResultClass, regexpResultObject, true));
    ANI_FATAL_IF_ERROR(SetIndexField(env, regexpResultClass, regexpResultObject, execResult.index));
    ANI_FATAL_IF_ERROR(SetResultField(env, regexpResultClass, regexpResultObject, execResult.indices));
    ANI_FATAL_IF_ERROR(SetGroupKeysField(env, regexpResultClass, regexpResultObject, execResult.namedGroups));
    ANI_FATAL_IF_ERROR(SetGroupValsField(env, regexpResultClass, regexpResultObject, execResult.namedGroups));
    return ANI_OK;
}

ani_status CreateRegExpResultObject(ani_env *env, ani_class regexpResultClass, ani_method regexpResultCtor,
                                    const RegExpExecResult &execResult, ani_object *out)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_RETURN_ON_PENDING_ERROR(env->Object_New(regexpResultClass, regexpResultCtor, out));
    return PopulateRegExpResultObject(env, regexpResultClass, *out, execResult);
}

}  // namespace ark::ets::stdlib
