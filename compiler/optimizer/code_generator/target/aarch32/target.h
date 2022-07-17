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

#ifndef COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH32_TARGET_H_
#define COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH32_TARGET_H_

#include "operands.h"
#include "encode.h"
#include "callconv.h"
#include "target_info.h"

#ifdef USE_VIXL_ARM32
#include "aarch32/constants-aarch32.h"
#include "aarch32/instructions-aarch32.h"
#include "aarch32/macro-assembler-aarch32.h"
#else
#error "Wrong build type, please add VIXL in build"
#endif  // USE_VIXL_ARM32

namespace panda::compiler::aarch32 {
constexpr uint32_t AVAILABLE_DOUBLE_WORD_REGISTERS = 4;

static inline constexpr const uint8_t UNDEF_REG = std::numeric_limits<uint8_t>::max();

const size_t AARCH32_COUNT_REG = 3;
const size_t AARCH32_COUNT_VREG = 2;

const size_t MAX_SCALAR_PARAM_ID = 3;          // r0-r3
const size_t MAX_VECTOR_SINGLE_PARAM_ID = 15;  // s0-s15
const size_t MAX_VECTOR_DOUBLE_PARAM_ID = 7;   // d0-d7

// Temporary registers used (r12 already used by vixl)
// r11 is used as FP register for frames
const std::array<unsigned, AARCH32_COUNT_REG> AARCH32_TMP_REG = {
    vixl::aarch32::r8.GetCode(), vixl::aarch32::r9.GetCode(), vixl::aarch32::r12.GetCode()};

// Temporary vector registers used
const std::array<unsigned, AARCH32_COUNT_VREG> AARCH32_TMP_VREG = {vixl::aarch32::s14.GetCode(),
                                                                   vixl::aarch32::s15.GetCode()};

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

/**
 * Converters
 */
static inline vixl::aarch32::Condition Convert(const Condition CC)
{
    switch (CC) {
        case Condition::EQ:
            return vixl::aarch32::eq;
        case Condition::NE:
            return vixl::aarch32::ne;
        case Condition::LT:
            return vixl::aarch32::lt;
        case Condition::GT:
            return vixl::aarch32::gt;
        case Condition::LE:
            return vixl::aarch32::le;
        case Condition::GE:
            return vixl::aarch32::ge;
        case Condition::LO:
            return vixl::aarch32::lo;
        case Condition::LS:
            return vixl::aarch32::ls;
        case Condition::HI:
            return vixl::aarch32::hi;
        case Condition::HS:
            return vixl::aarch32::hs;
        // TODO(igorban) : Remove them
        case Condition::MI:
            return vixl::aarch32::mi;
        case Condition::PL:
            return vixl::aarch32::pl;
        case Condition::VS:
            return vixl::aarch32::vs;
        case Condition::VC:
            return vixl::aarch32::vc;
        case Condition::AL:
            return vixl::aarch32::al;
        default:
            UNREACHABLE();
            return vixl::aarch32::eq;
    }
}

/**
 * Converters
 */
static inline vixl::aarch32::Condition ConvertTest(const Condition CC)
{
    ASSERT(CC == Condition::TST_EQ || CC == Condition::TST_NE);
    return CC == Condition::TST_EQ ? vixl::aarch32::eq : vixl::aarch32::ne;
}

static inline vixl::aarch32::Register VixlReg(Reg reg)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        auto vixl_reg = vixl::aarch32::Register(reg.GetId());
        ASSERT(vixl_reg.IsValid());
        return vixl_reg;
    }
    // Unsupported register type
    UNREACHABLE();
    return vixl::aarch32::Register();
}

// Upper half-part for register
static inline vixl::aarch32::Register VixlRegU(Reg reg)
{
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        auto vixl_reg = vixl::aarch32::Register(reg.GetId() + 1);
        ASSERT(reg.GetId() <= AVAILABLE_DOUBLE_WORD_REGISTERS * 2U);
        ASSERT(vixl_reg.IsValid());
        return vixl_reg;
    }
    // Unsupported register type
    UNREACHABLE();
    return vixl::aarch32::Register();
}

static inline vixl::aarch32::VRegister VixlVReg(Reg reg)
{
    ASSERT(reg.IsValid());
    ASSERT(reg.IsFloat());
    if (reg.GetSize() == WORD_SIZE) {
        // Aarch32 Vreg map double regs for 2 single-word registers
        auto vixl_vreg = vixl::aarch32::SRegister(reg.GetId());
        ASSERT(vixl_vreg.IsValid());
        return vixl_vreg;
    }
    ASSERT(reg.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(reg.GetId() % 2U == 0);
    auto vixl_vreg = vixl::aarch32::DRegister(reg.GetId() / 2U);
    ASSERT(vixl_vreg.IsValid());
    return vixl_vreg;
}

static inline vixl::aarch32::Operand VixlImm(const int32_t IMM)
{
    return vixl::aarch32::Operand(IMM);
}

static inline vixl::aarch32::NeonImmediate VixlNeonImm(const float IMM)
{
    return vixl::aarch32::NeonImmediate(IMM);
}

static inline vixl::aarch32::NeonImmediate VixlNeonImm(const double IMM)
{
    return vixl::aarch32::NeonImmediate(IMM);
}

static inline vixl::aarch32::DataType Convert(const TypeInfo INFO, const bool IS_SIGNED = false)
{
    if (!IS_SIGNED) {
        if (INFO.IsFloat()) {
            if (INFO.GetSize() == WORD_SIZE) {
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::F32);
            }
            ASSERT(INFO.GetSize() == DOUBLE_WORD_SIZE);
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::F64);
        }
        switch (INFO.GetSize()) {
            case BYTE_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I8);
            case HALF_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I16);
            case WORD_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I32);
            case DOUBLE_WORD_SIZE:
                return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::I64);
            default:
                break;
        }
    }
    ASSERT(!INFO.IsFloat());

    switch (INFO.GetSize()) {
        case BYTE_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S8);
        case HALF_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S16);
        case WORD_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S32);
        case DOUBLE_WORD_SIZE:
            return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::S64);
        default:
            break;
    }
    return vixl::aarch32::DataType(vixl::aarch32::DataTypeValue::kDataTypeValueInvalid);
}

static inline vixl::aarch32::Operand VixlImm(Imm imm)
{
    ASSERT(imm.IsValid());
    // Unsupported 64-bit values - force cast
    if (imm.GetType() == INT64_TYPE) {
        auto data = static_cast<int32_t>(imm.GetValue<int64_t>());
        return vixl::aarch32::Operand(data);
    }
    if (imm.GetType() == INT32_TYPE) {
        return vixl::aarch32::Operand(imm.GetValue<int32_t>());
    }
    if (imm.GetType() == INT16_TYPE) {
        return vixl::aarch32::Operand(imm.GetValue<int16_t>());
    }
    if (imm.GetType() == INT8_TYPE) {
        return vixl::aarch32::Operand(imm.GetValue<int8_t>());
    }
    if (imm.GetType() == FLOAT32_TYPE) {
        auto data = bit_cast<int32_t>(imm.GetValue<float>());
        return vixl::aarch32::Operand(data);
    }
    if (imm.GetType() == FLOAT64_TYPE) {
        auto data = static_cast<int32_t>(bit_cast<int64_t>(imm.GetValue<double>()));
        return vixl::aarch32::Operand(data);
    }
    // Invalid converted register
    UNREACHABLE();
    return vixl::aarch32::Operand(imm.GetValue<int8_t>());
}

// Upper half for immediate
static inline vixl::aarch32::Operand VixlImmU(Imm imm)
{
    ASSERT(imm.IsValid());
    // Unsupported 64-bit values - force cast
    if (imm.GetType() == INT64_TYPE) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        auto data = static_cast<int32_t>(imm.GetValue<int64_t>() >> WORD_SIZE);
        return vixl::aarch32::Operand(data);
    }
    if (imm.GetType() == FLOAT64_TYPE) {
        auto val = bit_cast<int64_t>(imm.GetValue<double>());
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        auto data = static_cast<int32_t>(val >> WORD_SIZE);
        return vixl::aarch32::Operand(data);
    }

    return vixl::aarch32::Operand(0x0);
}

static inline vixl::aarch32::DOperand VixlDImm(Imm imm)
{
    ASSERT(imm.IsValid());
    if (imm.GetType() == INT64_TYPE) {
        auto data = imm.GetValue<int64_t>();
        return vixl::aarch32::DOperand(data);
    }
    if (imm.GetType() == INT32_TYPE) {
        return vixl::aarch32::DOperand(imm.GetValue<int32_t>());
    }
    if (imm.GetType() == INT16_TYPE) {
        return vixl::aarch32::DOperand(imm.GetValue<int16_t>());
    }
    if (imm.GetType() == INT8_TYPE) {
        return vixl::aarch32::DOperand(imm.GetValue<int8_t>());
    }
    // Invalid converted register
    UNREACHABLE();
    return vixl::aarch32::DOperand(imm.GetValue<int8_t>());
}

class Aarch32RegisterDescription final : public RegistersDescription {
    //                 r4-r10 - "0000011111110000"
    const RegMask CALLEE_SAVED = RegMask(GetCalleeRegsMask(Arch::AARCH32, false));
    //                 s16-s31 - "11111111111111110000000000000000"
    const VRegMask CALLEE_SAVEDV = VRegMask(GetCalleeRegsMask(Arch::AARCH32, true));
    //                 r0-r3 -  "0000000000001111"
    const RegMask CALLER_SAVED = RegMask(GetCallerRegsMask(Arch::AARCH32, false));
    //                 s0-s15 - "00000000000000001111111111111111"
    const VRegMask CALLER_SAVEDV = VRegMask(GetCallerRegsMask(Arch::AARCH32, true));

public:
    explicit Aarch32RegisterDescription(ArenaAllocator *allocator);
    NO_MOVE_SEMANTIC(Aarch32RegisterDescription);
    NO_COPY_SEMANTIC(Aarch32RegisterDescription);
    ~Aarch32RegisterDescription() override = default;

    ArenaVector<Reg> GetCalleeSaved() override;
    void SetCalleeSaved(const ArenaVector<Reg> &regs) override;

    // Set used regs - change GetCallee
    void SetUsedRegs(const ArenaVector<Reg> &regs) override;

    RegMask GetCallerSavedRegMask() const override
    {
        return caller_saved_;
    }

    VRegMask GetCallerSavedVRegMask() const override
    {
        return caller_savedv_;
    }

    bool IsCalleeRegister(Reg reg) override
    {
        bool is_fp = reg.IsFloat();
        return reg.GetId() >= GetFirstCalleeReg(Arch::AARCH32, is_fp) &&
               reg.GetId() <= GetLastCalleeReg(Arch::AARCH32, is_fp);
    }

    Reg GetZeroReg() const override
    {
        return INVALID_REGISTER;
    }

    bool IsZeroReg([[maybe_unused]] Reg reg) const override
    {
        return false;
    }

    Reg::RegIDType GetTempReg() override
    {
        return INVALID_REG_ID;
    }

    Reg::RegIDType GetTempVReg() override
    {
        return INVALID_REG_ID;
    }

    // Reg Mask
    // r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15
    // -dwr0,-dwr1,-dwr2,-dwr3,-dwr4,---dwr8,---dwr6,---dwr7 <- double word
    // r0,r1,r2,r3,r4,r5,r6,r7,r8,r9, fp+tmp,  sp+ip, lr+pc
    // |----------------------------| <- available for regalloc
    // r0,   r2,   r4,   r6    r8    - market to be available
    RegMask GetDefaultRegMask() const override
    {
        // Set all to 1
        RegMask reg_mask;
        reg_mask.set();
        for (size_t i = 0; i < AVAILABLE_DOUBLE_WORD_REGISTERS; ++i) {
            reg_mask.reset(i * 2U);
        }
        reg_mask.set(GetThreadReg(Arch::AARCH32));
        return reg_mask;
    }

    VRegMask GetVRegMask() override
    {
        VRegMask vreg_mask;
        for (auto vreg_code : AARCH32_TMP_VREG) {
            vreg_mask.set(vreg_code);
        }
        // Only d0-d15 available for alloc
        // They mapped on s0-s31 same, like scalar:
        for (size_t i = 0; i < vreg_mask.size() / 2U; ++i) {
            vreg_mask.set(i * 2U + 1);
        }
        return vreg_mask;
    }

    bool SupportMapping(uint32_t type) override
    {
        // Current implementation does not support vreg-vreg mapping
        if ((type & (RegMapping::VECTOR_VECTOR | RegMapping::FLOAT_FLOAT)) != 0U) {
            return false;
        }
        // Scalar and float registers lay in different registers
        if ((type & (RegMapping::SCALAR_VECTOR | RegMapping::SCALAR_FLOAT)) != 0U) {
            return false;
        }
        // Supported mapping for upper half register-parts:
        // (type & RegMapping::SCALAR_SCALAR != 0)
        return true;
    };

    bool IsValid() const override;

    bool IsRegUsed(ArenaVector<Reg> vec_reg, Reg reg) override;

    // TODO(igorban): implement as virtual
    static bool IsTmp(Reg reg);

public:
    // Special implementation-specific getters
    RegMask GetCalleeSavedR()
    {
        return callee_saved_;
    }
    VRegMask GetCalleeSavedV()
    {
        return callee_savedv_;
    }
    RegMask GetCallerSavedR()
    {
        return caller_saved_;
    }
    VRegMask GetCallerSavedV()
    {
        return caller_savedv_;
    }

    uint8_t GetAligmentReg(bool is_callee)
    {
        auto allignment_reg = is_callee ? allignment_reg_callee_ : allignment_reg_caller_;
        ASSERT(allignment_reg != UNDEF_REG);
        return allignment_reg;
    }

private:
    // Full list of arm64 General-purpose registers (with vector registers)
    ArenaVector<Reg> aarch32_reg_list_;
    //
    ArenaVector<Reg> used_regs_;
    Reg tmp_reg1_;
    Reg tmp_reg2_;

    RegMask callee_saved_ {CALLEE_SAVED};
    RegMask caller_saved_ {CALLER_SAVED};

    VRegMask callee_savedv_ {CALLEE_SAVEDV};
    VRegMask caller_savedv_ {CALLER_SAVEDV};

    uint8_t allignment_reg_callee_ {UNDEF_REG};
    uint8_t allignment_reg_caller_ {UNDEF_REG};
};  // Aarch32RegisterDescription

class Aarch32Encoder;

class Aarch32LabelHolder final : public LabelHolder {
public:
    using LabelType = vixl::aarch32::Label;
    explicit Aarch32LabelHolder(Encoder *enc) : LabelHolder(enc), labels_(enc->GetAllocator()->Adapter()) {};

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

    LabelType *GetLabel(LabelId id)
    {
        ASSERT(labels_.size() > id);
        return labels_[id];
    }

    LabelId Size() override
    {
        return labels_.size();
    };

    NO_MOVE_SEMANTIC(Aarch32LabelHolder);
    NO_COPY_SEMANTIC(Aarch32LabelHolder);
    ~Aarch32LabelHolder() override = default;

private:
    ArenaVector<LabelType *> labels_;
    LabelId id_ {0};
    friend Aarch32Encoder;
};  // Aarch32LabelHolder

class Aarch32Encoder final : public Encoder {
public:
    explicit Aarch32Encoder(ArenaAllocator *allocator);

    LabelHolder *GetLabels() const override
    {
        return labels_;
    };

    ~Aarch32Encoder() override;

    NO_COPY_SEMANTIC(Aarch32Encoder);
    NO_MOVE_SEMANTIC(Aarch32Encoder);

    bool IsValid() const override
    {
        return true;
    }

    static constexpr auto GetTarget()
    {
        return panda::compiler::Target(Arch::AARCH32);
    }

    void SetMaxAllocatedBytes(size_t size) override
    {
        GetMasm()->GetBuffer()->SetMmapMaxBytes(size);
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UnaryOperation(opc) void Encode##opc(Reg dst, Reg src0) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperationRegRegReg(opc) void Encode##opc(Reg dst, Reg src0, Reg src1) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperationRegRegImm(opc) void Encode##opc(Reg dst, Reg src0, Imm src1) override;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperation(opc) BinaryOperationRegRegReg(opc) BinaryOperationRegRegImm(opc)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, TYPE) TYPE(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)

#undef UnaryOperation
#undef BinaryOperation
#undef INST_DEF

    void EncodeNop() override;

    // Additional special instructions
    void EncodeCastToBool(Reg dst, Reg src) override;
    void EncodeCast(Reg dst, bool dst_signed, Reg src, bool src_signed) override;
    void EncodeMin(Reg dst, bool dst_signed, Reg src0, Reg src1) override;
    void EncodeDiv(Reg dst, bool dst_signed, Reg src0, Reg src1) override;
    void EncodeMod(Reg dst, bool dst_signed, Reg src0, Reg src1) override;
    void EncodeMax(Reg dst, bool dst_signed, Reg src0, Reg src1) override;

    void EncodeLdr(Reg dst, bool dst_signed, MemRef mem) override;
    void EncodeLdr(Reg dst, bool dst_signed, const vixl::aarch32::MemOperand &vixl_mem);
    void EncodeLdrAcquire(Reg dst, bool dst_signed, MemRef mem) override;

    void EncodeMemoryBarrier(MemoryOrder::Order order) override;

    void EncodeMov(Reg dst, Imm src) override;
    void EncodeStr(Reg src, const vixl::aarch32::MemOperand &vixl_mem);
    void EncodeStr(Reg src, MemRef mem) override;
    void EncodeStrRelease(Reg src, MemRef mem) override;
    void EncodeStp(Reg src0, Reg src1, MemRef mem) override;

    /* builtins-related encoders */
    void EncodeIsInf(Reg dst, Reg src) override;
    void EncodeBitCount(Reg dst, Reg src) override;
    void EncodeCountLeadingZeroBits(Reg dst, Reg src) override;
    void EncodeCeil([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeFloor([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRint([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeRound([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src) override;
    void EncodeReverseBytes(Reg dst, Reg src) override;
    void EncodeReverseBits(Reg dst, Reg src) override;
    void EncodeFpToBits(Reg dst, Reg src) override;
    void EncodeMoveBitsRaw(Reg dst, Reg src) override;

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

    bool CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, bool signed_compare) override;
    bool CanEncodeImmLogical(uint64_t imm, uint32_t size) override;

    size_t GetCursorOffset() const override
    {
        return GetMasm()->GetBuffer()->GetCursorOffset();
    }

    void SetCursorOffset(size_t offset) override
    {
        GetMasm()->GetBuffer()->Rewind(offset);
    }

    Reg AcquireScratchRegister(TypeInfo type) override
    {
        ASSERT(GetMasm()->GetCurrentScratchRegisterScope() == nullptr);
        if (type.IsFloat()) {
            if (type == FLOAT32_TYPE) {
                auto reg = GetMasm()->GetScratchVRegisterList()->GetFirstAvailableSRegister();
                ASSERT(reg.IsValid());
                GetMasm()->GetScratchVRegisterList()->Remove(reg);
                return Reg(reg.GetCode(), type);
            }
            // Get 2 float registers instead one double - to save agreement about hi-regs in VixlVReg
            auto reg1 = GetMasm()->GetScratchVRegisterList()->GetFirstAvailableSRegister();
            auto reg2 = GetMasm()->GetScratchVRegisterList()->GetFirstAvailableSRegister();
            ASSERT(reg1.IsValid());
            ASSERT(reg2.IsValid());
            GetMasm()->GetScratchVRegisterList()->Remove(reg1);
            GetMasm()->GetScratchVRegisterList()->Remove(reg2);
            return Reg(reg1.GetCode(), type);
        }
        auto reg = GetMasm()->GetScratchRegisterList()->GetFirstAvailableRegister();
        ASSERT(reg.IsValid());
        GetMasm()->GetScratchRegisterList()->Remove(reg);
        return Reg(reg.GetCode(), type);
    }

    void AcquireScratchRegister(Reg reg) override
    {
        if (reg == GetTarget().GetLinkReg()) {
            ASSERT_PRINT(!lr_acquired_, "Trying to acquire LR, which hasn't been released before");
            lr_acquired_ = true;
        } else if (reg.IsFloat()) {
            ASSERT(GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(vixl::aarch32::SRegister(reg.GetId())));
            GetMasm()->GetScratchVRegisterList()->Remove(vixl::aarch32::SRegister(reg.GetId()));
        } else {
            ASSERT(GetMasm()->GetScratchRegisterList()->Includes(vixl::aarch32::Register(reg.GetId())));
            GetMasm()->GetScratchRegisterList()->Remove(vixl::aarch32::Register(reg.GetId()));
        }
    }

    void ReleaseScratchRegister(Reg reg) override
    {
        if (reg == GetTarget().GetLinkReg()) {
            ASSERT_PRINT(lr_acquired_, "Trying to release LR, which hasn't been acquired before");
            lr_acquired_ = false;
        } else if (reg.IsFloat()) {
            GetMasm()->GetScratchVRegisterList()->Combine(vixl::aarch32::SRegister(reg.GetId()));
        } else {
            GetMasm()->GetScratchRegisterList()->Combine(vixl::aarch32::Register(reg.GetId()));
        }
    }

    bool IsScratchRegisterReleased(Reg reg) override
    {
        if (reg == GetTarget().GetLinkReg()) {
            return !lr_acquired_;
        }
        if (reg.IsFloat()) {
            return GetMasm()->GetScratchVRegisterList()->IncludesAliasOf(vixl::aarch32::SRegister(reg.GetId()));
        }
        return GetMasm()->GetScratchRegisterList()->Includes(vixl::aarch32::Register(reg.GetId()));
    }

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

    void SetRegister(RegMask *mask, VRegMask *vmask, Reg reg, bool val) const override
    {
        if (!reg.IsValid()) {
            return;
        }
        if (reg.IsScalar()) {
            ASSERT(mask != nullptr);
            mask->set(reg.GetId(), val);
            if (reg.GetSize() > WORD_SIZE) {
                mask->set(reg.GetId() + 1, val);
            }
        } else {
            ASSERT(vmask != nullptr);
            ASSERT(reg.IsFloat());
            vmask->set(reg.GetId(), val);
            if (reg.GetSize() > WORD_SIZE) {
                vmask->set(reg.GetId() + 1, val);
            }
        }
    }

    TypeInfo GetRefType() override
    {
        return INT32_TYPE;
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
    void MakeCall(const void *entry_point) override;
    void MakeCall(MemRef entry_point) override;
    void MakeCall(Reg reg) override;

    void MakeCallAot(intptr_t offset) override;
    void MakeCallByOffset(intptr_t offset) override;
    void MakeLoadAotTable(intptr_t offset, Reg reg) override;
    void MakeLoadAotTableAddr(intptr_t offset, Reg addr, Reg val) override;

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

    void EncodeStackOverflowCheck(ssize_t offset) override;

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

    static inline vixl::aarch32::MemOperand ConvertMem(MemRef mem)
    {
        bool has_index = mem.HasIndex();
        bool has_shift = mem.HasScale();
        bool has_offset = mem.HasDisp();
        auto base_reg = VixlReg(mem.GetBase());
        if (has_index) {
            // MemRef with index and offser isn't supported
            ASSERT(!has_offset);
            auto index_reg = mem.GetIndex();
            if (has_shift) {
                auto shift = mem.GetScale();
                return vixl::aarch32::MemOperand(base_reg, VixlReg(index_reg), vixl::aarch32::LSL, shift);
            }
            return vixl::aarch32::MemOperand(base_reg, VixlReg(index_reg));
        }
        if (has_offset) {
            auto offset = mem.GetDisp();
            return vixl::aarch32::MemOperand(base_reg, offset);
        }
        return vixl::aarch32::MemOperand(base_reg);
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
    static bool IsNeedToPrepareMemLdS(MemRef mem, const TypeInfo &mem_type, bool is_signed);
    vixl::aarch32::MemOperand PrepareMemLdS(MemRef mem, const TypeInfo &mem_type, vixl::aarch32::Register tmp,
                                            bool is_signed, bool copy_sp = false);

    void MakeLibCall(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value = false);

    void MakeLibCall(Reg dst, Reg src, void *entry_point);

    vixl::aarch32::MacroAssembler *GetMasm() const
    {
        ASSERT(masm_ != nullptr);
        return masm_;
    }

    size_t GetLabelAddress(LabelHolder::LabelId label) override
    {
        auto plabel = labels_->GetLabel(label);
        ASSERT(plabel->IsBound());
        return GetMasm()->GetBuffer()->GetOffsetAddress<size_t>(plabel->GetLocation());
    }

    bool LabelHasLinks(LabelHolder::LabelId label) override
    {
        auto plabel = labels_->GetLabel(label);
        return plabel->IsReferenced();
    }

private:
    template <bool is_store>
    void LoadStoreRegisters(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp);

    template <bool is_store>
    void LoadStoreRegisters(RegMask registers, bool is_fp, int32_t slot, Reg base, RegMask mask);

private:
    vixl::aarch32::MemOperand PrepareMemLdSForFloat(MemRef mem, vixl::aarch32::Register tmp);
    void EncodeCastFloatToFloat(Reg dst, Reg src);
    void EncodeCastFloatToInt64(Reg dst, Reg src);
    void EncodeCastDoubleToInt64(Reg dst, Reg src);
    void EncodeCastScalarToFloat(Reg dst, Reg src, bool src_signed);
    void EncodeCastFloatToScalar(Reg dst, bool dst_signed, Reg src);
    void EncodeCastFloatToScalarWithSmallDst(Reg dst, bool dst_signed, Reg src);

    void EncoderCastExtendFromInt32(Reg dst, bool dst_signed);
    void EncodeCastScalar(Reg dst, bool dst_signed, Reg src, bool src_signed);
    void EncodeCastScalarFromSignedScalar(Reg dst, Reg src);
    void EncodeCastScalarFromUnsignedScalar(Reg dst, Reg src);
    template <bool is_max>
    void EncodeMinMaxFp(Reg dst, Reg src0, Reg src1);
    void EncodeVorr(Reg dst, Reg src0, Reg src1);
    void EncodeVand(Reg dst, Reg src0, Reg src1);
    void MakeLibCallWithFloatResult(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value);
    void MakeLibCallWithDoubleResult(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value);
    void MakeLibCallWithInt64Result(Reg dst, Reg src0, Reg src1, void *entry_point, bool second_value);
    void CompareHelper(Reg src0, Reg src1, Condition *cc);
    void TestHelper(Reg src0, Reg src1, Condition cc);
    bool CompareImmHelper(Reg src, Imm imm, Condition *cc);
    void TestImmHelper(Reg src, Imm imm, Condition cc);
    bool CompareNegImmHelper(Reg src, int64_t value, const Condition *cc);
    bool ComparePosImmHelper(Reg src, int64_t value, Condition *cc);
    void CompareZeroHelper(Reg src, Condition *cc);
    void EncodeFloatSti(Imm src, MemRef mem);
    static inline constexpr int32_t MEM_BIG_OFFSET = 4095;
    static inline constexpr int32_t MEM_SMALL_OFFSET = 255;
    static inline constexpr int32_t VMEM_OFFSET = 1020;
    Aarch32LabelHolder *labels_ {nullptr};
    vixl::aarch32::MacroAssembler *masm_ {nullptr};
    bool lr_acquired_ {false};
};  // Aarch32Encoder

class Aarch32ParameterInfo final : public ParameterInfo {
public:
    std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo &type) override;
    Location GetNextLocation(DataType::Type type) override;
};

class Aarch32CallingConvention : public CallingConvention {
public:
    Aarch32CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr, CallConvMode mode);

    static constexpr auto GetTarget()
    {
        return panda::compiler::Target(Arch::AARCH32);
    }

    bool IsValid() const override
    {
        return true;
    }

    void GeneratePrologue(const FrameInfo &frame_info) override;
    void GenerateEpilogue(const FrameInfo &frame_info, std::function<void()> post_job) override;
    void GenerateNativePrologue(const FrameInfo &frame_info) override
    {
        GeneratePrologue(frame_info);
    }
    void GenerateNativeEpilogue(const FrameInfo &frame_info, std::function<void()> post_job) override
    {
        GenerateEpilogue(frame_info, post_job);
    }

    void *GetCodeEntry() override;
    uint32_t GetCodeSize() override;

    vixl::aarch32::MacroAssembler *GetMasm()
    {
        return (static_cast<Aarch32Encoder *>(GetEncoder()))->GetMasm();
    }

    // Calculating information about parameters and save regs_offset registers for special needs
    ParameterInfo *GetParameterInfo(uint8_t regs_offset) override;

    NO_MOVE_SEMANTIC(Aarch32CallingConvention);
    NO_COPY_SEMANTIC(Aarch32CallingConvention);
    ~Aarch32CallingConvention() override = default;

private:
    uint8_t PushPopVRegs(VRegMask vregs, bool is_push);
    uint8_t PushRegs(RegMask regs, VRegMask vregs, bool is_callee);
    uint8_t PopRegs(RegMask regs, VRegMask vregs, bool is_callee);
};  // Aarch32CallingConvention
}  // namespace panda::compiler::aarch32

#endif  // COMPILER_OPTIMIZER_CODEGEN_TARGET_AARCH32_TARGET_H_
