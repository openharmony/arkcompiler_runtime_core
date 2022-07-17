/**
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

#ifndef COMPILER_AOT_AOT_BULDER_AOT_FILE_BUILDER_H
#define COMPILER_AOT_AOT_BULDER_AOT_FILE_BUILDER_H

#include <string>
#include <vector>
#include "aot/compiled_method.h"
#include "aot/aot_file.h"
#include "elf_builder.h"
#include "utils/arch.h"
#include "utils/arena_containers.h"
#include "utils/bit_vector.h"
#include "optimizer/ir/runtime_interface.h"
#include "mem/gc/gc_types.h"

namespace panda {
class Class;
}  // namespace panda

namespace panda::compiler {

template <Arch arch, bool is_jit_mode>
class ElfBuilder;

class AotBuilder : public ElfWriter {
public:
    void SetGcType(uint32_t gc_type)
    {
        gc_type_ = gc_type;
    }
    uint32_t GetGcType() const
    {
        return gc_type_;
    }

    uint64_t *GetIntfInlineCacheIndex()
    {
        return &intf_inline_cache_index_;
    }

    int Write(const std::string &cmdline, const std::string &file_name);

    void StartFile(const std::string &name, uint32_t checksum);
    void EndFile();

    auto *GetGotPlt()
    {
        return &got_plt_;
    }

    auto *GetGotVirtIndexes()
    {
        return &got_virt_indexes_;
    }

    auto *GetGotClass()
    {
        return &got_class_;
    }

    auto *GetGotString()
    {
        return &got_string_;
    }

    auto *GetGotIntfInlineCache()
    {
        return &got_intf_inline_cache_;
    }

    void SetBootAot(bool boot_aot)
    {
        boot_aot_ = boot_aot;
    }

    void SetWithCha(bool with_cha)
    {
        with_cha_ = with_cha;
    }

    void SetGenerateSymbols(bool generate_symbols)
    {
        generate_symbols_ = generate_symbols;
    }

    void AddClassHashTable(const panda_file::File &panda_file);

    void InsertEntityPairHeader(uint32_t class_hash, uint32_t class_id)
    {
        entity_pair_headers_.emplace_back();
        auto &entity_pair = entity_pair_headers_.back();
        entity_pair.descriptor_hash = class_hash;
        entity_pair.entity_id_offset = class_id;
    }

    auto *GetEntityPairHeaders() const
    {
        return &entity_pair_headers_;
    }

    void InsertClassHashTableSize(uint32_t size)
    {
        class_hash_tables_size_.emplace_back(size);
    }

    auto *GetClassHashTableSize() const
    {
        return &class_hash_tables_size_;
    }

private:
    template <Arch arch>
    int WriteImpl(const std::string &cmdline, const std::string &file_name);

    template <Arch arch>
    void GenerateSymbols(ElfBuilder<arch> &builder);

    template <Arch arch>
    void EmitPlt(Span<typename ArchTraits<arch>::WordType> ptr_view, size_t got_data_size);

    void FillHeader(const std::string &cmdline, const std::string &file_name);

    void ResolveConflictClassHashTable(const panda_file::File &panda_file,
                                       std::vector<unsigned int> conflict_entity_table, size_t conflict_num,
                                       std::vector<panda_file::EntityPairHeader> &entity_pairs);

private:
    std::string file_name_;
    compiler::AotHeader aot_header_ {};
    uint32_t gc_type_ {static_cast<uint32_t>(mem::GCType::INVALID_GC)};
    uint64_t intf_inline_cache_index_ {0};
    std::map<std::pair<const panda_file::File *, uint32_t>, int32_t> got_plt_;
    std::map<std::pair<const panda_file::File *, uint32_t>, int32_t> got_virt_indexes_;
    std::map<std::pair<const panda_file::File *, uint32_t>, int32_t> got_class_;
    std::map<std::pair<const panda_file::File *, uint32_t>, int32_t> got_string_;
    std::map<std::pair<const panda_file::File *, uint64_t>, int32_t> got_intf_inline_cache_;
    bool boot_aot_ {false};
    bool with_cha_ {true};
    bool generate_symbols_ {false};

    std::vector<panda_file::EntityPairHeader> entity_pair_headers_;
    std::vector<uint32_t> class_hash_tables_size_;
    friend class CodeDataProvider;
    friend class JitCodeDataProvider;
};

}  // namespace panda::compiler

#endif  // COMPILER_AOT_AOT_BULDER_AOT_FILE_BUILDER_H
