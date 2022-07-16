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
Encoder (implementation of math and mem Low-level emitters)
*/

#include <aarch32/disasm-aarch32.h>
#include "compiler/optimizer/code_generator/relocations.h"
#include "operands.h"
#include "target/aarch32/target.h"
#include "lib_helpers.inl"
#include "encode.h"

#ifndef PANDA_TARGET_MACOS
#include "elf.h"
#endif  // PANDA_TARGET_MACOS

#include <iomanip>

namespace panda::compiler::aarch32 {
void Aarch32LabelHolder::BindLabel(LabelId id)
{
    static_cast<Aarch32Encoder *>(GetEncoder())->GetMasm()->Bind(labels_[id]);
}

Aarch32Encoder::Aarch32Encoder(ArenaAllocator *allocator) : Encoder(allocator, Arch::AARCH32)
{
    labels_ = allocator->New<Aarch32LabelHolder>(this);
    if (labels_ == nullptr) {
        SetFalseResult();
    }
    EnableLrAsTempReg(true);
}

Aarch32Encoder::~Aarch32Encoder()
{
    auto labels = static_cast<Aarch32LabelHolder *>(GetLabels())->labels_;
    for (auto label : labels) {
        label->~Label();
    }
    if (masm_ != nullptr) {
        masm_->~MacroAssembler();
        masm_ = nullptr;
    }
}

bool Aarch32Encoder::InitMasm()
{
    if (masm_ == nullptr) {
        auto allocator = GetAllocator();

        // Initialize Masm
        masm_ = allocator->New<vixl::aarch32::MacroAssembler>(allocator);
        if (masm_ == nullptr || !masm_->IsValid()) {
            SetFalseResult();
            return false;
        }

        ASSERT(masm_);
        for (auto reg_code : AARCH32_TMP_REG) {
            masm_->GetScratchRegisterList()->Combine(vixl::aarch32::Register(reg_code));
        }
        for (auto vreg_code : AARCH32_TMP_VREG) {
            masm_->GetScratchVRegisterList()->Combine(vixl::aarch32::SRegister(vreg_code));
        }

        // Make sure that the compiler uses the same scratch registers as the assembler
        CHECK_EQ(RegMask(masm_->GetScratchRegisterList()->GetList()), GetTarget().GetTempRegsMask());
        CHECK_EQ(RegMask(masm_->GetScratchVRegisterList()->GetList()), GetTarget().GetTempVRegsMask());
    }
    return true;
}

void Aarch32Encoder::Finalize()
{
    GetMasm()->FinalizeCode();
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label);
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    CompareHelper(src0, src1, &cc);
    GetMasm()->B(Convert(cc), label);
}

void Aarch32Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    TestHelper(src0, src1, cc);
    GetMasm()->B(ConvertTest(cc), label);
}

void Aarch32Encoder::EncodeBitTestAndBranch(LabelHolder::LabelId id, Reg reg, uint32_t bit_pos, bool bit_value)
{
    ASSERT(reg.IsScalar() && reg.GetSize() > bit_pos);
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    if (reg.GetSize() == DOUBLE_WORD_SIZE) {
        if (bit_pos < WORD_SIZE) {
            GetMasm()->tst(VixlReg(reg), VixlImm(1U << bit_pos));
        } else {
            GetMasm()->tst(VixlRegU(reg), VixlImm(1U << (bit_pos - WORD_SIZE)));
        }
    } else {
        GetMasm()->tst(VixlReg(reg), VixlImm(1U << bit_pos));
    }
    if (bit_value) {
        GetMasm()->B(Convert(Condition::NE), label);
    } else {
        GetMasm()->B(Convert(Condition::EQ), label);
    }
}

bool Aarch32Encoder::CompareImmHelper(Reg src, Imm imm, Condition *cc)
{
    auto value = GetIntValue(imm);
    ASSERT(src.IsScalar());
    ASSERT(value != 0);
    ASSERT(-static_cast<int64_t>(UINT32_MAX) <= value && value <= UINT32_MAX);
    ASSERT(CanEncodeImmAddSubCmp(value, src.GetSize(), IsConditionSigned(*cc)));

    return value < 0 ? CompareNegImmHelper(src, value, cc) : ComparePosImmHelper(src, value, cc);
}

void Aarch32Encoder::TestImmHelper(Reg src, Imm imm, [[maybe_unused]] Condition cc)
{
    auto value = GetIntValue(imm);
    ASSERT(src.IsScalar());
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);
    ASSERT(CanEncodeImmLogical(value, src.GetSize()));

    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Tst(VixlReg(src), VixlImm(value));
    } else {
        GetMasm()->Tst(VixlRegU(src), VixlImm(0x0));
        GetMasm()->Tst(Convert(Condition::EQ), VixlReg(src), VixlImm(value));
    }
}

bool Aarch32Encoder::CompareNegImmHelper(Reg src, int64_t value, const Condition *cc)
{
    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmn(VixlReg(src), VixlImm(-value));
    } else {
        if (!IsConditionSigned(*cc)) {
            GetMasm()->Cmn(VixlRegU(src), VixlImm(0x1));
            GetMasm()->Cmn(Convert(Condition::EQ), VixlReg(src), VixlImm(-value));
        } else {
            // There are no effective implementation in this case
            // Can't get here because of logic behind CanEncodeImmAddSubCmp
            UNREACHABLE();
            SetFalseResult();
            return false;
        }
    }
    return true;
}

bool Aarch32Encoder::ComparePosImmHelper(Reg src, int64_t value, Condition *cc)
{
    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), VixlImm(value));
    } else {
        if (!IsConditionSigned(*cc)) {
            GetMasm()->Cmp(VixlRegU(src), VixlImm(0x0));
            GetMasm()->Cmp(Convert(Condition::EQ), VixlReg(src), VixlImm(value));
        } else {
            bool swap = false;
            switch (*cc) {
                case Condition::GT:
                    swap = true;
                    *cc = Condition::LT;
                    break;
                case Condition::LE:
                    swap = true;
                    *cc = Condition::GE;
                    break;
                case Condition::GE:
                case Condition::LT:
                    break;
                default:
                    UNREACHABLE();
            }

            ScopedTmpRegU32 tmp_reg(this);
            if (swap) {
                GetMasm()->Rsbs(VixlReg(tmp_reg), VixlReg(src), VixlImm(value));
                GetMasm()->Rscs(VixlReg(tmp_reg), VixlRegU(src), vixl::aarch32::Operand(0x0));
            } else {
                GetMasm()->Cmp(VixlReg(src), VixlImm(value));
                GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(src), vixl::aarch32::Operand(0x0));
            }
        }
    }
    return true;
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    auto value = GetIntValue(imm);
    if (value == 0) {
        EncodeJump(id, src, cc);
        return;
    }

    if (!CompareImmHelper(src, imm, &cc)) {
        return;
    }

    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(Convert(cc), label);
}

void Aarch32Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    TestImmHelper(src, imm, cc);
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(ConvertTest(cc), label);
}

void Aarch32Encoder::CompareZeroHelper(Reg src, Condition *cc)
{
    ASSERT(src.IsScalar());
    if (src.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), vixl::aarch32::Operand(0x0));
    } else {
        ScopedTmpRegU32 tmp_reg(this);
        uint32_t imm = 0x0;

        switch (*cc) {
            case Condition::EQ:
            case Condition::NE:
                GetMasm()->Orrs(VixlReg(tmp_reg), VixlReg(src), VixlRegU(src));
                break;
            case Condition::LE:
                imm = 0x1;
                *cc = Condition::LT;
                /* fallthrough */
                [[fallthrough]];
            case Condition::LT:
                GetMasm()->Cmp(VixlReg(src), vixl::aarch32::Operand(imm));
                GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(src), vixl::aarch32::Operand(0x0));
                break;
            case Condition::GT:
                imm = 0x1;
                *cc = Condition::GE;
                /* fallthrough */
                [[fallthrough]];
            case Condition::GE:
                GetMasm()->Cmp(VixlReg(src), vixl::aarch32::Operand(imm));
                GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(src), vixl::aarch32::Operand(0x0));
                break;
            default:
                UNREACHABLE();
        }
    }
}

void Aarch32Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc)
{
    auto label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(id);
    ASSERT(src.IsScalar());

    switch (cc) {
        case Condition::LO:
            // LO is always false
            return;
        case Condition::HS:
            // HS is always true
            GetMasm()->B(label);
            return;
        case Condition::LS:
            // LS is same as EQ
            cc = Condition::EQ;
            break;
        case Condition::HI:
            // HI is same as NE
            cc = Condition::NE;
            break;
        default:
            break;
    }

    CompareZeroHelper(src, &cc);

    GetMasm()->B(Convert(cc), label);
}

void Aarch32Encoder::EncodeJump(Reg dst)
{
    GetMasm()->Bx(VixlReg(dst));
}

void Aarch32Encoder::EncodeJump([[maybe_unused]] RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_ARM_CALL;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0xeafffffe;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch32Encoder::EncodeNop()
{
    GetMasm()->Nop();
}

void Aarch32Encoder::MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_ARM_CALL;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0xebfffffe;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch32Encoder::MakeCall(const void *entry_point)
{
    ScopedTmpRegU32 tmp_reg(this);

    auto entry = static_cast<int32_t>(reinterpret_cast<int64_t>(entry_point));
    EncodeMov(tmp_reg, Imm(entry));
    GetMasm()->Blx(VixlReg(tmp_reg));
}

void Aarch32Encoder::MakeCall(MemRef entry_point)
{
    ScopedTmpRegU32 tmp_reg(this);

    EncodeLdr(tmp_reg, false, entry_point);
    GetMasm()->Blx(VixlReg(tmp_reg));
}

void Aarch32Encoder::MakeCall(Reg reg)
{
    GetMasm()->Blx(VixlReg(reg));
}

void Aarch32Encoder::MakeCallAot([[maybe_unused]] intptr_t offset)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::MakeCallByOffset([[maybe_unused]] intptr_t offset)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::MakeLoadAotTable([[maybe_unused]] intptr_t offset, [[maybe_unused]] Reg reg)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::MakeLoadAotTableAddr([[maybe_unused]] intptr_t offset, [[maybe_unused]] Reg addr,
                                          [[maybe_unused]] Reg val)
{
    // Unimplemented
    SetFalseResult();
}

void Aarch32Encoder::EncodeReturn()
{
    GetMasm()->Bx(vixl::aarch32::lr);
}

void Aarch32Encoder::EncodeAbort()
{
    GetMasm()->Udf(0);
}

void Aarch32Encoder::EncodeMul([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Imm imm)
{
    SetFalseResult();
}

/* static */
bool Aarch32Encoder::IsNeedToPrepareMemLdS(MemRef mem, const TypeInfo &mem_type, bool is_signed)
{
    bool has_index = mem.HasIndex();
    bool has_shift = mem.HasScale();
    bool has_offset = mem.HasDisp();
    ASSERT(mem.HasBase());
    // VLDR and VSTR has base and offset. The offset must be a multiple of 4, and lie in the range -1020 to +1020.
    if (mem_type.IsFloat()) {
        if (has_index) {
            return true;
        }
        if (has_offset) {
            auto offset = mem.GetDisp();
            constexpr int32_t IMM_4 = 4;
            if (!((offset >= -VMEM_OFFSET) && (offset <= VMEM_OFFSET) && ((offset % IMM_4) == 0))) {
                return true;
            }
        }
        return false;
    }
    int32_t max_offset = MEM_SMALL_OFFSET;
    bool has_lsl = false;
    if (!is_signed && (mem_type == INT32_TYPE || mem_type == INT8_TYPE)) {
        has_lsl = true;
        max_offset = MEM_BIG_OFFSET;
    }
    // LDR/STR with register offset(for A32)
    // |  mem type  |    shift    |
    // | ---------- | ----------- |
    // |word or byte|    0 to 31  |
    // |   others   |      --     |
    //
    if (has_index) {
        // MemRef with index and offser isn't supported
        ASSERT(!has_offset);
        if (has_shift) {
            ASSERT(mem.GetScale() != 0);
            [[maybe_unused]] constexpr int32_t MAX_SHIFT = 3;
            ASSERT(mem.GetScale() <= MAX_SHIFT);
            if (!has_lsl) {
                return true;
            }
        }
        return false;
    }
    // LDR/STR with immediate offset(for A32)
    // |  mem type  | offset size |
    // | ---------- | ----------- |
    // |word or byte|-4095 to 4095|
    // |   others   | -255 to 255 |
    //
    if (has_offset) {
        ASSERT(mem.GetDisp() != 0);
        [[maybe_unused]] auto offset = mem.GetDisp();
        if (!((offset >= -max_offset) && (offset <= max_offset))) {
            return true;
        }
    }
    return false;
}

/**
 * The function construct additional instruction for encode memory instructions and returns MemOperand for ldr/str
 * LDR/STR with immediate offset(for A32)
 * |  mem type  | offset size |
 * | ---------- | ----------- |
 * |word or byte|-4095 to 4095|
 * |   others   | -255 to 255 |
 *
 * LDR/STR with register offset(for A32)
 * |  mem type  |    shift    |
 * | ---------- | ----------- |
 * |word or byte|    0 to 31  |
 * |   others   |      --     |
 *
 * VLDR and VSTR has base and offset. The offset must be a multiple of 4, and lie in the range -1020 to +1020.
 */
vixl::aarch32::MemOperand Aarch32Encoder::PrepareMemLdS(MemRef mem, const TypeInfo &mem_type,
                                                        vixl::aarch32::Register tmp, bool is_signed, bool copy_sp)
{
    bool has_index = mem.HasIndex();
    bool has_shift = mem.HasScale();
    bool has_offset = mem.HasDisp();
    ASSERT(mem.HasBase());
    auto base_reg = VixlReg(mem.GetBase());
    if (copy_sp) {
        if (base_reg.IsSP()) {
            GetMasm()->Mov(tmp, base_reg);
            base_reg = tmp;
        }
    }
    // VLDR and VSTR has base and offset. The offset must be a multiple of 4, and lie in the range -1020 to +1020.
    if (mem_type.IsFloat()) {
        return PrepareMemLdSForFloat(mem, tmp);
    }
    int32_t max_offset = MEM_SMALL_OFFSET;
    bool has_lsl = false;
    if (!is_signed && (mem_type == INT32_TYPE || mem_type == INT8_TYPE)) {
        has_lsl = true;
        max_offset = MEM_BIG_OFFSET;
    }
    // LDR/STR with register offset(for A32)
    // |  mem type  |    shift    |
    // | ---------- | ----------- |
    // |word or byte|    0 to 31  |
    // |   others   |      --     |
    //
    if (has_index) {
        // MemRef with index and offser isn't supported
        ASSERT(!has_offset);
        auto index_reg = mem.GetIndex();
        if (has_shift) {
            ASSERT(mem.GetScale() != 0);
            auto shift = mem.GetScale();
            [[maybe_unused]] constexpr int32_t MAX_SHIFT = 3;
            ASSERT(mem.GetScale() <= MAX_SHIFT);
            if (has_lsl) {
                return vixl::aarch32::MemOperand(base_reg, VixlReg(index_reg), vixl::aarch32::LSL, shift);
            }
            // from:
            //   mem: base, index, scale
            // to:
            //   add tmp, base, index, scale
            //   mem tmp
            GetMasm()->Add(tmp, base_reg, vixl::aarch32::Operand(VixlReg(index_reg), vixl::aarch32::LSL, shift));
            return vixl::aarch32::MemOperand(tmp);
        }
        return vixl::aarch32::MemOperand(base_reg, VixlReg(index_reg));
    }
    // LDR/STR with immediate offset(for A32):
    // |  mem type  | offset size |
    // | ---------- | ----------- |
    // |word or byte|-4095 to 4095|
    // |   others   | -255 to 255 |
    //
    if (has_offset) {
        ASSERT(mem.GetDisp() != 0);
        auto offset = mem.GetDisp();
        if ((offset >= -max_offset) && (offset <= max_offset)) {
            return vixl::aarch32::MemOperand(base_reg, offset);
        }
        // from:
        //   mem: base, offset
        // to:
        //   add tmp, base, offset
        //   mem tmp
        GetMasm()->Add(tmp, base_reg, VixlImm(offset));
        base_reg = tmp;
    }
    return vixl::aarch32::MemOperand(base_reg);
}

vixl::aarch32::MemOperand Aarch32Encoder::PrepareMemLdSForFloat(MemRef mem, vixl::aarch32::Register tmp)
{
    bool has_index = mem.HasIndex();
    bool has_shift = mem.HasScale();
    bool has_offset = mem.HasDisp();
    auto base_reg = VixlReg(mem.GetBase());
    if (has_index) {
        auto index_reg = mem.GetIndex();
        auto scale = mem.GetScale();
        // from:
        //   vmem: base, index, scale, offset
        // to:
        //   add tmp, base, index, scale
        //   vmem tmp, offset
        if (has_shift) {
            ASSERT(scale != 0);
            [[maybe_unused]] constexpr int32_t MAX_SHIFT = 3;
            ASSERT(scale > 0 && scale <= MAX_SHIFT);
            GetMasm()->Add(tmp, base_reg, vixl::aarch32::Operand(VixlReg(index_reg), vixl::aarch32::LSL, scale));
        } else {
            ASSERT(scale == 0);
            GetMasm()->Add(tmp, base_reg, VixlReg(index_reg));
        }
        base_reg = tmp;
    }
    if (has_offset) {
        ASSERT(mem.GetDisp() != 0);
        auto offset = mem.GetDisp();
        constexpr int32_t IMM_4 = 4;
        if ((offset >= -VMEM_OFFSET) && (offset <= VMEM_OFFSET) && ((offset % IMM_4) == 0)) {
            return vixl::aarch32::MemOperand(base_reg, offset);
        }
        // from:
        //   vmem: base, offset
        // to:
        //   add tmp, base, offset
        //   vmem tmp
        GetMasm()->Add(tmp, base_reg, VixlImm(offset));
        base_reg = tmp;
    }
    return vixl::aarch32::MemOperand(base_reg);
}

void Aarch32Encoder::EncodeFpToBits(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    if (dst.GetSize() == WORD_SIZE) {
        ASSERT(src.GetSize() == WORD_SIZE);

        constexpr uint32_t NANF = 0x7fc00000U;

        GetMasm()->Vcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
        GetMasm()->Vmov(VixlReg(dst), VixlVReg(src).S());
        GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(NANF));
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);

        constexpr uint32_t NAND_HIGH = 0x7ff80000U;

        GetMasm()->Vcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
        GetMasm()->Vmov(VixlReg(dst), VixlRegU(dst), VixlVReg(src).D());
        GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0));
        GetMasm()->Mov(Convert(Condition::NE), VixlRegU(dst), VixlImm(NAND_HIGH));
    }
}

void Aarch32Encoder::EncodeMoveBitsRaw(Reg dst, Reg src)
{
    ASSERT((dst.IsFloat() && src.IsScalar()) || (src.IsFloat() && dst.IsScalar()));
    if (dst.IsScalar()) {
        ASSERT(src.GetSize() == dst.GetSize());
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlReg(dst), VixlVReg(src).S());
        } else {
            GetMasm()->Vmov(VixlReg(dst), VixlRegU(dst), VixlVReg(src).D());
        }
    } else {
        ASSERT(dst.GetSize() == src.GetSize());
        if (src.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlReg(src));
        } else {
            GetMasm()->Vmov(VixlVReg(dst).D(), VixlReg(src), VixlRegU(src));
        }
    }
}
void Aarch32Encoder::EncodeMov(Reg dst, Reg src)
{
    if (src.GetSize() <= WORD_SIZE && dst.GetSize() == DOUBLE_WORD_SIZE && !src.IsFloat() && !dst.IsFloat()) {
        SetFalseResult();
        return;
    }
    if (dst == src) {
        return;
    }
    if (dst.IsFloat()) {
        if (src.GetType().IsScalar()) {
            if (src.GetSize() == DOUBLE_WORD_SIZE) {
                GetMasm()->Vmov(VixlVReg(dst).D(), VixlReg(src), VixlRegU(src));
                return;
            }
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlReg(src));
            return;
        }
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlVReg(src));
        } else {
            GetMasm()->Vmov(VixlVReg(dst).D(), VixlVReg(src));
        }
        return;
    }
    ASSERT(dst.IsScalar());
    if (src.IsFloat()) {
        if (src.GetSize() == DOUBLE_WORD_SIZE) {
            GetMasm()->Vmov(VixlReg(dst), VixlRegU(dst), VixlVReg(src).D());
            return;
        }
        GetMasm()->Vmov(VixlReg(dst), VixlVReg(src).S());
        return;
    }
    ASSERT(src.IsScalar());
    GetMasm()->Mov(VixlReg(dst), VixlReg(src));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(VixlRegU(dst), VixlRegU(src));
    }
}

void Aarch32Encoder::EncodeNeg(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Vneg(VixlVReg(dst), VixlVReg(src));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Rsb(VixlReg(dst), VixlReg(src), VixlImm(0x0));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Rsbs(VixlReg(dst), VixlReg(src), VixlImm(0x0));
    GetMasm()->Rsc(VixlRegU(dst), VixlRegU(src), VixlImm(0x0));
}

void Aarch32Encoder::EncodeAbs(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Vabs(VixlVReg(dst), VixlVReg(src));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), VixlImm(0x0));
        GetMasm()->Rsb(Convert(Condition::MI), VixlReg(dst), VixlReg(src), VixlImm(0x0));
        GetMasm()->Mov(Convert(Condition::PL), VixlReg(dst), VixlReg(src));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 lo_reg(this);
    ScopedTmpRegU32 hi_reg(this);

    GetMasm()->Rsbs(VixlReg(lo_reg), VixlReg(src), VixlImm(0x0));
    GetMasm()->Rsc(VixlReg(hi_reg), VixlRegU(src), VixlImm(0x0));
    GetMasm()->Cmp(VixlRegU(src), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(lo_reg), VixlReg(src));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(hi_reg), VixlRegU(src));

    GetMasm()->Mov(VixlReg(dst), VixlReg(lo_reg));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hi_reg));
}

void Aarch32Encoder::EncodeSqrt(Reg dst, Reg src)
{
    ASSERT(dst.IsFloat());
    GetMasm()->Vsqrt(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    GetMasm()->Mvn(VixlReg(dst), VixlReg(src));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mvn(VixlRegU(dst), VixlRegU(src));
    }
}

void Aarch32Encoder::EncodeIsInf(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    if (src.GetSize() == WORD_SIZE) {
        constexpr uint32_t INF_MASK = 0xff000000;

        ScopedTmpRegU32 tmp(this);

        GetMasm()->Vmov(VixlReg(tmp), VixlVReg(src).S());
        GetMasm()->Lsl(VixlReg(tmp), VixlReg(tmp), 1); /* 0xff000000 if Infinity */
        GetMasm()->Mov(VixlReg(dst), INF_MASK);
        GetMasm()->Cmp(VixlReg(dst), VixlReg(tmp));
    } else {
        constexpr uint32_t INF_MASK = 0xffe00000;

        ScopedTmpRegU32 tmp(this);
        ScopedTmpRegU32 tmp1(this);

        GetMasm()->Vmov(VixlReg(tmp), VixlReg(tmp1), VixlVReg(src).D());
        GetMasm()->Lsl(VixlReg(tmp1), VixlReg(tmp1), 1); /* 0xffe00000 if Infinity */
        GetMasm()->Mov(VixlReg(dst), INF_MASK);
        GetMasm()->Cmp(VixlReg(dst), VixlReg(tmp1));
    }

    GetMasm()->Mov(VixlReg(dst), VixlImm(0));
    GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(1));
}

void Aarch32Encoder::EncodeReverseBytes(Reg dst, Reg src)
{
    ASSERT(src.GetSize() > BYTE_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Rev16(VixlReg(dst), VixlReg(src));
        GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
    } else if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Rev(VixlReg(dst), VixlReg(src));
    } else {
        if (src == dst) {
            ScopedTmpRegU32 tmp_reg(this);
            GetMasm()->Mov(VixlReg(tmp_reg), VixlReg(src));
            GetMasm()->Rev(VixlReg(dst), VixlRegU(src));
            GetMasm()->Rev(VixlRegU(dst), VixlReg(tmp_reg));
        } else {
            GetMasm()->Rev(VixlRegU(dst), VixlReg(src));
            GetMasm()->Rev(VixlReg(dst), VixlRegU(src));
        }
    }
}

void Aarch32Encoder::EncodeBitCount([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
{
    SetFalseResult();
}

void Aarch32Encoder::EncodeCountLeadingZeroBits(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);
    if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Clz(VixlReg(dst), VixlReg(src));
        return;
    }

    auto low = CreateLabel();
    auto end = CreateLabel();
    auto high_bits = Reg(src.GetId() + 1, INT32_TYPE);
    EncodeJump(low, high_bits, Condition::EQ);
    GetMasm()->Clz(VixlReg(dst), VixlReg(high_bits));
    EncodeJump(end);

    BindLabel(low);
    GetMasm()->Clz(VixlReg(dst), VixlReg(src));
    GetMasm()->Adds(VixlReg(dst), VixlReg(dst), VixlImm(WORD_SIZE));

    BindLabel(end);
}

void Aarch32Encoder::EncodeCeil(Reg dst, Reg src)
{
    GetMasm()->Vrintp(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeFloor(Reg dst, Reg src)
{
    GetMasm()->Vrintm(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeRint(Reg dst, Reg src)
{
    GetMasm()->Vrintn(VixlVReg(dst), VixlVReg(src));
}

void Aarch32Encoder::EncodeRound(Reg dst, Reg src)
{
    ScopedTmpRegF64 temp(this);
    vixl::aarch32::SRegister temp1(temp.GetReg().GetId());
    vixl::aarch32::SRegister temp2(temp.GetReg().GetId() + 1);

    auto done = CreateLabel();
    // round to nearest integer, ties away from zero
    GetMasm()->Vcvta(vixl::aarch32::DataTypeValue::S32, vixl::aarch32::DataTypeValue::F32, temp1, VixlVReg(src).S());
    GetMasm()->Vmov(VixlReg(dst), temp1);
    // for positive, zero and NaN inputs, rounding is done
    GetMasm()->Cmp(VixlReg(dst), 0);
    GetMasm()->B(vixl::aarch32::ge, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(done));
    // if input is negative but not a tie, round to nearest is valid.
    // if input is a negative tie, change rounding direction to positive infinity, dst += 1.
    GetMasm()->Vrinta(vixl::aarch32::DataTypeValue::F32, temp1, VixlVReg(src).S());
    // NOLINTNEXTLINE(readability-magic-numbers)
    const auto HALF = 0.5;
    GetMasm()->Vmov(temp2, HALF);
    GetMasm()->Vsub(vixl::aarch32::DataTypeValue::F32, temp1, VixlVReg(src).S(), temp1);
    GetMasm()->Vcmp(vixl::aarch32::DataTypeValue::F32, temp1, temp2);
    GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    GetMasm()->add(vixl::aarch32::eq, VixlReg(dst), VixlReg(dst), 1);

    BindLabel(done);
}

void Aarch32Encoder::EncodeReverseBits(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Rbit(VixlReg(dst), VixlReg(src));
        return;
    }

    if (src == dst) {
        ScopedTmpRegU32 tmp_reg(this);
        GetMasm()->Mov(VixlReg(tmp_reg), VixlReg(src));
        GetMasm()->Rbit(VixlReg(dst), VixlRegU(src));
        GetMasm()->Rbit(VixlRegU(dst), VixlReg(tmp_reg));
        return;
    }

    GetMasm()->Rbit(VixlRegU(dst), VixlReg(src));
    GetMasm()->Rbit(VixlReg(dst), VixlRegU(src));
}

void Aarch32Encoder::EncodeCastToBool(Reg dst, Reg src)
{
    // In ISA says that we only support casts:
    // i32tou1, i64tou1, u32tou1, u64tou1
    ASSERT(src.IsScalar());
    ASSERT(dst.IsScalar());

    GetMasm()->Cmp(VixlReg(src), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0x1));
    if (src.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Cmp(VixlRegU(src), VixlImm(0x0));
        GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0x1));
    }
}

void Aarch32Encoder::EncodeCast(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    // float/double -> float/double
    if (dst.IsFloat() && src.IsFloat()) {
        EncodeCastFloatToFloat(dst, src);
        return;
    }

    // uint/int -> float/double
    if (dst.IsFloat() && src.IsScalar()) {
        EncodeCastScalarToFloat(dst, src, src_signed);
        return;
    }

    // float/double -> uint/int
    if (dst.IsScalar() && src.IsFloat()) {
        EncodeCastFloatToScalar(dst, dst_signed, src);
        return;
    }

    // uint/int -> uint/int
    ASSERT(dst.IsScalar() && src.IsScalar());
    EncodeCastScalar(dst, dst_signed, src, src_signed);
}

void Aarch32Encoder::EncodeCastScalar(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    size_t src_size = src.GetSize();
    size_t dst_size = dst.GetSize();
    // In our ISA minimal type is 32-bit, so type less then 32-bit
    // we should extend to 32-bit. So we can have 2 cast
    // (For examble, i8->u16 will work as i8->u16 and u16->u32)
    if (dst_size < WORD_SIZE) {
        if (src_size > dst_size) {
            if (dst_signed) {
                EncodeCastScalarFromSignedScalar(dst, src);
            } else {
                EncodeCastScalarFromUnsignedScalar(dst, src);
            }
            return;
        }
        if (src_size == dst_size) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (src_signed == dst_signed) {
                return;
            }
            if (dst_signed) {
                EncodeCastScalarFromSignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            } else {
                EncodeCastScalarFromUnsignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            }
            return;
        }
        if (src_signed) {
            EncodeCastScalarFromSignedScalar(dst, src);
            if (!dst_signed) {
                EncodeCastScalarFromUnsignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        } else {
            EncodeCastScalarFromUnsignedScalar(dst, src);
            if (dst_signed) {
                EncodeCastScalarFromSignedScalar(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        }
    } else {
        if (src_size == dst_size) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (src_size == DOUBLE_WORD_SIZE) {
                GetMasm()->Mov(VixlRegU(dst), VixlRegU(src));
            }
            return;
        }

        if (src_signed) {
            EncodeCastScalarFromSignedScalar(dst, src);
        } else {
            EncodeCastScalarFromUnsignedScalar(dst, src);
        }
    }
}

void Aarch32Encoder::EncodeCastFloatToFloat(Reg dst, Reg src)
{
    // float/double -> float/double
    if (dst.GetSize() == src.GetSize()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(VixlVReg(dst).S(), VixlVReg(src));
        } else {
            GetMasm()->Vmov(VixlVReg(dst).D(), VixlVReg(src));
        }
        return;
    }

    // double -> float
    if (dst.GetSize() == WORD_SIZE) {
        GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F32, vixl::aarch32::DataTypeValue::F64, VixlVReg(dst).S(),
                        VixlVReg(src).D());
    } else {
        // float -> double
        GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F64, vixl::aarch32::DataTypeValue::F32, VixlVReg(dst).D(),
                        VixlVReg(src).S());
    }
}

void Aarch32Encoder::EncodeCastScalarToFloat(Reg dst, Reg src, bool src_signed)
{
    // uint/int -> float/double
    switch (src.GetSize()) {
        case BYTE_SIZE:
        case HALF_SIZE:
        case WORD_SIZE: {
            ScopedTmpRegF32 tmp_reg(this);

            GetMasm()->Vmov(VixlVReg(tmp_reg).S(), VixlReg(src));
            auto data_type = src_signed ? vixl::aarch32::DataTypeValue::S32 : vixl::aarch32::DataTypeValue::U32;
            if (dst.GetSize() == WORD_SIZE) {
                GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F32, data_type, VixlVReg(dst).S(), VixlVReg(tmp_reg).S());
            } else {
                GetMasm()->Vcvt(vixl::aarch32::DataTypeValue::F64, data_type, VixlVReg(dst).D(), VixlVReg(tmp_reg).S());
            }
            break;
        }
        case DOUBLE_WORD_SIZE: {
            if (dst.GetSize() == WORD_SIZE) {
                if (src_signed) {
                    // int64 -> float
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_l2f));
                } else {
                    // uint64 -> float
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_ul2f));
                }
            } else {
                if (src_signed) {
                    // int64 -> double
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_l2d));
                } else {
                    // uint64 -> double
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_ul2d));
                }
            }
            break;
        }
        default:
            SetFalseResult();
            break;
    }
}

void Aarch32Encoder::EncodeCastFloatToScalar(Reg dst, bool dst_signed, Reg src)
{
    // We DON'T support casts from float32/64 to int8/16 and bool, because this caste is not declared anywhere
    // in other languages and architecture, we do not know what the behavior should be.
    // But there is one implementation in other function: "EncodeCastFloatToScalarWithSmallDst". Call it in the
    // "EncodeCast" function instead of "EncodeCastFloat". It works as follows: cast from float32/64 to int32, moving
    // sign bit from int32 to dst type, then extend number from dst type to int32 (a necessary condition for an isa).
    // All work in dst register.
    ASSERT(dst.GetSize() >= WORD_SIZE);

    switch (dst.GetSize()) {
        case WORD_SIZE: {
            ScopedTmpRegF32 tmp_reg(this);

            auto data_type = dst_signed ? vixl::aarch32::DataTypeValue::S32 : vixl::aarch32::DataTypeValue::U32;
            if (src.GetSize() == WORD_SIZE) {
                GetMasm()->Vcvt(data_type, vixl::aarch32::DataTypeValue::F32, VixlVReg(tmp_reg).S(), VixlVReg(src).S());
            } else {
                GetMasm()->Vcvt(data_type, vixl::aarch32::DataTypeValue::F64, VixlVReg(tmp_reg).S(), VixlVReg(src).D());
            }

            GetMasm()->Vmov(VixlReg(dst), VixlVReg(tmp_reg).S());
            break;
        }
        case DOUBLE_WORD_SIZE: {
            if (src.GetSize() == WORD_SIZE) {
                if (dst_signed) {
                    // float -> int64
                    EncodeCastFloatToInt64(dst, src);
                } else {
                    // float -> uint64
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_f2ulz));
                }
            } else {
                if (dst_signed) {
                    // double -> int64
                    EncodeCastDoubleToInt64(dst, src);
                } else {
                    // double -> uint64
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_d2ulz));
                }
            }
            break;
        }
        default:
            SetFalseResult();
            break;
    }
}

void Aarch32Encoder::EncodeCastFloatToScalarWithSmallDst(Reg dst, bool dst_signed, Reg src)
{
    switch (dst.GetSize()) {
        case BYTE_SIZE:
        case HALF_SIZE:
        case WORD_SIZE: {
            ScopedTmpRegF32 tmp_reg(this);

            auto data_type = dst_signed ? vixl::aarch32::DataTypeValue::S32 : vixl::aarch32::DataTypeValue::U32;
            if (src.GetSize() == WORD_SIZE) {
                GetMasm()->Vcvt(data_type, vixl::aarch32::DataTypeValue::F32, VixlVReg(tmp_reg).S(), VixlVReg(src).S());
            } else {
                GetMasm()->Vcvt(data_type, vixl::aarch32::DataTypeValue::F64, VixlVReg(tmp_reg).S(), VixlVReg(src).D());
            }

            GetMasm()->Vmov(VixlReg(dst), VixlVReg(tmp_reg).S());
            if (dst.GetSize() < WORD_SIZE) {
                EncoderCastExtendFromInt32(dst, dst_signed);
            }
            break;
        }
        case DOUBLE_WORD_SIZE: {
            if (src.GetSize() == WORD_SIZE) {
                if (dst_signed) {
                    // float -> int64
                    EncodeCastFloatToInt64(dst, src);
                } else {
                    // float -> uint64
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_f2ulz));
                }
            } else {
                if (dst_signed) {
                    // double -> int64
                    EncodeCastDoubleToInt64(dst, src);
                } else {
                    // double -> uint64
                    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_d2ulz));
                }
            }
            break;
        }
        default:
            SetFalseResult();
            break;
    }
}

void Aarch32Encoder::EncoderCastExtendFromInt32(Reg dst, bool dst_signed)
{
    if (dst_signed) {
        constexpr uint32_t TEST_BIT = (1U << (static_cast<uint32_t>(WORD_SIZE) - 1));
        ScopedTmpReg tmp_reg(this, dst.GetType());

        uint32_t set_bit = (dst.GetSize() == BYTE_SIZE) ? (1U << static_cast<uint32_t>(BYTE_SIZE - 1))
                                                        : (1U << static_cast<uint32_t>(HALF_SIZE - 1));
        uint32_t rem_bit = set_bit - 1;
        GetMasm()->And(VixlReg(tmp_reg), VixlReg(dst), TEST_BIT);
        auto label_skip = CreateLabel();
        auto label_end_if = CreateLabel();
        EncodeJump(label_skip, tmp_reg, Condition::EQ);
        // Set signed bit in dst reg (accordingly destination type)
        // If signed bit == 1
        GetMasm()->Orr(VixlReg(dst), VixlReg(dst), set_bit);
        EncodeJump(label_end_if);
        BindLabel(label_skip);
        // If signed bit == 0
        GetMasm()->And(VixlReg(dst), VixlReg(dst), rem_bit);
        BindLabel(label_end_if);
    }
    EncodeCastScalar(Reg(dst.GetId(), INT32_TYPE), dst_signed, dst, dst_signed);
}

void Aarch32Encoder::EncodeCastDoubleToInt64(Reg dst, Reg src)
{
    auto label_check_nan = CreateLabel();
    auto label_not_nan = CreateLabel();
    auto label_exit = CreateLabel();
    ScopedTmpReg tmp_reg1(this, INT32_TYPE);

    // Mov double to 2x reg to storage double in hex format and work with them
    EncodeMov(dst, src);
    // See the exponent of number
    GetMasm()->Ubfx(VixlReg(tmp_reg1), VixlRegU(dst), START_EXP_DOUBLE, SIZE_EXP_DOUBLE);
    // Max exponent that we can load in int64
    // Check that x > MIN_INT64 & x < MAX_INT64, else jump
    GetMasm()->Cmp(VixlReg(tmp_reg1), VixlImm(POSSIBLE_EXP_DOUBLE));
    // If greater than or equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::hs, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(label_check_nan));
    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_d2lz));
    EncodeJump(label_exit);

    BindLabel(label_check_nan);
    // Form of nan number
    GetMasm()->Cmp(VixlReg(tmp_reg1), VixlImm(UP_BITS_NAN_DOUBLE));
    // If not equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::ne, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(label_not_nan));

    GetMasm()->Orrs(VixlReg(tmp_reg1), VixlReg(dst),
                    vixl::aarch32::Operand(VixlRegU(dst), vixl::aarch32::LSL, SHIFT_BITS_DOUBLE));
    auto addr_label = static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(label_not_nan);
    GetMasm()->B(vixl::aarch32::eq, addr_label);
    GetMasm()->Mov(VixlReg(dst), VixlImm(0));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(0));
    EncodeJump(label_exit);

    BindLabel(label_not_nan);
    GetMasm()->Adds(VixlRegU(dst), VixlRegU(dst), VixlRegU(dst));
    GetMasm()->Mov(VixlReg(dst), VixlImm(UINT32_MAX));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(INT32_MAX));
    GetMasm()->Adc(VixlReg(dst), VixlReg(dst), VixlImm(0));
    // If exponent negative, transform maxint64 to minint64
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(dst), VixlImm(0));

    BindLabel(label_exit);
}

void Aarch32Encoder::EncodeCastFloatToInt64(Reg dst, Reg src)
{
    auto label_check_nan = CreateLabel();
    auto label_not_nan = CreateLabel();
    auto label_exit = CreateLabel();

    ScopedTmpReg tmp_reg(this, INT32_TYPE);
    ScopedTmpReg moved_src(this, INT32_TYPE);
    EncodeMov(moved_src, src);
    // See the exponent of number
    GetMasm()->Ubfx(VixlReg(tmp_reg), VixlReg(moved_src), START_EXP_FLOAT, SIZE_EXP_FLOAT);
    // Max exponent that we can load in int64
    // Check that x > MIN_INT64 & x < MAX_INT64, else jump
    GetMasm()->Cmp(VixlReg(tmp_reg), VixlImm(POSSIBLE_EXP_FLOAT));
    // If greater than or equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::hs, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(label_check_nan));
    MakeLibCall(dst, src, reinterpret_cast<void *>(__aeabi_f2lz));
    EncodeJump(label_exit);

    BindLabel(label_check_nan);
    // Form of nan number
    GetMasm()->Cmp(VixlReg(tmp_reg), VixlImm(UP_BITS_NAN_FLOAT));
    // If not equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::ne, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(label_not_nan));

    GetMasm()->Lsls(VixlReg(tmp_reg), VixlReg(moved_src), vixl::aarch32::Operand(SHIFT_BITS_FLOAT));
    // If equal, branch to "label_not_nan"
    GetMasm()->B(vixl::aarch32::eq, static_cast<Aarch32LabelHolder *>(GetLabels())->GetLabel(label_not_nan));
    GetMasm()->Mov(VixlReg(dst), VixlImm(0));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(0));
    EncodeJump(label_exit);

    BindLabel(label_not_nan);
    GetMasm()->Adds(VixlReg(moved_src), VixlReg(moved_src), VixlReg(moved_src));
    GetMasm()->Mov(VixlReg(dst), VixlImm(UINT32_MAX));
    GetMasm()->Mov(VixlRegU(dst), VixlImm(INT32_MAX));
    GetMasm()->Adc(VixlReg(dst), VixlReg(dst), VixlImm(0));
    // If exponent negative, transform maxint64 to minint64
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(dst), VixlImm(0));

    BindLabel(label_exit);
}

void Aarch32Encoder::EncodeCastScalarFromSignedScalar(Reg dst, Reg src)
{
    size_t src_size = src.GetSize();
    size_t dst_size = dst.GetSize();
    if (src_size > dst_size) {
        src_size = dst_size;
    }
    switch (src_size) {
        case BYTE_SIZE:
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(src));
            break;
        case HALF_SIZE:
            GetMasm()->Sxth(VixlReg(dst), VixlReg(src));
            break;
        case WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            break;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (dst_size == DOUBLE_WORD_SIZE) {
                GetMasm()->Mov(VixlReg(dst), VixlReg(src));
                return;
            }
            break;
        default:
            SetFalseResult();
            break;
    }
    if (dst_size == DOUBLE_WORD_SIZE) {
        GetMasm()->Asr(VixlRegU(dst), VixlReg(dst), VixlImm(WORD_SIZE - 1));
    }
}

void Aarch32Encoder::EncodeCastScalarFromUnsignedScalar(Reg dst, Reg src)
{
    size_t src_size = src.GetSize();
    size_t dst_size = dst.GetSize();
    if (src_size > dst_size && dst_size < WORD_SIZE) {
        // We need to cut the number, if it is less, than 32-bit. It is by ISA agreement.
        int64_t cut_value = (1ULL << dst_size) - 1;
        GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(cut_value));
        return;
    }
    // Else unsigned extend
    switch (src_size) {
        case BYTE_SIZE:
            GetMasm()->Uxtb(VixlReg(dst), VixlReg(src));
            break;
        case HALF_SIZE:
            GetMasm()->Uxth(VixlReg(dst), VixlReg(src));
            break;
        case WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            break;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (dst_size == DOUBLE_WORD_SIZE) {
                GetMasm()->Mov(VixlReg(dst), VixlReg(src));
                return;
            }
            break;
        default:
            SetFalseResult();
            break;
    }
    if (dst_size == DOUBLE_WORD_SIZE) {
        GetMasm()->Mov(VixlRegU(dst), VixlImm(0x0));
    }
}

void Aarch32Encoder::EncodeAdd(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        ASSERT(src0.IsFloat() && src1.IsFloat());
        GetMasm()->Vadd(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Add(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Adds(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
}

void Aarch32Encoder::EncodeSub(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Vsub(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Sub(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Subs(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    GetMasm()->Sbc(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
}

void Aarch32Encoder::EncodeMul(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Vmul(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Mul(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 lo_reg(this);
    ScopedTmpRegU32 hi_reg(this);

    GetMasm()->Umull(VixlReg(lo_reg), VixlReg(hi_reg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Mla(VixlReg(hi_reg), VixlRegU(src0), VixlReg(src1), VixlReg(hi_reg));
    GetMasm()->Mla(VixlReg(hi_reg), VixlReg(src0), VixlRegU(src1), VixlReg(hi_reg));

    GetMasm()->Mov(VixlReg(dst), VixlReg(lo_reg));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hi_reg));
}

void Aarch32Encoder::MakeLibCall(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        MakeLibCallWithFloatResult(dst, src0, src1, entry_point, second_value);
        return;
    }

    if (dst.GetType() == FLOAT64_TYPE) {
        MakeLibCallWithDoubleResult(dst, src0, src1, entry_point, second_value);
        return;
    }

    if (dst.GetType() == INT64_TYPE) {
        MakeLibCallWithInt64Result(dst, src0, src1, entry_point, second_value);
        return;
    }

    ASSERT(dst.GetSize() < DOUBLE_WORD_SIZE);

    if (src1.GetId() == vixl::aarch32::r0.GetCode() || src0 == src1) {
        ScopedTmpRegU32 tmp(this);
        GetMasm()->Mov(VixlReg(tmp), VixlReg(src1));
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlReg(tmp));
    } else {
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlReg(src1));
    };

    // Call lib-method
    MakeCall(entry_point);

    auto dst_register = second_value ? vixl::aarch32::r1 : vixl::aarch32::r0;

    if (dst.GetId() <= vixl::aarch32::r3.GetCode()) {
        ScopedTmpRegU32 tmp(this);
        GetMasm()->Mov(VixlReg(tmp), dst_register);
        GetMasm()->Mov(VixlReg(dst), VixlReg(tmp));
    } else {
        GetMasm()->Mov(VixlReg(dst), dst_register);
    }
}

void Aarch32Encoder::MakeLibCallWithFloatResult(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value)
{
#if (PANDA_TARGET_ARM32_ABI_HARD)
    // gnueabihf
    // use double parameters
    if (src1.GetId() == vixl::aarch32::s0.GetCode() || src0 == src1) {
        ScopedTmpRegF32 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp), VixlVReg(src1));
        GetMasm()->Vmov(vixl::aarch32::s0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::s1, VixlVReg(tmp));
    } else {
        GetMasm()->Vmov(vixl::aarch32::s0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::s1, VixlVReg(src1));
    };

    MakeCall(entry_point);

    auto dst_register = second_value ? vixl::aarch32::s1 : vixl::aarch32::s0;
    if (dst.GetId() <= vixl::aarch32::s1.GetCode()) {
        ScopedTmpRegF32 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp).S(), dst_register);
        GetMasm()->Vmov(VixlVReg(dst).S(), VixlVReg(tmp).S());
    } else {
        GetMasm()->Vmov(VixlVReg(dst).S(), dst_register);
    }
#else
    // gnueabi
    // use scalar parameters
    GetMasm()->Vmov(vixl::aarch32::r0, VixlVReg(src0).S());
    GetMasm()->Vmov(vixl::aarch32::r1, VixlVReg(src1).S());

    MakeCall(entry_point);

    auto dst_register = second_value ? vixl::aarch32::r1 : vixl::aarch32::r0;
    GetMasm()->Vmov(VixlVReg(dst).S(), dst_register);
#endif  // PANDA_TARGET_ARM32_ABI_HARD
}

void Aarch32Encoder::MakeLibCallWithDoubleResult(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value)
{
#if (PANDA_TARGET_ARM32_ABI_HARD)
    // Scope for temp
    if (src1.GetId() == vixl::aarch32::d0.GetCode() || src0 == src1) {
        ScopedTmpRegF64 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp), VixlVReg(src1));
        GetMasm()->Vmov(vixl::aarch32::d0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::d1, VixlVReg(tmp));
    } else {
        GetMasm()->Vmov(vixl::aarch32::d0, VixlVReg(src0));
        GetMasm()->Vmov(vixl::aarch32::d1, VixlVReg(src1));
    };
    MakeCall(entry_point);
    auto dst_register = second_value ? vixl::aarch32::d1 : vixl::aarch32::d0;

    if (dst.GetId() <= vixl::aarch32::d1.GetCode()) {
        ScopedTmpRegF64 tmp(this);
        GetMasm()->Vmov(VixlVReg(tmp), dst_register);
        GetMasm()->Vmov(VixlVReg(dst), VixlVReg(tmp));
    } else {
        GetMasm()->Vmov(VixlVReg(dst), dst_register);
    }

    // use double parameters
#else
    // use scalar parameters
    GetMasm()->Vmov(vixl::aarch32::r0, vixl::aarch32::r1, VixlVReg(src0).D());
    GetMasm()->Vmov(vixl::aarch32::r2, vixl::aarch32::r3, VixlVReg(src1).D());

    MakeCall(entry_point);

    auto dst_register_1 = second_value ? vixl::aarch32::r2 : vixl::aarch32::r0;
    auto dst_register_2 = second_value ? vixl::aarch32::r3 : vixl::aarch32::r1;

    GetMasm()->Vmov(VixlVReg(dst).D(), dst_register_1, dst_register_2);
#endif  // PANDA_TARGET_ARM32_ABI_HARD
}

void Aarch32Encoder::MakeLibCallWithInt64Result(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value)
{
    // Here I look only for this case, because src - is mapped on two regs.
    // (INT64_TYPE), and src0 can't be rewrited
    // TODO(igorban) If src0==src1 - the result will be 1 or UB(if src0 = 0)
    // It is better to check them in optimizations
    // ASSERT(src0 != src1); - do not enable for tests

    if (src1.GetId() == vixl::aarch32::r0.GetCode() || src0 == src1) {
        ScopedTmpRegU32 tmp1(this);
        ScopedTmpRegU32 tmp2(this);
        GetMasm()->Mov(VixlReg(tmp1), VixlReg(src1));
        GetMasm()->Mov(VixlReg(tmp2), VixlRegU(src1));
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlRegU(src0));
        GetMasm()->Mov(vixl::aarch32::r2, VixlReg(tmp1));
        GetMasm()->Mov(vixl::aarch32::r3, VixlReg(tmp2));
    } else {
        GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src0));
        GetMasm()->Mov(vixl::aarch32::r1, VixlRegU(src0));
        GetMasm()->Mov(vixl::aarch32::r2, VixlReg(src1));
        GetMasm()->Mov(vixl::aarch32::r3, VixlRegU(src1));
    };

    // Call lib-method
    MakeCall(entry_point);

    auto dst_register_1 = second_value ? vixl::aarch32::r2 : vixl::aarch32::r0;
    auto dst_register_2 = second_value ? vixl::aarch32::r3 : vixl::aarch32::r1;

    if (dst.GetId() <= vixl::aarch32::r3.GetCode()) {
        ScopedTmpRegU32 tmp1(this);
        ScopedTmpRegU32 tmp2(this);
        GetMasm()->Mov(VixlReg(tmp1), dst_register_1);
        GetMasm()->Mov(VixlReg(tmp2), dst_register_2);
        GetMasm()->Mov(VixlReg(dst), VixlReg(tmp1));
        GetMasm()->Mov(VixlRegU(dst), VixlReg(tmp2));
    } else {
        GetMasm()->Mov(VixlReg(dst), dst_register_1);
        GetMasm()->Mov(VixlRegU(dst), dst_register_2);
    }
}

void Aarch32Encoder::MakeLibCall(Reg dst, Reg src, void *entry_point)
{
    if (dst.IsFloat() && src.IsScalar()) {
        if (src.GetSize() != DOUBLE_WORD_SIZE) {
            SetFalseResult();
            return;
        }

        bool save_r1 {src.GetId() != vixl::aarch32::r0.GetCode() || dst.GetType() == FLOAT64_TYPE};

        GetMasm()->Push(vixl::aarch32::r0);
        if (save_r1) {
            GetMasm()->Push(vixl::aarch32::r1);
        }

        if (src.GetId() != vixl::aarch32::r0.GetCode()) {
            GetMasm()->Mov(vixl::aarch32::r0, VixlReg(src));
            GetMasm()->Mov(vixl::aarch32::r1, VixlRegU(src));
        }

        MakeCall(entry_point);

        if (dst.GetType() == FLOAT64_TYPE) {
            GetMasm()->Vmov(VixlVReg(dst).D(), vixl::aarch32::r0, vixl::aarch32::r1);
        } else {
            GetMasm()->Vmov(VixlVReg(dst).S(), vixl::aarch32::r0);
        }

        if (save_r1) {
            GetMasm()->Pop(vixl::aarch32::r1);
        }
        GetMasm()->Pop(vixl::aarch32::r0);
    } else if (dst.IsScalar() && src.IsFloat()) {
        if (dst.GetSize() != DOUBLE_WORD_SIZE) {
            SetFalseResult();
            return;
        }

        bool save_r0_r1 {dst.GetId() != vixl::aarch32::r0.GetCode()};

        if (save_r0_r1) {
            GetMasm()->Push(vixl::aarch32::r0);
            GetMasm()->Push(vixl::aarch32::r1);
        }

        if (src.GetType() == FLOAT64_TYPE) {
            GetMasm()->Vmov(vixl::aarch32::r0, vixl::aarch32::r1, VixlVReg(src).D());
        } else {
            GetMasm()->Vmov(vixl::aarch32::r0, VixlVReg(src).S());
        }

        MakeCall(entry_point);

        if (save_r0_r1) {
            GetMasm()->Mov(VixlReg(dst), vixl::aarch32::r0);
            GetMasm()->Mov(VixlRegU(dst), vixl::aarch32::r1);

            GetMasm()->Pop(vixl::aarch32::r1);
            GetMasm()->Pop(vixl::aarch32::r0);
        }
    } else {
        SetFalseResult();
        return;
    }
}

void Aarch32Encoder::EncodeDiv(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Vdiv(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (!dst_signed) {
            GetMasm()->Udiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
            return;
        }

        GetMasm()->Sdiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }
    if (dst_signed) {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(__aeabi_ldivmod));
    } else {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(__aeabi_uldivmod));
    }
}

void Aarch32Encoder::EncodeMod(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        using fp = float (*)(float, float);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<fp>(fmodf)));
        return;
    }
    if (dst.GetType() == FLOAT64_TYPE) {
        using fp = double (*)(double, double);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<fp>(fmod)));
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (dst_signed) {
            MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(__aeabi_idivmod), true);
        } else {
            MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(__aeabi_uidivmod), true);
        }

        // dst = -(tmp * src0) + src1
        if ((dst.GetSize() == BYTE_SIZE) && dst_signed) {
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(dst));
        }
        if ((dst.GetSize() == HALF_SIZE) && dst_signed) {
            GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
        }
        return;
    }

    // Call lib-method
    if (dst_signed) {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(__aeabi_ldivmod), true);
    } else {
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(__aeabi_uldivmod), true);
    }
}

void Aarch32Encoder::EncodeMin(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeMinMaxFp<false>(dst, src0, src1);
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (dst_signed) {
            GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
            GetMasm()->Mov(Convert(Condition::LE), VixlReg(dst), VixlReg(src0));
            GetMasm()->Mov(Convert(Condition::GT), VixlReg(dst), VixlReg(src1));
            return;
        }
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Mov(Convert(Condition::LS), VixlReg(dst), VixlReg(src0));
        GetMasm()->Mov(Convert(Condition::HI), VixlReg(dst), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 tmp_reg(this);

    GetMasm()->Subs(VixlReg(tmp_reg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(src0), VixlRegU(src1));

    auto cc = Convert(dst_signed ? Condition::LT : Condition::LO);
    GetMasm()->Mov(cc, VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(cc, VixlRegU(dst), VixlRegU(src0));
    GetMasm()->Mov(cc.Negate(), VixlReg(dst), VixlReg(src1));
    GetMasm()->Mov(cc.Negate(), VixlRegU(dst), VixlRegU(src1));
}

void Aarch32Encoder::EncodeMax(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeMinMaxFp<true>(dst, src0, src1);
        return;
    }

    if (dst.GetSize() <= WORD_SIZE) {
        if (dst_signed) {
            GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
            GetMasm()->Mov(Convert(Condition::GT), VixlReg(dst), VixlReg(src0));
            GetMasm()->Mov(Convert(Condition::LE), VixlReg(dst), VixlReg(src1));
            return;
        }
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Mov(Convert(Condition::HI), VixlReg(dst), VixlReg(src0));
        GetMasm()->Mov(Convert(Condition::LS), VixlReg(dst), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 tmp_reg(this);

    GetMasm()->Subs(VixlReg(tmp_reg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(src0), VixlRegU(src1));

    auto cc = Convert(dst_signed ? Condition::LT : Condition::LO);
    GetMasm()->Mov(cc, VixlReg(dst), VixlReg(src1));
    GetMasm()->Mov(cc, VixlRegU(dst), VixlRegU(src1));
    GetMasm()->Mov(cc.Negate(), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(cc.Negate(), VixlRegU(dst), VixlRegU(src0));
}

template <bool is_max>
void Aarch32Encoder::EncodeMinMaxFp(Reg dst, Reg src0, Reg src1)
{
    Aarch32LabelHolder::LabelType not_equal(GetAllocator());
    Aarch32LabelHolder::LabelType got_nan(GetAllocator());
    Aarch32LabelHolder::LabelType end(GetAllocator());
    auto &src_a = dst.GetId() != src1.GetId() ? src0 : src1;
    auto &src_b = src_a.GetId() == src0.GetId() ? src1 : src0;
    GetMasm()->Vmov(VixlVReg(dst), VixlVReg(src_a));
    // Vcmp change flags:
    // NZCV
    // 0011 <- if any operand is NaN
    // 0110 <- operands are equals
    // 1000 <- operand0 < operand1
    // 0010 <- operand0 > operand1
    GetMasm()->Vcmp(VixlVReg(src_b), VixlVReg(src_a));
    GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    GetMasm()->B(Convert(Condition::VS), &got_nan);
    GetMasm()->B(Convert(Condition::NE), &not_equal);

    // calculate result for positive/negative zero operands
    if (is_max) {
        EncodeVand(dst, src_a, src_b);
    } else {
        EncodeVorr(dst, src_a, src_b);
    }
    GetMasm()->B(&end);
    GetMasm()->Bind(&got_nan);
    // if any operand is NaN result is NaN
    EncodeVorr(dst, src_a, src_b);
    GetMasm()->B(&end);
    GetMasm()->bind(&not_equal);
    // calculate min/max for other cases
    if (is_max) {
        GetMasm()->B(Convert(Condition::MI), &end);
    } else {
        GetMasm()->B(Convert(Condition::HI), &end);
    }
    GetMasm()->Vmov(VixlVReg(dst), VixlVReg(src_b));
    GetMasm()->bind(&end);
}

void Aarch32Encoder::EncodeVorr(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF64 tmp_reg(this);
        GetMasm()->Vmov(vixl::aarch32::SRegister(tmp_reg.GetReg().GetId() + (src0.GetId() & 1U)), VixlVReg(src1).S());
        GetMasm()->Vorr(vixl::aarch32::DRegister(tmp_reg.GetReg().GetId() / 2U),
                        vixl::aarch32::DRegister(src0.GetId() / 2U),
                        vixl::aarch32::DRegister(tmp_reg.GetReg().GetId() / 2U));
        GetMasm()->Vmov(VixlVReg(dst).S(), vixl::aarch32::SRegister(tmp_reg.GetReg().GetId() + (src0.GetId() & 1U)));

    } else {
        GetMasm()->Vorr(VixlVReg(dst).D(), VixlVReg(src0).D(), VixlVReg(src1).D());
    }
}

void Aarch32Encoder::EncodeVand(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF64 tmp_reg(this);
        GetMasm()->Vmov(vixl::aarch32::SRegister(tmp_reg.GetReg().GetId() + (src0.GetId() & 1U)), VixlVReg(src1).S());
        GetMasm()->Vand(vixl::aarch32::kDataTypeValueNone, vixl::aarch32::DRegister(tmp_reg.GetReg().GetId() / 2U),
                        vixl::aarch32::DRegister(src0.GetId() / 2U),
                        vixl::aarch32::DRegister(tmp_reg.GetReg().GetId() / 2U));
        GetMasm()->Vmov(VixlVReg(dst).S(), vixl::aarch32::SRegister(tmp_reg.GetReg().GetId() + (src0.GetId() & 1U)));
    } else {
        GetMasm()->Vand(vixl::aarch32::kDataTypeValueNone, VixlVReg(dst).D(), VixlVReg(src0).D(), VixlVReg(src1).D());
    }
}

void Aarch32Encoder::EncodeShl(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsl(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 hi_reg(this);
    ScopedTmpRegU32 tmp_reg(this);

    GetMasm()->Rsb(VixlReg(tmp_reg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(VixlReg(tmp_reg), VixlReg(src0), VixlReg(tmp_reg));
    GetMasm()->Orr(VixlReg(hi_reg), VixlReg(tmp_reg),
                   vixl::aarch32::Operand(VixlRegU(src0), vixl::aarch32::LSL, VixlReg(src1)));
    GetMasm()->Subs(VixlReg(tmp_reg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsl(Convert(Condition::PL), VixlReg(hi_reg), VixlReg(src0), VixlReg(tmp_reg));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Lsl(Convert(Condition::MI), VixlReg(dst), VixlReg(src0), VixlReg(src1));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hi_reg));
}

void Aarch32Encoder::EncodeShr(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 lo_reg(this);
    ScopedTmpRegU32 tmp_reg(this);

    GetMasm()->Rsb(VixlReg(tmp_reg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(VixlReg(lo_reg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Orr(VixlReg(lo_reg), VixlReg(lo_reg),
                   vixl::aarch32::Operand(VixlRegU(src0), vixl::aarch32::LSL, VixlReg(tmp_reg)));
    GetMasm()->Subs(VixlReg(tmp_reg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(Convert(Condition::PL), VixlReg(lo_reg), VixlRegU(src0), VixlReg(tmp_reg));
    GetMasm()->Mov(Convert(Condition::PL), VixlRegU(dst), VixlImm(0x0));
    GetMasm()->Lsr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src0), VixlReg(src1));
    GetMasm()->Mov(VixlReg(dst), VixlReg(lo_reg));
}

void Aarch32Encoder::EncodeAShr(Reg dst, Reg src0, Reg src1)
{
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Asr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 lo_reg(this);
    ScopedTmpRegU32 tmp_reg(this);

    GetMasm()->Subs(VixlReg(tmp_reg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Lsr(VixlReg(lo_reg), VixlReg(src0), VixlReg(src1));
    GetMasm()->Rsb(VixlReg(tmp_reg), VixlReg(src1), VixlImm(WORD_SIZE));
    GetMasm()->Orr(VixlReg(lo_reg), VixlReg(lo_reg),
                   vixl::aarch32::Operand(VixlRegU(src0), vixl::aarch32::LSL, VixlReg(tmp_reg)));
    GetMasm()->Rsb(Convert(Condition::PL), VixlReg(tmp_reg), VixlReg(tmp_reg), VixlImm(0x0));
    GetMasm()->Asr(Convert(Condition::PL), VixlReg(lo_reg), VixlRegU(src0), VixlReg(tmp_reg));
    GetMasm()->Asr(Convert(Condition::PL), VixlRegU(dst), VixlRegU(src0), VixlImm(WORD_SIZE - 1));
    GetMasm()->Asr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src0), VixlReg(src1));
    GetMasm()->Mov(VixlReg(dst), VixlReg(lo_reg));
}

void Aarch32Encoder::EncodeAnd(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->And(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->And(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeOr(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Orr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Orr(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeXor(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Eor(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Eor(VixlRegU(dst), VixlRegU(src0), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeAdd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Add(VixlReg(dst), VixlReg(src), VixlImm(imm));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Adds(VixlReg(dst), VixlReg(src), VixlImm(imm));
    GetMasm()->Adc(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
}

void Aarch32Encoder::EncodeSub(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Sub(VixlReg(dst), VixlReg(src), VixlImm(imm));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Subs(VixlReg(dst), VixlReg(src), VixlImm(imm));
    GetMasm()->Sbc(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
}

void Aarch32Encoder::EncodeShl(Reg dst, Reg src, Imm imm)
{
    auto value = static_cast<uint32_t>(GetIntValue(imm));
    int32_t imm_value = value & (dst.GetSize() - 1);

    ASSERT(dst.IsScalar() && "Invalid operand type");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsl(VixlReg(dst), VixlReg(src), VixlImm(imm_value));
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 hi_reg(this);
    ScopedTmpRegU32 tmp_reg(this);

    GetMasm()->Lsr(VixlReg(tmp_reg), VixlReg(src), VixlImm(WORD_SIZE - imm_value));
    GetMasm()->Mov(VixlReg(hi_reg), VixlImm(imm_value));
    GetMasm()->Orr(VixlReg(hi_reg), VixlReg(tmp_reg),
                   vixl::aarch32::Operand(VixlRegU(src), vixl::aarch32::LSL, VixlReg(hi_reg)));
    GetMasm()->Movs(VixlReg(tmp_reg), VixlImm(imm_value - WORD_SIZE));
    GetMasm()->Lsl(Convert(Condition::PL), VixlReg(hi_reg), VixlReg(src), VixlReg(tmp_reg));
    GetMasm()->Mov(Convert(Condition::PL), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Lsl(Convert(Condition::MI), VixlReg(dst), VixlReg(src), VixlImm(imm_value));
    GetMasm()->Mov(VixlRegU(dst), VixlReg(hi_reg));
}

void Aarch32Encoder::EncodeShr(Reg dst, Reg src, Imm imm)
{
    auto value = static_cast<uint32_t>(GetIntValue(imm));
    int32_t imm_value = value & (dst.GetSize() - 1);

    ASSERT(dst.IsScalar() && "Invalid operand type");
    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Lsr(VixlReg(dst), VixlReg(src), imm_value);
        return;
    }

    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    ScopedTmpRegU32 lo_reg(this);
    ScopedTmpRegU32 tmp_reg(this);

    GetMasm()->Mov(VixlReg(tmp_reg), VixlImm(WORD_SIZE - imm_value));
    GetMasm()->Lsr(VixlReg(lo_reg), VixlReg(src), VixlImm(imm_value));
    GetMasm()->Orr(VixlReg(lo_reg), VixlReg(lo_reg),
                   vixl::aarch32::Operand(VixlRegU(src), vixl::aarch32::LSL, VixlReg(tmp_reg)));
    GetMasm()->Movs(VixlReg(tmp_reg), VixlImm(imm_value - WORD_SIZE));
    GetMasm()->Lsr(Convert(Condition::PL), VixlReg(lo_reg), VixlRegU(src), VixlReg(tmp_reg));
    GetMasm()->Mov(Convert(Condition::PL), VixlRegU(dst), VixlImm(0x0));
    GetMasm()->Lsr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src), VixlImm(imm_value));
    GetMasm()->Mov(VixlReg(dst), VixlReg(lo_reg));
}

void Aarch32Encoder::EncodeAShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");

    auto value = static_cast<uint32_t>(GetIntValue(imm));
    int32_t imm_value = value & (dst.GetSize() - 1);

    if (dst.GetSize() <= WORD_SIZE) {
        GetMasm()->Asr(VixlReg(dst), VixlReg(src), imm_value);
        return;
    }

    ScopedTmpRegU32 lo_reg(this);
    ScopedTmpRegU32 tmp_reg(this);
    GetMasm()->Movs(VixlReg(tmp_reg), VixlImm(imm_value - WORD_SIZE));
    GetMasm()->Lsr(VixlReg(lo_reg), VixlReg(src), VixlImm(imm_value));
    GetMasm()->Mov(VixlReg(tmp_reg), VixlImm(WORD_SIZE - imm_value));
    GetMasm()->Orr(VixlReg(lo_reg), VixlReg(lo_reg),
                   vixl::aarch32::Operand(VixlRegU(src), vixl::aarch32::LSL, VixlReg(tmp_reg)));
    GetMasm()->Rsb(Convert(Condition::PL), VixlReg(tmp_reg), VixlReg(tmp_reg), VixlImm(0x0));
    GetMasm()->Asr(Convert(Condition::PL), VixlReg(lo_reg), VixlRegU(src), VixlReg(tmp_reg));
    GetMasm()->Asr(Convert(Condition::PL), VixlRegU(dst), VixlRegU(src), VixlImm(WORD_SIZE - 1));
    GetMasm()->Asr(Convert(Condition::MI), VixlRegU(dst), VixlRegU(src), VixlImm(imm_value));
    GetMasm()->Mov(VixlReg(dst), VixlReg(lo_reg));
}

void Aarch32Encoder::EncodeAnd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(imm));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->And(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
    }
}

void Aarch32Encoder::EncodeOr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Orr(VixlReg(dst), VixlReg(src), VixlImm(imm));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Orr(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
    }
}

void Aarch32Encoder::EncodeXor(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Eor(VixlReg(dst), VixlReg(src), VixlImm(imm));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Eor(VixlRegU(dst), VixlRegU(src), VixlImmU(imm));
    }
}

void Aarch32Encoder::EncodeMov(Reg dst, Imm src)
{
    if (dst.IsFloat()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vmov(Convert(dst.GetType()), VixlVReg(dst).S(), VixlNeonImm(src.GetValue<float>()));
        } else {
            GetMasm()->Vmov(Convert(dst.GetType()), VixlVReg(dst).D(), VixlNeonImm(src.GetValue<double>()));
        }
        return;
    }

    GetMasm()->Mov(VixlReg(dst), VixlImm(src));
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(VixlRegU(dst), VixlImmU(src));
    }
}

void Aarch32Encoder::EncodeLdr(Reg dst, bool dst_signed, const vixl::aarch32::MemOperand &vixl_mem)
{
    if (dst.IsFloat()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Vldr(VixlVReg(dst).S(), vixl_mem);
        } else {
            GetMasm()->Vldr(VixlVReg(dst).D(), vixl_mem);
        }
        return;
    }
    if (dst_signed) {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldrsb(VixlReg(dst), vixl_mem);
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldrsh(VixlReg(dst), vixl_mem);
            return;
        }
    } else {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldrb(VixlReg(dst), vixl_mem);
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldrh(VixlReg(dst), vixl_mem);
            return;
        }
    }
    if (dst.GetSize() == WORD_SIZE) {
        GetMasm()->Ldr(VixlReg(dst), vixl_mem);
        return;
    }
    ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
    GetMasm()->Ldrd(VixlReg(dst), VixlRegU(dst), vixl_mem);
}

void Aarch32Encoder::EncodeLdr(Reg dst, bool dst_signed, MemRef mem)
{
    auto type = dst.GetType();
    if (IsNeedToPrepareMemLdS(mem, type, dst_signed)) {
        ScopedTmpRegU32 tmp_reg(this);
        auto tmp = VixlReg(tmp_reg);
        auto vixl_mem = PrepareMemLdS(mem, type, tmp, dst_signed);
        EncodeLdr(dst, dst_signed, vixl_mem);
    } else {
        auto vixl_mem = ConvertMem(mem);
        EncodeLdr(dst, dst_signed, vixl_mem);
    }
}

void Aarch32Encoder::EncodeLdrAcquire(Reg dst, bool dst_signed, MemRef mem)
{
    EncodeLdr(dst, dst_signed, mem);
    GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
}

void Aarch32Encoder::EncodeMemoryBarrier(MemoryOrder::Order order)
{
    switch (order) {
        case MemoryOrder::Acquire:
        case MemoryOrder::Release: {
            GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
            break;
        }
        case MemoryOrder::Full: {
            GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISHST);
            break;
        }
        default:
            break;
    }
}

void Aarch32Encoder::EncodeStr(Reg src, const vixl::aarch32::MemOperand &vixl_mem)
{
    if (src.IsFloat()) {
        if (src.GetSize() == WORD_SIZE) {
            GetMasm()->Vstr(VixlVReg(src).S(), vixl_mem);
        } else {
            GetMasm()->Vstr(VixlVReg(src).D(), vixl_mem);
        }
    } else if (src.GetSize() == BYTE_SIZE) {
        GetMasm()->Strb(VixlReg(src), vixl_mem);
    } else if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Strh(VixlReg(src), vixl_mem);
    } else if (src.GetSize() == WORD_SIZE) {
        GetMasm()->Str(VixlReg(src), vixl_mem);
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);
        GetMasm()->Strd(VixlReg(src), VixlRegU(src), vixl_mem);
    }
}

void Aarch32Encoder::EncodeStr(Reg src, MemRef mem)
{
    auto type = src.GetType();
    if (IsNeedToPrepareMemLdS(mem, type, false)) {
        ScopedTmpRegU32 tmp_reg(this);
        auto tmp = VixlReg(tmp_reg);
        auto vixl_mem = PrepareMemLdS(mem, type, tmp, false);
        EncodeStr(src, vixl_mem);
    } else {
        auto vixl_mem = ConvertMem(mem);
        EncodeStr(src, vixl_mem);
    }
}

void Aarch32Encoder::EncodeStrRelease(Reg src, MemRef mem)
{
    GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
    EncodeStr(src, mem);
    GetMasm()->Dmb(vixl::aarch32::MemoryBarrierType::ISH);
}

void Aarch32Encoder::EncodeStrz(Reg src, MemRef mem)
{
    if (src.GetSize() <= WORD_SIZE) {
        EncodeSti(Imm(static_cast<int64_t>(0)), mem);
    }
    EncodeStr(src, mem);
}

void Aarch32Encoder::EncodeStp(Reg src0, Reg src1, MemRef mem)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    ASSERT(src0.GetSize() == src1.GetSize());
    EncodeStr(src0, mem);
    EncodeStr(src1, MemRef(mem.GetBase(), mem.GetIndex(), mem.GetScale(), mem.GetDisp() + WORD_SIZE_BYTE));
}

void Aarch32Encoder::EncodeLdrExclusive(Reg dst, Reg addr, bool acquire)
{
    ASSERT(dst.IsScalar());
    auto dst_reg = VixlReg(dst);
    auto mem_cvt = ConvertMem(MemRef(addr));
    if (dst.GetSize() == BYTE_SIZE) {
        if (acquire) {
            GetMasm()->Ldaexb(dst_reg, mem_cvt);
            return;
        }
        GetMasm()->Ldrexb(dst_reg, mem_cvt);
        return;
    }
    if (dst.GetSize() == HALF_SIZE) {
        if (acquire) {
            GetMasm()->Ldaexh(dst_reg, mem_cvt);
            return;
        }
        GetMasm()->Ldrexh(dst_reg, mem_cvt);
        return;
    }
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        auto dst_reg_u = VixlRegU(dst);
        if (acquire) {
            GetMasm()->Ldaexd(dst_reg, dst_reg_u, mem_cvt);
            return;
        }
        GetMasm()->Ldrexd(dst_reg, dst_reg_u, mem_cvt);
        return;
    }
    if (acquire) {
        GetMasm()->Ldaex(dst_reg, mem_cvt);
        return;
    }
    GetMasm()->Ldrex(dst_reg, mem_cvt);
}

void Aarch32Encoder::EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release)
{
    ASSERT(dst.IsScalar() && src.IsScalar());
    ASSERT(dst.GetSize() != DOUBLE_WORD_SIZE);

    bool copy_dst = dst.GetId() == src.GetId() || dst.GetId() == addr.GetId();
    ScopedTmpReg tmp(this);
    auto dst_reg = copy_dst ? VixlReg(tmp) : VixlReg(dst);
    auto src_reg = VixlReg(src);
    auto mem_cvt = ConvertMem(MemRef(addr));

    if (src.GetSize() == BYTE_SIZE) {
        if (release) {
            GetMasm()->Stlexb(dst_reg, src_reg, mem_cvt);
        } else {
            GetMasm()->Strexb(dst_reg, src_reg, mem_cvt);
        }
    } else if (src.GetSize() == HALF_SIZE) {
        if (release) {
            GetMasm()->Stlexh(dst_reg, src_reg, mem_cvt);
        } else {
            GetMasm()->Strexh(dst_reg, src_reg, mem_cvt);
        }
    } else if (src.GetSize() == DOUBLE_WORD_SIZE) {
        auto src_reg_u = VixlRegU(src);
        if (release) {
            GetMasm()->Stlexd(dst_reg, src_reg, src_reg_u, mem_cvt);
        } else {
            GetMasm()->Strexd(dst_reg, src_reg, src_reg_u, mem_cvt);
        }
    } else {
        if (release) {
            GetMasm()->Stlex(dst_reg, src_reg, mem_cvt);
        } else {
            GetMasm()->Strex(dst_reg, src_reg, mem_cvt);
        }
    }

    if (copy_dst) {
        EncodeMov(dst, tmp);
    }
}

inline static int32_t FindRegForMem(vixl::aarch32::MemOperand mem)
{
    int32_t base_reg_id = mem.GetBaseRegister().GetCode();
    int32_t index_reg_id = -1;
    if (mem.IsShiftedRegister()) {
        index_reg_id = mem.GetOffsetRegister().GetCode();
    }
    // find regs for mem
    constexpr int32_t STEP = 2;
    for (int32_t i = 0; i < static_cast<int32_t>(BYTE_SIZE); i += STEP) {
        if (base_reg_id == i || base_reg_id == i + 1 || index_reg_id == i || index_reg_id == i + 1) {
            continue;
        }
        return i;
    }
    UNREACHABLE();
    return -1;
}

void Aarch32Encoder::EncodeSti(Imm src, MemRef mem)
{
    if (src.GetType().IsFloat()) {
        EncodeFloatSti(src, mem);
        return;
    }
    ScopedTmpRegU32 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg);
    auto type = src.GetType();
    if (src.GetSize() <= WORD_SIZE) {
        auto vixl_mem = PrepareMemLdS(mem, type, tmp, false);
        if (vixl_mem.GetBaseRegister().GetCode() == tmp.GetCode()) {
            ScopedTmpRegU32 tmp1_reg(this);
            tmp = VixlReg(tmp1_reg);
        }
        GetMasm()->Mov(tmp, VixlImm(src));
        if (src.GetSize() == BYTE_SIZE) {
            GetMasm()->Strb(tmp, vixl_mem);
            return;
        }
        if (src.GetSize() == HALF_SIZE) {
            GetMasm()->Strh(tmp, vixl_mem);
            return;
        }
        GetMasm()->Str(tmp, vixl_mem);
        return;
    }

    auto vixl_mem = PrepareMemLdS(mem, type, tmp, false, true);
    ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);
    vixl::aarch32::Register tmp_imm1;
    vixl::aarch32::Register tmp_imm2;
    // if tmp isn't base reg and tmp is even and tmp+1 isn't SP we can use tmp and tmp + 1
    if (vixl_mem.GetBaseRegister().GetCode() != tmp.GetCode() && (tmp.GetCode() % 2U == 0) &&
        tmp.GetCode() + 1 != vixl::aarch32::sp.GetCode()) {
        tmp_imm1 = tmp;
        tmp_imm2 = vixl::aarch32::Register(tmp.GetCode() + 1);
    } else {
        auto reg_id = FindRegForMem(vixl_mem);
        ASSERT(reg_id != -1);
        tmp_imm1 = vixl::aarch32::Register(reg_id);
        tmp_imm2 = vixl::aarch32::Register(reg_id + 1);
        GetMasm()->Push(tmp_imm1);
    }

    ASSERT(tmp_imm1.IsValid() && tmp_imm2.IsValid());
    GetMasm()->Push(tmp_imm2);
    GetMasm()->Mov(tmp_imm1, VixlImm(src));
    GetMasm()->Mov(tmp_imm2, VixlImmU(src));
    GetMasm()->Strd(tmp_imm1, tmp_imm2, vixl_mem);
    GetMasm()->Pop(tmp_imm2);
    if (tmp_imm1.GetCode() != tmp.GetCode()) {
        GetMasm()->Pop(tmp_imm1);
    }
}

void Aarch32Encoder::EncodeFloatSti(Imm src, MemRef mem)
{
    ASSERT(src.GetType().IsFloat());
    if (src.GetSize() == WORD_SIZE) {
        ScopedTmpRegF32 tmp_reg(this);
        GetMasm()->Vmov(VixlVReg(tmp_reg).S(), src.GetValue<float>());
        EncodeStr(tmp_reg, mem);
    } else {
        ScopedTmpRegF64 tmp_reg(this);
        GetMasm()->Vmov(VixlVReg(tmp_reg).D(), src.GetValue<double>());
        EncodeStr(tmp_reg, mem);
    }
}

void Aarch32Encoder::EncodeMemCopy(MemRef mem_from, MemRef mem_to, size_t size)
{
    if (size == DOUBLE_WORD_SIZE && mem_from.IsOffsetMem() && mem_to.IsOffsetMem()) {
        EncodeMemCopy(mem_from, mem_to, WORD_SIZE);
        constexpr int32_t STEP = 4;
        auto offset_from = mem_from.GetDisp() + STEP;
        auto offset_to = mem_to.GetDisp() + STEP;
        EncodeMemCopy(MemRef(mem_from.GetBase(), offset_from), MemRef(mem_to.GetBase(), offset_to), WORD_SIZE);

        return;
    }
    ScopedTmpRegU32 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg);
    ScopedTmpRegU32 tmp_reg1(this);
    auto tmp1 = VixlReg(tmp_reg1);
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmp, PrepareMemLdS(mem_from, INT8_TYPE, tmp, false));
        GetMasm()->Strb(tmp, PrepareMemLdS(mem_to, INT8_TYPE, tmp1, false));
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmp, PrepareMemLdS(mem_from, INT16_TYPE, tmp, false));
        GetMasm()->Strh(tmp, PrepareMemLdS(mem_to, INT16_TYPE, tmp1, false));
    } else if (size == WORD_SIZE) {
        GetMasm()->Ldr(tmp, PrepareMemLdS(mem_from, INT32_TYPE, tmp, false));
        GetMasm()->Str(tmp, PrepareMemLdS(mem_to, INT32_TYPE, tmp1, false));
    } else {
        ASSERT(size == DOUBLE_WORD_SIZE);

        auto vixl_mem_from = PrepareMemLdS(mem_from, INT64_TYPE, tmp, false, true);
        auto vixl_mem_to = PrepareMemLdS(mem_to, INT64_TYPE, tmp1, false, true);
        auto reg_id = FindRegForMem(vixl_mem_to);
        ASSERT(reg_id != -1);
        [[maybe_unused]] constexpr auto IMM_2 = 2;
        ASSERT(reg_id % IMM_2 == 0);
        vixl::aarch32::Register tmp_copy1(reg_id);
        vixl::aarch32::Register tmp_copy2(reg_id + 1);

        GetMasm()->Push(tmp_copy1);
        GetMasm()->Push(tmp_copy2);
        GetMasm()->Ldrd(tmp_copy1, tmp_copy2, vixl_mem_from);
        GetMasm()->Strd(tmp_copy1, tmp_copy2, vixl_mem_to);
        GetMasm()->Pop(tmp_copy2);
        GetMasm()->Pop(tmp_copy1);
    }
}

void Aarch32Encoder::EncodeMemCopyz(MemRef mem_from, MemRef mem_to, size_t size)
{
    ScopedTmpRegU32 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg);
    ScopedTmpRegU32 tmp_reg1(this);
    auto tmp1 = VixlReg(tmp_reg1);

    auto type = TypeInfo::GetScalarTypeBySize(size);

    auto vixl_mem_from = PrepareMemLdS(mem_from, type, tmp, false, true);
    auto vixl_mem_to = PrepareMemLdS(mem_to, INT64_TYPE, tmp1, false, true);
    auto reg_id = FindRegForMem(vixl_mem_to);
    ASSERT(reg_id != -1);
    [[maybe_unused]] constexpr auto IMM_2 = 2;
    ASSERT(reg_id % IMM_2 == 0);
    vixl::aarch32::Register tmp_copy1(reg_id);
    vixl::aarch32::Register tmp_copy2(reg_id + 1);

    GetMasm()->Push(tmp_copy1);
    GetMasm()->Push(tmp_copy2);
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmp_copy1, vixl_mem_from);
        GetMasm()->Mov(tmp_copy2, VixlImm(0));
        GetMasm()->Strd(tmp_copy1, tmp_copy2, vixl_mem_to);
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmp_copy1, vixl_mem_from);
        GetMasm()->Mov(tmp_copy2, VixlImm(0));
        GetMasm()->Strd(tmp_copy1, tmp_copy2, vixl_mem_to);
    } else if (size == WORD_SIZE) {
        GetMasm()->Ldr(tmp_copy1, vixl_mem_from);
        GetMasm()->Mov(tmp_copy2, VixlImm(0));
        GetMasm()->Strd(tmp_copy1, tmp_copy2, vixl_mem_to);
    } else {
        ASSERT(size == DOUBLE_WORD_SIZE);
        GetMasm()->Ldrd(tmp_copy1, tmp_copy2, vixl_mem_from);
        GetMasm()->Strd(tmp_copy1, tmp_copy2, vixl_mem_to);
    }
    GetMasm()->Pop(tmp_copy2);
    GetMasm()->Pop(tmp_copy1);
}

void Aarch32Encoder::CompareHelper(Reg src0, Reg src1, Condition *cc)
{
    if (src0.IsFloat() && src1.IsFloat()) {
        GetMasm()->Vcmp(VixlVReg(src0), VixlVReg(src1));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    } else if (src0.GetSize() <= WORD_SIZE && src1.GetSize() <= WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    } else {
        if (!IsConditionSigned(*cc)) {
            GetMasm()->Cmp(VixlRegU(src0), VixlRegU(src1));
            GetMasm()->Cmp(Convert(Condition::EQ), VixlReg(src0), VixlReg(src1));
        } else {
            bool swap = false;
            switch (*cc) {
                case Condition::GT:
                    swap = true;
                    *cc = Condition::LT;
                    break;
                case Condition::LE:
                    swap = true;
                    *cc = Condition::GE;
                    break;
                case Condition::GE:
                case Condition::LT:
                    break;
                default:
                    UNREACHABLE();
            }

            Reg op0 = swap ? src1 : src0;
            Reg op1 = swap ? src0 : src1;
            ScopedTmpRegU32 tmp_reg(this);
            GetMasm()->Cmp(VixlReg(op0), VixlReg(op1));
            GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(op0), VixlRegU(op1));
        }
    }
}

void Aarch32Encoder::TestHelper(Reg src0, Reg src1, [[maybe_unused]] Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);

    if (src0.GetSize() <= WORD_SIZE && src1.GetSize() <= WORD_SIZE) {
        GetMasm()->Tst(VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Tst(VixlRegU(src0), VixlRegU(src1));
        GetMasm()->Tst(Convert(Condition::EQ), VixlReg(src0), VixlReg(src1));
    }
}

void Aarch32Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc)
{
    CompareHelper(src0, src1, &cc);
    GetMasm()->Mov(Convert(cc), VixlReg(dst), 0x1);
    GetMasm()->Mov(Convert(cc).Negate(), VixlReg(dst), 0x0);
}

void Aarch32Encoder::EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc)
{
    TestHelper(src0, src1, cc);
    GetMasm()->Mov(ConvertTest(cc), VixlReg(dst), 0x1);
    GetMasm()->Mov(ConvertTest(cc).Negate(), VixlReg(dst), 0x0);
}

void Aarch32Encoder::EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsFloat()) {
        ASSERT(src1.IsFloat());
        ASSERT(cc == Condition::MI || cc == Condition::LT);
        GetMasm()->Vcmp(VixlVReg(src0), VixlVReg(src1));
        GetMasm()->Vmrs(vixl::aarch32::RegisterOrAPSR_nzcv(vixl::aarch32::kPcCode), vixl::aarch32::FPSCR);
    } else {
        ASSERT(src0.IsScalar() && src1.IsScalar());
        ASSERT(cc == Condition::LO || cc == Condition::LT);
        if (src0.GetSize() <= WORD_SIZE && src1.GetSize() <= WORD_SIZE) {
            GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        } else {
            if (cc == Condition::LO) {
                GetMasm()->Cmp(VixlRegU(src0), VixlRegU(src1));
                GetMasm()->Cmp(Convert(Condition::EQ), VixlReg(src0), VixlReg(src1));
            } else if (cc == Condition::LT) {
                auto label_holder = static_cast<Aarch32LabelHolder *>(GetLabels());
                auto end_label = label_holder->CreateLabel();
                ScopedTmpRegU32 tmp_reg(this);

                GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
                GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(src0), VixlRegU(src1));
                GetMasm()->Mov(Convert(Condition::LT), VixlReg(dst), VixlImm(-1));
                GetMasm()->B(Convert(Condition::LT), label_holder->GetLabel(end_label));

                GetMasm()->Cmp(VixlReg(src1), VixlReg(src0));
                GetMasm()->Sbcs(VixlReg(tmp_reg), VixlRegU(src1), VixlRegU(src0));
                GetMasm()->Mov(Convert(Condition::LT), VixlReg(dst), VixlImm(1));
                GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(0));

                label_holder->BindLabel(end_label);
                return;
            } else {
                UNREACHABLE();
            }
        }
    }

    GetMasm()->Mov(Convert(Condition::EQ), VixlReg(dst), VixlImm(0x0));
    GetMasm()->Mov(Convert(Condition::NE), VixlReg(dst), VixlImm(0x1));

    GetMasm()->Rsb(Convert(cc), VixlReg(dst), VixlReg(dst), VixlImm(0x0));
}

void Aarch32Encoder::EncodeStackOverflowCheck(ssize_t offset)
{
    ScopedTmpReg tmp(this);
    EncodeAdd(tmp, GetTarget().GetStackReg(), Imm(offset));
    EncodeLdr(tmp, false, MemRef(tmp));
}

void Aarch32Encoder::EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat());

    CompareHelper(src2, src3, &cc);

    GetMasm()->Mov(Convert(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(Convert(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(Convert(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(Convert(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());
    auto value = GetIntValue(imm);
    if (value == 0) {
        switch (cc) {
            case Condition::LO:
                // LO is always false, select src1
                GetMasm()->Mov(VixlReg(dst), VixlReg(src1));
                if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
                    GetMasm()->Mov(VixlRegU(dst), VixlRegU(src1));
                }
                return;
            case Condition::HS:
                // HS is always true, select src0
                GetMasm()->Mov(VixlReg(dst), VixlReg(src0));
                if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
                    GetMasm()->Mov(VixlRegU(dst), VixlRegU(src0));
                }
                return;
            case Condition::LS:
                // LS is same as EQ
                cc = Condition::EQ;
                break;
            case Condition::HI:
                // HI is same as NE
                cc = Condition::NE;
                break;
            default:
                break;
        }
        CompareZeroHelper(src2, &cc);
    } else {  // value != 0
        if (!CompareImmHelper(src2, imm, &cc)) {
            return;
        }
    }

    GetMasm()->Mov(Convert(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(Convert(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(Convert(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(Convert(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    TestHelper(src2, src3, cc);

    GetMasm()->Mov(ConvertTest(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(ConvertTest(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(ConvertTest(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(ConvertTest(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

void Aarch32Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    TestImmHelper(src2, imm, cc);
    GetMasm()->Mov(ConvertTest(cc), VixlReg(dst), VixlReg(src0));
    GetMasm()->Mov(ConvertTest(cc).Negate(), VixlReg(dst), VixlReg(src1));

    if (src0.GetSize() > WORD_SIZE || src1.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(ConvertTest(cc), VixlRegU(dst), VixlRegU(src0));
        GetMasm()->Mov(ConvertTest(cc).Negate(), VixlRegU(dst), VixlRegU(src1));
    }
}

bool Aarch32Encoder::CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signed_compare)
{
    if (imm == INT64_MIN) {
        return false;
    }
    if (imm < 0) {
        imm = -imm;
        if (size > WORD_SIZE && signed_compare) {
            return false;
        }
    }
    // We don't support 64-bit immediate, even when both higher and lower parts are legal immediates
    if (imm > UINT32_MAX) {
        return false;
    }
    return vixl::aarch32::ImmediateA32::IsImmediateA32(imm);
}

bool Aarch32Encoder::CanEncodeImmLogical(uint64_t imm, uint32_t size)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    uint64_t high = imm >> WORD_SIZE;
    if (size == DOUBLE_WORD_SIZE) {
        if (high != 0U && high != UINT32_MAX) {
            return false;
        }
    }
    return vixl::aarch32::ImmediateA32::IsImmediateA32(imm);
}

using vixl::aarch32::MemOperand;
using vixl::aarch32::Register;
using vixl::aarch32::SRegister;

template <bool is_store>
void Aarch32Encoder::LoadStoreRegisters(RegMask registers, bool is_fp, int32_t slot, Reg base, RegMask mask)
{
    if (registers.none()) {
        return;
    }

    vixl::aarch32::Register base_reg = VixlReg(base);
    int32_t max_offset = (slot + helpers::ToSigned(registers.GetMaxRegister())) * WORD_SIZE_BYTE;

    ScopedTmpRegU32 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg);
    // Construct single add for big offset
    if (is_fp) {
        if ((max_offset < -VMEM_OFFSET) || (max_offset > VMEM_OFFSET)) {
            GetMasm()->Add(tmp, base_reg, VixlImm(slot * WORD_SIZE_BYTE));
            slot = 0;
            base_reg = tmp;
        }
    } else {
        if ((max_offset < -MEM_BIG_OFFSET) || (max_offset > MEM_BIG_OFFSET)) {
            GetMasm()->Add(tmp, base_reg, VixlImm(slot * WORD_SIZE_BYTE));
            slot = 0;
            base_reg = tmp;
        }
    }
    bool has_mask = mask.any();
    int32_t index = has_mask ? static_cast<int32_t>(mask.GetMinRegister()) : 0;
    slot -= index;
    for (size_t i = index; i < registers.size(); i++) {
        if (has_mask) {
            if (!mask.test(i)) {
                continue;
            }
            index++;
        }
        if (!registers.test(i)) {
            continue;
        }

        if (!has_mask) {
            index++;
        }
        auto mem = MemOperand(base_reg, (slot + index - 1) * WORD_SIZE_BYTE);
        if (is_fp) {
            auto reg = vixl::aarch32::SRegister(i);
            if constexpr (is_store) {  // NOLINT
                GetMasm()->Vstr(reg, mem);
            } else {  // NOLINT
                GetMasm()->Vldr(reg, mem);
            }
        } else {
            auto reg = vixl::aarch32::Register(i);
            if constexpr (is_store) {  // NOLINT
                GetMasm()->Str(reg, mem);
            } else {  // NOLINT
                GetMasm()->Ldr(reg, mem);
            }
        }
    }
}

template <bool is_store>
void Aarch32Encoder::LoadStoreRegisters(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp)
{
    if (registers.none()) {
        return;
    }
    int32_t last_reg = registers.size() - 1;
    for (; last_reg >= 0; --last_reg) {
        if (registers.test(last_reg)) {
            break;
        }
    }
    vixl::aarch32::Register base_reg = vixl::aarch32::sp;
    auto max_offset = (slot + last_reg) * WORD_SIZE_BYTE;
    ScopedTmpRegU32 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg);
    // Construct single add for big offset
    if (is_fp) {
        if ((max_offset < -VMEM_OFFSET) || (max_offset > VMEM_OFFSET)) {
            GetMasm()->Add(tmp, base_reg, VixlImm(slot * WORD_SIZE_BYTE));
            slot = 0;
            base_reg = tmp;
        }
    } else {
        if ((max_offset < -MEM_BIG_OFFSET) || (max_offset > MEM_BIG_OFFSET)) {
            GetMasm()->Add(tmp, base_reg, VixlImm(slot * WORD_SIZE_BYTE));
            slot = 0;
            base_reg = tmp;
        }
    }
    for (auto i = start_reg; i < registers.size(); i++) {
        if (!registers.test(i)) {
            continue;
        }
        auto mem = MemOperand(base_reg, (slot + i - start_reg) * WORD_SIZE_BYTE);
        if (is_fp) {
            auto reg = vixl::aarch32::SRegister(i);
            if constexpr (is_store) {  // NOLINT
                GetMasm()->Vstr(reg, mem);
            } else {  // NOLINT
                GetMasm()->Vldr(reg, mem);
            }
        } else {
            auto reg = vixl::aarch32::Register(i);
            if constexpr (is_store) {  // NOLINT
                GetMasm()->Str(reg, mem);
            } else {  // NOLINT
                GetMasm()->Ldr(reg, mem);
            }
        }
    }
}

void Aarch32Encoder::PushRegisters(RegMask registers, bool is_fp, bool align)
{
    (void)registers;
    (void)is_fp;
    (void)align;
    // TODO(msherstennikov): Implement
}

void Aarch32Encoder::PopRegisters(RegMask registers, bool is_fp, bool align)
{
    (void)registers;
    (void)is_fp;
    (void)align;
    // TODO(msherstennikov): Implement
}

size_t Aarch32Encoder::DisasmInstr(std::ostream &stream, size_t pc, ssize_t code_offset) const
{
    auto addr = GetMasm()->GetBuffer()->GetOffsetAddress<const uint32_t *>(pc);
    // Display pc is seted, because disassembler use pc
    // for upper bits (e.g. 0x40000000), when print one instruction.
    if (code_offset < 0) {
        vixl::aarch32::PrintDisassembler disasm(GetAllocator(), stream);
        disasm.DisassembleA32Buffer(addr, vixl::aarch32::k32BitT32InstructionSizeInBytes);
    } else {
        const uint64_t DISPLAY_PC = 0x10000000;
        vixl::aarch32::PrintDisassembler disasm(GetAllocator(), stream, DISPLAY_PC + pc + code_offset);
        disasm.DisassembleA32Buffer(addr, vixl::aarch32::k32BitT32InstructionSizeInBytes);

        stream << std::setfill(' ');
    }
    return pc + vixl::aarch32::k32BitT32InstructionSizeInBytes;
}
}  // namespace panda::compiler::aarch32
