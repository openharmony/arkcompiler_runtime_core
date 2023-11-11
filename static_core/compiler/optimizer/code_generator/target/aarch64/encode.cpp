/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "compiler/optimizer/code_generator/target/aarch64/target.h"
#include "compiler/optimizer/code_generator/encode.h"
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
    auto value = imm.GetAsInt();
    if (value == 0) {
        EncodeJump(id, src, cc);
        return;
    }

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

    auto value = imm.GetAsInt();
    if (CanEncodeImmLogical(value, src.GetSize() > WORD_SIZE ? DOUBLE_WORD_SIZE : WORD_SIZE)) {
        GetMasm()->Tst(VixlReg(src), VixlImm(value));
        auto label = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(id);
        GetMasm()->B(label, ConvertTest(cc));
    } else {
        ScopedTmpReg tmp_reg(this, src.GetType());
        EncodeMov(tmp_reg, imm);
        EncodeJumpTest(id, src, tmp_reg, cc);
    }
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
    ScopedTmpReg tmp(this, true);
    EncodeMov(tmp, Imm(reinterpret_cast<uintptr_t>(entry_point)));
    GetMasm()->Blr(VixlReg(tmp));
}

void Aarch64Encoder::MakeCall(MemRef entry_point)
{
    ScopedTmpReg tmp(this, true);
    EncodeLdr(tmp, false, entry_point);
    GetMasm()->Blr(VixlReg(tmp));
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
    ScopedTmpReg tmp(this, true);
    LoadPcRelative(tmp, offset);
    GetMasm()->Blr(VixlReg(tmp));
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

void Aarch64Encoder::EncodeCmpFracWithDelta(Reg src)
{
    ASSERT(src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    // Encode (fabs(src - trunc(src)) <= DELTA)
    if (src.GetSize() == WORD_SIZE) {
        ScopedTmpRegF32 tmp(this);
        GetMasm()->Frintz(VixlVReg(tmp), VixlVReg(src));
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        GetMasm()->Fcmp(VixlVReg(tmp), std::numeric_limits<float>::epsilon());
    } else {
        ScopedTmpRegF64 tmp(this);
        GetMasm()->Frintz(VixlVReg(tmp), VixlVReg(src));
        EncodeSub(tmp, src, tmp);
        EncodeAbs(tmp, tmp);
        GetMasm()->Fcmp(VixlVReg(tmp), std::numeric_limits<double>::epsilon());
    }
}

void Aarch64Encoder::EncodeIsInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    auto label_exit = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_inf_or_nan = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    EncodeCmpFracWithDelta(src);
    GetMasm()->B(label_inf_or_nan, vixl::aarch64::Condition::vs);  // Inf or NaN
    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::le);
    GetMasm()->B(label_exit);

    // IsInteger returns false if src is Inf or NaN
    GetMasm()->Bind(label_inf_or_nan);
    EncodeMov(dst, Imm(false));

    GetMasm()->Bind(label_exit);
}

void Aarch64Encoder::EncodeIsSafeInteger(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());
    ASSERT(src.GetSize() == WORD_SIZE || src.GetSize() == DOUBLE_WORD_SIZE);

    auto label_exit = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());
    auto label_false = static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(CreateLabel());

    // Check if IsInteger
    EncodeCmpFracWithDelta(src);
    GetMasm()->B(label_false, vixl::aarch64::Condition::vs);  // Inf or NaN
    GetMasm()->B(label_false, vixl::aarch64::Condition::gt);

    // Check if it is safe, i.e. src can be represented in float/double without losing precision
    if (src.GetSize() == WORD_SIZE) {
        ScopedTmpRegF32 tmp(this);
        EncodeAbs(tmp, src);
        GetMasm()->Fcmp(VixlVReg(tmp), MaxIntAsExactFloat());
    } else {
        ScopedTmpRegF64 tmp(this);
        EncodeAbs(tmp, src);
        GetMasm()->Fcmp(VixlVReg(tmp), MaxIntAsExactDouble());
    }
    GetMasm()->Cset(VixlReg(dst), vixl::aarch64::Condition::le);
    GetMasm()->B(label_exit);

    // Return false if src !IsInteger
    GetMasm()->Bind(label_false);
    EncodeMov(dst, Imm(false));

    GetMasm()->Bind(label_exit);
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

void Aarch64Encoder::EncodeTrunc(Reg dst, Reg src)
{
    GetMasm()->Frintz(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeRoundAway(Reg dst, Reg src)
{
    GetMasm()->Frinta(VixlVReg(dst), VixlVReg(src));
}

void Aarch64Encoder::EncodeRoundToPInf(Reg dst, Reg src)
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
    GetMasm()->Fcmp(VixlVReg(tmp), 0.5);
    GetMasm()->Cinc(VixlReg(dst), VixlReg(dst), vixl::aarch64::Condition::eq);
    GetMasm()->Bind(done);
}

void Aarch64Encoder::EncodeCrc32Update(Reg dst, Reg crc_reg, Reg val_reg)
{
    auto tmp =
        dst.GetId() != crc_reg.GetId() && dst.GetId() != val_reg.GetId() ? dst : ScopedTmpReg(this, dst.GetType());
    GetMasm()->Mvn(VixlReg(tmp), VixlReg(crc_reg));
    GetMasm()->Crc32b(VixlReg(tmp), VixlReg(tmp), VixlReg(val_reg));
    GetMasm()->Mvn(VixlReg(dst), VixlReg(tmp));
}

void Aarch64Encoder::EncodeCompressEightUtf16ToUtf8CharsUsingSimd(Reg src_addr, Reg dst_addr)
{
    ScopedTmpReg tmp1(this, FLOAT64_TYPE);
    ScopedTmpReg tmp2(this, FLOAT64_TYPE);
    auto vixl_vreg1 = vixl::aarch64::VRegister(tmp1.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8B);
    ASSERT(vixl_vreg1.IsValid());
    auto vixl_vreg2 = vixl::aarch64::VRegister(tmp2.GetReg().GetId(), vixl::aarch64::VectorFormat::kFormat8B);
    ASSERT(vixl_vreg2.IsValid());
    auto src = vixl::aarch64::MemOperand(VixlReg(src_addr));
    auto dst = vixl::aarch64::MemOperand(VixlReg(dst_addr));
    GetMasm()->Ld2(vixl_vreg1, vixl_vreg2, src);
    GetMasm()->St1(vixl_vreg1, dst);
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
    cur.Release();
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
    last.Release();
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
    last.Release();
    EncodeStrExclusive(dst, newval, addr, true);
    EncodeJump(retry_ldaxr, dst, Imm(0), Condition::NE);

    EncodeSub(obj, addr, offset); /* restore the original value */
    EncodeMov(dst, cur);
}

void Aarch64Encoder::EncodeMemoryBarrier(memory_order::Order order)
{
    switch (order) {
        case memory_order::ACQUIRE: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierReads);
            break;
        }
        case memory_order::RELEASE: {
            GetMasm()->Dmb(vixl::aarch64::InnerShareable, vixl::aarch64::BarrierWrites);
            break;
        }
        case memory_order::FULL: {
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
            GetMasm()->Fcvtzs(VixlReg(dst), VixlVReg(src));
        } else {
            GetMasm()->Fcvtzu(VixlReg(dst), VixlVReg(src));
        }
        return;
    }
    if (src.IsScalar() && dst.IsFloat()) {
        auto rzero = GetRegfile()->GetZeroReg().GetId();
        if (src.GetId() == rzero) {
            if (dst.GetSize() == WORD_SIZE) {
                GetMasm()->Fmov(VixlVReg(dst), 0.0F);
            } else {
                GetMasm()->Fmov(VixlVReg(dst), 0.0);
            }
        } else if (src_signed) {
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

void Aarch64Encoder::EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow)
{
    ASSERT(IsJsNumberCast());
    ASSERT(src.IsFloat() && dst.IsScalar());

    CHECK_EQ(src.GetSize(), BITS_PER_UINT64);
    CHECK_EQ(dst.GetSize(), BITS_PER_UINT32);

    // We use slow path, because in general JS double -> int32 cast is complex and we check only few common cases here
    // and move other checks in slow path. In case CPU supports special JS double -> int32 instruction we do not need
    // slow path.
    if (!IsLabelValid(slow)) {
        // use special JS aarch64 instruction
#ifndef NDEBUG
        vixl::CPUFeaturesScope scope(GetMasm(), vixl::CPUFeatures::kFP, vixl::CPUFeatures::kJSCVT);
#endif
        GetMasm()->Fjcvtzs(VixlReg(dst), VixlVReg(src));
        return;
    }

    // infinite and big numbers will overflow here to INT64_MIN or INT64_MAX, but NaN casts to 0
    GetMasm()->Fcvtzs(VixlReg(dst, DOUBLE_WORD_SIZE), VixlVReg(src));
    // check INT64_MIN
    GetMasm()->Cmp(VixlReg(dst, DOUBLE_WORD_SIZE), VixlImm(1));
    // check INT64_MAX
    GetMasm()->Ccmp(VixlReg(dst, DOUBLE_WORD_SIZE), VixlImm(-1), vixl::aarch64::StatusFlags::VFlag,
                    vixl::aarch64::Condition::vc);
    auto slow_label {static_cast<Aarch64LabelHolder *>(GetLabels())->GetLabel(slow)};
    // jump to slow path in case of overflow
    GetMasm()->B(slow_label, vixl::aarch64::Condition::vs);
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

void Aarch64Encoder::EncodeNegOverflowAndZero(compiler::LabelHolder::LabelId id, Reg dst, Reg src)
{
    ASSERT(!dst.IsFloat() && !src.IsFloat());
    // NOLINTNEXTLINE(readability-magic-numbers)
    EncodeJumpTest(id, src, Imm(0x7fffffff), Condition::TST_EQ);
    GetMasm()->Neg(VixlReg(dst).W(), VixlReg(src).W());
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
        using Fp = float (*)(float, float);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<Fp>(fmodf)));
    } else {
        using Fp = double (*)(double, double);
        MakeLibCall(dst, src0, src1, reinterpret_cast<void *>(static_cast<Fp>(fmod)));
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

    GetMasm()->Lsl(VixlReg(dst), VixlReg(src), imm.GetAsInt());
}

void Aarch64Encoder::EncodeShr(Reg dst, Reg src, Imm imm)
{
    int64_t imm_value = static_cast<uint64_t>(imm.GetAsInt()) & (dst.GetSize() - 1);

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
    GetMasm()->Asr(VixlReg(dst), VixlReg(src), imm.GetAsInt());
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
            GetMasm()->Fmov(VixlVReg(dst), src.GetAsFloat());
        } else {
            GetMasm()->Fmov(VixlVReg(dst), src.GetAsDouble());
        }
        return;
    }
    if (dst.GetSize() > WORD_SIZE) {
        GetMasm()->Mov(VixlReg(dst), VixlImm(src));
    } else {
        GetMasm()->Mov(VixlReg(dst), VixlImm(static_cast<int32_t>(src.GetAsInt())));
    }
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
    if (mem.HasIndex()) {
        ScopedTmpRegU64 tmp_reg(this);
        if (mem.HasScale()) {
            EncodeAdd(tmp_reg, mem.GetBase(), Shift(mem.GetIndex(), mem.GetScale()));
        } else {
            EncodeAdd(tmp_reg, mem.GetBase(), mem.GetIndex());
        }
        mem = MemRef(tmp_reg, mem.GetDisp());
    }

    auto rzero = GetRegfile()->GetZeroReg().GetId();
    if (!ConvertMem(mem).IsValid() || (dst.GetId() == rzero && dst.IsScalar())) {
        EncodeLdrAcquireInvalid(dst, dst_signed, mem);
        return;
    }

    ASSERT(!mem.HasIndex() && !mem.HasScale());
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
        if (mem.HasScale()) {
            EncodeAdd(base, mem_was_fixed ? base : mem.GetBase(), Shift(mem.GetIndex(), mem.GetScale()));
        } else {
            EncodeAdd(base, mem_was_fixed ? base : mem.GetBase(), mem.GetIndex());
        }
        mem_was_fixed = true;
    }

    if (mem_was_fixed) {
        fixed_mem = MemRef(base);
    } else {
        fixed_mem = mem;
    }

#ifndef NDEBUG
    CheckAlignment(fixed_mem, src.GetSize());
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

void Aarch64Encoder::EncodeSti(int64_t src, uint8_t src_size_bytes, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        EncodeStr(rzero, mem);
        return;
    }

    ScopedTmpRegU64 tmp_reg(this);
    auto tmp = VixlReg(tmp_reg);
    GetMasm()->Mov(tmp, VixlImm(src));
    if (src_size_bytes == 1U) {
        GetMasm()->Strb(tmp, ConvertMem(mem));
        return;
    }
    if (src_size_bytes == HALF_WORD_SIZE_BYTES) {
        GetMasm()->Strh(tmp, ConvertMem(mem));
        return;
    }
    ASSERT((src_size_bytes == WORD_SIZE_BYTES) || (src_size_bytes == DOUBLE_WORD_SIZE_BYTES));
    GetMasm()->Str(tmp, ConvertMem(mem));
}

void Aarch64Encoder::EncodeSti(float src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        EncodeStr(rzero, mem);
        return;
    }
    ScopedTmpRegF32 tmp_reg(this);
    GetMasm()->Fmov(VixlVReg(tmp_reg).S(), src);
    EncodeStr(tmp_reg, mem);
}

void Aarch64Encoder::EncodeSti(double src, MemRef mem)
{
    if (!ConvertMem(mem).IsValid()) {
        auto rzero = GetRegfile()->GetZeroReg();
        EncodeStr(rzero, mem);
        return;
    }
    ScopedTmpRegF64 tmp_reg(this);
    GetMasm()->Fmov(VixlVReg(tmp_reg).D(), src);
    EncodeStr(tmp_reg, mem);
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
    if (src2.IsScalar()) {
        GetMasm()->Cmp(VixlReg(src2), VixlReg(src3));
    } else {
        GetMasm()->Fcmp(VixlVReg(src2), VixlVReg(src3));
    }
    if (dst.IsFloat()) {
        GetMasm()->Fcsel(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), Convert(cc));
    } else {
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), Convert(cc));
    }
}

void Aarch64Encoder::EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    if (src2.IsScalar()) {
        GetMasm()->Cmp(VixlReg(src2), VixlImm(imm));
    } else {
        GetMasm()->Fcmp(VixlVReg(src2), imm.GetAsDouble());
    }
    if (dst.IsFloat()) {
        GetMasm()->Fcsel(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), Convert(cc));
    } else {
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), Convert(cc));
    }
}

void Aarch64Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc)
{
    ASSERT(!src2.IsFloat() && !src3.IsFloat());
    GetMasm()->Tst(VixlReg(src2), VixlReg(src3));
    if (dst.IsFloat()) {
        GetMasm()->Fcsel(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), ConvertTest(cc));
    } else {
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), ConvertTest(cc));
    }
}

void Aarch64Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    ASSERT(!src2.IsFloat());
    ASSERT(CanEncodeImmLogical(imm.GetAsInt(), src2.GetSize() > WORD_SIZE ? DOUBLE_WORD_SIZE : WORD_SIZE));
    GetMasm()->Tst(VixlReg(src2), VixlImm(imm));
    if (dst.IsFloat()) {
        GetMasm()->Fcsel(VixlVReg(dst), VixlVReg(src0), VixlVReg(src1), ConvertTest(cc));
    } else {
        GetMasm()->Csel(VixlReg(dst), VixlReg(src0), VixlReg(src1), ConvertTest(cc));
    }
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
    GetMasm()->Ubfx(VixlReg(dst), VixlReg(src0), imm1.GetAsInt(), imm2.GetAsInt());
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
#ifndef NDEBUG
    if (size < DOUBLE_WORD_SIZE) {
        // Test if the highest part is consistent:
        ASSERT(((imm >> size) == 0) || (((~imm) >> size) == 0));
    }
#endif  // NDEBUG
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

bool Aarch64Encoder::CanEncodeFloatSelect()
{
    return true;
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

template <bool IS_STORE>
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
    auto last_offset = (slot + last_reg - start_reg) * DOUBLE_WORD_SIZE_BYTES;

    if (!vixl::aarch64::Assembler::IsImmLSPair(last_offset, vixl::aarch64::kXRegSizeInBytesLog2)) {
        ScopedTmpReg lr_reg(this, true);
        auto tmp = VixlReg(lr_reg);
        sp_offset = slot * DOUBLE_WORD_SIZE_BYTES;
        slot = 0;
        if (vixl::aarch64::Assembler::IsImmAddSub(sp_offset)) {
            GetMasm()->Add(tmp, vixl::aarch64::sp, VixlImm(sp_offset));
        } else {
            GetMasm()->Mov(tmp, VixlImm(sp_offset));
            GetMasm()->Add(tmp, vixl::aarch64::sp, tmp);
        }
        LoadStoreRegistersLoop<IS_STORE>(registers, slot, start_reg, is_fp, tmp);
    } else {
        LoadStoreRegistersLoop<IS_STORE>(registers, slot, start_reg, is_fp, vixl::aarch64::sp);
    }
}

template <bool IS_STORE>
void Aarch64Encoder::LoadStoreRegisters(RegMask registers, bool is_fp, int32_t slot, Reg base, RegMask mask)
{
    if (registers.none()) {
        return;
    }

    int32_t max_offset = (slot + helpers::ToSigned(registers.GetMaxRegister())) * DOUBLE_WORD_SIZE_BYTES;
    int32_t min_offset = (slot + helpers::ToSigned(registers.GetMinRegister())) * DOUBLE_WORD_SIZE_BYTES;

    ScopedTmpRegLazy tmp_reg(this, true);
    // Construct single add for big offset
    if (!vixl::aarch64::Assembler::IsImmLSPair(min_offset, vixl::aarch64::kXRegSizeInBytesLog2) ||
        !vixl::aarch64::Assembler::IsImmLSPair(max_offset, vixl::aarch64::kXRegSizeInBytesLog2)) {
        tmp_reg.AcquireWithLr();
        auto lr_reg = VixlReg(tmp_reg);
        ssize_t sp_offset = slot * DOUBLE_WORD_SIZE_BYTES;
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
        if (last_id == -1) {
            last_id = id;
            last_index = index;
            continue;
        }

        auto reg = CPURegister(id, vixl::aarch64::kXRegSize, is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
        auto last_reg =
            CPURegister(last_id, vixl::aarch64::kXRegSize, is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
        if (!has_mask || last_id + 1 == id) {
            static constexpr ssize_t OFFSET = 2;
            if constexpr (IS_STORE) {  // NOLINT
                GetMasm()->Stp(last_reg, reg, MemOperand(base_reg, (slot + index - OFFSET) * DOUBLE_WORD_SIZE_BYTES));
            } else {  // NOLINT
                GetMasm()->Ldp(last_reg, reg, MemOperand(base_reg, (slot + index - OFFSET) * DOUBLE_WORD_SIZE_BYTES));
            }
            last_id = -1;
        } else {
            if constexpr (IS_STORE) {  // NOLINT
                GetMasm()->Str(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTES));
            } else {  // NOLINT
                GetMasm()->Ldr(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTES));
            }
            last_id = id;
            last_index = index;
        }
    }
    if (last_id != -1) {
        auto last_reg =
            CPURegister(last_id, vixl::aarch64::kXRegSize, is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
        if constexpr (IS_STORE) {  // NOLINT
            GetMasm()->Str(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTES));
        } else {  // NOLINT
            GetMasm()->Ldr(last_reg, MemOperand(base_reg, (slot + last_index - 1) * DOUBLE_WORD_SIZE_BYTES));
        }
    }
}

template <bool IS_STORE>
void Aarch64Encoder::LoadStoreRegistersLoop(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp,
                                            const vixl::aarch64::Register &base_reg)
{
    size_t i = 0;
    const auto get_next_reg = [&registers, &i, is_fp]() {
        for (; i < registers.size(); i++) {
            if (registers.test(i)) {
                return CPURegister(i++, vixl::aarch64::kXRegSize,
                                   is_fp ? CPURegister::kVRegister : CPURegister::kRegister);
            }
        }
        return CPURegister();
    };

    for (CPURegister next_reg = get_next_reg(); next_reg.IsValid();) {
        const CPURegister curr_reg = next_reg;
        next_reg = get_next_reg();
        if (next_reg.IsValid() && (next_reg.GetCode() - 1 == curr_reg.GetCode())) {
            if constexpr (IS_STORE) {  // NOLINT
                GetMasm()->Stp(curr_reg, next_reg,
                               MemOperand(base_reg, (slot + curr_reg.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTES));
            } else {  // NOLINT
                GetMasm()->Ldp(curr_reg, next_reg,
                               MemOperand(base_reg, (slot + curr_reg.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTES));
            }
            next_reg = get_next_reg();
        } else {
            if constexpr (IS_STORE) {  // NOLINT
                GetMasm()->Str(curr_reg,
                               MemOperand(base_reg, (slot + curr_reg.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTES));
            } else {  // NOLINT
                GetMasm()->Ldr(curr_reg,
                               MemOperand(base_reg, (slot + curr_reg.GetCode() - start_reg) * DOUBLE_WORD_SIZE_BYTES));
            }
        }
    }
}

void Aarch64Encoder::PushRegisters(RegMask registers, bool is_fp)
{
    static constexpr size_t PAIR_OFFSET = 2 * DOUBLE_WORD_SIZE_BYTES;
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
                           MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
        } else {
            GetMasm()->str(vixl::aarch64::Register(last_reg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, -PAIR_OFFSET, vixl::aarch64::AddrMode::PreIndex));
        }
    }
}

void Aarch64Encoder::PopRegisters(RegMask registers, bool is_fp)
{
    static constexpr size_t PAIR_OFFSET = 2 * DOUBLE_WORD_SIZE_BYTES;
    Register last_reg;
    if ((registers.count() & 1U) != 0) {
        last_reg = registers.GetMaxRegister();
        if (is_fp) {
            GetMasm()->ldr(vixl::aarch64::VRegister(last_reg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
        } else {
            GetMasm()->ldr(vixl::aarch64::Register(last_reg, DOUBLE_WORD_SIZE),
                           MemOperand(vixl::aarch64::sp, PAIR_OFFSET, vixl::aarch64::AddrMode::PostIndex));
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
vixl::aarch64::Decoder &Aarch64Encoder::GetDecoder() const
{
    if (!decoder_) {
        decoder_.emplace(GetAllocator());
        decoder_->visitors()->push_back(&GetDisasm());
    }
    return *decoder_;
}

vixl::aarch64::Disassembler &Aarch64Encoder::GetDisasm() const
{
    if (!disasm_) {
        disasm_.emplace(GetAllocator());
    }
    return *disasm_;
}
#endif

size_t Aarch64Encoder::DisasmInstr([[maybe_unused]] std::ostream &stream, size_t pc,
                                   [[maybe_unused]] ssize_t code_offset) const
{
#ifndef PANDA_MINIMAL_VIXL
    auto buffer_start = GetMasm()->GetBuffer()->GetOffsetAddress<uintptr_t>(0);
    auto instr = GetMasm()->GetBuffer()->GetOffsetAddress<vixl::aarch64::Instruction *>(pc);
    GetDecoder().Decode(instr);
    if (code_offset < 0) {
        stream << GetDisasm().GetOutput();
    } else {
        stream << std::setw(0x4) << std::right << std::setfill('0') << std::hex
               << reinterpret_cast<uintptr_t>(instr) - buffer_start + code_offset << ": " << GetDisasm().GetOutput()
               << std::setfill(' ') << std::dec;
    }

#endif
    return pc + vixl::aarch64::kInstructionSize;
}
}  // namespace panda::compiler::aarch64
