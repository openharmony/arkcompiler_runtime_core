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
/*
Low-level calling convention
*/
#include "target/aarch64/target.h"

namespace panda::compiler::aarch64 {

constexpr int32_t IMM_2 = 2;

Aarch64CallingConvention::Aarch64CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr,
                                                   CallConvMode mode)
    : CallingConvention(allocator, enc, descr, mode)
{
}

ParameterInfo *Aarch64CallingConvention::GetParameterInfo(uint8_t regs_offset)
{
    auto param_info = GetAllocator()->New<aarch64::Aarch64ParameterInfo>();
    for (int i = 0; i < regs_offset; ++i) {
        param_info->GetNativeParam(INT64_TYPE);
    }
    return param_info;
}

void *Aarch64CallingConvention::GetCodeEntry()
{
    return reinterpret_cast<void *>(GetMasm()->GetInstructionAt(0));
}

uint32_t Aarch64CallingConvention::GetCodeSize()
{
    return GetMasm()->GetSizeOfCodeGenerated();
}

size_t Aarch64CallingConvention::PushRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs,
                                          bool is_callee)
{
    if ((regs.GetCount() % IMM_2) == 1) {
        ASSERT((regs.GetList() & (UINT64_C(1) << vixl::aarch64::xzr.GetCode())) == 0);
        regs.Combine(vixl::aarch64::xzr);
    }
    if ((vregs.GetCount() % IMM_2) == 1) {
        auto regdescr = static_cast<Aarch64RegisterDescription *>(GetRegfile());
        uint8_t allignment_vreg = regdescr->GetAlignmentVreg(is_callee);
        ASSERT((vregs.GetList() & (UINT64_C(1) << allignment_vreg)) == 0);
        vregs.Combine(allignment_vreg);
    }
    GetMasm()->PushCPURegList(vregs);
    GetMasm()->PushCPURegList(regs);
    return vregs.GetCount() + regs.GetCount();
}

size_t Aarch64CallingConvention::PopRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs,
                                         bool is_callee)
{
    if ((regs.GetCount() % IMM_2) == 1) {
        ASSERT((regs.GetList() & (UINT64_C(1) << vixl::aarch64::xzr.GetCode())) == 0);
        regs.Combine(vixl::aarch64::xzr);
    }
    if ((vregs.GetCount() % IMM_2) == 1) {
        auto regdescr = static_cast<Aarch64RegisterDescription *>(GetRegfile());
        uint8_t allignment_vreg = regdescr->GetAlignmentVreg(is_callee);
        ASSERT((vregs.GetList() & (UINT64_C(1) << allignment_vreg)) == 0);
        vregs.Combine(allignment_vreg);
    }
    GetMasm()->PopCPURegList(regs);
    GetMasm()->PopCPURegList(vregs);
    return vregs.GetCount() + regs.GetCount();
}

std::variant<Reg, uint8_t> Aarch64ParameterInfo::GetNativeParam(const TypeInfo &type)
{
    if (type.IsFloat()) {
        if (current_vector_number_ > MAX_VECTOR_PARAM_ID) {
            return current_stack_offset_++;
        }
        return Reg(current_vector_number_++, type);
    }
    if (current_scalar_number_ > MAX_SCALAR_PARAM_ID) {
        return current_stack_offset_++;
    }
    auto ret = Reg(current_scalar_number_++, type);
    if (type.GetSize() > DOUBLE_WORD_SIZE) {
        current_scalar_number_++;
    }
    return ret;
}

Location Aarch64ParameterInfo::GetNextLocation(DataType::Type type)
{
    if (DataType::IsFloatType(type)) {
        if (current_vector_number_ > MAX_VECTOR_PARAM_ID) {
            return Location::MakeStackArgument(current_stack_offset_++);
        }
        return Location::MakeFpRegister(current_vector_number_++);
    }
    if (current_scalar_number_ > MAX_SCALAR_PARAM_ID) {
        return Location::MakeStackArgument(current_stack_offset_++);
    }
    Target target(Arch::AARCH64);
    return Location::MakeRegister(target.GetParamRegId(current_scalar_number_++));
}

Reg Aarch64CallingConvention::InitFlagsReg(bool has_float_regs)
{
    auto flags {static_cast<uint64_t>(has_float_regs) << CFrameLayout::HasFloatRegsFlag::START_BIT};
    auto flags_reg {GetTarget().GetZeroReg()};
    if (flags != 0U) {
        flags_reg = GetTarget().GetLinkReg();
        GetEncoder()->EncodeMov(flags_reg, Imm(flags));
    }
    return flags_reg;
}

using vixl::aarch64::CPURegList, vixl::aarch64::CPURegister, vixl::aarch64::MemOperand;

void Aarch64CallingConvention::GeneratePrologue(const FrameInfo &frame_info)
{
    static_assert((CFrameLayout::GetLocalsCount() & 1U) == 0);
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto regdescr = static_cast<Aarch64RegisterDescription *>(GetRegfile());
    auto sp = GetTarget().GetStackReg();
    auto fp = GetTarget().GetFrameReg();
    auto lr = GetTarget().GetLinkReg();
    auto sp_to_regs_slots = CFrameLayout::GetTopToRegsSlotsCount();

    // Save FP and LR
    if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
        static_assert(CFrameLayout::GetTopToRegsSlotsCount() > CFrameLayout::GetFpLrSlotsCount());
        GetMasm()->PushCPURegList(vixl::aarch64::CPURegList(VixlReg(fp), VixlReg(lr)));
        SET_CFI_OFFSET(push_fplr, encoder->GetCursorOffset());
        sp_to_regs_slots -= CFrameLayout::GetFpLrSlotsCount();
    }

    // Setup FP
    if (frame_info.GetSetupFrame() || ProvideCFI()) {
        // If SetupFrame flag is set, then SaveFrameAndLinkRegs must be set also.
        // These are separate flags as it looks like Irtoc does not need frame setup
        // but requires to save frame and link regs.
        ASSERT(!frame_info.GetSetupFrame() || frame_info.GetSaveFrameAndLinkRegs());
        encoder->EncodeMov(fp, sp);
        SET_CFI_OFFSET(set_fp, encoder->GetCursorOffset());
    }

    // Reset flags and setup method
    if (frame_info.GetSetupFrame()) {
        static_assert(CFrameMethod::End() == CFrameFlags::Start());
        constexpr int64_t slots_count = CFrameMethod::GetSize() + CFrameFlags::GetSize();

        GetMasm()->Stp(VixlReg(InitFlagsReg(frame_info.GetHasFloatRegs())),  // Reset OSR flag and set HasFloatRegsFlag
                       VixlReg(GetTarget().GetParamReg(0)),                  // Set Method pointer
                       vixl::aarch64::MemOperand(VixlReg(sp), VixlImm(-slots_count * fl.GetSlotSize()),
                                                 vixl::aarch64::AddrMode::PreIndex));
        sp_to_regs_slots -= slots_count;
    }

    // Save callee-saved registers
    RegMask callee_regs_mask;
    VRegMask callee_vregs_mask;
    regdescr->FillUsedCalleeSavedRegisters(&callee_regs_mask, &callee_vregs_mask, frame_info.GetSaveUnusedCalleeRegs());
    SET_CFI_CALLEE_REGS(callee_regs_mask);
    SET_CFI_CALLEE_VREGS(callee_vregs_mask);
    auto last_callee_reg = sp_to_regs_slots + callee_regs_mask.Count();
    auto last_callee_vreg = sp_to_regs_slots + fl.GetCalleeRegistersCount(false) + callee_vregs_mask.Count();
    auto callee_regs = CPURegList(CPURegister::kRegister, vixl::aarch64::kXRegSize, callee_regs_mask.GetValue());
    auto callee_vregs = CPURegList(CPURegister::kVRegister, vixl::aarch64::kXRegSize, callee_vregs_mask.GetValue());
    GetMasm()->StoreCPURegList(callee_regs, MemOperand(VixlReg(sp), VixlImm(-last_callee_reg * fl.GetSlotSize())));
    GetMasm()->StoreCPURegList(callee_vregs, MemOperand(VixlReg(sp), VixlImm(-last_callee_vreg * fl.GetSlotSize())));
    SET_CFI_OFFSET(push_callees, encoder->GetCursorOffset());

    // Adjust SP
    if (frame_info.GetAdjustSpReg()) {
        auto sp_to_frame_end_offset = (sp_to_regs_slots + fl.GetRegsSlotsCount()) * fl.GetSlotSize();
        encoder->EncodeSub(sp, sp, Imm(sp_to_frame_end_offset));
    }
}

void Aarch64CallingConvention::GenerateEpilogue(const FrameInfo &frame_info, std::function<void()> post_job)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto regdescr = static_cast<Aarch64RegisterDescription *>(GetRegfile());
    auto sp = GetTarget().GetStackReg();
    auto fp = GetTarget().GetFrameReg();
    auto lr = GetTarget().GetLinkReg();

    if (post_job) {
        post_job();
    }

    // Restore callee-registers
    RegMask callee_regs_mask;
    VRegMask callee_vregs_mask;
    regdescr->FillUsedCalleeSavedRegisters(&callee_regs_mask, &callee_vregs_mask, frame_info.GetSaveUnusedCalleeRegs());

    auto callee_regs = CPURegList(CPURegister::kRegister, vixl::aarch64::kXRegSize, callee_regs_mask.GetValue());
    auto callee_vregs = CPURegList(CPURegister::kVRegister, vixl::aarch64::kXRegSize, callee_vregs_mask.GetValue());

    if (frame_info.GetAdjustSpReg()) {
        // SP points to the frame's bottom
        auto last_callee_reg = fl.GetRegsSlotsCount() - callee_regs_mask.Count();
        auto last_callee_vreg = fl.GetRegsSlotsCount() - fl.GetCalleeRegistersCount(false) - callee_vregs_mask.Count();
        GetMasm()->LoadCPURegList(callee_regs, MemOperand(VixlReg(sp), VixlImm(last_callee_reg * fl.GetSlotSize())));
        GetMasm()->LoadCPURegList(callee_vregs, MemOperand(VixlReg(sp), VixlImm(last_callee_vreg * fl.GetSlotSize())));
    } else {
        // SP either points to the frame's top or frame's top + FPLR slot
        auto sp_to_regs_slots = CFrameLayout::GetTopToRegsSlotsCount();
        if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
            // Adjust for FPLR slot
            sp_to_regs_slots -= CFrameLayout::GetFpLrSlotsCount();
        }
        auto last_callee_reg = sp_to_regs_slots + callee_regs_mask.Count();
        auto last_callee_vreg = sp_to_regs_slots + fl.GetCalleeRegistersCount(false) + callee_vregs_mask.Count();
        GetMasm()->LoadCPURegList(callee_regs, MemOperand(VixlReg(sp), VixlImm(-last_callee_reg * fl.GetSlotSize())));
        GetMasm()->LoadCPURegList(callee_vregs, MemOperand(VixlReg(sp), VixlImm(-last_callee_vreg * fl.GetSlotSize())));
    }
    SET_CFI_OFFSET(pop_callees, encoder->GetCursorOffset());

    // Adjust SP
    if (frame_info.GetAdjustSpReg()) {
        // SP points to the frame's bottom
        auto sp_to_frame_top_slots = fl.GetRegsSlotsCount() + CFrameRegs::Start() - CFrameReturnAddr::Start();
        if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
            sp_to_frame_top_slots -= CFrameLayout::GetFpLrSlotsCount();
        }
        auto sp_to_frame_top_offset = sp_to_frame_top_slots * fl.GetSlotSize();
        encoder->EncodeAdd(sp, sp, Imm(sp_to_frame_top_offset));
    }

    // Restore FP and LR
    if (IsOsrMode()) {
        encoder->EncodeAdd(sp, sp, Imm(CFrameLayout::GetFpLrSlotsCount() * fl.GetSlotSize()));
        encoder->EncodeLdp(fp, lr, false, MemRef(fp, -fl.GetOsrFpLrOffset()));
    } else if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
        GetMasm()->PopCPURegList(vixl::aarch64::CPURegList(VixlReg(fp), VixlReg(lr)));
    }
    SET_CFI_OFFSET(pop_fplr, encoder->GetCursorOffset());

    GetMasm()->Ret();
}

void Aarch64CallingConvention::GenerateNativePrologue(const FrameInfo &frame_info)
{
    static_assert((CFrameLayout::GetLocalsCount() & 1U) == 0);
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto regdescr = static_cast<Aarch64RegisterDescription *>(GetRegfile());
    auto sp = GetTarget().GetStackReg();
    auto fp = GetTarget().GetFrameReg();
    auto lr = GetTarget().GetLinkReg();
    auto sp_to_regs_slots = CFrameLayout::GetTopToRegsSlotsCount();

    // Save FP and LR
    if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
        static_assert(CFrameLayout::GetTopToRegsSlotsCount() > CFrameLayout::GetFpLrSlotsCount());
        GetMasm()->PushCPURegList(vixl::aarch64::CPURegList(VixlReg(fp), VixlReg(lr)));
        SET_CFI_OFFSET(push_fplr, encoder->GetCursorOffset());
        sp_to_regs_slots -= CFrameLayout::GetFpLrSlotsCount();
    }

    // Note that 'Native' calling convention does not reqire setting up FP.
    // We have to setup FP iff CFI is requested.
    ASSERT(!frame_info.GetSetupFrame());
    if (ProvideCFI()) {
        encoder->EncodeMov(fp, sp);
        SET_CFI_OFFSET(set_fp, encoder->GetCursorOffset());
    }

    // Save callee-saved registers
    RegMask callee_regs_mask;
    VRegMask callee_vregs_mask;
    regdescr->FillUsedCalleeSavedRegisters(&callee_regs_mask, &callee_vregs_mask, frame_info.GetSaveUnusedCalleeRegs());
    SET_CFI_CALLEE_REGS(callee_regs_mask);
    SET_CFI_CALLEE_VREGS(callee_vregs_mask);
    auto last_callee_reg = sp_to_regs_slots + callee_regs_mask.Count();
    auto last_callee_vreg = sp_to_regs_slots + fl.GetCalleeRegistersCount(false) + callee_vregs_mask.Count();
    auto callee_regs = CPURegList(CPURegister::kRegister, vixl::aarch64::kXRegSize, callee_regs_mask.GetValue());
    auto callee_vregs = CPURegList(CPURegister::kVRegister, vixl::aarch64::kXRegSize, callee_vregs_mask.GetValue());
    GetMasm()->StoreCPURegList(callee_regs, MemOperand(VixlReg(sp), VixlImm(-last_callee_reg * fl.GetSlotSize())));
    GetMasm()->StoreCPURegList(callee_vregs, MemOperand(VixlReg(sp), VixlImm(-last_callee_vreg * fl.GetSlotSize())));
    SET_CFI_OFFSET(push_callees, encoder->GetCursorOffset());

    // Adjust SP
    if (frame_info.GetAdjustSpReg()) {
        auto sp_to_frame_end_offset = (sp_to_regs_slots + fl.GetRegsSlotsCount()) * fl.GetSlotSize();
        encoder->EncodeSub(sp, sp, Imm(sp_to_frame_end_offset));
    }
}

void Aarch64CallingConvention::GenerateNativeEpilogue(const FrameInfo &frame_info, std::function<void()> post_job)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto regdescr = static_cast<Aarch64RegisterDescription *>(GetRegfile());
    auto sp = GetTarget().GetStackReg();
    auto fp = GetTarget().GetFrameReg();
    auto lr = GetTarget().GetLinkReg();

    if (post_job) {
        post_job();
    }

    // Restore callee-registers
    RegMask callee_regs_mask;
    VRegMask callee_vregs_mask;
    regdescr->FillUsedCalleeSavedRegisters(&callee_regs_mask, &callee_vregs_mask, frame_info.GetSaveUnusedCalleeRegs());

    auto callee_regs = CPURegList(CPURegister::kRegister, vixl::aarch64::kXRegSize, callee_regs_mask.GetValue());
    auto callee_vregs = CPURegList(CPURegister::kVRegister, vixl::aarch64::kXRegSize, callee_vregs_mask.GetValue());

    if (frame_info.GetAdjustSpReg()) {
        // SP points to the frame's bottom
        auto last_callee_reg = fl.GetRegsSlotsCount() - callee_regs_mask.Count();
        auto last_callee_vreg = fl.GetRegsSlotsCount() - fl.GetCalleeRegistersCount(false) - callee_vregs_mask.Count();
        GetMasm()->LoadCPURegList(callee_regs, MemOperand(VixlReg(sp), VixlImm(last_callee_reg * fl.GetSlotSize())));
        GetMasm()->LoadCPURegList(callee_vregs, MemOperand(VixlReg(sp), VixlImm(last_callee_vreg * fl.GetSlotSize())));
    } else {
        // SP either points to the frame's top or frame's top + FPLR slot
        auto sp_to_regs_slots = CFrameLayout::GetTopToRegsSlotsCount();
        if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
            // Adjust for FPLR slot
            sp_to_regs_slots -= CFrameLayout::GetFpLrSlotsCount();
        }
        auto last_callee_reg = sp_to_regs_slots + callee_regs_mask.Count();
        auto last_callee_vreg = sp_to_regs_slots + fl.GetCalleeRegistersCount(false) + callee_vregs_mask.Count();
        GetMasm()->LoadCPURegList(callee_regs, MemOperand(VixlReg(sp), VixlImm(-last_callee_reg * fl.GetSlotSize())));
        GetMasm()->LoadCPURegList(callee_vregs, MemOperand(VixlReg(sp), VixlImm(-last_callee_vreg * fl.GetSlotSize())));
    }
    SET_CFI_OFFSET(pop_callees, encoder->GetCursorOffset());

    // Adjust SP
    if (frame_info.GetAdjustSpReg()) {
        // SP points to the frame's bottom
        auto sp_to_frame_top_slots = fl.GetRegsSlotsCount() + CFrameRegs::Start() - CFrameReturnAddr::Start();
        if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
            sp_to_frame_top_slots -= CFrameLayout::GetFpLrSlotsCount();
        }
        auto sp_to_frame_top_offset = sp_to_frame_top_slots * fl.GetSlotSize();
        encoder->EncodeAdd(sp, sp, Imm(sp_to_frame_top_offset));
    }

    // Restore FP and LR
    if (IsOsrMode()) {
        encoder->EncodeAdd(sp, sp, Imm(CFrameLayout::GetFpLrSlotsCount() * fl.GetSlotSize()));
        encoder->EncodeLdp(fp, lr, false, MemRef(fp, -fl.GetOsrFpLrOffset()));
    } else if (frame_info.GetSaveFrameAndLinkRegs() || ProvideCFI()) {
        GetMasm()->PopCPURegList(vixl::aarch64::CPURegList(VixlReg(fp), VixlReg(lr)));
    }
    SET_CFI_OFFSET(pop_fplr, encoder->GetCursorOffset());

    GetMasm()->Ret();
}
}  // namespace panda::compiler::aarch64
