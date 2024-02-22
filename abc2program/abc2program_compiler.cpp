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

#include "abc2program_compiler.h"
#include <iostream>
#include "abc_file_processor.h"

namespace panda::abc2program {

bool Abc2ProgramCompiler::OpenAbcFile(const std::string &file_path)
{
    file_ = panda_file::File::Open(file_path);
    if (file_ == nullptr) {
        std::cerr << "Unable to open specified abc file " << file_path << std::endl;
        return false;
    }
    string_table_ = std::make_unique<AbcStringTable>(*file_);
    return true;
}

const panda_file::File &Abc2ProgramCompiler::GetAbcFile() const
{
    return *file_;
}

AbcStringTable &Abc2ProgramCompiler::GetAbcStringTable() const
{
    return *string_table_;
}

bool Abc2ProgramCompiler::FillProgramData(pandasm::Program &program)
{
    key_data_ = std::make_unique<Abc2ProgramKeyData>(*file_, *string_table_, program);
    AbcFileProcessor file_processor(*key_data_);
    bool success = file_processor.ProcessFile();
    return success;
}

} // namespace panda::abc2program
