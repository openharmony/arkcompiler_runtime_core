/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "loop_peeling.h"

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/MemoryBuiltins.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Operator.h>
#include <llvm/Support/KnownBits.h>
#include <llvm/Transforms/Utils/LoopPeel.h>

#include "transforms/builtins.h"
#include "transforms/transform_utils.h"

#define DEBUG_TYPE "ark-loop-peeling"

// Peel only loops with GVNable builtins
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_gvnOnly("ark-loop-peeling-gvn-only", llvm::cl::Hidden, llvm::cl::init(false));

namespace ark::llvmbackend::passes {

llvm::PreservedAnalyses ArkLoopPeeling::run(llvm::Loop &loop,
                                            [[maybe_unused]] llvm::LoopAnalysisManager &analysisManager,
                                            llvm::LoopStandardAnalysisResults &loopStandardAnalysisResults,
                                            [[maybe_unused]] llvm::LPMUpdater &lpmUpdater)
{
    if (llvm::canPeel(&loop)) {
        if (!g_gvnOnly || ContainsGvnBuiltin(&loop)) {
            [[maybe_unused]] bool result =
                llvm::peelLoop(&loop, 1U, &loopStandardAnalysisResults.LI, &loopStandardAnalysisResults.SE,
                               loopStandardAnalysisResults.DT, nullptr, true);
            // Always returns true, false is unexpected
            ASSERT(result);
            return llvm::PreservedAnalyses::none();
        }
    }
    return llvm::PreservedAnalyses::all();
}

bool ArkLoopPeeling::ContainsGvnBuiltin(llvm::Loop *loop)
{
    ASSERT(loop->getHeader() != nullptr);
    auto module = loop->getHeader()->getModule();
    std::array builtins = {llvmbackend::builtins::LoadClass(module), llvmbackend::builtins::LoadInitClass(module),
                           llvmbackend::builtins::LoadString(module), llvmbackend::builtins::ResolveVirtual(module)};

    for (auto &block : loop->blocks()) {
        for (auto &inst : *block) {
            auto call = llvm::dyn_cast<llvm::CallInst>(&inst);
            if (call == nullptr) {
                continue;
            }
            auto function = call->getCalledFunction();
            if (std::find(builtins.cbegin(), builtins.cend(), function) != builtins.cend()) {
                return true;
            }
        }
    }
    return false;
}

bool ArkLoopPeeling::ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return true;
}
}  // namespace ark::llvmbackend::passes
