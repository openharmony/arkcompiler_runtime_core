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

#include "dwarf_builder.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "function.h"

#ifdef PANDA_COMPILER_DEBUG_INFO
#include <libdwarf/dwarf.h>
#endif

#include <numeric>

namespace panda::irtoc {
template <typename T>
static constexpr bool IsDwarfBadAddr(T v)
{
    return reinterpret_cast<Dwarf_Addr>(v) == DW_DLV_BADADDR;
}

int DwarfBuilder::CreateSectionCallback([[maybe_unused]] char *name, [[maybe_unused]] int size,
                                        [[maybe_unused]] Dwarf_Unsigned type, [[maybe_unused]] Dwarf_Unsigned flags,
                                        [[maybe_unused]] Dwarf_Unsigned link, [[maybe_unused]] Dwarf_Unsigned info,
                                        [[maybe_unused]] Dwarf_Unsigned *sect_name_index,
                                        [[maybe_unused]] void *user_data, [[maybe_unused]] int *error)
{
    auto self = reinterpret_cast<DwarfBuilder *>(user_data);
    ELFIO::section *section = self->GetElfBuilder()->sections.add(name);
    if (strncmp(name, ".rel", 4U) == 0) {
        self->rel_map_.insert(
            {section->get_index(), ELFIO::relocation_section_accessor(*self->GetElfBuilder(), section)});
        section->set_entry_size(self->GetElfBuilder()->get_default_entry_size(ELFIO::SHT_REL));
    }
    self->index_map_[section->get_index()] = self->sections_.size();
    self->sections_.push_back(section);

    section->set_type(type);
    section->set_flags(flags);
    section->set_link(3U);
    section->set_info(info);

    return section->get_index();
}

DwarfBuilder::DwarfBuilder(Arch arch, ELFIO::elfio *elf) : elf_builder_(elf), arch_(arch)
{
    auto to_u {[](int n) { return static_cast<Dwarf_Unsigned>(n); }};

    Dwarf_Error error {nullptr};
    dwarf_producer_init(to_u(DW_DLC_WRITE) | to_u(DW_DLC_POINTER64) | to_u(DW_DLC_OFFSET32) | to_u(DW_DLC_SIZE_64) |
                            to_u(DW_DLC_SYMBOLIC_RELOCATIONS) | to_u(DW_DLC_TARGET_LITTLEENDIAN),
                        reinterpret_cast<Dwarf_Callback_Func>(DwarfBuilder::CreateSectionCallback), nullptr, nullptr,
                        this, GetIsaName(GetArch()), "V4", nullptr, &dwarf_, &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "Dwarf initialization failed: " << dwarf_errmsg(error);
    }

    int res = dwarf_pro_set_default_string_form(dwarf_, DW_FORM_string, &error);
    if (res != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_pro_set_default_string_form failed: " << dwarf_errmsg(error);
    }

    compile_unit_die_ = dwarf_new_die(dwarf_, DW_TAG_compile_unit, nullptr, nullptr, nullptr, nullptr, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(compile_unit_die_)) {
        LOG(FATAL, COMPILER) << "dwarf_new_die failed: " << dwarf_errmsg(error);
    }

    dwarf_add_AT_producer(compile_unit_die_, const_cast<char *>("ASD"), &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_add_AT_producer failed: " << dwarf_errmsg(error);
    }

    dwarf_add_die_to_debug(dwarf_, compile_unit_die_, &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_add_die_to_debug_a failed: " << dwarf_errmsg(error);
    }

    dwarf_pro_set_default_string_form(dwarf_, DW_FORM_strp, &error);

    dwarf_add_AT_unsigned_const(dwarf_, compile_unit_die_, DW_AT_language, DW_LANG_C, &error);
    if (error != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "DW_AT_language failed: " << dwarf_errmsg(error);
    }
}

bool DwarfBuilder::BuildGraph(const Function *func, uint32_t code_offset, unsigned symbol)
{
    auto graph = func->GetGraph();
    if (!graph->IsLineDebugInfoEnabled()) {
        return true;
    }

    Dwarf_Error error = nullptr;
    auto die = dwarf_new_die(dwarf_, DW_TAG_subprogram, nullptr, nullptr, nullptr, nullptr, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(die)) {
        LOG(ERROR, COMPILER) << "dwarf_new_die failed: " << dwarf_errmsg(error);
        return false;
    }

    Dwarf_P_Die res;
    if (prev_program_die_ != nullptr) {
        res = dwarf_die_link(die, nullptr, nullptr, prev_program_die_, nullptr, &error);
    } else {
        res = dwarf_die_link(die, compile_unit_die_, nullptr, nullptr, nullptr, &error);
    }
    if (error != DW_DLV_OK || IsDwarfBadAddr(res)) {
        LOG(ERROR, COMPILER) << "dwarf_die_link failed: " << dwarf_errmsg(error);
    }
    prev_program_die_ = die;

    Dwarf_P_Attribute a = dwarf_add_AT_name(die, const_cast<char *>(func->GetName()), &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(a)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b failed: " << dwarf_errmsg(error);
    }

    a = dwarf_add_AT_flag(dwarf_, die, DW_AT_external, static_cast<Dwarf_Small>(true), &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(a)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_flag failed: " << dwarf_errmsg(error);
    }

    auto source_dirs {func->GetSourceDirs()};
    auto source_files {func->GetSourceFiles()};

    if (dwarf_lne_set_address(dwarf_, 0, symbol, &error) != DW_DLV_OK) {
        LOG(FATAL, COMPILER) << "dwarf_lne_set_address failed: " << dwarf_errmsg(error);
    }

    for (auto bb : *graph) {
        if (bb == nullptr) {
            continue;
        }
        for (auto inst : bb->Insts()) {
            auto debug_info = inst->GetDebugInfo();
            if (debug_info == nullptr) {
                continue;
            }

            auto dir_index {AddDir(source_dirs[debug_info->GetDirIndex()])};
            auto file_index {AddFile(source_files[debug_info->GetFileIndex()], dir_index)};
            if (dwarf_add_line_entry(dwarf_, file_index, debug_info->GetOffset(), debug_info->GetLineNumber(), 0, 1, 0,
                                     &error) != DW_DLV_OK) {
                LOG(ERROR, COMPILER) << "dwarf_add_line_entry failed: " << dwarf_errmsg(error);
                return false;
            }
        }
    }

    if (dwarf_lne_end_sequence(dwarf_, graph->GetCode().size(), &error) != DW_DLV_OK) {
        LOG(ERROR, COMPILER) << "dwarf_lne_end_sequence failed: " << dwarf_errmsg(error);
        return false;
    }

    auto attr = dwarf_add_AT_targ_address_b(dwarf_, die, DW_AT_low_pc, 0, symbol, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_low_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    attr = dwarf_add_AT_targ_address_b(dwarf_, die, DW_AT_high_pc, graph->GetCode().size(), symbol, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_high_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    code_size_ = code_offset + graph->GetCode().size();
    return true;
}

bool DwarfBuilder::Finalize(uint32_t code_size)
{
    Dwarf_Error error {nullptr};

    auto attr = dwarf_add_AT_targ_address_b(dwarf_, compile_unit_die_, DW_AT_low_pc, 0, 0, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_low_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    attr = dwarf_add_AT_targ_address_b(dwarf_, compile_unit_die_, DW_AT_high_pc, code_size, 0, &error);
    if (error != DW_DLV_OK || IsDwarfBadAddr(attr)) {
        LOG(ERROR, COMPILER) << "dwarf_add_AT_targ_address_b(DW_AT_high_pc) failed: " << dwarf_errmsg(error);
        return false;
    }

    auto sections = dwarf_transform_to_disk_form(dwarf_, &error);
    if (error != DW_DLV_OK || sections == DW_DLV_NOCOUNT) {
        LOG(ERROR, COMPILER) << "dwarf_transform_to_disk_form failed: " << dwarf_errmsg(error);
        return false;
    }

    for (decltype(sections) i = 0; i < sections; ++i) {
        Dwarf_Unsigned len = 0;
        Dwarf_Signed elf_idx = 0;
        auto bytes = reinterpret_cast<const char *>(dwarf_get_section_bytes(dwarf_, i, &elf_idx, &len, &error));
        ASSERT(error == DW_DLV_OK);
        (void)bytes;
        sections_[index_map_[elf_idx]]->append_data(bytes, len);
    }

    Dwarf_Unsigned rel_sections_count;
    int buf_version;
    auto res = dwarf_get_relocation_info_count(dwarf_, &rel_sections_count, &buf_version, &error);
    if (res != DW_DLV_OK) {
        LOG(ERROR, COMPILER) << "dwarf_get_relocation_info_count failed: " << dwarf_errmsg(error);
        return false;
    }
    for (Dwarf_Unsigned i = 0; i < rel_sections_count; ++i) {
        Dwarf_Signed elf_index = 0;
        Dwarf_Signed link_index = 0;
        Dwarf_Unsigned buf_count;
        Dwarf_Relocation_Data data;
        if (dwarf_get_relocation_info(dwarf_, &elf_index, &link_index, &buf_count, &data, &error) != DW_DLV_OK) {
            LOG(ERROR, COMPILER) << "dwarf_get_relocation_info failed: " << dwarf_errmsg(error);
            return false;
        }
        auto &rel_writer = rel_map_.at(elf_index);
        for (Dwarf_Unsigned j = 0; j < buf_count; j++, data++) {
            if (data->drd_symbol_index == 0) {
                continue;
            }
            rel_writer.add_entry(data->drd_offset,
                                 // NOLINTNEXTLINE (hicpp-signed-bitwise)
                                 static_cast<ELFIO::Elf_Xword>(ELF64_R_INFO(data->drd_symbol_index, data->drd_type)));
        }
    }

    if (dwarf_producer_finish(dwarf_, &error) != DW_DLV_OK) {
        LOG(ERROR, COMPILER) << "dwarf_producer_finish failed: " << dwarf_errmsg(error);
        return false;
    }

    return true;
}

Dwarf_Unsigned DwarfBuilder::AddFile(const std::string &fname, Dwarf_Unsigned dir_index)
{
    if (auto it = source_files_map_.find(fname); it != source_files_map_.end()) {
        return it->second;
    }
    Dwarf_Error error {nullptr};
    Dwarf_Unsigned index = dwarf_add_file_decl(dwarf_, const_cast<char *>(fname.data()), dir_index, 0U, 0U, &error);
    if (index == static_cast<Dwarf_Unsigned>(DW_DLV_NOCOUNT)) {
        LOG(FATAL, COMPILER) << "dwarf_add_file_decl failed: " << dwarf_errmsg(error);
    }
    source_files_map_[fname] = index;
    return index;
}

Dwarf_Unsigned DwarfBuilder::AddDir(const std::string &dname)
{
    if (auto it = dirs_map_.find(dname); it != dirs_map_.end()) {
        return it->second;
    }
    Dwarf_Error error {nullptr};
    Dwarf_Unsigned index = dwarf_add_directory_decl(dwarf_, const_cast<char *>(dname.data()), &error);
    if (index == static_cast<Dwarf_Unsigned>(DW_DLV_NOCOUNT)) {
        LOG(FATAL, COMPILER) << "dwarf_add_file_decl failed: " << dwarf_errmsg(error);
    }
    dirs_map_[dname] = index;
    return index;
}
}  // namespace panda::irtoc
