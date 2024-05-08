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

#include "abc_file_processor.h"
#include <iostream>
#include "abc_class_processor.h"
#include "abc2program_log.h"
#include "common/abc_file_utils.h"
#include "abc_literal_array_processor.h"

namespace panda::abc2program {

AbcFileProcessor::AbcFileProcessor(Abc2ProgramEntityContainer &entity_container) : entity_container_(entity_container)
{
    file_ = &(entity_container_.GetAbcFile());
    string_table_ = &(entity_container_.GetAbcStringTable());
    program_ = &(entity_container_.GetProgram());
}

bool AbcFileProcessor::FillProgramData()
{
    program_->lang = panda_file::SourceLang::ECMASCRIPT;
    FillClassesData();
    return true;
}

void AbcFileProcessor::FillClassesData()
{
    const auto classes = file_->GetClasses();
    for (size_t i = 0; i < classes.size(); i++) {
        uint32_t class_idx = classes[i];
        auto class_off = file_->GetHeader()->class_idx_off + sizeof(uint32_t) * i;
        if (class_idx > file_->GetHeader()->file_size) {
            LOG(FATAL, ABC2PROGRAM)  << "> error encountered in record at " << class_off << " (0x" << std::hex
                                     << class_off << "). binary file corrupted. record offset (0x" << class_idx
                                     << ") out of bounds (0x" << file_->GetHeader()->file_size << ")!";
            break;
        }
        panda_file::File::EntityId record_id(class_idx);
        if (file_->IsExternal(record_id)) {
            continue;
        }
        entity_container_.SetCurrentClassId(class_idx);
        entity_container_.ClearLiteralArrayIdSet();
        AbcClassProcessor class_processor(record_id, entity_container_);
        class_processor.FillProgramData();
        FillLiteralArrayTable();
    }
}

void AbcFileProcessor::FillLiteralArrayTable()
{
    literal_data_accessor_ = std::make_unique<panda_file::LiteralDataAccessor>(*file_, file_->GetLiteralArraysId());
    FillModuleLiteralArrays();
    FillUnnestedLiteralArrays();
    FillNestedLiteralArrays();
}

void AbcFileProcessor::FillModuleLiteralArrays()
{
    for (uint32_t module_literal_array_id : entity_container_.GetMouleLiteralArrayIdSet()) {
        FillModuleLiteralArrayById(module_literal_array_id);
    }
}

void AbcFileProcessor::FillUnnestedLiteralArrays()
{
    for (uint32_t unnested_literal_array_id : entity_container_.GetUnnestedLiteralArrayIdSet()) {
        FillLiteralArrayById(unnested_literal_array_id);
    }
}

void AbcFileProcessor::FillNestedLiteralArrays()
{
    auto &unprocessed_literal_array_id_set = entity_container_.GetUnprocessedNestedLiteralArrayIdSet();
    while (!unprocessed_literal_array_id_set.empty()) {
        auto id = unprocessed_literal_array_id_set.begin();
        entity_container_.AddProcessedNestedLiteralArrayId(*id);
        FillLiteralArrayById(*id);
        unprocessed_literal_array_id_set.erase(id);
    }
}

void AbcFileProcessor::FillModuleLiteralArrayById(uint32_t module_literal_array_id)
{
    panda_file::File::EntityId entity_id(module_literal_array_id);
    AbcModuleArrayProcessor module_array_processor(entity_id, entity_container_);
    module_array_processor.FillProgramData();
}

void AbcFileProcessor::FillLiteralArrayById(uint32_t literal_array_id)
{
    panda_file::File::EntityId entity_id(literal_array_id);
    AbcLiteralArrayProcessor literal_array_processor(entity_id, entity_container_, *literal_data_accessor_);
    literal_array_processor.FillProgramData();
}

} // namespace panda::abc2program
