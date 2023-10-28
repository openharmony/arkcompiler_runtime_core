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

#include "expand_atomics.h"

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Instructions.h>

#include "llvm_ark_interface.h"

#define DEBUG_TYPE "expand-atomics"

namespace panda::llvmbackend::passes {

ExpandAtomics::ExpandAtomics() = default;

llvm::PreservedAnalyses ExpandAtomics::run(llvm::Function &function,
                                           [[maybe_unused]] llvm::FunctionAnalysisManager &analysis_manager)
{
    bool changed = false;

    if (llvm::Triple {function.getParent()->getTargetTriple()}.getArch() != llvm::Triple::x86_64) {
        return llvm::PreservedAnalyses::all();
    }

    // Cast 32-bit pointers to 0 address space to support atomic operations on X86
    for (auto &basic_block : function) {
        llvm::SmallVector<llvm::Instruction *> instructions;
        for (auto &instruction : basic_block) {
            if (instruction.isAtomic()) {
                instructions.push_back(&instruction);
            }
        }

        for (auto instruction : instructions) {
            bool c = InsertAddrSpaceCast(instruction);
            changed |= c;
        }
    }

    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

bool ExpandAtomics::InsertAddrSpaceCast(llvm::Instruction *atomic_instruction)
{
    assert(atomic_instruction->isAtomic());
    if (llvm::isa<llvm::FenceInst>(atomic_instruction)) {
        // Fences do not have pointer operands
        return false;
    }
    unsigned pointer_index = 0;
    if (llvm::isa<llvm::StoreInst>(atomic_instruction)) {
        pointer_index = 1U;
    }
    auto pointer = atomic_instruction->getOperand(pointer_index);
    assert(pointer->getType()->isPointerTy());
    if (pointer->getType()->getPointerAddressSpace() != LLVMArkInterface::GC_ADDR_SPACE) {
        return false;
    }

    LLVM_DEBUG(llvm::dbgs() << "Inserting addrspacecast for '");
    LLVM_DEBUG(atomic_instruction->print(llvm::dbgs()));
    LLVM_DEBUG(llvm::dbgs() << "'\n");

    auto cast = llvm::CastInst::Create(llvm::CastInst::AddrSpaceCast, pointer,
                                       llvm::PointerType::get(atomic_instruction->getContext(), 0));
    cast->insertBefore(atomic_instruction);
    atomic_instruction->setOperand(pointer_index, cast);

    LLVM_DEBUG(llvm::dbgs() << "Result: cast = '");
    LLVM_DEBUG(cast->print(llvm::dbgs()));
    LLVM_DEBUG(llvm::dbgs() << "', atomic_instruction = '");
    LLVM_DEBUG(atomic_instruction->print(llvm::dbgs()));
    LLVM_DEBUG(llvm::dbgs() << "'\n");
    return true;
}

}  // namespace panda::llvmbackend::passes
