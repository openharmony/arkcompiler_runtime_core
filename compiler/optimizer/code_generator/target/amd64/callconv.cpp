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
#include "target/amd64/target.h"

namespace panda::compiler::amd64 {

Amd64CallingConvention::Amd64CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr,
                                               CallConvMode mode)
    : CallingConvention(allocator, enc, descr, mode)
{
}

ParameterInfo *Amd64CallingConvention::GetParameterInfo(uint8_t regs_offset)
{
    auto param_info = GetAllocator()->New<amd64::Amd64ParameterInfo>();
    // reserve first parameter to method pointer
    for (int i = 0; i < regs_offset; ++i) {
        param_info->GetNativeParam(INT64_TYPE);
    }
    return param_info;
}

void *Amd64CallingConvention::GetCodeEntry()
{
    auto code = static_cast<Amd64Encoder *>(GetEncoder())->GetMasm()->code();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return reinterpret_cast<void *>(code->baseAddress());
}

uint32_t Amd64CallingConvention::GetCodeSize()
{
    return static_cast<Amd64Encoder *>(GetEncoder())->GetMasm()->code()->codeSize();
}

size_t Amd64CallingConvention::PushRegs(RegList regs, RegList vregs)
{
    size_t regs_count {0};
    size_t vregs_count {0};

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        uint32_t ii {MAX_NUM_REGS - i - 1};
        if (vregs.Has(ii)) {
            ++vregs_count;
            GetMasm()->sub(asmjit::x86::rsp, asmjit::imm(DOUBLE_WORD_SIZE_BYTE));
            GetMasm()->movsd(asmjit::x86::ptr(asmjit::x86::rsp), asmjit::x86::xmm(ii));
        }
    }

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        uint32_t ii {MAX_NUM_REGS - i - 1};
        if (regs.Has(ii)) {
            ++regs_count;
            GetMasm()->push(asmjit::x86::gpq(ConvertRegNumber(ii)));
        }
    }

    return vregs_count + regs_count;
}

size_t Amd64CallingConvention::PopRegs(RegList regs, RegList vregs)
{
    size_t regs_count {0};
    size_t vregs_count {0};

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if (regs.Has(i)) {
            ++regs_count;
            GetMasm()->pop(asmjit::x86::gpq(ConvertRegNumber(i)));
        }
    }

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if (vregs.Has(i)) {
            ++vregs_count;
            GetMasm()->movsd(asmjit::x86::xmm(i), asmjit::x86::ptr(asmjit::x86::rsp));
            GetMasm()->add(asmjit::x86::rsp, asmjit::imm(DOUBLE_WORD_SIZE_BYTE));
        }
    }

    return vregs_count + regs_count;
}

std::variant<Reg, uint8_t> Amd64ParameterInfo::GetNativeParam(const TypeInfo &type)
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

    return Target(Arch::X86_64).GetParamReg(current_scalar_number_++, type);
}

Location Amd64ParameterInfo::GetNextLocation(DataType::Type type)
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
    Target target(Arch::X86_64);
    return Location::MakeRegister(target.GetParamRegId(current_scalar_number_++));
}

void Amd64CallingConvention::GeneratePrologue([[maybe_unused]] const FrameInfo &frame_info)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto fp_reg = GetTarget().GetFrameReg();
    auto sp_reg = GetTarget().GetStackReg();

    // we do not push return address, because in amd64 call instruction already pushed it
    GetMasm()->push(asmjit::x86::rbp);  // frame pointer
    SET_CFI_OFFSET(push_fplr, encoder->GetCursorOffset());

    encoder->EncodeMov(fp_reg, sp_reg);
    SET_CFI_OFFSET(set_fp, encoder->GetCursorOffset());
    encoder->EncodeSub(sp_reg, sp_reg, Imm(2U * DOUBLE_WORD_SIZE_BYTE));

    encoder->EncodeStr(GetTarget().GetParamReg(0), MemRef(sp_reg, DOUBLE_WORD_SIZE_BYTE));

    // Reset OSR flag and set HasFloatRegsFlag
    auto flags {static_cast<uint64_t>(frame_info.GetHasFloatRegs()) << CFrameLayout::HasFloatRegsFlag::START_BIT};
    encoder->EncodeSti(Imm(flags), MemRef(sp_reg));
    // Allocate space for locals
    encoder->EncodeSub(sp_reg, sp_reg, Imm(DOUBLE_WORD_SIZE_BYTE * (CFrameSlots::Start() - CFrameData::Start())));
    static_assert((CFrameLayout::GetLocalsCount() & 1U) == 0);

    RegList callee_regs {GetCalleeRegsMask(Arch::X86_64, false).GetValue()};
    RegList callee_vregs {GetCalleeRegsMask(Arch::X86_64, true).GetValue()};
    SET_CFI_CALLEE_REGS(RegMask(static_cast<size_t>(callee_regs)));
    SET_CFI_CALLEE_VREGS(VRegMask(static_cast<size_t>(callee_vregs)));
    PushRegs(callee_regs, callee_vregs);
    SET_CFI_OFFSET(push_callees, encoder->GetCursorOffset());

    encoder->EncodeSub(
        sp_reg, sp_reg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            DOUBLE_WORD_SIZE_BYTE));
}

void Amd64CallingConvention::GenerateEpilogue([[maybe_unused]] const FrameInfo &frame_info,
                                              std::function<void()> post_job)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto sp_reg = GetTarget().GetStackReg();

    if (post_job) {
        post_job();
    }

    encoder->EncodeAdd(
        sp_reg, sp_reg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            DOUBLE_WORD_SIZE_BYTE));

    PopRegs(RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue()),
            RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue()));
    SET_CFI_OFFSET(pop_callees, encoder->GetCursorOffset());

    // X86_64 doesn't support OSR mode
    ASSERT(!IsOsrMode());
    // Support restoring of LR and FP registers once OSR is supported in x86_64
    static_assert(!ArchTraits<Arch::X86_64>::SUPPORT_OSR);
    constexpr auto SHIFT = DOUBLE_WORD_SIZE_BYTE * (2 + CFrameSlots::Start() - CFrameData::Start());
    encoder->EncodeAdd(sp_reg, sp_reg, Imm(SHIFT));

    GetMasm()->pop(asmjit::x86::rbp);  // frame pointer
    SET_CFI_OFFSET(pop_fplr, encoder->GetCursorOffset());
    GetMasm()->ret();
}
}  // namespace panda::compiler::amd64
