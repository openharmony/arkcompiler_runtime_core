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

#include "llvm_ark_interface.h"
#include "patch_return_handler_stack_adjustment.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/CodeGen/GlobalISel/MachineIRBuilder.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/Debug.h>

#include "transforms/transform_utils.h"

using llvm::MachineFunction;
using llvm::MachineFunctionPass;
using llvm::RegisterPass;
using llvm::report_fatal_error;
using llvm::StringRef;
using panda::llvmbackend::LLVMArkInterface;

#define DEBUG_TYPE "patch-return-handler-stack-adjustment"

namespace {

/**
 * Patch stack adjustment value in return handler
 *
 * We generate inline assembly with hardcoded constant for return handlers in LLVMEntry::EmitInterpreterReturn
 *
 * The inline assembly uses stack pointer and inserts own return. Examples of inline assemblies:
 *
 * 1. leaq  $0, %rsp - x86. We add a hardcoded value to %rsp and retq then
 * 2. add  sp, sp, $0 - aarch64. We add hardcoded value to sp and ret then
 *
 * LLVM does not know about our rets
 *
 * We use stack pointer in inline assemblies assuming that llvm does not touch sp itself.
 * For example, we assume that llvm does not spill any register value onto the stack
 * But llvm can do it, example: 'sub    $0x10,%rsp' in function prologue.
 * LLVM will insert corresponding 'add    $0x10,%rsp' before its own rets but not for ours.
 *
 * So we add the stack size of machine function to our "hardcoded value" in inline assemblies.
 * To find such assemblies the pass looks for a comment in the inline assembly template -
 * LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT
 */
class PatchReturnHandlerStackAdjustment : public MachineFunctionPass {
public:
    explicit PatchReturnHandlerStackAdjustment(LLVMArkInterface *ark_interface = nullptr)
        : MachineFunctionPass(ID), ark_interface_(ark_interface)
    {
    }

    bool runOnMachineFunction(MachineFunction &machine_function) override
    {
        ASSERT(ark_interface_ != nullptr);
        if (!ark_interface_->IsIrtocReturnHandler(machine_function.getFunction())) {
            return false;
        }

        auto &frame_info = machine_function.getFrameInfo();
        if (frame_info.hasVarSizedObjects()) {
            report_fatal_error(StringRef("Return handler '") + machine_function.getName() + "' uses var sized objects");
            return false;
        }
        auto stack_size = frame_info.getStackSize();
        if (stack_size == 0) {
            return false;
        }

        bool changed = false;
        for (auto &basic_block : machine_function) {
            for (auto &instruction : basic_block) {
                if (!instruction.isInlineAsm()) {
                    continue;
                }
                static constexpr unsigned INLINE_ASM_INDEX = 0;
                static constexpr unsigned STACK_ADJUSTMENT_INDEX = 3;

                std::string_view inline_asm {instruction.getOperand(INLINE_ASM_INDEX).getSymbolName()};
                if (inline_asm.find(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT) != std::string::npos) {
                    auto &stack_adjustment = instruction.getOperand(STACK_ADJUSTMENT_INDEX);
                    ASSERT(stack_adjustment.isImm());
                    auto old_stack_size = stack_adjustment.getImm();
                    auto new_stack_size = old_stack_size + stack_size;
                    LLVM_DEBUG(llvm::dbgs()
                               << "Replaced old_stack_size = " << old_stack_size
                               << " with new_stack_size = " << new_stack_size << " in inline_asm = '" << inline_asm
                               << "' because llvm used " << stack_size << " bytes of stack in function = '"
                               << machine_function.getName() << "'\n");
                    stack_adjustment.setImm(new_stack_size);
                    changed = true;
                }
            }
        }

        return changed;
    }

    StringRef getPassName() const override
    {
        return PASS_NAME;
    }

    static inline char ID = 0;  // NOLINT(readability-identifier-naming)
    static constexpr StringRef PASS_NAME = "ARK-LLVM patch stack adjustment";
    static constexpr StringRef ARG_NAME = "patch-return-handler-stack-adjustment";

private:
    LLVMArkInterface *ark_interface_;
};

}  // namespace

namespace panda::llvmbackend {

MachineFunctionPass *CreatePatchReturnHandlerStackAdjustmentPass(LLVMArkInterface *ark_interface)
{
    return new PatchReturnHandlerStackAdjustment(ark_interface);
}

}  // namespace panda::llvmbackend

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RegisterPass<PatchReturnHandlerStackAdjustment> P1(PatchReturnHandlerStackAdjustment::ARG_NAME,
                                                          PatchReturnHandlerStackAdjustment::PASS_NAME, false, false);
