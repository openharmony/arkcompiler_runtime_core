/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "abc2program_key_data.h"
#include "method_data_accessor.h"

namespace ark::abc2program {

const panda_file::File &Abc2ProgramKeyData::GetAbcFile() const
{
    return file_;
}

ExternalFieldTable &Abc2ProgramKeyData::GetExternalFieldTable()
{
    return externalFieldTable_;
}

NameToIdTable &Abc2ProgramKeyData::GetMethodNameToIdTable()
{
    return methodNameToId_;
}

NameToIdTable &Abc2ProgramKeyData::GetRecordNameToIdTable()
{
    return recordNameToId_;
}

AbcStringTable &Abc2ProgramKeyData::GetAbcStringTable() const
{
    return stringTable_;
}

pandasm::Program &Abc2ProgramKeyData::GetProgram() const
{
    return program_;
}

std::string Abc2ProgramKeyData::GetFullRecordNameById(const panda_file::File::EntityId &classId) const
{
    std::string name = stringTable_.GetStringById(classId);
    pandasm::Type type = pandasm::Type::FromDescriptor(name);
    return type.GetPandasmName();
}

std::string Abc2ProgramKeyData::GetFullFunctionNameById(const panda_file::File::EntityId &methodId) const
{
    ark::panda_file::MethodDataAccessor methodAccessor(file_, methodId);

    const auto methodNameRaw = stringTable_.GetStringById(methodAccessor.GetNameId());

    std::string className = GetFullRecordNameById(methodAccessor.GetClassId());
    if (IsSystemType(className)) {
        className = "";
    } else {
        className += ".";
    }

    return className + methodNameRaw;
}

inline bool Abc2ProgramKeyData::IsSystemType(const std::string &typeName) const
{
    bool isArrayType = typeName.back() == ']';
    bool isGlobal = typeName == "_GLOBAL";

    return isArrayType || isGlobal;
}

ark::panda_file::SourceLang Abc2ProgramKeyData::GetFileLanguage() const
{
    return fileLanguage_;
}

void Abc2ProgramKeyData::SetFileLanguage(ark::panda_file::SourceLang language)
{
    fileLanguage_ = language;
}

}  // namespace ark::abc2program