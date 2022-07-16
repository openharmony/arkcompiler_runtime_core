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

#include "encode.h"
#include "target/aarch64/target.h"
#include "compiler/optimizer/code_generator/relocations.h"

#if defined(USE_VIXL_ARM64) && !defined(PANDA_MINIMAL_VIXL)
#include "aarch64/disasm-aarch64.h"
#endif

#include <iomanip>

#include "lib_helpers.inl"

#ifndef PANDA_TARGET_MACOS
#include "elf.h"
#endif  // PANDA_TARGET_MACOS

namespace panda::compiler::aarch64 {
using vixl::aarch64::CPURegister;
using vixl::aarch64::MemOperand;

static inline Reg Promote(Reg reg)
{
    if (reg.GetType() == INT8_TYPE) {
        return Reg(reg.GetId(), INT16_TYPE);
    }
    return reg;
}

void Aarch64LabelHolder::BindLabel(LabelId id)
{
    static_cast<Aarch64Encoder *>(GetEncoder())->GetMasm()->Bind(labels_[id]);
}

Aarch64Encoder::Aarch64Encoder(ArenaAllocator *allocator) : Encoder(allocator, Arch::AARCH64)
{
    labels_ = allocator->New<Aarch64LabelHolder>(this);
    if (labels_ == nullptr) {
        SetFalseResult();
    }
    // We enable LR tmp reg by default in Aarch64
    EnableLrAsTempReg(true);
}

Aarch64Encoder::~Aarch64Encoder()
{
    auto labels = static_cast<Aarch64LabelHolder *>(GetLabels())->labels_;
    for (auto label : labels) {
        label->~Label();
    }
    if (masm_ != nullptr) {
        masm_->~MacroAssembler();
        masm_ = nullptr;
    }
#ifndef PANDA_MINIMAL_VIXL
    if (decoder_ != nullptr) {
        decoder_->~Decoder();
        decoder_ = nullptr;
    }
#endif
}

bool Aarch64Encoder::InitMasm()
{
    if (masm_ == nullptr) {
        // Initialize Masm
        masm_ = GetAllocator()->New<vixl::aarch64::MacroAssembler>(GetAllocator());
        if (masm_ == nullptr || !masm_->IsValid()) {
            SetFalseResult();
            return false;
        }
        ASSERT(GetMasm());

        // Make sure that the compiler uses the same scratch registers as the assembler
        CHECK_EQ(RegMask(GetMasm()->GetScratchRegisterList()->GetList()), GetTarget().GetTempRegsMask());
        CHECK_EQ(RegMask(GetMasm()->GetScratchVRegisterList()->GetList()), GetTarget().GetTempVRegsMask());
    }
    return true;
}

void Aarch64Encoder::Finalize()
{
    GetMasm()->FinalizeCode();
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id)
{
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label);
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    if (src1.GetId() == GetRegfile()->GetZeroReg().GetId()) {
        EncodeJump(id, src0, cc);
        return;
    }

    if (src0.IsScalar()) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Fcmp(VixlVReg(src0), VixlVReg(src1));
    }

    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    auto value = GetIntValue(imm);
    if (value == 0) {
        EncodeJump(id, src, cc);
        return;
    }

    ASSERT(CanEncodeImmAddSubCmp(value, src.GetSize(), false));
    if (value < 0) {
        GetMasm()->Cmn(VixlReg(src), VixlImm(-value));
    } else {  // if (value > 0)
        GetMasm()->Cmp(VixlReg(src), VixlImm(value));
    }

    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar() && src1.IsScalar());

    GetMasm()->Tst(VixlReg(src0), VixlReg(src1));
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, ConvertTest(cc));
}

void Aarch64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    auto value = GetIntValue(imm);
    ASSERT(CanEncodeImmLogical(value, imm.GetSize() > WORD_SIZE ? DOUBLE_WORD_SIZE : WORD_SIZE));

    GetMasm()->Tst(VixlReg(src), VixlImm(value));
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, ConvertTest(cc));
}

void Aarch64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc)
{
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    ASSERT(src.IsScalar());
    auto rzero = Reg(GetRegfile()->GetZeroReg().GetId(), src.GetType());

    switch (cc) {
        case Condition::LO:
            // Always false
            return;
        case Condition::HS:
            // Always true
            GetMasm()->B(label);
            return;
        case Condition::EQ:
        case Condition::LS:
            if (src.GetId() == rzero.GetId()) {
                GetMasm()->B(label);
                return;
            }
            // True only when zero
            GetMasm()->Cbz(VixlReg(src), label);
            return;
        case Condition::NE:
        case Condition::HI:
            if (src.GetId() == rzero.GetId()) {
                // Do nothing
                return;
            }
            // True only when non-zero
            GetMasm()->Cbnz(VixlReg(src), label);
            return;
        default:
            break;
    }

    ASSERT(rzero.IsValid());
    GetMasm()->Cmp(VixlReg(src), VixlReg(rzero));
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeJump(Reg dst)
{
    GetMasm()->Br(VixlReg(dst));
}

void Aarch64Encoder::EncodeJump([[maybe_unused]] RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_AARCH64_CALL26;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0x14000000;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch64Encoder::EncodeBitTestAndBranch(LabelHolder::LabelId id, compiler::Reg reg, uint32_t bit_pos,
                                            bool bit_value)
{
    ASSERT(reg.IsScalar() && reg.GetSize() > bit_pos);
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    if (bit_value) {
        GetMasm()->Tbnz(VixlReg(reg), bit_pos, label);
    } else {
        GetMasm()->Tbz(VixlReg(reg), bit_pos, label);
    }
}

void Aarch64Encoder::EncodeNop()
{
    GetMasm()->Nop();
}

void Aarch64Encoder::MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    auto buffer = GetMasm()->GetBuffer();
    relocation->offset = GetCursorOffset();
    relocation->addend = 0;
    relocation->type = R_AARCH64_CALL26;
    static constexpr uint32_t CALL_WITH_ZERO_OFFSET = 0x94000000;
    buffer->Emit32(CALL_WITH_ZERO_OFFSET);
#endif
}

void Aarch64Encoder::MakeCall(const void *entry_point)
{
    auto lr_reg = GetTarget().GetLinkReg();
    EncodeMov(lr_reg, Imm(reinterpret_cast<uintptr_t>(entry_point)));
    GetMasm()->Blr(VixlReg(lr_reg));
}

void Aarch64Encoder::MakeCall(MemRef entry_point)
{
    auto lr_reg = GetTarget().GetLinkReg();
    EncodeLdr(lr_reg, false, entry_point);
    GetMasm()->Blr(VixlReg(lr_reg));
}

void Aarch64Encoder::MakeCall(Reg reg)
{
    GetMasm()->Blr(VixlReg(reg));
}

void Aarch64Encoder::MakeCall(LabelHolder::LabelId id)
{
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->Bl(label);
}

void Aarch64Encoder::LoadPcRelative(Reg reg, intptr_t offset, Reg reg_addr)
{
    ASSERT(GetCodeOffset() != Encoder::INVALID_OFFSET);
    ASSERT(reg.IsValid() || reg_addr.IsValid());

    if (!reg_addr.IsValid()) {
        reg_addr = reg.As(INT64_TYPE);
    }

    if (vixl::IsInt21(offset)) {
        GetMasm()->adr(VixlReg(reg_addr), offset);
        if (reg != INVALID_REGISTER) {
            EncodeLdr(reg, false, MemRef(reg_addr));
        }
    } else {
        size_t pc = GetCodeOffset() + GetCursorOffset();
        size_t addr;
        if (intptr_t res = helpers::ToSigned(pc) + offset; res < 0) {
            // Make both, pc and addr, positive
            ssize_t extend = RoundUp(std::abs(res), vixl::aarch64::kPageSize);
            addr = res + extend;
            pc += extend;
        } else {
            addr = res;
        }

        ssize_t adrp_imm = (addr >> vixl::aarch64::kPageSizeLog2) - (pc >> vixl::aarch64::kPageSizeLog2);

        GetMasm()->adrp(VixlReg(reg_addr), adrp_imm);

        offset = panda::helpers::ToUnsigned(addr) & (vixl::aarch64::kPageSize - 1);
        if (reg.GetId() != reg_addr.GetId()) {
            EncodeAdd(reg_addr, reg_addr, Imm(offset));
            if (reg != INVALID_REGISTER) {
                EncodeLdr(reg, true, MemRef(reg_addr));
            }
        } else {
            EncodeLdr(reg, true, MemRef(reg_addr, offset));
        }
    }
}

void Aarch64Encoder::MakeCallAot(intptr_t offset)
{
    auto lr_reg = GetTarget().GetLinkReg();
    LoadPcRelative(lr_reg, offset);
    GetMasm()->Blr(VixlReg(lr_reg));
}

bool Aarch64Encoder::CanMakeCallByOffset(intptr_t offset)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    auto off = (offset >> vixl::aarch64::kInstructionSizeLog2);
    return vixl::aarch64::Instruction::IsValidImmPCOffset(vixl::aarch64::ImmBranchType::UncondBranchType, off);
}

void Aarch64Encoder::MakeCallByOffset(intptr_t offset)
{
    GetMasm()->Bl(offset);
}

void Aarch64Encoder::MakeLoadAotTable(intptr_t offset, Reg reg)
{
    LoadPcRelative(reg, offset);
}

void Aarch64Encoder::MakeLoadAotTableAddr(intptr_t offset, Reg addr, Reg val)
{
    LoadPcRelative(val, offset, addr);
}

void Aarch64Encoder::EncodeAbort()
{
    GetMasm()->Brk();
}

void Aarch64Encoder::EncodeReturn()
{
    GetMasm()->Ret();
}

void Aarch64Encoder::EncodeMul([[maybe_unused]] Reg unused1, [[maybe_unused]] Reg unused2, [[maybe_unused]] Imm unused3)
{
    SetFalseResult();
}

void Aarch64Encoder::EncodeMov(Reg dst, Reg src)
{
    if (dst == src) {
        return;
    }
    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            GetMasm()->Fcvt(VixlVReg(dst), VixlVReg(src));
            return;
        }
        GetMasm()->Fmov(VixlVReg(dst), VixlVReg(src));
        return;
    }
    if (src.IsFloat() && !dst.IsFloat()) {
        GetMasm()->Fmov(VixlReg(dst, src.GetSize()), VixlVReg(src));
        return;
    }
    if (dst.IsFloat()) {
        ASSERT(src.IsScalar());
        GetMasm()->Fmov(VixlVReg(dst), VixlReg(src));
        return;
    }
    // DiscardForSameWReg below means we would drop "mov w0, w0", but it is guarded by "dst == src" above anyway.
    // NOTE: "mov w0, w0" is not equal "nop", as it clears upper bits of x0.
    // Keeping the option here helps to generate nothing when e.g. src is x0 and dst is w0.
    // Probably, a better solution here is to system-wide checking register size on Encoder level.
    if (src.GetSize() != dst.GetSize()) {
        auto src_reg = Reg(src.GetId(), dst.GetType());
        GetMasm()->Mov(VixlReg(dst), VixlReg(src_reg), vixl::aarch64::DiscardMoveMode::kDiscardForSameWReg);
        return;
    }
    GetMasm()->Mov(VixlReg(dst), VixlReg(src), vixl::aarch64::DiscardMoveMode::kDiscardForSameWReg);
}

void Aarch64Encoder::EncodeNeg(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Fneg(VixlVReg(dst), VixlVReg(src));
        return;
    }
    GetMasm()->Neg(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeAbs(Reg dst, Reg src)
{
    if (dst.IsFloat()) {
        GetMasm()->Fabs(VixlVReg(dst), VixlVReg(src));
        return;
    }

    ASSERT(!GetRegfile()->IsZeroReg(dst));
    if (GetRegfile()->IsZeroReg(src)) {
        EncodeMov(dst, src);
        return;
    }

    if (src.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Cmp(VixlReg(src), vixl::aarch64::xzr);
    } else {
        GetMasm()->Cmp(VixlReg(src), vixl::aarch64::wzr);
    }
    GetMasm()->Cneg(VixlReg(Promote(dst)), VixlReg(Promote(src)), vixl::aarch64::Condition::lt);
}

void Aarch64Encoder::EncodeSqrt(Reg dst, Reg src)
{
    ASSERT(dst.IsFloat());
    GetMasm()->Fsqrt(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeIsInf(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    if (src.GetSize() == WORD_SIZE) {
        constexpr uint32_t INF_MASK = 0xff000000;

        ScopedTmpRegU32 tmp_reg(this);
        auto tmp = VixlReg(tmp_reg);
        GetMasm()->Fmov(tmp, VixlVReg(src));
        GetMasm()->Mov(VixlReg(dst).W(), INF_MASK);
        GetMasm()->Lsl(tmp, tmp, 1);
        GetMasm()->Cmp(tmp, VixlReg(dst, WORD_SIZE));
    } else {
        constexpr uint64_t INF_MASK = 0xffe0000000000000;

        ScopedTmpRegU64 tmp_reg(this);
        auto tmp = VixlReg(tmp_reg);
        GetMasm()->Fmov(tmp, VixlVReg(src));
        GetMasm()->Mov(VixlReg(dst).X(), INF_MASK);
        GetMasm()->Lsl(tmp, tmp, 1);
        GetMasm()->Cmp(tmp, VixlReg(dst, DOUBLE_WORD_SIZE));
    }

    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::eq);
}

/* NaN values are needed to be canonicalized */
void Aarch64Encoder::EncodeFpToBits(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(dst.GetSize() == WORD_SIZE || dst.GetSize() == DOUBLE_WORD_SIZE);

    if (dst.GetSize() == WORD_SIZE) {
        ASSERT(src.GetSize() == WORD_SIZE);

        constexpr auto FNAN = 0x7fc00000;

        ScopedTmpRegU32 tmp(this);

        GetMasm()->Fcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Mov(VixlReg(tmp), FNAN);
        GetMasm()->Umov(VixlReg(dst), VixlVReg(src), 0);
        GetMasm()->Csel(VixlReg(dst), VixlReg(tmp), VixlReg(dst), vixl::aarch64::Condition::ne);
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);

        constexpr auto DNAN = 0x7ff8000000000000;

        ScopedTmpRegU64 tmp_reg(this);
        auto tmp = VixlReg(tmp_reg);

        GetMasm()->Fcmp(VixlVReg(src), VixlVReg(src));
        GetMasm()->Mov(tmp, DNAN);
        GetMasm()->Umov(VixlReg(dst), VixlVReg(src), 0);
        GetMasm()->Csel(VixlReg(dst), tmp, VixlReg(dst), vixl::aarch64::Condition::ne);
    }
}

void Aarch64Encoder::EncodeMoveBitsRaw(Reg dst, Reg src)
{
    ASSERT((dst.IsFloat() && src.IsScalar()) || (src.IsFloat() && dst.IsScalar()));
    if (dst.IsScalar()) {
        ASSERT(src.GetSize() == dst.GetSize());
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Umov(VixlReg(dst).W(), VixlVReg(src).S(), 0);
        } else {
            GetMasm()->Umov(VixlReg(dst), VixlVReg(src), 0);
        }
    } else {
        ASSERT(dst.GetSize() == src.GetSize());
        ScopedTmpReg tmp_reg(this, src.GetType());
        auto src_reg = src;
        auto rzero = GetRegfile()->GetZeroReg();
        if (src.GetId() == rzero.GetId()) {
            EncodeMov(tmp_reg, Imm(0));
            src_reg = tmp_reg;
        }

        if (src_reg.GetSize() == WORD_SIZE) {
            GetMasm()->Fmov(VixlVReg(dst).S(), VixlReg(src_reg).W());
        } else {
            GetMasm()->Fmov(VixlVReg(dst), VixlReg(src_reg));
        }
    }
}

void Aarch64Encoder::EncodeReverseBytes(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (src.GetId() == rzero.GetId()) {
        EncodeMov(dst, Imm(0));
        return;
    }

    ASSERT(src.GetSize() > BYTE_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Rev16(VixlReg(dst), VixlReg(src));
        GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
    } else {
        GetMasm()->Rev(VixlReg(dst), VixlReg(src));
    }
}

void Aarch64Encoder::EncodeBitCount(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (src.GetId() == rzero.GetId()) {
        EncodeMov(dst, Imm(0));
        return;
    }

    ASSERT(dst.GetSize() == WORD_SIZE);

    ScopedTmpRegF64 tmp_reg0(this);
    vixl::aarch64::VRegister tmp_reg;
    if (src.GetSize() == DOUBLE_WORD_SIZE) {
        tmp_reg = VixlVReg(tmp_reg0).D();
    } else {
        tmp_reg = VixlVReg(tmp_reg0).S();
    }

    if (src.GetSize() < WORD_SIZE) {
        int64_t cut_value = (1ULL << src.GetSize()) - 1;
        EncodeAnd(src, src, Imm(cut_value));
    }

    GetMasm()->Fmov(tmp_reg, VixlReg(src));
    GetMasm()->Cnt(tmp_reg.V8B(), tmp_reg.V8B());
    GetMasm()->Addv(tmp_reg.B(), tmp_reg.V8B());
    EncodeMov(dst, tmp_reg0);
}

/* Since only ROR is supported on AArch64 we do
 * left rotaion as ROR(v, -count) */
void Aarch64Encoder::EncodeRotate(Reg dst, Reg src1, Reg src2, bool is_ror)
{
    ASSERT(src1.GetSize() == WORD_SIZE || src1.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src1.GetSize() == dst.GetSize());
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src2.GetId() || rzero.GetId() == src1.GetId()) {
        EncodeMov(dst, src1);
        return;
    }
    /* as the second parameters is always 32-bits long we have to
     * adjust the counter register for the 64-bits first operand case */
    if (is_ror) {
        auto count = (dst.GetSize() == WORD_SIZE ? VixlReg(src2) : VixlReg(src2).X());
        GetMasm()->Ror(VixlReg(dst), VixlReg(src1), count);
    } else {
        ScopedTmpReg tmp(this);
        auto cnt = (dst.GetId() == src1.GetId() ? tmp : dst);
        auto count = (dst.GetSize() == WORD_SIZE ? VixlReg(cnt).W() : VixlReg(cnt).X());
        auto source2 = (dst.GetSize() == WORD_SIZE ? VixlReg(src2).W() : VixlReg(src2).X());
        GetMasm()->Neg(count, source2);
        GetMasm()->Ror(VixlReg(dst), VixlReg(src1), count);
    }
}

void Aarch64Encoder::EncodeSignum(Reg dst, Reg src)
{
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    ScopedTmpRegU32 tmp(this);
    auto sign = (dst.GetId() == src.GetId() ? tmp : dst);

    GetMasm()->Cmp(VixlReg(src), VixlImm(0));
    GetMasm()->Cset(VixlReg(sign), vixl::aarch64::Condition::gt);

    constexpr auto SHIFT_WORD_BITS = 31;
    constexpr auto SHIFT_DWORD_BITS = 63;

    /* The operation below is "sub dst, dst, src, lsr #reg_size-1"
     * however, we can only encode as many as 32 bits in lsr field, so
     * for 64-bits cases we cannot avoid having a separate lsr instruction */
    if (src.GetSize() == WORD_SIZE) {
        auto shift = Shift(src, LSR, SHIFT_WORD_BITS);
        EncodeSub(dst, sign, shift);
    } else {
        ScopedTmpRegU64 shift(this);
        sign = Reg(sign.GetId(), INT64_TYPE);
        EncodeShr(shift, src, Imm(SHIFT_DWORD_BITS));
        EncodeSub(dst, sign, shift);
    }
}

void Aarch64Encoder::EncodeCountLeadingZeroBits(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src.GetId()) {
        EncodeMov(dst, Imm(src.GetSize()));
        return;
    }
    GetMasm()->Clz(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeCountTrailingZeroBits(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src.GetId()) {
        EncodeMov(dst, Imm(src.GetSize()));
        return;
    }
    GetMasm()->Rbit(VixlReg(dst), VixlReg(src));
    GetMasm()->Clz(VixlReg(dst), VixlReg(dst));
}

void Aarch64Encoder::EncodeCeil(Reg dst, Reg src)
{
    GetMasm()->Frintp(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeFloor(Reg dst, Reg src)
{
    GetMasm()->Frintm(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeRint(Reg dst, Reg src)
{
    GetMasm()->Frintn(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeRound(Reg dst, Reg src)
{
    auto done = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    ScopedTmpReg tmp(this, src.GetType());
    // round to nearest integer, ties away from zero
    GetMasm()->Fcvtas(VixlReg(dst), VixlVReg(src));
    // for positive values, zero and NaN inputs rounding is done
    GetMasm()->Tbz(VixlReg(dst), dst.GetSize() - 1, done);
    // if input is negative but not a tie, round to nearest is valid
    // if input is a negative tie, dst += 1
    GetMasm()->Frinta(VixlVReg(tmp), VixlVReg(src));
    GetMasm()->Fsub(VixlVReg(tmp), VixlVReg(src), VixlVReg(tmp));
    // NOLINTNEXTLINE(readability-magic-numbers)
    const auto HALF = 0.5;
    GetMasm()->Fcmp(VixlVReg(tmp), HALF);
    GetMasm()->Cinc(VixlReg(dst), VixlReg(dst), vixl::aarch64::Condition::eq);
    GetMasm()->Bind(done);
}

void Aarch64Encoder::EncodeStringEquals(Reg dst, Reg str1, Reg str2, bool COMPRESSION, uint32_t LENGTH_OFFSET,
                                        uint32_t DATA_OFFSET)
{
    /* Pseudo code:
      if (str1 == str2) return true;                                // pointers compare. Fast path for same object
      if (str1.length_field() != str2.length_field()) return false; // case length or compression is different

      // code below use tmp3 both as counter and as offset to keep str1 and str2 untouched and to
      // use minimal amount of scratch register. Then only 3 scratch registers are used: tmp1 and tmp2 for
      // loaded string data of str1 and str2 respectively. And tmp3 as counter and offset at the same time.
      // Then tmp3 will be "DATA_OFFSET + <offset inside string data>" almost everywhere. Check string from
      // the end to make tmp3 manipulation easier. It'll be probably a bit less effective on large string and
      // almost identical strings due to mostly unaligned access, but we can ignore it because most strings
      // are less than 32 chars and in most cases it'll be different characters on first comparison. Then simpler
      // code without additional operations wins.

      int tmp3 = str1.length() * <size of str1 characters>;         // data size in bytes
      tmp3 = tmp3 + DATA_OFFSET - DOUBLE_WORD_SIZE_BYTE;            // offset of last 8 data bytes (last octet)
      while (tmp3 >= DATA_OFFSET) {
          if (<load-8-bytes-at>(str1 + tmp3) != <load-8-bytes-at>(str2 + tmp3)) return false;
          tmp3 -= 8;
      }
      // less than 8 bytes left to load and check. possibly 0.
      if (tmp3 == DATA_OFFSET - DOUBLE_WORD_SIZE_BYTE) return true; // 0 bytes left
      // 1..7 bytes left. Read whole octet (8 bytes) including few bytes from object header. Shift off header bytes
      tmp1 = <load-8-bytes-at>(str1 + tmp3);
      tmp2 = <load-8-bytes-at>(str2 + tmp3);
      tmp3 = tmp3 - DATA_OFFSET;                                    // <useful bytes> - 8 (== -<bytes to shift off>)
      // calculate amount of bits to shift off. Note that for negative numbers shift result is undefined behavior
      // for some languages like c/c++, but it's still fine for h/w logical shift on assembly level. We can use it.
      tmp3 = - (tmp3 << 3);
      if ((tmp1 >> tmp3) != (tmp2 >> tmp3)) return false;
      return true;
    */

    ASSERT(dst.IsScalar());

    ScopedTmpRegU64 tmp1_scoped(this);
    ScopedTmpRegU64 tmp2_scoped(this);
    ScopedTmpRegU64 tmp3_scoped(this);

    auto tmp1_u32 = VixlReg(Reg(tmp1_scoped.GetReg().GetId(), INT32_TYPE));  // 32-bit alias for tmp1
    auto tmp2_u32 = VixlReg(Reg(tmp2_scoped.GetReg().GetId(), INT32_TYPE));  // 32-bit alias for tmp2

    auto tmp1 = VixlReg(tmp1_scoped.GetReg());
    auto tmp2 = VixlReg(tmp2_scoped.GetReg());
    auto tmp3 = VixlReg(tmp3_scoped.GetReg());

    auto label_false = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_cset = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    // compare to itself case
    GetMasm()->Cmp(VixlReg(str1), VixlReg(str2));
    GetMasm()->B(label_cset, vixl::aarch64::Condition::eq);

    EncodeLdr(tmp1_scoped.GetReg().As(INT32_TYPE), false, MemRef(str1, LENGTH_OFFSET));
    EncodeLdr(tmp2_scoped.GetReg().As(INT32_TYPE), false, MemRef(str2, LENGTH_OFFSET));

    // compare length and potentially, compressed-string status
    GetMasm()->Cmp(tmp1_u32, tmp2_u32);
    GetMasm()->B(label_cset, vixl::aarch64::Condition::ne);

    // compare data. Assume result is "true" unless different bytes found
    if (COMPRESSION) {
        // branchless byte length calculation
        GetMasm()->Lsr(tmp1_u32, tmp1_u32, 1);  // string length
        GetMasm()->And(tmp2_u32, tmp2_u32, 1);  // compressed-string bit. If 1 then not compressed.
        GetMasm()->Lsl(tmp3, tmp1, tmp2);       // if not compressed, then shift left by 1 bit
    }
    EncodeStringEqualsMainLoop(dst, str1, str2, tmp1_scoped, tmp2_scoped, tmp3_scoped, label_false, label_cset,
                               DATA_OFFSET);
}

void Aarch64Encoder::EncodeStringEqualsMainLoop(Reg dst, Reg str1, Reg str2, Reg tmp1_scoped, Reg tmp2_scoped,
                                                Reg tmp3_scoped, vixl::aarch64::Label *label_false,
                                                vixl::aarch64::Label *label_cset, const uint32_t DATA_OFFSET)
{
    auto tmp1 = VixlReg(tmp1_scoped);
    auto tmp2 = VixlReg(tmp2_scoped);
    auto tmp3 = VixlReg(tmp3_scoped);

    auto label_loop_begin = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_end = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_loop = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    // Now tmp3 is byte-counter. Use it as offset register as well.
    GetMasm()->Add(tmp3, tmp3, DATA_OFFSET - DOUBLE_WORD_SIZE_BYTE);
    GetMasm()->B(label_loop_begin);
    GetMasm()->Bind(label_false);
    EncodeMov(dst, Imm(0));
    GetMasm()->B(label_end);
    // case: >=8 bytes
    GetMasm()->Bind(label_loop);

    auto str1_last_word_mem = MemRef(str1, tmp3_scoped, 0);
    auto str2_last_word_mem = MemRef(str2, tmp3_scoped, 0);

    {
        EncodeLdr(tmp1_scoped, false, str1_last_word_mem);
        EncodeLdr(tmp2_scoped, false, str2_last_word_mem);
        GetMasm()->Cmp(tmp1, tmp2);
        GetMasm()->B(label_cset, vixl::aarch64::Condition::ne);
        GetMasm()->Sub(tmp3, tmp3, DOUBLE_WORD_SIZE_BYTE);
        GetMasm()->Bind(label_loop_begin);
        GetMasm()->Cmp(tmp3, DATA_OFFSET);
        GetMasm()->B(label_loop, vixl::aarch64::Condition::ge);
    }

    // case: 0..7 bytes left (tmp3 is DATA_OFFSET + -8..0)
    GetMasm()->Cmp(tmp3, DATA_OFFSET - DOUBLE_WORD_SIZE_BYTE);
    GetMasm()->B(label_cset, vixl::aarch64::Condition::eq);
    EncodeLdr(tmp1_scoped, false, str1_last_word_mem);
    EncodeLdr(tmp2_scoped, false, str2_last_word_mem);
    // 1..7 bytes left to check. tmp3 is DATA_OFFSET + -7..-1
    GetMasm()->Sub(tmp3, tmp3, DATA_OFFSET);

    auto zero = VixlReg(GetRegfile()->GetZeroReg(), DOUBLE_WORD_SIZE);
    // tmp3 is now -(amount_of_bytes_to_shift_off). Convert it to bits via single instruction
    GetMasm()->Sub(tmp3, zero, vixl::aarch64::Operand(tmp3, vixl::aarch64::Shift::LSL, 3));
    GetMasm()->Lsr(tmp1, tmp1, tmp3);
    GetMasm()->Lsr(tmp2, tmp2, tmp3);
    GetMasm()->Cmp(tmp1, tmp2);
    GetMasm()->Bind(label_cset);
    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::eq);
    GetMasm()->Bind(label_end);
}

void Aarch64Encoder::EncodeCrc32Update(Reg dst, Reg crc_reg, Reg val_reg)
{
    auto tmp =
        dst.GetId() != crc_reg.GetId() && dst.GetId() != val_reg.GetId() ? dst : ScopedTmpReg(this, dst.GetType());
    GetMasm()->Mvn(VixlReg(tmp), VixlReg(crc_reg));
    GetMasm()->Crc32b(VixlReg(tmp), VixlReg(tmp), VixlReg(val_reg));
    GetMasm()->Mvn(VixlReg(dst), VixlReg(tmp));
}

/**
 * Helper function for generating String::indexOf intrinsic: case of Latin1 (8-bit) character search
 *
 * Inputs: str - pointer to first character in string
 *         character - character to search
 *         idx: original start index
 *         tmp: address of 1st string character
 *         tmp1: length field value (potentially with compression bit).
 *         tmp2: MAX(idx, 0) if idx is not zero register. Anything otherwise.
 *         tmp3: temporary register to use
 *         label_found: label to jump when match found.
 *               Label contract requirement 1: leave calculated result in tmp1.
 *         label_not_found: label to jump when no match found.
 * Assumptions: starting search index is less than string length (tmp1)
 */
void Aarch64Encoder::IndexOfHandleLatin1Case(Reg str, Reg character, Reg idx, Reg tmp, const bool COMPRESSION,
                                             const uint32_t DATA_OFFSET, const vixl::aarch64::Register &tmp1,
                                             const vixl::aarch64::Register &tmp2, const vixl::aarch64::Register &tmp3,
                                             vixl::aarch64::Label *label_found, vixl::aarch64::Label *label_not_found)
{
    // vixl register aliases
    auto character_w = VixlReg(character).W();
    auto character_x = VixlReg(character).X();
    auto tmp0_x = VixlReg(tmp).X();

    // more vixl aliases
    auto lsl = vixl::aarch64::Shift::LSL;
    auto lsr = vixl::aarch64::Shift::LSR;

    bool idx_is_zero = (idx.GetId() == GetRegfile()->GetZeroReg().GetId());
    bool character_is_zero = (character.GetId() == GetRegfile()->GetZeroReg().GetId());

    // calculate address of first byte after string
    if (COMPRESSION) {
        GetMasm()->Add(tmp0_x, tmp0_x, vixl::aarch64::Operand(tmp1.X(), lsr, 1));
        if (idx_is_zero) {
            GetMasm()->Neg(tmp2.X(), vixl::aarch64::Operand(tmp1.X(), lsr, 1));
        } else {
            GetMasm()->Sub(tmp2.X(), tmp2.X(), vixl::aarch64::Operand(tmp1.X(), lsr, 1));
        }
    } else {
        if (idx_is_zero) {
            GetMasm()->Neg(tmp2.X(), tmp1.X());
        } else {
            GetMasm()->Sub(tmp2.X(), tmp2.X(), tmp1.X());
        }
    }
    GetMasm()->Cmp(tmp2.X(), -DOUBLE_WORD_SIZE_BYTE);

    auto label_small_loop = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    GetMasm()->B(label_small_loop, vixl::aarch64::Condition::gt);
    // clone character to the size of register (i.e. 8 x 8-bit characters)
    if (!character_is_zero) {
        GetMasm()->Orr(character_w, character_w, vixl::aarch64::Operand(character_w, lsl, BYTE_SIZE));
        GetMasm()->Orr(character_w, character_w, vixl::aarch64::Operand(character_w, lsl, HALF_SIZE));
        GetMasm()->Orr(character_x, character_x, vixl::aarch64::Operand(character_x, lsl, WORD_SIZE));
    }
    IndexOfHandleLatin1CaseMainLoop(str, character, tmp, DATA_OFFSET, tmp1, tmp2, tmp3, label_found, label_not_found,
                                    label_small_loop);
}

// constants for the indexOf implementation
constexpr int32_t MAX_8BIT_CHAR = 0xFF;
constexpr int32_t LOG2_BITS_PER_BYTE = 3;
constexpr uint32_t CLEAR_BIT_MASK = -2;
constexpr int32_t MIN_SUPPLEMENTARY_CODE_POINT = 0x10000;
constexpr int32_t MAX_SUPPLEMENTARY_CODE_POINT = 0X10FFFF;
constexpr uint64_t LATIN1_MASK = 0x7f7f7f7f7f7f7f7f;
constexpr uint64_t LATIN1_MASK2 = 0x0101010101010101;

void Aarch64Encoder::IndexOfHandleLatin1CaseMainLoop(
    Reg str, Reg character, Reg tmp, const uint32_t DATA_OFFSET, const vixl::aarch64::Register &tmp1,
    const vixl::aarch64::Register &tmp2, const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
    vixl::aarch64::Label *label_not_found, vixl::aarch64::Label *label_small_loop)
{
    auto character_w = VixlReg(character).W();
    bool character_is_zero = (character.GetId() == GetRegfile()->GetZeroReg().GetId());

    auto label_loop = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_has_zero = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_small_match = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_not_found_restore_char = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    auto mem_tmp_tmp2_x = vixl::aarch64::MemOperand(VixlReg(tmp).X(), tmp2.X());

    GetMasm()->Bind(label_loop);
    {
        GetMasm()->Ldr(tmp1.X(), mem_tmp_tmp2_x);
        GetMasm()->Mov(tmp3.X(), LATIN1_MASK2);  // can (re)init during ldr to save 1 reg
        GetMasm()->Eor(tmp1.X(), tmp1.X(), VixlReg(character).X());
        GetMasm()->Sub(tmp3.X(), tmp1.X(), tmp3.X());
        GetMasm()->Orr(tmp1.X(), tmp1.X(), LATIN1_MASK);
        GetMasm()->Bics(tmp1.X(), tmp3.X(), tmp1.X());
        GetMasm()->B(label_has_zero, vixl::aarch64::Condition::ne);
        GetMasm()->Add(tmp2.X(), tmp2.X(), DOUBLE_WORD_SIZE_BYTE);
        GetMasm()->Cmp(tmp2.X(), -DOUBLE_WORD_SIZE_BYTE);        // has enough bytes left to read whole register?
        GetMasm()->B(label_loop, vixl::aarch64::Condition::lt);  // yes. time to loop
    }
    GetMasm()->Cbz(tmp2.X(), character_is_zero ? label_not_found : label_not_found_restore_char);  // done
    GetMasm()->Mov(tmp2.X(), -DOUBLE_WORD_SIZE_BYTE);  // setup data to read last 8 bytes. One more loop
    GetMasm()->B(label_loop);
    GetMasm()->Bind(label_small_loop);
    {
        GetMasm()->Ldrb(tmp1.W(), mem_tmp_tmp2_x);
        GetMasm()->Cmp(tmp1.W(), character_w);
        GetMasm()->B(label_small_match, vixl::aarch64::Condition::eq);
        GetMasm()->Adds(tmp2.X(), tmp2.X(), BYTE_SIZE / BITS_PER_BYTE);
        GetMasm()->Cbnz(tmp2.X(), label_small_loop);
        GetMasm()->B(label_not_found);
    }
    GetMasm()->Bind(label_has_zero);
    GetMasm()->Rev(tmp1.X(), tmp1.X());
    if (!character_is_zero) {
        GetMasm()->And(character_w, character_w, MAX_8BIT_CHAR);
    }
    GetMasm()->Clz(tmp1.X(), tmp1.X());  // difference bit index in current octet
    GetMasm()->Add(tmp2.X(), tmp2.X(), vixl::aarch64::Operand(tmp1.X(), vixl::aarch64::Shift::ASR, LOG2_BITS_PER_BYTE));
    GetMasm()->Bind(label_small_match);
    // string length in bytes is: tmp - str - DATA_OFFSET
    GetMasm()->Add(tmp2.X(), tmp2.X(), VixlReg(tmp).X());
    GetMasm()->Sub(tmp2.X(), tmp2.X(), VixlReg(str).X());
    GetMasm()->Sub(tmp2.X(), tmp2.X(), DATA_OFFSET);
    GetMasm()->B(label_found);
    GetMasm()->Bind(label_not_found_restore_char);
    if (!character_is_zero) {
        GetMasm()->And(character_w, character_w, MAX_8BIT_CHAR);
    }
    GetMasm()->B(label_not_found);
}

constexpr uint32_t UTF16_IDX2OFFSET_SHIFT = 1;

/**
 * Helper function for generating String::indexOf intrinsic: case of normal utf-16 character search
 *
 * Inputs: str - pointer to first character in string
 *         character - character to search
 *         idx: original start index
 *         tmp: address of 1st string character
 *         tmp1: length field value (potentially with compression bit).
 *         tmp2: MAX(idx, 0) if idx is not zero register. Anything otherwise.
 *         tmp3: temporary register to use
 *         label_found: label to jump when match found.
 *               Label contract requirement 1: leave calculated result in tmp1.
 *         label_not_found: label to jump when no match found.
 * Assumptions: starting search index is less than string length (tmp1)
 */
void Aarch64Encoder::IndexOfHandleUtf16NormalCase(Reg str, Reg character, Reg idx, Reg tmp, const bool COMPRESSION,
                                                  const uint32_t DATA_OFFSET, const vixl::aarch64::Register &tmp1,
                                                  const vixl::aarch64::Register &tmp2,
                                                  const vixl::aarch64::Register &tmp3,
                                                  vixl::aarch64::Label *label_found,
                                                  vixl::aarch64::Label *label_not_found)
{
    // vixl register aliases
    auto character_w = VixlReg(character).W();
    auto character_x = VixlReg(character).X();
    auto tmp0_x = VixlReg(tmp).X();

    // more vixl aliases
    auto lsl = vixl::aarch64::Shift::LSL;

    // local labels
    auto label_small_loop = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    bool idx_is_zero = (idx.GetId() == GetRegfile()->GetZeroReg().GetId());
    bool character_is_zero = (character.GetId() == GetRegfile()->GetZeroReg().GetId());

    if (COMPRESSION) {
        GetMasm()->And(tmp1.W(), tmp1.W(), CLEAR_BIT_MASK);  // clear lowest bit to get string length in bytes
    } else {
        GetMasm()->Lsl(tmp1.W(), tmp1.W(), 1);  // string length in bytes for non-compressed case
    }
    // amount of bytes to scan in worst case
    GetMasm()->Add(tmp0_x, tmp0_x, tmp1.X());  // calculate address of first byte after string
    if (idx_is_zero) {
        GetMasm()->Neg(tmp2.X(), tmp1.X());
    } else {
        GetMasm()->Sub(tmp2.X(), tmp1.X(), vixl::aarch64::Operand(tmp2.X(), lsl, UTF16_IDX2OFFSET_SHIFT));
        GetMasm()->Neg(tmp2.X(), tmp2.X());
    }
    GetMasm()->Cmp(tmp2.X(), -DOUBLE_WORD_SIZE_BYTE);
    GetMasm()->B(label_small_loop, vixl::aarch64::Condition::gt);
    // clone character to the size of register (i.e. 4 x 16-bit characters)
    if (!character_is_zero) {
        GetMasm()->Orr(character_w, character_w, vixl::aarch64::Operand(character_w, lsl, HALF_SIZE));
        GetMasm()->Orr(character_x, character_x, vixl::aarch64::Operand(character_x, lsl, WORD_SIZE));
    }
    IndexOfHandleUtf16NormalCaseMainLoop(str, character, tmp, DATA_OFFSET, tmp1, tmp2, tmp3, label_found,
                                         label_not_found, label_small_loop);
}

constexpr uint64_t UTF16_MASK = 0x7fff7fff7fff7fff;
constexpr uint64_t UTF16_MASK2 = 0x0001000100010001;
constexpr int32_t MAX_UTF16_CHAR = 0xFFFF;

void Aarch64Encoder::IndexOfHandleUtf16NormalCaseMainLoop(
    Reg str, Reg character, Reg tmp, const uint32_t DATA_OFFSET, const vixl::aarch64::Register &tmp1,
    const vixl::aarch64::Register &tmp2, const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
    vixl::aarch64::Label *label_not_found, vixl::aarch64::Label *label_small_loop)
{
    auto tmp0_x = VixlReg(tmp).X();
    auto character_w = VixlReg(character).W();
    bool character_is_zero = (character.GetId() == GetRegfile()->GetZeroReg().GetId());

    auto label_loop = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_has_zero = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_small_match = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_not_found_restore_char = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Bind(label_loop);
    {
        GetMasm()->Ldr(tmp1.X(), vixl::aarch64::MemOperand(tmp0_x, tmp2.X()));
        GetMasm()->Mov(tmp3.X(), UTF16_MASK2);  // can (re)init during ldr to save 1 reg
        GetMasm()->Eor(tmp1.X(), tmp1.X(), VixlReg(character).X());
        GetMasm()->Sub(tmp3.X(), tmp1.X(), tmp3.X());
        GetMasm()->Orr(tmp1.X(), tmp1.X(), UTF16_MASK);
        GetMasm()->Bics(tmp1.X(), tmp3.X(), tmp1.X());
        GetMasm()->B(label_has_zero, vixl::aarch64::Condition::ne);
        GetMasm()->Add(tmp2.X(), tmp2.X(), DOUBLE_WORD_SIZE_BYTE);
        GetMasm()->Cmp(tmp2.X(), -DOUBLE_WORD_SIZE_BYTE);        // has enough bytes left to read whole register?
        GetMasm()->B(label_loop, vixl::aarch64::Condition::lt);  // yes. time to loop
    }
    GetMasm()->Cbz(tmp2.X(), character_is_zero ? label_not_found : label_not_found_restore_char);  // done
    GetMasm()->Mov(tmp2.X(), -DOUBLE_WORD_SIZE_BYTE);  // setup data to read last 8 bytes. One more loop
    GetMasm()->B(label_loop);
    GetMasm()->Bind(label_small_loop);
    {
        GetMasm()->Ldrh(tmp1.W(), vixl::aarch64::MemOperand(tmp0_x, tmp2.X()));
        GetMasm()->Cmp(tmp1.W(), character_w);
        GetMasm()->B(label_small_match, vixl::aarch64::Condition::eq);
        GetMasm()->Adds(tmp2.X(), tmp2.X(), HALF_SIZE / BITS_PER_BYTE);
        GetMasm()->Cbnz(tmp2.X(), label_small_loop);
        GetMasm()->B(label_not_found);
    }
    GetMasm()->Bind(label_has_zero);
    GetMasm()->Rev(tmp1.X(), tmp1.X());
    if (!character_is_zero) {
        GetMasm()->And(character_w, character_w, MAX_UTF16_CHAR);
    }
    GetMasm()->Clz(tmp1.X(), tmp1.X());  // difference bit index in current octet
    GetMasm()->Add(tmp2.X(), tmp2.X(), vixl::aarch64::Operand(tmp1.X(), vixl::aarch64::Shift::ASR, LOG2_BITS_PER_BYTE));
    GetMasm()->Bind(label_small_match);
    // string length in bytes is: tmp - str - DATA_OFFSET
    GetMasm()->Add(tmp2.X(), tmp2.X(), tmp0_x);
    GetMasm()->Sub(tmp2.X(), tmp2.X(), VixlReg(str).X());
    GetMasm()->Sub(tmp2.X(), tmp2.X(), DATA_OFFSET);
    GetMasm()->Lsr(tmp2.X(), tmp2.X(), UTF16_IDX2OFFSET_SHIFT);
    GetMasm()->B(label_found);
    GetMasm()->Bind(label_not_found_restore_char);
    if (!character_is_zero) {
        GetMasm()->And(character_w, character_w, MAX_UTF16_CHAR);
    }
    GetMasm()->B(label_not_found);
}

/**
 * Helper function for generating String::indexOf intrinsic: case of surrogate character search
 *
 * Inputs: str - pointer to first character in string
 *         character - character to search
 *         idx: original start index
 *         tmp: address of 1st string character
 *         tmp1: length field value (potentially with compression bit).
 *         tmp2: MAX(idx, 0) if idx is not zero register. Anything otherwise.
 *         tmp3: temporary register to use
 *         label_found: label to jump when match found.
 *               Label contract requirement 1: leave calculated result in tmp1.
 *         label_not_found: label to jump when no match found.
 * Assumptions: starting search index is less than string length (tmp1)
 */
void Aarch64Encoder::IndexOfHandleSurrogateCase(Reg str, Reg character, Reg idx, Reg tmp, const bool COMPRESSION,
                                                const uint32_t DATA_OFFSET, const vixl::aarch64::Register &tmp1,
                                                const vixl::aarch64::Register &tmp2,
                                                const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
                                                vixl::aarch64::Label *label_not_found)
{
    // local constants
    constexpr uint32_t MIN_HIGH_SURROGATE = 0xD800;
    constexpr uint32_t MIN_LOW_SURROGATE = 0xDC00;
    constexpr uint32_t SURROGATE_LOW_BITS = 10;

    // vixl register aliases
    auto character_w = VixlReg(character).W();
    auto str_x = VixlReg(str).X();
    auto tmp0_x = VixlReg(tmp).X();

    // more vixl aliases
    auto lsl = vixl::aarch64::Shift::LSL;
    auto lsr = vixl::aarch64::Shift::LSR;

    // local labels
    auto label_sur_loop = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_match = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    bool idx_is_zero = (idx.GetId() == GetRegfile()->GetZeroReg().GetId());

    if (COMPRESSION) {
        GetMasm()->And(tmp1.W(), tmp1.W(), CLEAR_BIT_MASK);  // clear lowest bit to get string length in bytes
    } else {
        GetMasm()->Lsl(tmp1.W(), tmp1.W(), 1);  // string length in bytes for non-compressed case
    }
    GetMasm()->Add(tmp0_x, tmp0_x, tmp1.X());                   // calculate address of first byte after string
    GetMasm()->Sub(tmp0_x, tmp0_x, HALF_SIZE / BITS_PER_BYTE);  // don't scan last UTF-16 entry
    // amount of bytes to scan in worst case
    if (idx_is_zero) {
        GetMasm()->Neg(tmp2.X(), tmp1.X());
    } else {
        GetMasm()->Sub(tmp2.X(), tmp1.X(), vixl::aarch64::Operand(tmp2.X(), lsl, UTF16_IDX2OFFSET_SHIFT));
        GetMasm()->Neg(tmp2.X(), tmp2.X());
    }
    GetMasm()->Add(tmp2.X(), tmp2.X(), HALF_SIZE / BITS_PER_BYTE);
    GetMasm()->Cbz(tmp2.X(), label_not_found);
    GetMasm()->Sub(tmp1.W(), character_w, MIN_SUPPLEMENTARY_CODE_POINT);  // shifted immediate version
    GetMasm()->Mov(tmp3.W(), MIN_HIGH_SURROGATE);
    GetMasm()->Add(tmp1.W(), tmp3.W(), vixl::aarch64::Operand(tmp1.W(), lsr, SURROGATE_LOW_BITS));  // high surrogate
    // low surrogate calculation below
    GetMasm()->Movk(tmp1.X(), MIN_LOW_SURROGATE, HALF_SIZE);
    // copy lowest 10 bits into (low surrogate)'s lowest 10 bits
    GetMasm()->Bfm(tmp1.W(), character_w, HALF_SIZE, SURROGATE_LOW_BITS - 1);
    GetMasm()->Bind(label_sur_loop);
    GetMasm()->Ldr(tmp3.W(), vixl::aarch64::MemOperand(tmp0_x, tmp2.X()));
    GetMasm()->Cmp(tmp3.W(), tmp1.W());
    GetMasm()->B(label_match, vixl::aarch64::Condition::eq);
    GetMasm()->Adds(tmp2.X(), tmp2.X(), HALF_SIZE / BITS_PER_BYTE);
    GetMasm()->Cbnz(tmp2.X(), label_sur_loop);
    GetMasm()->B(label_not_found);
    GetMasm()->Bind(label_match);
    // string length in bytes is: tmp - str - DATA_OFFSET
    GetMasm()->Add(tmp2.X(), tmp2.X(), tmp0_x);
    GetMasm()->Sub(tmp2.X(), tmp2.X(), str_x);
    GetMasm()->Sub(tmp2.X(), tmp2.X(), DATA_OFFSET);
    GetMasm()->Lsr(tmp2.X(), tmp2.X(), UTF16_IDX2OFFSET_SHIFT);
    GetMasm()->B(label_found);
}

void Aarch64Encoder::EncodeStringIndexOfAfter(Reg dst, Reg str, Reg character, Reg idx, Reg tmp, bool COMPRESSION,
                                              uint32_t LENGTH_OFFSET, uint32_t DATA_OFFSET, int32_t CHAR_CONST_VALUE)
{
    // NullCheck must check str register before StringIndexOfAfter.
    // If str is zero register, execution mustn't go to this instruction.
    auto zero_reg_id = GetRegfile()->GetZeroReg().GetId();
    if (str.GetId() == zero_reg_id) {
        EncodeAbort();
        return;
    }

    auto zero = VixlReg(GetRegfile()->GetZeroReg(), DOUBLE_WORD_SIZE);
    ScopedTmpRegU64 tmp1_scoped(this);
    ScopedTmpRegU64 tmp2_scoped(this);
    ScopedTmpRegU64 tmp3_scoped(this);
    auto tmp1 = VixlReg(tmp1_scoped.GetReg());
    auto tmp2 = VixlReg(tmp2_scoped.GetReg());
    auto tmp3 = VixlReg(tmp3_scoped.GetReg());

    // vixl register aliases
    bool idx_is_zero = (idx.GetId() == zero_reg_id);

    /*  Pseudo code:
        if (idx < 0) idx = 0;

        if (idx >= length) {
            return -1;
        }

        if (!<character_is_utf16_surrogate_pair>) { // main case
            if (<string_is_utf16>) {
                <search char in utf-16 string>; // IndexOfHandleUtf16NormalCase
            } else { // 8-bit string case
                if (<character_is_utf16>) {
                    return -1;
                }
                <search 8-bit char in 8-bit string>; // IndexOfHandleLatin1
            }
        } else { // surrogate pair case
            if (!<string_is_utf16>) {
                return -1;
            }
            <per-character surrogate pair search>;  // IndexOfHandleSurrogateCase
        }
    */

    if (!idx_is_zero) {
        auto idx_w = VixlReg(idx).W();
        GetMasm()->Cmp(idx_w, zero.W());
        GetMasm()->Csel(tmp2.W(), idx_w, zero.W(), vixl::aarch64::Condition::gt);  // max(idx, 0)
    }

    GetMasm()->Ldr(tmp1.W(), vixl::aarch64::MemOperand(VixlReg(str).X(),
                                                       LENGTH_OFFSET));  // string length with potential compression bit
    GetMasm()->Cmp(idx_is_zero ? zero.W() : tmp2.W(),
                   COMPRESSION ? vixl::aarch64::Operand(tmp1.W(), vixl::aarch64::Shift::LSR, 1) : tmp1.W());

    auto label_not_found = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    GetMasm()->B(label_not_found, vixl::aarch64::Condition::ge);

    // check if character is larger than upper bound of UTF-16
    GetMasm()->Mov(tmp3.X(), MAX_SUPPLEMENTARY_CODE_POINT);
    GetMasm()->Cmp(VixlReg(character).X(), tmp3);
    GetMasm()->B(label_not_found, vixl::aarch64::Condition::gt);

    // memo: compression: 0 = compressed(i.e. 8 bits), 1 = uncompressed(i.e. utf16)
    EncodeStringIndexOfAfterMainCase(dst, str, character, idx, tmp, tmp1, tmp2, tmp3, COMPRESSION, DATA_OFFSET,
                                     CHAR_CONST_VALUE, label_not_found);

    // local constants
    constexpr int32_t RESULT_NOT_FOUND = -1;
    auto label_done = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->B(label_done);
    GetMasm()->Bind(label_not_found);
    GetMasm()->Mov(VixlReg(dst).W(), RESULT_NOT_FOUND);
    GetMasm()->Bind(label_done);
}

void Aarch64Encoder::HandleChar(int32_t ch, const vixl::aarch64::Register &tmp, vixl::aarch64::Label *label_not_found,
                                vixl::aarch64::Label *label_uncompressed_string)
{
    if (ch > MAX_8BIT_CHAR) {
        GetMasm()->Tbz(tmp.W(), 0,
                       label_not_found);  // no need to search 16-bit character in compressed string
    } else {
        GetMasm()->Tbnz(tmp.W(), 0,
                        label_uncompressed_string);  // go to utf16 case if string is uncompressed
    }
}

void Aarch64Encoder::EncodeStringIndexOfAfterMainCase(Reg dst, Reg str, Reg character, Reg idx, Reg tmp,
                                                      const vixl::aarch64::Register &tmp1,
                                                      const vixl::aarch64::Register &tmp2,
                                                      const vixl::aarch64::Register &tmp3, const bool COMPRESSION,
                                                      const uint32_t DATA_OFFSET, const int32_t CHAR_CONST_VALUE,
                                                      vixl::aarch64::Label *label_not_found)
{
    constexpr int32_t CHAR_CONST_UNKNOWN = -1;
    auto label_surrogate = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_found = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_uncompressed_string = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Add(VixlReg(tmp).X(), VixlReg(str).X(), DATA_OFFSET);

    if (CHAR_CONST_VALUE < MIN_SUPPLEMENTARY_CODE_POINT) {
        // case of non-surrogate constant char or non-constant char
        if (CHAR_CONST_VALUE == CHAR_CONST_UNKNOWN) {  // run time check for surrogate pair
            GetMasm()->Cmp(VixlReg(character).W(),
                           MIN_SUPPLEMENTARY_CODE_POINT);  // shifted immediate form of Cmp (i.e. 0x10 << 12)
            GetMasm()->B(label_surrogate, vixl::aarch64::Condition::ge);
        }
        if (COMPRESSION) {
            if (CHAR_CONST_VALUE != CHAR_CONST_UNKNOWN) {
                HandleChar(CHAR_CONST_VALUE, tmp1, label_not_found, label_uncompressed_string);
            } else {
                GetMasm()->Tbnz(tmp1.W(), 0, label_uncompressed_string);
                GetMasm()->Cmp(VixlReg(character).W(), MAX_8BIT_CHAR);
                GetMasm()->B(label_not_found,
                             vixl::aarch64::Condition::gt);  // do no search 16-bit char in compressed string
            }
            if (CHAR_CONST_VALUE <= MAX_8BIT_CHAR) {  // i.e. character is 8-bit constant or unknown
                IndexOfHandleLatin1Case(str, character, idx, tmp, COMPRESSION, DATA_OFFSET, tmp1, tmp2, tmp3,
                                        label_found, label_not_found);
            }
            GetMasm()->Bind(label_uncompressed_string);
        }
        IndexOfHandleUtf16NormalCase(str, character, idx, tmp, COMPRESSION, DATA_OFFSET, tmp1, tmp2, tmp3, label_found,
                                     label_not_found);
    }

    if (CHAR_CONST_VALUE >= MIN_SUPPLEMENTARY_CODE_POINT || CHAR_CONST_VALUE == CHAR_CONST_UNKNOWN) {
        GetMasm()->Bind(label_surrogate);
        if (COMPRESSION) {
            GetMasm()->Tbz(tmp1.W(), 0, label_not_found);  // no need to search 16-bit character in compressed string
        }
        IndexOfHandleSurrogateCase(str, character, idx, tmp, COMPRESSION, DATA_OFFSET, tmp1, tmp2, tmp3, label_found,
                                   label_not_found);
    }
    // various exit handling below
    GetMasm()->Bind(label_found);
    GetMasm()->Mov(VixlReg(dst).W(), tmp2.W());
}

/* return the power of 2 for the size of the type */
void Aarch64Encoder::EncodeGetTypeSize(Reg size, Reg type)
{
    auto sreg = VixlReg(type);
    auto dreg = VixlReg(size);
    constexpr uint8_t I16 = 0x5;
    constexpr uint8_t I32 = 0x7;
    constexpr uint8_t F64 = 0xa;
    constexpr uint8_t REF = 0xd;
    constexpr uint8_t SMALLREF = panda::OBJECT_POINTER_SIZE < sizeof(uint64_t) ? 1 : 0;
    auto end = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    GetMasm()->Mov(dreg, VixlImm(0));
    GetMasm()->Cmp(sreg, VixlImm(I16));
    GetMasm()->Cinc(dreg, dreg, vixl::aarch64::Condition::ge);
    GetMasm()->Cmp(sreg, VixlImm(I32));
    GetMasm()->Cinc(dreg, dreg, vixl::aarch64::Condition::ge);
    GetMasm()->Cmp(sreg, VixlImm(F64));
    GetMasm()->Cinc(dreg, dreg, vixl::aarch64::Condition::ge);
    GetMasm()->Cmp(sreg, VixlImm(REF));
    GetMasm()->B(end, vixl::aarch64::Condition::ne);
    GetMasm()->Sub(dreg, dreg, VixlImm(SMALLREF));
    GetMasm()->Bind(end);
}

void Aarch64Encoder::EncodeReverseBits(Reg dst, Reg src)
{
    auto rzero = GetRegfile()->GetZeroReg();
    if (rzero.GetId() == src.GetId()) {
        EncodeMov(dst, Imm(0));
        return;
    }
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    GetMasm()->Rbit(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeCompressedStringCharAt(Reg dst, Reg str, Reg idx, Reg length, Reg tmp, size_t data_offset,
                                                  uint32_t shift)
{
    ASSERT(dst.GetSize() == HALF_SIZE);

    auto label_not_compressed = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_char_loaded = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto vixl_tmp = VixlReg(tmp, DOUBLE_WORD_SIZE);
    auto vixl_dst = VixlReg(dst);

    GetMasm()->Tbnz(VixlReg(length), 0, label_not_compressed);
    EncodeAdd(tmp, str, idx);
    GetMasm()->ldrb(vixl_dst, MemOperand(vixl_tmp, data_offset));
    GetMasm()->B(label_char_loaded);
    GetMasm()->Bind(label_not_compressed);
    EncodeAdd(tmp, str, Shift(idx, shift));
    GetMasm()->ldrh(vixl_dst, MemOperand(vixl_tmp, data_offset));
    GetMasm()->Bind(label_char_loaded);
}

void Aarch64Encoder::EncodeCompressedStringCharAtI(Reg dst, Reg str, Reg length, size_t data_offset, uint32_t index,
                                                   uint32_t shift)
{
    ASSERT(dst.GetSize() == HALF_SIZE);

    auto label_not_compressed = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_char_loaded = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto vixl_str = VixlReg(str);
    auto vixl_dst = VixlReg(dst);

    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (str.GetId() == rzero) {
        return;
    }
    GetMasm()->Tbnz(VixlReg(length), 0, label_not_compressed);
    GetMasm()->Ldrb(vixl_dst, MemOperand(vixl_str, data_offset + index));
    GetMasm()->B(label_char_loaded);
    GetMasm()->Bind(label_not_compressed);
    GetMasm()->Ldrh(vixl_dst, MemOperand(vixl_str, data_offset + (index << shift)));
    GetMasm()->Bind(label_char_loaded);
}

/* Unsafe builtins implementation */
void Aarch64Encoder::EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval)
{
    /* Modeled according to the following logic:
      .L2:
      ldaxr   cur, [addr]
      cmp     cur, old
      bne     .L3
      stlxr   res, new, [addr]
      cbnz    res, .L2
      .L3:
      cset    w0, eq
    */
    ScopedTmpReg addr(this, true); /* LR is used */
    ScopedTmpReg cur(this, val.GetType());
    ScopedTmpReg res(this, val.GetType());
    auto loop = CreateLabel();
    auto exit = CreateLabel();

    /* ldaxr wants [reg]-form of memref (no offset or disp) */
    EncodeAdd(addr, obj, offset);

    BindLabel(loop);
    EncodeLdrExclusive(cur, addr, true);
    EncodeJump(exit, cur, val, Condition::NE);
    EncodeStrExclusive(res, newval, addr, true);
    EncodeJump(loop, res, Imm(0), Condition::NE);
    BindLabel(exit);

    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::eq);
}

void Aarch64Encoder::EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val)
{
    auto cur = ScopedTmpReg(this, val.GetType());
    auto last = ScopedTmpReg(this, val.GetType());
    auto addr = ScopedTmpReg(this, true); /* LR is used */
    auto mem = MemRef(addr);
    auto restart = CreateLabel();
    auto retry_ldaxr = CreateLabel();

    /* ldaxr wants [reg]-form of memref (no offset or disp) */
    EncodeAdd(addr, obj, offset);

    /* Since GetAndSet is defined as a non-faulting operation we
     * have to cover two possible faulty cases:
     *      1. stlxr failed, we have to retry ldxar
     *      2. the value we got via ldxar was not the value we initially
     *         loaded, we have to start from the very beginning */
    BindLabel(restart);
    EncodeLdrAcquire(last, false, mem);

    BindLabel(retry_ldaxr);
    EncodeLdrExclusive(cur, addr, true);
    EncodeJump(restart, cur, last, Condition::NE);
    EncodeStrExclusive(dst, val, addr, true);
    EncodeJump(retry_ldaxr, dst, Imm(0), Condition::NE);

    EncodeMov(dst, cur);
}

void Aarch64Encoder::EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp)
{
    ScopedTmpReg cur(this, val.GetType());
    ScopedTmpReg last(this, val.GetType());
    auto newval = Reg(tmp.GetId(), val.GetType());

    auto restart = CreateLabel();
    auto retry_ldaxr = CreateLabel();

    /* addr_reg aliases obj, obj reg will be restored bedore exit */
    auto addr = Reg(obj.GetId(), INT64_TYPE);

    /* ldaxr wants [reg]-form of memref (no offset or disp) */
    auto mem = MemRef(addr);
    EncodeAdd(addr, obj, offset);

    /* Since GetAndAdd is defined as a non-faulting operation we
     * have to cover two possible faulty cases:
     *      1. stlxr failed, we have to retry ldxar
     *      2. the value we got via ldxar was not the value we initially
     *         loaded, we have to start from the very beginning */
    BindLabel(restart);
    EncodeLdrAcquire(last, false, mem);
    EncodeAdd(newval, last, val);

    BindLabel(retry_ldaxr);
    EncodeLdrExclusive(cur, addr, true);
    EncodeJump(restart, cur, last, Condition::NE);
    EncodeStrExclusive(dst, newval, addr, true);
    EncodeJump(retry_ldaxr, dst, Imm(0), Condition::NE);

    EncodeSub(obj, addr, offset); /* restore the original value */
    EncodeMov(dst, cur);
}

void Aarch64Encoder::EncodeMemoryBarrier(MemoryOrder::Order order)
{
    switch (order) {
        case MemoryOrder::Acquire: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierReads);
            break;
        }
        case MemoryOrder::Release: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierWrites);
            break;
        }
        case MemoryOrder::Full: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierAll);
            break;
        }
        default:
            break;
    }
}

void Aarch64Encoder::EncodeNot(Reg dst, Reg src)
{
    GetMasm()->Mvn(VixlReg(dst), VixlReg(src));
}

void Aarch64Encoder::EncodeCastFloat(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    // We DON'T support casts from float32/64 to int8/16 and bool, because this caste is not declared anywhere
    // in other languages and architecture, we do not know what the behavior should be.
    // But there is one implementation in other function: "EncodeCastFloatWithSmallDst". Call it in the "EncodeCast"
    // function instead of "EncodeCastFloat". It works as follows: cast from float32/64 to int32, moving sign bit from
    // int32 to dst type, then extend number from dst type to int32 (a necessary condition for an isa). All work in dst
    // register.
    ASSERT(dst.GetSize() >= WORD_SIZE);

    if (src.IsFloat() && dst.IsScalar()) {
        if (dst_signed) {
            if (!IsJsNumberCast()) {
                GetMasm()->Fcvtzs(VixlReg(dst), VixlVReg(src));
            } else {
                CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
                vixl::CPUFeaturesScope scope(GetMasm(), vixl::CPUFeatures::kFP, vixl::CPUFeatures::kJSCVT);
                GetMasm()->Fjcvtzs(VixlReg(dst.As(INT32_TYPE)), VixlVReg(src));
            }
            return;
        }
        GetMasm()->Fcvtzu(VixlReg(dst), VixlVReg(src));
        return;
    }
    if (src.IsScalar() && dst.IsFloat()) {
        if (src_signed) {
            GetMasm()->Scvtf(VixlVReg(dst), VixlReg(src));
        } else {
            GetMasm()->Ucvtf(VixlVReg(dst), VixlReg(src));
        }
        return;
    }
    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            GetMasm()->Fcvt(VixlVReg(dst), VixlVReg(src));
            return;
        }
        GetMasm()->Fmov(VixlVReg(dst), VixlVReg(src));
        return;
    }
    UNREACHABLE();
}

void Aarch64Encoder::EncodeCastFloatWithSmallDst(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    // Dst bool type don't supported!

    if (src.IsFloat() && dst.IsScalar()) {
        if (dst_signed) {
            GetMasm()->Fcvtzs(VixlReg(dst), VixlVReg(src));
            if (dst.GetSize() < WORD_SIZE) {
                constexpr uint32_t TEST_BIT = (1U << (static_cast<uint32_t>(WORD_SIZE) - 1));
                ScopedTmpReg tmp_reg1(this, dst.GetType());
                auto tmp1 = VixlReg(tmp_reg1);
                ScopedTmpReg tmp_reg2(this, dst.GetType());
                auto tmp2 = VixlReg(tmp_reg2);

                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                int32_t set_bit = (dst.GetSize() == BYTE_SIZE) ? (1UL << (BYTE_SIZE - 1)) : (1UL << (HALF_SIZE - 1));
                int32_t rem_bit = set_bit - 1;
                GetMasm()->Ands(tmp1, VixlReg(dst), TEST_BIT);

                GetMasm()->Orr(tmp1, VixlReg(dst), set_bit);
                GetMasm()->And(tmp2, VixlReg(dst), rem_bit);
                // Select result - if zero set - tmp2, else tmp1
                GetMasm()->Csel(VixlReg(dst), tmp2, tmp1, vixl::aarch64::eq);
                EncodeCastScalar(Reg(dst.GetId(), INT32_TYPE), dst_signed, dst, dst_signed);
            }
            return;
        }
        GetMasm()->Fcvtzu(VixlReg(dst), VixlVReg(src));
        if (dst.GetSize() < WORD_SIZE) {
            EncodeCastScalar(Reg(dst.GetId(), INT32_TYPE), dst_signed, dst, dst_signed);
        }
        return;
    }
    if (src.IsScalar() && dst.IsFloat()) {
        if (src_signed) {
            GetMasm()->Scvtf(VixlVReg(dst), VixlReg(src));
        } else {
            GetMasm()->Ucvtf(VixlVReg(dst), VixlReg(src));
        }
        return;
    }
    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            GetMasm()->Fcvt(VixlVReg(dst), VixlVReg(src));
            return;
        }
        GetMasm()->Fmov(VixlVReg(dst), VixlVReg(src));
        return;
    }
    UNREACHABLE();
}

void Aarch64Encoder::EncodeCastSigned(Reg dst, Reg src)
{
    size_t src_size = src.GetSize();
    size_t dst_size = dst.GetSize();
    auto src_r = Reg(src.GetId(), dst.GetType());
    // Else signed extend
    if (src_size > dst_size) {
        src_size = dst_size;
    }
    switch (src_size) {
        case BYTE_SIZE:
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(src_r));
            break;
        case HALF_SIZE:
            GetMasm()->Sxth(VixlReg(dst), VixlReg(src_r));
            break;
        case WORD_SIZE:
            GetMasm()->Sxtw(VixlReg(dst), VixlReg(src_r));
            break;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src_r));
            break;
        default:
            SetFalseResult();
            break;
    }
}

void Aarch64Encoder::EncodeCastUnsigned(Reg dst, Reg src)
{
    size_t src_size = src.GetSize();
    size_t dst_size = dst.GetSize();
    auto src_r = Reg(src.GetId(), dst.GetType());
    if (src_size > dst_size && dst_size < WORD_SIZE) {
        // We need to cut the number, if it is less, than 32-bit. It is by ISA agreement.
        int64_t cut_value = (1ULL << dst_size) - 1;
        GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(cut_value));
        return;
    }
    // Else unsigned extend
    switch (src_size) {
        case BYTE_SIZE:
            GetMasm()->Uxtb(VixlReg(dst), VixlReg(src_r));
            return;
        case HALF_SIZE:
            GetMasm()->Uxth(VixlReg(dst), VixlReg(src_r));
            return;
        case WORD_SIZE:
            GetMasm()->Uxtw(VixlReg(dst), VixlReg(src_r));
            return;
        case DOUBLE_WORD_SIZE:
            GetMasm()->Mov(VixlReg(dst), VixlReg(src_r));
            return;
        default:
            SetFalseResult();
            return;
    }
}

void Aarch64Encoder::EncodeCastScalar(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    size_t src_size = src.GetSize();
    size_t dst_size = dst.GetSize();
    // In our ISA minimal type is 32-bit, so type less then 32-bit
    // we should extend to 32-bit. So we can have 2 cast
    // (For examble, i8->u16 will work as i8->u16 and u16->u32)
    if (dst_size < WORD_SIZE) {
        if (src_size > dst_size) {
            if (dst_signed) {
                EncodeCastSigned(dst, src);
            } else {
                EncodeCastUnsigned(dst, src);
            }
            return;
        }
        if (src_size == dst_size) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            if (!(src_signed || dst_signed) || (src_signed && dst_signed)) {
                return;
            }
            if (dst_signed) {
                EncodeCastSigned(Reg(dst.GetId(), INT32_TYPE), dst);
            } else {
                EncodeCastUnsigned(Reg(dst.GetId(), INT32_TYPE), dst);
            }
            return;
        }
        if (src_signed) {
            EncodeCastSigned(dst, src);
            if (!dst_signed) {
                EncodeCastUnsigned(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        } else {
            EncodeCastUnsigned(dst, src);
            if (dst_signed) {
                EncodeCastSigned(Reg(dst.GetId(), INT32_TYPE), dst);
            }
        }
    } else {
        if (src_size == dst_size) {
            GetMasm()->Mov(VixlReg(dst), VixlReg(src));
            return;
        }
        if (src_signed) {
            EncodeCastSigned(dst, src);
        } else {
            EncodeCastUnsigned(dst, src);
        }
    }
}

void Aarch64Encoder::EncodeCast(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    if (src.IsFloat() || dst.IsFloat()) {
        EncodeCastFloat(dst, dst_signed, src, src_signed);
        return;
    }

    ASSERT(src.IsScalar() && dst.IsScalar());
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (src.GetId() == rzero) {
        ASSERT(dst.GetId() != rzero);
        EncodeMov(dst, Imm(0));
        return;
    }
    // Scalar part
    EncodeCastScalar(dst, dst_signed, src, src_signed);
}

void Aarch64Encoder::EncodeCastToBool(Reg dst, Reg src)
{
    // In ISA says that we only support casts:
    // i32tou1, i64tou1, u32tou1, u64tou1
    ASSERT(src.IsScalar());
    ASSERT(dst.IsScalar());

    GetMasm()->Cmp(VixlReg(src), VixlImm(0));
    // In our ISA minimal type is 32-bit, so bool in 32bit
    GetMasm()->Cset(VixlReg(Reg(dst.GetId(), INT32_TYPE)), vixl::aarch64::Condition::ne);
}

void Aarch64Encoder::EncodeAdd(Reg dst, Reg src0, Shift src1)
{
    if (dst.IsFloat()) {
        UNREACHABLE();
    }
    ASSERT(src0.GetSize() <= dst.GetSize());
    if (src0.GetSize() < dst.GetSize()) {
        auto src0_reg = Reg(src0.GetId(), dst.GetType());
        auto src1_reg = Reg(src1.GetBase().GetId(), dst.GetType());
        GetMasm()->Add(VixlReg(dst), VixlReg(src0_reg), VixlShift(Shift(src1_reg, src1.GetType(), src1.GetScale())));
        return;
    }
    GetMasm()->Add(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeAdd(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fadd(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    /* if any of the operands has 64-bits size,
     * forcibly do the 64-bits wide operation */
    if ((src0.GetSize() | src1.GetSize() | dst.GetSize()) >= DOUBLE_WORD_SIZE) {
        GetMasm()->Add(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Add(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
}

void Aarch64Encoder::EncodeSub(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.IsScalar());
    GetMasm()->Sub(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeSub(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fsub(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    /* if any of the operands has 64-bits size,
     * forcibly do the 64-bits wide operation */
    if ((src0.GetSize() | src1.GetSize() | dst.GetSize()) >= DOUBLE_WORD_SIZE) {
        GetMasm()->Sub(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Sub(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
}

void Aarch64Encoder::EncodeMul(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fmul(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (src0.GetId() == rzero || src1.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    GetMasm()->Mul(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Adds(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Adds(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->Subs(VixlReg(dst).X(), VixlReg(src0).X(), VixlReg(src1).X());
    } else {
        /* Otherwise do 32-bits operation as any lesser
         * sizes have to be upcasted to 32-bits anyway */
        GetMasm()->Subs(VixlReg(dst).W(), VixlReg(src0).W(), VixlReg(src1).W());
    }
    auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->B(label, Convert(cc));
}

void Aarch64Encoder::EncodeDiv(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fdiv(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }

    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (src1.GetId() == rzero || src0.GetId() == rzero) {
        ScopedTmpReg tmp_reg(this, src1.GetType());
        EncodeMov(tmp_reg, Imm(0));
        // Denominator is zero-reg
        if (src1.GetId() == rzero) {
            // Encode Abort
            GetMasm()->Udiv(VixlReg(dst), VixlReg(tmp_reg), VixlReg(tmp_reg));
            return;
        }

        // But src1 still may be zero
        if (src1.GetId() != src0.GetId()) {
            if (dst_signed) {
                GetMasm()->Sdiv(VixlReg(dst), VixlReg(tmp_reg), VixlReg(src1));
            } else {
                GetMasm()->Udiv(VixlReg(dst), VixlReg(tmp_reg), VixlReg(src1));
            }
            return;
        }
        UNREACHABLE();
    }
    if (dst_signed) {
        GetMasm()->Sdiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Udiv(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    }
}

void Aarch64Encoder::EncodeMod(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        auto rzero = GetRegfile()->GetZeroReg().GetId();
        if (src1.GetId() == rzero || src0.GetId() == rzero) {
            ScopedTmpReg tmp_reg(this, src1.GetType());
            EncodeMov(tmp_reg, Imm(0));
            // Denominator is zero-reg
            if (src1.GetId() == rzero) {
                // Encode Abort
                GetMasm()->Udiv(VixlReg(dst), VixlReg(tmp_reg), VixlReg(tmp_reg));
                return;
            }

            if (src1.GetId() == src0.GetId()) {
                SetFalseResult();
                return;
            }
            // But src1 still may be zero
            ScopedTmpRegU64 tmp_reg_ud(this);
            if (dst.GetSize() < DOUBLE_WORD_SIZE) {
                tmp_reg_ud.ChangeType(INT32_TYPE);
            }
            auto tmp = VixlReg(tmp_reg_ud);
            if (!dst_signed) {
                GetMasm()->Udiv(tmp, VixlReg(tmp_reg), VixlReg(src1));
                GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(tmp_reg));
                return;
            }
            GetMasm()->Sdiv(tmp, VixlReg(tmp_reg), VixlReg(src1));
            GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(tmp_reg));
            return;
        }

        ScopedTmpRegU64 tmp_reg(this);
        if (dst.GetSize() < DOUBLE_WORD_SIZE) {
            tmp_reg.ChangeType(INT32_TYPE);
        }
        auto tmp = VixlReg(tmp_reg);

        if (!dst_signed) {
            GetMasm()->Udiv(tmp, VixlReg(src0), VixlReg(src1));
            GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(src0));
            return;
        }
        GetMasm()->Sdiv(tmp, VixlReg(src0), VixlReg(src1));
        GetMasm()->Msub(VixlReg(dst), tmp, VixlReg(src1), VixlReg(src0));
        return;
    }

    EncodeFMod(dst, src0, src1);
}

void Aarch64Encoder::EncodeFMod(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsFloat());

    if (dst.GetType() == FLOAT32_TYPE) {
        using fp = float (*)(float, float);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<fp>(fmodf)));
    } else {
        using fp = double (*)(double, double);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<fp>(fmod)));
    }
}

void Aarch64Encoder::EncodeMin(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fmin(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }
    if (dst_signed) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::lt);
        return;
    }
    GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::ls);
}

void Aarch64Encoder::EncodeMax(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        GetMasm()->Fmax(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
        return;
    }
    if (dst_signed) {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::gt);
        return;
    }
    GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), vixl::aarch64::Condition::hi);
}

void Aarch64Encoder::EncodeShl(Reg dst, Reg src0, Reg src1)
{
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src0.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    if (src1.GetId() == rzero) {
        EncodeMov(dst, src0);
    }
    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }
    GetMasm()->Lsl(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeShr(Reg dst, Reg src0, Reg src1)
{
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src0.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    if (src1.GetId() == rzero) {
        EncodeMov(dst, src0);
    }

    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }

    GetMasm()->Lsr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAShr(Reg dst, Reg src0, Reg src1)
{
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src0.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }
    if (src1.GetId() == rzero) {
        EncodeMov(dst, src0);
    }

    if (dst.GetSize() < WORD_SIZE) {
        GetMasm()->And(VixlReg(src1), VixlReg(src1), VixlImm(dst.GetSize() - 1));
    }
    GetMasm()->Asr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAnd(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->And(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAnd(Reg dst, Reg src0, Shift src1)
{
    GetMasm()->And(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeOr(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Orr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeOr(Reg dst, Reg src0, Shift src1)
{
    GetMasm()->Orr(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeXor(Reg dst, Reg src0, Reg src1)
{
    GetMasm()->Eor(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeXor(Reg dst, Reg src0, Shift src1)
{
    GetMasm()->Eor(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeAdd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    ASSERT(dst.GetSize() >= src.GetSize());
    if (dst.GetSize() != src.GetSize()) {
        auto src_reg = Reg(src.GetId(), dst.GetType());
        GetMasm()->Add(VixlReg(dst), VixlReg(src_reg), VixlImm(imm));
        return;
    }
    GetMasm()->Add(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeSub(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "UNIMPLEMENTED");
    GetMasm()->Sub(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeShl(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }

    GetMasm()->Lsl(VixlReg(dst), VixlReg(src), GetIntValue(imm));
}

void Aarch64Encoder::EncodeShr(Reg dst, Reg src, Imm imm)
{
    int64_t imm_value = static_cast<uint64_t>(GetIntValue(imm)) & (dst.GetSize() - 1);

    ASSERT(dst.IsScalar() && "Invalid operand type");
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    ASSERT(dst.GetId() != rzero);
    if (src.GetId() == rzero) {
        EncodeMov(dst, Imm(0));
        return;
    }

    GetMasm()->Lsr(VixlReg(dst), VixlReg(src), imm_value);
}

void Aarch64Encoder::EncodeAShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Asr(VixlReg(dst), VixlReg(src), GetIntValue(imm));
}

void Aarch64Encoder::EncodeAnd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->And(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeOr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Orr(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeXor(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar() && "Invalid operand type");
    GetMasm()->Eor(VixlReg(dst), VixlReg(src), VixlImm(imm));
}

void Aarch64Encoder::EncodeMov(Reg dst, Imm src)
{
    if (dst.IsFloat()) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Fmov(VixlVReg(dst), src.GetValue<float>());
        } else {
            GetMasm()->Fmov(VixlVReg(dst), src.GetValue<double>());
        }
        return;
    }
    GetMasm()->Mov(VixlReg(dst), VixlImm(src));
}

void Aarch64Encoder::EncodeLdr(Reg dst, bool dst_signed, MemRef mem)
{
    auto rzero = GetRegfile()->GetZeroReg().GetId();

    if (!ConvertMem(mem).IsValid() || (dst.GetId() == rzero && dst.IsScalar())) {
        // Try move zero reg to dst (for do not create temp-reg)
        // Check: dst not vector, dst not index, dst not rzero
        [[maybe_unused]] auto base_reg = mem.GetBase();
        auto index_reg = mem.GetIndex();

        // Invalid == base is rzero or invalid
        ASSERT(base_reg.GetId() == rzero || !base_reg.IsValid());
        // checks for use dst-register
        if (dst.IsScalar() && dst.IsValid() &&     // not float
            (index_reg.GetId() != dst.GetId()) &&  // not index
            (dst.GetId() != rzero)) {              // not rzero
            // May use dst like rzero
            EncodeMov(dst, Imm(0));

            auto fix_mem = MemRef(dst, index_reg, mem.GetScale(), mem.GetDisp());
            ASSERT(ConvertMem(fix_mem).IsValid());
            EncodeLdr(dst, dst_signed, fix_mem);
        } else {
            // Use tmp-reg
            ScopedTmpReg tmp_reg(this);
            EncodeMov(tmp_reg, Imm(0));

            auto fix_mem = MemRef(tmp_reg, index_reg, mem.GetScale(), mem.GetDisp());
            ASSERT(ConvertMem(fix_mem).IsValid());
            // Used for zero-dst
            EncodeLdr(tmp_reg, dst_signed, fix_mem);
        }
        return;
    }
    ASSERT(ConvertMem(mem).IsValid());
    if (dst.IsFloat()) {
        GetMasm()->Ldr(VixlVReg(dst), ConvertMem(mem));
        return;
    }
    if (dst_signed) {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldrsb(VixlReg(dst, DOUBLE_WORD_SIZE), ConvertMem(mem));
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldrsh(VixlReg(dst), ConvertMem(mem));
            return;
        }
    } else {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldrb(VixlReg(dst, WORD_SIZE), ConvertMem(mem));
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldrh(VixlReg(dst), ConvertMem(mem));
            return;
        }
    }
    GetMasm()->Ldr(VixlReg(dst), ConvertMem(mem));
}

void Aarch64Encoder::EncodeLdrAcquireInvalid(Reg dst, bool dst_signed, MemRef mem)
{
    // Try move zero reg to dst (for do not create temp-reg)
    // Check: dst not vector, dst not index, dst not rzero
    [[maybe_unused]] auto base_reg = mem.GetBase();
    auto rzero = GetRegfile()->GetZeroReg().GetId();

    auto index_reg = mem.GetIndex();

    // Invalid == base is rzero or invalid
    ASSERT(base_reg.GetId() == rzero || !base_reg.IsValid());
    // checks for use dst-register
    if (dst.IsScalar() && dst.IsValid() &&     // not float
        (index_reg.GetId() != dst.GetId()) &&  // not index
        (dst.GetId() != rzero)) {              // not rzero
        // May use dst like rzero
        EncodeMov(dst, Imm(0));

        auto fix_mem = MemRef(dst, index_reg, mem.GetScale(), mem.GetDisp());
        ASSERT(ConvertMem(fix_mem).IsValid());
        EncodeLdrAcquire(dst, dst_signed, fix_mem);
    } else {
        // Use tmp-reg
        ScopedTmpReg tmp_reg(this);
        EncodeMov(tmp_reg, Imm(0));

        auto fix_mem = MemRef(tmp_reg, index_reg, mem.GetScale(), mem.GetDisp());
        ASSERT(ConvertMem(fix_mem).IsValid());
        // Used for zero-dst
        EncodeLdrAcquire(tmp_reg, dst_signed, fix_mem);
    }
}

void Aarch64Encoder::EncodeLdrAcquireScalar(Reg dst, bool dst_signed, MemRef mem)
{
#ifndef NDEBUG
    CheckAlignment(mem, dst.GetSize());
#endif  // NDEBUG
    if (dst_signed) {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldarb(VixlReg(dst), ConvertMem(mem));
            GetMasm()->Sxtb(VixlReg(dst), VixlReg(dst));
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldarh(VixlReg(dst), ConvertMem(mem));
            GetMasm()->Sxth(VixlReg(dst), VixlReg(dst));
            return;
        }
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->Ldar(VixlReg(dst), ConvertMem(mem));
            GetMasm()->Sxtw(VixlReg(dst), VixlReg(dst));
            return;
        }
    } else {
        if (dst.GetSize() == BYTE_SIZE) {
            GetMasm()->Ldarb(VixlReg(dst, WORD_SIZE), ConvertMem(mem));
            return;
        }
        if (dst.GetSize() == HALF_SIZE) {
            GetMasm()->Ldarh(VixlReg(dst), ConvertMem(mem));
            return;
        }
    }
    GetMasm()->Ldar(VixlReg(dst), ConvertMem(mem));
}

void Aarch64Encoder::CheckAlignment(MemRef mem, size_t size)
{
    ASSERT(size == WORD_SIZE || size == BYTE_SIZE || size == HALF_SIZE || size == DOUBLE_WORD_SIZE);
    if (size == BYTE_SIZE) {
        return;
    }
    size_t alignment_mask = (size >> 3U) - 1;
    ASSERT(!mem.HasIndex() && !mem.HasScale());
    if (mem.HasDisp()) {
        // We need additional tmp register for check base + offset.
        // The case when separately the base and the offset are not aligned, but in sum there are aligned very rarely.
        // Therefore, the alignment check for base and offset takes place separately
        [[maybe_unused]] size_t offset = mem.GetDisp();
        ASSERT((offset & alignment_mask) == 0);
    }
    auto base_reg = mem.GetBase();
    auto end = CreateLabel();
    EncodeJumpTest(end, base_reg, Imm(alignment_mask), Condition::TST_EQ);
    EncodeAbort();
    BindLabel(end);
}

void Aarch64Encoder::EncodeLdrAcquire(Reg dst, bool dst_signed, MemRef mem)
{
    ASSERT(!mem.HasIndex() && !mem.HasScale());
    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (!ConvertMem(mem).IsValid() || (dst.GetId() == rzero && dst.IsScalar())) {
        EncodeLdrAcquireInvalid(dst, dst_signed, mem);
        return;
    }

    if (dst.IsFloat()) {
        ScopedTmpRegU64 tmp_reg(this);
        auto mem_ldar = mem;
        if (mem.HasDisp()) {
            if (vixl::aarch64::Assembler::IsImmAddSub(mem.GetDisp())) {
                EncodeAdd(tmp_reg, mem.GetBase(), Imm(mem.GetDisp()));
            } else {
                EncodeMov(tmp_reg, Imm(mem.GetDisp()));
                EncodeAdd(tmp_reg, mem.GetBase(), tmp_reg);
            }
            mem_ldar = MemRef(tmp_reg);
        }
#ifndef NDEBUG
        CheckAlignment(mem_ldar, dst.GetSize());
#endif  // NDEBUG
        auto tmp = VixlReg(tmp_reg, dst.GetSize());
        GetMasm()->Ldar(tmp, ConvertMem(mem_ldar));
        GetMasm()->Fmov(VixlVReg(dst), tmp);
        return;
    }

    if (!mem.HasDisp()) {
        EncodeLdrAcquireScalar(dst, dst_signed, mem);
        return;
    }

    Reg dst_64(dst.GetId(), INT64_TYPE);
    if (vixl::aarch64::Assembler::IsImmAddSub(mem.GetDisp())) {
        EncodeAdd(dst_64, mem.GetBase(), Imm(mem.GetDisp()));
    } else {
        EncodeMov(dst_64, Imm(mem.GetDisp()));
        EncodeAdd(dst_64, mem.GetBase(), dst_64);
    }
    EncodeLdrAcquireScalar(dst, dst_signed, MemRef(dst_64));
}

void Aarch64Encoder::EncodeStr(Reg src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        auto index_reg = mem.GetIndex();
        auto rzero = GetRegfile()->GetZeroReg().GetId();
        // Invalid == base is rzero or invalid
        ASSERT(mem.GetBase().GetId() == rzero || !mem.GetBase().IsValid());
        // Use tmp-reg
        ScopedTmpReg tmp_reg(this);
        EncodeMov(tmp_reg, Imm(0));

        auto fix_mem = MemRef(tmp_reg, index_reg, mem.GetScale(), mem.GetDisp());
        ASSERT(ConvertMem(fix_mem).IsValid());
        if (src.GetId() != rzero) {
            EncodeStr(src, fix_mem);
        } else {
            EncodeStr(tmp_reg, fix_mem);
        }
        return;
    }
    ASSERT(ConvertMem(mem).IsValid());
    if (src.IsFloat()) {
        GetMasm()->Str(VixlVReg(src), ConvertMem(mem));
        return;
    }
    if (src.GetSize() == BYTE_SIZE) {
        GetMasm()->Strb(VixlReg(src), ConvertMem(mem));
        return;
    }
    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Strh(VixlReg(src), ConvertMem(mem));
        return;
    }
    GetMasm()->Str(VixlReg(src), ConvertMem(mem));
}

void Aarch64Encoder::EncodeStrRelease(Reg src, MemRef mem)
{
    ASSERT(!mem.HasScale());

    ScopedTmpRegLazy base(this);
    MemRef fixed_mem;
    bool mem_was_fixed = false;
    if (mem.HasDisp()) {
        if (vixl::aarch64::Assembler::IsImmAddSub(mem.GetDisp())) {
            base.AcquireIfInvalid();
            EncodeAdd(base, mem.GetBase(), Imm(mem.GetDisp()));
        } else {
            base.AcquireIfInvalid();
            EncodeMov(base, Imm(mem.GetDisp()));
            EncodeAdd(base, mem.GetBase(), base);
        }
        mem_was_fixed = true;
    }
    if (mem.HasIndex()) {
        base.AcquireIfInvalid();
        EncodeAdd(base, mem_was_fixed ? base : mem.GetBase(), mem.GetIndex());
        mem_was_fixed = true;
    }

    if (mem_was_fixed) {
        fixed_mem = MemRef(base);
    } else {
        fixed_mem = mem;
    }

#ifndef NDEBUG
    CheckAlignment(mem, src.GetSize());
#endif  // NDEBUG
    if (src.IsFloat()) {
        ScopedTmpRegU64 tmp_reg(this);
        auto tmp = VixlReg(tmp_reg, src.GetSize());
        GetMasm()->Fmov(tmp, VixlVReg(src));
        GetMasm()->Stlr(tmp, ConvertMem(fixed_mem));
        return;
    }
    if (src.GetSize() == BYTE_SIZE) {
        GetMasm()->Stlrb(VixlReg(src), ConvertMem(fixed_mem));
        return;
    }
    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Stlrh(VixlReg(src), ConvertMem(fixed_mem));
        return;
    }
    GetMasm()->Stlr(VixlReg(src), ConvertMem(fixed_mem));
}

void Aarch64Encoder::EncodeLdrExclusive(Reg dst, Reg addr, bool acquire)
{
    ASSERT(dst.IsScalar());
    auto dst_reg = VixlReg(dst);
    auto mem_cvt = ConvertMem(MemRef(addr));
#ifndef NDEBUG
    CheckAlignment(MemRef(addr), dst.GetSize());
#endif  // NDEBUG
    if (dst.GetSize() == BYTE_SIZE) {
        if (acquire) {
            GetMasm()->Ldaxrb(dst_reg, mem_cvt);
            return;
        }
        GetMasm()->Ldxrb(dst_reg, mem_cvt);
        return;
    }
    if (dst.GetSize() == HALF_SIZE) {
        if (acquire) {
            GetMasm()->Ldaxrh(dst_reg, mem_cvt);
            return;
        }
        GetMasm()->Ldxrh(dst_reg, mem_cvt);
        return;
    }
    if (acquire) {
        GetMasm()->Ldaxr(dst_reg, mem_cvt);
        return;
    }
    GetMasm()->Ldxr(dst_reg, mem_cvt);
}

void Aarch64Encoder::EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release)
{
    ASSERT(dst.IsScalar() && src.IsScalar());

    bool copy_dst = dst.GetId() == src.GetId() || dst.GetId() == addr.GetId();
    ScopedTmpReg tmp(this);
    auto src_reg = VixlReg(src);
    auto mem_cvt = ConvertMem(MemRef(addr));
    auto dst_reg = copy_dst ? VixlReg(tmp) : VixlReg(dst);
#ifndef NDEBUG
    CheckAlignment(MemRef(addr), src.GetSize());
#endif  // NDEBUG

    if (src.GetSize() == BYTE_SIZE) {
        if (release) {
            GetMasm()->Stlxrb(dst_reg, src_reg, mem_cvt);
        } else {
            GetMasm()->Stxrb(dst_reg, src_reg, mem_cvt);
        }
    } else if (src.GetSize() == HALF_SIZE) {
        if (release) {
            GetMasm()->Stlxrh(dst_reg, src_reg, mem_cvt);
        } else {
            GetMasm()->Stxrh(dst_reg, src_reg, mem_cvt);
        }
    } else {
        if (release) {
            GetMasm()->Stlxr(dst_reg, src_reg, mem_cvt);
        } else {
            GetMasm()->Stxr(dst_reg, src_reg, mem_cvt);
        }
    }
    if (copy_dst) {
        EncodeMov(dst, tmp);
    }
}

void Aarch64Encoder::EncodeStrz(Reg src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        EncodeStr(src, mem);
        return;
    }
    ASSERT(ConvertMem(mem).IsValid());
    // Upper half of registers must be zeroed by-default
    if (src.IsFloat()) {
        EncodeStr(src.As(FLOAT64_TYPE), mem);
        return;
    }
    if (src.GetSize() < WORD_SIZE) {
        EncodeCast(src, false, src.As(INT64_TYPE), false);
    }
    GetMasm()->Str(VixlReg(src.As(INT64_TYPE)), ConvertMem(mem));
}

void Aarch64Encoder::EncodeSti(Imm src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        EncodeStr(rzero, mem);
        return;
    }

    if (src.GetType().IsFloat()) {
        if (src.GetSize() == WORD_SIZE) {
            ScopedTmpRegF32 tmp_reg(this);
            GetMasm()->Fmov(VixlVReg(tmp_reg).S(), src.GetValue<float>());
            EncodeStr(tmp_reg, mem);
        } else {
            ScopedTmpRegF64 tmp_reg(this);
            GetMasm()->Fmov(VixlVReg(tmp_reg).D(), src.GetValue<double>());
            EncodeStr(tmp_reg, mem);
        }
        return;
    }

    ScopedTmpRegU64 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg);
    GetMasm()->Mov(tmp, VixlImm(src));
    if (src.GetSize() == BYTE_SIZE) {
        GetMasm()->Strb(tmp, ConvertMem(mem));
        return;
    }
    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->Strh(tmp, ConvertMem(mem));
        return;
    }
    GetMasm()->Str(tmp, ConvertMem(mem));
}

void Aarch64Encoder::EncodeMemCopy(MemRef mem_from, MemRef mem_to, size_t size)
{
    if (!ConvertMem(mem_from).IsValid() || !ConvertMem(mem_to).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        if (!ConvertMem(mem_from).IsValid()) {
            // Encode one load - will fix inside
            EncodeLdr(rzero, false, mem_from);
        } else {
            ASSERT(!ConvertMem(mem_to).IsValid());
            // Encode one store - will fix inside
            EncodeStr(rzero, mem_to);
        }
        return;
    }
    ASSERT(ConvertMem(mem_from).IsValid());
    ASSERT(ConvertMem(mem_to).IsValid());
    ScopedTmpRegU64 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg, std::min(size, static_cast<size_t>(DOUBLE_WORD_SIZE)));
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmp, ConvertMem(mem_from));
        GetMasm()->Strb(tmp, ConvertMem(mem_to));
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmp, ConvertMem(mem_from));
        GetMasm()->Strh(tmp, ConvertMem(mem_to));
    } else {
        ASSERT(size == WORD_SIZE || size == DOUBLE_WORD_SIZE);
        GetMasm()->Ldr(tmp, ConvertMem(mem_from));
        GetMasm()->Str(tmp, ConvertMem(mem_to));
    }
}

void Aarch64Encoder::EncodeMemCopyz(MemRef mem_from, MemRef mem_to, size_t size)
{
    if (!ConvertMem(mem_from).IsValid() || !ConvertMem(mem_to).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        if (!ConvertMem(mem_from).IsValid()) {
            // Encode one load - will fix inside
            EncodeLdr(rzero, false, mem_from);
        } else {
            ASSERT(!ConvertMem(mem_to).IsValid());
            // Encode one store - will fix inside
            EncodeStr(rzero, mem_to);
        }
        return;
    }
    ASSERT(ConvertMem(mem_from).IsValid());
    ASSERT(ConvertMem(mem_to).IsValid());
    ScopedTmpRegU64 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg, std::min(size, static_cast<size_t>(DOUBLE_WORD_SIZE)));
    auto zero = VixlReg(GetRegfile()->GetZeroReg(), WORD_SIZE);
    if (size == BYTE_SIZE) {
        GetMasm()->Ldrb(tmp, ConvertMem(mem_from));
        GetMasm()->Stp(tmp, zero, ConvertMem(mem_to));
    } else if (size == HALF_SIZE) {
        GetMasm()->Ldrh(tmp, ConvertMem(mem_from));
        GetMasm()->Stp(tmp, zero, ConvertMem(mem_to));
    } else {
        ASSERT(size == WORD_SIZE || size == DOUBLE_WORD_SIZE);
        GetMasm()->Ldr(tmp, ConvertMem(mem_from));
        if (size == WORD_SIZE) {
            GetMasm()->Stp(tmp, zero, ConvertMem(mem_to));
        } else {
            GetMasm()->Str(tmp, ConvertMem(mem_to));
        }
    }
}

void Aarch64Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    if (src0.IsFloat()) {
        GetMasm()->Fcmp(VixlVReg(src0), VixlVReg(src1));
    } else {
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    }
    GetMasm()->Cset(VixlReg(dst), Convert(cc));
}

void Aarch64Encoder::EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar() && src1.IsScalar());

    GetMasm()->Tst(VixlReg(src0), VixlReg(src1));
    GetMasm()->Cset(VixlReg(dst), ConvertTest(cc));
}

void Aarch64Encoder::EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsFloat()) {
        ASSERT(src1.IsFloat());
        ASSERT(cc == Condition::MI || cc == Condition::LT);
        GetMasm()->Fcmp(VixlVReg(src0), VixlVReg(src1));
    } else {
        ASSERT(src0.IsScalar() && src1.IsScalar());
        ASSERT(cc == Condition::LO || cc == Condition::LT);
        GetMasm()->Cmp(VixlReg(src0), VixlReg(src1));
    }
    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::ne);
    GetMasm()->Cneg(VixlReg(Promote(dst)), VixlReg(Promote(dst)), Convert(cc));
}

void Aarch64Encoder::EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat());
    if (src2.IsScalar()) {
        GetMasm()->Cmp(VixlReg(src2), VixlReg(src3));
    } else {
        GetMasm()->Fcmp(VixlVReg(src2), VixlVReg(src3));
    }
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), Convert(cc));
}

void Aarch64Encoder::EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());
    GetMasm()->Cmp(VixlReg(src2), VixlImm(imm));
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), Convert(cc));
}

void Aarch64Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat() && !src3.IsFloat());
    GetMasm()->Tst(VixlReg(src2), VixlReg(src3));
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), ConvertTest(cc));
}

void Aarch64Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());
    ASSERT(CanEncodeImmLogical(GetIntValue(imm), imm.GetSize() > WORD_SIZE ? DOUBLE_WORD_SIZE : WORD_SIZE));
    GetMasm()->Tst(VixlReg(src2), VixlImm(imm));
    GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), ConvertTest(cc));
}

void Aarch64Encoder::EncodeLdp(Reg dst0, Reg dst1, bool dst_signed, MemRef mem)
{
    ASSERT(dst0.IsFloat() == dst1.IsFloat());
    ASSERT(dst0.GetSize() == dst1.GetSize());
    if (!ConvertMem(mem).IsValid()) {
        // Encode one Ldr - will fix inside
        EncodeLdr(dst0, dst_signed, mem);
        return;
    }

    if (dst0.IsFloat()) {
        GetMasm()->Ldp(VixlVReg(dst0), VixlVReg(dst1), ConvertMem(mem));
        return;
    }
    if (dst_signed && dst0.GetSize() == WORD_SIZE) {
        GetMasm()->Ldpsw(VixlReg(dst0, DOUBLE_WORD_SIZE), VixlReg(dst1, DOUBLE_WORD_SIZE), ConvertMem(mem));
        return;
    }
    GetMasm()->Ldp(VixlReg(dst0), VixlReg(dst1), ConvertMem(mem));
}

void Aarch64Encoder::EncodeStp(Reg src0, Reg src1, MemRef mem)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    ASSERT(src0.GetSize() == src1.GetSize());
    if (!ConvertMem(mem).IsValid()) {
        // Encode one Str - will fix inside
        EncodeStr(src0, mem);
        return;
    }

    if (src0.IsFloat()) {
        GetMasm()->Stp(VixlVReg(src0), VixlVReg(src1), ConvertMem(mem));
        return;
    }
    GetMasm()->Stp(VixlReg(src0), VixlReg(src1), ConvertMem(mem));
}

void Aarch64Encoder::EncodeMAdd(Reg dst, Reg src0, Reg src1, Reg src2)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize() && dst.GetSize() == src2.GetSize());
    ASSERT(dst.IsScalar() == src0.IsScalar() && dst.IsScalar() == src1.IsScalar() && dst.IsScalar() == src2.IsScalar());

    ASSERT(!GetRegfile()->IsZeroReg(dst));

    if (GetRegfile()->IsZeroReg(src0) || GetRegfile()->IsZeroReg(src1)) {
        EncodeMov(dst, src2);
        return;
    }

    if (GetRegfile()->IsZeroReg(src2)) {
        EncodeMul(dst, src0, src1);
        return;
    }

    if (dst.IsScalar()) {
        GetMasm()->Madd(VixlReg(dst), VixlReg(src0), VixlReg(src1), VixlReg(src2));
    } else {
        GetMasm()->Fmadd(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), VixlVReg(src2));
    }
}

void Aarch64Encoder::EncodeMSub(Reg dst, Reg src0, Reg src1, Reg src2)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize() && dst.GetSize() == src2.GetSize());
    ASSERT(dst.IsScalar() == src0.IsScalar() && dst.IsScalar() == src1.IsScalar() && dst.IsScalar() == src2.IsScalar());

    ASSERT(!GetRegfile()->IsZeroReg(dst));

    if (GetRegfile()->IsZeroReg(src0) || GetRegfile()->IsZeroReg(src1)) {
        EncodeMov(dst, src2);
        return;
    }

    if (GetRegfile()->IsZeroReg(src2)) {
        EncodeMNeg(dst, src0, src1);
        return;
    }

    if (dst.IsScalar()) {
        GetMasm()->Msub(VixlReg(dst), VixlReg(src0), VixlReg(src1), VixlReg(src2));
    } else {
        GetMasm()->Fmsub(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), VixlVReg(src2));
    }
}

void Aarch64Encoder::EncodeMNeg(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() == src0.IsScalar() && dst.IsScalar() == src1.IsScalar());

    ASSERT(!GetRegfile()->IsZeroReg(dst));

    if (GetRegfile()->IsZeroReg(src0) || GetRegfile()->IsZeroReg(src1)) {
        EncodeMov(dst, Imm(0U));
        return;
    }

    if (dst.IsScalar()) {
        GetMasm()->Mneg(VixlReg(dst), VixlReg(src0), VixlReg(src1));
    } else {
        GetMasm()->Fnmul(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1));
    }
}

void Aarch64Encoder::EncodeOrNot(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.IsScalar());
    GetMasm()->Orn(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeOrNot(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.GetSize() == src0.GetSize() && dst.GetSize() == src1.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.GetBase().IsScalar());
    GetMasm()->Orn(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeExtractBits(Reg dst, Reg src0, Imm imm1, Imm imm2)
{
    GetMasm()->Ubfx(VixlReg(dst), VixlReg(src0), GetIntValue(imm1), GetIntValue(imm2));
}

void Aarch64Encoder::EncodeAndNot(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.IsScalar());
    GetMasm()->Bic(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeAndNot(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.GetSize() == src0.GetSize() && dst.GetSize() == src1.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.GetBase().IsScalar());
    GetMasm()->Bic(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeXorNot(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.GetSize() == src1.GetSize() && dst.GetSize() == src0.GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.IsScalar());
    GetMasm()->Eon(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}

void Aarch64Encoder::EncodeXorNot(Reg dst, Reg src0, Shift src1)
{
    ASSERT(dst.GetSize() == src0.GetSize() && dst.GetSize() == src1.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src0.IsScalar() && src1.GetBase().IsScalar());
    GetMasm()->Eon(VixlReg(dst), VixlReg(src0), VixlShift(src1));
}

void Aarch64Encoder::EncodeNeg(Reg dst, Shift src)
{
    ASSERT(dst.GetSize() == src.GetBase().GetSize());
    ASSERT(dst.IsScalar() && src.GetBase().IsScalar());
    GetMasm()->Neg(VixlReg(dst), VixlShift(src));
}

void Aarch64Encoder::EncodeStackOverflowCheck(ssize_t offset)
{
    ScopedTmpReg tmp(this);
    EncodeAdd(tmp, GetTarget().GetStackReg(), Imm(offset));
    EncodeLdr(tmp, false, MemRef(tmp));
}

bool Aarch64Encoder::CanEncodeImmAddSubCmp(int64_t imm, [[maybe_unused]] uint32_t size,
                                           [[maybe_unused]] bool signed_compare)
{
    if (imm == INT64_MIN) {
        return false;
    }
    if (imm < 0) {
        imm = -imm;
    }
    return vixl::aarch64::Assembler::IsImmAddSub(imm);
}

bool Aarch64Encoder::CanEncodeImmLogical(uint64_t imm, uint32_t size)
{
    return vixl::aarch64::Assembler::IsImmLogical(imm, size);
}

/*
 * From aarch64 instruction set
 *
 * ========================================================
 * Syntax
 *
 * LDR  Wt, [Xn|SP, Rm{, extend {amount}}]    ; 32-bit general registers
 *
 * LDR  Xt, [Xn|SP, Rm{, extend {amount}}]    ; 64-bit general registers
 *
 * amount
 * Is the index shift amount, optional and defaulting to #0 when extend is not LSL:
 *
 * 32-bit general registers
 * Can be one of #0 or #2.
 *
 * 64-bit general registers
 * Can be one of #0 or #3.
 * ========================================================
 * Syntax
 *
 * LDRH  Wt, [Xn|SP, Rm{, extend {amount}}]
 *
 * amount
 * Is the index shift amount, optional and defaulting to #0 when extend is not LSL, and can be either #0 or #1.
 * ========================================================
 *
 * Scale can be 0 or 1 for half load, 2 for word load, 3 for double word load
 */
bool Aarch64Encoder::CanEncodeScale(uint64_t imm, uint32_t size)
{
    return (imm == 0) || ((1U << imm) == (size >> 3U));
}

bool Aarch64Encoder::CanEncodeShiftedOperand(ShiftOpcode opcode, ShiftType shift_type)
{
    switch (opcode) {
        case ShiftOpcode::NEG_SR:
        case ShiftOpcode::ADD_SR:
        case ShiftOpcode::SUB_SR:
            return shift_type == ShiftType::LSL || shift_type == ShiftType::LSR || shift_type == ShiftType::ASR;
        case ShiftOpcode::AND_SR:
        case ShiftOpcode::OR_SR:
        case ShiftOpcode::XOR_SR:
        case ShiftOpcode::AND_NOT_SR:
        case ShiftOpcode::OR_NOT_SR:
        case ShiftOpcode::XOR_NOT_SR:
            return shift_type != ShiftType::INVALID_SHIFT;
        default:
            return false;
    }
}

Reg Aarch64Encoder::AcquireScratchRegister(TypeInfo type)
{
    ASSERT(GetMasm()->GetCurrentScratchRegisterScope() == nullptr);
    auto reg = type.IsFloat() ? GetMasm()->GetScratchVRegisterList()->PopLowestIndex()
                              : GetMasm()->GetScratchRegisterList()->PopLowestIndex();
    ASSERT(reg.IsValid());
    return Reg(reg.GetCode(), type);
}

void Aarch64Encoder::AcquireScratchRegister(Reg reg)
{
    ASSERT(GetMasm()->GetCurrentScratchRegisterScope() == nullptr);
    if (reg == GetTarget().GetLinkReg()) {
        ASSERT_PRINT(!lr_acquired_, "Trying to acquire LR, which hasn't been released before");
        lr_acquired_ = true;
        return;
    }
    auto type = reg.GetType();
    auto reg_id = reg.GetId();

    if (type.IsFloat()) {
        ASSERT(GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(VixlVReg(reg)));
        GetMasm()->GetScratchVRegisterList()->Remove(reg_id);
    } else {
        ASSERT(GetMasm()->GetScratchRegisterList()->IncludesAliasOf(VixlReg(reg)));
        GetMasm()->GetScratchRegisterList()->Remove(reg_id);
    }
}

void Aarch64Encoder::ReleaseScratchRegister(Reg reg)
{
    if (reg == GetTarget().GetLinkReg()) {
        ASSERT_PRINT(lr_acquired_, "Trying to release LR, which hasn't been acquired before");
        lr_acquired_ = false;
    } else if (reg.IsFloat()) {
        GetMasm()->GetScratchVRegisterList()->Combine(reg.GetId());
    } else if (reg.GetId() != GetTarget().GetLinkReg().GetId()) {
        GetMasm()->GetScratchRegisterList()->Combine(reg.GetId());
    }
}

bool Aarch64Encoder::IsScratchRegisterReleased(Reg reg)
{
    if (reg == GetTarget().GetLinkReg()) {
        return !lr_acquired_;
    }
    if (reg.IsFloat()) {
        return GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(VixlVReg(reg));
    }
    return GetMasm()->GetScratchRegisterList()->IncludesAliasOf(VixlReg(reg));
}

void Aarch64Encoder::MakeLibCall(Reg dst, Reg src0, Reg src1, const void *entry_point)
{
    if (!dst.IsFloat()) {
        SetFalseResult();
        return;
    }
    if (dst.GetType() == FLOAT32_TYPE) {
        if (!src0.IsFloat() || !src1.IsFloat()) {
            SetFalseResult();
            return;
        }

        if (src0.GetId() != vixl::aarch64::s0.GetCode() || src1.GetId() != vixl::aarch64::s1.GetCode()) {
            ScopedTmpRegF32 tmp(this);
            GetMasm()->Fmov(VixlVReg(tmp), VixlVReg(src1));
            GetMasm()->Fmov(vixl::aarch64::s0, VixlVReg(src0));
            GetMasm()->Fmov(vixl::aarch64::s1, VixlVReg(tmp));
        }

        MakeCall(entry_point);

        if (dst.GetId() != vixl::aarch64::s0.GetCode()) {
            GetMasm()->Fmov(VixlVReg(dst), vixl::aarch64::s0);
        }
    } else if (dst.GetType() == FLOAT64_TYPE) {
        if (!src0.IsFloat() || !src1.IsFloat()) {
            SetFalseResult();
            return;
        }

        if (src0.GetId() != vixl::aarch64::d0.GetCode() || src1.GetId() != vixl::aarch64::d1.GetCode()) {
            ScopedTmpRegF64 tmp(this);
            GetMasm()->Fmov(VixlVReg(tmp), VixlVReg(src1));

            GetMasm()->Fmov(vixl::aarch64::d0, VixlVReg(src0));
            GetMasm()->Fmov(vixl::aarch64::d1, VixlVReg(tmp));
        }

        MakeCall(entry_point);

        if (dst.GetId() != vixl::aarch64::d0.GetCode()) {
            GetMasm()->Fmov(VixlVReg(dst), vixl::aarch64::d0);
        }
    } else {
        UNREACHABLE();
    }
}

template <bool is_store>
void Aarch64Encoder::LoadStoreRegisters(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp)
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
    // Construct single add for big offset
    size_t sp_offset = 0;
    auto last_offset = (slot + last_reg - start_reg) * DOUBLE_WORD_SIZE_BYTE;

    if (!vixl::aarch64::Assembler::IsImmLSPair(last_offset, vixl::aarch64::kXRegSizeInBytesLog2)) {
        ScopedTmpReg lr_reg(this, true);
        auto tmp = VixlReg(lr_reg);
        sp_offset = slot * DOUBLE_WORD_SIZE_BYTE;
        slot = 0;
        if (vixl::aarch64::Assembler::IsImmAddSub(sp_offset)) {
            GetMasm()->Add(tmp, vixl::aarch64::sp, VixlImm(sp_offset));
        } else {
            GetMasm()->Mov(tmp, VixlImm(sp_offset));
            GetMasm()->Add(tmp, vixl::aarch64::sp, tmp);
        }
        LoadStoreRegistersLoop<is_store>(registers, slot, start_reg, is_fp, tmp);
    } else {
        LoadStoreRegistersLoop<is_store>(registers, slot, start_reg, is_fp, vixl::aarch64::sp);
    }
}

template <bool is_store>
void Aarch64Encoder::LoadStoreRegisters(RegMask registers, bool is_fp, int32_t slot, Reg base, RegMask mask)
{
    if (registers.none()) {
        return;
    }

    int32_t max_offset = (slot + helpers::ToSigned(registers.GetMaxRegister())) * DOUBLE_WORD_SIZE_BYTE;
    int32_t min_offset = (slot + helpers::ToSigned(registers.GetMinRegister())) * DOUBLE_WORD_SIZE_BYTE;

    ScopedTmpRegLazy tmp_reg(this, true);
    // Construct single add for big offset
    if (!vixl::aarch64::Assembler::IsImmLSPair(min_offset, vixl::aarch64::kXRegSizeInBytesLog2) ||
        !vixl::aarch64::Assembler::IsImmLSPair(max_offset, vixl::aarch64::kXRegSizeInBytesLog2)) {
        tmp_reg.Acquire();
        auto lr_reg = VixlReg(tmp_reg);
        ssize_t sp_offset = slot * DOUBLE_WORD_SIZE_BYTE;
        if (vixl::aarch64::Assembler::IsImmAddSub(sp_offset)) {
            GetMasm()->Add(lr_reg, VixlReg(base), VixlImm(sp_offset));
        } else {
            GetMasm()->Mov(lr_reg, VixlImm(sp_offset));
            GetMasm()->Add(lr_reg, VixlReg(base), lr_reg);
        }
        // Adjust new values for slot and base register
        slot = 0;
        base = tmp_reg;
    }

    auto base_reg = VixlReg(base);
    bool has_mask = mask.any();
    int32_t index = has_mask ? static_cast<int32_t>(mask.GetMinRegister()) : 0;
    int32_t last_index = -1;
    ssize_t last_id = -1;

    slot -= index;
    for (ssize_t id = index; id < helpers::ToSigned(registers.size()); id++) {
        if (has_mask) {
            if (!mask.test(id)) {
                continue;
            }
            index++;
        }
        if (!registers.test(id)) {
            continue;
        }
        if (!has_mask) {
            index++;
        }
        if (last_id != -1) {
            auto reg =
                CPURegister(id, vixl::aarch64::kXRegSize, is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
            auto last_reg = CPURegister(last_id, vixl::aarch64::kXRegSize,
                                        is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
            if (!has_mask || last_id + 1 == id) {
                static constexpr ssize_t OFFSET = 2;
                if constexpr (is_store) {  // NOLINT
                    GetMasm()->Stp(last_reg, reg,
                                   MemOperand(base_reg, (slot + index - OFFSET) * DOUBLE_WORD_SIZE_BYTE));
                } else {  // NOLINT
                    GetMasm()->Ldp(last_reg, reg,
                                   MemOperand(base_reg, (slot + index - OFFSET) * DOUBLE_WORD_SIZE_BYTE));
                }
                last_id = -1;
            } else {
                if constexpr (is_store) {  // NOLINT
                    GetMasm()->Str(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTE));
                } else {  // NOLINT
                    GetMasm()->Ldr(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTE));
                }
                last_id = id;
                last_index = index;
            }
        } else {
            last_id = id;
            last_index = index;
        }
    }
    if (last_id != -1) {
        auto last_reg =
            CPURegister(last_id, vixl::aarch64::kXRegSize, is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
        if constexpr (is_store) {  // NOLINT
            GetMasm()->Str(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTE));
        } else {  // NOLINT
            GetMasm()->Ldr(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTE));
        }
    }
}

template <bool is_store>
void Aarch64Encoder::LoadStoreRegistersLoop(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp,
                                            const vixl::aarch64::Register &base_reg)
{
    size_t i = 0;
    const auto GET_NEXT_REG = [&registers, &i, is_fp]() {
        for (; i < registers.size(); i++) {
            if (registers.test(i)) {
                return CPURegister(i++, vixl::aarch64::kXRegSize,
                                   is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
            }
        }
        return CPURegister();
    };

    for (CPURegister next_reg = GET_NEXT_REG(); next_reg.IsValid();) {
        const CPURegister CURR_REG = next_reg;
        next_reg = GET_NEXT_REG();
        if (next_reg.IsValid() && (next_reg.GetCode() - 1 == CURR_REG.GetCode())) {
            if constexpr (is_store) {  // NOLINT
                GetMasm()->Stp(CURR_REG, next_reg,
                               MemOperand(base_reg, (slot + CURR_REG.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTE));
            } else {  // NOLINT
                GetMasm()->Ldp(CURR_REG, next_reg,
                               MemOperand(base_reg, (slot + CURR_REG.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTE));
            }
            next_reg = GET_NEXT_REG();
        } else {
            if constexpr (is_store) {  // NOLINT
                GetMasm()->Str(CURR_REG,
                               MemOperand(base_reg, (slot + CURR_REG.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTE));
            } else {  // NOLINT
                GetMasm()->Ldr(CURR_REG,
                               MemOperand(base_reg, (slot + CURR_REG.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTE));
            }
        }
    }
}

void Aarch64Encoder::PushRegisters(RegMask registers, bool is_fp, [[maybe_unused]] bool align)
{
    static constexpr size_t PAIR_OFFSET = 2 * DOUBLE_WORD_SIZE_BYTE;
    Register last_reg = INVALID_REG;
    for (size_t i = 0; i < registers.size(); i++) {
        if (registers[i]) {
            if (last_reg == INVALID_REG) {
                last_reg = i;
                continue;
            }
            if (is_fp) {
                GetMasm()->stp(vixl::aarch64::VRegister(last_reg, DOUBLE_WORD_SIZE),
                               vixl::aarch64::VRegister(i, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
            } else {
                GetMasm()->stp(vixl::aarch64::Register(last_reg, DOUBLE_WORD_SIZE),
                               vixl::aarch64::Register(i, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
            }
            last_reg = INVALID_REG;
        }
    }
    if (last_reg != INVALID_REG) {
        if (is_fp) {
            GetMasm()->str(vixl::aarch64::VRegister(last_reg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, align ? -PAIR_OFFSET : -DOUBLE_WORD_SIZE_BYTE,
                                      vixl::aarch64::AddrMode::PreIndex));
        } else {
            GetMasm()->str(vixl::aarch64::Register(last_reg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, align ? -PAIR_OFFSET : -DOUBLE_WORD_SIZE_BYTE,
                                      vixl::aarch64::AddrMode::PreIndex));
        }
    }
}

void Aarch64Encoder::PopRegisters(RegMask registers, bool is_fp, [[maybe_unused]] bool align)
{
    static constexpr size_t PAIR_OFFSET = 2 * DOUBLE_WORD_SIZE_BYTE;
    Register last_reg;
    if ((registers.count() & 1U) != 0) {
        last_reg = registers.GetMaxRegister();
        if (is_fp) {
            GetMasm()->ldr(vixl::aarch64::VRegister(last_reg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, align ? PAIR_OFFSET : DOUBLE_WORD_SIZE_BYTE,
                                      vixl::aarch64::AddrMode::PostIndex));
        } else {
            GetMasm()->ldr(vixl::aarch64::Register(last_reg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, align ? PAIR_OFFSET : DOUBLE_WORD_SIZE_BYTE,
                                      vixl::aarch64::AddrMode::PostIndex));
        }
        registers.reset(last_reg);
    }
    last_reg = INVALID_REG;
    for (ssize_t i = registers.size() - 1; i >= 0; i--) {
        if (registers[i]) {
            if (last_reg == INVALID_REG) {
                last_reg = i;
                continue;
            }
            if (is_fp) {
                GetMasm()->ldp(vixl::aarch64::VRegister(i, DOUBLE_WORD_SIZE),
                               vixl::aarch64::VRegister(last_reg, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
            } else {
                GetMasm()->ldp(vixl::aarch64::Register(i, DOUBLE_WORD_SIZE),
                               vixl::aarch64::Register(last_reg, DOUBLE_WORD_SIZE),
                               MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
            }
            last_reg = INVALID_REG;
        }
    }
}

#ifndef PANDA_MINIMAL_VIXL
auto &Aarch64Encoder::GetDecoder() const
{
    if (decoder_ == nullptr) {
        decoder_ = GetAllocator()->New<vixl::aarch64::Decoder>(GetAllocator());
    }
    return *decoder_;
}
#endif

size_t Aarch64Encoder::DisasmInstr([[maybe_unused]] std::ostream &stream, size_t pc,
                                   [[maybe_unused]] ssize_t code_offset) const
{
#ifndef PANDA_MINIMAL_VIXL
    // NOLINTNEXTLINE (cppcoreguidelines-pro-type-member-ini)
    std::array<char, vixl::aarch64::Disassembler::GetDefaultBufferSize()> buf;
    vixl::aarch64::Disassembler disasm(std::data(buf), std::size(buf));

    auto &decoder {GetDecoder()};
    vixl::aarch64::Decoder::ScopedVisitors sv(decoder, {&disasm});
    auto instr = GetMasm()->GetBuffer()->GetOffsetAddress<vixl::aarch64::Instruction *>(pc);

    auto buffer_start = GetMasm()->GetBuffer()->GetOffsetAddress<uintptr_t>(0);
    decoder.Decode(instr);
    if (code_offset < 0) {
        stream << disasm.GetOutput();
    } else {
        stream << std::setw(0x4) << std::right << std::setfill('0') << std::hex
               << reinterpret_cast<uintptr_t>(instr) - buffer_start + code_offset << ": " << disasm.GetOutput()
               << std::setfill(' ') << std::dec;
    }

#endif
    return pc + vixl::aarch64::kInstructionSize;
}
}  // namespace panda::compiler::aarch64
