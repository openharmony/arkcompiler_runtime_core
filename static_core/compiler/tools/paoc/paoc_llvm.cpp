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

#include "paoc_llvm.h"

#include "aot/aot_builder/llvm_aot_builder.h"

#include "optimizer/analysis/monitor_analysis.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_PAOC(level) LOG(level, COMPILER) << "PAOC: "

namespace panda::paoc {

using namespace panda::compiler;  // NOLINT(google-build-using-namespace)

void PaocLLVM::Clear(panda::mem::InternalAllocatorPtr allocator)
{
    llvm_aot_compiler_ = nullptr;
    Paoc::Clear(allocator);
}

Paoc::LLVMCompilerStatus PaocLLVM::TryLLVM(CompilingContext *ctx)
{
    // Check monitors balance
    ctx->graph->GetAnalysis<MonitorAnalysis>().SetCheckNonCatchOnly(true);
    bool monitors_correct = ctx->graph->RunPass<MonitorAnalysis>();
    ctx->graph->InvalidateAnalysis<MonitorAnalysis>();
    ctx->graph->GetAnalysis<MonitorAnalysis>().SetCheckNonCatchOnly(false);

    auto can = llvm_aot_compiler_->CanCompile(ctx->graph);
    if (can.HasValue()) {
        bool can_compile = can.Value() && monitors_correct;

        // In normal workflow LLVM is not chosen only when monitors are unbalanced
        // In safe mode - LLVM is used only for CanCompile graphs
        bool use_llvm = llvmbackend::OPTIONS.IsLlvmSafemode() ? can_compile : monitors_correct;
        if (use_llvm) {
            if (AddGraphToLLVM(ctx)) {
                return LLVMCompilerStatus::COMPILED;
            }
            // No fallback for case "LLVM was chosen, but add graph failed"
            return LLVMCompilerStatus::SKIP;
        }
        ASSERT(!can_compile);
    } else if (!llvmbackend::OPTIONS.IsLlvmAllowBreakage() || !ShouldIgnoreFailures()) {
        LOG_PAOC(ERROR) << can.Error() << "\n";
        return LLVMCompilerStatus::ERROR;
    }

    // Check if fallback allowed
    if (llvmbackend::OPTIONS.IsLlvmFallback()) {
        return LLVMCompilerStatus::FALLBACK;
    }
    return LLVMCompilerStatus::SKIP;
}

bool PaocLLVM::EndLLVM()
{
    ASSERT(IsLLVMAotMode());
    llvm_aot_compiler_->CompileAll();
    if (!ShouldIgnoreFailures() && llvm_aot_compiler_->IsIrFailed()) {
        return false;
    }
    if (!llvmbackend::OPTIONS.IsLlvmFallback() && !llvm_aot_compiler_->HasCompiledCode()) {
        return false;
    }
    return true;
}

void PaocLLVM::PrepareLLVM(const panda::Span<const char *> &args)
{
    ASSERT(IsLLVMAotMode());
    std::string cmdline;
    for (auto arg : args) {
        cmdline += arg;
        cmdline += " ";
    }

    auto output_file = GetPaocOptions()->GetPaocOutput();
    if (GetPaocOptions()->WasSetPaocBootOutput()) {
        output_file = GetPaocOptions()->GetPaocBootOutput();
    }
    llvm_aot_compiler_ =
        llvmbackend::CreateLLVMAotCompiler(GetRuntime(), GetCodeAllocator(), GetAotBuilder(), cmdline, output_file);
}

bool PaocLLVM::AddGraphToLLVM(CompilingContext *ctx)
{
    if (llvm_aot_compiler_->AddGraph(ctx->graph)) {
        GetAotBuilder()->AddMethodHeader(ctx->method, ctx->index);
        return true;
    }
    if (!llvmbackend::OPTIONS.IsLlvmAllowBreakage() || !ShouldIgnoreFailures()) {
        LOG_PAOC(FATAL) << "LLVM function creation was broken!\n";
    }
    return false;
}

void PaocLLVM::ValidateExtraOptions()
{
    auto llvm_options_err = panda::llvmbackend::OPTIONS.Validate();
#ifdef NDEBUG
    if (!llvmbackend::OPTIONS.GetLlvmBreakIrRegex().empty()) {
        LOG_PAOC(FATAL) << "--llvm-break-ir-regex is available only in debug builds";
    }
#endif
    if (llvm_options_err) {
        LOG_PAOC(FATAL) << llvm_options_err.value().GetMessage();
    }
}

std::unique_ptr<compiler::AotBuilder> PaocLLVM::CreateAotBuilder()
{
    return std::make_unique<LLVMAotBuilder>();
}

compiler::LLVMAotBuilder *PaocLLVM::GetAotBuilder()
{
    return static_cast<LLVMAotBuilder *>(aot_builder_.get());
}
}  // namespace panda::paoc

#undef LOG_PAOC
