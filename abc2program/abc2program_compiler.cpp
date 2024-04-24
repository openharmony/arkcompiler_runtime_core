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

#include "abc2program_compiler.h"
#include "abc_file_processor.h"
#include "file_format_version.h"

namespace panda::abc2program {

bool Abc2ProgramCompiler::OpenAbcFile(const std::string &file_path)
{
    file_ = panda_file::File::Open(file_path);
    if (file_ == nullptr) {
        return false;
    }
    string_table_ = std::make_unique<AbcStringTable>(*file_);
    debug_info_extractor_ = std::make_unique<panda_file::DebugInfoExtractor>(file_.get());
    return true;
}

bool Abc2ProgramCompiler::IsVersionLessEqual(
    const std::array<uint8_t, panda_file::File::VERSION_SIZE> &version_1,
    const std::array<uint8_t, panda_file::File::VERSION_SIZE> &version_2) const
{
    for (size_t i = 0; i < panda_file::File::VERSION_SIZE; ++i) {
        if (version_1[i] != version_2[i]) {
            return version_1[i] < version_2[i];
        }
    }
    return true;
}

bool Abc2ProgramCompiler::CheckFileVersionIsSupported(uint8_t min_api_version, uint8_t target_api_version) const
{
    auto min_version = panda_file::GetVersionByApi(min_api_version);
    auto target_version = panda_file::GetVersionByApi(target_api_version);
    if (!min_version.has_value() || !target_version.has_value()) {
        return false;
    }
    const auto &file_version = file_->GetHeader()->version;
    return IsVersionLessEqual(min_version.value(), file_version) &&
        IsVersionLessEqual(file_version, target_version.value());
}

bool Abc2ProgramCompiler::FillProgramData(pandasm::Program &program)
{
    entity_container_ = std::make_unique<Abc2ProgramEntityContainer>(*file_,
                                                                     *string_table_,
                                                                     program,
                                                                     *debug_info_extractor_);
    AbcFileProcessor file_processor(*entity_container_);
    bool success = file_processor.FillProgramData();
    return success;
}

const panda_file::File &Abc2ProgramCompiler::GetAbcFile() const
{
    return *file_;
}

AbcStringTable &Abc2ProgramCompiler::GetAbcStringTable() const
{
    return *string_table_;
}

} // namespace panda::abc2program
