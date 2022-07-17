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
#include <cmath>
#include "target/aarch32/target.h"

namespace panda::compiler::aarch32 {
using vixl::aarch32::RegisterList;
using vixl::aarch32::SRegister;
using vixl::aarch32::SRegisterList;

Aarch32CallingConvention::Aarch32CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr,
                                                   CallConvMode mode)
    : CallingConvention(allocator, enc, descr, mode)
{
}

ParameterInfo *Aarch32CallingConvention::GetParameterInfo(uint8_t regs_offset)
{
    auto param_info = GetAllocator()->New<aarch32::Aarch32ParameterInfo>();
    for (int i = 0; i < regs_offset; ++i) {
        param_info->GetNativeParam(INT32_TYPE);
    }
    return param_info;
}

void *Aarch32CallingConvention::GetCodeEntry()
{
    auto res = GetMasm()->GetBuffer()->GetOffsetAddress<uint32_t *>(0);
    return reinterpret_cast<void *>(res);
}

uint32_t Aarch32CallingConvention::GetCodeSize()
{
    return GetMasm()->GetSizeOfCodeGenerated();
}

uint8_t Aarch32CallingConvention::PushPopVRegs(VRegMask vregs, bool is_push = true)
{
    int8_t first = -1;
    uint8_t size = 0;
    bool is_sequential = true;
    for (size_t i = 0; i < vregs.size(); ++i) {
        if (-1 == first && vregs.test(i)) {
            first = i;
            ++size;
            continue;
        }
        if (vregs.test(i)) {
            if (!vregs.test(i - 1)) {
                is_sequential = false;
                break;
            }
            ++size;
        }
    }
    if (first == -1) {
        ASSERT(size == 0);
        return 0;
    }

    if (is_sequential) {
        auto reg_list = vixl::aarch32::SRegisterList(vixl::aarch32::SRegister(first), size);
        if (is_push) {
            GetMasm()->Vpush(reg_list);
        } else {
            GetMasm()->Vpop(reg_list);
        }
        return size;
    }

    uint32_t real_offset = 0;
    if (is_push) {
        for (int32_t i = vregs.size() - 1; i >= 0; --i) {
            if (vregs.test(i)) {
                GetMasm()->PushRegister(VixlVReg(Reg(i, FLOAT32_TYPE)).S());
                ++real_offset;
            }
        }
    } else {
        constexpr auto VREG_SIZE = 1;
        for (size_t i = 0; i < vregs.size(); ++i) {
            if (vregs.test(i)) {
                GetMasm()->Vpop(vixl::aarch32::SRegisterList(VixlVReg(Reg(i, FLOAT32_TYPE)).S(), VREG_SIZE));
                ++real_offset;
            }
        }
    }
    return real_offset;
}

uint8_t Aarch32CallingConvention::PushRegs(RegMask regs, VRegMask vregs, bool is_callee)
{
    auto regdescr = static_cast<Aarch32RegisterDescription *>(GetRegfile());
    auto fp = GetTarget().GetFrameReg().GetId();
    if (regs.test(fp)) {
        regs.reset(fp);
    }
    auto lr = GetTarget().GetLinkReg().GetId();
    if (regs.test(lr)) {
        regs.reset(lr);
    }

    uint8_t real_offset = 0;
    uint32_t saved_registers_mask = 0;

    for (size_t i = 0; i < regs.size(); ++i) {
        if (regs.test(i)) {
            saved_registers_mask |= 1UL << i;
            ++real_offset;
        }
    }

    if (((regs.count() + vregs.count()) & 1U) == 1) {
        // TODO(igorban) move them to Sub(sp)
        uint8_t align_reg = regdescr->GetAligmentReg(is_callee);
        GetMasm()->PushRegister(vixl::aarch32::Register(align_reg));
        ++real_offset;
    }

    if (saved_registers_mask != 0) {
        GetMasm()->Push(vixl::aarch32::RegisterList(saved_registers_mask));
    }
    real_offset += PushPopVRegs(vregs, true);
    ASSERT((real_offset & 1U) == 0);

    return real_offset;
}

uint8_t Aarch32CallingConvention::PopRegs(RegMask regs, VRegMask vregs, bool is_callee)
{
    auto regdescr = static_cast<Aarch32RegisterDescription *>(GetRegfile());

    auto fp = GetTarget().GetFrameReg().GetId();
    if (regs.test(fp)) {
        regs.reset(fp);
    }
    auto lr = GetTarget().GetLinkReg().GetId();
    if (regs.test(lr)) {
        regs.reset(lr);
    }

    uint8_t real_offset = 0;
    real_offset += PushPopVRegs(vregs, false);

    uint32_t saved_registers_mask = 0;

    for (size_t i = 0; i < regs.size(); ++i) {
        if (regs.test(i)) {
            saved_registers_mask |= 1UL << i;
            ++real_offset;
        }
    }

    if (saved_registers_mask != 0) {
        GetMasm()->Pop(vixl::aarch32::RegisterList(saved_registers_mask));
    }

    if (((regs.count() + vregs.count()) & 1U) == 1) {
        uint8_t align_reg = regdescr->GetAligmentReg(is_callee);
        GetMasm()->Pop(vixl::aarch32::Register(align_reg));
        ++real_offset;
    }
    ASSERT((real_offset & 1U) == 0);

    return real_offset;
}

std::variant<Reg, uint8_t> Aarch32ParameterInfo::GetNativeParam(const TypeInfo &type)
{
    constexpr int32_t STEP = 2;
#if (PANDA_TARGET_ARM32_ABI_HARD)
    // Use vector registers
    if (type == FLOAT32_TYPE) {
        if (current_vector_number_ > MAX_VECTOR_SINGLE_PARAM_ID) {
            return current_stack_offset_++;
        }
        return Reg(current_vector_number_++, FLOAT32_TYPE);
    }
    if (type == FLOAT64_TYPE) {
        // Allignment for 8 bytes (in stack and registers)
        if ((current_vector_number_ & 1U) == 1) {
            ++current_vector_number_;
        }
        if ((current_vector_number_ >> 1U) > MAX_VECTOR_DOUBLE_PARAM_ID) {
            if ((current_stack_offset_ & 1U) == 1) {
                ++current_stack_offset_;
            }
            auto stack_offset = current_stack_offset_;
            current_stack_offset_ += STEP;
            return stack_offset;
        }
        auto vector_number = current_vector_number_;
        current_vector_number_ += STEP;
        return Reg(vector_number, FLOAT64_TYPE);
    }
#endif  // PANDA_TARGET_ARM32_ABI_HARD
    if (type.GetSize() == DOUBLE_WORD_SIZE) {
        if ((current_scalar_number_ & 1U) == 1) {
            ++current_scalar_number_;
        }
        // Allignment for 8 bytes (in stack and registers)
        if (current_scalar_number_ > MAX_SCALAR_PARAM_ID) {
            if ((current_stack_offset_ & 1U) == 1) {
                ++current_stack_offset_;
            }
            auto stack_offset = current_stack_offset_;
            current_stack_offset_ += STEP;
            return stack_offset;
        }
        auto scalar_number = current_scalar_number_;
        current_scalar_number_ += STEP;
        return Reg(scalar_number, INT64_TYPE);
    }
    if (current_scalar_number_ > MAX_SCALAR_PARAM_ID) {
        return current_stack_offset_++;
    }
    ASSERT(!type.IsFloat() || type == FLOAT32_TYPE);
    return Reg(current_scalar_number_++, type.IsFloat() ? INT32_TYPE : type);
}

Location Aarch32ParameterInfo::GetNextLocation(DataType::Type type)
{
    auto res = GetNativeParam(TypeInfo::FromDataType(type, Arch::AARCH32));
    if (std::holds_alternative<Reg>(res)) {
        auto reg = std::get<Reg>(res);
#if (PANDA_TARGET_ARM32_ABI_SOFT || PANDA_TARGET_ARM32_ABI_SOFTFP)
        if (DataType::IsFloatType(type)) {
            return Location::MakeRegister(reg.GetId());
        }
#endif
        return Location::MakeRegister(reg.GetId(), type);
    }
    return Location::MakeStackArgument(std::get<uint8_t>(res));
}

void Aarch32CallingConvention::GeneratePrologue([[maybe_unused]] const FrameInfo &frame_info)
{
    auto encoder = GetEncoder();
    ASSERT(encoder->IsValid());
    ASSERT(encoder->InitMasm());
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto fp_reg = GetTarget().GetFrameReg();
    auto sp_reg = GetTarget().GetStackReg();

    GetMasm()->Push(RegisterList(vixl::aarch32::r11, vixl::aarch32::lr));
    SET_CFI_OFFSET(push_fplr, encoder->GetCursorOffset());

    encoder->EncodeMov(fp_reg, sp_reg);
    SET_CFI_OFFSET(set_fp, encoder->GetCursorOffset());
    constexpr auto IMM_2 = 2;
    encoder->EncodeSub(sp_reg, sp_reg, Imm(WORD_SIZE_BYTE * IMM_2));
    encoder->EncodeStr(GetTarget().GetParamReg(0), MemRef(sp_reg, WORD_SIZE_BYTE));

    // Allocate space for locals
    auto locals_size = (CFrameSlots::Start() - CFrameData::Start()) * WORD_SIZE_BYTE;
    encoder->EncodeSub(sp_reg, sp_reg, Imm(locals_size));

    SET_CFI_CALLEE_REGS(GetCalleeRegsMask(Arch::AARCH32, false));
    SET_CFI_CALLEE_VREGS(GetCalleeRegsMask(Arch::AARCH32, true));
    GetMasm()->Push(RegisterList(GetCalleeRegsMask(Arch::AARCH32, false).GetValue()));
    GetMasm()->Vpush(
        SRegisterList(SRegister(GetFirstCalleeReg(Arch::AARCH32, true)), GetCalleeRegsCount(Arch::AARCH32, true)));
    SET_CFI_OFFSET(push_callees, encoder->GetCursorOffset());

    // Reset OSR flag and set HasFloatRegsFlag
    auto callee_regs_size =
        (GetCalleeRegsCount(Arch::AARCH32, true) + GetCalleeRegsCount(Arch::AARCH32, false)) * WORD_SIZE_BYTE;
    auto flags {static_cast<uint32_t>(frame_info.GetHasFloatRegs()) << CFrameLayout::HasFloatRegsFlag::START_BIT};
    encoder->EncodeSti(Imm(flags), MemRef(sp_reg, callee_regs_size + locals_size));

    encoder->EncodeSub(
        sp_reg, sp_reg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            WORD_SIZE_BYTE));
}

void Aarch32CallingConvention::GenerateEpilogue([[maybe_unused]] const FrameInfo &frame_info,
                                                std::function<void()> /* post_job */)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto sp_reg = GetTarget().GetStackReg();

    encoder->EncodeAdd(
        sp_reg, sp_reg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            WORD_SIZE_BYTE));

    GetMasm()->Vpop(
        SRegisterList(SRegister(GetFirstCalleeReg(Arch::AARCH32, true)), GetCalleeRegsCount(Arch::AARCH32, true)));
    GetMasm()->Pop(RegisterList(GetCalleeRegsMask(Arch::AARCH32, false).GetValue()));
    SET_CFI_OFFSET(pop_callees, encoder->GetCursorOffset());

    // ARM32 doesn't support OSR mode
    ASSERT(!IsOsrMode());
    // Support restoring of LR and FP registers once OSR is supported in arm32
    static_assert(!ArchTraits<Arch::AARCH32>::SUPPORT_OSR);
    constexpr auto IMM_2 = 2;
    encoder->EncodeAdd(sp_reg, sp_reg, Imm(WORD_SIZE_BYTE * IMM_2));
    encoder->EncodeAdd(sp_reg, sp_reg, Imm(WORD_SIZE_BYTE * (CFrameSlots::Start() - CFrameData::Start())));

    GetMasm()->Pop(RegisterList(vixl::aarch32::r11, vixl::aarch32::lr));
    SET_CFI_OFFSET(pop_fplr, encoder->GetCursorOffset());

    encoder->EncodeReturn();
}
}  // namespace panda::compiler::aarch32
