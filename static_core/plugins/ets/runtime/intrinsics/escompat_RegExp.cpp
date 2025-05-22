/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <iterator>
#include "ets_class_root.h"
#include "ets_vm.h"
#include "include/mem/panda_containers.h"
#include "macros.h"
#include "mem/vm_handle.h"
#include "plugins/ets/runtime/regexp/regexp_executor.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "runtime/regexp/ecmascript/regexp_parser.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/handle_scope-inl.h"
#include "types/ets_object.h"
#include "types/ets_primitives.h"
#include <array>
#include "plugins/ets/runtime/regexp/regexp.h"

namespace ark::ets::intrinsics {
using RegExpParser = ark::RegExpParser;
using RegExpExecutor = ark::ets::RegExpExecutor;
using RegExpMatchResult = ark::RegExpMatchResult<PandaString>;
using Array = ark::coretypes::Array;

namespace {

constexpr const int LAST_PAREN_FIELDS_COUNT = 10;

constexpr const char *GROUP_NAMES_FIELD_NAME = "groupNames";
constexpr const char *LAST_INDEX_FIELD_NAME = "lastIndex";
constexpr const char *FLAGS_FIELD_NAME = "flags_";
constexpr std::array<const char *, LAST_PAREN_FIELDS_COUNT> PAREN_FIELD_NAMES = {"",    "$1_", "$2_", "$3_", "$4_",
                                                                                 "$5_", "$6_", "$7_", "$8_", "$9_"};
constexpr const char *LAST_MATCH_FIELD_NAME = "lastMatch_";
constexpr const char *INPUT_STATIC_FIELD_NAME = "input_";
constexpr const char *LAST_PAREN_FIELD_NAME = "lastParen_";
constexpr const char *LEFT_CONTEXT_FIELD_NAME = "leftContext_";
constexpr const char *RIGHT_CONTEXT_FIELD_NAME = "rightContext_";

constexpr const char *RESULT_CLASS_NAME = "Lescompat/RegExpExecArray;";
constexpr const char *INDEX_FIELD_NAME = "index";
constexpr const char *INPUT_FIELD_NAME = "input";
constexpr const char *INDICES_FIELD_NAME = "indices";
constexpr const char *RESULT_FIELD_NAME = "result";
constexpr const char *IS_CORRECT_FIELD_NAME = "isCorrect";

constexpr const uint32_t INDICES_DIMENSIONS_NUM = 2;

uint32_t CastToBitMask(EtsString *checkStr)
{
    uint32_t flagsBits = 0;
    uint32_t flagsBitsTemp = 0;
    for (int i = 0; i < checkStr->GetLength(); i++) {
        switch (checkStr->At(i)) {
            case 'd':
                flagsBitsTemp = RegExpParser::FLAG_HASINDICES;
                break;
            case 'g':
                flagsBitsTemp = RegExpParser::FLAG_GLOBAL;
                break;
            case 'i':
                flagsBitsTemp = RegExpParser::FLAG_IGNORECASE;
                break;
            case 'm':
                flagsBitsTemp = RegExpParser::FLAG_MULTILINE;
                break;
            case 's':
                flagsBitsTemp = RegExpParser::FLAG_DOTALL;
                break;
            case 'u':
                flagsBitsTemp = RegExpParser::FLAG_UTF16;
                break;
            case 'y':
                flagsBitsTemp = RegExpParser::FLAG_STICKY;
                break;
            default: {
                auto *thread = ManagedThread::GetCurrent();
                auto ctx = PandaEtsVM::GetCurrent()->GetLanguageContext();
                std::string message = "invalid regular expression flags";
                ark::ThrowException(ctx, thread, utf::CStringAsMutf8("Lstd/core/IllegalArgumentException;"),
                                    utf::CStringAsMutf8(message.c_str()));
                return 0;
            }
        }
        if ((flagsBits & flagsBitsTemp) != 0) {
            auto *thread = ManagedThread::GetCurrent();
            auto ctx = PandaEtsVM::GetCurrent()->GetLanguageContext();
            std::string message = "invalid regular expression flags";
            ark::ThrowException(ctx, thread, utf::CStringAsMutf8("Lstd/core/IllegalArgumentException;"),
                                utf::CStringAsMutf8(message.c_str()));
            return 0;
        }
        flagsBits |= flagsBitsTemp;
    }
    return flagsBits;
}
}  // namespace

void SetFlags(EtsObject *regexpObject, EtsString *checkStr)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> regexp(coroutine, regexpObject->GetCoreType());

    auto *regexpClass = regexp->GetClass();
    EtsField *flagsField = regexpClass->GetDeclaredFieldIDByName(FLAGS_FIELD_NAME);

    auto flags = checkStr->GetMutf8();
    std::sort(flags.begin(), flags.end());
    VMHandle<EtsString> newFlags(coroutine, EtsString::CreateFromMUtf8(flags.c_str())->GetCoreType());
    regexp->SetFieldObject(flagsField, newFlags->AsObject());
}

void SetGroupNames(EtsObject *regexpObject, const RegExpParser &parser)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> regexp(coroutine, regexpObject->GetCoreType());
    auto *regexpClass = regexp->GetClass();

    auto *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto *stringClass = classLinker->GetClassRoot(EtsClassRoot::STRING);

    auto groupName = parser.GetGroupNames();
    EtsObjectArray *etsGroupNames = EtsObjectArray::Create(stringClass, groupName.size());
    VMHandle<EtsObjectArray> arrHandle(coroutine, etsGroupNames->GetCoreType());

    EtsField *groupNamesField = regexpClass->GetDeclaredFieldIDByName(GROUP_NAMES_FIELD_NAME);
    for (size_t i = 0; i < groupName.size(); ++i) {
        VMHandle<EtsString> str(coroutine,
                                EtsString::CreateFromMUtf8(groupName[i].c_str(), groupName[i].size())->GetCoreType());
        arrHandle.GetPtr()->Set(i, str->AsObject());
    }
    regexp.GetPtr()->SetFieldObject(groupNamesField, arrHandle.GetPtr()->AsObject());
}

extern "C" EtsObject *EscompatRegExpCompile(EtsObject *regexpObj)
{
    // NOTE(kparshukov): remove #20259
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    VMHandle<EtsObject> regexp(coroutine, regexpObj->GetCoreType());
    return regexp.GetPtr();
}

extern "C" EtsString *EscompatRegExpParse(EtsString *pattern)
{
    RegExpParser parse = RegExpParser();
    auto patternstr = ark::PandaStringToStd(pattern->GetUtf8());
    parse.Init(const_cast<char *>(reinterpret_cast<const char *>(patternstr.c_str())), patternstr.length(), 0);
    parse.Parse();
    if (parse.IsError()) {
        auto errormsg = ark::PandaStringToStd(parse.GetErrorMsg());
        return EtsString::CreateFromMUtf8(errormsg.c_str(), errormsg.length());
    }
    return nullptr;
}

void SetSuccessfulMatchLegacyProperties(EtsClass *type, const EtsObject *regexpExecArrayObj, EtsString *inputStrObj,
                                        uint32_t index)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    EtsClass *resultClass = regexpExecArrayObj->GetClass();
    auto *resultField = resultClass->GetDeclaredFieldIDByName(RESULT_FIELD_NAME);
    VMHandle<EtsObjectArray> matches(coroutine, regexpExecArrayObj->GetFieldObject(resultField)->GetCoreType());
    ASSERT(matches->GetLength() != 0);

    VMHandle<EtsString> inputStr(coroutine, inputStrObj->GetCoreType());
    VMHandle<EtsString> emptyString(coroutine, EtsString::CreateNewEmptyString()->GetCoreType());

    EtsField *lastMatchField = type->GetStaticFieldIDByName(LAST_MATCH_FIELD_NAME);
    type->SetStaticFieldObject(lastMatchField, matches->Get(0U));
    for (size_t i = 1; i < LAST_PAREN_FIELDS_COUNT; ++i) {
        EtsField *parenField = type->GetStaticFieldIDByName(PAREN_FIELD_NAMES[i]);
        if (i < matches->GetLength()) {
            type->SetStaticFieldObject(parenField, matches->Get(i));
        } else {
            type->SetStaticFieldObject(parenField, emptyString->AsObject());
        }
    }

    EtsField *inputField = type->GetStaticFieldIDByName(INPUT_STATIC_FIELD_NAME);
    type->SetStaticFieldObject(inputField, inputStr->AsObject());

    EtsField *lastParenField = type->GetStaticFieldIDByName(LAST_PAREN_FIELD_NAME);
    if (matches->GetLength() > 1) {
        type->SetStaticFieldObject(lastParenField, matches->Get(matches->GetLength() - 1U));
    } else {
        type->SetStaticFieldObject(lastParenField, emptyString->AsObject());
    }

    EtsField *leftContextField = type->GetStaticFieldIDByName(LEFT_CONTEXT_FIELD_NAME);
    EtsString *prefix = EtsString::FastSubString(inputStr.GetPtr(), 0, index);
    type->SetStaticFieldObject(leftContextField, prefix->AsObject());

    EtsField *rightContextField = type->GetStaticFieldIDByName(RIGHT_CONTEXT_FIELD_NAME);
    auto suffixBegin = index + EtsString::FromEtsObject(matches->Get(0U))->GetLength();
    EtsString *suffix = EtsString::FastSubString(inputStr.GetPtr(), suffixBegin, inputStr->GetLength() - suffixBegin);
    type->SetStaticFieldObject(rightContextField, suffix->AsObject());
}

void SetUnsuccessfulMatchLegacyProperties(EtsClass *type)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> emptyString(coroutine, EtsString::CreateNewEmptyString()->GetCoreType());

    {
        EtsField *lastMatchField = type->GetStaticFieldIDByName(LAST_MATCH_FIELD_NAME);
        type->SetStaticFieldObject(lastMatchField, emptyString->AsObject());
    }
    for (size_t i = 1; i < LAST_PAREN_FIELDS_COUNT; ++i) {
        EtsField *lastParenField = type->GetStaticFieldIDByName(PAREN_FIELD_NAMES[i]);
        type->SetStaticFieldObject(lastParenField, emptyString->AsObject());
    }
}

PandaVector<uint8_t> ExtractString(EtsString *from, const bool asUtf16)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> inputStr(coroutine, from->GetCoreType());
    PandaVector<uint8_t> result;
    if (asUtf16) {
        const auto size = inputStr.GetPtr()->GetLength();
        PandaVector<uint16_t> u16Buffer(size);
        inputStr.GetPtr()->CopyDataUtf16(u16Buffer.data(), size);
        auto strBuffer = reinterpret_cast<uint8_t *>(u16Buffer.data());
        const auto newSize = size * 2;
        for (int i = 0; i < newSize; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            result.push_back(strBuffer[i]);
        }
        result.emplace_back(0U);
        result.emplace_back(0U);
    } else {
        const size_t vecLen = inputStr.GetPtr()->GetLength();
        result.resize(vecLen);
        const auto data = inputStr.GetPtr()->GetDataMUtf8();
        for (size_t i = 0U; i < vecLen; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            result[i] = data[i];
        }
        result.emplace_back(0U);
    }
    return result;
}

RegExpMatchResult Execute(EtsString *pattern, EtsString *flags, EtsString *inputStrObj, EtsInt lastIndex,
                          bool hasIndices)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> patternHandle(coroutine, pattern->GetCoreType());
    VMHandle<EtsString> flagsHandle(coroutine, flags->GetCoreType());
    VMHandle<EtsString> inputStr(coroutine, inputStrObj->GetCoreType());

    const auto stringLength = inputStr.GetPtr()->GetLength();
    const bool isUtf16Str = inputStr->IsUtf16();
    const bool isUtf16Pattern = patternHandle->IsUtf16();
    auto re = EtsRegExp();
    re.SetFlags(flagsHandle.GetPtr());
    const bool isUtf16 = isUtf16Str || isUtf16Pattern || re.IsUtf16();

    auto str = ExtractString(inputStr.GetPtr(), isUtf16);
    auto patternStr = ExtractString(patternHandle.GetPtr(), isUtf16);
    auto compiled = re.Compile(patternStr, isUtf16, patternHandle.GetPtr()->GetLength());
    if (!compiled) {
        RegExpMatchResult badResult;
        badResult.isSuccess = false;
        return badResult;
    }

    auto result = re.Execute(str, stringLength, lastIndex);
    re.Destroy();
    if (!hasIndices) {
        result.indices.clear();
    }
    return result;
}

void SetResultField(EtsObject *regexpExecArrayObj, const PandaVector<std::pair<bool, PandaString>> &matches,
                    bool isWide)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> regexpExecArray(coroutine, regexpExecArrayObj->GetCoreType());
    EtsClass *resultClass = regexpExecArray->GetClass();

    auto *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto *stringClass = classLinker->GetClassRoot(EtsClassRoot::STRING);

    auto *resultField = resultClass->GetDeclaredFieldIDByName(RESULT_FIELD_NAME);
    VMHandle<EtsObjectArray> resultArray(coroutine, EtsObjectArray::Create(stringClass, matches.size())->GetCoreType());
    VMHandle<EtsString> match;
    for (size_t i = 0; i < matches.size(); ++i) {
        if (isWide) {
            match = VMHandle<EtsString>(
                coroutine, EtsString::CreateFromUtf16(reinterpret_cast<const ets_char *>(matches[i].second.c_str()),
                                                      matches[i].second.length() / RegExpExecutor::WIDE_CHAR_SIZE)
                               ->GetCoreType());
        } else {
            match = VMHandle<EtsString>(
                coroutine, EtsString::CreateFromUtf8(reinterpret_cast<const char *>(matches[i].second.c_str()),
                                                     matches[i].second.length())
                               ->GetCoreType());
        }
        resultArray->Set(i, match->AsObject());
    }
    regexpExecArray->SetFieldObject(resultField, resultArray->AsObject());
}

void SetIndicesField(EtsObject *regexpExecArrayObj, const PandaVector<std::pair<uint32_t, uint32_t>> &indices,
                     bool hasIndices)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> regexpExecArray(coroutine, regexpExecArrayObj->GetCoreType());
    auto *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();

    EtsClass *resultClass = regexpExecArray->GetClass();
    auto *indicesField = resultClass->GetDeclaredFieldIDByName(INDICES_FIELD_NAME);

    if (!hasIndices) {
        VMHandle<EtsDoubleArray> defaultVal(coroutine, EtsDoubleArray::Create(0)->GetCoreType());
        regexpExecArray->SetFieldObject(indicesField, defaultVal->AsObject());
        return;
    }

    VMHandle<EtsObjectArray> indicesArray(
        coroutine,
        EtsObjectArray::Create(classLinker->GetClassRoot(EtsClassRoot::DOUBLE_ARRAY), indices.size())->GetCoreType());
    for (size_t i = 0; i < indices.size(); ++i) {
        VMHandle<EtsDoubleArray> index(coroutine, EtsDoubleArray::Create(INDICES_DIMENSIONS_NUM)->GetCoreType());
        index->Set(0, static_cast<EtsDouble>(indices[i].first));
        index->Set(1, static_cast<EtsDouble>(indices[i].second));
        indicesArray->Set(i, index->AsObject());
    }
    regexpExecArray->SetFieldObject(indicesField, indicesArray->AsObject());
}

void SetIsCorrectField(EtsObject *regexpExecArrayObj, bool value)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> regexpExecArray(coroutine, regexpExecArrayObj->GetCoreType());
    EtsClass *resultClass = regexpExecArray->GetClass();
    auto *resultCorrectField = resultClass->GetDeclaredFieldIDByName(IS_CORRECT_FIELD_NAME);

    regexpExecArray->SetFieldPrimitive<EtsBoolean>(resultCorrectField, ToEtsBoolean(value));
}

void SetInputField(EtsObject *regexpExecArrayObj, EtsString *inputStrObj)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> inputStr(coroutine, inputStrObj->GetCoreType());
    VMHandle<EtsObject> regexpExecArray(coroutine, regexpExecArrayObj->GetCoreType());
    EtsClass *resultClass = regexpExecArray->GetClass();
    auto *inputField = resultClass->GetDeclaredFieldIDByName(INPUT_FIELD_NAME);

    regexpExecArray->SetFieldObject(inputField, inputStr->AsObject());
}

void SetIndexField(EtsObject *regexpExecArrayObj, uint32_t index)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> regexpExecArray(coroutine, regexpExecArrayObj->GetCoreType());
    EtsClass *resultClass = regexpExecArray->GetClass();

    auto *indexField = resultClass->GetDeclaredFieldIDByName(INDEX_FIELD_NAME);
    regexpExecArray->SetFieldPrimitive<EtsDouble>(indexField, static_cast<EtsDouble>(index));
}

void SetLastIndexField(EtsObject *regexp, EtsField *lastIndexField, bool global, bool sticky, EtsDouble value)
{
    if (!global && !sticky) {
        return;
    }
    regexp->SetFieldPrimitive<EtsDouble>(lastIndexField, value);
}

extern "C" EtsObject *EscompatRegExpExec(EtsObject *obj, EtsString *patternStr, EtsString *flagsStr, EtsString *str)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();

    VMHandle<EtsObject> regexp(coroutine, obj->GetCoreType());
    VMHandle<EtsString> pattern(coroutine, patternStr->GetCoreType());
    VMHandle<EtsString> flags(coroutine, flagsStr->GetCoreType());
    VMHandle<EtsString> strHandle(coroutine, str->GetCoreType());
    auto *regexpExecArrayClass = classLinker->GetClass(RESULT_CLASS_NAME);
    VMHandle<EtsObject> regexpExecArrayObject(coroutine, EtsObject::Create(regexpExecArrayClass)->GetCoreType());

    auto *regexpClass = regexp->GetClass();

    EtsField *lastIndexField = regexpClass->GetDeclaredFieldIDByName(LAST_INDEX_FIELD_NAME);
    auto lastIdx = static_cast<int32_t>(regexp.GetPtr()->GetFieldPrimitive<EtsDouble>(lastIndexField));
    auto flagsBits = static_cast<uint8_t>(CastToBitMask(flags.GetPtr()));

    bool global = (flagsBits & RegExpParser::FLAG_GLOBAL) > 0;
    bool sticky = (flagsBits & RegExpParser::FLAG_STICKY) > 0;
    bool hasIndices = (flagsBits & RegExpParser::FLAG_HASINDICES) > 0;
    if ((!global && !sticky) || lastIdx < 0) {
        lastIdx = 0;
    }
    EtsInt strLen = strHandle->GetLength();
    if (lastIdx > strLen) {
        SetLastIndexField(regexp.GetPtr(), lastIndexField, global, sticky, 0.0);
        SetUnsuccessfulMatchLegacyProperties(regexpClass);
        SetIsCorrectField(regexpExecArrayObject.GetPtr(), false);
        return regexpExecArrayObject.GetPtr();
    }

    auto execResult = Execute(pattern.GetPtr(), flags.GetPtr(), strHandle.GetPtr(), lastIdx, hasIndices);
    if (!execResult.isSuccess) {
        SetLastIndexField(regexp.GetPtr(), lastIndexField, global, sticky, 0.0);
        SetUnsuccessfulMatchLegacyProperties(regexpClass);
        SetIsCorrectField(regexpExecArrayObject.GetPtr(), false);
        return regexpExecArrayObject.GetPtr();
    }

    SetLastIndexField(regexp.GetPtr(), lastIndexField, global, sticky, static_cast<EtsDouble>(execResult.endIndex));
    SetIsCorrectField(regexpExecArrayObject.GetPtr(), true);
    SetIndexField(regexpExecArrayObject.GetPtr(), execResult.index);
    SetInputField(regexpExecArrayObject.GetPtr(), strHandle.GetPtr());
    SetResultField(regexpExecArrayObject.GetPtr(), execResult.captures, execResult.isWide);
    SetSuccessfulMatchLegacyProperties(regexpClass, regexpExecArrayObject.GetPtr(), strHandle.GetPtr(),
                                       execResult.index);
    SetIndicesField(regexpExecArrayObject.GetPtr(), execResult.indices, hasIndices);
    return regexpExecArrayObject.GetPtr();
}
}  // namespace ark::ets::intrinsics
