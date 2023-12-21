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

#if USE_VIXL_ARM64 && !PANDA_MINIMAL_VIXL
#include "aarch64/disasm-aarch64.h"
#endif
#include "compilation.h"
#include "function.h"
#include "mem/pool_manager.h"
#include "elfio/elfio.hpp"
#include "irtoc_runtime.h"

#ifdef PANDA_COMPILER_DEBUG_INFO
#include "dwarf_builder.h"
#endif

namespace panda::irtoc {

#if USE_VIXL_ARM64 && !PANDA_MINIMAL_VIXL && PANDA_LLVMAOT
class UsedRegistersCollector : public vixl::aarch64::Disassembler {
public:
    explicit UsedRegistersCollector(panda::ArenaAllocator *allocator) : Disassembler(allocator) {}

    RegMask &GetUsedRegs(bool is_fp)
    {
        return is_fp ? vreg_mask_ : reg_mask_;
    }

    static UsedRegisters CollectForCode(ArenaAllocator *allocator, Span<const uint8_t> code)
    {
        ASSERT(allocator != nullptr);
        ASSERT(!code.Empty());

        vixl::aarch64::Decoder decoder(allocator);
        UsedRegistersCollector used_regs_collector(allocator);
        decoder.AppendVisitor(&used_regs_collector);
        bool skipping = false;

        auto start_instr = reinterpret_cast<const vixl::aarch64::Instruction *>(code.data());
        auto end_instr = reinterpret_cast<const vixl::aarch64::Instruction *>(&(*code.end()));
        // To determine real registers usage we check each assembly instruction which has
        // destination register(s). There is a problem with handlers with `return` cause
        // there is an epilogue part with registers restoring. We need to separate both register
        // restoring and real register usage. There are some heuristics were invented to make it
        // possible, it work as follows:
        //   1) We parse assembly code in reverse mode to fast the epilogue part finding.
        //   2) For each instruction we add all its destination registers to the result set
        //      of used registers.
        //   3) When we met `ret` instruction then we raise special `skipping` flag for a few
        //      next instructions.
        //   4) When `skipping` flag is raised, while we meet `load` instructions or `add`
        //      arithmetic involving `sp` (stack pointer) as destination we continue to skip such
        //      instructions (assuming they are related to epilogue part) but without adding their
        //      registers into the result set. If we meet another kind of intruction we unset
        //      the `skipping` flag.
        for (auto instr = used_regs_collector.GetPrevInstruction(end_instr); instr >= start_instr;
             instr = used_regs_collector.GetPrevInstruction(instr)) {
            if (instr->Mask(vixl::aarch64::UnconditionalBranchToRegisterMask) == vixl::aarch64::RET) {
                skipping = true;
                continue;
            }
            if (skipping && (instr->IsLoad() || used_regs_collector.CheckSPAdd(instr))) {
                continue;
            }
            skipping = false;
            decoder.Decode(instr);
        }

        UsedRegisters used_registers;
        used_registers.gpr |= used_regs_collector.GetUsedRegs(false);
        used_registers.fp |= used_regs_collector.GetUsedRegs(true);
        return used_registers;
    }

protected:
    const vixl::aarch64::Instruction *GetPrevInstruction(const vixl::aarch64::Instruction *instr) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return instr - vixl::aarch64::kInstructionSize;
    }

    bool CheckSPAdd(const vixl::aarch64::Instruction *instr) const
    {
        constexpr int32_t SP_REG = GetDwarfSP(Arch::AARCH64);
        return instr->Mask(vixl::aarch64::AddSubOpMask) == vixl::aarch64::ADD && (instr->GetRd() == SP_REG);
    }

    void AppendRegisterNameToOutput(const vixl::aarch64::Instruction *instr,
                                    const vixl::aarch64::CPURegister &reg) override
    {
        Disassembler::AppendRegisterNameToOutput(instr, reg);
        if (instr->IsStore()) {
            return;
        }
        uint32_t code = reg.GetCode();
        // We need to account for both registers in case of a pair load
        bool is_pair = instr->Mask(vixl::aarch64::LoadStorePairAnyFMask) == vixl::aarch64::LoadStorePairAnyFixed;
        if (!(code == static_cast<uint32_t>(instr->GetRd()) ||
              (is_pair && code == static_cast<uint32_t>(instr->GetRt2())))) {
            return;
        }
        if (reg.IsRegister()) {
            if (!reg.IsZero()) {
                reg_mask_.Set(code);
            }
        } else {
            ASSERT(reg.IsVRegister());
            vreg_mask_.Set(code);
        }
    }

private:
    RegMask reg_mask_;
    VRegMask vreg_mask_;
};
#endif  // USE_VIXL_ARM64 && !PANDA_MINIMAL_VIXL && PANDA_LLVMAOT

// elfio library missed some elf constants, so lets define it here for a while. We can't include elf.h header because
// it conflicts with elfio.
static constexpr size_t EF_ARM_EABI_VER5 = 0x05000000;

void Compilation::CheckUsedRegisters()
{
#if USE_VIXL_ARM64 && !PANDA_MINIMAL_VIXL && PANDA_LLVMAOT
    if (used_registers_.gpr.Count() > 0) {
        LOG(INFO, IRTOC) << "LLVM Irtoc compilation: used registers " << used_registers_.gpr;
        used_registers_.gpr &= GetCalleeRegsMask(arch_, false);
        auto diff = used_registers_.gpr ^ GetCalleeRegsMask(arch_, false, true);
        if (diff.Any()) {
            LOG(FATAL, IRTOC) << "LLVM Irtoc compilation callee saved register usage is different from optimized set"
                              << std::endl
                              << "Expected: " << GetCalleeRegsMask(arch_, false, true) << std::endl
                              << "Got: " << used_registers_.gpr;
        }
    }
    if (used_registers_.fp.Count() > 0) {
        LOG(INFO, IRTOC) << "LLVM Irtoc compilation: used fp registers " << used_registers_.fp;
        used_registers_.fp &= GetCalleeRegsMask(arch_, true);
        auto diff = used_registers_.fp ^ GetCalleeRegsMask(arch_, true, true);
        if (diff.Any()) {
            LOG(FATAL, IRTOC) << "LLVM Irtoc compilation callee saved fp register usage is different from optimized set"
                              << std::endl
                              << "Expected: " << GetCalleeRegsMask(arch_, true, true) << std::endl
                              << "Got: " << used_registers_.fp;
        }
    }
#endif
}

Compilation::Result Compilation::Run()
{
    if (compiler::OPTIONS.WasSetCompilerRegex()) {
        methods_regex_ = compiler::OPTIONS.GetCompilerRegex();
    }

    PoolManager::Initialize(PoolType::MALLOC);

    allocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);
    local_allocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);

    if (RUNTIME_ARCH == Arch::X86_64 && compiler::OPTIONS.WasSetCompilerCrossArch()) {
        arch_ = GetArchFromString(compiler::OPTIONS.GetCompilerCrossArch());
        if (arch_ == Arch::NONE) {
            LOG(FATAL, IRTOC) << "FATAL: unknown arch: " << compiler::OPTIONS.GetCompilerCrossArch();
        }
        compiler::OPTIONS.AdjustCpuFeatures(arch_ != RUNTIME_ARCH);
    } else {
        compiler::OPTIONS.AdjustCpuFeatures(false);
    }

    LOG(INFO, IRTOC) << "Start Irtoc compilation for " << GetArchString(arch_) << "...";

    auto result = Compile();
    if (result) {
        CheckUsedRegisters();
        LOG(INFO, IRTOC) << "Irtoc compilation success";
    } else {
        LOG(FATAL, IRTOC) << "Irtoc compilation failed: " << result.Error();
    }

    if (result = MakeElf(OPTIONS.GetIrtocOutput()); !result) {
        return result;
    }

    for (auto unit : units_) {
        delete unit;
    }

    allocator_.reset();
    local_allocator_.reset();

    PoolManager::Finalize();

    return result;
}

Compilation::Result Compilation::Compile()
{
#ifdef PANDA_LLVMAOT
    IrtocRuntimeInterface runtime;
    ArenaAllocator allocator(SpaceType::SPACE_TYPE_COMPILER);
    std::shared_ptr<compiler::IrtocCompilerInterface> llvm_compiler =
        llvmaot::CreateLLVMIrtocCompiler(&runtime, &allocator, arch_);
#endif

    for (auto unit : units_) {
        if (compiler::OPTIONS.WasSetCompilerRegex() && !std::regex_match(unit->GetName(), methods_regex_)) {
            continue;
        }
        LOG(INFO, IRTOC) << "Compile " << unit->GetName();
#ifdef PANDA_LLVMAOT
        unit->SetLLVMCompiler(llvm_compiler);
#endif
        auto result = unit->Compile(arch_, allocator_.get(), local_allocator_.get());
        if (!result) {
            return result;
        }
#ifdef PANDA_COMPILER_DEBUG_INFO
        has_debug_info_ |= unit->GetGraph()->IsLineDebugInfoEnabled();
#endif
    }

#ifdef PANDA_LLVMAOT
    llvm_compiler->CompileAll();
    ASSERT(!OPTIONS.GetIrtocOutputLlvm().empty());
    llvm_compiler->WriteObjectFile(OPTIONS.GetIrtocOutputLlvm());

    for (auto unit : units_) {
        if (unit->IsCompiledByLlvm()) {
            auto code = llvm_compiler->GetCompiledCode(unit->GetName());
            Span<uint8_t> span = {const_cast<uint8_t *>(code.code), code.size};
            unit->SetCode(span);
        }
        unit->ReportCompilationStatistic(&std::cerr);
    }
    if (OPTIONS.GetIrtocLlvmStats() != "none" && !llvm_compiler->IsEmpty()) {
        std::cerr << "LLVM total: " << llvm_compiler->GetObjectFileSize() << " bytes" << std::endl;
    }

#if USE_VIXL_ARM64 && !PANDA_MINIMAL_VIXL
    for (auto unit : units_) {
        if (!(unit->GetGraphMode().IsInterpreter() || unit->GetGraphMode().IsInterpreterEntry()) ||
            arch_ != Arch::AARCH64 || unit->GetLLVMCompilationResult() == LLVMCompilationResult::USE_ARK_AS_NO_SUFFIX) {
            continue;
        }
        used_registers_ |= UsedRegistersCollector::CollectForCode(&allocator, unit->GetCode());
    }
#endif
#endif  // PANDA_LLVMAOT

    return 0;
}

static size_t GetElfArch(Arch arch)
{
    switch (arch) {
        case Arch::AARCH32:
            return ELFIO::EM_ARM;
        case Arch::AARCH64:
            return ELFIO::EM_AARCH64;
        case Arch::X86:
            return ELFIO::EM_386;
        case Arch::X86_64:
            return ELFIO::EM_X86_64;
        default:
            UNREACHABLE();
    }
}

// CODECHECK-NOLINTNEXTLINE(C_RULE_ID_FUNCTION_SIZE)
Compilation::Result Compilation::MakeElf(std::string_view output)
{
    ELFIO::elfio elf_writer;
    elf_writer.create(Is64BitsArch(arch_) ? ELFIO::ELFCLASS64 : ELFIO::ELFCLASS32, ELFIO::ELFDATA2LSB);
    elf_writer.set_type(ELFIO::ET_REL);
    if (arch_ == Arch::AARCH32) {
        elf_writer.set_flags(EF_ARM_EABI_VER5);
    }
    elf_writer.set_os_abi(ELFIO::ELFOSABI_NONE);
    elf_writer.set_machine(GetElfArch(arch_));

    ELFIO::section *str_sec = elf_writer.sections.add(".strtab");
    str_sec->set_type(ELFIO::SHT_STRTAB);
    str_sec->set_addr_align(0x1);

    ELFIO::string_section_accessor str_writer(str_sec);

    static constexpr size_t FIRST_GLOBAL_SYMBOL_INDEX = 2;
    static constexpr size_t SYMTAB_ADDR_ALIGN = 8;

    ELFIO::section *sym_sec = elf_writer.sections.add(".symtab");
    sym_sec->set_type(ELFIO::SHT_SYMTAB);
    sym_sec->set_info(FIRST_GLOBAL_SYMBOL_INDEX);
    sym_sec->set_link(str_sec->get_index());
    sym_sec->set_addr_align(SYMTAB_ADDR_ALIGN);
    sym_sec->set_entry_size(elf_writer.get_default_entry_size(ELFIO::SHT_SYMTAB));

    ELFIO::symbol_section_accessor symbol_writer(elf_writer, sym_sec);

    symbol_writer.add_symbol(str_writer, "irtoc.cpp", 0, 0, ELFIO::STB_LOCAL, ELFIO::STT_FILE, 0, ELFIO::SHN_ABS);

    ELFIO::section *text_sec = elf_writer.sections.add(".text");
    text_sec->set_type(ELFIO::SHT_PROGBITS);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    text_sec->set_flags(ELFIO::SHF_ALLOC | ELFIO::SHF_EXECINSTR);
    text_sec->set_addr_align(GetCodeAlignment(arch_));

    ELFIO::section *rel_sec = elf_writer.sections.add(".rela.text");
    rel_sec->set_type(ELFIO::SHT_RELA);
    rel_sec->set_info(text_sec->get_index());
    rel_sec->set_link(sym_sec->get_index());
    rel_sec->set_addr_align(4U);  // CODECHECK-NOLINT(C_RULE_ID_MAGICNUMBER)
    rel_sec->set_entry_size(elf_writer.get_default_entry_size(ELFIO::SHT_RELA));
    ELFIO::relocation_section_accessor rel_writer(elf_writer, rel_sec);

    /* Use symbols map to avoid saving the same symbols multiple times */
    std::unordered_map<std::string, uint32_t> symbols_map;
    auto add_symbol = [&symbols_map, &symbol_writer, &str_writer](const char *name) {
        if (auto it = symbols_map.find(name); it != symbols_map.end()) {
            return it->second;
        }
        uint32_t index = symbol_writer.add_symbol(str_writer, name, 0, 0, ELFIO::STB_GLOBAL, ELFIO::STT_NOTYPE, 0, 0);
        symbols_map.insert({name, index});
        return index;
    };
#ifdef PANDA_COMPILER_DEBUG_INFO
    auto dwarf_builder {has_debug_info_ ? std::make_optional<DwarfBuilder>(arch_, &elf_writer) : std::nullopt};
#endif

    static constexpr size_t MAX_CODE_ALIGNMENT = 64;
    static constexpr std::array<uint8_t, MAX_CODE_ALIGNMENT> PADDING_DATA {0};
    CHECK_LE(GetCodeAlignment(GetArch()), MAX_CODE_ALIGNMENT);

    uint32_t code_alignment = GetCodeAlignment(GetArch());
    size_t offset = 0;
    for (auto unit : units_) {
        if (unit->IsCompiledByLlvm()) {
            continue;
        }
        auto code = unit->GetCode();

        // Align function
        if (auto padding = offset % code_alignment; padding != 0) {
            text_sec->append_data(reinterpret_cast<const char *>(PADDING_DATA.data()), padding);
            offset += padding;
        }
        auto symbol = symbol_writer.add_symbol(str_writer, unit->GetName(), offset, code.size(), ELFIO::STB_GLOBAL,
                                               ELFIO::STT_FUNC, 0, text_sec->get_index());
        (void)symbol;
        text_sec->append_data(reinterpret_cast<const char *>(code.data()), code.size());
        for (auto &rel : unit->GetRelocations()) {
            size_t rel_offset = offset + rel.offset;
            auto sindex = add_symbol(unit->GetExternalFunction(rel.data));
            if (Is64BitsArch(arch_)) {
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                rel_writer.add_entry(rel_offset, static_cast<ELFIO::Elf_Xword>(ELF64_R_INFO(sindex, rel.type)),
                                     rel.addend);
            } else {
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                rel_writer.add_entry(rel_offset, static_cast<ELFIO::Elf_Xword>(ELF32_R_INFO(sindex, rel.type)),
                                     rel.addend);
            }
        }
#ifdef PANDA_COMPILER_DEBUG_INFO
        ASSERT(!unit->GetGraph()->IsLineDebugInfoEnabled() || dwarf_builder);
        if (dwarf_builder && !dwarf_builder->BuildGraph(unit, offset, symbol)) {
            return Unexpected("DwarfBuilder::BuildGraph failed!");
        }
#endif
        offset += code.size();
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    if (dwarf_builder && !dwarf_builder->Finalize(offset)) {
        return Unexpected("DwarfBuilder::Finalize failed!");
    }
#endif

    elf_writer.save(output.data());

    return 0;
}
}  // namespace panda::irtoc
