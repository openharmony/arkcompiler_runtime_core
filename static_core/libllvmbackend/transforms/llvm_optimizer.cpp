/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "llvm_optimizer.h"

#include "passes/ark_inlining.h"
#include "passes/ark_speculation.h"
#include "passes/expand_atomics.h"

#include "passes/inline_ir/cleanup_inline_module.h"
#include "passes/inline_ir/discard_inline_module.h"
#include "passes/inline_ir/mark_always_inline.h"
#include "passes/inline_ir/mark_inline_module.h"
#include "passes/inline_ir/remove_unused_functions.h"

#include "llvm_ark_interface.h"

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/GlobalsModRef.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/ProfileSummaryInfo.h>
#include <llvm/Transforms/Utils/CanonicalizeAliases.h>
#include <llvm/Transforms/Utils/NameAnonGlobals.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Target/TargetMachine.h>

#include <fstream>
#include <streambuf>
#include <type_traits>

#include "transforms/transform_utils.h"

namespace {

template <typename PassManagerT, typename PassT>
void AddPassIf(PassManagerT &pass_manager, PassT &&pass, bool need_insert = true)
{
    if (!need_insert) {
        return;
    }
    pass_manager.addPass(std::forward<PassT>(pass));
#ifndef NDEBUG
    // VerifierPass can be insterted only in ModulePassManager or FunctionPassManager
    constexpr auto IS_MODULE_PM = std::is_same_v<llvm::ModulePassManager, PassManagerT>;
    constexpr auto IS_FUNCTION_PM = std::is_same_v<llvm::FunctionPassManager, PassManagerT>;
    // Disable checks due to clang-tidy bug https://bugs.llvm.org/show_bug.cgi?id=32203
    // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
    if constexpr (IS_MODULE_PM || IS_FUNCTION_PM) {  // NOLINT(bugprone-suspicious-semicolon)
        pass_manager.addPass(llvm::VerifierPass());
    }
#endif
}

std::string PreprocessPipelineFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        llvm::report_fatal_error(llvm::Twine("Cant open pipeline file: `") + filename + "`", false);
    }
    std::string raw_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Remove comments
    size_t insert_pos = 0;
    size_t copy_pos = 0;
    constexpr auto COMMENT_SYMBOL = '#';
    constexpr auto ENDLINE_SYMBOL = '\n';
    while (copy_pos < raw_data.size()) {
        while (copy_pos < raw_data.size() && raw_data[copy_pos] != COMMENT_SYMBOL) {
            raw_data[insert_pos++] = raw_data[copy_pos++];
        }
        while (copy_pos < raw_data.size() && raw_data[copy_pos] != ENDLINE_SYMBOL) {
            copy_pos++;
        }
    }
    raw_data.resize(insert_pos);

    // Remove space symbols
    auto predicate = [](char sym) { return std::isspace(sym); };
    raw_data.erase(std::remove_if(raw_data.begin(), raw_data.end(), predicate), raw_data.end());

    return raw_data;
}

#include <pipeline_irtoc_gen.inc>

std::string GetOptimizationPipeline(const std::string &filename)
{
    std::string pipeline;
    if (!filename.empty()) {
        pipeline = PreprocessPipelineFile(filename);
    } else {
        // PIPELINE_IRTOC variable is defined in pipeline_irtoc_gen.inc
        pipeline = std::string {PIPELINE_IRTOC};
    }
    return pipeline;
}

}  // namespace

#include <llvm_passes.inl>

namespace panda::llvmbackend {

LLVMOptimizer::LLVMOptimizer(panda::llvmbackend::LLVMCompilerOptions options, LLVMArkInterface *ark_interface,
                             std::shared_ptr<llvm::TargetMachine> target_machine)
    : options_(std::move(options)), ark_interface_(ark_interface), target_machine_(std::move(target_machine))
{
}

void LLVMOptimizer::ProcessInlineModule(llvm::Module *inline_module) const
{
    namespace pass = panda::llvmbackend::passes;
    llvm::ModulePassManager module_pm;
    llvm::LoopAnalysisManager loop_am;
    llvm::FunctionAnalysisManager function_am;
    llvm::CGSCCAnalysisManager cgscc_am;
    llvm::ModuleAnalysisManager module_am;

    llvm::PassBuilder pass_builder(target_machine_.get());
    pass_builder.registerModuleAnalyses(module_am);
    pass_builder.registerCGSCCAnalyses(cgscc_am);
    pass_builder.registerFunctionAnalyses(function_am);
    pass_builder.registerLoopAnalyses(loop_am);
    pass_builder.crossRegisterProxies(loop_am, function_am, cgscc_am, module_am);

    AddPassIf(module_pm, llvm::CanonicalizeAliasesPass(), true);
    AddPassIf(module_pm, llvm::NameAnonGlobalPass(), true);
    AddPassIf(module_pm, pass::MarkInlineModule(), true);
    AddPassIf(module_pm, pass::CleanupInlineModule(), true);

    module_pm.run(*inline_module, module_am);
}

void LLVMOptimizer::OptimizeModule(llvm::Module *module) const
{
    ASSERT(ark_interface_ != nullptr);

    if (options_.dump_module_before_optimizations) {
        llvm::errs() << "; =========================================\n";
        llvm::errs() << "; LLVM IR module BEFORE LLVM optimizations:\n";
        llvm::errs() << "; =========================================\n";
        llvm::errs() << *module << '\n';
    }

    DoOptimizeModule(module);

    if (options_.dump_module_after_optimizations) {
        llvm::errs() << "; =========================================\n";
        llvm::errs() << "; LLVM IR module AFTER LLVM optimizations: \n";
        llvm::errs() << "; =========================================\n";
        llvm::errs() << *module << '\n';
    }
}

void LLVMOptimizer::DoOptimizeModule(llvm::Module *module) const
{
    // Create the analysis managers.
    llvm::LoopAnalysisManager loop_am;
    llvm::FunctionAnalysisManager function_am;
    llvm::CGSCCAnalysisManager cgscc_am;
    llvm::ModuleAnalysisManager module_am;

    llvm::StandardInstrumentations instrumentation(false);
    llvm::PassInstrumentationCallbacks pass_instrumentation;
    instrumentation.registerCallbacks(pass_instrumentation);
    panda::libllvmbackend::RegisterPasses(pass_instrumentation);

    llvm::PassBuilder pass_builder(target_machine_.get(), llvm::PipelineTuningOptions(), llvm::None,
                                   &pass_instrumentation);

    // Register the AA manager first so that our version is the one used.
    function_am.registerPass([&] { return pass_builder.buildDefaultAAPipeline(); });
    // Register the target library analysis directly.
    function_am.registerPass(
        [&] { return llvm::TargetLibraryAnalysis(llvm::TargetLibraryInfoImpl(target_machine_->getTargetTriple())); });
    // Register all the basic analyses with the managers.
    pass_builder.registerModuleAnalyses(module_am);
    pass_builder.registerCGSCCAnalyses(cgscc_am);
    pass_builder.registerFunctionAnalyses(function_am);
    pass_builder.registerLoopAnalyses(loop_am);
    pass_builder.crossRegisterProxies(loop_am, function_am, cgscc_am, module_am);

    panda::libllvmbackend::PassParser pass_parser(ark_interface_);
    pass_parser.RegisterParserCallbacks(pass_builder, options_);

    llvm::ModulePassManager module_pm;
    if (options_.optimize) {
        cantFail(pass_builder.parsePassPipeline(module_pm, GetOptimizationPipeline(options_.pipeline_file)));
    } else {
        namespace pass = panda::llvmbackend::passes;
        llvm::FunctionPassManager function_pm;
        AddPassIf(function_pm, pass::ExpandAtomics());
        module_pm.addPass(createModuleToFunctionPassAdaptor(std::move(function_pm)));
    }
    module_pm.run(*module, module_am);
}

}  // namespace panda::llvmbackend
