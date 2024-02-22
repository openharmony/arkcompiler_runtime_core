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

#include "abc_file_processor.h"
#include <iostream>
#include "abc_class_processor.h"
#include "abc2program_log.h"
#include "abc_file_utils.h"

namespace panda::abc2program {

AbcFileProcessor::AbcFileProcessor(Abc2ProgramKeyData &key_data) : key_data_(key_data)
{
    file_ = &(key_data_.GetAbcFile());
    string_table_ = &(key_data_.GetAbcStringTable());
    program_ = &(key_data_.GetProgram());
}

bool AbcFileProcessor::ProcessFile()
{
    program_->lang = panda_file::SourceLang::ECMASCRIPT;
    ProcessClasses();
    FillProgramStrings();
    return true;
}

void AbcFileProcessor::ProcessClasses()
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
        AbcClassProcessor classProcessor(record_id, key_data_);
    }
}

void AbcFileProcessor::FillProgramStrings()
{
    program_->strings = string_table_->GetStringSet();
}

} // namespace panda::abc2program
