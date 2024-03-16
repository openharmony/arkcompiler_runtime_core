/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

namespace panda::abc2program {

const panda_file::File &Abc2ProgramKeyData::GetAbcFile() const
{
    return file_;
}

AbcStringTable &Abc2ProgramKeyData::GetAbcStringTable() const
{
    return string_table_;
}

pandasm::Program &Abc2ProgramKeyData::GetProgram() const
{
    return program_;
}

bool Abc2ProgramKeyData::AddRecord(panda_file::File::EntityId class_id, const pandasm::Record &record)
{
    auto it = record_map_.find(class_id);
    if (it != record_map_.end()) {
        return false;
    }
    record_map_[class_id] = &record;
    return true;
}

bool Abc2ProgramKeyData::AddFunction(panda_file::File::EntityId method_id, const pandasm::Function &function)
{
    auto it = function_map_.find(method_id);
    if (it != function_map_.end()) {
        return false;
    }
    function_map_[method_id] = &function;
    return true;
}

bool Abc2ProgramKeyData::AddField(panda_file::File::EntityId field_id, const pandasm::Field &field)
{
    auto it = field_map_.find(field_id);
    if (it != field_map_.end()) {
        return false;
    }
    field_map_[field_id] = &field;
    return true;
}

const pandasm::Record *Abc2ProgramKeyData::GetRecordById(panda_file::File::EntityId class_id) const
{
    auto it = record_map_.find(class_id);
    if (it != record_map_.end()) {
        return it->second;
    }
    return nullptr;
}

const pandasm::Function *Abc2ProgramKeyData::GetFunctionById(panda_file::File::EntityId method_id) const
{
    auto it = function_map_.find(method_id);
    if (it != function_map_.end()) {
        return it->second;
    }
    return nullptr;
}

const pandasm::Field *Abc2ProgramKeyData::GetFieldById(panda_file::File::EntityId field_id) const
{
    auto it = field_map_.find(field_id);
    if (it != field_map_.end()) {
        return it->second;
    }
    return nullptr;
}

std::string Abc2ProgramKeyData::GetFullRecordNameById(const panda_file::File::EntityId &class_id) const
{
    std::string name = string_table_.GetStringById(class_id);
    pandasm::Type type = pandasm::Type::FromDescriptor(name);
    return type.GetPandasmName();
}

}  // namespace panda::abc2program