/**
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

#include "codegen_fastpath.h"
#include "relocations.h"

namespace panda::compiler {

static void SaveCallerRegistersInFrame(RegMask mask, Encoder *encoder, const CFrameLayout &fl, bool is_fp)
{
    if (mask.none()) {
        return;
    }
    auto fp_reg = Target(fl.GetArch()).GetFrameReg();

    mask &= GetCallerRegsMask(fl.GetArch(), is_fp);
    auto start_slot = fl.GetStackStartSlot() + fl.GetCallerLastSlot(is_fp);
    encoder->SaveRegisters(mask, is_fp, -start_slot, fp_reg, GetCallerRegsMask(fl.GetArch(), is_fp));
}

static void RestoreCallerRegistersFromFrame(RegMask mask, Encoder *encoder, const CFrameLayout &fl, bool is_fp)
{
    if (mask.none()) {
        return;
    }
    auto fp_reg = Target(fl.GetArch()).GetFrameReg();

    mask &= GetCallerRegsMask(fl.GetArch(), is_fp);
    auto start_slot = fl.GetStackStartSlot() + fl.GetCallerLastSlot(is_fp);
    encoder->LoadRegisters(mask, is_fp, -start_slot, fp_reg, GetCallerRegsMask(fl.GetArch(), is_fp));
}

void CodegenFastPath::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "FastPath Prologue");

    auto caller_regs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto args_num = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    caller_regs &= GetUsedRegs() & ~GetTarget().GetParamRegsMask(args_num);
    SaveCallerRegistersInFrame(caller_regs, GetEncoder(), GetFrameLayout(), false);

    saved_registers_ = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    GetEncoder()->PushRegisters(saved_registers_, false, GetTarget().SupportLinkReg());

    if (GetUsedVRegs().Any()) {
        SaveCallerRegistersInFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), GetEncoder(), GetFrameLayout(),
                                   true);
        saved_fp_registers_ = GetUsedVRegs() & VRegMask(GetCalleeRegsMask(GetArch(), true));
        GetEncoder()->PushRegisters(saved_fp_registers_, true, GetTarget().SupportLinkReg());
    }

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        GetEncoder()->EncodeSub(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }
}

RegMask CodegenFastPath::GetCallerRegistersToRestore() const
{
    RegMask caller_regs = GetUsedRegs() & RegMask(GetCallerRegsMask(GetArch(), false));

    auto args_num = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    caller_regs &= ~GetTarget().GetParamRegsMask(args_num);

    if (auto ret_type {GetRuntime()->GetMethodReturnType(GetGraph()->GetMethod())}; ret_type != DataType::VOID) {
        ASSERT(!DataType::IsFloatType(ret_type));
        caller_regs.reset(GetTarget().GetReturnRegId());
    }
    return caller_regs;
}

void CodegenFastPath::GenerateEpilogue()
{
    SCOPED_DISASM_STR(this, "FastPath Epilogue");

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        GetEncoder()->EncodeAdd(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }

    RestoreCallerRegistersFromFrame(GetCallerRegistersToRestore(), GetEncoder(), GetFrameLayout(), false);

    if (GetUsedVRegs().Any()) {
        RestoreCallerRegistersFromFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), GetEncoder(),
                                        GetFrameLayout(), true);
        GetEncoder()->PopRegisters(saved_fp_registers_, true, GetTarget().SupportLinkReg());
    }

    GetEncoder()->PopRegisters(saved_registers_, false, GetTarget().SupportLinkReg());

    GetEncoder()->EncodeReturn();
}

void CodegenFastPath::CreateFrameInfo()
{
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(false) |
        FrameInfo::CallersRelativeFp::Encode(true) | FrameInfo::CalleesRelativeFp::Encode(false));
    frame->SetSpillsCount(GetGraph()->GetStackSlotsCount());
    CFrameLayout fl(GetGraph()->GetArch(), GetGraph()->GetStackSlotsCount());

    frame->SetCallersOffset(
        fl.GetOffset<CFrameLayout::SP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
    frame->SetFpCallersOffset(
        fl.GetOffset<CFrameLayout::SP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
    frame->SetCalleesOffset(
        -fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
    frame->SetFpCalleesOffset(
        -fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

    SetFrameInfo(frame);
}

void CodegenFastPath::IntrinsicSlowPathEntry(IntrinsicInst *inst)
{
    auto encoder = GetEncoder();

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        encoder->EncodeAdd(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }

    /* Once we reach the slow path, we can release all temp registers, since slow path terminates execution */
    auto temps_mask = GetTarget().GetTempRegsMask();
    for (size_t reg = temps_mask.GetMinRegister(); reg <= temps_mask.GetMaxRegister(); reg++) {
        if (temps_mask.Test(reg)) {
            GetEncoder()->ReleaseScratchRegister(Reg(reg, INT32_TYPE));
        }
    }

    RegMask caller_regs = ~GetUsedRegs() & RegMask(GetCallerRegsMask(GetArch(), false));
    auto args_num = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    caller_regs &= ~GetTarget().GetParamRegsMask(args_num);

    if (GetUsedVRegs().Any()) {
        VRegMask fp_caller_regs = ~GetUsedVRegs() & RegMask(GetCallerRegsMask(GetArch(), true));
        SaveCallerRegistersInFrame(fp_caller_regs, encoder, GetFrameLayout(), true);
        encoder->PopRegisters(saved_fp_registers_, true, GetTarget().SupportLinkReg());
    }

    SaveCallerRegistersInFrame(caller_regs, encoder, GetFrameLayout(), false);
    encoder->PopRegisters(saved_registers_, false, GetTarget().SupportLinkReg());

    /* Offset of the runtime entrypoint is stored in the first intrinsic's immediate */
    CHECK_EQ(inst->GetImms().size(), 1U);
    if (inst->GetRelocate()) {
        RelocationInfo relocation;
        encoder->EncodeJump(&relocation);
        GetGraph()->GetRelocationHandler()->AddRelocation(relocation);
    } else {
        ScopedTmpReg tmp(encoder);
        auto offset = inst->GetImms()[0];
        encoder->EncodeLdr(tmp, false, MemRef(ThreadReg(), offset));
        encoder->EncodeJump(tmp);
    }
}

void CodegenFastPath::IntrinsicSaveRegisters([[maybe_unused]] IntrinsicInst *inst)
{
    RegMask callee_regs = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    // We need to save all caller regs, since caller doesn't care about registers at all (except parameters)
    auto caller_regs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto caller_vregs = RegMask(GetCallerRegsMask(GetArch(), true));
    // Alignment to 16 bytes is required in two cases:
    // 1. If target supports link register, then it didn't push return address on the stack, so SP is already aligned,
    //    thus, we need to keep it aligned after pushing registers.
    // 2. If amount of the saved registers in prologue is even(i.e. aligned), then even if target doesn't support LR,
    //    we need to align SP, because targets, that don't support LR, push LR on stack, thus SP is not aligned when we
    //    came to this prologue.
    bool align = IsPrologueAligned() || GetTarget().SupportLinkReg();
    for (auto &input : inst->GetInputs()) {
        callee_regs.reset(input.GetInst()->GetDstReg());
        caller_regs.reset(input.GetInst()->GetDstReg());
    }
    if (GetTarget().SupportLinkReg()) {
        caller_regs.set(GetTarget().GetLinkReg().GetId());
    }
    GetEncoder()->PushRegisters(caller_regs | callee_regs, false, align);
    GetEncoder()->PushRegisters(caller_vregs, true, align);
}

void CodegenFastPath::IntrinsicRestoreRegisters([[maybe_unused]] IntrinsicInst *inst)
{
    RegMask callee_regs = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    // We need to restore all caller regs, since caller doesn't care about registers at all (except parameters)
    auto caller_regs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto caller_vregs = RegMask(GetCallerRegsMask(GetArch(), true));
    bool align = IsPrologueAligned() || GetTarget().SupportLinkReg();
    for (auto &input : inst->GetInputs()) {
        callee_regs.reset(input.GetInst()->GetDstReg());
        caller_regs.reset(input.GetInst()->GetDstReg());
    }
    if (GetTarget().SupportLinkReg()) {
        caller_regs.set(GetTarget().GetLinkReg().GetId());
    }
    GetEncoder()->PopRegisters(caller_vregs, true, align);
    GetEncoder()->PopRegisters(caller_regs | callee_regs, false, align);
}

}  // namespace panda::compiler
