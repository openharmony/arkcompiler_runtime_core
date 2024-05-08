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

#ifndef ABC2PROGRAM_ABC2PROGRAM_ENTITY_CONTAINER_H
#define ABC2PROGRAM_ABC2PROGRAM_ENTITY_CONTAINER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <assembly-program.h>
#include <debug_info_extractor.h>
#include "file.h"
#include "abc_string_table.h"

namespace panda::abc2program {

const panda::panda_file::SourceLang lang = panda::panda_file::SourceLang::ECMASCRIPT;

class Abc2ProgramEntityContainer {
public:
    Abc2ProgramEntityContainer(const panda_file::File &file,
                               AbcStringTable &string_table,
                               pandasm::Program &program,
                               panda_file::DebugInfoExtractor &debug_info_extractor)
        : file_(file), string_table_(string_table), program_(program), debug_info_extractor_(debug_info_extractor) {}
    const panda_file::File &GetAbcFile() const;
    AbcStringTable &GetAbcStringTable() const;
    pandasm::Program &GetProgram() const;
    bool AddRecord(uint32_t class_id, const pandasm::Record &record);
    bool AddRecord(const panda_file::File::EntityId &class_id, const pandasm::Record &record);
    bool AddFunction(uint32_t method_id, const pandasm::Function &function);
    bool AddFunction(const panda_file::File::EntityId &method_id, const pandasm::Function &function);
    bool AddField(uint32_t field_id, const pandasm::Field &field);
    bool AddField(const panda_file::File::EntityId &field_id, const pandasm::Field &field);
    const pandasm::Record *GetRecordById(const panda_file::File::EntityId &class_id) const;
    const pandasm::Function *GetFunctionById(const panda_file::File::EntityId &method_id) const;
    const pandasm::Field *GetFieldById(const panda_file::File::EntityId &field_id) const;

    std::string GetFullRecordNameById(const panda_file::File::EntityId &class_id);
    std::string GetFullMethodNameById(uint32_t method_id);
    std::string GetFullMethodNameById(const panda_file::File::EntityId &method_id);
    void AddProgramString(const std::string &str) const;

    std::unordered_set<uint32_t> GetLiteralArrayIdSet();
    const std::unordered_set<uint32_t> &GetMouleLiteralArrayIdSet() const;
    const std::unordered_set<uint32_t> &GetUnnestedLiteralArrayIdSet() const;
    std::unordered_set<uint32_t> &GetUnprocessedNestedLiteralArrayIdSet();
    void AddModuleLiteralArrayId(uint32_t module_literal_array_id);
    void AddUnnestedLiteralArrayId(const panda_file::File::EntityId &literal_array_id);
    void AddProcessedNestedLiteralArrayId(uint32_t nested_literal_array_id);
    void TryAddUnprocessedNestedLiteralArrayId(uint32_t nested_literal_array_id);
    std::string GetLiteralArrayIdName(uint32_t literal_array_id);
    std::string GetLiteralArrayIdName(const panda_file::File::EntityId &literal_array_id);
    std::string GetAbcFileAbsolutePath() const;
    panda_file::DebugInfoExtractor &GetDebugInfoExtractor() const;

    void SetCurrentClassId(uint32_t class_id);
    void ClearLiteralArrayIdSet();

private:
    std::string ConcatFullMethodNameById(const panda_file::File::EntityId &method_id);
    const panda_file::File &file_;
    AbcStringTable &string_table_;
    pandasm::Program &program_;
    panda_file::DebugInfoExtractor &debug_info_extractor_;
    std::unordered_map<uint32_t, const pandasm::Record*> record_map_;
    std::unordered_map<uint32_t, const pandasm::Function*> function_map_;
    std::unordered_map<uint32_t, const pandasm::Field*> field_map_;
    std::unordered_map<uint32_t, std::string> record_full_name_map_;
    std::unordered_map<uint32_t, std::string> method_full_name_map_;
    std::unordered_set<uint32_t> module_literal_array_id_set_;
    std::unordered_set<uint32_t> unnested_literal_array_id_set_;
    std::unordered_set<uint32_t> processed_nested_literal_array_id_set_;
    std::unordered_set<uint32_t> unprocessed_nested_literal_array_id_set_;
    uint32_t current_class_id_{0};
}; // class Abc2ProgramEntityContainer

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC2PROGRAM_ENTITY_CONTAINER_H