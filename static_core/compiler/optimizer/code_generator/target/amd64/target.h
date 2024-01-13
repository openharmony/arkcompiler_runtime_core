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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_AMD64_TARGET_H
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_AMD64_TARGET_H

#include "compiler/optimizer/code_generator/callconv.h"

#include "asmjit/x86.h"
#include "target_info.h"

namespace panda::compiler::amd64 {
const size_t MAX_SCALAR_PARAM_ID = 5;  // %rdi, %rsi, %rdx, %rcx, %r8, %r9
const size_t MAX_VECTOR_PARAM_ID = 7;  // %xmm0-%xmm7

static inline bool IsConditionSigned(Condition cc)
{
    switch (cc) {
        case Condition::LT:
        case Condition::LE:
        case Condition::GT:
        case Condition::GE:
            return true;

        default:
            return false;
    }
}

/// Converters
static inline asmjit::x86::Condition::Code ArchCc(Condition cc, bool isFloat = false)
{
    switch (cc) {
        case Condition::EQ:
            return asmjit::x86::Condition::Code::kEqual;
        case Condition::NE:
            return asmjit::x86::Condition::Code::kNotEqual;
        case Condition::LT:
            return isFloat ? asmjit::x86::Condition::Code::kUnsignedLT : asmjit::x86::Condition::Code::kSignedLT;
        case Condition::GT:
            return isFloat ? asmjit::x86::Condition::Code::kUnsignedGT : asmjit::x86::Condition::Code::kSignedGT;
        case Condition::LE:
            return isFloat ? asmjit::x86::Condition::Code::kUnsignedLE : asmjit::x86::Condition::Code::kSignedLE;
        case Condition::GE:
            return isFloat ? asmjit::x86::Condition::Code::kUnsignedGE : asmjit::x86::Condition::Code::kSignedGE;
        case Condition::LO:
            return asmjit::x86::Condition::Code::kUnsignedLT;
        case Condition::LS:
            return asmjit::x86::Condition::Code::kUnsignedLE;
        case Condition::HI:
            return asmjit::x86::Condition::Code::kUnsignedGT;
        case Condition::HS:
            return asmjit::x86::Condition::Code::kUnsignedGE;
        // NOTE(igorban) : Remove them
        case Condition::MI:
            return asmjit::x86::Condition::Code::kNegative;
        case Condition::PL:
            return asmjit::x86::Condition::Code::kPositive;
        case Condition::VS:
            return asmjit::x86::Condition::Code::kOverflow;
        case Condition::VC:
            return asmjit::x86::Condition::Code::kNotOverflow;
        case Condition::AL:
        case Condition::NV:
        default:
            UNREACHABLE();
            return asmjit::x86::Condition::Code::kEqual;
    }
}

static inline asmjit::x86::Condition::Code ArchCcTest(Condition cc)
{
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);
    return cc == Condition::TST_EQ ? asmjit::x86::Condition::Code::kEqual : asmjit::x86::Condition::Code::kNotEqual;
}

static inline bool CcMatchesNan(Condition cc)
{
    switch (cc) {
        case Condition::NE:
        case Condition::LT:
        case Condition::LE:
        case Condition::HI:
        case Condition::HS:
            return true;

        default:
            return false;
    }
}

class AsmJitErrorHandler : public asmjit::ErrorHandler {
public:
    explicit AsmJitErrorHandler(Encoder *encoder) : encoder_(encoder)
    {
        ASSERT(encoder != nullptr);
    }

    void handleError([[maybe_unused]] asmjit::Error err, [[maybe_unused]] const char *message,
                     [[maybe_unused]] asmjit::BaseEmitter *origin) override
    {
        encoder_->SetFalseResult();
    }

    NO_MOVE_SEMANTIC(AsmJitErrorHandler);
    NO_COPY_SEMANTIC(AsmJitErrorHandler);
    ~AsmJitErrorHandler() override = default;

private:
    Encoder *encoder_ {nullptr};
};

class RegList {
public:
    explicit RegList(size_t mask) : mask_(mask)
    {
        for (size_t i = 0; i < sizeof(size_t) * BITS_PER_BYTE; ++i) {
            if (Has(i)) {
                ++count_;
            }
        }
    }

    DEFAULT_MOVE_SEMANTIC(RegList);
    DEFAULT_COPY_SEMANTIC(RegList);
    ~RegList() = default;

    explicit operator size_t() const
    {
        return mask_;
    }

    bool IsEmpty() const
    {
        return count_ == size_t(0);
    }

    size_t GetCount() const
    {
        return count_;
    }

    bool Has(size_t i) const
    {
        return (mask_ & (size_t(1) << i)) != 0;
    }

    void Add(size_t i)
    {
        if (Has(i)) {
            return;
        }
        mask_ |= size_t(1) << i;
        ++count_;
    }

    void Remove(size_t i)
    {
        if (!Has(i)) {
            return;
        }
        mask_ &= ~(size_t(1) << i);
        --count_;
    }

    size_t Pop()
    {
        ASSERT(!IsEmpty());
        size_t i = __builtin_ctzll(mask_);
        Remove(i);
        return i;
    }

    size_t GetMask() const
    {
        return mask_;
    }

private:
    size_t mask_ {0};
    size_t count_ {0};
};

/// Converters
static inline asmjit::x86::Gp ArchReg(Reg reg, uint8_t size = 0)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        size_t regSize = size == 0 ? reg.GetSize() : size;
        auto archId = ConvertRegNumber(reg.GetId());

        asmjit::x86::Gp archReg;
        switch (regSize) {
            case DOUBLE_WORD_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::Gpq::kSignature, archId);
                break;
            case WORD_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::Gpd::kSignature, archId);
                break;
            case HALF_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::Gpw::kSignature, archId);
                break;
            case BYTE_SIZE:
                archReg = asmjit::x86::Gp(asmjit::x86::GpbLo::kSignature, archId);
                break;

            default:
                UNREACHABLE();
        }

        ASSERT(archReg.isValid());
        return archReg;
    }
    if (reg.GetId() == ConvertRegNumber(asmjit::x86::rsp.id())) {
        return asmjit::x86::rsp;
    }

    // Invalid register type
    UNREACHABLE();
    return asmjit::x86::rax;
}

static inline asmjit::x86::Xmm ArchVReg(Reg reg)
{
    ASSERT(reg.IsValid() && reg.IsFloat());
    auto archVreg = asmjit::x86::xmm(reg.GetId());
    return archVreg;
}

static inline asmjit::Imm ArchImm(Imm imm)
{
    ASSERT(imm.GetType() == INT64_TYPE);
    return asmjit::imm(imm.GetAsInt());
}

static inline uint64_t ImmToUnsignedInt(Imm imm)
{
    ASSERT(imm.GetType() == INT64_TYPE);
    return uint64_t(imm.GetAsInt());
}

static inline bool ImmFitsSize(int64_t imm, uint8_t size)
{
    if (size == DOUBLE_WORD_SIZE) {
        size = WORD_SIZE;
    }

    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    int64_t max = (uint64_t(1) << (size - 1U)) - 1U;
    int64_t min = ~uint64_t(max);
    ASSERT(min < 0);
    ASSERT(max > 0);

    return imm >= min && imm <= max;
}

class ArchMem {
public:
    explicit ArchMem(MemRef mem)
    {
        bool base = mem.HasBase();
        bool regoffset = mem.HasIndex();
        bool shift = mem.HasScale();
        bool offset = mem.HasDisp();

        if (base && !regoffset && !shift) {
            // Default memory - base + offset
            mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), mem.GetDisp());
        } else if (base && regoffset && !offset) {
            auto baseSize = mem.GetBase().GetSize();
            auto indexSize = mem.GetIndex().GetSize();

            ASSERT(baseSize >= indexSize);
            ASSERT(indexSize >= WORD_SIZE);

            if (baseSize > indexSize) {
                needExtendIndex_ = true;
            }

            if (mem.GetScale() == 0) {
                mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), ArchReg(mem.GetIndex(), baseSize));
            } else {
                auto scale = mem.GetScale();
                if (scale <= 3U) {
                    mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), ArchReg(mem.GetIndex(), baseSize), scale);
                } else {
                    mem_ = asmjit::x86::ptr(ArchReg(mem.GetBase()), ArchReg(mem.GetIndex(), baseSize));
                    bigShift_ = scale;
                }
            }
        } else {
            // Wrong memRef
            UNREACHABLE();
        }
    }

    DEFAULT_MOVE_SEMANTIC(ArchMem);
    DEFAULT_COPY_SEMANTIC(ArchMem);
    ~ArchMem() = default;

    asmjit::x86::Mem Prepare(asmjit::x86::Assembler *masm)
    {
        if (isPrepared_) {
            return mem_;
        }

        if (bigShift_ != 0) {
            ASSERT(!mem_.hasOffset() && mem_.hasIndex() && bigShift_ > 3U);
            masm->shl(mem_.indexReg().as<asmjit::x86::Gp>(), asmjit::imm(bigShift_));
        }

        if (needExtendIndex_) {
            ASSERT(mem_.hasIndex());
            auto qIndex = mem_.indexReg().as<asmjit::x86::Gp>();
            auto dIndex {qIndex};
            dIndex.setSignature(asmjit::x86::Gpd::kSignature);
            masm->movsxd(qIndex, dIndex);
        }

        isPrepared_ = true;
        return mem_;
    }

private:
    int64_t bigShift_ {0};
    asmjit::x86::Mem mem_;
    bool needExtendIndex_ {false};
    bool isPrepared_ {false};
};

/*
 * Scalar registers mapping:
 * +-----------+---------------------+
 * | AMD64 Reg |      Panda Reg      |
 * +-----------+---------------------+
 * | rax       | r0                  |
 * | rcx       | r1                  |
 * | rdx       | r2                  |
 * | rbx       | r3 (renamed to r11) |
 * | rsp       | r4 (renamed to r10) |
 * | rbp       | r5 (renamed to r9)  |
 * | rsi       | r6                  |
 * | rdi       | r7                  |
 * | r8        | r8                  |
 * | r9        | r9 (renamed to r5)  |
 * | r10       | r10 (renamed to r4) |
 * | r11       | r11 (renamed to r3) |
 * | r12       | r12                 |
 * | r13       | r13                 |
 * | r14       | r14                 |
 * | r15       | r15                 |
 * | <no reg>  | r16-r31             |
 * +-----------+---------------------+
 *
 * Vector registers mapping:
 * xmm[i] <-> vreg[i], 0 <= i <= 15
 */
class Amd64RegisterDescription final : public RegistersDescription {
public:
    explicit Amd64RegisterDescription(ArenaAllocator *allocator);
    NO_MOVE_SEMANTIC(Amd64RegisterDescription);
    NO_COPY_SEMANTIC(Amd64RegisterDescription);
    ~Amd64RegisterDescription() override = default;

    ArenaVector<Reg> GetCalleeSaved() override;
    void SetCalleeSaved(const ArenaVector<Reg> &regs) override;
    // Set used regs - change GetCallee
    void SetUsedRegs(const ArenaVector<Reg> &regs) override;

    RegMask GetCallerSavedRegMask() const override
    {
        return RegMask(callerSaved_.GetMask());
    }

    VRegMask GetCallerSavedVRegMask() const override
    {
        return VRegMask(callerSavedv_.GetMask());
    }

    bool IsCalleeRegister(Reg reg) override
    {
        bool isFp = reg.IsFloat();
        return reg.GetId() >= GetFirstCalleeReg(Arch::X86_64, isFp) &&
               reg.GetId() <= GetLastCalleeReg(Arch::X86_64, isFp);
    }

    Reg GetZeroReg() const override
    {
        return INVALID_REGISTER;  // there is no one
    }

    bool IsZeroReg([[maybe_unused]] Reg reg) const override
    {
        return false;
    }

    Reg::RegIDType GetTempReg() override
    {
        return compiler::arch_info::x86_64::TEMP_REGS.GetMaxRegister();
    }

    Reg::RegIDType GetTempVReg() override
    {
        return compiler::arch_info::x86_64::TEMP_FP_REGS.GetMaxRegister();
    }

    RegMask GetDefaultRegMask() const override
    {
        static constexpr size_t HIGH_MASK {0xFFFF0000};

        RegMask regMask(HIGH_MASK);
        regMask |= compiler::arch_info::x86_64::TEMP_REGS;
        regMask.set(ConvertRegNumber(asmjit::x86::rbp.id()));
        regMask.set(ConvertRegNumber(asmjit::x86::rsp.id()));
        regMask.set(GetThreadReg(Arch::X86_64));
        return regMask;
    }

    VRegMask GetVRegMask() override
    {
        static constexpr size_t HIGH_MASK {0xFFFF0000};

        VRegMask vregMask(HIGH_MASK);
        vregMask |= compiler::arch_info::x86_64::TEMP_FP_REGS;
        return vregMask;
    }

    // Check register mapping
    bool SupportMapping(uint32_t type) override
    {
        // Current implementation does not support reg-reg mapping
        if ((type & (RegMapping::VECTOR_VECTOR | RegMapping::FLOAT_FLOAT)) != 0U) {
            return false;
        }
        // Scalar and float registers lay in different registers
        if ((type & (RegMapping::SCALAR_VECTOR | RegMapping::SCALAR_FLOAT)) != 0U) {
            return false;
        }
        return true;
    };

    bool IsValid() const override
    {
        return true;
    }

    bool IsRegUsed(ArenaVector<Reg> vecReg, Reg reg) override;

public:
    // Special implementation-specific getters
    size_t GetCalleeSavedR()
    {
        return static_cast<size_t>(calleeSaved_);
    }
    size_t GetCalleeSavedV()
    {
        return static_cast<size_t>(calleeSavedv_);
    }
    size_t GetCallerSavedR()
    {
        return static_cast<size_t>(callerSaved_);
    }
    size_t GetCallerSavedV()
    {
        return static_cast<size_t>(callerSavedv_);
    }

    Reg AcquireScratchRegister(TypeInfo type)
    {
        if (type.IsFloat()) {
            return Reg(scratchv_.Pop(), type);
        }
        return Reg(scratch_.Pop(), type);
    }

    void AcquireScratchRegister(Reg reg)
    {
        if (reg.GetType().IsFloat()) {
            ASSERT(scratchv_.Has(reg.GetId()));
            scratchv_.Remove(reg.GetId());
        } else {
            ASSERT(scratch_.Has(reg.GetId()));
            scratch_.Remove(reg.GetId());
        }
    }

    void ReleaseScratchRegister(Reg reg)
    {
        if (reg.IsFloat()) {
            scratchv_.Add(reg.GetId());
        } else {
            scratch_.Add(reg.GetId());
        }
    }

    bool IsScratchRegisterReleased(Reg reg)
    {
        if (reg.GetType().IsFloat()) {
            return scratchv_.Has(reg.GetId());
        }
        return scratch_.Has(reg.GetId());
    }

    RegList GetScratchRegisters() const
    {
        return scratch_;
    }

    RegList GetScratchFPRegisters() const
    {
        return scratchv_;
    }

    size_t GetScratchRegistersCount() const
    {
        return scratch_.GetCount();
    }

    size_t GetScratchFPRegistersCount() const
    {
        return scratchv_.GetCount();
    }

    RegMask GetScratchRegistersMask() const
    {
        return RegMask(scratch_.GetMask());
    }

    RegMask GetScratchFpRegistersMask() const
    {
        return RegMask(scratchv_.GetMask());
    }

private:
    ArenaVector<Reg> usedRegs_;

    RegList calleeSaved_ {GetCalleeRegsMask(Arch::X86_64, false).GetValue()};
    RegList callerSaved_ {GetCallerRegsMask(Arch::X86_64, false).GetValue()};

    RegList calleeSavedv_ {GetCalleeRegsMask(Arch::X86_64, true).GetValue()};
    RegList callerSavedv_ {GetCallerRegsMask(Arch::X86_64, true).GetValue()};

    RegList scratch_ {compiler::arch_info::x86_64::TEMP_REGS.to_ulong()};
    RegList scratchv_ {compiler::arch_info::x86_64::TEMP_FP_REGS.to_ulong()};
};  // Amd64RegisterDescription

class Amd64Encoder;

class Amd64LabelHolder final : public LabelHolder {
public:
    using LabelType = asmjit::Label;

    explicit Amd64LabelHolder(Encoder *enc) : LabelHolder(enc), labels_(enc->GetAllocator()->Adapter()) {};
    NO_MOVE_SEMANTIC(Amd64LabelHolder);
    NO_COPY_SEMANTIC(Amd64LabelHolder);
    ~Amd64LabelHolder() override = default;

    LabelId CreateLabel() override;

    void CreateLabels(LabelId max) override
    {
        for (LabelId i = 0; i < max; ++i) {
            CreateLabel();
        }
    };

    void BindLabel(LabelId id) override;

    LabelType *GetLabel(LabelId id)
    {
        ASSERT(labels_.size() > id);
        return labels_[id];
    }

    LabelId Size() override
    {
        return labels_.size();
    };

private:
    ArenaVector<LabelType *> labels_;
    LabelId id_ {0};
    friend Amd64Encoder;
};  // Amd64LabelHolder

class Amd64Encoder final : public Encoder {
public:
    using Encoder::Encoder;
    explicit Amd64Encoder(ArenaAllocator *allocator);

    LabelHolder *GetLabels() const override
    {
        ASSERT(labels_ != nullptr);
        return labels_;
    };

    ~Amd64Encoder() override;

    NO_COPY_SEMANTIC(Amd64Encoder);
    NO_MOVE_SEMANTIC(Amd64Encoder);

    bool IsValid() const override
    {
        return true;
    }

    static constexpr auto GetTarget()
    {
        return panda::compiler::Target(Arch::X86_64);
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UnaryOperation(opc) void Encode##opc(Reg dst, Reg src0) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperation(opc)                                \
    void Encode##opc(Reg dst, Reg src0, Reg src1) override; \
    void Encode##opc(Reg dst, Reg src0, Imm src1) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, TYPE) TYPE(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)

#undef UnaryOperation
#undef BinaryOperation
#undef INST_DEF

    void EncodeNop() override;

    // Additional special instructions
    void EncodeAdd(Reg dst, Reg src0, Shift src1) override;

    void EncodeCastToBool(Reg dst, Reg src) override;
    void EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned) override;
    void EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow) override;
    void EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1) override;

    void EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeNegOverflowAndZero(compiler::LabelHolder::LabelId id, Reg dst, Reg src) override;

    void EncodeLdr(Reg dst, bool dstSigned, MemRef mem) override;
    void EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem) override;

    void EncodeMov(Reg dst, Imm src) override;
    void EncodeStr(Reg src, MemRef mem) override;
    void EncodeStrRelease(Reg src, MemRef mem) override;
    // zerod high part: [reg.size, 64)
    void EncodeStrz(Reg src, MemRef mem) override;
    void EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem) override;
    void EncodeSti(float src, MemRef mem) override;
    void EncodeSti(double src, MemRef mem) override;
    // size must be 8, 16,32 or 64
    void EncodeMemCopy(MemRef memFrom, MemRef memTo, size_t size) override;
    // size must be 8, 16,32 or 64
    // zerod high part: [reg.size, 64)
    void EncodeMemCopyz(MemRef memFrom, MemRef memTo, size_t size) override;

    void EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc) override;

    void EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc) override;

    void EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc) override;
    void EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc) override;
    void EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc) override;
    void EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc) override;

    void EncodeLdp(Reg dst0, Reg dst1, bool dstSigned, MemRef mem) override;

    void EncodeStp(Reg src0, Reg src1, MemRef mem) override;

    /* builtins-related encoders */
    void EncodeIsInf(Reg dst, Reg src) override;
    void EncodeIsInteger(Reg dst, Reg src) override;
    void EncodeIsSafeInteger(Reg dst, Reg src) override;
    void EncodeBitCount(Reg dst, Reg src) override;
    void EncodeCountLeadingZeroBits(Reg dst, Reg src) override;
    void EncodeCountTrailingZeroBits(Reg dst, Reg src) override;
    void EncodeCeil([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeFloor([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRint([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeTrunc([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundAway([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRoundToPInf([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeReverseBytes(Reg dst, Reg src) override;
    void EncodeReverseBits(Reg dst, Reg src) override;
    void EncodeFpToBits(Reg dst, Reg src) override;
    void EncodeMoveBitsRaw(Reg dst, Reg src) override;

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signedCompare) override;
    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override;
    bool CanEncodeScale(uint64_t imm, uint32_t size) override;
    bool CanEncodeBitCount() override;

    void EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval) override
    {
        EncodeCompareAndSwap(dst, obj, &offset, val, newval);
    }

    void EncodeCompareAndSwap(Reg dst, Reg addr, Reg val, Reg newval) override
    {
        EncodeCompareAndSwap(dst, addr, nullptr, val, newval);
    }

    void EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val) override;
    void EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp) override;
    void EncodeMemoryBarrier(memory_order::Order order) override;

    void EncodeStackOverflowCheck(ssize_t offset) override;

    size_t GetCursorOffset() const override
    {
        // NOLINTNEXTLINE(readability-identifier-naming)
        return GetMasm()->offset();
    }

    void SetCursorOffset(size_t offset) override
    {
        // NOLINTNEXTLINE(readability-identifier-naming)
        GetMasm()->setOffset(offset);
    }

    Reg AcquireScratchRegister(TypeInfo type) override
    {
        return (static_cast<Amd64RegisterDescription *>(GetRegfile()))->AcquireScratchRegister(type);
    }

    void AcquireScratchRegister(Reg reg) override
    {
        (static_cast<Amd64RegisterDescription *>(GetRegfile()))->AcquireScratchRegister(reg);
    }

    void ReleaseScratchRegister(Reg reg) override
    {
        (static_cast<Amd64RegisterDescription *>(GetRegfile()))->ReleaseScratchRegister(reg);
    }

    bool IsScratchRegisterReleased(Reg reg) override
    {
        return (static_cast<Amd64RegisterDescription *>(GetRegfile()))->IsScratchRegisterReleased(reg);
    }

    RegMask GetScratchRegistersMask() const override
    {
        return (static_cast<const Amd64RegisterDescription *>(GetRegfile()))->GetScratchRegistersMask();
    }

    RegMask GetScratchFpRegistersMask() const override
    {
        return (static_cast<const Amd64RegisterDescription *>(GetRegfile()))->GetScratchFpRegistersMask();
    }

    RegMask GetAvailableScratchRegisters() const override
    {
        auto regfile = static_cast<const Amd64RegisterDescription *>(GetRegfile());
        return RegMask(regfile->GetScratchRegisters().GetMask());
    }

    VRegMask GetAvailableScratchFpRegisters() const override
    {
        auto regfile = static_cast<const Amd64RegisterDescription *>(GetRegfile());
        return VRegMask(regfile->GetScratchFPRegisters().GetMask());
    }

    TypeInfo GetRefType() override
    {
        return INT64_TYPE;
    };

    size_t DisasmInstr(std::ostream &stream, size_t pc, ssize_t codeOffset) const override;

    void *BufferData() const override
    {
        // NOLINTNEXTLINE(readability-identifier-naming)
        return GetMasm()->bufferData();
    };

    size_t BufferSize() const override
    {
        // NOLINTNEXTLINE(readability-identifier-naming)
        return GetMasm()->offset();
    };

    bool InitMasm() override;

    void Finalize() override;

    void MakeCall(compiler::RelocationInfo *relocation) override;
    void MakeCall(LabelHolder::LabelId id) override;
    void MakeCall(const void *entryPoint) override;
    void MakeCall(Reg reg) override;
    void MakeCall(MemRef entryPoint) override;

    void MakeCallAot(intptr_t offset) override;
    void MakeCallByOffset(intptr_t offset) override;
    void MakeLoadAotTable(intptr_t offset, Reg reg) override;
    void MakeLoadAotTableAddr(intptr_t offset, Reg addr, Reg val) override;
    bool CanMakeCallByOffset(intptr_t offset) override;

    // Encode unconditional branch
    void EncodeJump(LabelHolder::LabelId id) override;

    // Encode jump with compare to zero
    void EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc) override;

    // Compare reg and immediate and branch
    void EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc) override;

    // Compare two regs and branch
    void EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc) override;

    // Compare reg and immediate and branch
    void EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc) override;

    // Compare two regs and branch
    void EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc) override;

    // Encode jump by register value
    void EncodeJump(Reg dst) override;

    void EncodeJump(RelocationInfo *relocation) override;

    void EncodeBitTestAndBranch(LabelHolder::LabelId id, compiler::Reg reg, uint32_t bitPos, bool bitValue) override;

    void EncodeAbort() override;

    void EncodeReturn() override;

    void MakeLibCall(Reg dst, Reg src0, Reg src1, void *entryPoint);

    void SaveRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override
    {
        LoadStoreRegisters<true>(registers, slot, startReg, isFp);
    }
    void LoadRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp) override
    {
        LoadStoreRegisters<false>(registers, slot, startReg, isFp);
    }
    void SaveRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override
    {
        LoadStoreRegisters<true>(registers, isFp, slot, base, mask);
    }
    void LoadRegisters(RegMask registers, bool isFp, ssize_t slot, Reg base, RegMask mask) override
    {
        LoadStoreRegisters<false>(registers, isFp, slot, base, mask);
    }

    void PushRegisters(RegMask registers, bool isFp) override;
    void PopRegisters(RegMask registers, bool isFp) override;

    template <typename Func>
    void EncodeRelativePcMov(Reg reg, intptr_t offset, Func encodeInstruction);

    asmjit::x86::Assembler *GetMasm() const
    {
        ASSERT(masm_ != nullptr);
        return masm_;
    }

    size_t GetLabelAddress(LabelHolder::LabelId label) override
    {
        auto code = GetMasm()->code();
        ASSERT(code->isLabelBound(label));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return code->baseAddress() + code->labelOffset(label);
    }

    bool LabelHasLinks(LabelHolder::LabelId label) override
    {
        auto code = GetMasm()->code();
        auto entry = code->labelEntry(label);
        return entry->links() != nullptr;
    }

private:
    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp);

    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask);

    inline Reg MakeShift(Shift shift);

    template <typename T>
    void EncodeReverseBitsImpl(Reg dst0, Reg src0);

    void EncodeRoundToPInfFloat(Reg dst, Reg src);
    void EncodeRoundToPInfDouble(Reg dst, Reg src);

    void EncodeCastFloatToScalar(Reg dst, bool dstSigned, Reg src);
    inline void EncodeCastFloatSignCheckRange(Reg dst, Reg src, const asmjit::Label &end);
    inline void EncodeCastFloatUnsignCheckRange(Reg dst, Reg src, const asmjit::Label &end);
    void EncodeCastFloatCheckNan(Reg dst, Reg src, const asmjit::Label &end);
    void EncodeCastFloatCheckRange(Reg dst, Reg src, const asmjit::Label &end, int64_t minValue, uint64_t maxValue);
    void EncodeCastFloat32ToUint64(Reg dst, Reg src);
    void EncodeCastFloat64ToUint64(Reg dst, Reg src);

    void EncodeCastScalarToFloat(Reg dst, Reg src, bool srcSigned);
    void EncodeCastScalarToFloatUnsignDouble(Reg dst, Reg src);
    void EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned);

    void EncodeDivFloat(Reg dst, Reg src0, Reg src1);
    void EncodeModFloat(Reg dst, Reg src0, Reg src1);
    template <bool IS_MAX>
    void EncodeMinMaxFp(Reg dst, Reg src0, Reg src1);

    template <typename T, size_t N>
    void CopyArrayToXmm(Reg xmm, const std::array<T, N> &arr);

    template <typename T>
    void CopyImmToXmm(Reg xmm, T imm);

    void EncodeCompareAndSwap(Reg dst, Reg obj, const Reg *offset, Reg val, Reg newval);
    void EncodeCmpFracWithDelta(Reg src);

private:
    Amd64LabelHolder *labels_ {nullptr};
    asmjit::ErrorHandler *errorHandler_ {nullptr};
    asmjit::CodeHolder *codeHolder_ {nullptr};
    asmjit::x86::Assembler *masm_ {nullptr};
};  // Amd64Encoder

class Amd64ParameterInfo : public ParameterInfo {
public:
    std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo &type) override;
    Location GetNextLocation(DataType::Type type) override;
};

class Amd64CallingConvention : public CallingConvention {
public:
    Amd64CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr, CallConvMode mode);
    NO_MOVE_SEMANTIC(Amd64CallingConvention);
    NO_COPY_SEMANTIC(Amd64CallingConvention);
    ~Amd64CallingConvention() override = default;

    static constexpr auto GetTarget()
    {
        return panda::compiler::Target(Arch::X86_64);
    }

    bool IsValid() const override
    {
        return true;
    }

    void GeneratePrologue(const FrameInfo &frameInfo) override;
    void GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateNativePrologue(const FrameInfo &frameInfo) override
    {
        GeneratePrologue(frameInfo);
    }
    void GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override
    {
        GenerateEpilogue(frameInfo, postJob);
    }

    void *GetCodeEntry() override;
    uint32_t GetCodeSize() override;

    // Pushes regs and returns number of regs(from boths vectors)
    size_t PushRegs(RegList regs, RegList vregs);
    // Pops regs and returns number of regs(from boths vectors)
    size_t PopRegs(RegList regs, RegList vregs);

    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regsOffset) override;

    asmjit::x86::Assembler *GetMasm()
    {
        return (static_cast<Amd64Encoder *>(GetEncoder()))->GetMasm();
    }

private:
};  // Amd64CallingConvention
}  // namespace panda::compiler::amd64

#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_AMD64_TARGET_H
