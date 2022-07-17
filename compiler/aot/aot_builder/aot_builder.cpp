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

#include "aot_builder.h"
#include "aot/aot_file.h"
#include "elf_builder.h"
#include "include/class.h"
#include "include/method.h"
#include "optimizer/code_generator/encode.h"
#include "code_info/code_info.h"

#include <numeric>

namespace panda::compiler {

/**
 * Fills text section in the ELF builder by the code from the methods in AotBuilder.
 */
class CodeDataProvider : public ElfSectionDataProvider {
public:
    explicit CodeDataProvider(AotBuilder *aot_builder) : aot_builder_(aot_builder) {}

    void FillData(Span<uint8_t> stream, size_t stream_begin) const override
    {
        const size_t code_offset = CodeInfo::GetCodeOffset(aot_builder_->GetArch());
        CodePrefix prefix;
        size_t curr_pos = stream_begin;
        for (size_t i = 0; i < aot_builder_->methods_.size(); i++) {
            auto &method = aot_builder_->methods_[i];
            auto &method_header = aot_builder_->method_headers_[i];
            prefix.code_size = method.GetCode().size();
            prefix.code_info_offset = code_offset + RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT);
            prefix.code_info_size = method.GetCodeInfo().size();
            // Prefix
            curr_pos = stream_begin + method_header.code_offset;
            const char *data = reinterpret_cast<char *>(&prefix);
            CopyToSpan(stream, data, sizeof(prefix), curr_pos);
            curr_pos += sizeof(prefix);

            // Code
            curr_pos += code_offset - sizeof(prefix);
            data = reinterpret_cast<const char *>(method.GetCode().data());
            CopyToSpan(stream, data, method.GetCode().size(), curr_pos);
            curr_pos += method.GetCode().size();

            // CodeInfo
            curr_pos += RoundUp(method.GetCode().size(), CodeInfo::ALIGNMENT) - method.GetCode().size();
            data = reinterpret_cast<const char *>(method.GetCodeInfo().data());
            CopyToSpan(stream, data, method.GetCodeInfo().size(), curr_pos);
        }
    }

    size_t GetDataSize() const override
    {
        return aot_builder_->current_code_size_;
    }

private:
    AotBuilder *aot_builder_;
};

void AotBuilder::StartFile(const std::string &name, uint32_t checksum)
{
    auto &file_header = file_headers_.emplace_back();
    file_header.classes_offset = class_headers_.size();
    file_header.file_checksum = checksum;
    file_header.file_offset = 0;
    file_header.file_name_str = AddString(name);
    file_header.methods_offset = method_headers_.size();
}

void AotBuilder::EndFile()
{
    ASSERT(!file_headers_.empty());
    auto &file_header = file_headers_.back();
    file_header.classes_count = class_headers_.size() - file_header.classes_offset;
    if (file_header.classes_count == 0 && (class_hash_tables_size_.empty() || class_hash_tables_size_.back() == 0)) {
        /* Just return, if there is nothing compiled in the file */
        CHECK_EQ(file_header.methods_count, 0U);
        file_headers_.pop_back();
        return;
    }
    ASSERT(!class_hash_tables_size_.empty());
    file_header.class_hash_table_offset =
        (entity_pair_headers_.size() - class_hash_tables_size_.back()) * sizeof(panda_file::EntityPairHeader);
    file_header.class_hash_table_size = class_hash_tables_size_.back();
    file_header.methods_count = method_headers_.size() - file_header.methods_offset;
    // We should keep class headers sorted, since AOT manager uses binary search to find classes.
    std::sort(class_headers_.begin() + file_header.classes_offset, class_headers_.end(),
              [](const auto &a, const auto &b) { return a.class_id < b.class_id; });
}

int AotBuilder::Write(const std::string &cmdline, const std::string &file_name)
{
    switch (arch_) {
        case Arch::AARCH32:
            return WriteImpl<Arch::AARCH32>(cmdline, file_name);
        case Arch::AARCH64:
            return WriteImpl<Arch::AARCH64>(cmdline, file_name);
        case Arch::X86:
            return WriteImpl<Arch::X86>(cmdline, file_name);
        case Arch::X86_64:
            return WriteImpl<Arch::X86_64>(cmdline, file_name);
        default:
            LOG(ERROR, COMPILER) << "AotBuilder: Unsupported arch";
            return 1;
    }
}

void AotBuilder::FillHeader(const std::string &cmdline, const std::string &file_name)
{
    aot_header_.magic = compiler::AotFile::MAGIC;
    aot_header_.version = compiler::AotFile::VERSION;
    aot_header_.checksum = 0;  // TODO(msherstennikov)
    ASSERT(GetRuntime() != nullptr);
    aot_header_.environment_checksum = GetRuntime()->GetEnvironmentChecksum(arch_);
    aot_header_.arch = static_cast<uint32_t>(arch_);
    aot_header_.gc_type = gc_type_;
    aot_header_.files_offset = sizeof(aot_header_);
    aot_header_.files_count = file_headers_.size();
    aot_header_.class_hash_tables_offset =
        aot_header_.files_offset + aot_header_.files_count * sizeof(compiler::PandaFileHeader);
    size_t class_hash_tables_size = entity_pair_headers_.size() * sizeof(panda_file::EntityPairHeader);
    aot_header_.classes_offset = aot_header_.class_hash_tables_offset + class_hash_tables_size;
    aot_header_.methods_offset = aot_header_.classes_offset + class_headers_.size() * sizeof(compiler::ClassHeader);
    aot_header_.bitmap_offset = aot_header_.methods_offset + methods_.size() * sizeof(compiler::MethodHeader);
    size_t bitmaps_size =
        std::accumulate(class_methods_bitmaps_.begin(), class_methods_bitmaps_.end(), 0U,
                        [](size_t sum, const auto &vec) { return vec.GetContainerSizeInBytes() + sum; });
    aot_header_.strtab_offset = aot_header_.bitmap_offset + bitmaps_size;
    aot_header_.file_name_str = AddString(file_name);
    aot_header_.cmdline_str = AddString(cmdline);
    aot_header_.boot_aot = static_cast<uint32_t>(boot_aot_);
    aot_header_.with_cha = static_cast<uint32_t>(with_cha_);
    aot_header_.class_ctx_str = AddString(class_ctx_);
}

template <Arch arch>
int AotBuilder::WriteImpl(const std::string &cmdline, const std::string &file_name)
{
    constexpr size_t PAGE_SIZE_BYTES = 0x1000;
    constexpr size_t CALL_STATIC_SLOT_SIZE = 3;
    constexpr size_t CALL_VIRTUAL_SLOT_SIZE = 2;
    constexpr size_t STRING_SLOT_SIZE = 2;
    constexpr size_t INLINE_CACHE_SLOT_SIZE = 1;
    ElfBuilder<arch> builder;

    auto text_section = builder.GetTextSection();
    auto aot_section = builder.GetAotSection();
    auto got_section = builder.GetGotSection();
    std::vector<uint8_t> &got_data = got_section->GetVector();
    // +1 is the extra slot that indicates the end of the aot table
    auto got_data_size = static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) + 1 +
                         CALL_STATIC_SLOT_SIZE * (got_plt_.size() + got_class_.size()) +
                         CALL_VIRTUAL_SLOT_SIZE * got_virt_indexes_.size() + STRING_SLOT_SIZE * got_string_.size() +
                         INLINE_CACHE_SLOT_SIZE * got_intf_inline_cache_.size();
    // We need to fill the whole segment with aot_got section because it is filled from the end.
    got_data.resize(RoundUp(PointerSize(arch) * got_data_size, PAGE_SIZE_BYTES), 0);

    GenerateSymbols(builder);

    FillHeader(cmdline, file_name);

    aot_section->AppendData(&aot_header_, sizeof(aot_header_));
    aot_section->AppendData(file_headers_.data(), file_headers_.size() * sizeof(compiler::PandaFileHeader));
    aot_section->AppendData(entity_pair_headers_.data(),
                            entity_pair_headers_.size() * sizeof(panda_file::EntityPairHeader));
    aot_section->AppendData(class_headers_.data(), class_headers_.size() * sizeof(compiler::ClassHeader));
    aot_section->AppendData(method_headers_.data(), method_headers_.size() * sizeof(compiler::MethodHeader));

    for (auto &bitmap : class_methods_bitmaps_) {
        aot_section->AppendData(bitmap.data(), bitmap.GetContainerSizeInBytes());
    }
    aot_section->AppendData(string_table_.data(), string_table_.size());

    CodeDataProvider code_provider(this);
    text_section->SetDataProvider(&code_provider);

    using PtrType = typename ArchTraits<arch>::WordType;
    auto ptr_view = Span(got_data).template SubSpan<PtrType>(0, got_data.size() / sizeof(PtrType));
    EmitPlt<arch>(ptr_view, got_data_size);

#ifdef PANDA_COMPILER_CFI
    builder.SetFrameData(&frame_data_);
#endif
    builder.Build(file_name);
    builder.Write(file_name);
    return 0;
}

template <Arch arch>
void AotBuilder::EmitPlt(Span<typename ArchTraits<arch>::WordType> ptr_view, size_t got_data_size)
{
    if (!got_plt_.empty() || !got_virt_indexes_.empty() || !got_class_.empty() || !got_string_.empty() ||
        !got_intf_inline_cache_.empty()) {
        ASSERT(PointerSize(arch) >= sizeof(uint32_t));

        auto ptr_cnt = ptr_view.Size();
        auto end = static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT);

        ptr_view[ptr_cnt - got_data_size] = 0;
        constexpr size_t IMM_2 = 2;
        for (auto [method, idx] : got_plt_) {
            ASSERT(idx <= 0);
            ptr_view[ptr_cnt - end + idx] = AotFile::AotSlotType::PLT_SLOT;
            ptr_view[ptr_cnt - end + idx - IMM_2] = method.second;
        }
        for (auto [method, idx] : got_virt_indexes_) {
            ASSERT(idx <= 0);
            ptr_view[ptr_cnt - end + idx] = AotFile::AotSlotType::VTABLE_INDEX;
            ptr_view[ptr_cnt - end + idx - 1] = method.second;
        }
        for (auto [klass, idx] : got_class_) {
            ASSERT(idx <= 0);
            ptr_view[ptr_cnt - end + idx] = AotFile::AotSlotType::CLASS_SLOT;
            ptr_view[ptr_cnt - end + idx - IMM_2] = klass.second;
        }
        for (auto [string_id, idx] : got_string_) {
            ASSERT(idx <= 0);
            ptr_view[ptr_cnt - end + idx] = AotFile::AotSlotType::STRING_SLOT;
            ptr_view[ptr_cnt - end + idx - 1] = string_id.second;
        }
        for (auto [cache, idx] : got_intf_inline_cache_) {
            (void)cache;
            ASSERT(idx < 0);
            ptr_view[ptr_cnt - end + idx] = AotFile::AotSlotType::INLINECACHE_SLOT;
        }
    }
}

/**
 * Add method names to the symbol table
 */
template <Arch arch>
void AotBuilder::GenerateSymbols(ElfBuilder<arch> &builder)
{
    if (generate_symbols_) {
        auto text_section = builder.GetTextSection();
        size_t offset = 0;
        std::string method_name;
        for (auto &method : methods_) {
            if (method.GetMethod()->GetPandaFile() == nullptr) {
                method_name = "Error: method doesn't belong to any panda file";
            } else {
                auto method_casted = reinterpret_cast<RuntimeInterface::MethodPtr>(method.GetMethod());
                method_name = runtime_->GetMethodFullName(method_casted, true);
            }
            builder.template AddSymbol<true>(
                method_name, method.GetOverallSize(), *text_section, [offset, text_section]() {
                    return text_section->GetAddress() + offset + CodeInfo::GetCodeOffset(arch);
                });
            offset += RoundUp(method.GetOverallSize(), ArchTraits<arch>::CODE_ALIGNMENT);
        }
    }
}

void AotBuilder::AddClassHashTable(const panda_file::File &panda_file)
{
    const panda_file::File::Header *header = panda_file.GetHeader();
    uint32_t num_classes = header->num_classes;
    if (num_classes == 0) {
        return;
    }

    size_t hash_table_size = panda::helpers::math::GetPowerOfTwoValue32(num_classes);
    std::vector<panda_file::EntityPairHeader> entity_pairs;
    std::vector<unsigned int> conflict_entity_table;
    entity_pairs.resize(hash_table_size);
    conflict_entity_table.resize(hash_table_size);
    size_t conflict_num = 0;

    auto classes = panda_file.GetClasses();
    for (size_t i = 0; i < num_classes; ++i) {
        auto entity_id = panda_file::File::EntityId(classes[i]);
        auto name = panda_file.GetStringData(entity_id).data;
        uint32_t hash = GetHash32String(name);
        uint32_t pos = hash & (hash_table_size - 1);
        auto &entity_pair = entity_pairs[pos];
        if (entity_pair.descriptor_hash == 0) {
            entity_pair.descriptor_hash = hash;
            entity_pair.entity_id_offset = entity_id.GetOffset();
        } else {
            conflict_entity_table[conflict_num] = i;
            conflict_num++;
        }
    }
    if (conflict_num == 0) {
        entity_pair_headers_.insert(entity_pair_headers_.end(), entity_pairs.begin(), entity_pairs.end());
        class_hash_tables_size_.emplace_back(entity_pairs.size());
    } else {
        ResolveConflictClassHashTable(panda_file, std::move(conflict_entity_table), conflict_num, entity_pairs);
    }
}

void AotBuilder::ResolveConflictClassHashTable(const panda_file::File &panda_file,
                                               std::vector<unsigned int> conflict_entity_table, size_t conflict_num,
                                               std::vector<panda_file::EntityPairHeader> &entity_pairs)
{
    auto classes = panda_file.GetClasses();
    auto hash_table_size = entity_pairs.size();
    for (size_t j = 0; j < conflict_num; ++j) {
        if (j > 0 && conflict_entity_table[j - 1] == conflict_entity_table[j]) {
            break;  // Exit for loop if there is no conlict elements anymore
        }
        auto i = conflict_entity_table[j];
        auto entity_id = panda_file::File::EntityId(classes[i]);
        auto name = panda_file.GetStringData(entity_id).data;
        uint32_t hash = GetHash32String(name);
        uint32_t theory_pos = hash & (hash_table_size - 1);
        ASSERT(entity_pairs[theory_pos].descriptor_hash != 0);

        uint32_t actual_pos = theory_pos;
        while (actual_pos < (hash_table_size - 1) && entity_pairs[actual_pos].descriptor_hash != 0) {
            actual_pos++;
        }
        if (actual_pos == (hash_table_size - 1) && entity_pairs[actual_pos].descriptor_hash != 0) {
            actual_pos = 0;
            while (actual_pos < theory_pos && entity_pairs[actual_pos].descriptor_hash != 0) {
                actual_pos++;
            }
        }
        ASSERT(entity_pairs[actual_pos].descriptor_hash == 0);
        auto &entity_pair = entity_pairs[actual_pos];
        entity_pair.descriptor_hash = hash;
        entity_pair.entity_id_offset = entity_id.GetOffset();
        while (entity_pairs[theory_pos].next_pos != 0) {
            theory_pos = entity_pairs[theory_pos].next_pos - 1;
        }
        // add 1 is to distinguish the initial value 0 of next_pos and the situation that the next pos is really 0
        entity_pairs[theory_pos].next_pos = actual_pos + 1;
    }
    entity_pair_headers_.insert(entity_pair_headers_.end(), entity_pairs.begin(), entity_pairs.end());
    class_hash_tables_size_.emplace_back(entity_pairs.size());
}

}  // namespace panda::compiler
