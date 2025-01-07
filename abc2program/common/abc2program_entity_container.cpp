/*
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
#include "abc_file_utils.h"
#include "file-inl.h"
#include "method_data_accessor-inl.h"

namespace panda::abc2program {

const panda_file::File &Abc2ProgramEntityContainer::GetAbcFile() const
{
    return file_;
}

pandasm::Program &Abc2ProgramEntityContainer::GetProgram() const
{
    return program_;
}

const panda_file::DebugInfoExtractor &Abc2ProgramEntityContainer::GetDebugInfoExtractor() const
{
    return debug_info_extractor_;
}

std::string Abc2ProgramEntityContainer::GetStringById(const panda_file::File::EntityId &entity_id) const
{
    panda_file::File::StringData sd = file_.GetStringData(entity_id);
    return (reinterpret_cast<const char *>(sd.data));
}

std::string Abc2ProgramEntityContainer::GetFullRecordNameById(const panda_file::File::EntityId &class_id)
{
    uint32_t class_id_offset = class_id.GetOffset();
    auto it = record_full_name_map_.find(class_id_offset);
    if (it != record_full_name_map_.end()) {
        return it->second;
    }
    std::string name = GetStringById(class_id);
    pandasm::Type type = pandasm::Type::FromDescriptor(name);
    std::string record_full_name = type.GetName();
    ModifyRecordName(record_full_name);
    record_full_name_map_.emplace(class_id_offset, record_full_name);
    return record_full_name;
}

/**
 * Support the inter-app hsp dependents bytecode har.
 * The record name format: <bundleName>&<normalizedImportPath>&<version>
 * The ohmurl specs that must have bundleName in inter-app package. The records need compile into the inter-app
 * hsp package. So the recordNames need to add bundleName in front when the abc-file as input for the
 * inter-app hsp package.
 */
void Abc2ProgramEntityContainer::ModifyRecordName(std::string &record_name)
{
    if (bundle_name_.empty()) {
        return;
    }
    if (IsSourceFileRecord(record_name)) {
        record_name = bundle_name_ + record_name;
    }
}

bool Abc2ProgramEntityContainer::IsSourceFileRecord(const std::string& record_name)
{
    return record_name.find(NORMALIZED_OHMURL_SEPARATOR) == 0;
}

std::string Abc2ProgramEntityContainer::GetFullMethodNameById(const panda_file::File::EntityId &method_id)
{
    auto method_id_offset = method_id.GetOffset();
    auto it = method_full_name_map_.find(method_id_offset);
    if (it != method_full_name_map_.end()) {
        return it->second;
    }
    std::string full_method_name = ConcatFullMethodNameById(method_id);
    method_full_name_map_.emplace(method_id_offset, full_method_name);
    return full_method_name;
}

std::string Abc2ProgramEntityContainer::ConcatFullMethodNameById(const panda_file::File::EntityId &method_id)
{
    panda::panda_file::MethodDataAccessor method_data_accessor(file_, method_id);
    std::string method_name_raw = GetStringById(method_data_accessor.GetNameId());
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

const std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetMouleLiteralArrayIdSet() const
{
    return module_literal_array_id_set_;
}

const std::unordered_set<uint32_t> &Abc2ProgramEntityContainer::GetModuleRequestPhaseIdSet() const
{
    return module_request_phase_id_set_;
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


void Abc2ProgramEntityContainer::AddModuleRequestPhaseId(uint32_t module_request_phase_id)
{
    module_request_phase_id_set_.insert(module_request_phase_id);
}

void Abc2ProgramEntityContainer::AddUnnestedLiteralArrayId(uint32_t literal_array_id)
{
    unnested_literal_array_id_set_.insert(literal_array_id);
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

void Abc2ProgramEntityContainer::AddProgramString(const std::string &str) const
{
    program_.strings.insert(str);
}

}  // namespace panda::abc2program