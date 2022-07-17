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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H_
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H_

#include "operands.h"
#include "encode.h"
#include "callconv.h"
#include "target_info.h"

#ifndef USE_VIXL_ARM64
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

/**
 * Converters
 */
static inline vixl::aarch64::Condition Convert(const Condition CC)
{
    switch (CC) {
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
        // TODO(igorban) : Remove them
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

static inline vixl::aarch64::Condition ConvertTest(const Condition CC)
{
    ASSERT(CC == Condition::TST_EQ || CC == Condition::TST_NE);
    return CC == Condition::TST_EQ ? vixl::aarch64::Condition::eq : vixl::aarch64::Condition::ne;
}

static inline vixl::aarch64::Shift Convert(const ShiftType TYPE)
{
    switch (TYPE) {
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
        size_t reg_size = reg.GetSize();
        if (reg_size < WORD_SIZE) {
            reg_size = WORD_SIZE;
        }
        if (reg_size > DOUBLE_WORD_SIZE) {
            reg_size = DOUBLE_WORD_SIZE;
        }
        auto vixl_reg = vixl::aarch64::Register(reg.GetId(), reg_size);
        ASSERT(vixl_reg.IsValid());
        return vixl_reg;
    }
    if (reg.GetId() == vixl::aarch64::sp.GetCode()) {
        return vixl::aarch64::sp;
    }

    // Invalid register type
    UNREACHABLE();
    return vixl::aarch64::xzr;
}

static inline vixl::aarch64::Register VixlReg(Reg reg, const uint8_t SIZE)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        auto vixl_reg = vixl::aarch64::Register(reg.GetId(), (SIZE < WORD_SIZE ? WORD_SIZE : SIZE));
        ASSERT(vixl_reg.IsValid());
        return vixl_reg;
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
        auto vixl_reg = vixl::aarch64::Register(reg.GetId() + 1, DOUBLE_WORD_SIZE);
        ASSERT(vixl_reg.IsValid());
        return vixl_reg;
    }

    // Invalid register type
    UNREACHABLE();
    return vixl::aarch64::xzr;
}

static inline vixl::aarch64::VRegister VixlVReg(Reg reg)
{
    ASSERT(reg.IsValid());
    auto vixl_vreg = vixl::aarch64::VRegister(reg.GetId(), reg.GetSize());
    ASSERT(vixl_vreg.IsValid());
    return vixl_vreg;
}

static inline vixl::aarch64::Operand VixlShift(Shift shift)
{
    Reg reg = shift.GetBase();
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        ASSERT(reg.IsScalar());
        size_t reg_size = reg.GetSize();
        if (reg_size < WORD_SIZE) {
            reg_size = WORD_SIZE;
        }
        auto vixl_reg = vixl::aarch64::Register(reg.GetId(), reg_size);
        ASSERT(vixl_reg.IsValid());

        return vixl::aarch64::Operand(vixl_reg, Convert(shift.GetType()), shift.GetScale());
    }

    // Invalid register type
    UNREACHABLE();
}

static inline vixl::aarch64::Operand VixlImm(const int64_t IMM)
{
    return vixl::aarch64::Operand(IMM);
}

static inline vixl::aarch64::Operand VixlImm(Imm imm)
{
    ASSERT(imm.IsValid());
    if (imm.GetType() == INT64_TYPE) {
        return vixl::aarch64::Operand(imm.GetValue<int64_t>());
    }
    if (imm.GetType() == INT32_TYPE) {
        return vixl::aarch64::Operand(imm.GetValue<int32_t>());
    }
    if (imm.GetType() == INT16_TYPE) {
        return vixl::aarch64::Operand(imm.GetValue<int16_t>());
    }
    if (imm.GetType() == INT8_TYPE) {
        return vixl::aarch64::Operand(imm.GetValue<int8_t>());
    }
    // Invalid converted register
    UNREACHABLE();
    return vixl::aarch64::Operand(imm.GetValue<int8_t>());
}

static inline vixl::aarch64::MemOperand ConvertMem(MemRef mem)
{
    bool base = mem.HasBase() && (mem.GetBase().GetId() != vixl::aarch64::xzr.GetCode());
    bool has_index = mem.HasIndex();
    bool shift = mem.HasScale();
    bool offset = mem.HasDisp();
    auto base_reg = Reg(mem.GetBase().GetId(), INT64_TYPE);
    if (base && !has_index && !shift) {
        // Default memory - base + offset
        if (mem.GetDisp() != 0) {
            auto disp = mem.GetDisp();
            return vixl::aarch64::MemOperand(VixlReg(base_reg), VixlImm(disp));
        }
        return vixl::aarch64::MemOperand(VixlReg(mem.GetBase(), DOUBLE_WORD_SIZE));
    }
    if (base && has_index && !offset) {
        auto scale = mem.GetScale();
        auto index_reg = mem.GetIndex();
        if (index_reg.GetSize() == WORD_SIZE) {
            // Unsign-extend and shift w-register in offset-position
            return vixl::aarch64::MemOperand(VixlReg(base_reg), VixlReg(index_reg), vixl::aarch64::Extend::UXTW, scale);
        }
        if (scale != 0) {
            ASSERT(index_reg.GetSize() == DOUBLE_WORD_SIZE);
            return vixl::aarch64::MemOperand(VixlReg(base_reg), VixlReg(index_reg), vixl::aarch64::LSL, scale);
        }
        return vixl::aarch64::MemOperand(VixlReg(base_reg), VixlReg(index_reg));
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
        return RegMask(caller_saved_.GetList());
    }

    VRegMask GetCallerSavedVRegMask() const override
    {
        return VRegMask(caller_savedv_.GetList());
    }

    bool IsCalleeRegister(Reg reg) override
    {
        bool is_fp = reg.IsFloat();
        return reg.GetId() >= GetFirstCalleeReg(Arch::AARCH64, is_fp) &&
               reg.GetId() <= GetLastCalleeReg(Arch::AARCH64, is_fp);
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
        RegMask reg_mask = compiler::arch_info::arm64::TEMP_REGS;
        reg_mask.set(Target(Arch::AARCH64).GetZeroReg().GetId());
        reg_mask.set(GetThreadReg(Arch::AARCH64));
        reg_mask.set(vixl::aarch64::x29.GetCode());
        reg_mask.set(vixl::aarch64::lr.GetCode());
        return reg_mask;
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

    bool IsRegUsed(ArenaVector<Reg> vec_reg, Reg reg) override;

public:
    // Special implementation-specific getters
    vixl::aarch64::CPURegList GetCalleeSavedR()
    {
        return callee_saved_;
    }
    vixl::aarch64::CPURegList GetCalleeSavedV()
    {
        return callee_savedv_;
    }
    vixl::aarch64::CPURegList GetCallerSavedR()
    {
        return caller_saved_;
    }
    vixl::aarch64::CPURegList GetCallerSavedV()
    {
        return caller_savedv_;
    }
    uint8_t GetAlignmentVreg(bool is_callee)
    {
        auto allignment_vreg = is_callee ? allignment_vreg_callee_ : allignment_vreg_caller_;
        // !TODO Ishin Pavel fix if allignment_vreg == UNDEF_VREG
        ASSERT(allignment_vreg != UNDEF_VREG);

        return allignment_vreg;
    }

private:
    ArenaVector<Reg> used_regs_;

    vixl::aarch64::CPURegList callee_saved_ {vixl::aarch64::kCalleeSaved};
    vixl::aarch64::CPURegList caller_saved_ {vixl::aarch64::kCallerSaved};

    vixl::aarch64::CPURegList callee_savedv_ {vixl::aarch64::kCalleeSavedV};
    vixl::aarch64::CPURegList caller_savedv_ {vixl::aarch64::kCallerSavedV};

    static inline constexpr const uint8_t UNDEF_VREG = std::numeric_limits<uint8_t>::max();
    // The number of register in Push/Pop list must be even. The regisers are used for alignment vetor register lists
    uint8_t allignment_vreg_callee_ {UNDEF_VREG};
    uint8_t allignment_vreg_caller_ {UNDEF_VREG};
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

    void LoadPcRelative(Reg reg, intptr_t offset, Reg reg_addr = INVALID_REGISTER);

    void SetMaxAllocatedBytes(size_t size) override
    {
        GetMasm()->GetBuffer()->SetMmapMaxBytes(size);
    }

#ifndef PANDA_MINIMAL_VIXL
    auto &GetDecoder() const;
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UnaryOperation(opc) void Encode##opc(Reg dst, Reg src0) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperationReg(opc) void Encode##opc(Reg dst, Reg src0, Reg src1) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperationImm(opc) void Encode##opc(Reg dst, Reg src0, Imm src1) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperation(opc) BinaryOperationReg(opc) BinaryOperationImm(opc)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, TYPE) TYPE(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)

#undef UnaryOperation
#undef BinaryOperation
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

    void EncodeCast(Reg dst, bool dst_signed, Reg src, bool src_signed) override;
    void EncodeCastToBool(Reg dst, Reg src) override;

    void EncodeMin(Reg dst, bool dst_signed, Reg src0, Reg src1) override;
    void EncodeDiv(Reg dst, bool dst_signed, Reg src0, Reg src1) override;
    void EncodeMod(Reg dst, bool dst_signed, Reg src0, Reg src1) override;
    void EncodeMax(Reg dst, bool dst_signed, Reg src0, Reg src1) override;

    void EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc) override;

    void EncodeLdr(Reg dst, bool dst_signed, MemRef mem) override;
    void EncodeLdrAcquire(Reg dst, bool dst_signed, MemRef mem) override;
    void EncodeLdrAcquireInvalid(Reg dst, bool dst_signed, MemRef mem);
    void EncodeLdrAcquireScalar(Reg dst, bool dst_signed, MemRef mem);

    void EncodeMov(Reg dst, Imm src) override;
    void EncodeStr(Reg src, MemRef mem) override;
    void EncodeStrRelease(Reg src, MemRef mem) override;

    void EncodeLdrExclusive(Reg dst, Reg addr, bool acquire) override;
    void EncodeStrExclusive(Reg dst, Reg src, Reg addr, bool release) override;

    // zerod high part: [reg.size, 64)
    void EncodeStrz(Reg src, MemRef mem) override;
    void EncodeSti(Imm src, MemRef mem) override;
    // size must be 8, 16,32 or 64
    void EncodeMemCopy(MemRef mem_from, MemRef mem_to, size_t size) override;
    // size must be 8, 16,32 or 64
    // zerod high part: [reg.size, 64)
    void EncodeMemCopyz(MemRef mem_from, MemRef mem_to, size_t size) override;

    void EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc) override;

    void EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc) override;
    void EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc) override;

    void EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc) override;
    void EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc) override;
    void EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc) override;
    void EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc) override;

    void EncodeLdp(Reg dst0, Reg dst1, bool dst_signed, MemRef mem) override;

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
    void EncodeBitCount(Reg dst, Reg src) override;
    void EncodeCountLeadingZeroBits(Reg dst, Reg src) override;
    void EncodeCountTrailingZeroBits(Reg dst, Reg src) override;
    void EncodeCeil([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeFloor([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRint([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRound([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;

    void EncodeStringEquals(Reg dst, Reg str1, Reg str2, bool compression, uint32_t length_offset,
                            uint32_t data_offset) override;

    void EncodeStringIndexOfAfter(Reg dst, Reg str, Reg character, Reg idx, Reg tmp, bool compression,
                                  uint32_t length_offset, uint32_t data_offset, int32_t char_const_value) override;

    void EncodeReverseBytes(Reg dst, Reg src) override;
    void EncodeReverseBits(Reg dst, Reg src) override;
    void EncodeRotate(Reg dst, Reg src1, Reg src2, bool is_ror) override;
    void EncodeSignum(Reg dst, Reg src) override;
    void EncodeCompressedStringCharAt(Reg dst, Reg str, Reg idx, Reg length, Reg tmp, size_t data_offset,
                                      uint32_t shift) override;
    void EncodeCompressedStringCharAtI(Reg dst, Reg str, Reg length, size_t data_offset, uint32_t index,
                                       uint32_t shift) override;

    void EncodeFpToBits(Reg dst, Reg src) override;
    void EncodeMoveBitsRaw(Reg dst, Reg src) override;
    void EncodeGetTypeSize(Reg size, Reg type) override;

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signed_compare) override;
    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override;
    bool CanEncodeScale(uint64_t imm, uint32_t size) override;

    void EncodeCompareAndSwap(Reg dst, Reg obj, Reg offset, Reg val, Reg newval) override;
    void EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val) override;
    void EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, Reg tmp) override;
    void EncodeMemoryBarrier(MemoryOrder::Order order) override;

    void EncodeStackOverflowCheck(ssize_t offset) override;
    void EncodeCrc32Update(Reg dst, Reg crc_reg, Reg val_reg) override;

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
    bool CanEncodeShiftedOperand(ShiftOpcode opcode, ShiftType shift_type) override;

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

    size_t DisasmInstr(std::ostream &stream, size_t pc, ssize_t code_offset) const override;

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
    void MakeCall(const void *entry_point) override;
    void MakeCall(MemRef entry_point) override;
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

    void EncodeBitTestAndBranch(LabelHolder::LabelId id, compiler::Reg reg, uint32_t bit_pos, bool bit_value) override;

    void EncodeAbort() override;

    void EncodeReturn() override;

    void MakeLibCall(Reg dst, Reg src0, Reg src1, const void *entry_point);

    void SaveRegisters(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp) override
    {
        LoadStoreRegisters<true>(registers, slot, start_reg, is_fp);
    }
    void LoadRegisters(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp) override
    {
        LoadStoreRegisters<false>(registers, slot, start_reg, is_fp);
    }

    void SaveRegisters(RegMask registers, bool is_fp, ssize_t slot, Reg base, RegMask mask) override
    {
        LoadStoreRegisters<true>(registers, is_fp, slot, base, mask);
    }
    void LoadRegisters(RegMask registers, bool is_fp, ssize_t slot, Reg base, RegMask mask) override
    {
        LoadStoreRegisters<false>(registers, is_fp, slot, base, mask);
    }

    void PushRegisters(RegMask registers, bool is_fp, bool align) override;
    void PopRegisters(RegMask registers, bool is_fp, bool align) override;

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
    template <bool is_store>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp);

    template <bool is_store>
    void LoadStoreRegistersLoop(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp,
                                const vixl::aarch64::Register &base_reg);

    template <bool is_store>
    void LoadStoreRegisters(RegMask registers, bool is_fp, int32_t slot, Reg base, RegMask mask);

    void EncodeCastFloat(Reg dst, bool dst_signed, Reg src, bool src_signed);
    // This function not used, but it is working and can be used.
    // Unlike "EncodeCastFloat", it implements castes float32/64 to int8/16.
    void EncodeCastFloatWithSmallDst(Reg dst, bool dst_signed, Reg src, bool src_signed);

    void EncodeCastScalar(Reg dst, bool dst_signed, Reg src, bool src_signed);

    void EncodeCastSigned(Reg dst, Reg src);
    void EncodeCastUnsigned(Reg dst, Reg src);

    void EncodeCastCheckNaN(Reg dst, Reg src, LabelHolder::LabelId exit_id);

    // helpers to split generation logic
    void IndexOfHandleLatin1Case(Reg str, Reg character, Reg idx, Reg tmp, bool compression, uint32_t data_offset,
                                 const vixl::aarch64::Register &tmp1, const vixl::aarch64::Register &tmp2,
                                 const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
                                 vixl::aarch64::Label *label_not_found);
    void IndexOfHandleUtf16NormalCase(Reg str, Reg character, Reg idx, Reg tmp, bool compression, uint32_t data_offset,
                                      const vixl::aarch64::Register &tmp1, const vixl::aarch64::Register &tmp2,
                                      const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
                                      vixl::aarch64::Label *label_not_found);
    void IndexOfHandleSurrogateCase(Reg str, Reg character, Reg idx, Reg tmp, bool compression, uint32_t data_offset,
                                    const vixl::aarch64::Register &tmp1, const vixl::aarch64::Register &tmp2,
                                    const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
                                    vixl::aarch64::Label *label_not_found);

    void EncodeStringEqualsMainLoop(Reg dst, Reg str1, Reg str2, Reg tmp1_scoped, Reg tmp2_scoped, Reg tmp3_scoped,
                                    vixl::aarch64::Label *label_false, vixl::aarch64::Label *label_cset,
                                    uint32_t data_offset);

    void IndexOfHandleLatin1CaseMainLoop(Reg str, Reg character, Reg tmp, uint32_t data_offset,
                                         const vixl::aarch64::Register &tmp1, const vixl::aarch64::Register &tmp2,
                                         const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
                                         vixl::aarch64::Label *label_not_found, vixl::aarch64::Label *label_small_loop);

    void IndexOfHandleUtf16NormalCaseMainLoop(Reg str, Reg character, Reg tmp, uint32_t data_offset,
                                              const vixl::aarch64::Register &tmp1, const vixl::aarch64::Register &tmp2,
                                              const vixl::aarch64::Register &tmp3, vixl::aarch64::Label *label_found,
                                              vixl::aarch64::Label *label_not_found,
                                              vixl::aarch64::Label *label_small_loop);

    void EncodeStringIndexOfAfterMainCase(Reg dst, Reg str, Reg character, Reg idx, Reg tmp,
                                          const vixl::aarch64::Register &tmp1, const vixl::aarch64::Register &tmp2,
                                          const vixl::aarch64::Register &tmp3, bool compression, uint32_t data_offset,
                                          int32_t char_const_value, vixl::aarch64::Label *label_not_found);
    void EncodeFMod(Reg dst, Reg src0, Reg src1);
    void HandleChar(int32_t ch, const vixl::aarch64::Register &tmp, vixl::aarch64::Label *label_not_found,
                    vixl::aarch64::Label *label_uncompressed_string);

private:
    Aarch64LabelHolder *labels_ {nullptr};
    vixl::aarch64::MacroAssembler *masm_ {nullptr};
#ifndef PANDA_MINIMAL_VIXL
    mutable vixl::aarch64::Decoder *decoder_ {nullptr};
#endif
    bool lr_acquired_ {false};
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

    void GeneratePrologue(const FrameInfo &frame_info) override;
    void GenerateEpilogue(const FrameInfo &frame_info, std::function<void()> post_job) override;
    void GenerateNativePrologue(const FrameInfo &frame_info) override;
    void GenerateNativeEpilogue(const FrameInfo &frame_info, std::function<void()> post_job) override;

    void *GetCodeEntry() override;
    uint32_t GetCodeSize() override;

    Reg InitFlagsReg(bool has_float_regs);

    // Pushes regs and returns number of regs(from boths vectos)
    size_t PushRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs, bool is_callee);
    // Pops regs and returns number of regs(from boths vectos)
    size_t PopRegs(vixl::aarch64::CPURegList regs, vixl::aarch64::CPURegList vregs, bool is_callee);

    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regs_offset) override;

    vixl::aarch64::MacroAssembler *GetMasm()
    {
        return (static_cast<Aarch64Encoder *>(GetEncoder()))->GetMasm();
    }
};  // Aarch64CallingConvention
}  // namespace panda::compiler::aarch64
#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH64_TARGET_H_
