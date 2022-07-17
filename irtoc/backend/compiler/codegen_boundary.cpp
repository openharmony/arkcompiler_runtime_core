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

#include "codegen_boundary.h"
#include "utils/cframe_layout.h"

namespace panda::compiler {

void CodegenBoundary::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "Boundary Prologue");
    auto encoder = GetEncoder();
    auto frame = GetFrameInfo();

    if (GetTarget().SupportLinkReg()) {
        ScopedTmpReg tmp(encoder);
        static constexpr ssize_t FP_OFFSET = 3;
        static constexpr ssize_t LR_OFFSET = 2;
        encoder->EncodeMov(tmp, Imm(FrameBridgeKind::COMPILED_CODE_TO_INTERPRETER));
        encoder->EncodeStp(tmp, GetTarget().GetLinkReg(),
                           MemRef(GetTarget().GetStackReg(), -LR_OFFSET * GetTarget().WordSize()));
        encoder->EncodeStr(GetTarget().GetFrameReg(),
                           MemRef(GetTarget().GetStackReg(), -FP_OFFSET * GetTarget().WordSize()));

        encoder->EncodeSub(GetTarget().GetLinkReg(), GetTarget().GetStackReg(),
                           Imm(FP_OFFSET * GetTarget().WordSize()));
        encoder->EncodeStr(GetTarget().GetLinkReg(), MemRef(ThreadReg(), GetRuntime()->GetTlsFrameOffset(GetArch())));
    } else {
        static constexpr ssize_t FP_OFFSET = 2;
        encoder->EncodeSti(Imm(FrameBridgeKind::COMPILED_CODE_TO_INTERPRETER),
                           MemRef(GetTarget().GetStackReg(), -1 * GetTarget().WordSize()));
        encoder->EncodeStr(GetTarget().GetFrameReg(),
                           MemRef(GetTarget().GetStackReg(), -FP_OFFSET * GetTarget().WordSize()));

        {
            ScopedTmpReg tmp(GetEncoder());
            encoder->EncodeSub(tmp, GetTarget().GetStackReg(), Imm(2U * GetTarget().WordSize()));
            encoder->EncodeStr(tmp, MemRef(ThreadReg(), GetRuntime()->GetTlsFrameOffset(GetArch())));
        }
    }

    encoder->EncodeSub(GetTarget().GetStackReg(), GetTarget().GetStackReg(), Imm(frame->GetFrameSize()));

    auto callee_regs =
        GetCalleeRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong() & ~ThreadReg().GetId();
    auto callee_vregs = GetCalleeRegsMask(GetArch(), true) & ~GetTarget().GetTempVRegsMask().to_ulong();
    auto caller_regs = GetCallerRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong();
    auto caller_vregs = GetCallerRegsMask(GetArch(), true) & ~GetTarget().GetTempRegsMask().to_ulong();

    Reg base = GetTarget().GetFrameReg();
    auto fl = GetFrameLayout();
    {
        SCOPED_DISASM_STR(this, "Save caller registers");
        ssize_t offset = fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(CFrameLayout::GetStackStartSlot() +
                                                                             fl.GetCallerLastSlot(false));
        encoder->SaveRegisters(caller_regs, false, -offset, base, GetCallerRegsMask(GetArch(), false));
        offset = fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(CFrameLayout::GetStackStartSlot() +
                                                                     fl.GetCallerLastSlot(true));
        encoder->SaveRegisters(caller_vregs, true, -offset, base, GetCallerRegsMask(GetArch(), true));
    }
    {
        SCOPED_DISASM_STR(this, "Save callee registers");
        base = frame->GetCalleesRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
        encoder->SaveRegisters(callee_regs, false, frame->GetCalleesOffset(), base,
                               GetCalleeRegsMask(GetArch(), false));
        encoder->SaveRegisters(callee_vregs, true, frame->GetFpCalleesOffset(), base,
                               GetCalleeRegsMask(GetArch(), true));
    }
}

void CodegenBoundary::GenerateEpilogue()
{
    SCOPED_DISASM_STR(this, "Boundary Epilogue");
    RemoveBoundaryFrame(GetGraph()->GetEndBlock());
    GetEncoder()->EncodeReturn();
}

void CodegenBoundary::CreateFrameInfo()
{
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(false) | FrameInfo::PositionedCallees::Encode(true) |
        FrameInfo::CallersRelativeFp::Encode(false) | FrameInfo::CalleesRelativeFp::Encode(false));
    auto target = Target(GetGraph()->GetArch());
    size_t spills_count = GetGraph()->GetStackSlotsCount();
    size_t padding = 0;
    size_t frame_size =
        (CFrameLayout::HEADER_SIZE - (target.SupportLinkReg() ? 0 : 1) + GetCalleeRegsCount(target.GetArch(), false) +
         GetCalleeRegsCount(target.GetArch(), true) + GetCallerRegsCount(target.GetArch(), false) +
         GetCallerRegsCount(target.GetArch(), true) + spills_count) *
        target.WordSize();
    if (target.SupportLinkReg()) {
        padding = RoundUp(frame_size, target.GetSpAlignment()) - frame_size;
    } else {
        if ((frame_size % target.GetSpAlignment()) == 0) {
            padding = target.GetSpAlignment() - target.WordSize();
        }
    }
    CHECK_EQ(padding % target.WordSize(), 0U);
    spills_count += padding / target.WordSize();
    frame_size += padding;
    if (target.SupportLinkReg()) {
        CHECK_EQ(frame_size % target.GetSpAlignment(), 0U);
    } else {
        CHECK_EQ(frame_size % target.GetSpAlignment(), target.GetSpAlignment() - target.WordSize());
    }

    ssize_t offset = spills_count;
    frame->SetFpCallersOffset(offset);
    offset += helpers::ToSigned(GetCallerRegsCount(target.GetArch(), true));
    frame->SetCallersOffset(offset);
    offset += helpers::ToSigned(GetCallerRegsCount(target.GetArch(), false));

    frame->SetFpCalleesOffset(offset);
    offset += helpers::ToSigned(GetCalleeRegsCount(target.GetArch(), true));
    frame->SetCalleesOffset(offset);

    frame->SetSpillsCount(spills_count);
    frame->SetFrameSize(frame_size);

    SetFrameInfo(frame);
}

void CodegenBoundary::IntrinsicTailCall(IntrinsicInst *inst)
{
    auto location = inst->GetLocation(0);
    ASSERT(location.IsFixedRegister() && location.IsRegisterValid());
    auto src = Reg(location.GetValue(), GetTarget().GetPtrRegType());
    RegMask liveout_mask = GetLiveOut(inst->GetBasicBlock());
    ScopedTmpRegLazy target(GetEncoder());
    if (!liveout_mask.Test(src.GetId())) {
        target.Acquire();
        ASSERT(target.GetReg().IsValid());
        GetEncoder()->EncodeMov(target, src);
        src = target.GetReg();
    }
    ASSERT(src.IsValid());
    RemoveBoundaryFrame(inst->GetBasicBlock());
    GetEncoder()->EncodeJump(src);
}

void CodegenBoundary::RemoveBoundaryFrame(const BasicBlock *bb) const
{
    auto encoder = GetEncoder();
    auto frame = GetFrameInfo();

    RegMask liveout_mask = GetLiveOut(bb);

    RegMask callee_regs = GetCalleeRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong();
    callee_regs &= ~liveout_mask;
    callee_regs.reset(ThreadReg().GetId());
    RegMask callee_vregs = GetCalleeRegsMask(GetArch(), true) & ~GetTarget().GetTempVRegsMask().to_ulong();
    RegMask caller_regs = GetCallerRegsMask(GetArch(), false) & ~GetTarget().GetTempRegsMask().to_ulong();
    caller_regs &= ~liveout_mask;
    RegMask caller_vregs = GetCallerRegsMask(GetArch(), true) & ~GetTarget().GetTempRegsMask().to_ulong();

    Reg base = GetTarget().GetFrameReg();
    auto fl = GetFrameLayout();
    ssize_t offset = fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(CFrameLayout::GetStackStartSlot() +
                                                                         fl.GetCallerLastSlot(false));
    encoder->LoadRegisters(caller_regs, false, -offset, base, GetCallerRegsMask(GetArch(), false));
    offset = fl.GetOffset<CFrameLayout::FP, CFrameLayout::SLOTS>(CFrameLayout::GetStackStartSlot() +
                                                                 fl.GetCallerLastSlot(true));
    encoder->LoadRegisters(caller_vregs, true, -offset, base, GetCallerRegsMask(GetArch(), true));

    base = frame->GetCalleesRelativeFp() ? GetTarget().GetFrameReg() : GetTarget().GetStackReg();
    encoder->LoadRegisters(callee_regs, false, frame->GetCalleesOffset(), base, GetCalleeRegsMask(GetArch(), false));
    encoder->LoadRegisters(callee_vregs, true, frame->GetFpCalleesOffset(), base, GetCalleeRegsMask(GetArch(), true));

    encoder->EncodeAdd(GetTarget().GetStackReg(), GetTarget().GetStackReg(), Imm(frame->GetFrameSize()));

    if (GetTarget().SupportLinkReg()) {
        static constexpr ssize_t FP_OFFSET = -3;
        encoder->EncodeLdr(GetTarget().GetLinkReg(), false,
                           MemRef(GetTarget().GetStackReg(), -1 * GetTarget().WordSize()));
        encoder->EncodeLdr(GetTarget().GetFrameReg(), false,
                           MemRef(GetTarget().GetStackReg(), FP_OFFSET * GetTarget().WordSize()));
    }
}
}  // namespace panda::compiler
