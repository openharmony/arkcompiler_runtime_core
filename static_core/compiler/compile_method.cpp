/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "compile_method.h"
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
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/code_generator/codegen.h"

namespace panda::compiler {

#ifdef PANDA_COMPILER_DEBUG_INFO
static Span<uint8_t> EmitElf(Graph *graph, CodeAllocator *code_allocator, ArenaAllocator *gdb_debug_info_allocator,
                             const std::string &method_name);
#endif

void JITStats::SetCompilationStart()
{
    ASSERT(start_time_ == 0);
    start_time_ = time::GetCurrentTimeInNanos();
}
void JITStats::EndCompilationWithStats(const std::string &method_name, bool is_osr, size_t bc_size, size_t code_size)
{
    ASSERT(start_time_ != 0);
    auto time = time::GetCurrentTimeInNanos() - start_time_;
    stats_list_.push_back(
        Entry {PandaString(method_name, internal_allocator_->Adapter()), is_osr, bc_size, code_size, time});
    start_time_ = 0;
}

void JITStats::ResetCompilationStart()
{
    start_time_ = 0;
}

void JITStats::DumpCsv(char sep)
{
    ASSERT(OPTIONS.WasSetCompilerDumpJitStatsCsv());
    std::ofstream csv(OPTIONS.GetCompilerDumpJitStatsCsv(), std::ofstream::trunc);
    for (const auto &i : stats_list_) {
        csv << "\"" << i.method_name << "\"" << sep;
        csv << i.is_osr << sep;
        csv << i.bc_size << sep;
        csv << i.code_size << sep;
        csv << i.time;
        csv << '\n';
    }
}

static void EndCompilation(const std::string &method_name, bool is_osr, size_t bc_size,
                           [[maybe_unused]] uintptr_t address, size_t code_size, [[maybe_unused]] size_t info_size,
                           [[maybe_unused]] events::CompilationStatus status, JITStats *jit_stats)
{
    EVENT_COMPILATION(method_name, is_osr, bc_size, address, code_size, info_size, status);
    if (jit_stats != nullptr) {
        ASSERT((code_size != 0) == (status == events::CompilationStatus::COMPILED));
        jit_stats->EndCompilationWithStats(method_name, is_osr, bc_size, code_size);
    }
}

Arch ChooseArch(Arch arch)
{
    if (arch != Arch::NONE) {
        return arch;
    }

    arch = RUNTIME_ARCH;
    if (RUNTIME_ARCH == Arch::X86_64 && OPTIONS.WasSetCompilerCrossArch()) {
        arch = GetArchFromString(OPTIONS.GetCompilerCrossArch());
    }

    return arch;
}

static bool CheckSingleImplementation(Graph *graph)
{
    // Check that all methods that were inlined due to its single implementation property, still have this property,
    // otherwise we must drop compiled code.
    // NOTE(compiler): we need to reset hotness counter hereby avoid yet another warmup phase.
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
    prefix.code_size = graph->GetCode().size();
    prefix.code_info_offset = code_offset + RoundUp(graph->GetCode().size(), sizeof(uint32_t));
    prefix.code_info_size = graph->GetCodeInfoData().size();
    size_t code_size = prefix.code_info_offset + prefix.code_info_size;
    auto mem_range = allocator->AllocateCodeUnprotected(code_size);
    if (mem_range.GetSize() == 0) {
        return Span<uint8_t> {};
    }

    auto data = reinterpret_cast<uint8_t *>(mem_range.GetData());
    memcpy_s(data, sizeof(CodePrefix), &prefix, sizeof(CodePrefix));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy_s(&data[code_offset], graph->GetCode().size(), graph->GetCode().data(), graph->GetCode().size());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy_s(&data[prefix.code_info_offset], graph->GetCodeInfoData().size(), graph->GetCodeInfoData().data(),
             graph->GetCodeInfoData().size());

    allocator->ProtectCode(mem_range);

    return Span<uint8_t>(reinterpret_cast<uint8_t *>(mem_range.GetData()), code_size);
}

static uint8_t *GetEntryPoint(Graph *graph, [[maybe_unused]] Method *method, const std::string &method_name,
                              [[maybe_unused]] bool is_osr, CodeAllocator *code_allocator,
                              [[maybe_unused]] ArenaAllocator *gdb_debug_info_allocator, JITStats *jit_stats)
{
#ifdef PANDA_COMPILER_DEBUG_INFO
    auto generated_data = OPTIONS.IsCompilerEmitDebugInfo()
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
    EndCompilation(method_name, is_osr, method->GetCodeSize(), reinterpret_cast<uintptr_t>(entry_point),
                   code_info.GetCodeSize(), code_info.GetInfoSize(), events::CompilationStatus::COMPILED, jit_stats);
    return entry_point;
}

template <TaskRunnerMode RUNNER_MODE>
static void RunOptimizations(CompilerTaskRunner<RUNNER_MODE> task_runner, JITStats *jit_stats)
{
    auto &task_ctx = task_runner.GetContext();
    task_ctx.GetGraph()->SetLanguage(task_ctx.GetMethod()->GetClass()->GetSourceLang());

    task_runner.AddCallbackOnSuccess([]([[maybe_unused]] CompilerContext<RUNNER_MODE> &compiler_ctx) {
        LOG(DEBUG, COMPILER) << "The method " << compiler_ctx.GetMethodName() << " is compiled";
    });
    task_runner.AddCallbackOnFail([jit_stats](CompilerContext<RUNNER_MODE> &compiler_ctx) {
        if (!compiler::OPTIONS.IsCompilerIgnoreFailures()) {
            LOG(FATAL, COMPILER) << "RunOptimizations failed!";
        }
        LOG(WARNING, COMPILER) << "RunOptimizations failed!";
        EndCompilation(compiler_ctx.GetMethodName(), compiler_ctx.IsOsr(), compiler_ctx.GetMethod()->GetCodeSize(), 0,
                       0, 0, events::CompilationStatus::FAILED, jit_stats);
    });

    // Run compiler optimizations over created graph
    RunOptimizations<RUNNER_MODE>(std::move(task_runner));
}

template <TaskRunnerMode RUNNER_MODE>
static bool CheckCompilation(RuntimeInterface *runtime, CodeAllocator *code_allocator,
                             ArenaAllocator *gdb_debug_info_allocator, JITStats *jit_stats, bool is_dynamic, Arch arch,
                             CompilerContext<RUNNER_MODE> &compiler_ctx)
{
    auto *graph = compiler_ctx.GetGraph();

    ASSERT(graph != nullptr && graph->GetCode().data() != nullptr);

    auto &name = compiler_ctx.GetMethodName();
    auto is_osr = compiler_ctx.IsOsr();
    auto *method = compiler_ctx.GetMethod();

    if (!is_dynamic && !CheckSingleImplementation(graph)) {
        EndCompilation(name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED_SINGLE_IMPL,
                       jit_stats);
        return false;
    }

    // Drop non-native code in any case
    if (arch != RUNTIME_ARCH) {
        EndCompilation(name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::DROPPED, jit_stats);
        return false;
    }

    auto entry_point = GetEntryPoint(graph, method, name, is_osr, code_allocator, gdb_debug_info_allocator, jit_stats);
    if (entry_point == nullptr) {
        return false;
    }
    if (is_osr) {
        if (!runtime->TrySetOsrCode(method, entry_point)) {
            // Compiled code has been deoptimized, so we shouldn't install osr code.
            // NOTE(compiler): release compiled code memory, when CodeAllocator supports freeing the memory.
            return false;
        }
    } else {
        runtime->SetCompiledEntryPoint(method, entry_point);
    }
    return true;
}

template <TaskRunnerMode RUNNER_MODE>
void JITCompileMethod(RuntimeInterface *runtime, CodeAllocator *code_allocator,
                      ArenaAllocator *gdb_debug_info_allocator, JITStats *jit_stats,
                      CompilerTaskRunner<RUNNER_MODE> task_runner)
{
    auto &task_ctx = task_runner.GetContext();
    auto *task_method = task_ctx.GetMethod();
    task_ctx.SetMethodName(runtime->GetMethodFullName(task_method, false));
    auto &method_name = task_ctx.GetMethodName();

    SCOPED_TRACE_STREAM << "JIT compiling " << method_name;

    if (!OPTIONS.MatchesRegex(method_name)) {
        LOG(DEBUG, COMPILER) << "Skip the method due to regexp mismatch: " << method_name;
        task_ctx.SetCompilationStatus(false);
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), false);
        return;
    }

    if (jit_stats != nullptr) {
        jit_stats->SetCompilationStart();
    }

    task_runner.AddFinalize([jit_stats](CompilerContext<RUNNER_MODE> &compiler_ctx) {
        if (jit_stats != nullptr) {
            // Reset compilation start time in all cases for consistency
            jit_stats->ResetCompilationStart();
        }
        auto *graph = compiler_ctx.GetGraph();
        if (graph != nullptr) {
            graph->~Graph();
        }
    });

    auto arch = ChooseArch(Arch::NONE);
    bool is_dynamic = panda::panda_file::IsDynamicLanguage(task_method->GetClass()->GetSourceLang());

    task_runner.AddCallbackOnSuccess([runtime, code_allocator, gdb_debug_info_allocator, jit_stats, is_dynamic,
                                      arch](CompilerContext<RUNNER_MODE> &compiler_ctx) {
        bool compilation_status = CheckCompilation<RUNNER_MODE>(runtime, code_allocator, gdb_debug_info_allocator,
                                                                jit_stats, is_dynamic, arch, compiler_ctx);
        compiler_ctx.SetCompilationStatus(compilation_status);
    });
    task_runner.AddCallbackOnFail(
        [](CompilerContext<RUNNER_MODE> &compiler_ctx) { compiler_ctx.SetCompilationStatus(false); });

    CompileInGraph<RUNNER_MODE>(runtime, is_dynamic, arch, std::move(task_runner), jit_stats);
}

template <TaskRunnerMode RUNNER_MODE>
void CompileInGraph(RuntimeInterface *runtime, bool is_dynamic, Arch arch, CompilerTaskRunner<RUNNER_MODE> task_runner,
                    JITStats *jit_stats)
{
    auto &task_ctx = task_runner.GetContext();
    auto is_osr = task_ctx.IsOsr();
    auto *method = task_ctx.GetMethod();
    auto &method_name = task_ctx.GetMethodName();

    LOG(INFO, COMPILER) << "Compile method" << (is_osr ? "(OSR)" : "") << ": " << method_name << " ("
                        << runtime->GetFileName(method) << ')';

    if (arch == Arch::NONE || !BackendSupport(arch)) {
        LOG(DEBUG, COMPILER) << "Compilation unsupported for this platform!";
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), false);
        return;
    }

    auto *allocator = task_ctx.GetAllocator();
    auto *local_allocator = task_ctx.GetLocalAllocator();
    auto *graph =
        allocator->template New<Graph>(allocator, local_allocator, arch, method, runtime, is_osr, nullptr, is_dynamic);
    task_ctx.SetGraph(graph);
    if (graph == nullptr) {
        LOG(ERROR, COMPILER) << "Creating graph failed!";
        EndCompilation(method_name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED,
                       jit_stats);
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), false);
        return;
    }

    task_runner.SetTaskOnSuccess([jit_stats](CompilerTaskRunner<RUNNER_MODE> next_runner) {
        RunOptimizations<RUNNER_MODE>(std::move(next_runner), jit_stats);
    });

    bool success = graph->template RunPass<IrBuilder>();
    if (!success) {
        if (!compiler::OPTIONS.IsCompilerIgnoreFailures()) {
            LOG(FATAL, COMPILER) << "IrBuilder failed!";
        }
        LOG(WARNING, COMPILER) << "IrBuilder failed!";
        EndCompilation(method_name, is_osr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED,
                       jit_stats);
    };
    CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), success);
}

template void JITCompileMethod<BACKGROUND_MODE>(RuntimeInterface *, CodeAllocator *, ArenaAllocator *, JITStats *,
                                                CompilerTaskRunner<BACKGROUND_MODE>);
template void JITCompileMethod<INPLACE_MODE>(RuntimeInterface *, CodeAllocator *, ArenaAllocator *, JITStats *,
                                             CompilerTaskRunner<INPLACE_MODE>);
template void CompileInGraph<BACKGROUND_MODE>(RuntimeInterface *, bool, Arch, CompilerTaskRunner<BACKGROUND_MODE>,
                                              JITStats *);
template void CompileInGraph<INPLACE_MODE>(RuntimeInterface *, bool, Arch, CompilerTaskRunner<INPLACE_MODE>,
                                           JITStats *);
template void RunOptimizations<BACKGROUND_MODE>(CompilerTaskRunner<BACKGROUND_MODE>, JITStats *);
template void RunOptimizations<INPLACE_MODE>(CompilerTaskRunner<INPLACE_MODE>, JITStats *);
template bool CheckCompilation<BACKGROUND_MODE>(RuntimeInterface *, CodeAllocator *, ArenaAllocator *, JITStats *, bool,
                                                Arch, CompilerContext<BACKGROUND_MODE> &);
template bool CheckCompilation<INPLACE_MODE>(RuntimeInterface *, CodeAllocator *, ArenaAllocator *, JITStats *, bool,
                                             Arch, CompilerContext<INPLACE_MODE> &);

}  // namespace panda::compiler

#ifdef PANDA_COMPILER_DEBUG_INFO

#include "optimizer/ir/aot_data.h"
#include "tools/debug/jit_writer.h"

// Next "C"-code need for enable interaction with gdb
// Please read "JIT Compilation Interface" from gdb-documentation for more information
extern "C" {
// Gdb will replace implementation of this function
// NOLINTNEXTLINE(readability-identifier-naming)
void NO_INLINE __jit_debug_register_code(void)
{
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm("");
}

// Default version for descriptor (may be checked before register code)
// NOLINTNEXTLINE(modernize-use-nullptr, readability-identifier-naming)
jit_descriptor __jit_debug_descriptor = {1, JIT_NOACTION, NULL, NULL};
}  // extern "C"

namespace panda::compiler {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static os::memory::Mutex JIT_DEBUG_LOCK;

// Will register jit-elf description in linked list
static void RegisterJitCode(jit_code_entry *entry)
{
    ASSERT(OPTIONS.IsCompilerEmitDebugInfo());

    os::memory::LockHolder lock(JIT_DEBUG_LOCK);
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
    ASSERT(OPTIONS.IsCompilerEmitDebugInfo());

    os::memory::LockHolder lock(JIT_DEBUG_LOCK);
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
    ASSERT(OPTIONS.IsCompilerEmitDebugInfo());

    if (graph->GetCode().Empty()) {
        return {};
    }

    JitDebugWriter jit_writer(graph->GetArch(), graph->GetRuntime(), code_allocator, method_name);

    jit_writer.Start();

    auto method = reinterpret_cast<Method *>(graph->GetMethod());
    auto klass = reinterpret_cast<Class *>(graph->GetRuntime()->GetClass(method));
    jit_writer.StartClass(*klass);

    CompiledMethod compiled_method(graph->GetArch(), method, 0);
    compiled_method.SetCode(graph->GetCode().ToConst());
    compiled_method.SetCodeInfo(graph->GetCodeInfoData());
    compiled_method.SetCfiInfo(graph->GetCallingConvention()->GetCfiInfo());

    jit_writer.AddMethod(compiled_method);
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
