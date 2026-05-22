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

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 0
#include "pcre2.h"

#include <string>
#include <vector>

namespace ark::ets::stdlib {

namespace {

constexpr uint32_t UNMAPPED_INDEX = static_cast<uint32_t>(-1);
constexpr uint32_t OVECTOR_GROUP_SIZE = 2;
constexpr uint32_t NAME_ENTRY_GROUP_NUM_SIZE = 2;
constexpr uint32_t NAME_ENTRY_GROUP_SHIFT = 8;
constexpr uint32_t UTF16_CODE_UNIT_SIZE = 2;
constexpr ani_int INVALID_CAPTURE_OFFSET = -1;

struct MatchMeta {
    uint32_t ecmaCaptureCount = 0;
    std::vector<uint32_t> pcre2ToEcma;
    std::vector<std::pair<uint32_t, uint32_t>> sanitizePairs;
    std::vector<std::string> groupNames;
    bool hasNamedGroups = false;
};

struct FieldCache {
    ani_field isCorrect = nullptr;
    ani_field index = nullptr;
    ani_field result = nullptr;
    ani_field groupKeys = nullptr;
    ani_field groupVals = nullptr;
    ani_class stringClass = nullptr;
};

template <typename CharT>
struct MatchDataGuard {
    using MdType = std::conditional_t<std::is_same_v<CharT, uint8_t>, pcre2_match_data_8, pcre2_match_data_16>;

    explicit MatchDataGuard(MdType *md) : data_(md) {}
    ~MatchDataGuard()
    {
        if (data_ != nullptr) {
            if constexpr (std::is_same_v<CharT, uint8_t>) {
                pcre2_match_data_free_8(data_);
            } else {
                pcre2_match_data_free_16(data_);
            }
        }
    }

    MatchDataGuard(const MatchDataGuard &) = delete;
    MatchDataGuard &operator=(const MatchDataGuard &) = delete;
    MatchDataGuard(MatchDataGuard &&) = delete;
    MatchDataGuard &operator=(MatchDataGuard &&) = delete;

    MdType *Get() const
    {
        return data_;
    }

private:
    MdType *data_ = nullptr;
};

template <typename CharT>
struct NameTableParser;

template <>
struct NameTableParser<uint8_t> {
    static int GetGroupNumber(const uint8_t *entry)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return static_cast<int32_t>(static_cast<PCRE2_UCHAR8>(entry[0] << NAME_ENTRY_GROUP_SHIFT) | entry[1]);
    }

    static std::string GetName(const uint8_t *entry, int entrySize)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto charPtr = reinterpret_cast<const char *>(entry + NAME_ENTRY_GROUP_NUM_SIZE);
        auto size = static_cast<size_t>(entrySize - NAME_ENTRY_GROUP_NUM_SIZE - 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        while (size > 0 && static_cast<uint8_t>(*(charPtr + size - 1)) == 0) {
            --size;
        }
        return std::string(charPtr, size);
    }
};

template <>
struct NameTableParser<uint16_t> {
    static int GetGroupNumber(const uint16_t *entry)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return static_cast<int32_t>(entry[0]);
    }

    static std::string GetName(const uint16_t *entry, int entrySize)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto charPtr = reinterpret_cast<const char *>(entry + 1);
        size_t size =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            static_cast<size_t>(entrySize) * UTF16_CODE_UNIT_SIZE - NAME_ENTRY_GROUP_NUM_SIZE - UTF16_CODE_UNIT_SIZE;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        while (size > 0 && static_cast<uint8_t>(*(charPtr + size - UTF16_CODE_UNIT_SIZE)) == 0) {
            size -= UTF16_CODE_UNIT_SIZE;
        }
        std::string key16(charPtr, size);
        std::string key;
        key.reserve(key16.size() / UTF16_CODE_UNIT_SIZE);
        for (size_t j = 0; j < key16.size(); j += UTF16_CODE_UNIT_SIZE) {
            key += key16[j];
        }
        return key;
    }
};

template <typename CharT>
void ParseNameTable(MatchMeta &meta, void *nameTablePtr, int nameCount, int nameEntrySize)
{
    using Parser = NameTableParser<CharT>;
    auto tabPtr = reinterpret_cast<const CharT *>(nameTablePtr);
    for (int i = 0; i < nameCount; ++i) {
        int n = Parser::GetGroupNumber(tabPtr);
        auto key = Parser::GetName(tabPtr, nameEntrySize);
        if (static_cast<size_t>(n) < meta.groupNames.size()) {
            meta.groupNames[n] = std::move(key);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        tabPtr += nameEntrySize;
    }
}

template <typename CharT>
// CC-OFFNXT(G.FUN.01, huge_method) solid logic
MatchMeta BuildMatchMeta(EtsRegExp &re)
{
    MatchMeta meta;
    auto compiled = std::is_same_v<CharT, uint8_t> ? re.GetCompiledRe8() : re.GetCompiledRe16();
    const auto &countable = compiled->groupMeta.CountableGroups();

    meta.pcre2ToEcma.resize(countable.size(), UNMAPPED_INDEX);
    uint32_t ecmaIdx = 0;
    for (size_t i = 0; i < countable.size(); ++i) {
        if (i == 0 || countable[i]) {
            meta.pcre2ToEcma[i] = ecmaIdx++;
        }
    }
    meta.ecmaCaptureCount = ecmaIdx > 0 ? ecmaIdx - 1 : 0;

    for (const auto [child, parent] : compiled->groupMeta.ParentGroups()) {
        if (child >= countable.size() || parent >= countable.size()) {
            continue;
        }
        if (!countable[child] || !countable[parent]) {
            continue;
        }
        meta.sanitizePairs.emplace_back(meta.pcre2ToEcma[child], meta.pcre2ToEcma[parent]);
    }

    using CodeType = std::conditional_t<std::is_same_v<CharT, uint8_t>, pcre2_code_8, pcre2_code_16>;
    auto *code = reinterpret_cast<CodeType *>(compiled->pcre2Code);
    int nameCount = 0;
    if constexpr (std::is_same_v<CharT, uint8_t>) {
        pcre2_pattern_info_8(code, PCRE2_INFO_NAMECOUNT, &nameCount);
    } else {
        pcre2_pattern_info_16(code, PCRE2_INFO_NAMECOUNT, &nameCount);
    }
    if (nameCount > 0) {
        uint32_t captureCount = 0;
        if constexpr (std::is_same_v<CharT, uint8_t>) {
            pcre2_pattern_info_8(code, PCRE2_INFO_CAPTURECOUNT, &captureCount);
        } else {
            pcre2_pattern_info_16(code, PCRE2_INFO_CAPTURECOUNT, &captureCount);
        }
        meta.groupNames.resize(captureCount + 1);
        void *nameTablePtr = nullptr;
        int nameEntrySize = 0;
        if constexpr (std::is_same_v<CharT, uint8_t>) {
            pcre2_pattern_info_8(code, PCRE2_INFO_NAMETABLE, &nameTablePtr);
            pcre2_pattern_info_8(code, PCRE2_INFO_NAMEENTRYSIZE, &nameEntrySize);
        } else {
            pcre2_pattern_info_16(code, PCRE2_INFO_NAMETABLE, &nameTablePtr);
            pcre2_pattern_info_16(code, PCRE2_INFO_NAMEENTRYSIZE, &nameEntrySize);
        }
        ParseNameTable<CharT>(meta, nameTablePtr, nameCount, nameEntrySize);
        meta.hasNamedGroups = true;
    }
    return meta;
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
ani_status PopulateFromOvector(ani_env *env, const FieldCache &fields, ani_object obj, const PCRE2_SIZE *ovector,
                               const MatchMeta &meta)
{
    ANI_FATAL_IF_ERROR(env->Object_SetField_Boolean(obj, fields.isCorrect, true));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(obj, fields.index, static_cast<ani_int>(ovector[0])));

    const uint32_t totalSlots = meta.ecmaCaptureCount + 1;
    const uint32_t flatSize = totalSlots * OVECTOR_GROUP_SIZE;
    if (flatSize > 0) {
        ani_fixedarray_int intArray;
        ANI_RETURN_ON_PENDING_ERROR(env->FixedArray_New_Int(flatSize, &intArray));
        std::vector<ani_int> flat(flatSize);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        flat[0] = static_cast<ani_int>(ovector[0]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        flat[1] = static_cast<ani_int>(ovector[1]);
        uint32_t ecmaIdx = 1;
        uint32_t captureIdx = 1;
        for (uint32_t groupId = 1; groupId < meta.pcre2ToEcma.size() && ecmaIdx < totalSlots; ++groupId) {
            if (meta.pcre2ToEcma[groupId] == UNMAPPED_INDEX) {
                continue;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            flat[OVECTOR_GROUP_SIZE * ecmaIdx] = static_cast<ani_int>(ovector[OVECTOR_GROUP_SIZE * captureIdx]);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            flat[OVECTOR_GROUP_SIZE * ecmaIdx + 1] = static_cast<ani_int>(ovector[OVECTOR_GROUP_SIZE * captureIdx + 1]);
            ++ecmaIdx;
            ++captureIdx;
        }
        for (auto [childIdx, parentIdx] : meta.sanitizePairs) {
            if (childIdx >= totalSlots || parentIdx >= totalSlots) {
                continue;
            }
            if (flat[OVECTOR_GROUP_SIZE * childIdx] < flat[OVECTOR_GROUP_SIZE * parentIdx] ||
                flat[OVECTOR_GROUP_SIZE * childIdx + 1] > flat[OVECTOR_GROUP_SIZE * parentIdx + 1]) {
                flat[OVECTOR_GROUP_SIZE * childIdx] = INVALID_CAPTURE_OFFSET;
                flat[OVECTOR_GROUP_SIZE * childIdx + 1] = INVALID_CAPTURE_OFFSET;
            }
        }
        ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Int(intArray, 0, flatSize, flat.data()));
        ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(obj, fields.result, static_cast<ani_ref>(intArray)));
    }

    if (meta.hasNamedGroups) {
        std::vector<std::string> keys;
        std::vector<ani_int> vals;
        for (size_t pcreIdx = 1; pcreIdx < meta.groupNames.size(); ++pcreIdx) {
            if (meta.groupNames[pcreIdx].empty()) {
                continue;
            }
            keys.push_back(meta.groupNames[pcreIdx]);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            vals.push_back(static_cast<ani_int>(ovector[OVECTOR_GROUP_SIZE * pcreIdx]));
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            vals.push_back(static_cast<ani_int>(ovector[OVECTOR_GROUP_SIZE * pcreIdx + 1]));
        }
        if (!keys.empty()) {
            ani_string firstKey;
            ANI_RETURN_ON_PENDING_ERROR(env->String_NewUTF8(keys[0].c_str(), keys[0].size(), &firstKey));
            ani_fixedarray_ref keysArray;
            ANI_RETURN_ON_PENDING_ERROR(env->FixedArray_New_Ref(fields.stringClass, keys.size(), firstKey, &keysArray));
            for (size_t i = 1; i < keys.size(); ++i) {
                ani_string key;
                ANI_RETURN_ON_PENDING_ERROR(env->String_NewUTF8(keys[i].c_str(), keys[i].size(), &key));
                ANI_FATAL_IF_ERROR(env->FixedArray_Set_Ref(keysArray, i, key));
            }
            ani_fixedarray_int valsArray;
            ANI_RETURN_ON_PENDING_ERROR(env->FixedArray_New_Int(vals.size(), &valsArray));
            ANI_FATAL_IF_ERROR(env->FixedArray_SetRegion_Int(valsArray, 0, vals.size(), vals.data()));
            ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(obj, fields.groupKeys, static_cast<ani_ref>(keysArray)));
            ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(obj, fields.groupVals, static_cast<ani_ref>(valsArray)));
        }
    }
    return ANI_OK;
}

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
        return status == ANI_OK ? arr : nullptr;
    }

    FieldCache fields;
    ANI_FATAL_IF_ERROR(env->Class_FindField(resultClass, IS_CORRECT_FIELD_NAME, &fields.isCorrect));
    ANI_FATAL_IF_ERROR(env->Class_FindField(resultClass, "index_", &fields.index));
    ANI_FATAL_IF_ERROR(env->Class_FindField(resultClass, "resultRaw_", &fields.result));
    ANI_FATAL_IF_ERROR(env->Class_FindField(resultClass, "groupKeysRaw_", &fields.groupKeys));
    ANI_FATAL_IF_ERROR(env->Class_FindField(resultClass, "groupValsRaw_", &fields.groupVals));
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.String", &fields.stringClass));

    auto meta = BuildMatchMeta<CharT>(re);
    auto compiled = std::is_same_v<CharT, uint8_t> ? re.GetCompiledRe8() : re.GetCompiledRe16();
    auto *code = compiled->pcre2Code;

    using MdType = std::conditional_t<std::is_same_v<CharT, uint8_t>, pcre2_match_data_8, pcre2_match_data_16>;
    MdType *md = nullptr;
    if constexpr (std::is_same_v<CharT, uint8_t>) {
        md = pcre2_match_data_create_from_pattern_8(reinterpret_cast<pcre2_code_8 *>(code), nullptr);
    } else {
        md = pcre2_match_data_create_from_pattern_16(reinterpret_cast<pcre2_code_16 *>(code), nullptr);
    }
    if (md == nullptr) {
        return nullptr;
    }
    MatchDataGuard<CharT> mdGuard(md);

    bool firstIter = true;
    while (lastIndex <= inputSize) {
        int rc = 0;
        if constexpr (std::is_same_v<CharT, uint8_t>) {
            rc = pcre2_match_8(reinterpret_cast<pcre2_code_8 *>(code), reinterpret_cast<PCRE2_SPTR8>(input),
                               static_cast<PCRE2_SIZE>(inputSize), static_cast<PCRE2_SIZE>(lastIndex), matchFlags, md,
                               nullptr);
        } else {
            rc = pcre2_match_16(reinterpret_cast<pcre2_code_16 *>(code), reinterpret_cast<PCRE2_SPTR16>(input),
                                static_cast<PCRE2_SIZE>(inputSize), static_cast<PCRE2_SIZE>(lastIndex), matchFlags, md,
                                nullptr);
        }
        if (rc < 0) {
            break;
        }

        const PCRE2_SIZE *ovector = nullptr;
        if constexpr (std::is_same_v<CharT, uint8_t>) {
            ovector = pcre2_get_ovector_pointer_8(md);
        } else {
            ovector = pcre2_get_ovector_pointer_16(md);
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto matchStart = static_cast<int32_t>(ovector[0]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto matchEnd = static_cast<int32_t>(ovector[1]);

        ani_object resultObj = nullptr;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        if (env->Object_New(resultClass, resultCtor, &resultObj) != ANI_OK) {
            return nullptr;
        }
        if (PopulateFromOvector(env, fields, resultObj, ovector, meta) != ANI_OK) {
            return nullptr;
        }
        results.push_back(resultObj);

        lastIndex = matchEnd;
        if (matchStart == matchEnd) {
            lastIndex = AdvanceIndex(input, inputSize, lastIndex, unicode);
        }

        if (firstIter) {
            matchFlags |= PCRE2_NO_UTF_CHECK;
            firstIter = false;
        }
    }

    ani_array arr = nullptr;
    ani_status status = CreateResultArray(env, results, &arr);
    return status == ANI_OK ? arr : nullptr;
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
