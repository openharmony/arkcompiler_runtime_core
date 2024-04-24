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

#ifndef ABC2PROGRAM_ABC2PROGRAM_COMPILER_H
#define ABC2PROGRAM_ABC2PROGRAM_COMPILER_H

#include <string>
#include <assembly-program.h>
#include <debug_info_extractor.h>
#include "common/abc2program_entity_container.h"

namespace panda::abc2program {

class Abc2ProgramCompiler {
public:
    bool OpenAbcFile(const std::string &file_path);
    bool FillProgramData(pandasm::Program &program);
    bool CheckFileVersionIsSupported(uint8_t min_api_version, uint8_t target_api_version) const;
    const panda_file::File &GetAbcFile() const;
    AbcStringTable &GetAbcStringTable() const;
private:
    bool IsVersionLessEqual(const std::array<uint8_t, panda_file::File::VERSION_SIZE> &version_1,
        const std::array<uint8_t, panda_file::File::VERSION_SIZE> &version_2) const;
    std::unique_ptr<const panda_file::File> file_;
    std::unique_ptr<AbcStringTable> string_table_;
    std::unique_ptr<Abc2ProgramEntityContainer> entity_container_;
    std::unique_ptr<panda_file::DebugInfoExtractor> debug_info_extractor_;
}; // class Abc2ProgramCompiler

} // namespace panda::abc2program

#endif // ABC2PROGRAM_ABC2PROGRAM_COMPILER_H
