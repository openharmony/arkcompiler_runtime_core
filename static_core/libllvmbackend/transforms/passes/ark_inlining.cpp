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

#include "ark_inlining.h"
#include "llvm_compiler_options.h"

#include <llvm/Analysis/InlineAdvisor.h>
#include <llvm/Analysis/ReplayInlineAdvisor.h>

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Demangle/Demangle.h>

#include <vector>

namespace panda::llvmbackend::passes {

bool InlinePrepare::ShouldInsert(const panda::llvmbackend::LLVMCompilerOptions *options)
{
    return options->inlining;
}

InlinePrepare InlinePrepare::Create([[maybe_unused]] LLVMArkInterface *ark_interface,
                                    const panda::llvmbackend::LLVMCompilerOptions *options)
{
    static constexpr int INLINING_THRESHOLD = 500;
    auto inline_params = llvm::getInlineParams(INLINING_THRESHOLD);
    inline_params.AllowRecursiveCall = options->recursive_inlining;
    return InlinePrepare(inline_params);
}

llvm::PreservedAnalyses InlinePrepare::run(llvm::Module &module, llvm::ModuleAnalysisManager &module_am)
{
    auto &advisor_result = module_am.getResult<llvm::InlineAdvisorAnalysis>(module);
    if (!advisor_result.tryCreate(
            inline_params_, llvm::InliningAdvisorMode::Default, {},
            llvm::InlineContext {llvm::ThinOrFullLTOPhase::None, llvm::InlinePass::ModuleInliner})) {
        module.getContext().emitError("Could not setup Inlining Advisor for the requested mode and/or options");
    }
    return llvm::PreservedAnalyses::all();
}

bool IrtocInlineChecker::ShouldInsert(const panda::llvmbackend::LLVMCompilerOptions *options)
{
    return options->do_irtoc_inline;
}

void IrtocInlineChecker::CheckShouldInline(llvm::CallBase *call_base)
{
    using llvm::StringRef;
    if (!call_base->hasFnAttr(llvm::Attribute::AlwaysInline)) {
        return;
    }
    auto caller = call_base->getCaller();
    auto callee = call_base->getCalledFunction();
    std::string msg = "Unknown reason";
    if (call_base->hasFnAttr("inline-remark")) {
        msg = call_base->getFnAttr("inline-remark").getValueAsString();
    }

    auto dem_caller_name = llvm::demangle(std::string(caller->getName()));
    if (callee == nullptr) {
        llvm::report_fatal_error(llvm::Twine("Can't inline with alwaysinline attr  'nullptr") + "' into '" +
                                 dem_caller_name + " due to " + msg + "'");
        return;
    }
    auto dem_callee_name = llvm::demangle(std::string(callee->getName()));
#ifdef __SANITIZE_THREAD__
    // The functions from EXCLUSIONS are come from panda runtime (array-inl.h and class.h)
    // These function are recursive (Thay are optimized by tail recursive in normal way
    // but not if PANDA_ENABLE_THREAD_SANITIZER)
    static constexpr std::array EXCLUSIONS = {StringRef("panda::coretypes::Array::CreateMultiDimensionalArray"),
                                              StringRef("panda::Class::IsAssignableFrom(panda::Class const*)")};
    if (std::find_if(EXCLUSIONS.cbegin(), EXCLUSIONS.cend(), [dem_callee_name](StringRef pat) {
            return dem_callee_name.find(pat) != std::string::npos;
        }) == EXCLUSIONS.cend()) {
        llvm::report_fatal_error(llvm::Twine("Can't inline with alwaysinline attr '") + dem_callee_name + "' into '" +
                                 dem_caller_name + "' due to '" + msg + "'");
    }
#else
    llvm::report_fatal_error(llvm::Twine("Can't inline with alwaysinline attr '") + dem_callee_name + "' into '" +
                             dem_caller_name + "' due to '" + msg + "'");
#endif
}

llvm::PreservedAnalyses IrtocInlineChecker::run(llvm::LazyCallGraph::SCC &component,
                                                llvm::CGSCCAnalysisManager & /*unused*/,
                                                llvm::LazyCallGraph & /*unused*/, llvm::CGSCCUpdateResult & /*unused*/)
{
    for (const auto &node : component) {
        auto &func = node.getFunction();
        if (func.isDeclaration()) {
            continue;
        }

        for (llvm::Instruction &inst : llvm::instructions(func)) {
            auto *call_base = llvm::dyn_cast<llvm::CallBase>(&inst);
            if (call_base == nullptr || llvm::isa<llvm::IntrinsicInst>(&inst)) {
                continue;
            }
            llvm::Function *callee = call_base->getCalledFunction();
            if (callee == nullptr || callee->isDeclaration()) {
                continue;
            }
            CheckShouldInline(call_base);
        }
    }

    return llvm::PreservedAnalyses::all();
}

}  // namespace panda::llvmbackend::passes
