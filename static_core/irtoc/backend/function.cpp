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

#include "function.h"
#include "compiler/codegen_boundary.h"
#include "compiler/codegen_fastpath.h"
#include "compiler/codegen_interpreter.h"
#include "compiler/optimizer_run.h"
#include "compiler/dangling_pointers_checker.h"
#include "compiler/optimizer/code_generator/target_info.h"
#include "compiler/optimizer/optimizations/balance_expressions.h"
#include "compiler/optimizer/optimizations/branch_elimination.h"
#include "compiler/optimizer/optimizations/checks_elimination.h"
#include "compiler/optimizer/optimizations/code_sink.h"
#include "compiler/optimizer/optimizations/cse.h"
#include "compiler/optimizer/optimizations/deoptimize_elimination.h"
#include "compiler/optimizer/optimizations/if_conversion.h"
#include "compiler/optimizer/optimizations/if_merging.h"
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
#include "elfio/elfio.hpp"
#include "irtoc_runtime.h"

namespace panda::irtoc {

using compiler::Graph;

static bool RunIrtocOptimizations(Graph *graph);
static bool RunIrtocInterpreterOptimizations(Graph *graph);

Function::Result Function::Compile(Arch arch, ArenaAllocator *allocator, ArenaAllocator *local_allocator)
{
    IrtocRuntimeInterface runtime;

    // NOLINTNEXTLINE(readability-identifier-naming)
    ArenaAllocator &allocator_ = *allocator;
    // NOLINTNEXTLINE(readability-identifier-naming)
    ArenaAllocator &local_allocator_ = *local_allocator;
    // NOLINTNEXTLINE(readability-identifier-naming)
    graph_ = allocator_.New<Graph>(&allocator_, &local_allocator_, arch, this, &runtime, false);
    builder_ = std::make_unique<compiler::IrConstructor>();

    MakeGraphImpl();

#ifdef PANDA_COMPILER_DEBUG_INFO
    // need to fix other archs
    if (arch == Arch::X86_64) {
        GetGraph()->SetLineDebugInfoEnabled();
    }
#endif

#ifdef PANDA_LLVMAOT
    graph_mode_ = GetGraph()->GetMode();
    bool has_llvm = true;
#else
    bool has_llvm = false;
#endif
    bool has_llvm_suffix = std::string_view(GetName()).find(LLVM_SUFFIX) != std::string_view::npos;

    if (GetGraph()->GetMode().IsNative()) {
        if (!RunOptimizations(GetGraph())) {
            return Unexpected("RunOptimizations failed!");
        }
        llvm_compilation_result_ = LLVMCompilationResult::USE_ARK_AS_NO_SUFFIX;
    } else {
        LLVMCompilationResult compilation_result = LLVMCompilationResult::INVALID;
        if (GetGraph()->GetMode().IsInterpreter() || GetGraph()->GetMode().IsInterpreterEntry()) {
            bool compile_by_ark = false;
            if (has_llvm_suffix) {
                // When build without LLVM, interpreter handlers with "_LLVM" suffix must be skipped
                if (!has_llvm) {
                    return 1;
                }
                compilation_result = CompileByLLVM(&runtime, &allocator_);
                ASSERT(compilation_result == LLVMCompilationResult::USE_ARK_AS_SKIP_LIST ||
                       compilation_result == LLVMCompilationResult::LLVM_COMPILED ||
                       compilation_result == LLVMCompilationResult::LLVM_CAN_NOT_COMPILE);
                compile_by_ark = compilation_result != LLVMCompilationResult::LLVM_COMPILED;
            } else {
                compilation_result = LLVMCompilationResult::USE_ARK_AS_NO_SUFFIX;
                compile_by_ark = true;
            }
            if (compile_by_ark) {
                ASSERT(GetGraph()->GetCode().Empty());
                if (!RunIrtocInterpreterOptimizations(GetGraph())) {
                    LOG(ERROR, IRTOC) << "RunIrtocInterpreterOptimizations failed for " << GetName();
                    return Unexpected("RunIrtocInterpreterOptimizations failed!");
                }
            }
        } else {
            LOG_IF(has_llvm_suffix, FATAL, IRTOC)
                << "'" << GetName() << "' must not contain '" << LLVM_SUFFIX
                << "' because only interpreter handlers are compiled by different compilers";
            bool compile_by_ark = false;
            if (has_llvm) {
                compilation_result = CompileByLLVM(&runtime, &allocator_);
                ASSERT(compilation_result == LLVMCompilationResult::USE_ARK_AS_SKIP_LIST ||
                       compilation_result == LLVMCompilationResult::LLVM_COMPILED ||
                       compilation_result == LLVMCompilationResult::LLVM_CAN_NOT_COMPILE);
                compile_by_ark = compilation_result != LLVMCompilationResult::LLVM_COMPILED;
            } else {
                compilation_result = LLVMCompilationResult::USE_ARK_AS_SKIP_LIST;
                compile_by_ark = true;
            }
            if (compile_by_ark) {
                ASSERT(GetGraph()->GetCode().Empty());
                if (!RunIrtocOptimizations(GetGraph())) {
                    return Unexpected("RunOptimizations failed!");
                }
            }
        }
        ASSERT(compilation_result != LLVMCompilationResult::INVALID);
        llvm_compilation_result_ = compilation_result;
    }
    builder_.reset(nullptr);

    auto code = GetGraph()->GetCode();
    SetCode(code);

    return 0;
}

void Function::SetCode(Span<uint8_t> code)
{
    std::copy(code.begin(), code.end(), std::back_inserter(code_));
}

LLVMCompilationResult Function::CompileByLLVM([[maybe_unused]] panda::compiler::RuntimeInterface *runtime,
                                              [[maybe_unused]] ArenaAllocator *allocator)
{
#ifndef PANDA_LLVMAOT
    UNREACHABLE();
#else
    if (SkippedByLLVM()) {
        return LLVMCompilationResult::USE_ARK_AS_SKIP_LIST;
    }
    ASSERT(llvm_aot_compiler_ != nullptr);
    auto can = llvm_aot_compiler_->CanCompile(GetGraph());
    if (!can.HasValue()) {
        LOG(FATAL, IRTOC) << can.Error() << "\n";
    }
    if (!can.Value()) {
        return LLVMCompilationResult::LLVM_CAN_NOT_COMPILE;
    }
    if (!(llvm_aot_compiler_->AddGraph(GetGraph()))) {
        LOG(FATAL, IRTOC) << "LLVM compilation failed on compilable code graph for " << GetName() << " unit";
    }
    return LLVMCompilationResult::LLVM_COMPILED;
#endif  // ifndef PANDA_LLVMAOT
}

#ifdef PANDA_LLVMAOT
void Function::ReportCompilationStatistic(std::ostream *out)
{
    ASSERT(out != nullptr);
    ASSERT(!OPTIONS.Validate());
    std::string stats_level = OPTIONS.GetIrtocLlvmStats();
    if (stats_level == "none") {
        return;
    }
    if (stats_level != "full" && GetGraph()->IsDynamicMethod()) {
        return;
    }
    if (stats_level == "short") {
        if (llvm_compilation_result_ == LLVMCompilationResult::LLVM_COMPILED ||
            llvm_compilation_result_ == LLVMCompilationResult::USE_ARK_AS_NO_SUFFIX) {
            return;
        }
    }

    static constexpr auto HANDLER_WIDTH = 96;
    static constexpr auto RESULT_WIDTH = 5;

    // "ark" if compilation unit was compiled by ark because the name does not end with LLVM_SUFFIX
    // "ok" if successfully compiled by llvm
    // "cant" compiled by ark because LLVMAotCompiler::CanCompile returned false
    // "skip" compiled by ark because SkippedByLLVM returned true
    *out << "LLVM " << GraphModeToString() << std::setw(HANDLER_WIDTH) << GetName() << " --- "
         << std::setw(RESULT_WIDTH) << LLVMCompilationResultToString() << "(size " << GetCodeSize() << ")" << std::endl;
}

std::string_view Function::LLVMCompilationResultToString() const
{
    if (llvm_compilation_result_ == LLVMCompilationResult::USE_ARK_AS_NO_SUFFIX) {
        return "ark";
    }
    if (llvm_compilation_result_ == LLVMCompilationResult::LLVM_COMPILED) {
        return "ok";
    }
    if (llvm_compilation_result_ == LLVMCompilationResult::LLVM_CAN_NOT_COMPILE) {
        return "cant";
    }
    if (llvm_compilation_result_ == LLVMCompilationResult::USE_ARK_AS_SKIP_LIST) {
        return "skip";
    }
    UNREACHABLE();
}

std::string_view Function::GraphModeToString()
{
    if (GetGraphMode().IsFastPath()) {
        return "fastpath";
    }
    if (GetGraphMode().IsBoundary()) {
        return "boundary";
    }
    if (GetGraphMode().IsInterpreter() || GetGraphMode().IsInterpreterEntry()) {
        return "interp";
    }
    if (GetGraphMode().IsNative()) {
        return "native";
    }
    UNREACHABLE();
}
#endif

void Function::AddRelocation(const compiler::RelocationInfo &info)
{
    relocation_entries_.emplace_back(info);
}

#ifdef PANDA_LLVMAOT
bool Function::SkippedByLLVM()
{
    auto name = std::string(GetName());
    if (GetGraph()->GetMode().IsInterpreterEntry() || GetGraph()->GetMode().IsInterpreter()) {
        ASSERT(name.find(LLVM_SUFFIX) == name.size() - LLVM_SUFFIX.size());
        name = name.substr(0, name.size() - LLVM_SUFFIX.size());
    }

    if (GetArch() == Arch::X86_64) {
        if (GetGraph()->GetMode().IsInterpreterEntry() || GetGraph()->GetMode().IsInterpreter()) {
            return std::find(X86_64_SKIPPED_INTERPRETER_HANDLERS.begin(), X86_64_SKIPPED_INTERPRETER_HANDLERS.end(),
                             name) != X86_64_SKIPPED_INTERPRETER_HANDLERS.end();
        }
        return true;
    }
    if (GetArch() == Arch::AARCH64) {
        if (GetGraph()->GetMode().IsInterpreter() || GetGraph()->GetMode().IsInterpreterEntry()) {
            return std::find(AARCH64_SKIPPED_INTERPRETER_HANDLERS.begin(), AARCH64_SKIPPED_INTERPRETER_HANDLERS.end(),
                             name) != AARCH64_SKIPPED_INTERPRETER_HANDLERS.end();
        }
        if (GetGraph()->GetMode().IsFastPath()) {
            return std::find(AARCH64_SKIPPED_FASTPATHS.begin(), AARCH64_SKIPPED_FASTPATHS.end(), name) !=
                   AARCH64_SKIPPED_FASTPATHS.end();
        }
        if (GetGraph()->GetMode().IsBoundary()) {
            return true;
        }
    }
    return true;
}
#endif

static bool RunIrtocInterpreterOptimizations(Graph *graph)
{
    compiler::OPTIONS.SetCompilerChecksElimination(false);
    // aantipina: re-enable Lse
    compiler::OPTIONS.SetCompilerLse(false);
#ifdef PANDA_COMPILER_TARGET_AARCH64
    compiler::OPTIONS.SetCompilerMemoryCoalescing(false);
#endif
    if (!compiler::OPTIONS.IsCompilerNonOptimizing()) {
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::BranchElimination>();
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::IfMerging>();
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Cse>();
        graph->RunPass<compiler::Licm>(compiler::OPTIONS.GetCompilerLicmHoistLimit());
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
        graph->RunPass<compiler::LoopUnroll>(compiler::OPTIONS.GetCompilerLoopUnrollInstLimit(),
                                             compiler::OPTIONS.GetCompilerLoopUnrollFactor());
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
        graph->RunPass<compiler::MemoryCoalescing>(compiler::OPTIONS.IsCompilerMemoryCoalescingAligned());
        graph->RunPass<compiler::IfConversion>(compiler::OPTIONS.GetCompilerIfConversionLimit());
        graph->RunPass<compiler::MoveConstants>();
    }

    graph->RunPass<compiler::Cleanup>();
    if (!compiler::RegAlloc(graph)) {
        return false;
    }

    if (!graph->RunPass<compiler::DanglingPointersChecker>()) {
        return false;
    }

    return graph->RunPass<compiler::CodegenInterpreter>();
}

static bool RunIrtocOptimizations(Graph *graph)
{
    if (!compiler::OPTIONS.IsCompilerNonOptimizing()) {
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::Cse>();
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Lowering>();
        graph->RunPass<compiler::CodeSink>();
        graph->RunPass<compiler::MemoryCoalescing>(compiler::OPTIONS.IsCompilerMemoryCoalescingAligned());
        graph->RunPass<compiler::IfConversion>(compiler::OPTIONS.GetCompilerIfConversionLimit());
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

}  // namespace panda::irtoc
