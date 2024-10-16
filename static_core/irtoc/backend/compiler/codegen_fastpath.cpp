/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::compiler {

static void SaveCallerRegistersInFrame(RegMask mask, Encoder *encoder, const CFrameLayout &fl, bool isFp)
{
    if (mask.none()) {
        return;
    }
    auto fpReg = Target(fl.GetArch()).GetFrameReg();

    mask &= GetCallerRegsMask(fl.GetArch(), isFp);
    auto startSlot = fl.GetStackStartSlot() + fl.GetCallerLastSlot(isFp);
    encoder->SaveRegisters(mask, isFp, -startSlot, fpReg, GetCallerRegsMask(fl.GetArch(), isFp));
}

static void RestoreCallerRegistersFromFrame(RegMask mask, Encoder *encoder, const CFrameLayout &fl, bool isFp)
{
    if (mask.none()) {
        return;
    }
    auto fpReg = Target(fl.GetArch()).GetFrameReg();

    mask &= GetCallerRegsMask(fl.GetArch(), isFp);
    auto startSlot = fl.GetStackStartSlot() + fl.GetCallerLastSlot(isFp);
    encoder->LoadRegisters(mask, isFp, -startSlot, fpReg, GetCallerRegsMask(fl.GetArch(), isFp));
}

static bool InstHasRuntimeCall(const Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Store:
            if (inst->CastToStore()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::StoreI:
            if (inst->CastToStoreI()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::StoreArray:
            if (inst->CastToStoreArray()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::StoreObject:
            if (inst->CastToStoreObject()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::LoadObjectDynamic:
        case Opcode::StoreObjectDynamic:
            return true;
        case Opcode::Cast:
            if (inst->CastToCast()->IsDynamicCast()) {
                return true;
            }
            break;
        default:
            break;
    }
    if (inst->IsRuntimeCall()) {
        if (!inst->IsIntrinsic()) {
            return true;
        }
        auto intrinsicId = inst->CastToIntrinsic()->GetIntrinsicId();
        if (intrinsicId != RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY &&
            intrinsicId != RuntimeInterface::IntrinsicId::INTRINSIC_TAIL_CALL) {
            return true;
        }
    }
    return false;
}
/*
 * We determine runtime calls manually, not using MethodProperties::HasRuntimeCalls, because we need to ignore
 * SLOW_PATH_ENTRY intrinsic, since it doesn't require LR to be preserved.
 */
static bool HasRuntimeCalls(const Graph &graph)
{
    for (auto bb : graph.GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (InstHasRuntimeCall(inst)) {
                return true;
            }
        }
    }
    return false;
}

void CodegenFastPath::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "FastPath Prologue");

    auto callerRegs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto argsNum = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    callerRegs &= GetUsedRegs() & ~GetTarget().GetParamRegsMask(argsNum);
    SaveCallerRegistersInFrame(callerRegs, GetEncoder(), GetFrameLayout(), false);

    auto hasRuntimeCalls = HasRuntimeCalls(*GetGraph());

    savedRegisters_ = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    if (GetTarget().SupportLinkReg() && hasRuntimeCalls) {
        savedRegisters_ |= GetTarget().GetLinkReg().GetMask();
        GetEncoder()->EnableLrAsTempReg(true);
    }

    if (GetUsedVRegs().Any()) {
        SaveCallerRegistersInFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), GetEncoder(), GetFrameLayout(),
                                   true);
        savedFpRegisters_ = GetUsedVRegs() & VRegMask(GetCalleeRegsMask(GetArch(), true));
    }

    GetEncoder()->PushRegisters(savedRegisters_, savedFpRegisters_, GetTarget().SupportLinkReg());

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        GetEncoder()->EncodeSub(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }
}

RegMask CodegenFastPath::GetCallerRegistersToRestore() const
{
    RegMask callerRegs = GetUsedRegs() & RegMask(GetCallerRegsMask(GetArch(), false));

    auto argsNum = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    callerRegs &= ~GetTarget().GetParamRegsMask(argsNum);

    if (auto retType {GetRuntime()->GetMethodReturnType(GetGraph()->GetMethod())};
        retType != DataType::VOID && retType != DataType::NO_TYPE) {
        ASSERT(!DataType::IsFloatType(retType));
        callerRegs.reset(GetTarget().GetReturnRegId());
    }
    return callerRegs;
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

    GetEncoder()->PopRegisters(savedRegisters_, savedFpRegisters_, GetTarget().SupportLinkReg());

    GetEncoder()->EncodeReturn();
}

void CodegenFastPath::CreateFrameInfo()
{
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(false) |
        FrameInfo::CallersRelativeFp::Encode(true) | FrameInfo::CalleesRelativeFp::Encode(false) |
        FrameInfo::PushCallers::Encode(true));
    frame->SetSpillsCount(GetGraph()->GetStackSlotsCount());
    CFrameLayout fl(GetGraph()->GetArch(), GetGraph()->GetStackSlotsCount(), false);

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

/*
 * Safe call of the c++ function from the irtoc
 * */
void CodegenFastPath::IntrinsicSaveTlabStatsSafe([[maybe_unused]] IntrinsicInst *inst, Reg src1, Reg src2, Reg tmp)
{
    ASSERT(!inst->HasUsers());
    ASSERT(tmp.IsValid());
    ASSERT(tmp != GetRegfile()->GetZeroReg());

    auto regs = GetCallerRegsMask(GetArch(), false) | GetCalleeRegsMask(GetArch(), false);
    auto vregs = GetCallerRegsMask(GetArch(), true);
    GetEncoder()->PushRegisters(regs, vregs);

    FillCallParams(src1, src2);

    auto id = RuntimeInterface::EntrypointId::WRITE_TLAB_STATS_NO_BRIDGE;
    MemRef entry(ThreadReg(), GetRuntime()->GetEntrypointTlsOffset(GetArch(), id));
    GetEncoder()->EncodeLdr(tmp, false, entry);
    GetEncoder()->MakeCall(tmp);

    GetEncoder()->PopRegisters(regs, vregs);
}

void CodegenFastPath::IntrinsicSaveRegisters([[maybe_unused]] IntrinsicInst *inst)
{
    RegMask calleeRegs = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    // We need to save all caller regs, since caller doesn't care about registers at all (except parameters)
    auto callerRegs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto callerVregs = RegMask(GetCallerRegsMask(GetArch(), true));
    for (auto &input : inst->GetInputs()) {
        calleeRegs.reset(input.GetInst()->GetDstReg());
        callerRegs.reset(input.GetInst()->GetDstReg());
    }
    if (GetTarget().SupportLinkReg()) {
        callerRegs.set(GetTarget().GetLinkReg().GetId());
    }
    if (!inst->HasUsers()) {
        callerRegs.set(GetTarget().GetReturnReg(GetPtrRegType()).GetId());
    }
    GetEncoder()->PushRegisters(callerRegs | calleeRegs, callerVregs);
}

void CodegenFastPath::IntrinsicRestoreRegisters([[maybe_unused]] IntrinsicInst *inst)
{
    RegMask calleeRegs = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    // We need to restore all caller regs, since caller doesn't care about registers at all (except parameters)
    auto callerRegs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto callerVregs = RegMask(GetCallerRegsMask(GetArch(), true));
    for (auto &input : inst->GetInputs()) {
        calleeRegs.reset(input.GetInst()->GetDstReg());
        callerRegs.reset(input.GetInst()->GetDstReg());
    }
    if (GetTarget().SupportLinkReg()) {
        callerRegs.set(GetTarget().GetLinkReg().GetId());
    }
    if (!inst->HasUsers()) {
        callerRegs.set(GetTarget().GetReturnReg(GetPtrRegType()).GetId());
    }
    GetEncoder()->PopRegisters(callerRegs | calleeRegs, callerVregs);
}

void CodegenFastPath::IntrinsicTailCall(IntrinsicInst *inst)
{
    CreateTailCall(inst, true);
}

void CodegenFastPath::CreateTailCall(IntrinsicInst *inst, bool isFastpath)
{
    auto encoder = GetEncoder();

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        encoder->EncodeAdd(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }

    /* Once we reach the slow path, we can release all temp registers, since slow path terminates execution */
    auto tempsMask = GetTarget().GetTempRegsMask();
    for (size_t reg = tempsMask.GetMinRegister(); reg <= tempsMask.GetMaxRegister(); reg++) {
        if (tempsMask.Test(reg)) {
            GetEncoder()->ReleaseScratchRegister(Reg(reg, INT32_TYPE));
        }
    }

    if (isFastpath) {
        RestoreCallerRegistersFromFrame(GetCallerRegistersToRestore(), encoder, GetFrameLayout(), false);
        if (GetUsedVRegs().Any()) {
            RestoreCallerRegistersFromFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), encoder,
                                            GetFrameLayout(), true);
        }
    } else {
        RegMask callerRegs = ~GetUsedRegs() & RegMask(GetCallerRegsMask(GetArch(), false));
        auto argsNum = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
        callerRegs &= ~GetTarget().GetParamRegsMask(argsNum);

        if (GetUsedVRegs().Any()) {
            VRegMask fpCallerRegs = ~GetUsedVRegs() & RegMask(GetCallerRegsMask(GetArch(), true));
            SaveCallerRegistersInFrame(fpCallerRegs, encoder, GetFrameLayout(), true);
        }

        SaveCallerRegistersInFrame(callerRegs, encoder, GetFrameLayout(), false);
    }
    encoder->PopRegisters(savedRegisters_, savedFpRegisters_, GetTarget().SupportLinkReg());

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

}  // namespace ark::compiler
