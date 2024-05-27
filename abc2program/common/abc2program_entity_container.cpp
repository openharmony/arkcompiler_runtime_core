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

#include "abc2program_entity_container.h"
#include "os/file.h"
#include "abc_file_utils.h"
#include "method_data_accessor-inl.h"
#include "mangling.h"

namespace panda::abc2program {

const panda_file::File &Abc2ProgramEntityContainer::GetAbcFile() const
{
    return file_;
}

AbcStringTable &Abc2ProgramEntityContainer::GetAbcStringTable() const
{
    return string_table_;
}

pandasm::Program &Abc2ProgramEntityContainer::GetProgram() const
{
    return program_;
}

panda_file::DebugInfoExtractor &Abc2ProgramEntityContainer::GetDebugInfoExtractor() const
{
    return debug_info_extractor_;
}

bool Abc2ProgramEntityContainer::AddRecord(uint32_t class_id, const pandasm::Record &record)
{
    auto it = record_map_.find(class_id);
    if (it != record_map_.end()) {
        return false;
    }
    record_map_.emplace(class_id, &record);
    return true;
}

bool Abc2ProgramEntityContainer::AddRecord(const panda_file::File::EntityId &class_id, const pandasm::Record &record)
{
    return AddRecord(class_id.GetOffset(), record);
}

bool Abc2ProgramEntityContainer::AddFunction(uint32_t method_id, const pandasm::Function &function)
{
    auto it = function_map_.find(method_id);
    if (it != function_map_.end()) {
        return false;
    }
    function_map_.emplace(method_id, &function);
    return true;
}

bool Abc2ProgramEntityContainer::AddFunction(const panda_file::File::EntityId &method_id,
                                             const pandasm::Function &function)
{
    return AddFunction(method_id.GetOffset(), function);
}

bool Abc2ProgramEntityContainer::AddField(uint32_t field_id, const pandasm::Field &field)
{
    auto it = field_map_.find(field_id);
    if (it != field_map_.end()) {
        return false;
    }
    field_map_.emplace(field_id, &field);
    return true;
}

bool Abc2ProgramEntityContainer::AddField(const panda_file::File::EntityId &field_id, const pandasm::Field &field)
{
    return AddField(field_id.GetOffset(), field);
}

const pandasm::Record *Abc2ProgramEntityContainer::GetRecordById(const panda_file::File::EntityId &class_id) const
{
    auto it = record_map_.find(class_id.GetOffset());
    if (it != record_map_.end()) {
        return it->second;
    }
    return nullptr;
}

const pandasm::Function *Abc2ProgramEntityContainer::GetFunctionById(const panda_file::File::EntityId &method_id) const
{
    auto it = function_map_.find(method_id.GetOffset());
    if (it != function_map_.end()) {
        return it->second;
    }
    return nullptr;
}

const pandasm::Field *Abc2ProgramEntityContainer::GetFieldById(const panda_file::File::EntityId &field_id) const
{
    auto it = field_map_.find(field_id.GetOffset());
    if (it != field_map_.end()) {
        return it->second;
    }
    return nullptr;
}

std::string Abc2ProgramEntityContainer::GetFullRecordNameById(const panda_file::File::EntityId &class_id)
{
    uint32_t class_id_offset = class_id.GetOffset();
    auto it = record_full_name_map_.find(class_id_offset);
    if (it != record_full_name_map_.end()) {
        return it->second;
    }
    std::string name = string_table_.GetStringById(class_id);
    pandasm::Type type = pandasm::Type::FromDescriptor(name);
    std::string record_full_name = type.GetName();
    record_full_name_map_.emplace(class_id_offset, record_full_name);
    return record_full_name;
}

std::string Abc2ProgramEntityContainer::GetFullMethodNameById(uint32_t method_id)
{
    auto it = method_full_name_map_.find(method_id);
    if (it != method_full_name_map_.end()) {
        return it->second;
    }
    std::string full_method_name = ConcatFullMethodNameById(panda_file::File::EntityId(method_id));
    method_full_name_map_.emplace(method_id, full_method_name);
    return full_method_name;
}

std::string Abc2ProgramEntityContainer::GetFullMethodNameById(const panda_file::File::EntityId &method_id)
{
    return GetFullMethodNameById(method_id.GetOffset());
}

std::string Abc2ProgramEntityContainer::ConcatFullMethodNameById(const panda_file::File::EntityId &method_id)
{
    panda::panda_file::MethodDataAccessor method_data_accessor(file_, method_id);
    std::string method_name_raw = string_table_.GetStringById(method_data_accessor.GetNameId());
    std::string record_name = GetFullRecordNameById(method_data_accessor.GetClassId());
    std::stringstream ss;
    if (AbcFileUtils::IsSystemTypeName(record_name)) {
        ss << DOT;
    } else {
        ss << record_name << DOT;
    }
    ss << method_name_raw;
    return ss.str();
}

// ark_disasm can use this method to get all literal array ids except module literal ids
std::unordered_set<uint32_t> Abc2ProgramEntityContainer::GetLiteralArrayIdSet()
{
    std::unordered_set<uint32_t> literal_array_id_set;
    set_union(module_literal_array_id_set_.begin(), module_literal_array_id_set_.end(),
              processed_nested_literal_array_id_set_.begin(), processed_nested_literal_array_id_set_.end(),
              inserter(literal_array_id_set, literal_array_id_set.begin()));
    return literal_array_id_set;
}

const std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetMouleLiteralArrayIdSet() const
{
    return module_literal_array_id_set_;
}

const std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetUnnestedLiteralArrayIdSet() const
{
    return unnested_literal_array_id_set_;
}

std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetUnprocessedNestedLiteralArrayIdSet()
{
    return unprocessed_nested_literal_array_id_set_;
}

void Abc2ProgramEntityContainer::AddModuleLiteralArrayId(uint32_t module_literal_array_id)
{
    module_literal_array_id_set_.insert(module_literal_array_id);
}

void Abc2ProgramEntityContainer::AddUnnestedLiteralArrayId(const panda_file::File::EntityId &literal_array_id)
{
    unnested_literal_array_id_set_.insert(literal_array_id.GetOffset());
}

void Abc2ProgramEntityContainer::AddProcessedNestedLiteralArrayId(uint32_t nested_literal_array_id)
{
    processed_nested_literal_array_id_set_.insert(nested_literal_array_id);
}

void Abc2ProgramEntityContainer::TryAddUnprocessedNestedLiteralArrayId(uint32_t nested_literal_array_id)
{
    if (unnested_literal_array_id_set_.count(nested_literal_array_id)) {
        return;
    }
    if (processed_nested_literal_array_id_set_.count(nested_literal_array_id)) {
        return;
    }
    unprocessed_nested_literal_array_id_set_.insert(nested_literal_array_id);
}

std::string Abc2ProgramEntityContainer::GetLiteralArrayIdName(uint32_t literal_array_id)
{
    std::stringstream name;
    auto cur_record_name = GetFullRecordNameById(panda_file::File::EntityId(current_class_id_));
    name << cur_record_name << UNDERLINE << literal_array_id;
    return name.str();
}

std::string Abc2ProgramEntityContainer::GetLiteralArrayIdName(const panda_file::File::EntityId &literal_array_id)
{
    return GetLiteralArrayIdName(literal_array_id.GetOffset());
}

void Abc2ProgramEntityContainer::AddProgramString(const std::string &str) const
{
    program_.strings.insert(str);
}

std::string Abc2ProgramEntityContainer::GetAbcFileAbsolutePath() const
{
    return os::file::File::GetAbsolutePath(file_.GetFilename()).Value();
}

void Abc2ProgramEntityContainer::SetCurrentClassId(uint32_t class_id)
{
    current_class_id_ = class_id;
}

void Abc2ProgramEntityContainer::ClearLiteralArrayIdSet()
{
    module_literal_array_id_set_.clear();
    unnested_literal_array_id_set_.clear();
    processed_nested_literal_array_id_set_.clear();
    unprocessed_nested_literal_array_id_set_.clear();
}

}  // namespace panda::abc2program