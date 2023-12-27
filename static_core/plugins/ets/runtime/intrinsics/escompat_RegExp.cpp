/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/regexp/regexp_executor.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_void.h"
#include "runtime/regexp/ecmascript/regexp_parser.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/handle_scope-inl.h"

namespace panda::ets::intrinsics {
using RegExpParser = panda::RegExpParser;
using RegExpExecutor = panda::ets::RegExpExecutor;
using RegExpMatchResult = panda::RegExpMatchResult<VMHandle<EtsString>>;
using Array = panda::coretypes::Array;

namespace {
EtsObject *GetFieldObjectByName(EtsObject *object, const char *name)
{
    auto *cls = object->GetClass();
    EtsField *field = cls->GetDeclaredFieldIDByName(name);
    ASSERT(field != nullptr);
    return object->GetFieldObject(field);
}

uint32_t CastToBitMask(EtsString *checkStr)
{
    uint32_t flagsBits = 0;
    uint32_t flagsBitsTemp = 0;
    for (int i = 0; i < checkStr->GetLength(); i++) {
        switch (checkStr->At(i)) {
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
                panda::ThrowException(ctx, thread, utf::CStringAsMutf8("Lstd/core/IllegalArgumentException;"),
                                      utf::CStringAsMutf8(message.c_str()));
                return 0;
            }
        }
        if ((flagsBits & flagsBitsTemp) != 0) {
            auto *thread = ManagedThread::GetCurrent();
            auto ctx = PandaEtsVM::GetCurrent()->GetLanguageContext();
            std::string message = "invalid regular expression flags";
            panda::ThrowException(ctx, thread, utf::CStringAsMutf8("Lstd/core/IllegalArgumentException;"),
                                  utf::CStringAsMutf8(message.c_str()));
            return 0;
        }
        flagsBits |= flagsBitsTemp;
    }
    return flagsBits;
}
}  // namespace

extern "C" EtsVoid *EscompatRegExpCompile(ObjectHeader *obj)
{
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto stringClass = classLinker->GetClassRoot(EtsClassRoot::STRING);
    auto regexpObject = EtsObject::FromCoreType(obj);
    VMHandle<EtsObject> regObjHandle(thread, regexpObject->GetCoreType());
    auto regexpClass = regObjHandle.GetPtr()->GetClass();

    EtsField *groupNamesField = regexpClass->GetDeclaredFieldIDByName("groupNames");
    EtsString *patternStr = EtsString::FromEtsObject(GetFieldObjectByName(regObjHandle.GetPtr(), "pattern"));
    VMHandle<coretypes::String> sHandle(thread, patternStr->GetCoreType());

    EtsString *flags = EtsString::FromEtsObject(GetFieldObjectByName(regObjHandle.GetPtr(), "flags"));
    auto flagsBits = static_cast<uint8_t>(CastToBitMask(flags));

    RegExpParser parser = RegExpParser();
    parser.Init(reinterpret_cast<char *>(sHandle.GetPtr()->GetDataMUtf8()), sHandle.GetPtr()->GetLength(), flagsBits);
    parser.Parse();

    PandaVector<PandaString> groupName = parser.GetGroupNames();

    EtsObjectArray *etsGroupNames = EtsObjectArray::Create(stringClass, groupName.size());
    VMHandle<EtsObjectArray> arrHandle(thread, etsGroupNames->GetCoreType());

    for (size_t i = 0; i < groupName.size(); ++i) {
        EtsString *str = EtsString::CreateFromMUtf8(groupName[i].c_str(), groupName[i].size());
        arrHandle.GetPtr()->Set(i, str->AsObject());
    }
    regObjHandle.GetPtr()->SetFieldObject(groupNamesField, arrHandle.GetPtr()->AsObject());

    auto bufferSize = parser.GetOriginBufferSize();
    auto buffer = parser.GetOriginBuffer();
    EtsField *bufferField = regexpClass->GetDeclaredFieldIDByName("buffer");
    EtsByteArray *etsBuffer = EtsByteArray::Create(bufferSize);
    for (size_t i = 0; i < bufferSize; ++i) {
        etsBuffer->Set(i, buffer[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    regObjHandle.GetPtr()->SetFieldObject(bufferField, etsBuffer->AsObject());

    return EtsVoid::GetInstance();
}

extern "C" EtsObject *EscompatRegExpExec(ObjectHeader *obj, EtsString *str)
{
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<EtsString> strHandle(thread, str->GetCoreType());

    auto regexpObject = EtsObject::FromCoreType(obj);
    VMHandle<EtsObject> regObjHandle(thread, regexpObject->GetCoreType());

    auto regexpClass = regObjHandle.GetPtr()->GetClass();
    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto stringClass = classLinker->GetClassRoot(EtsClassRoot::STRING);

    EtsField *lastIndexField = regexpClass->GetDeclaredFieldIDByName("lastIndex");
    auto lastIndex = regObjHandle.GetPtr()->GetFieldPrimitive<int32_t>(lastIndexField);
    EtsString *flags = EtsString::FromEtsObject(GetFieldObjectByName(regObjHandle.GetPtr(), "flags"));
    auto flagsBits = static_cast<uint8_t>(CastToBitMask(flags));

    auto resultClass = classLinker->GetClass("Lescompat/RegExpExecResult;");
    auto resultObject = EtsObject::Create(resultClass);
    VMHandle<EtsObject> objHandle(thread, resultObject->GetCoreType());

    auto resultCorrectField = resultClass->GetDeclaredFieldIDByName("isCorrect");

    bool global = (flagsBits & RegExpParser::FLAG_GLOBAL) > 0;
    bool sticky = (flagsBits & RegExpParser::FLAG_STICKY) > 0;
    if (!global && !sticky) {
        lastIndex = 0;
    }

    int32_t stringLength = strHandle.GetPtr()->GetLength();
    if (lastIndex > stringLength) {
        if (global || sticky) {
            regObjHandle.GetPtr()->SetFieldPrimitive(lastIndexField, 0);
        }
        objHandle.GetPtr()->SetFieldPrimitive(resultCorrectField, false);
        return objHandle.GetPtr();
    }

    RegExpExecutor executor = RegExpExecutor();
    PandaVector<uint8_t> u8Buffer;
    PandaVector<uint16_t> u16Buffer;
    const uint8_t *strBuffer;
    bool isUtf16 = strHandle.GetPtr()->IsUtf16();
    if (isUtf16) {
        u16Buffer = PandaVector<uint16_t>(stringLength);
        strHandle.GetPtr()->CopyDataUtf16(u16Buffer.data(), stringLength);
        strBuffer = reinterpret_cast<uint8_t *>(u16Buffer.data());
    } else {
        u8Buffer = PandaVector<uint8_t>(stringLength + 1);
        strHandle.GetPtr()->CopyDataMUtf8(u8Buffer.data(), stringLength + 1, true);
        strBuffer = u8Buffer.data();
    }

    auto etsBuffer = reinterpret_cast<EtsByteArray *>(GetFieldObjectByName(regObjHandle.GetPtr(), "buffer"));
    auto buffer = reinterpret_cast<uint8_t *>(etsBuffer->GetData<int8_t>());
    bool ret = executor.Execute(strBuffer, lastIndex, stringLength, buffer, isUtf16);
    RegExpMatchResult execResult = executor.GetResult(ret);
    if (!execResult.isSuccess) {
        if (global || sticky) {
            regObjHandle.GetPtr()->SetFieldPrimitive(lastIndexField, 0);
        }
        objHandle.GetPtr()->SetFieldPrimitive<bool>(resultCorrectField, false);
        return objHandle.GetPtr();
    }

    uint32_t endIndex = execResult.endIndex;
    if (global || sticky) {
        regObjHandle.GetPtr()->SetFieldPrimitive(lastIndexField, endIndex);
    }
    // isCorrect field
    objHandle.GetPtr()->SetFieldPrimitive<bool>(resultCorrectField, true);
    // index field
    auto indexField = resultClass->GetDeclaredFieldIDByName("index");
    objHandle.GetPtr()->SetFieldPrimitive<int32_t>(indexField, execResult.index);
    // input field
    auto inputField = resultClass->GetDeclaredFieldIDByName("input");
    objHandle.GetPtr()->SetFieldObject(inputField, strHandle.GetPtr()->AsObject());
    // result field
    auto resultField = resultClass->GetDeclaredFieldIDByName("result");
    uint32_t capturesSize = execResult.captures.size();
    PandaVector<VMHandle<EtsString>> matches;
    for (size_t i = 0; i < capturesSize; ++i) {
        if (!execResult.captures[i].first) {
            matches.push_back(execResult.captures[i].second);
        }
    }
    EtsObjectArray *resultArray = EtsObjectArray::Create(stringClass, matches.size());
    for (size_t i = 0; i < matches.size(); i++) {
        resultArray->Set(i, matches[i]->AsObject());
    }
    objHandle.GetPtr()->SetFieldObject(resultField, resultArray->AsObject());
    return objHandle.GetPtr();
}
}  // namespace panda::ets::intrinsics
