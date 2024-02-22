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

namespace panda::abc2program {

bool AbcFileProcessor::ProcessFile()
{
    program_.lang = panda_file::SourceLang::ECMASCRIPT;
    ProcessClasses();
    FillUpProgramStrings();
    FillUpProgramArrayTypes();
    return true;
}

void AbcFileProcessor::ProcessClasses()
{
    const auto classes = abc_file_.GetClasses();
    for (const uint32_t class_idx : classes) {
        panda_file::File::EntityId class_id(class_idx);
        if (abc_file_.IsExternal(class_id)) {
            continue;
        }
        AbcClassProcessor classProcessor(class_id, abc_file_, abc_string_table_);
    }
}

void AbcFileProcessor::FillUpProgramStrings()
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

void AbcFileProcessor::FillUpProgramArrayTypes()
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

} // namespace panda::abc2program
