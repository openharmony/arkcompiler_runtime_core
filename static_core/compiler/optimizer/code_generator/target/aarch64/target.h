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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H

#include "operands.h"
#include "encode.h"
#include "callconv.h"
#include "target_info.h"

#ifdef USE_VIXL_ARM64
#include "aarch64/constants-aarch64.h"
#include "aarch64/macro-assembler-aarch64.h"
#include "aarch64/operands-aarch64.h"
#else
#error "Wrong build type, please add VIXL in build"
#endif  // USE_VIXL_ARM64

namespace panda::compiler::aarch64 {
// Ensure that vixl has same callee regs as our arch util
static constexpr auto CALLEE_REG_LIST =
    vixl::aarch64::CPURegList(vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kXRegSize,
                              GetFirstCalleeReg(Arch::AARCH64, false), GetLastCalleeReg(Arch::AARCH64, false));
static constexpr auto CALLEE_VREG_LIST =
    vixl::aarch64::CPURegList(vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kDRegSize,
                              GetFirstCalleeReg(Arch::AARCH64, true), GetLastCalleeReg(Arch::AARCH64, true));
static constexpr auto CALLER_REG_LIST =
    vixl::aarch64::CPURegList(vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kXRegSize,
                              GetCallerRegsMask(Arch::AARCH64, false).GetValue());
static constexpr auto CALLER_VREG_LIST = vixl::aarch64::CPURegList(
    vixl::aarch64::CPURegister::kRegister, vixl::aarch64::kXRegSize, GetCallerRegsMask(Arch::AARCH64, true).GetValue());

static_assert(vixl::aarch64::kCalleeSaved.GetList() == CALLEE_REG_LIST.GetList());
static_assert(vixl::aarch64::kCalleeSavedV.GetList() == CALLEE_VREG_LIST.GetList());
static_assert(vixl::aarch64::kCallerSaved.GetList() == CALLER_REG_LIST.GetList());
static_assert(vixl::aarch64::kCallerSavedV.GetList() == CALLER_VREG_LIST.GetList());

const size_t MAX_SCALAR_PARAM_ID = 7;  // r0-r7
const size_t MAX_VECTOR_PARAM_ID = 7;  // v0-v7

/// Converters
static inline vixl::aarch64::Condition Convert(const Condition cc)
{
    switch (cc) {
        case Condition::EQ:
            return vixl::aarch64::Condition::eq;
        case Condition::NE:
            return vixl::aarch64::Condition::ne;
        case Condition::LT:
            return vixl::aarch64::Condition::lt;
        case Condition::GT:
            return vixl::aarch64::Condition::gt;
        case Condition::LE:
            return vixl::aarch64::Condition::le;
        case Condition::GE:
            return vixl::aarch64::Condition::ge;
        case Condition::LO:
            return vixl::aarch64::Condition::lo;
        case Condition::LS:
            return vixl::aarch64::Condition::ls;
        case Condition::HI:
            return vixl::aarch64::Condition::hi;
        case Condition::HS:
            return vixl::aarch64::Condition::hs;
        // NOTE(igorban) : Remove them
        case Condition::MI:
            return vixl::aarch64::Condition::mi;
        case Condition::PL:
            return vixl::aarch64::Condition::pl;
        case Condition::VS:
            return vixl::aarch64::Condition::vs;
        case Condition::VC:
            return vixl::aarch64::Condition::vc;
        case Condition::AL:
            return vixl::aarch64::Condition::al;
        case Condition::NV:
            return vixl::aarch64::Condition::nv;
        default:
            UNREACHABLE();
            return vixl::aarch64::Condition::eq;
    }
}

static inline vixl::aarch64::Condition ConvertTest(const Condition cc)
{
    ASSERT(cc == Condition::TST_EQ || cc == Condition::TST_NE);
    return cc == Condition::TST_EQ ? vixl::aarch64::Condition::eq : vixl::aarch64::Condition::ne;
}

static inline vixl::aarch64::Shift Convert(const ShiftType type)
{
    switch (type) {
        case ShiftType::LSL:
            return vixl::aarch64::Shift::LSL;
        case ShiftType::LSR:
            return vixl::aarch64::Shift::LSR;
        case ShiftType::ASR:
            return vixl::aarch64::Shift::ASR;
        case ShiftType::ROR:
            return vixl::aarch64::Shift::ROR;
        default:
            UNREACHABLE();
    }
}

static inline vixl::aarch64::Register VixlReg(Reg reg)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        size_t regSize = reg.GetSize();
        if (regSize < WORD_SIZE) {
            regSize = WORD_SIZE;
        }
        if (regSize > DOUBLE_WORD_SIZE) {
            regSize = DOUBLE_WORD_SIZE;
        }
        auto vixlReg = vixl::aarch64::Register(reg.GetId(), regSize);
        ASSERT(vixlReg.IsValid());
        return vixlReg;
    }
    if (reg.GetId() == vixl::aarch64::sp.GetCode()) {
        return vixl::aarch64::sp;
    }

    // Invalid register type
    UNREACHABLE();
    return vixl::aarch64::xzr;
}

static inline vixl::aarch64::Register VixlReg(Reg reg, const uint8_t size)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        auto vixlReg = vixl::aarch64::Register(reg.GetId(), (size < WORD_SIZE ? WORD_SIZE : size));
        ASSERT(vixlReg.IsValid());
        return vixlReg;
    }
    if (reg.GetId() == vixl::aarch64::sp.GetCode()) {
        return vixl::aarch64::sp;
    }

    // Invalid register type
    UNREACHABLE();
    return vixl::aarch64::xzr;
}

// Upper half-part for 128bit register
static inline vixl::aarch64::Register VixlRegU(Reg reg)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        auto vixlReg = vixl::aarch64::Register(reg.GetId() + 1, DOUBLE_WORD_SIZE);
        ASSERT(vixlReg.IsValid());
        return vixlReg;
    }

    // Invalid register type
    UNREACHABLE();
    return vixl::aarch64::xzr;
}

static inline vixl::aarch64::VRegister VixlVReg(Reg reg)
{
    ASSERT(reg.IsValid());
    auto vixlVreg = vixl::aarch64::VRegister(reg.GetId(), reg.GetSize());
    ASSERT(vixlVreg.IsValid());
    return vixlVreg;
}

static inline vixl::aarch64::Operand VixlShift(Shift shift)
{
    Reg reg = shift.GetBase();
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        ASSERT(reg.IsScalar());
        size_t regSize = reg.GetSize();
        if (regSize < WORD_SIZE) {
            regSize = WORD_SIZE;
        }
        auto vixlReg = vixl::aarch64::Register(reg.GetId(), regSize);
        ASSERT(vixlReg.IsValid());

        return vixl::aarch64::Operand(vixlReg, Convert(shift.GetType()), shift.GetScale());
    }

    // Invalid register type
    UNREACHABLE();
}

static inline vixl::aarch64::Operand VixlImm(const int64_t imm)
{
    return vixl::aarch64::Operand(imm);
}

static inline vixl::aarch64::Operand VixlImm(Imm imm)
{
    return vixl::aarch64::Operand(imm.GetAsInt());
}

static inline vixl::aarch64::MemOperand ConvertMem(MemRef mem)
{
    bool base = mem.HasBase() && (mem.GetBase().GetId() != vixl::aarch64::xzr.GetCode());
    bool hasIndex = mem.HasIndex();
    bool shift = mem.HasScale();
    bool offset = mem.HasDisp();
    auto baseReg = Reg(mem.GetBase().GetId(), INT64_TYPE);
    if (base && !hasIndex && !shift) {
        // Memory address = x_reg(base) + imm(offset)
        if (mem.GetDisp() != 0) {
            auto disp = mem.GetDisp();
            return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlImm(disp));
        }
        // Memory address = x_reg(base)
        return vixl::aarch64::MemOperand(VixlReg(mem.GetBase(), DOUBLE_WORD_SIZE));
    }
    if (base && hasIndex && !offset) {
        auto scale = mem.GetScale();
        auto indexReg = mem.GetIndex();
        // Memory address = x_reg(base) + (SXTW(w_reg(index)) << scale)
        if (indexReg.GetSize() == WORD_SIZE) {
            // Sign-extend and shift w-register in offset-position (signed because index always has signed type)
            return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlReg(indexReg), vixl::aarch64::Extend::SXTW, scale);
        }
        // Memory address = x_reg(base) + (x_reg(index) << scale)
        if (scale != 0) {
            ASSERT(indexReg.GetSize() == DOUBLE_WORD_SIZE);
            return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlReg(indexReg), vixl::aarch64::LSL, scale);
        }
        // Memory address = x_reg(base) + x_reg(index)
        return vixl::aarch64::MemOperand(VixlReg(baseReg), VixlReg(indexReg));
    }
    // Wrong memRef
    // Return invalid memory operand
    auto tmp = vixl::aarch64::MemOperand();
    ASSERT(!tmp.IsValid());
    return tmp;
}

class Aarch64RegisterDescription final : public RegistersDescription {
public:
    explicit Aarch64RegisterDescription(ArenaAllocator *allocator);

    NO_MOVE_SEMANTIC(Aarch64RegisterDescription);
    NO_COPY_SEMANTIC(Aarch64RegisterDescription);
    ~Aarch64RegisterDescription() override = default;

    ArenaVector<Reg> GetCalleeSaved() override;
    void SetCalleeSaved(const ArenaVector<Reg> &regs) override;
    // Set used regs - change GetCallee
    void SetUsedRegs(const ArenaVector<Reg> &regs) override;

    RegMask GetCallerSavedRegMask() const override
    {
        return RegMask(callerSaved_.GetList());
    }

    VRegMask GetCallerSavedVRegMask() const override
    {
        return VRegMask(callerSavedv_.GetList());
    }

    bool IsCalleeRegister(Reg reg) override
    {
        bool isFp = reg.IsFloat();
        return reg.GetId() >= GetFirstCalleeReg(Arch::AARCH64, isFp) &&
               reg.GetId() <= GetLastCalleeReg(Arch::AARCH64, isFp);
    }

    Reg GetZeroReg() const override
    {
        return Target(Arch::AARCH64).GetZeroReg();
    }

    bool IsZeroReg(Reg reg) const override
    {
        return reg.IsValid() && reg.IsScalar() && reg.GetId() == GetZeroReg().GetId();
    }

    Reg::RegIDType GetTempReg() override
    {
        return compiler::arch_info::arm64::TEMP_REGS.GetMaxRegister();
    }

    Reg::RegIDType GetTempVReg() override
    {
        return compiler::arch_info::arm64::TEMP_FP_REGS.GetMaxRegister();
    }

    RegMask GetDefaultRegMask() const override
    {
        RegMask regMask = compiler::arch_info::arm64::TEMP_REGS;
        regMask.set(Target(Arch::AARCH64).GetZeroReg().GetId());
        regMask.set(GetThreadReg(Arch::AARCH64));
        regMask.set(vixl::aarch64::x29.GetCode());
        regMask.set(vixl::aarch64::lr.GetCode());
        return regMask;
    }

    VRegMask GetVRegMask() override
    {
        return compiler::arch_info::arm64::TEMP_FP_REGS;
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
    vixl::aarch64::CPURegList GetCalleeSavedR()
    {
        return calleeSaved_;
    }
    vixl::aarch64::CPURegList GetCalleeSavedV()
    {
        return calleeSavedv_;
    }
    vixl::aarch64::CPURegList GetCallerSavedR()
    {
        return callerSaved_;
    }
    vixl::aarch64::CPURegList GetCallerSavedV()
    {
        return callerSavedv_;
    }
    uint8_t GetAlignmentVreg(bool isCallee)
    {
        auto allignmentVreg = isCallee ? allignmentVregCallee_ : allignmentVregCaller_;
        // !NOTE Ishin Pavel fix if allignment_vreg == UNDEF_VREG
        ASSERT(allignmentVreg != UNDEF_VREG);

        return allignmentVreg;
    }

private:
    ArenaVector<Reg> usedRegs_;

    vixl::aarch64::CPURegList calleeSaved_ {vixl::aarch64::kCalleeSaved};
    vixl::aarch64::CPURegList callerSaved_ {vixl::aarch64::kCallerSaved};

    vixl::aarch64::CPURegList calleeSavedv_ {vixl::aarch64::kCalleeSavedV};
    vixl::aarch64::CPURegList callerSavedv_ {vixl::aarch64::kCallerSavedV};

    static inline constexpr const uint8_t UNDEF_VREG = std::numeric_limits<uint8_t>::max();
    // The number of register in Push/Pop list must be even. The regisers are used for alignment vetor register lists
    uint8_t allignmentVregCallee_ {UNDEF_VREG};
    uint8_t allignmentVregCaller_ {UNDEF_VREG};
};  // Aarch64RegisterDescription

class Aarch64Encoder;

class Aarch64LabelHolder final : public LabelHolder {
public:
    using LabelType = vixl::aarch64::Label;
    explicit Aarch64LabelHolder(Encoder *enc) : LabelHolder(enc), labels_(enc->GetAllocator()->Adapter()) {};

    NO_MOVE_SEMANTIC(Aarch64LabelHolder);
    NO_COPY_SEMANTIC(Aarch64LabelHolder);
    ~Aarch64LabelHolder() override = default;

    LabelId CreateLabel() override
    {
        ++id_;
        auto allocator = GetEncoder()->GetAllocator();
        auto *label = allocator->New<LabelType>(allocator);
        labels_.push_back(label);
        ASSERT(labels_.size() == id_);
        return id_ - 1;
    };

    void CreateLabels(LabelId size) override
    {
        for (LabelId i = 0; i <= size; ++i) {
            CreateLabel();
        }
    };

    void BindLabel(LabelId id) override;

    LabelType *GetLabel(LabelId id) const
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
    friend Aarch64Encoder;
};  // Aarch64LabelHolder

class Aarch64Encoder final : public Encoder {
public:
    explicit Aarch64Encoder(ArenaAllocator *allocator);

    LabelHolder *GetLabels() const override
    {
        ASSERT(labels_ != nullptr);
        return labels_;
    };

    ~Aarch64Encoder() override;

    NO_COPY_SEMANTIC(Aarch64Encoder);
    NO_MOVE_SEMANTIC(Aarch64Encoder);

    bool IsValid() const override
    {
        return true;
    }

    static constexpr auto GetTarget()
    {
        return panda::compiler::Target(Arch::AARCH64);
    }

    void LoadPcRelative(Reg reg, intptr_t offset, Reg regAddr = INVALID_REGISTER);

    void SetMaxAllocatedBytes(size_t size) override
    {
        GetMasm()->GetBuffer()->SetMmapMaxBytes(size);
    }

#ifndef PANDA_MINIMAL_VIXL
    inline vixl::aarch64::Decoder &GetDecoder() const;
    inline vixl::aarch64::Disassembler &GetDisasm() const;
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNARY_OPERATION(opc) void Encode##opc(Reg dst, Reg src0) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION_REG(opc) void Encode##opc(Reg dst, Reg src0, Reg src1) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION_IMM(opc) void Encode##opc(Reg dst, Reg src0, Imm src1) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION(opc) BINARY_OPERATION_REG(opc) BINARY_OPERATION_IMM(opc)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, TYPE) TYPE(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)

#undef UNARY_OPERATION
#undef BINARY_OPERATION
#undef INST_DEF

    void EncodeNop() override;
    void CheckAlignment(MemRef mem, size_t size);

    // Additional special instructions
    void EncodeAdd(Reg dst, Reg src0, Shift src1) override;
    void EncodeSub(Reg dst, Reg src0, Shift src1) override;
    void EncodeAnd(Reg dst, Reg src0, Shift src1) override;
    void EncodeOr(Reg dst, Reg src0, Shift src1) override;
    void EncodeXor(Reg dst, Reg src0, Shift src1) override;
    void EncodeOrNot(Reg dst, Reg src0, Shift src1) override;
    void EncodeAndNot(Reg dst, Reg src0, Shift src1) override;
    void EncodeXorNot(Reg dst, Reg src0, Shift src1) override;
    void EncodeNeg(Reg dst, Shift src) override;

    void EncodeCast(Reg dst, bool dstSigned, Reg src, bool srcSigned) override;
    void EncodeFastPathDynamicCast(Reg dst, Reg src, LabelHolder::LabelId slow) override;
    void EncodeCastToBool(Reg dst, Reg src) override;

    void EncodeMin(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeDiv(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeMod(Reg dst, bool dstSigned, Reg src0, Reg src1) override;
    void EncodeMax(Reg dst, bool dstSigned, Reg src0, Reg src1) override;

    void EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeNegOverflowAndZero(compiler::LabelHolder::LabelId id, Reg dst, Reg src) override;

    void EncodeLdr(Reg dst, bool dstSigned, MemRef mem) override;
    void EncodeLdrAcquire(Reg dst, bool dstSigned, MemRef mem) override;
    void EncodeLdrAcquireInvalid(Reg dst, bool dstSigned, MemRef mem);
    void EncodeLdrAcquireScalar(Reg dst, bool dstSigned, MemRef mem);

    void EncodeMov(Reg dst, Imm src) override;
    void EncodeStr(Reg src, MemRef mem) override;
    void EncodeStrRelease(Reg src, MemRef mem) override;

    void EncodeLdrExclusive(Reg dst, Reg addr, bool acquire) override;
    void EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release) override;

    // zerod high part: [reg.size, 64)
    void EncodeStrz(Reg src, MemRef mem) override;
    void EncodeSti(int64_t src, uint8_t srcSizeBytes, MemRef mem) override;
    void EncodeSti(double src, MemRef mem) override;
    void EncodeSti(float src, MemRef mem) override;
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

    void EncodeMAdd(Reg dst, Reg src0, Reg src1, Reg src2) override;
    void EncodeMSub(Reg dst, Reg src0, Reg src1, Reg src2) override;

    void EncodeMNeg(Reg dst, Reg src0, Reg src1) override;
    void EncodeXorNot(Reg dst, Reg src0, Reg src1) override;
    void EncodeAndNot(Reg dst, Reg src0, Reg src1) override;
    void EncodeOrNot(Reg dst, Reg src0, Reg src1) override;

    void EncodeExtractBits(Reg dst, Reg src0, Imm imm1, Imm imm2) override;

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
    void EncodeRotate(Reg dst, Reg src1, Reg src2, bool isRor) override;
    void EncodeSignum(Reg dst, Reg src) override;
    void EncodeCompressedStringCharAt(Reg dst, Reg str, Reg idx, Reg length, Reg tmp, size_t dataOffset,
                                      uint32_t shift) override;
    void EncodeCompressedStringCharAtI(Reg dst, Reg str, Reg length, size_t dataOffset, uint32_t index,
                                       uint32_t shift) override;

    void EncodeFpToBits(Reg dst, Reg src) override;
    void EncodeMoveBitsRaw(Reg dst, Reg src) override;
    void EncodeGetTypeSize(Reg size, Reg type) override;

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signedCompare) override;
    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override;
    bool CanEncodeScale(uint64_t imm, uint32_t size) override;
    bool CanEncodeFloatSelect() override;

    void EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval) override;
    void EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val) override;
    void EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp) override;
    void EncodeMemoryBarrier(memory_order::Order order) override;

    void EncodeStackOverflowCheck(ssize_t offset) override;
    void EncodeCrc32Update(Reg dst, Reg crcReg, Reg valReg) override;
    void EncodeCompressEightUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr) override;
    void EncodeCompressSixteenUtf16ToUtf8CharsUsingSimd(Reg srcAddr, Reg dstAddr) override;

    bool CanEncodeBitCount() override
    {
        return true;
    }

    bool CanEncodeCompressedStringCharAt() override
    {
        return true;
    }

    bool CanEncodeCompressedStringCharAtI() override
    {
        return true;
    }

    bool CanEncodeMAdd() override
    {
        return true;
    }
    bool CanEncodeMSub() override
    {
        return true;
    }
    bool CanEncodeMNeg() override
    {
        return true;
    }
    bool CanEncodeOrNot() override
    {
        return true;
    }
    bool CanEncodeAndNot() override
    {
        return true;
    }
    bool CanEncodeXorNot() override
    {
        return true;
    }
    bool CanEncodeShiftedOperand(ShiftOpcode opcode, ShiftType shiftType) override;

    size_t GetCursorOffset() const override
    {
        return GetMasm()->GetBuffer()->GetCursorOffset();
    }
    void SetCursorOffset(size_t offset) override
    {
        GetMasm()->GetBuffer()->Rewind(offset);
    }

    Reg AcquireScratchRegister(TypeInfo type) override;
    void AcquireScratchRegister(Reg reg) override;
    void ReleaseScratchRegister(Reg reg) override;
    bool IsScratchRegisterReleased(Reg reg) override;

    RegMask GetScratchRegistersMask() const override
    {
        return RegMask(GetMasm()->GetScratchRegisterList()->GetList());
    }

    RegMask GetScratchFpRegistersMask() const override
    {
        return RegMask(GetMasm()->GetScratchVRegisterList()->GetList());
    }

    RegMask GetAvailableScratchRegisters() const override
    {
        return RegMask(GetMasm()->GetScratchRegisterList()->GetList());
    }

    VRegMask GetAvailableScratchFpRegisters() const override
    {
        return VRegMask(GetMasm()->GetScratchVRegisterList()->GetList());
    }

    TypeInfo GetRefType() override
    {
        return INT64_TYPE;
    };

    size_t DisasmInstr(std::ostream &stream, size_t pc, ssize_t codeOffset) const override;

    void *BufferData() const override
    {
        return GetMasm()->GetBuffer()->GetStartAddress<void *>();
    };

    size_t BufferSize() const override
    {
        return GetMasm()->GetBuffer()->GetSizeInBytes();
    };

    bool InitMasm() override;

    void Finalize() override;

    void MakeCall(compiler::RelocationInfo *relocation) override;
    void MakeCall(LabelHolder::LabelId id) override;
    void MakeCall(const void *entryPoint) override;
    void MakeCall(MemRef entryPoint) override;
    void MakeCall(Reg reg) override;

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

    void MakeLibCall(Reg dst, Reg src0, Reg src1, const void *entryPoint);

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

    vixl::aarch64::MacroAssembler *GetMasm() const
    {
        ASSERT(masm_ != nullptr);
        return masm_;
    }

    size_t GetLabelAddress(LabelHolder::LabelId label) override
    {
        auto plabel = labels_->GetLabel(label);
        ASSERT(plabel->IsBound());
        return GetMasm()->GetLabelAddress<size_t>(plabel);
    }

    bool LabelHasLinks(LabelHolder::LabelId label) override
    {
        auto plabel = labels_->GetLabel(label);
        return plabel->IsLinked();
    }

private:
    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, size_t startReg, bool isFp);

    template <bool IS_STORE>
    void LoadStoreRegistersLoop(RegMask registers, ssize_t slot, size_t startReg, bool isFp,
                                const vixl::aarch64::Register &baseReg);

    template <bool IS_STORE>
    void LoadStoreRegisters(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask);

    template <bool IS_STORE>
    void LoadStoreRegistersMainLoop(RegMask registers, bool isFp, int32_t slot, Reg base, RegMask mask);

    void EncodeCastFloat(Reg dst, bool dstSigned, Reg src, bool srcSigned);
    // This function not used, but it is working and can be used.
    // Unlike "EncodeCastFloat", it implements castes float32/64 to int8/16.
    void EncodeCastFloatWithSmallDst(Reg dst, bool dstSigned, Reg src, bool srcSigned);

    void EncodeCastScalar(Reg dst, bool dstSigned, Reg src, bool srcSigned);

    void EncodeCastSigned(Reg dst, Reg src);
    void EncodeCastUnsigned(Reg dst, Reg src);

    void EncodeCastCheckNaN(Reg dst, Reg src, LabelHolder::LabelId exitId);

    void EncodeFMod(Reg dst, Reg src0, Reg src1);
    void HandleChar(int32_t ch, const vixl::aarch64::Register &tmp, vixl::aarch64::Label *labelNotFound,
                    vixl::aarch64::Label *labelUncompressedString);

    void EncodeCmpFracWithDelta(Reg src);

private:
    Aarch64LabelHolder *labels_ {nullptr};
    vixl::aarch64::MacroAssembler *masm_ {nullptr};
#ifndef PANDA_MINIMAL_VIXL
    mutable std::optional<vixl::aarch64::Decoder> decoder_;
    mutable std::optional<vixl::aarch64::Disassembler> disasm_;
#endif
    bool lrAcquired_ {false};
};  // Aarch64Encoder

class Aarch64ParameterInfo : public ParameterInfo {
public:
    std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo &type) override;
    Location GetNextLocation(DataType::Type type) override;
};

class Aarch64CallingConvention : public CallingConvention {
public:
    Aarch64CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr, CallConvMode mode);
    NO_MOVE_SEMANTIC(Aarch64CallingConvention);
    NO_COPY_SEMANTIC(Aarch64CallingConvention);
    ~Aarch64CallingConvention() override = default;

    static constexpr auto GetTarget()
    {
        return panda::compiler::Target(Arch::AARCH64);
    }

    bool IsValid() const override
    {
        return true;
    }

    void GeneratePrologue(const FrameInfo &frameInfo) override;
    void GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;
    void GenerateNativePrologue(const FrameInfo &frameInfo) override;
    void GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob) override;

    void *GetCodeEntry() override;
    uint32_t GetCodeSize() override;

    Reg InitFlagsReg(bool hasFloatRegs);

    // Pushes regs and returns number of regs(from boths vectos)
    size_t PushRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs, bool isCallee);
    // Pops regs and returns number of regs(from boths vectos)
    size_t PopRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs, bool isCallee);

    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regsOffset) override;

    vixl::aarch64::MacroAssembler *GetMasm()
    {
        return (static_cast<Aarch64Encoder *>(GetEncoder()))->GetMasm();
    }
};  // Aarch64CallingConvention
}  // namespace panda::compiler::aarch64
#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H
