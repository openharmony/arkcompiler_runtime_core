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

#include "transforms/passes/mem_barriers.h"
#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"
#include "transforms/transform_utils.h"

#include <llvm/IR/IRBuilder.h>

using llvm::BasicBlock;
using llvm::CallInst;
using llvm::Function;
using llvm::FunctionAnalysisManager;
using llvm::Instruction;

namespace ark::llvmbackend::passes {

MemBarriers MemBarriers::Create([[maybe_unused]] LLVMArkInterface *arkInterface,
                                const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return MemBarriers(options->optimize);
}

MemBarriers::MemBarriers(bool optimize) : optimize_ {optimize} {}

llvm::PreservedAnalyses MemBarriers::run(Function &function, FunctionAnalysisManager & /*analysisManager*/)
{
    bool changed = false;
    for (BasicBlock &block : function) {
        llvm::SmallVector<llvm::Instruction *> needsBarrier;
        bool keepFence = true;
        for (auto &inst : llvm::reverse(block)) {
            auto callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
            if (callInst == nullptr || !callInst->hasFnAttr("needs-mem-barrier")) {
                keepFence |= inst.mayWriteToMemory();
                continue;
            }
            // Process 'callInst' with attribute "needs-mem-barrier"
            if (keepFence) {
                needsBarrier.push_back(callInst->getNextNode());
                keepFence = !optimize_;
            }
        }
        for (auto inst : needsBarrier) {
            auto builder = llvm::IRBuilder<>(inst);
            builder.CreateFence(llvm::AtomicOrdering::Release);
            changed = true;
        }
    }
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
