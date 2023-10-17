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

uint32_t CastToBitMask(EtsString *check_str)
{
    uint32_t flags_bits = 0;
    uint32_t flags_bits_temp = 0;
    for (int i = 0; i < check_str->GetLength(); i++) {
        switch (check_str->At(i)) {
            case 'g':
                flags_bits_temp = RegExpParser::FLAG_GLOBAL;
                break;
            case 'i':
                flags_bits_temp = RegExpParser::FLAG_IGNORECASE;
                break;
            case 'm':
                flags_bits_temp = RegExpParser::FLAG_MULTILINE;
                break;
            case 's':
                flags_bits_temp = RegExpParser::FLAG_DOTALL;
                break;
            case 'u':
                flags_bits_temp = RegExpParser::FLAG_UTF16;
                break;
            case 'y':
                flags_bits_temp = RegExpParser::FLAG_STICKY;
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
        if ((flags_bits & flags_bits_temp) != 0) {
            auto *thread = ManagedThread::GetCurrent();
            auto ctx = PandaEtsVM::GetCurrent()->GetLanguageContext();
            std::string message = "invalid regular expression flags";
            panda::ThrowException(ctx, thread, utf::CStringAsMutf8("Lstd/core/IllegalArgumentException;"),
                                  utf::CStringAsMutf8(message.c_str()));
            return 0;
        }
        flags_bits |= flags_bits_temp;
    }
    return flags_bits;
}
}  // namespace

extern "C" EtsVoid *EscompatRegExpCompile(ObjectHeader *obj)
{
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto string_class = class_linker->GetClassRoot(EtsClassRoot::STRING);
    auto regexp_object = EtsObject::FromCoreType(obj);
    VMHandle<EtsObject> reg_obj_handle(thread, regexp_object->GetCoreType());
    auto regexp_class = reg_obj_handle.GetPtr()->GetClass();

    EtsField *group_names_field = regexp_class->GetDeclaredFieldIDByName("groupNames");
    EtsString *pattern_str = EtsString::FromEtsObject(GetFieldObjectByName(reg_obj_handle.GetPtr(), "pattern"));
    VMHandle<coretypes::String> s_handle(thread, pattern_str->GetCoreType());

    EtsString *flags = EtsString::FromEtsObject(GetFieldObjectByName(reg_obj_handle.GetPtr(), "flags"));
    auto flags_bits = static_cast<uint8_t>(CastToBitMask(flags));

    RegExpParser parser = RegExpParser();
    parser.Init(reinterpret_cast<char *>(s_handle.GetPtr()->GetDataMUtf8()), s_handle.GetPtr()->GetLength(),
                flags_bits);
    parser.Parse();

    PandaVector<PandaString> group_name = parser.GetGroupNames();

    EtsObjectArray *ets_group_names = EtsObjectArray::Create(string_class, group_name.size());
    VMHandle<EtsObjectArray> arr_handle(thread, ets_group_names->GetCoreType());

    for (size_t i = 0; i < group_name.size(); ++i) {
        EtsString *str = EtsString::CreateFromMUtf8(group_name[i].c_str(), group_name[i].size());
        arr_handle.GetPtr()->Set(i, str->AsObject());
    }
    reg_obj_handle.GetPtr()->SetFieldObject(group_names_field, arr_handle.GetPtr()->AsObject());

    auto buffer_size = parser.GetOriginBufferSize();
    auto buffer = parser.GetOriginBuffer();
    EtsField *buffer_field = regexp_class->GetDeclaredFieldIDByName("buffer");
    EtsByteArray *ets_buffer = EtsByteArray::Create(buffer_size);
    for (size_t i = 0; i < buffer_size; ++i) {
        ets_buffer->Set(i, buffer[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    reg_obj_handle.GetPtr()->SetFieldObject(buffer_field, ets_buffer->AsObject());

    return EtsVoid::GetInstance();
}

extern "C" EtsObject *EscompatRegExpExec(ObjectHeader *obj, EtsString *str)
{
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<EtsString> str_handle(thread, str->GetCoreType());

    auto regexp_object = EtsObject::FromCoreType(obj);
    VMHandle<EtsObject> reg_obj_handle(thread, regexp_object->GetCoreType());

    auto regexp_class = reg_obj_handle.GetPtr()->GetClass();
    auto class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto string_class = class_linker->GetClassRoot(EtsClassRoot::STRING);

    EtsField *last_index_field = regexp_class->GetDeclaredFieldIDByName("lastIndex");
    auto last_index = reg_obj_handle.GetPtr()->GetFieldPrimitive<int32_t>(last_index_field);
    EtsString *flags = EtsString::FromEtsObject(GetFieldObjectByName(reg_obj_handle.GetPtr(), "flags"));
    auto flags_bits = static_cast<uint8_t>(CastToBitMask(flags));

    auto result_class = class_linker->GetClass("Lescompat/RegExpExecResult;");
    auto result_object = EtsObject::Create(result_class);
    VMHandle<EtsObject> obj_handle(thread, result_object->GetCoreType());

    auto result_correct_field = result_class->GetDeclaredFieldIDByName("isCorrect");

    bool global = (flags_bits & RegExpParser::FLAG_GLOBAL) > 0;
    bool sticky = (flags_bits & RegExpParser::FLAG_STICKY) > 0;
    if (!global && !sticky) {
        last_index = 0;
    }

    int32_t string_length = str_handle.GetPtr()->GetLength();
    if (last_index > string_length) {
        if (global || sticky) {
            reg_obj_handle.GetPtr()->SetFieldPrimitive(last_index_field, 0);
        }
        obj_handle.GetPtr()->SetFieldPrimitive(result_correct_field, false);
        return obj_handle.GetPtr();
    }

    RegExpExecutor executor = RegExpExecutor();
    PandaVector<uint8_t> u8_buffer;
    PandaVector<uint16_t> u16_buffer;
    const uint8_t *str_buffer;
    bool is_utf16 = str_handle.GetPtr()->IsUtf16();
    if (is_utf16) {
        u16_buffer = PandaVector<uint16_t>(string_length);
        str_handle.GetPtr()->CopyDataUtf16(u16_buffer.data(), string_length);
        str_buffer = reinterpret_cast<uint8_t *>(u16_buffer.data());
    } else {
        u8_buffer = PandaVector<uint8_t>(string_length + 1);
        str_handle.GetPtr()->CopyDataMUtf8(u8_buffer.data(), string_length + 1, true);
        str_buffer = u8_buffer.data();
    }

    auto ets_buffer = reinterpret_cast<EtsByteArray *>(GetFieldObjectByName(reg_obj_handle.GetPtr(), "buffer"));
    auto buffer = reinterpret_cast<uint8_t *>(ets_buffer->GetData<int8_t>());
    bool ret = executor.Execute(str_buffer, last_index, string_length, buffer, is_utf16);
    RegExpMatchResult exec_result = executor.GetResult(ret);
    if (!exec_result.is_success) {
        if (global || sticky) {
            reg_obj_handle.GetPtr()->SetFieldPrimitive(last_index_field, 0);
        }
        obj_handle.GetPtr()->SetFieldPrimitive<bool>(result_correct_field, false);
        return obj_handle.GetPtr();
    }

    uint32_t end_index = exec_result.end_index;
    if (global || sticky) {
        reg_obj_handle.GetPtr()->SetFieldPrimitive(last_index_field, end_index);
    }
    // isCorrect field
    obj_handle.GetPtr()->SetFieldPrimitive<bool>(result_correct_field, true);
    // index field
    auto index_field = result_class->GetDeclaredFieldIDByName("index");
    obj_handle.GetPtr()->SetFieldPrimitive<int32_t>(index_field, exec_result.index);
    // input field
    auto input_field = result_class->GetDeclaredFieldIDByName("input");
    obj_handle.GetPtr()->SetFieldObject(input_field, str_handle.GetPtr()->AsObject());
    // result field
    auto result_field = result_class->GetDeclaredFieldIDByName("result");
    uint32_t captures_size = exec_result.captures.size();
    PandaVector<VMHandle<EtsString>> matches;
    for (size_t i = 0; i < captures_size; ++i) {
        if (!exec_result.captures[i].first) {
            matches.push_back(exec_result.captures[i].second);
        }
    }
    EtsObjectArray *result_array = EtsObjectArray::Create(string_class, matches.size());
    for (size_t i = 0; i < matches.size(); i++) {
        result_array->Set(i, matches[i]->AsObject());
    }
    obj_handle.GetPtr()->SetFieldObject(result_field, result_array->AsObject());
    return obj_handle.GetPtr();
}
}  // namespace panda::ets::intrinsics
