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

#include "frame_lowering.h"
#include "frame_builder.h"
#include "llvm_ark_interface.h"
#include "compiler/optimizer/code_generator/target_info.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/CodeGen/MachineFunctionPass.h>

#define DEBUG_TYPE "frame-builder"

using llvm::MachineFunction;
using llvm::MachineFunctionPass;
using llvm::MachineInstr;
using llvm::StringRef;

namespace {

class FrameLoweringPass : public MachineFunctionPass {
public:
    static constexpr StringRef PASS_NAME = "ARK-LLVM Frame builder";

    explicit FrameLoweringPass(ark::llvmbackend::LLVMArkInterface *arkInterface = nullptr)
        : MachineFunctionPass(id_), arkInterface_ {arkInterface}
    {
    }

    StringRef getPassName() const override
    {
        return PASS_NAME;
    }

    ssize_t GetConstantFromRuntime(FrameConstantDescriptor descr)
    {
        switch (descr) {
            case FrameConstantDescriptor::TLS_FRAME_OFFSET:
                return arkInterface_->GetTlsFrameKindOffset();
            case FrameConstantDescriptor::FRAME_FLAGS:
                return arkInterface_->GetCFrameHasFloatRegsFlagMask();
            default:
                llvm_unreachable("Undefined FrameConstantDescriptor in constant_pool_handler");
        }
    }

    template <typename FrameBuilderT>
    bool VisitMachineFunction(MachineFunction &mfunc)
    {
        // Collect information about current function
        FrameInfo frameInfo;
        frameInfo.regMasks = GetUsedRegs(mfunc);
        frameInfo.hasCalls = HasCalls(mfunc);
        frameInfo.stackSize = mfunc.getFrameInfo().getStackSize();
        frameInfo.soOffset = -arkInterface_->GetStackOverflowCheckOffset();
        if (arkInterface_->IsArm64()) {
            constexpr uint32_t SLOT_SIZE = 8U;
            ASSERT(frameInfo.stackSize >= SLOT_SIZE);
            frameInfo.stackSize -= 2U * SLOT_SIZE;

            [[maybe_unused]] constexpr uint32_t MAX_IMM_12 = 4096;
            ASSERT(frameInfo.stackSize < MAX_IMM_12);
        }
        FrameBuilderT frameBuilder(frameInfo,
                                   [this](FrameConstantDescriptor descr) { return GetConstantFromRuntime(descr); });

        // Remove generated prolog/epilog and insert custom one
        for (auto &bb : mfunc) {
            auto isPrologue = frameBuilder.RemovePrologue(bb);
            auto isEpilogue = frameBuilder.RemoveEpilogue(bb);
            if (isPrologue) {
                frameBuilder.InsertPrologue(bb);
            }
            if (isEpilogue) {
                frameBuilder.InsertEpilogue(bb);
            }
        }

        frameInfo = frameBuilder.GetFrameInfo();

        // Remember info for CodeInfoProducer
        auto &func = mfunc.getFunction();
        arkInterface_->PutMethodStackSize(&func, mfunc.getFrameInfo().getStackSize());
        arkInterface_->PutCalleeSavedRegistersMask(&func, frameInfo.regMasks);

        return true;
    }

    bool runOnMachineFunction(MachineFunction &mfunc) override
    {
        if (mfunc.getFunction().getMetadata("use-ark-frame") == nullptr) {
            return false;
        }
        if (arkInterface_->IsArm64()) {
            return VisitMachineFunction<ARM64FrameBuilder>(mfunc);
        }
        return VisitMachineFunction<AMD64FrameBuilder>(mfunc);
    }

    static void FillOperand(const llvm::MCRegisterInfo *regInfo, const llvm::MachineOperand &operand,
                            std::function<void(unsigned)> &fillMask, bool isArm)
    {
        if (!operand.isReg()) {
            return;
        }
        llvm::MCRegister reg = operand.getReg().asMCReg();
        int dwarfId = regInfo->getDwarfRegNum(reg, false);
        if (dwarfId < 0 && !isArm) {
            // Check super regs as X86 dwarf info in LLVM is weird
            for (llvm::MCSuperRegIterator sreg(reg, regInfo); sreg.isValid(); ++sreg) {
                dwarfId = regInfo->getDwarfRegNum(*sreg, false);
                if (dwarfId >= 0) {
                    break;
                }
            }
        }
        if (dwarfId >= 0) {
            fillMask(dwarfId);
        }
    }

    // first -- X-regs, second -- V-regs
    FrameInfo::RegMasks GetUsedRegs(const MachineFunction &mfunc) const
    {
        FrameInfo::RegMasks masks {};

        std::function<void(unsigned)> fillMaskArm = [&masks](unsigned idx) {
            // Dwarf numbers from llvm/lib/Target/AArch64/AArch64RegisterInfo.td
            constexpr unsigned X0_IDX = 0;
            constexpr unsigned D0_IDX = 64;
            if (idx >= D0_IDX) {
                std::get<1>(masks) |= 1U << (idx - D0_IDX);
            } else {
                std::get<0>(masks) |= 1U << (idx - X0_IDX);
            }
        };

        std::function<void(unsigned)> fillMaskAmd = [&masks](unsigned idx) {
            // Dwarf numbers from llvm/lib/Target/X86/X86RegisterInfo.td
            constexpr unsigned RAX_IDX = 0;
            constexpr unsigned XMM0_IDX = 17;
            constexpr unsigned XMM16_IDX = 67;
            constexpr unsigned XMM_COUNT = 16U;
            if ((idx >= XMM0_IDX && idx < XMM0_IDX + XMM_COUNT) || (idx >= XMM16_IDX && idx < XMM16_IDX + XMM_COUNT)) {
                std::get<1>(masks) |= 1U << (idx - XMM0_IDX);
            } else {
                idx = ark::compiler::ConvertRegNumberX86(idx);
                std::get<0>(masks) |= 1U << (idx - RAX_IDX);
            }
        };

        bool isArm = arkInterface_->IsArm64();
        std::function<void(unsigned)> fillMask = isArm ? fillMaskArm : fillMaskAmd;

        const llvm::MCRegisterInfo *regInfo = mfunc.getSubtarget().getRegisterInfo();
        for (auto &mblock : mfunc) {
            for (auto &minst : mblock) {
                bool isFrameSetup = minst.getFlag(MachineInstr::FrameSetup);
                bool isFrameDestroy = minst.getFlag(MachineInstr::FrameDestroy);
                if (isFrameSetup || isFrameDestroy) {
                    continue;
                }
                for (auto &operand : minst.operands()) {
                    FillOperand(regInfo, operand, fillMask, isArm);
                }
            }
        }
        return masks;
    }

    bool HasCalls(const MachineFunction &mfunc)
    {
        for (auto &mblock : mfunc) {
            for (auto &minst : mblock) {
                bool isFrameSetup = minst.getFlag(MachineInstr::FrameSetup);
                bool isFrameDestroy = minst.getFlag(MachineInstr::FrameDestroy);
                if (isFrameSetup || isFrameDestroy) {
                    continue;
                }
                auto desc = minst.getDesc();
                if (!desc.isCall() || desc.isReturn()) {
                    continue;
                }
                return true;
            }
        }
        return false;
    }

    static char id_;  // NOLINT(readability-identifier-naming)
private:
    ark::llvmbackend::LLVMArkInterface *arkInterface_ {nullptr};
};
char FrameLoweringPass::id_ = 0;  // NOLINT(readability-identifier-naming)

}  // namespace

MachineFunctionPass *ark::llvmbackend::CreateFrameLoweringPass(ark::llvmbackend::LLVMArkInterface *arkInterface)
{
    return new FrameLoweringPass(arkInterface);
}
