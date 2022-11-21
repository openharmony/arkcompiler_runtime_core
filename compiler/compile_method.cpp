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

#include "optimizer_run.h"
#include "mem/pool_manager.h"
#include "mem/code_allocator.h"
#include "include/class.h"
#include "include/method.h"
#include "optimizer/ir/ir_constructor.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/pass.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "utils/logger.h"
#include "code_info/code_info.h"
#include "events/events.h"
#include "trace/trace.h"
#include "optimizer/code_generator/codegen.h"
#include "compile_method.h"

namespace panda::compiler {

#ifdef PANDA_COMPILER_CFI
static Span<uint8_t> EmitElf(Graph *graph, CodeAllocator *code_allocator, ArenaAllocator *gdb_debug_info_allocator,
                             const std::string &method_name);
#endif

static Arch ChooseArch(Arch arch)
{
    if (arch != Arch::NONE) {
        return arch;
    }

    arch = RUNTIME_ARCH;
    if (RUNTIME_ARCH == Arch::X86_64 && options.WasSetCompilerCrossArch()) {
        arch = GetArchFromString(options.GetCompilerCrossArch());
    }

    return arch;
}

static bool CheckSingleImplementation(Graph *graph)
{
    // Check that all methods that were inlined due to its single implementation property, still have this property,
    // otherwise we must drop compiled code.
    // TODO(compiler): we need to reset hotness counter hereby avoid yet another warmup phase.
    auto cha = graph->GetRuntime()->GetCha();
    for (auto si_method : graph->GetSingleImplementationList()) {
        if (!cha->IsSingleImplementation(si_method)) {
            LOG(WARNING, COMPILER)
                << "Method lost single-implementation property after compilation, so we need to drop "
                   "whole compiled code: "
                << graph->GetRuntime()->GetMethodFullName(si_method);
            return false;
        }
    }
    return true;
}

static Span<uint8_t> EmitCode(const Graph *graph, CodeAllocator *allocator)
{
    size_t code_offset = RoundUp(CodePrefix::STRUCT_SIZE, GetCodeAlignment(graph->GetArch()));
    CodePrefix prefix;
    prefix.code_size = graph->GetData().size();
    prefix.code_info_offset = code_offset + RoundUp(graph->GetData().size(), sizeof(uint32_t));
    prefix.code_info_size = graph->GetCodeInfoData().size();
    size_t code_size = prefix.code_info_offset + prefix.code_info_size;
    auto mem_range = allocator->AllocateCodeUnprotected(code_size);
    if (mem_range.GetSize() == 0) {
        return Span<uint8_t> {};
    }

    auto data = reinterpret_cast<uint8_t *>(mem_range.GetData());
    memcpy_s(data, sizeof(CodePrefix), &prefix, sizeof(CodePrefix));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy_s(&data[code_offset], graph->GetData().size(), graph->GetData().data(), graph->GetData().size());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy_s(&data[prefix.code_info_offset], graph->GetCodeInfoData().size(), graph->GetCodeInfoData().data(),
             graph->GetCodeInfoData().size());

    allocator->ProtectCode(mem_range);

    return Span<uint8_t>(reinterpret_cast<uint8_t *>(mem_range.GetData()), code_size);
}

uint8_t *GetEntryPoint(Graph *graph, [[maybe_unused]] Method *method, const std::string &method_name,
                       [[maybe_unused]] bool is_osr, CodeAllocator *code_allocator,
                       [[maybe_unused]] ArenaAllocator *gdb_debug_info_allocator)
{
#ifdef PANDA_COMPILER_CFI
    auto generated_data = options.IsCompilerEmitDebugInfo()
                              ? EmitElf(graph, code_allocator, gdb_debug_info_allocator, method_name)
                              : EmitCode(graph, code_allocator);
#else
    auto generated_data = EmitCode(graph, code_allocator);
#endif
    if (generated_data.Empty()) {
        LOG(INFO, COMPILER) << "Compilation failed due to memory allocation fail: " << method_name;
        return nullptr;
    }
    CodeInfo code_info(generated_data);
    LOG(INFO, COMPILER) << "Compiled code for '" << method_name << "' has been installed to "
                        << bit_cast<void *>(code_info.GetCode()) << ", code size " << code_info.GetCodeSize();

    auto entry_point = const_cast<uint8_t *>(code_info.GetCode());
    EVENT_COMPILATION(method_name, is_osr, method->GetCodeSize(), reinterpret_cast<uintptr_t>(entry_point),
                      code_info.GetCodeSize(), code_info.GetInfoSize(), events::CompilationStatus::COMPILED);
    return entry_point;
}

bool JITCompileMethod(RuntimeInterface *runtime, Method *method, bool is_osr, CodeAllocator *code_allocator,
                      ArenaAllocator *allocator, ArenaAllocator *local_allocator,
                      ArenaAllocator *gdb_debug_info_allocator)
{
    std::string method_name = runtime->GetMethodFullName(method, false);
    SCOPED_TRACE_STREAM << "JIT compiling " << method_name;

    if (!options.MatchesRegex(method_name)) {
        LOG(DEBUG, COMPILER) << "Skip the method due to regexp mismatch: " << method_name;
        return false;
    }

    Graph *graph {nullptr};
    auto finalizer = [&graph]([[maybe_unused]] void *ptr) {
        if (graph != nullptr) {
            graph->~Graph();
        }
    };
    std::unique_ptr<void, decltype(finalizer)> fin(&finalizer, finalizer);

    auto arch {Arch::NONE};
    bool is_dynamic = panda::panda_file::IsDynamicLanguage(method->GetClass()->GetSourceLang());

    if (!CompileInGraph(runtime, method, is_osr, allocator, local_allocator, is_dynamic, &arch, method_name, &graph)) {
        return false;
    }
    ASSERT(graph != nullptr && graph->GetData().data() != nullptr);

    if (!is_dynamic && !CheckSingleImplementation(graph)) {
        EVENT_COMPILATION(method_name, is_osr, method->GetCodeSize(), 0, 0, 0,
                          events::CompilationStatus::FAILED_SINGLE_IMPL);
        return false;
    }

    // Drop non-native code in any case
    if (arch != RUNTIME_ARCH) {
        EVENT_COMPILATION(method_name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::DROPPED);
        return false;
    }

    auto entry_point = GetEntryPoint(graph, method, method_name, is_osr, code_allocator, gdb_debug_info_allocator);
    if (entry_point == nullptr) {
        return false;
    }
    if (is_osr) {
        if (runtime->HasCompiledCode(method)) {
            runtime->SetOsrCode(method, entry_point);
            ASSERT(runtime->GetOsrCode(method) != nullptr);
        } else {
            // Compiled code has been deoptimized, so we shouldn't install osr code.
            // TODO(compiler): release compiled code memory, when CodeAllocator supports freeing the memory.
            return false;
        }
    } else {
        runtime->SetCompiledEntryPoint(method, entry_point);
    }
    ASSERT(graph != nullptr);
    return true;
}

bool CompileInGraph(RuntimeInterface *runtime, Method *method, bool is_osr, ArenaAllocator *allocator,
                    ArenaAllocator *local_allocator, bool is_dynamic, Arch *arch, const std::string &method_name,
                    Graph **graph)
{
    LOG(INFO, COMPILER) << "Compile method" << (is_osr ? "(OSR)" : "") << ": " << method_name << " ("
                        << runtime->GetFileName(method) << ')';
    *arch = ChooseArch(*arch);
    if (*arch == Arch::NONE || !BackendSupport(*arch)) {
        LOG(DEBUG, COMPILER) << "Compilation unsupported for this platform!";
        return false;
    }

    ASSERT(*graph == nullptr);
    *graph = allocator->New<Graph>(allocator, local_allocator, *arch, method, runtime, is_osr, nullptr, is_dynamic);
    if (*graph == nullptr) {
        LOG(ERROR, COMPILER) << "Creating graph failed!";
        EVENT_COMPILATION(method_name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED);
        return false;
    }

    if (!(*graph)->RunPass<IrBuilder>()) {
        if (!compiler::options.IsCompilerIgnoreFailures()) {
            LOG(FATAL, COMPILER) << "IrBuilder failed!";
        }
        LOG(WARNING, COMPILER) << "IrBuilder failed!";
        EVENT_COMPILATION(method_name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED);
        return false;
    }

    // Run compiler optimizations over created graph
    bool res = RunOptimizations(*graph);
    if (!res) {
        if (!compiler::options.IsCompilerIgnoreFailures()) {
            LOG(FATAL, COMPILER) << "RunOptimizations failed!";
        }
        LOG(WARNING, COMPILER) << "RunOptimizations failed!";
        EVENT_COMPILATION(method_name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED);
        return false;
    }

    LOG(DEBUG, COMPILER) << "The method is compiled";

    return true;
}
}  // namespace panda::compiler

#ifdef PANDA_COMPILER_CFI

#include "optimizer/ir/aot_data.h"
#include "tools/debug/jit_writer.h"

// Next "C"-code need for enable interaction with gdb
// Please read "JIT Compilation Interface" from gdb-documentation for more information
extern "C" {
// Gdb will replace implementation of this function
void NO_INLINE __jit_debug_register_code(void)
{
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm("");
}

// Default version for descriptor (may be checked before register code)
// NOLINTNEXTLINE(modernize-use-nullptr)
jit_descriptor __jit_debug_descriptor = {1, JIT_NOACTION, NULL, NULL};
}  // extern "C"

namespace panda::compiler {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static os::memory::Mutex jit_debug_lock;

// Will register jit-elf description in linked list
static void RegisterJitCode(jit_code_entry *entry)
{
    ASSERT(options.IsCompilerEmitDebugInfo());

    os::memory::LockHolder lock(jit_debug_lock);
    // Re-link list
    entry->next_entry = __jit_debug_descriptor.first_entry;
    if (__jit_debug_descriptor.first_entry != nullptr) {
        __jit_debug_descriptor.first_entry->prev_entry = entry;
    }
    __jit_debug_descriptor.first_entry = entry;

    // Fill last entry
    __jit_debug_descriptor.relevant_entry = entry;
    __jit_debug_descriptor.action_flag = JIT_REGISTER_FN;

    // Call gdb-callback
    __jit_debug_register_code();
    __jit_debug_descriptor.action_flag = JIT_NOACTION;
    __jit_debug_descriptor.relevant_entry = nullptr;
}

// When code allocator cleaned - also will clean entry
void CleanJitDebugCode()
{
    ASSERT(options.IsCompilerEmitDebugInfo());

    os::memory::LockHolder lock(jit_debug_lock);
    __jit_debug_descriptor.action_flag = JIT_UNREGISTER_FN;

    while (__jit_debug_descriptor.first_entry != nullptr) {
        __jit_debug_descriptor.first_entry->prev_entry = nullptr;
        __jit_debug_descriptor.relevant_entry = __jit_debug_descriptor.first_entry;
        // Call gdb-callback
        __jit_debug_register_code();

        __jit_debug_descriptor.first_entry = __jit_debug_descriptor.first_entry->next_entry;
    }

    __jit_debug_descriptor.action_flag = JIT_NOACTION;
    __jit_debug_descriptor.relevant_entry = nullptr;
}

// For each jit code - will generate small elf description and put them in gdb-special linked list.
static Span<uint8_t> EmitElf(Graph *graph, CodeAllocator *code_allocator, ArenaAllocator *gdb_debug_info_allocator,
                             const std::string &method_name)
{
    ASSERT(options.IsCompilerEmitDebugInfo());

    if (graph->GetData().Empty()) {
        return {};
    }

    JitDebugWriter jit_writer(graph->GetArch(), graph->GetRuntime(), code_allocator, method_name);

    jit_writer.Start();

    auto method = reinterpret_cast<Method *>(graph->GetMethod());
    auto klass = reinterpret_cast<Class *>(graph->GetRuntime()->GetClass(method));
    jit_writer.StartClass(*klass);

    CompiledMethod compiled_method(graph->GetArch(), method);
    compiled_method.SetCode(graph->GetData().ToConst());
    compiled_method.SetCodeInfo(graph->GetCodeInfoData());
    compiled_method.SetCfiInfo(graph->GetCallingConvention()->GetCfiInfo());

    jit_writer.AddMethod(compiled_method, 0);
    jit_writer.EndClass();
    jit_writer.End();
    if (!jit_writer.Write()) {
        return {};
    }

    auto gdb_entry {gdb_debug_info_allocator->New<jit_code_entry>()};
    if (gdb_entry == nullptr) {
        return {};
    }

    auto elf_file {jit_writer.GetElf()};
    // Pointer to Elf-file entry
    gdb_entry->symfile_addr = reinterpret_cast<const char *>(elf_file.Data());
    // Elf-in-memory file size
    gdb_entry->symfile_size = elf_file.Size();
    gdb_entry->prev_entry = nullptr;

    RegisterJitCode(gdb_entry);
    return jit_writer.GetCode();
}

}  // namespace panda::compiler
#endif
