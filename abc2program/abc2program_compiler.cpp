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

#include "abc2program_compiler.h"
#include "abc_class_processor.h"
#include "file_format_version.h"

namespace panda::abc2program {

Abc2ProgramCompiler::~Abc2ProgramCompiler()
{
    if (prog_ != nullptr) {
        delete prog_;
        prog_ = nullptr;
    }
}

bool Abc2ProgramCompiler::OpenAbcFile(const std::string &file_path)
{
    file_ = panda_file::File::Open(file_path);
    if (file_ == nullptr) {
        return false;
    }
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

bool Abc2ProgramCompiler::CheckFileVersionIsSupported(std::array<uint8_t, panda_file::File::VERSION_SIZE> min_version,
                                                      uint8_t target_api_version,
                                                      std::string target_api_sub_version) const
{
    auto target_version = panda_file::GetVersionByApi(target_api_version, target_api_sub_version);
    if (!target_version.has_value()) {
        return false;
    }
    const auto &file_version = file_->GetHeader()->version;
    return IsVersionLessEqual(min_version, file_version) &&
        IsVersionLessEqual(file_version, target_version.value());
}

const panda_file::File &Abc2ProgramCompiler::GetAbcFile() const
{
    return *file_;
}

const panda_file::DebugInfoExtractor &Abc2ProgramCompiler::GetDebugInfoExtractor() const
{
    return *debug_info_extractor_;
}

pandasm::Program *Abc2ProgramCompiler::CompileAbcFile()
{
    prog_ = new pandasm::Program();
    prog_->lang = LANG_ECMA;
    auto classes = file_->GetClasses();
    for (size_t i = 0; i < classes.size(); i++) {
        panda_file::File::EntityId record_id(classes[i]);
        if (file_->IsExternal(record_id)) {
            UNREACHABLE();
        }
        CompileAbcClass(record_id, *prog_);
    }
    return prog_;
}

void Abc2ProgramCompiler::CompileAbcClass(const panda_file::File::EntityId &record_id,
                                          pandasm::Program &program)
{
    ASSERT(!file_->IsExternal(record_id));
    Abc2ProgramEntityContainer entity_container(*file_, program, *debug_info_extractor_, record_id.GetOffset());
    AbcClassProcessor class_processor(record_id, entity_container);
    class_processor.FillProgramData();
}

bool Abc2ProgramCompiler::CheckClassId(uint32_t class_id, size_t offset) const
{
    auto *header = file_->GetHeader();
    auto class_off = header->class_idx_off + sizeof(uint32_t) * offset;
    if (class_id > header->file_size) {
        LOG(FATAL, ABC2PROGRAM) << "> error encountered in record at " << class_off << " (0x" << std::hex
                                << class_off << "). binary file corrupted. record offset (0x" << class_id
                                << ") out of bounds (0x" << header->file_size << ")!";
    }
    return !file_->IsExternal(panda_file::File::EntityId(class_id));
}

} // namespace panda::abc2program
