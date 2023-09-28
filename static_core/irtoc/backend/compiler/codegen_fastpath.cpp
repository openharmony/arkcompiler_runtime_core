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
#include "optimizer/ir/inst.h"
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

/*
 * We determine runtime calls manually, not using MethodProperties::HasRuntimeCalls, because we need to ignore
 * SLOW_PATH_ENTRY intrinsic, since it doesn't require LR to be preserved.
 */
static bool HasRuntimeCalls(const Graph &graph)
{
    bool has_runtime_calls = false;
    for (auto bb : graph.GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            switch (inst->GetOpcode()) {
                case Opcode::StoreArray:
                    has_runtime_calls = inst->CastToStoreArray()->GetNeedBarrier();
                    break;
                case Opcode::StoreObject:
                    has_runtime_calls = inst->CastToStoreObject()->GetNeedBarrier();
                    break;
                case Opcode::LoadObjectDynamic:
                case Opcode::StoreObjectDynamic:
                    has_runtime_calls = true;
                    break;
                case Opcode::Cast:
                    has_runtime_calls = inst->CastToCast()->IsDynamicCast();
                    break;
                default:
                    break;
            }
            if (has_runtime_calls) {
                break;
            }
            if (inst->IsRuntimeCall()) {
                if (inst->IsIntrinsic() &&
                    (inst->CastToIntrinsic()->GetIntrinsicId() ==
                         RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY ||
                     inst->CastToIntrinsic()->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_TAIL_CALL)) {
                    continue;
                }
                has_runtime_calls = true;
                break;
            }
        }
        if (has_runtime_calls) {
            break;
        }
    }
    return has_runtime_calls;
}

void CodegenFastPath::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "FastPath Prologue");

    auto caller_regs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto args_num = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    caller_regs &= GetUsedRegs() & ~GetTarget().GetParamRegsMask(args_num);
    SaveCallerRegistersInFrame(caller_regs, GetEncoder(), GetFrameLayout(), false);

    auto has_runtime_calls = HasRuntimeCalls(*GetGraph());

    saved_registers_ = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    if (GetTarget().SupportLinkReg() && has_runtime_calls) {
        saved_registers_ |= GetTarget().GetLinkReg().GetMask();
    }

    if (GetUsedVRegs().Any()) {
        SaveCallerRegistersInFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), GetEncoder(), GetFrameLayout(),
                                   true);
        saved_fp_registers_ = GetUsedVRegs() & VRegMask(GetCalleeRegsMask(GetArch(), true));
    }

    GetEncoder()->PushRegisters(saved_registers_, saved_fp_registers_, GetTarget().SupportLinkReg());

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

    if (auto ret_type {GetRuntime()->GetMethodReturnType(GetGraph()->GetMethod())};
        ret_type != DataType::VOID && ret_type != DataType::NO_TYPE) {
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
    }

    GetEncoder()->PopRegisters(saved_registers_, saved_fp_registers_, GetTarget().SupportLinkReg());

    GetEncoder()->EncodeReturn();
}

void CodegenFastPath::CreateFrameInfo()
{
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(false) |
        FrameInfo::CallersRelativeFp::Encode(true) | FrameInfo::CalleesRelativeFp::Encode(false) |
        FrameInfo::PushCallers::Encode(true));
    frame->SetSpillsCount(GetGraph()->GetStackSlotsCount());
    CFrameLayout fl(GetGraph()->GetArch(), GetGraph()->GetStackSlotsCount());

    frame->SetCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
    frame->SetFpCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
    frame->SetCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
    frame->SetFpCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

    SetFrameInfo(frame);
}

void CodegenFastPath::IntrinsicSlowPathEntry(IntrinsicInst *inst)
{
    CreateTailCall(inst, false);
}

void CodegenFastPath::IntrinsicSaveRegisters([[maybe_unused]] IntrinsicInst *inst)
{
    RegMask callee_regs = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    // We need to save all caller regs, since caller doesn't care about registers at all (except parameters)
    auto caller_regs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto caller_vregs = RegMask(GetCallerRegsMask(GetArch(), true));

    for (auto &input : inst->GetInputs()) {
        callee_regs.reset(input.GetInst()->GetDstReg());
        caller_regs.reset(input.GetInst()->GetDstReg());
    }
    if (GetTarget().SupportLinkReg()) {
        caller_regs.set(GetTarget().GetLinkReg().GetId());
    }
    GetEncoder()->PushRegisters(caller_regs | callee_regs, caller_vregs);
}

void CodegenFastPath::IntrinsicRestoreRegisters([[maybe_unused]] IntrinsicInst *inst)
{
    RegMask callee_regs = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    // We need to restore all caller regs, since caller doesn't care about registers at all (except parameters)
    auto caller_regs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto caller_vregs = RegMask(GetCallerRegsMask(GetArch(), true));
    for (auto &input : inst->GetInputs()) {
        callee_regs.reset(input.GetInst()->GetDstReg());
        caller_regs.reset(input.GetInst()->GetDstReg());
    }
    if (GetTarget().SupportLinkReg()) {
        caller_regs.set(GetTarget().GetLinkReg().GetId());
    }
    GetEncoder()->PopRegisters(caller_regs | callee_regs, caller_vregs);
}

void CodegenFastPath::IntrinsicTailCall(IntrinsicInst *inst)
{
    CreateTailCall(inst, true);
}

void CodegenFastPath::CreateTailCall(IntrinsicInst *inst, bool is_fastpath)
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

    if (is_fastpath) {
        RestoreCallerRegistersFromFrame(GetCallerRegistersToRestore(), encoder, GetFrameLayout(), false);
        if (GetUsedVRegs().Any()) {
            RestoreCallerRegistersFromFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), encoder,
                                            GetFrameLayout(), true);
        }
    } else {
        RegMask caller_regs = ~GetUsedRegs() & RegMask(GetCallerRegsMask(GetArch(), false));
        auto args_num = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
        caller_regs &= ~GetTarget().GetParamRegsMask(args_num);

        if (GetUsedVRegs().Any()) {
            VRegMask fp_caller_regs = ~GetUsedVRegs() & RegMask(GetCallerRegsMask(GetArch(), true));
            SaveCallerRegistersInFrame(fp_caller_regs, encoder, GetFrameLayout(), true);
        }

        SaveCallerRegistersInFrame(caller_regs, encoder, GetFrameLayout(), false);
    }
    encoder->PopRegisters(saved_registers_, saved_fp_registers_, GetTarget().SupportLinkReg());

    /* First Imm is offset of the runtime entrypoint for Ark Irtoc */
    /* Second Imm is necessary for proper LLVM Irtoc FastPath compilation */
    CHECK_LE(inst->GetImms().size(), 2U);
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

}  // namespace panda::compiler
