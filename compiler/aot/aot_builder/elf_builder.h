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

#ifndef COMPILER_AOT_AOT_BULDER_ELF_BUILDER_H
#define COMPILER_AOT_AOT_BULDER_ELF_BUILDER_H

#include <elf.h>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <functional>
#ifdef PANDA_COMPILER_CFI
#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>
#endif

#include "include/class.h"

namespace panda::compiler {

class ElfSectionDataProvider {
public:
    using DataCallback = void (*)(Span<const uint8_t>);

    virtual void FillData(Span<uint8_t> os, size_t pos) const = 0;
    virtual size_t GetDataSize() const = 0;

    ElfSectionDataProvider() = default;
    NO_COPY_SEMANTIC(ElfSectionDataProvider);
    NO_MOVE_SEMANTIC(ElfSectionDataProvider);
    virtual ~ElfSectionDataProvider() = default;
};

#ifdef PANDA_COMPILER_CFI
class DwarfSectionData {
public:
    using FdeInst = std::tuple<Dwarf_Small, Dwarf_Unsigned, Dwarf_Unsigned>;
    DwarfSectionData() = default;

    void AddFdeInst(Dwarf_Small op, Dwarf_Unsigned val1, Dwarf_Unsigned val2)
    {
        fde_.emplace_back(op, val1, val2);
    }

    void AddFdeInst(const FdeInst &fde)
    {
        fde_.push_back(fde);
    }

    void SetOffset(Dwarf_Unsigned offset)
    {
        offset_ = offset;
    }

    void SetSize(Dwarf_Unsigned size)
    {
        size_ = size;
    }

    const auto &GetFde() const
    {
        return fde_;
    }

    auto GetOffset() const
    {
        return offset_;
    }

    auto GetSize() const
    {
        return size_;
    }

private:
    std::vector<FdeInst> fde_;
    Dwarf_Unsigned offset_ {0};
    Dwarf_Unsigned size_ {0};
};
#endif  // PANDA_COMPILER_CFI

template <Arch arch, bool is_jit_mode = false>
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class ElfBuilder {
    static constexpr size_t PAGE_SIZE_VALUE = 0x1000;
    using ElfAddr = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Addr, Elf32_Addr>;
    using ElfOff = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Off, Elf32_Off>;
    using ElfHalf = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Half, Elf32_Half>;
    using ElfWord = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Word, Elf32_Word>;
    using ElfSword = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Sword, Elf32_Sword>;
    using ElfXword = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Xword, Elf32_Xword>;
    using ElfSxword = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Sxword, Elf32_Sxword>;
    using ElfEhdr = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Ehdr, Elf32_Ehdr>;
    using ElfShdr = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Shdr, Elf32_Shdr>;
    using ElfSym = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Sym, Elf32_Sym>;
    using ElfRel = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Rel, Elf32_Rel>;
    using ElfRela = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Rela, Elf32_Rela>;
    using ElfPhdr = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Phdr, Elf32_Phdr>;
    using ElfDyn = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Dyn, Elf32_Dyn>;
    using ElfSection = std::conditional_t<ArchTraits<arch>::IS_64_BITS, Elf64_Section, Elf32_Section>;

public:
    class Section {
    public:
        // NOLINTNEXTLINE(modernize-pass-by-value)
        Section(ElfBuilder &builder, const std::string &name, ElfWord type, ElfWord flags, Section *link, ElfWord info,
                ElfWord align, ElfWord entsize)
            : builder_(builder), name_(name), header_ {}, link_(link)
        {
            header_.sh_type = type;
            header_.sh_flags = flags;
            header_.sh_info = info;
            header_.sh_addralign = align;
            header_.sh_entsize = entsize;
        }
        virtual ~Section() = default;
        NO_MOVE_SEMANTIC(Section);
        NO_COPY_SEMANTIC(Section);

        virtual void *GetData()
        {
            UNREACHABLE();
        }
        virtual size_t GetDataSize() const
        {
            return header_.sh_size;
        }

        const std::string &GetName() const
        {
            return name_;
        }

        size_t GetIndex() const
        {
            return index_;
        }

        auto GetAddress() const
        {
            return header_.sh_addr;
        }

        void SetSize(size_t size)
        {
            header_.sh_size = size;
        }

        void SetDataProvider(ElfSectionDataProvider *data_provider)
        {
            data_provider_ = data_provider;
        }

        ElfSectionDataProvider *GetDataProvider() const
        {
            return data_provider_;
        }

    private:
        ElfBuilder &builder_;
        std::string name_;
        size_t index_ {std::numeric_limits<size_t>::max()};
        ElfShdr header_ {};
        Section *link_ {};
        ElfSectionDataProvider *data_provider_ {nullptr};

        friend class ElfBuilder;
    };

    class DataSection : public Section {
    public:
        using Section::Section;

        DataSection() = default;
        NO_COPY_SEMANTIC(DataSection);
        NO_MOVE_SEMANTIC(DataSection);
        ~DataSection() override = default;  // NOLINT(hicpp-use-override, modernize-use-override)

        void AppendData(const void *data, size_t size)
        {
            auto pdata = reinterpret_cast<const uint8_t *>(data);
            data_.insert(data_.end(), pdata, pdata + size);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }

        void *GetData() override
        {
            return data_.data();
        }

        auto &GetVector()
        {
            return data_;
        }

        size_t GetDataSize() const override
        {
            return this->data_provider_ != nullptr ? this->data_provider_->GetDataSize() : data_.size();
        }

    private:
        std::vector<uint8_t> data_;
    };

    class SymbolSection : public Section {
    public:
        using ThunkFunc = std::function<ElfAddr(void)>;

        using Section::Section;
        SymbolSection(ElfBuilder &builder, const std::string &name, ElfWord type, ElfWord flags, Section *link,
                      ElfWord info, ElfWord align, ElfWord entsize)
            : Section(builder, name, type, flags, link, info, align, entsize)
        {
            symbols_.emplace_back(ElfSym {});
            thunks_.emplace_back();
        }

        void *GetData() override
        {
            return symbols_.data();
        }

        size_t GetDataSize() const override
        {
            return symbols_.size() * sizeof(ElfSym);
        }

        void Resolve();

    private:
        std::vector<ElfSym> symbols_;
        std::vector<ThunkFunc> thunks_;

        friend class ElfBuilder;
    };

    class StringSection : public DataSection {
    public:
        StringSection(ElfBuilder &builder, const std::string &name, ElfWord flags, ElfWord align)
            : DataSection(builder, name, SHT_STRTAB, flags, nullptr, 0, align, 0)
        {
            AddString("\0");  // NOLINT(bugprone-string-literal-with-embedded-nul)
        }

        ElfWord AddString(const std::string &str)
        {
            auto pos = DataSection::GetDataSize();
            DataSection::AppendData(str.data(), str.size() + 1);
            return pos;
        }
    };

public:
    ~ElfBuilder()
    {
        for (auto segment : segments_) {
            delete segment;
        }
    }

    ElfBuilder()
    {
        AddSection(&hash_section_);
        AddSection(&text_section_);
        AddSection(&shstrtab_section_);
        AddSection(&dynstr_section_);
        AddSection(&dynsym_section_);
        if constexpr (!is_jit_mode) {  // NOLINT
            AddSection(&aot_section_);
            AddSection(&got_section_);
            AddSection(&dynamic_section_);
        }
#ifdef PANDA_COMPILER_CFI
        AddSection(&frame_section_);
#endif
    }

    NO_MOVE_SEMANTIC(ElfBuilder);
    NO_COPY_SEMANTIC(ElfBuilder);

    ElfWord AddSectionName(const std::string &name)
    {
        return name.empty() ? 0 : shstrtab_section_.AddString(name);
    }

    void AddSection(Section *section)
    {
        sections_.push_back(section);
        section->index_ = sections_.size() - 1;
    }

    template <bool is_function = false>
    void AddSymbol(const std::string &name, ElfWord size, const Section &section,
                   typename SymbolSection::ThunkFunc thunk);

    auto GetTextSection()
    {
        return &text_section_;
    }

    auto GetAotSection()
    {
        return &aot_section_;
    }

#ifdef PANDA_COMPILER_CFI
    auto GetFrameSection()
    {
        return &frame_section_;
    }

    void SetFrameData(std::vector<DwarfSectionData> *frame_data)
    {
        frame_data_ = frame_data;
    }
    void FillFrameSection();

    void SetCodeName(const std::string &method_name)
    {
        method_name_ = method_name;
    }
#endif

    auto GetGotSection()
    {
        return &got_section_;
    }

    void Build(const std::string &file_name);

    void SettleSection(Section *section);

    void Write(const std::string &file_name);
    void Write(Span<uint8_t> stream);

    ElfOff UpdateOffset(ElfWord align)
    {
        current_offset_ = RoundUp(current_offset_, align);
        return current_offset_;
    }

    ElfOff UpdateAddress(ElfWord align)
    {
        current_address_ = RoundUp(current_address_, align);
        return current_address_;
    }

    size_t GetFileSize() const
    {
        return current_offset_;
    }

#ifdef PANDA_COMPILER_CFI
    std::vector<DwarfSectionData> *GetFrameData()
    {
        return frame_data_;
    }

    Span<uint8_t> GetTextSectionData() const
    {
        static_assert(is_jit_mode);
        return {reinterpret_cast<uint8_t *>(text_section_.header_.sh_addr), text_section_.header_.sh_size};
    }

    void HackAddressesForJit(const uint8_t *elf_data)
    {
        static_assert(is_jit_mode);
        ASSERT(frame_data_->size() == 1U);
        ASSERT(segments_.empty());
        ASSERT(dynsym_section_.symbols_.size() == 2U);

        for (auto section : sections_) {
            if ((section->header_.sh_flags & SHF_ALLOC) != 0) {  // NOLINT(hicpp-signed-bitwise)
                section->header_.sh_addr += down_cast<typename ElfBuilder<arch, is_jit_mode>::ElfAddr>(elf_data);
            }
        }

        ASSERT(dynsym_section_.symbols_[0].st_value == 0);
        dynsym_section_.symbols_[1U].st_value +=
            down_cast<typename ElfBuilder<arch, is_jit_mode>::ElfAddr>(elf_data) + CodeInfo::GetCodeOffset(arch);

        // Some dark magic there. Here we patch the address of JIT code in frame debug info entry.
        // TODO (asidorov): rework to more readable code
        uint8_t *cie_addr_8 {static_cast<uint8_t *>(frame_section_.GetData())};
        uint32_t *cie_addr_32 {reinterpret_cast<uint32_t *>(cie_addr_8)};
        uint32_t cie_length {*cie_addr_32 + static_cast<uint32_t>(sizeof(uint32_t))};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t *fde_initial_pc_addr_8 {cie_addr_8 + cie_length + 2U * sizeof(uint32_t)};
        uintptr_t *fde_initial_pc_addr_ptr {reinterpret_cast<uintptr_t *>(fde_initial_pc_addr_8)};
        *fde_initial_pc_addr_ptr += down_cast<typename ElfBuilder<arch, is_jit_mode>::ElfAddr>(elf_data);
    }
#endif

private:
    void MakeHeader();
    void AddSymbols();
    void SettleSectionsForAot();
    void SettleSectionsForJit();
    void ConstructHashSection();
    void ConstructDynamicSection(const std::string &file_name);

    struct Segment {
        Segment(ElfAddr addr, ElfOff offset, ElfWord type, ElfWord flags, ElfWord align)
        {
            header.p_type = type;
            header.p_flags = flags;
            header.p_vaddr = header.p_paddr = addr;
            header.p_align = align;
            header.p_offset = offset;
        }

        ElfPhdr header {};  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    class SegmentScope {
    public:
        template <typename... Args>
        SegmentScope(ElfBuilder &builder, ElfWord type, ElfWord flags, bool first = false)
            : builder_(builder),
              start_address_(first ? 0 : RoundUp(builder.current_address_, PAGE_SIZE_VALUE)),
              start_offset_(first ? 0 : RoundUp(builder.current_offset_, PAGE_SIZE_VALUE))
        {
            auto *segment = new Segment(start_address_, start_offset_, type, flags, PAGE_SIZE_VALUE);
            builder_.segments_.push_back(segment);
            builder_.current_segment_ = segment;
        }
        ~SegmentScope()
        {
            builder_.current_segment_->header.p_filesz = builder_.current_offset_ - start_offset_;
            builder_.current_segment_->header.p_memsz = builder_.current_address_ - start_address_;
            builder_.current_segment_ = nullptr;
        }

        NO_MOVE_SEMANTIC(SegmentScope);
        NO_COPY_SEMANTIC(SegmentScope);

    private:
        ElfBuilder &builder_;
        ElfAddr start_address_;
        ElfAddr start_offset_;
    };

    class AddrPatch {
        using PatchFunc = std::function<ElfAddr(void)>;

    public:
        AddrPatch(ElfAddr *addr, PatchFunc func) : address_(addr), patch_func_(std::move(func)) {}
        void Patch()
        {
            *address_ = patch_func_();
        }

    private:
        ElfAddr *address_;
        PatchFunc patch_func_;
    };

private:
    static constexpr size_t MAX_SEGMENTS_COUNT = 10;
    static constexpr size_t DYNSTR_SECTION_ALIGN = 8;
    static constexpr size_t JIT_TEXT_ALIGNMENT = ArchTraits<arch>::CODE_ALIGNMENT;
    static constexpr size_t JIT_DATA_ALIGNMENT = ArchTraits<arch>::POINTER_SIZE;
    static constexpr size_t JIT_DYNSTR_ALIGNMENT = 1U;
    ElfEhdr header_ {};
    std::vector<Section *> sections_;
    std::vector<Segment *> segments_;
    std::vector<AddrPatch> patches_;
    ElfAddr current_address_ {0};
    ElfOff current_offset_ {0};
    Segment *current_segment_ {nullptr};

    DataSection hash_section_ =
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(*this, ".hash", SHT_HASH, SHF_ALLOC, &dynsym_section_, 0, sizeof(ElfWord), sizeof(ElfWord));
    DataSection text_section_ =
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(*this, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR, nullptr, 0,
                    is_jit_mode ? JIT_TEXT_ALIGNMENT : PAGE_SIZE_VALUE, 0);
    StringSection shstrtab_section_ = StringSection(*this, ".shstrtab", 0, 1);
    StringSection dynstr_section_ =  // NOLINTNEXTLINE(hicpp-signed-bitwise)
        StringSection(*this, ".dynstr", SHF_ALLOC, is_jit_mode ? JIT_DYNSTR_ALIGNMENT : DYNSTR_SECTION_ALIGN);
    SymbolSection dynsym_section_ =
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        SymbolSection(*this, ".dynsym", SHT_DYNSYM, SHF_ALLOC, &dynstr_section_, 1, sizeof(ElfOff), sizeof(ElfSym));
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    DataSection aot_section_ = DataSection(*this, ".aot", SHT_PROGBITS, SHF_ALLOC, nullptr, 0, sizeof(ElfWord), 0);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    DataSection got_section_ = DataSection(*this, ".aot_got", SHT_PROGBITS, SHF_ALLOC, nullptr, 0, PAGE_SIZE_VALUE, 0);
    DataSection dynamic_section_ =
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(*this, ".dynamic", SHT_DYNAMIC, SHF_ALLOC, &dynstr_section_, 0, PAGE_SIZE_VALUE, sizeof(ElfDyn));
#ifdef PANDA_COMPILER_CFI
    DataSection frame_section_ =
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        DataSection(*this, ".eh_frame", SHT_PROGBITS, SHF_ALLOC, nullptr, 0,
                    is_jit_mode ? JIT_DATA_ALIGNMENT : PAGE_SIZE_VALUE, 0);

    std::vector<DwarfSectionData> *frame_data_ {nullptr};
#endif
    std::string method_name_ = std::string("code");

    friend SegmentScope;
};

template <Arch arch, bool is_jit_mode>
template <bool is_function>
void ElfBuilder<arch, is_jit_mode>::AddSymbol(const std::string &name, ElfWord size, const Section &section,
                                              typename SymbolSection::ThunkFunc thunk)
{
    uint8_t symbol_type = is_function ? STT_FUNC : STT_OBJECT;
    auto name_idx = dynstr_section_.AddString(name);
    auto st_info =
        // NOLINTNEXTLINE(readability-magic-numbers, hicpp-signed-bitwise)
        static_cast<uint8_t>((STB_GLOBAL << 4) + (symbol_type & 0xf));

    if constexpr (ArchTraits<arch>::IS_64_BITS) {      // NOLINT(readability-braces-around-statements)
        dynsym_section_.symbols_.push_back({name_idx,  // st_nam
                                            st_info,   // st_info
                                            0,         /* st_other */
                                            static_cast<ElfSection>(section.GetIndex()),  // st_shndx
                                            0,                                            // st_value
                                            size});                                       // st_size
    } else {                                           // NOLINT(readability-misleading-indentation)
        dynsym_section_.symbols_.push_back({name_idx,  // st_name
                                            0,         // st_value
                                            size,      // st_size
                                            st_info,   // st_info
                                            0,         // st_other
                                            static_cast<ElfSection>(section.GetIndex())});  // st_shndx
    }
    dynsym_section_.thunks_.push_back(thunk);
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::SymbolSection::Resolve()
{
    for (auto i = 0U; i < thunks_.size(); i++) {
        if (thunks_[i]) {
            symbols_[i].st_value = thunks_[i]();
        }
    }
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::ConstructDynamicSection(const std::string &file_name)
{
    using ElfDynDValType = decltype(ElfDyn::d_un.d_val);  // NOLINT(cppcoreguidelines-pro-type-union-access)
    auto soname = dynstr_section_.AddString(file_name);
    auto dynstr_section_size = dynstr_section_.GetDataSize();
    // Make sure widening is zero-extension, if any
    static_assert(std::is_unsigned<decltype(soname)>::value && std::is_unsigned<decltype(dynstr_section_size)>::value);

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    ElfDyn dyns[] = {
        {DT_HASH, {0}},    // will be patched
        {DT_STRTAB, {0}},  // will be patched
        {DT_SYMTAB, {0}},  // will be patched
        {DT_SYMENT, {sizeof(ElfSym)}},
        {DT_STRSZ, {static_cast<ElfDynDValType>(dynstr_section_size)}},
        {DT_SONAME, {static_cast<ElfDynDValType>(soname)}},
        {DT_NULL, {0}},
    };
    dynamic_section_.AppendData(&dyns, sizeof(dyns));

    auto first_patch_argument =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        reinterpret_cast<ElfAddr *>(reinterpret_cast<uint8_t *>(dynamic_section_.GetData()) +
                                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                    2U * sizeof(ElfDyn) + offsetof(ElfDyn, d_un.d_ptr));
    patches_.emplace_back(first_patch_argument, [this]() { return dynsym_section_.GetAddress(); });

    auto second_patch_argument =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        reinterpret_cast<ElfAddr *>(reinterpret_cast<uint8_t *>(dynamic_section_.GetData()) +
                                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                    1U * sizeof(ElfDyn) + offsetof(ElfDyn, d_un.d_ptr));
    patches_.emplace_back(second_patch_argument, [this]() { return dynstr_section_.GetAddress(); });

    auto third_patch_argument =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        reinterpret_cast<ElfAddr *>(reinterpret_cast<uint8_t *>(dynamic_section_.GetData()) +
                                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                    0U * sizeof(ElfDyn) + offsetof(ElfDyn, d_un.d_ptr));
    patches_.emplace_back(third_patch_argument, [this]() { return hash_section_.GetAddress(); });
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::Build(const std::string &file_name)
{
    shstrtab_section_.header_.sh_name = AddSectionName(shstrtab_section_.GetName());
    for (auto section : sections_) {
        section->header_.sh_name = AddSectionName(section->GetName());
        if (section->link_) {
            section->header_.sh_link = section->link_->GetIndex();
        }
    }

    AddSymbols();

    ConstructHashSection();

    if constexpr (!is_jit_mode) {  // NOLINT
        ConstructDynamicSection(file_name);
    }

    if constexpr (is_jit_mode) {  // NOLINT
        SettleSectionsForJit();
    } else {  // NOLINT
        SettleSectionsForAot();
    }

    MakeHeader();

    dynsym_section_.Resolve();
    std::for_each(patches_.begin(), patches_.end(), [](auto &patch) { patch.Patch(); });
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::ConstructHashSection()
{
    ElfWord value = 1;
    auto sym_count = dynsym_section_.GetDataSize() / sizeof(ElfSym);
    hash_section_.AppendData(&value, sizeof(value));
    hash_section_.AppendData(&sym_count, sizeof(value));
    hash_section_.AppendData(&value, sizeof(value));
    value = 0;
    hash_section_.AppendData(&value, sizeof(value));
    for (auto i = 2U; i < sym_count; i++) {
        hash_section_.AppendData(&i, sizeof(value));
    }
    value = 0;
    hash_section_.AppendData(&value, sizeof(value));
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::MakeHeader()
{
    header_.e_ident[EI_MAG0] = ELFMAG0;
    header_.e_ident[EI_MAG1] = ELFMAG1;
    header_.e_ident[EI_MAG2] = ELFMAG2;
    header_.e_ident[EI_MAG3] = ELFMAG3;
    header_.e_ident[EI_CLASS] = ArchTraits<arch>::IS_64_BITS ? ELFCLASS64 : ELFCLASS32;
    header_.e_ident[EI_DATA] = ELFDATA2LSB;
    header_.e_ident[EI_VERSION] = EV_CURRENT;
    header_.e_ident[EI_OSABI] = ELFOSABI_LINUX;
    header_.e_ident[EI_ABIVERSION] = 0;
    std::fill_n(&header_.e_ident[EI_PAD], EI_NIDENT - EI_PAD, 0);
    header_.e_type = ET_DYN;
    header_.e_version = 1;
    switch (arch) {
        case Arch::AARCH32:
            header_.e_machine = EM_ARM;
            header_.e_flags = EF_ARM_EABI_VER5;
            break;
        case Arch::AARCH64:
            header_.e_machine = EM_AARCH64;
            header_.e_flags = 0;
            break;
        case Arch::X86:
            header_.e_machine = EM_386;
            header_.e_flags = 0;
            break;
        case Arch::X86_64:
            header_.e_machine = EM_X86_64;
            header_.e_flags = 0;
            break;
        default:
            UNREACHABLE();
    }
    header_.e_entry = 0;
    header_.e_ehsize = sizeof(ElfEhdr);
    header_.e_phentsize = sizeof(ElfPhdr);
    header_.e_shentsize = sizeof(ElfShdr);
    header_.e_shstrndx = shstrtab_section_.GetIndex();
    current_offset_ = RoundUp(current_offset_, alignof(ElfShdr));
    header_.e_shoff = UpdateOffset(alignof(ElfShdr));
    current_offset_ += sections_.size() * sizeof(ElfShdr);
    header_.e_phoff = is_jit_mode ? 0U : sizeof(ElfEhdr);
    header_.e_shnum = sections_.size();
    header_.e_phnum = is_jit_mode ? 0U : segments_.size();
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::AddSymbols()
{
    AddSymbol(method_name_, text_section_.GetDataSize(), text_section_,
              [this]() { return text_section_.GetAddress(); });
    if constexpr (!is_jit_mode) {  // NOLINT
        AddSymbol("code_end", text_section_.GetDataSize(), text_section_,
                  [this]() { return text_section_.GetAddress() + text_section_.GetDataSize(); });
        AddSymbol("aot", aot_section_.GetDataSize(), aot_section_, [this]() { return aot_section_.GetAddress(); });
        AddSymbol("aot_end", aot_section_.GetDataSize(), aot_section_,
                  [this]() { return aot_section_.GetAddress() + aot_section_.GetDataSize(); });
    }
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::SettleSectionsForAot()
{
    static_assert(!is_jit_mode);

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    auto phdr_segment = new Segment(sizeof(ElfEhdr), sizeof(ElfEhdr), PT_PHDR, PF_R, sizeof(ElfOff));
    segments_.push_back(phdr_segment);

    {
        SegmentScope segment_scope(*this, PT_LOAD, PF_R, true);  // NOLINT(hicpp-signed-bitwise)
        current_address_ = sizeof(ElfEhdr) + sizeof(ElfPhdr) * MAX_SEGMENTS_COUNT;
        current_offset_ = sizeof(ElfEhdr) + sizeof(ElfPhdr) * MAX_SEGMENTS_COUNT;
        SettleSection(&dynstr_section_);
        SettleSection(&dynsym_section_);
        SettleSection(&hash_section_);
        SettleSection(&aot_section_);
    }

    {
        SegmentScope segment_scope(*this, PT_LOAD, PF_R | PF_W);  // NOLINT(hicpp-signed-bitwise)
        SettleSection(&got_section_);
    }

    {
        SegmentScope segment_scope(*this, PT_LOAD, PF_R | PF_X);  // NOLINT(hicpp-signed-bitwise)
        SettleSection(&text_section_);
    }

#ifdef PANDA_COMPILER_CFI
    if (!frame_data_->empty()) {
        SegmentScope segment_scope(*this, PT_LOAD, PF_R);  // NOLINT(hicpp-signed-bitwise)
        FillFrameSection();
        SettleSection(&frame_section_);
    }
#endif

    {
        SegmentScope segment_scope(*this, PT_DYNAMIC, PF_R | PF_W);  // NOLINT(hicpp-signed-bitwise)
        SettleSection(&dynamic_section_);
    }

    SettleSection(&shstrtab_section_);

    auto lod_dynamic_segment = new Segment(*segments_.back());
    ASSERT(lod_dynamic_segment->header.p_type == PT_DYNAMIC);
    lod_dynamic_segment->header.p_type = PT_LOAD;
    segments_.insert(segments_.end() - 1, lod_dynamic_segment);

    ASSERT(segments_.size() <= MAX_SEGMENTS_COUNT);
    phdr_segment->header.p_filesz = phdr_segment->header.p_memsz = segments_.size() * sizeof(ElfPhdr);
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::SettleSectionsForJit()
{
    static_assert(is_jit_mode);

    current_address_ = sizeof(ElfEhdr);
    current_offset_ = sizeof(ElfEhdr);

    SettleSection(&text_section_);
#ifdef PANDA_COMPILER_CFI
    if (!frame_data_->empty()) {
        FillFrameSection();
        SettleSection(&frame_section_);
    }
#endif
    SettleSection(&dynsym_section_);
    SettleSection(&hash_section_);
    SettleSection(&dynstr_section_);
    SettleSection(&shstrtab_section_);
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::SettleSection(Section *section)
{
    bool is_section_alloc {(section->header_.sh_flags & SHF_ALLOC) != 0};  // NOLINT(hicpp-signed-bitwise)
    if (is_jit_mode || !is_section_alloc) {
        ASSERT(current_segment_ == nullptr);
    } else {
        ASSERT(current_segment_ != nullptr && current_segment_->header.p_type != PT_NULL);
    }

    section->header_.sh_size = section->GetDataSize();
    if (is_section_alloc) {
        section->header_.sh_addr = UpdateAddress(section->header_.sh_addralign);
        if (section->header_.sh_type != SHT_NOBITS) {
            ASSERT(section->GetDataSize() != 0 || section->header_.sh_type == SHT_PROGBITS);
            section->header_.sh_offset = UpdateOffset(section->header_.sh_addralign);
            current_offset_ += section->header_.sh_size;
        } else {
            section->header_.sh_offset = 0;
        }
        current_address_ += section->header_.sh_size;
    } else {
        section->header_.sh_offset = RoundUp(current_offset_, section->header_.sh_addralign);
        current_offset_ += section->header_.sh_size;
    }
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::Write(const std::string &file_name)
{
    std::vector<uint8_t> data(GetFileSize());
    auto data_span {Span(data)};
    Write(data_span);

    std::ofstream elf_file(file_name, std::ios::binary);
    elf_file.write(reinterpret_cast<char *>(data_span.Data()), data_span.Size());
}

static inline void CopyToSpan(Span<uint8_t> to, const char *from, size_t size, size_t begin_index)
{
    ASSERT(begin_index < to.Size());
    auto max_size {to.Size() - begin_index};
    errno_t res = memcpy_s(&to[begin_index], max_size, from, size);
    if (res != 0) {
        UNREACHABLE();
    }
}

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::Write(Span<uint8_t> stream)
{
    ASSERT(!stream.Empty());
    char *header = reinterpret_cast<char *>(&header_);
    CopyToSpan(stream, header, sizeof(header_), 0);
    for (auto section : sections_) {
        if (auto data_provider = section->GetDataProvider(); data_provider != nullptr) {
            auto i = section->header_.sh_offset;
            data_provider->FillData(stream, i);
        } else if (section->GetDataSize() && section->header_.sh_type != SHT_NOBITS) {
            auto i = section->header_.sh_offset;
            const char *data = reinterpret_cast<const char *>(section->GetData());
            CopyToSpan(stream, data, section->GetDataSize(), i);
        }
    }

    auto i = header_.e_shoff;
    for (auto section : sections_) {
        const char *data = reinterpret_cast<const char *>(&section->header_);
        CopyToSpan(stream, data, sizeof(section->header_), i);
        i += sizeof(section->header_);
    }

    i = header_.e_phoff;
    for (auto segment : segments_) {
        const char *data = reinterpret_cast<const char *>(&segment->header);
        CopyToSpan(stream, data, sizeof(segment->header), i);
        i += sizeof(segment->header);
    }
}

#ifdef PANDA_COMPILER_CFI
template <Arch arch>
class CfiGenerator {
public:
    explicit CfiGenerator(Span<DwarfSectionData> frame_data) : frame_data_(frame_data) {}

    void GenerateDebugInfo()
    {
        if (!debug_info_.empty() || frame_data_.empty()) {
            return;
        }

        auto dw {Initialize()};
        auto cie {CreateCie(dw)};

        for (const auto &data : frame_data_) {
            AddFde(dw, cie, data);
        }

        Finalize(dw);
    }

    Span<const uint8_t> GetDebugInfo() const
    {
        return Span(debug_info_);
    }

private:
    enum DwarfSection {
        REL_DEBUG_FRAME = 0,
        DEBUG_FRAME = 1,
    };

    static inline int CreateSectionCallback(char *name, [[maybe_unused]] int size, [[maybe_unused]] Dwarf_Unsigned type,
                                            [[maybe_unused]] Dwarf_Unsigned flags, [[maybe_unused]] Dwarf_Unsigned link,
                                            [[maybe_unused]] Dwarf_Unsigned info,
                                            [[maybe_unused]] Dwarf_Unsigned *sect_name_index,
                                            [[maybe_unused]] void *user_data, [[maybe_unused]] int *error)
    {
        if (strcmp(name, ".rel.debug_frame") == 0) {
            return DwarfSection::REL_DEBUG_FRAME;
        }
        if (strcmp(name, ".debug_frame") == 0) {
            return DwarfSection::DEBUG_FRAME;
        }
        UNREACHABLE();
    }

    Dwarf_P_Debug Initialize() const
    {
        Dwarf_P_Debug dw {nullptr};
        Dwarf_Error error {nullptr};
        [[maybe_unused]] auto ret =
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            dwarf_producer_init(DW_DLC_WRITE | DW_DLC_SIZE_64 | DW_DLC_SYMBOLIC_RELOCATIONS,
                                reinterpret_cast<Dwarf_Callback_Func>(CreateSectionCallback), nullptr, nullptr, nullptr,
                                GetIsaName(arch), "V2", nullptr, &dw, &error);

        ASSERT(error == DW_DLV_OK);
        return dw;
    }

    Span<Dwarf_Small> GetCieInitInstructions() const
    {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static Dwarf_Small cie_init_instructions_arm[] = {
            DW_CFA_def_cfa,
            GetDwarfSP(arch),
            0U,
        };

        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static Dwarf_Small cie_init_instructions_amd64[] = {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            DW_CFA_def_cfa, GetDwarfSP(arch), PointerSize(arch), DW_CFA_offset | GetDwarfRIP(arch), 1U,
        };

        if (arch == Arch::AARCH32 || arch == Arch::AARCH64) {
            return Span(cie_init_instructions_arm, std::size(cie_init_instructions_arm));
        }
        if (arch == Arch::X86_64) {
            return Span(cie_init_instructions_amd64, std::size(cie_init_instructions_amd64));
        }
        UNREACHABLE();
    }

    Dwarf_Unsigned CreateCie(Dwarf_P_Debug dw) const
    {
        Dwarf_Error error {nullptr};
        auto cie_init_instructions {GetCieInitInstructions()};
        Dwarf_Unsigned cie =
            dwarf_add_frame_cie(dw, const_cast<char *>(""), static_cast<Dwarf_Small>(GetInstructionAlignment(arch)),
                                static_cast<Dwarf_Small>(-PointerSize(arch)), GetDwarfRIP(arch),
                                cie_init_instructions.data(), cie_init_instructions.SizeBytes(), &error);
        ASSERT(error == DW_DLV_OK);
        return cie;
    }

    void AddFde(Dwarf_P_Debug dw, Dwarf_Unsigned cie, const DwarfSectionData &data) const
    {
        Dwarf_Error error {nullptr};
        Dwarf_P_Fde fde = dwarf_new_fde(dw, &error);
        ASSERT(error == DW_DLV_OK);
        for (const auto &inst : data.GetFde()) {
            auto [op, par1, par2] = inst;
            dwarf_add_fde_inst(fde, op, par1, par2, &error);
            ASSERT(error == DW_DLV_OK);
        }

        dwarf_add_frame_fde(dw, fde, nullptr, cie, data.GetOffset(), data.GetSize(), DwarfSection::DEBUG_FRAME, &error);
        ASSERT(error == DW_DLV_OK);
    }

    void Finalize(Dwarf_P_Debug dw)
    {
        Dwarf_Error error {nullptr};
        auto sections = dwarf_transform_to_disk_form(dw, &error);
        ASSERT(error == DW_DLV_OK);

        ASSERT(debug_info_.empty());
        for (decltype(sections) i {0}; i < sections; ++i) {
            Dwarf_Unsigned len = 0;
            Dwarf_Signed elfIdx = 0;
            auto bytes = reinterpret_cast<const uint8_t *>(
                dwarf_get_section_bytes(dw, DwarfSection::DEBUG_FRAME, &elfIdx, &len, &error));
            ASSERT(error == DW_DLV_OK);

            std::copy_n(bytes, len, std::back_inserter(debug_info_));
        }

        constexpr size_t TERMINATOR_SIZE {4U};
        constexpr uint8_t TERMINATOR_VALUE {0U};
        std::fill_n(std::back_inserter(debug_info_), TERMINATOR_SIZE, TERMINATOR_VALUE);

        dwarf_producer_finish(dw, &error);
        ASSERT(error == DW_DLV_OK);

        constexpr size_t CIE_ID_OFFSET {4U};
        // zero out CIE ID field
        *reinterpret_cast<uint32_t *>(&debug_info_[CIE_ID_OFFSET]) = 0;

        constexpr size_t FDE_CIE_DISTANCE_OFFSET {4U};
        constexpr size_t FDE_LENGTH_SIZE {4U};
        size_t base {0};
        for (size_t i {0}; i < frame_data_.size(); ++i) {
            // read FDE length field + 4 bytes (size of length field), get next FDE
            size_t fde_offset {base + *reinterpret_cast<uint32_t *>(&debug_info_[base]) + FDE_LENGTH_SIZE};
            // set distance to the parent CIE in FDE
            ASSERT(debug_info_.size() > fde_offset + FDE_CIE_DISTANCE_OFFSET + 3U);
            *reinterpret_cast<uint32_t *>(&debug_info_[fde_offset + FDE_CIE_DISTANCE_OFFSET]) =
                fde_offset + FDE_CIE_DISTANCE_OFFSET;
            base = fde_offset;
            ASSERT(debug_info_.size() > base);
        }
    }

    Span<DwarfSectionData> frame_data_;
    std::vector<uint8_t> debug_info_;
};

template <Arch arch, bool is_jit_mode>
void ElfBuilder<arch, is_jit_mode>::FillFrameSection()
{
    // compute code offset for method
    for (auto &data : *frame_data_) {
        data.SetOffset(text_section_.header_.sh_offset + data.GetOffset() + CodeInfo::GetCodeOffset(arch));
    }

    CfiGenerator<arch> cfi_gen {Span(*frame_data_)};
    cfi_gen.GenerateDebugInfo();
    auto debug_info {cfi_gen.GetDebugInfo()};

    // Generate frame data
    GetFrameSection()->AppendData(debug_info.data(), debug_info.size());
}
#endif  // #ifdef PANDA_COMPILER_CFI

class ElfWriter {
public:
    void SetArch(Arch arch)
    {
        arch_ = arch;
    }
    Arch GetArch() const
    {
        return arch_;
    }

    void SetRuntime(RuntimeInterface *runtime)
    {
        runtime_ = runtime;
    }

    RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

    uintptr_t GetCurrentCodeAddress() const
    {
        return current_code_size_;
    }

    void StartClass(const Class &klass)
    {
        ClassHeader *class_header = &class_headers_.emplace_back();
        current_bitmap_ = &class_methods_bitmaps_.emplace_back();
        class_header->class_id = klass.GetFileId().GetOffset();
        class_header->methods_offset = method_headers_.size();
        current_bitmap_->resize(klass.GetMethods().size());
    }

    void EndClass()
    {
        ASSERT(!class_headers_.empty());
        auto &class_header = class_headers_.back();
        class_header.methods_count = method_headers_.size() - class_header.methods_offset;
        if (class_header.methods_count != 0) {
            ASSERT(IsAligned<sizeof(uint32_t)>(current_bitmap_->GetContainerSizeInBytes()));
            class_header.methods_bitmap_offset = bitmap_size_;
            class_header.methods_bitmap_size = current_bitmap_->size();
            bitmap_size_ += current_bitmap_->GetContainerSize();
        } else {
            CHECK_EQ(class_methods_bitmaps_.size(), class_headers_.size());
            class_headers_.pop_back();
            class_methods_bitmaps_.pop_back();
        }
    }

    void AddMethod(const CompiledMethod &method, size_t method_index)
    {
        if (method.GetMethod() == nullptr || method.GetCode().Empty()) {
            return;
        }
        methods_.push_back(method);
        auto &method_header = method_headers_.emplace_back();
        method_header.method_id = method.GetMethod()->GetFileId().GetOffset();
        method_header.code_offset = current_code_size_;
        method_header.code_size = method.GetOverallSize();
        current_code_size_ += method_header.code_size;
        current_code_size_ = RoundUp(current_code_size_, GetCodeAlignment(arch_));
        current_bitmap_->SetBit(method_index);

#ifdef PANDA_COMPILER_CFI
        if (GetEmitDebugInfo()) {
            FillDebugInfo(method.GetCfiInfo(), method_header);
        }
#endif
    }

    void SetClassContext(const std::string &ctx)
    {
        class_ctx_ = ctx;
    }

#ifdef PANDA_COMPILER_CFI
    void SetEmitDebugInfo([[maybe_unused]] bool emit_debug_info)
    {
        emit_debug_info_ = emit_debug_info;
    }

    bool GetEmitDebugInfo() const
    {
        return emit_debug_info_;
    }
#endif

    size_t AddString(const std::string &str)
    {
        auto pos = string_table_.size();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        string_table_.insert(string_table_.end(), str.data(), str.data() + str.size() + 1);
        return pos;
    }

    template <Arch arch, bool is_jit_mode>
    void GenerateSymbols(ElfBuilder<arch, is_jit_mode> &builder);

#ifdef PANDA_COMPILER_CFI
    inline void PrepareOffsetsForDwarf(CfiOffsets *offsets) const;

    inline void FillPrologueInfo(DwarfSectionData *sect_data, const CfiOffsets &cfi_offsets) const;

    inline void FillCalleesInfo(DwarfSectionData *sect_data, const CfiInfo &cfi_info) const;

    inline void FillEpilogInfo(DwarfSectionData *sect_data, const CfiOffsets &cfi_offsets) const;

    inline void FillDebugInfo(CfiInfo cfi_info, const compiler::MethodHeader &method_header);

    void AddFrameData(const DwarfSectionData &data)
    {
        frame_data_.push_back(data);
    }

    std::vector<DwarfSectionData> *GetFrameData()
    {
        return &frame_data_;
    }

#endif
private:
    std::string class_ctx_;
    Arch arch_ {Arch::NONE};
    size_t current_code_size_ {0};

    std::vector<compiler::PandaFileHeader> file_headers_;
    std::vector<compiler::ClassHeader> class_headers_;
    std::vector<compiler::MethodHeader> method_headers_;
    std::vector<CompiledMethod> methods_;

    std::vector<char> string_table_;
    uint32_t gc_type_ {static_cast<uint32_t>(mem::GCType::INVALID_GC)};

    std::vector<BitVector<>> class_methods_bitmaps_;
    static_assert(sizeof(BitVector<>::container_value_type) == sizeof(uint32_t));
    BitVector<> *current_bitmap_ {nullptr};
    uint32_t bitmap_size_ {0};

    RuntimeInterface *runtime_ {nullptr};
#ifdef PANDA_COMPILER_CFI
    std::vector<DwarfSectionData> frame_data_;
    bool emit_debug_info_ {false};
#endif

    friend class CodeDataProvider;
    friend class JitCodeDataProvider;
    friend class AotBuilder;
    friend class JitDebugWriter;
};

#ifdef PANDA_COMPILER_CFI
static constexpr size_t LR_CFA_OFFSET {1U};
static constexpr size_t FP_CFA_OFFSET {2U};
static constexpr size_t DWARF_ARM_FP_REGS_START {64U};

void ElfWriter::PrepareOffsetsForDwarf(CfiOffsets *offsets) const
{
    // Make relative offsets
    offsets->pop_fplr -= offsets->pop_callees;
    offsets->pop_callees -= offsets->push_callees;
    offsets->push_callees -= offsets->set_fp;
    offsets->set_fp -= offsets->push_fplr;

    // Make offsets in alignment units
    auto inst_alignment {GetInstructionAlignment(arch_)};

    offsets->push_fplr /= inst_alignment;
    offsets->set_fp /= inst_alignment;
    offsets->push_callees /= inst_alignment;
    offsets->pop_callees /= inst_alignment;
    offsets->pop_fplr /= inst_alignment;
}

void ElfWriter::FillPrologueInfo(DwarfSectionData *sect_data, const CfiOffsets &cfi_offsets) const
{
    sect_data->AddFdeInst(DW_CFA_advance_loc, cfi_offsets.push_fplr, 0);
    sect_data->AddFdeInst(DW_CFA_def_cfa_offset, FP_CFA_OFFSET * PointerSize(arch_), 0);
    if (arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64) {
        sect_data->AddFdeInst(DW_CFA_offset, GetDwarfLR(arch_), LR_CFA_OFFSET);
    }
    sect_data->AddFdeInst(DW_CFA_offset, GetDwarfFP(arch_), FP_CFA_OFFSET);

    sect_data->AddFdeInst(DW_CFA_advance_loc, cfi_offsets.set_fp, 0);
    sect_data->AddFdeInst(DW_CFA_def_cfa_register, GetDwarfFP(arch_), 0);
}

void ElfWriter::FillCalleesInfo(DwarfSectionData *sect_data, const CfiInfo &cfi_info) const
{
    const auto &cfi_offsets {cfi_info.offsets};

    sect_data->AddFdeInst(DW_CFA_advance_loc, cfi_offsets.push_callees, 0);

    const auto &callees {cfi_info.callee_regs};
    size_t callee_slot {0};
    for (size_t i {0}; i < callees.size(); ++i) {
        auto reg {(callees.size() - 1U) - i};
        if (callees.test(reg)) {
#ifdef PANDA_COMPILER_TARGET_X86_64
            if (arch_ == Arch::X86_64) {
                reg = amd64::ConvertRegNumber(reg);
            }
#endif
            sect_data->AddFdeInst(DW_CFA_offset, reg,
                                  FP_CFA_OFFSET + CFrameLayout::CALLEE_REGS_START_SLOT + callee_slot++);
        }
    }

    const auto &vcallees {cfi_info.callee_vregs};
    for (size_t i {0}; i < vcallees.size(); ++i) {
        auto vreg {(vcallees.size() - 1) - i};
        if (vcallees.test(vreg)) {
            ASSERT(arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64);
            sect_data->AddFdeInst(DW_CFA_offset, DWARF_ARM_FP_REGS_START + vreg,
                                  FP_CFA_OFFSET + CFrameLayout::CALLEE_REGS_START_SLOT + callee_slot++);
        }
    }

    sect_data->AddFdeInst(DW_CFA_advance_loc, cfi_offsets.pop_callees, 0);
    for (size_t i {0}; i < callees.size(); ++i) {
        auto reg {(callees.size() - 1U) - i};
        if (callees.test(reg)) {
#ifdef PANDA_COMPILER_TARGET_X86_64
            if (arch_ == Arch::X86_64) {
                reg = amd64::ConvertRegNumber(reg);
            }
#endif
            sect_data->AddFdeInst(DW_CFA_same_value, reg, 0);
        }
    }

    for (size_t i {0}; i < vcallees.size(); ++i) {
        auto vreg {(vcallees.size() - 1) - i};
        if (vcallees.test(vreg)) {
            ASSERT(arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64);
            sect_data->AddFdeInst(DW_CFA_same_value, DWARF_ARM_FP_REGS_START + vreg, 0);
        }
    }
}

void ElfWriter::FillEpilogInfo(DwarfSectionData *sect_data, const CfiOffsets &cfi_offsets) const
{
    sect_data->AddFdeInst(DW_CFA_advance_loc, cfi_offsets.pop_fplr, 0);
    sect_data->AddFdeInst(DW_CFA_same_value, GetDwarfFP(arch_), 0);
    if (arch_ == Arch::AARCH32 || arch_ == Arch::AARCH64) {
        sect_data->AddFdeInst(DW_CFA_same_value, GetDwarfLR(arch_), 0);
        sect_data->AddFdeInst(DW_CFA_def_cfa, GetDwarfSP(arch_), 0);
    } else if (arch_ == Arch::X86_64) {
        sect_data->AddFdeInst(DW_CFA_def_cfa, GetDwarfSP(arch_), 1U * PointerSize(arch_));
    } else {
        UNREACHABLE();
    }
}

void ElfWriter::FillDebugInfo(CfiInfo cfi_info, const compiler::MethodHeader &method_header)
{
    DwarfSectionData sect_data;
    // Will be patched later
    sect_data.SetOffset(method_header.code_offset);
    sect_data.SetSize(method_header.code_size);

    auto &cfi_offsets {cfi_info.offsets};
    PrepareOffsetsForDwarf(&cfi_offsets);

    FillPrologueInfo(&sect_data, cfi_offsets);
    FillCalleesInfo(&sect_data, cfi_info);
    FillEpilogInfo(&sect_data, cfi_offsets);

    AddFrameData(sect_data);
}
#endif

}  // namespace panda::compiler

#endif  // COMPILER_AOT_AOT_BULDER_ELF_BUILDER_H
