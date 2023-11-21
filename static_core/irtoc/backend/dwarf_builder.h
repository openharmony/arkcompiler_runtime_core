/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_DWARF_BUILDER_H
#define PANDA_DWARF_BUILDER_H

#include "utils/arch.h"
#include "utils/span.h"

#ifdef PANDA_COMPILER_DEBUG_INFO
#include <libdwarf/libdwarf.h>
#endif

#include "elfio/elfio.hpp"

#include <string>
#include <unordered_map>

namespace panda::irtoc {
class Function;

class DwarfBuilder {
public:
    struct Section {
        std::string name;
        unsigned type;
        unsigned flags;
        unsigned link;
        unsigned info;
        std::vector<uint8_t> data;
    };

    enum class SectionKind { DEBUG_LINE = 1, REL_DEBUG_LINE };

    DwarfBuilder(Arch arch, ELFIO::elfio *elf);

    bool BuildGraph(const Function *func, uint32_t code_offset, unsigned symbol);

    bool Finalize(uint32_t code_size);

    Dwarf_Unsigned AddFile(const std::string &fname, Dwarf_Unsigned dir_index);

    Dwarf_Unsigned AddDir(const std::string &dname);

    Arch GetArch()
    {
        return arch_;
    }

    ELFIO::elfio *GetElfBuilder()
    {
        return elf_builder_;
    }

    static int CreateSectionCallback([[maybe_unused]] char *name, [[maybe_unused]] int size,
                                     [[maybe_unused]] Dwarf_Unsigned type, [[maybe_unused]] Dwarf_Unsigned flags,
                                     [[maybe_unused]] Dwarf_Unsigned link, [[maybe_unused]] Dwarf_Unsigned info,
                                     [[maybe_unused]] Dwarf_Unsigned *sect_name_index, [[maybe_unused]] void *user_data,
                                     [[maybe_unused]] int *error);

private:
    std::vector<ELFIO::section *> sections_;
    std::unordered_map<std::string, Dwarf_Unsigned> source_files_map_;
    std::unordered_map<std::string, Dwarf_Unsigned> dirs_map_;
    std::unordered_map<unsigned, unsigned> index_map_;
    std::unordered_map<unsigned, ELFIO::relocation_section_accessor> rel_map_;

    ELFIO::elfio *elf_builder_ {nullptr};
    Dwarf_P_Debug dwarf_ {nullptr};
    Dwarf_P_Die compile_unit_die_ {nullptr};
    Dwarf_P_Die prev_program_die_ {nullptr};
    uint32_t code_size_ {0};
    Arch arch_;
};
}  // namespace panda::irtoc

#endif  // PANDA_DWARF_BUILDER_H
