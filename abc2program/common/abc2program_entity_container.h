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

#ifndef ABC2PROGRAM_ABC2PROGRAM_ENTITY_CONTAINER_H
#define ABC2PROGRAM_ABC2PROGRAM_ENTITY_CONTAINER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <assembly-program.h>
#include <debug_info_extractor.h>
#include "file.h"

namespace panda::abc2program {

const panda::panda_file::SourceLang DEFUALT_SOURCE_LANG = panda::panda_file::DEFUALT_SOURCE_LANG;

class Abc2ProgramEntityContainer {
public:
    Abc2ProgramEntityContainer(const panda_file::File &file,
                               pandasm::Program &program,
                               const panda_file::DebugInfoExtractor &debug_info_extractor,
                               uint32_t class_id, std::string bundle_name)
        : file_(file), program_(program), debug_info_extractor_(debug_info_extractor),
          current_class_id_(class_id), bundle_name_(bundle_name) {}
    const panda_file::File &GetAbcFile() const;
    pandasm::Program &GetProgram() const;

    std::string GetStringById(const panda_file::File::EntityId &entity_id) const;
    std::string GetFullRecordNameById(const panda_file::File::EntityId &class_id);
    std::string GetFullMethodNameById(const panda_file::File::EntityId &method_id);
    void AddProgramString(const std::string &str) const;

    const std::unordered_set<uint32_t> &GetMouleLiteralArrayIdSet() const;
    const std::unordered_set<uint32_t> &GetModuleRequestPhaseIdSet() const;
    const std::unordered_set<uint32_t> &GetUnnestedLiteralArrayIdSet() const;
    std::unordered_set<uint32_t> &GetUnprocessedNestedLiteralArrayIdSet();
    void AddModuleLiteralArrayId(uint32_t module_literal_array_id);
    void AddUnnestedLiteralArrayId(uint32_t literal_array_id);
    void AddModuleRequestPhaseId(uint32_t module_request_phase_id);
    void AddProcessedNestedLiteralArrayId(uint32_t nested_literal_array_id);
    void TryAddUnprocessedNestedLiteralArrayId(uint32_t nested_literal_array_id);
    std::string GetLiteralArrayIdName(uint32_t literal_array_id);
    const panda_file::DebugInfoExtractor &GetDebugInfoExtractor() const;
    void ModifyRecordName(std::string &record_name);
    bool IsSourceFileRecord(const std::string& record_name);

private:
    std::string ConcatFullMethodNameById(const panda_file::File::EntityId &method_id);
    const panda_file::File &file_;
    pandasm::Program &program_;
    const panda_file::DebugInfoExtractor &debug_info_extractor_;
    std::unordered_map<uint32_t, std::string> record_full_name_map_;
    std::unordered_map<uint32_t, std::string> method_full_name_map_;
    std::unordered_set<uint32_t> module_literal_array_id_set_;
    std::unordered_set<uint32_t> module_request_phase_id_set_;
    std::unordered_set<uint32_t> unnested_literal_array_id_set_;
    std::unordered_set<uint32_t> processed_nested_literal_array_id_set_;
    std::unordered_set<uint32_t> unprocessed_nested_literal_array_id_set_;
    uint32_t current_class_id_{0};
    // It should modify record name when the bundle_name_ is not empty
    std::string bundle_name_ {};
};  // class Abc2ProgramEntityContainer

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_ABC2PROGRAM_ENTITY_CONTAINER_H