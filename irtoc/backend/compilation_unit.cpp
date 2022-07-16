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

#include "compilation_unit.h"
#include "compiler/codegen_boundary.h"
#include "compiler/codegen_fastpath.h"
#include "compiler/codegen_interpreter.h"
#include "compiler/optimizer_run.h"
#include "compiler/optimizer/code_generator/target_info.h"
#include "compiler/optimizer/optimizations/balance_expressions.h"
#include "compiler/optimizer/optimizations/branch_elimination.h"
#include "compiler/optimizer/optimizations/checks_elimination.h"
#include "compiler/optimizer/optimizations/code_sink.h"
#include "compiler/optimizer/optimizations/cse.h"
#include "compiler/optimizer/optimizations/deoptimize_elimination.h"
#include "compiler/optimizer/optimizations/if_conversion.h"
#include "compiler/optimizer/optimizations/licm.h"
#include "compiler/optimizer/optimizations/loop_peeling.h"
#include "compiler/optimizer/optimizations/loop_unroll.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/lse.h"
#include "compiler/optimizer/optimizations/memory_barriers.h"
#include "compiler/optimizer/optimizations/memory_coalescing.h"
#include "compiler/optimizer/optimizations/move_constants.h"
#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/optimizations/redundant_loop_elimination.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc.h"
#include "compiler/optimizer/optimizations/scheduler.h"
#include "compiler/optimizer/optimizations/try_catch_resolving.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "mem/pool_manager.h"
#include "irtoc_runtime.h"
#include "elfio/elfio.hpp"

namespace panda::irtoc {

// elfio library missed some elf constants, so lets define it here for a while. We can't include elf.h header because
// it conflicts with elfio.
static constexpr size_t EF_ARM_EABI_VER5 = 0x05000000;
static constexpr size_t EM_AARCH64 = 183;

static bool RunIrtocOptimizations(Graph *graph);

Compilation::Result Compilation::Run(std::string_view output)
{
    if (compiler::options.WasSetCompilerRegex()) {
        methods_regex_ = compiler::options.GetCompilerRegex();
    }
    // NOLINTNEXTLINE(readability-magic-numbers)
    panda::mem::MemConfig::Initialize(32_MB, 64_MB, 600_MB, 32_MB);
    PoolManager::Initialize();

    if (RUNTIME_ARCH == Arch::X86_64 && compiler::options.WasSetCompilerCrossArch()) {
        arch_ = GetArchFromString(compiler::options.GetCompilerCrossArch());
        if (arch_ == Arch::NONE) {
            LOG(FATAL, IRTOC) << "FATAL: unknown arch: " << compiler::options.GetCompilerCrossArch();
        } else if (!compiler::BackendSupport(arch_)) {
            LOG(FATAL, IRTOC) << "Backend is not supported: " << compiler::options.GetCompilerCrossArch();
        }
    }

    LOG(INFO, IRTOC) << "Start Irtoc compilation for " << GetArchString(arch_) << "...";

    auto result = Compile();

    if (result) {
        LOG(INFO, IRTOC) << "Irtoc compilation success";
    } else {
        LOG(ERROR, IRTOC) << "Irtoc compilation failed: " << result.Error();
    }

    MakeElf(output);

    for (auto unit : units_) {
        delete unit;
    }

    PoolManager::Finalize();
    panda::mem::MemConfig::Finalize();

    return result;
}

Compilation::Result Compilation::Compile()
{
    for (auto unit : units_) {
        if (compiler::options.WasSetCompilerRegex() && !std::regex_match(unit->GetName(), methods_regex_)) {
            continue;
        }
        LOG(INFO, IRTOC) << "Compile " << unit->GetName();
        auto result = unit->Compile(arch_);
        if (!result) {
            return result;
        }
    }
    return 0;
}

static bool RunIrtocInterpreterOptimizations(Graph *graph)
{
    compiler::options.SetCompilerChecksElimination(false);
    // TODO(aantipina): re-enable Lse for #6873
    compiler::options.SetCompilerLse(false);
#ifdef PANDA_COMPILER_TARGET_AARCH64
    compiler::options.SetCompilerMemoryCoalescing(false);
#endif
    if (!compiler::options.IsCompilerNonOptimizing()) {
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::BranchElimination>();
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Cse>();
        graph->RunPass<compiler::Licm>(compiler::options.GetCompilerLicmHoistLimit());
        graph->RunPass<compiler::RedundantLoopElimination>();
        graph->RunPass<compiler::LoopPeeling>();
        graph->RunPass<compiler::Lse>();
        graph->RunPass<compiler::ValNum>();
        if (graph->RunPass<compiler::Peepholes>() && graph->RunPass<compiler::BranchElimination>()) {
            graph->RunPass<compiler::Peepholes>();
        }
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Cse>();
        graph->RunPass<compiler::ChecksElimination>();
        graph->RunPass<compiler::LoopUnroll>(compiler::options.GetCompilerLoopUnrollInstLimit(),
                                             compiler::options.GetCompilerLoopUnrollFactor());
        graph->RunPass<compiler::BalanceExpressions>();
        if (graph->RunPass<compiler::Peepholes>()) {
            graph->RunPass<compiler::BranchElimination>();
        }
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::Cse>();

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif  // NDEBUG
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Lowering>();
        graph->RunPass<compiler::CodeSink>();
        graph->RunPass<compiler::MemoryCoalescing>(compiler::options.IsCompilerMemoryCoalescingAligned());
        graph->RunPass<compiler::IfConversion>(compiler::options.GetCompilerIfConversionLimit());
        graph->RunPass<compiler::MoveConstants>();
    }

    graph->RunPass<compiler::Cleanup>();
    if (!compiler::RegAlloc(graph)) {
        return false;
    }

    graph->RunPass<compiler::Cleanup>();
    return graph->RunPass<compiler::CodegenInterpreter>();
}

CompilationUnit::Result CompilationUnit::Compile(Arch arch)
{
    IrtocRuntimeInterface runtime;

    ArenaAllocator allocator_(SpaceType::SPACE_TYPE_COMPILER);
    ArenaAllocator local_allocator_(SpaceType::SPACE_TYPE_COMPILER);

    graph_ = allocator_.New<Graph>(&allocator_, &local_allocator_, arch, this, &runtime, false);
    builder_ = std::make_unique<compiler::IrConstructor>();

    MakeGraphImpl();

    if (GetGraph()->GetMode().IsNative()) {
        if (!RunOptimizations(GetGraph())) {
            return Unexpected("RunOptimizations failed!");
        }
    } else if (GetGraph()->GetMode().IsInterpreter() || GetGraph()->GetMode().IsInterpreterEntry()) {
        if (!RunIrtocInterpreterOptimizations(GetGraph())) {
            return Unexpected("RunIrtocInterpreterOptimizations failed!");
        }
    } else if (!RunIrtocOptimizations(GetGraph())) {
        return Unexpected("RunOptimizations failed!");
    }

    auto code = GetGraph()->GetData();
    std::copy(code.begin(), code.end(), std::back_inserter(code_));

    return 0;
}

void CompilationUnit::AddRelocation(const compiler::RelocationInfo &info)
{
    relocation_entries_.emplace_back(info);
}

static bool RunIrtocOptimizations(Graph *graph)
{
    if (!compiler::options.IsCompilerNonOptimizing()) {
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::Cse>();
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Lowering>();
        graph->RunPass<compiler::CodeSink>();
        graph->RunPass<compiler::MemoryCoalescing>(compiler::options.IsCompilerMemoryCoalescingAligned());
        graph->RunPass<compiler::IfConversion>(compiler::options.GetCompilerIfConversionLimit());
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Scheduler>();
        // Perform MoveConstants after Scheduler because Scheduler can rearrange constants
        // and cause spillfill in reg alloc
        graph->RunPass<compiler::MoveConstants>();
    }

    graph->RunPass<compiler::Cleanup>();
    if (!compiler::RegAlloc(graph)) {
        LOG(FATAL, IRTOC) << "RunOptimizations failed: register allocation error";
        return false;
    }

    graph->RunPass<compiler::Cleanup>();
    if (graph->GetMode().IsFastPath()) {
        if (!graph->RunPass<compiler::CodegenFastPath>()) {
            LOG(FATAL, IRTOC) << "RunOptimizations failed: code generation error";
            return false;
        }
    } else if (graph->GetMode().IsBoundary()) {
        if (!graph->RunPass<compiler::CodegenBoundary>()) {
            LOG(FATAL, IRTOC) << "RunOptimizations failed: code generation error";
            return false;
        }
    } else {
        UNREACHABLE();
    }

    return true;
}

static size_t GetElfArch(Arch arch)
{
    switch (arch) {
        case Arch::AARCH32:
            return EM_ARM;
        case Arch::AARCH64:
            return EM_AARCH64;
        case Arch::X86:
            return EM_386;
        case Arch::X86_64:
            return EM_X86_64;
        default:
            UNREACHABLE();
    }
}

Compilation::Result Compilation::MakeElf(std::string_view output)
{
    ELFIO::elfio elf_writer;
    elf_writer.create(Is64BitsArch(arch_) ? ELFCLASS64 : ELFCLASS32, ELFDATA2LSB);
    elf_writer.set_type(ET_REL);
    if (arch_ == Arch::AARCH32) {
        elf_writer.set_flags(EF_ARM_EABI_VER5);
    }
    elf_writer.set_os_abi(ELFOSABI_NONE);
    elf_writer.set_machine(GetElfArch(arch_));

    ELFIO::section *str_sec = elf_writer.sections.add(".strtab");
    str_sec->set_type(SHT_STRTAB);
    str_sec->set_addr_align(0x1);

    ELFIO::string_section_accessor str_writer(str_sec);

    static constexpr size_t FIRST_GLOBAL_SYMBOL_INDEX = 2;
    static constexpr size_t SYMTAB_ADDR_ALIGN = 8;

    ELFIO::section *sym_sec = elf_writer.sections.add(".symtab");
    sym_sec->set_type(SHT_SYMTAB);
    sym_sec->set_info(FIRST_GLOBAL_SYMBOL_INDEX);
    sym_sec->set_link(str_sec->get_index());
    sym_sec->set_addr_align(SYMTAB_ADDR_ALIGN);
    sym_sec->set_entry_size(elf_writer.get_default_entry_size(SHT_SYMTAB));

    ELFIO::symbol_section_accessor symbol_writer(elf_writer, sym_sec);

    symbol_writer.add_symbol(str_writer, "irtoc.cpp", 0, 0, STB_LOCAL, STT_FILE, 0, SHN_ABS);

    ELFIO::section *text_sec = elf_writer.sections.add(".text");
    text_sec->set_type(SHT_PROGBITS);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    text_sec->set_flags(SHF_ALLOC | SHF_EXECINSTR);
    text_sec->set_addr_align(GetCodeAlignment(arch_));

    ELFIO::section *rel_sec = elf_writer.sections.add(".rela.text");
    rel_sec->set_type(SHT_RELA);
    rel_sec->set_info(text_sec->get_index());
    rel_sec->set_link(sym_sec->get_index());
    const auto ADDR_ALIGN = 4;
    rel_sec->set_addr_align(ADDR_ALIGN);
    rel_sec->set_entry_size(elf_writer.get_default_entry_size(SHT_RELA));
    ELFIO::relocation_section_accessor rel_writer(elf_writer, rel_sec);

    /* Use symbols map to avoid saving the same symbols multiple times */
    std::unordered_map<std::string, uint32_t> symbols_map;
    auto add_symbol = [&](const char *name) {
        if (auto it = symbols_map.find(name); it != symbols_map.end()) {
            return it->second;
        }
        uint32_t index = symbol_writer.add_symbol(str_writer, name, 0, 0, STB_GLOBAL, STT_NOTYPE, 0, 0);
        symbols_map.insert({name, index});
        return index;
    };

    size_t offset = 0;
    for (auto unit : units_) {
        auto code = unit->GetCode();
        text_sec->append_data(reinterpret_cast<const char *>(code.data()), code.size());
        symbol_writer.add_symbol(str_writer, unit->GetName(), offset, code.size(), STB_GLOBAL, STT_FUNC, 0,
                                 text_sec->get_index());
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
        offset += code.size();
    }

    elf_writer.save(output.data());

    return 0;
}

}  // namespace panda::irtoc
