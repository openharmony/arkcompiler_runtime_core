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

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IntrinsicsAArch64.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using llvm::BasicBlock;
using llvm::CallInst;
using llvm::Function;
using llvm::FunctionAnalysisManager;
using llvm::Instruction;

namespace ark::llvmbackend::passes {

MemBarriers MemBarriers::Create([[maybe_unused]] LLVMArkInterface *arkInterface,
                                const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return MemBarriers(arkInterface, options->optimize);
}

MemBarriers::MemBarriers(LLVMArkInterface *arkInterface, bool optimize)
    : arkInterface_ {arkInterface}, optimize_ {optimize}
{
}

llvm::PreservedAnalyses MemBarriers::run(Function &function, FunctionAnalysisManager & /*analysisManager*/)
{
    bool changed = false;
    for (BasicBlock &block : function) {
        llvm::SmallVector<llvm::Instruction *> needsBarrier;
        llvm::SmallVector<llvm::Instruction *> mergeSet;
        for (auto &inst : block) {
            auto callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
            if (callInst != nullptr && callInst->hasFnAttr("needs-mem-barrier")) {
                mergeSet.push_back(callInst);
                continue;
            }
            if (optimize_ && inst.mayWriteToMemory() && GrabsGuarded(&inst, mergeSet)) {
                MergeBarriers(mergeSet, needsBarrier);
            }
        }

        MergeBarriers(mergeSet, needsBarrier);
        for (auto inst : needsBarrier) {
            auto builder = llvm::IRBuilder<>(inst->getNextNode());
            builder.CreateFence(llvm::AtomicOrdering::Release);
            changed = true;
        }
    }
    changed |= RelaxBarriers(function);

    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

bool MemBarriers::GrabsGuarded(llvm::Instruction *inst, llvm::SmallVector<llvm::Instruction *> &mergeSet)
{
    llvm::SmallVector<llvm::Value *> inputs;
    if (auto storeInst = llvm::dyn_cast<llvm::StoreInst>(inst)) {
        inputs.push_back(storeInst->getValueOperand());
    } else if (auto callInst = llvm::dyn_cast<llvm::CallInst>(inst)) {
        inputs.append(callInst->arg_begin(), callInst->arg_end());
    } else {
        inputs.append(inst->value_op_begin(), inst->value_op_end());
    }
    for (auto input : inputs) {
        auto inputInst = llvm::dyn_cast<llvm::Instruction>(input);
        if (inputInst != nullptr && std::find(mergeSet.begin(), mergeSet.end(), inputInst) != mergeSet.end()) {
            return true;
        }
    }
    return false;
}

void MemBarriers::MergeBarriers(llvm::SmallVector<llvm::Instruction *> &mergeSet,
                                llvm::SmallVector<llvm::Instruction *> &needsBarrier)
{
    if (mergeSet.empty()) {
        return;
    }
    if (optimize_) {
        needsBarrier.push_back(mergeSet.back());
    } else {
        needsBarrier.append(mergeSet);
    }
    mergeSet.clear();
}

bool MemBarriers::RelaxBarriers(llvm::Function &function)
{
    if (!arkInterface_->IsArm64() || !optimize_) {
        return false;
    }

    auto opcode = llvm::Intrinsic::AARCH64Intrinsics::aarch64_dmb;
    auto dmb = llvm::Intrinsic::getDeclaration(function.getParent(), opcode, {});
    static constexpr uint32_t ISHST = 10U;
    auto ishst = llvm::ConstantInt::get(llvm::Type::getInt32Ty(function.getContext()), ISHST);

    bool changed = false;
    for (auto &basicBlock : function) {
        llvm::SmallVector<llvm::FenceInst *> fences;
        for (auto &instruction : basicBlock) {
            auto fence = llvm::dyn_cast<llvm::FenceInst>(&instruction);
            if (fence != nullptr && fence->getOrdering() == llvm::AtomicOrdering::Release) {
                fences.push_back(fence);
            }
        }
        for (auto fence : fences) {
            auto upgraded = llvm::CallInst::Create(dmb, {ishst}, llvm::None);
            llvm::ReplaceInstWithInst(fence, upgraded);
        }
        changed |= !fences.empty();
    }
    return changed;
}

}  // namespace ark::llvmbackend::passes
