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

#ifndef COMPILER_OPTIMIZER_CODEGEN_ENCODE_H_
#define COMPILER_OPTIMIZER_CODEGEN_ENCODE_H_

/*
    Hi-level interface for encoding
    Wrapper for specialize concrete used encoding

    Responsible for
        Primitive (not-branch) instruction encoding
        Memory-instructions encoding
        Immediate and Memory operands
*/

#include <variant>

#include "operands.h"
#include "registers_description.h"
#include "utils/cframe_layout.h"
#include "target_info.h"

namespace panda::compiler {
class Encoder;
class CompilerOptions;
class RelocationInfo;

namespace MemoryOrder {
enum Order { Acquire, Release, Full };
}  // namespace MemoryOrder

class LabelHolder {
public:
    using LabelId = uintptr_t;
    static constexpr LabelId INVALID_LABEL = static_cast<uintptr_t>(-1);

    explicit LabelHolder(Encoder *enc) : enc_ {enc} {};
    virtual ~LabelHolder() = default;

    // TODO (igorban) : hide all this methods in CallConv
    virtual void CreateLabels(LabelId size) = 0;
    virtual LabelId CreateLabel() = 0;
    virtual LabelId Size() = 0;

    Encoder *GetEncoder() const
    {
        return enc_;
    }

    NO_COPY_SEMANTIC(LabelHolder);
    NO_MOVE_SEMANTIC(LabelHolder);

protected:
    virtual void BindLabel(LabelId) = 0;

private:
    Encoder *enc_ {nullptr};
    friend Encoder;
};

class Encoder {
public:
    // Main constructor
    explicit Encoder(ArenaAllocator *aa, Arch arch) : Encoder(aa, arch, false) {}
    Encoder(ArenaAllocator *aa, Arch arch, bool js_number_cast)
        : allocator_(aa), frame_layout_(arch, 0), target_(arch), js_number_cast_(js_number_cast)
    {
    }
    virtual ~Encoder() = default;

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }

    bool IsLabelValid(LabelHolder::LabelId label)
    {
        return label != LabelHolder::INVALID_LABEL;
    }

    Target GetTarget() const
    {
        return target_;
    }

    Arch GetArch() const
    {
        return GetTarget().GetArch();
    }

    bool IsJsNumberCast() const
    {
        return js_number_cast_;
    }

    void SetIsJsNumberCast(bool v)
    {
        js_number_cast_ = v;
    }

    /// Print instruction and return next pc
    virtual size_t DisasmInstr([[maybe_unused]] std::ostream &stream, [[maybe_unused]] size_t pc,
                               [[maybe_unused]] ssize_t code_offset) const
    {
        return 0;
    }

    virtual void *BufferData() const
    {
        return nullptr;
    }

    /// Size of used buffer
    virtual size_t BufferSize() const
    {
        return 0;
    }

    // Default behaviour - do nothing
    virtual bool InitMasm()
    {
        return true;
    }

    virtual void SetMaxAllocatedBytes([[maybe_unused]] size_t size) {};

// Define default math operations
// Encode (dst, src)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UnaryOperation(opc)            \
    virtual void Encode##opc(Reg, Reg) \
    {                                  \
        SetFalseResult();              \
    }

// Encode (dst, src0, src1)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryOperation(opc)                \
    virtual void Encode##opc(Reg, Reg, Reg) \
    {                                       \
        SetFalseResult();                   \
    }                                       \
    virtual void Encode##opc(Reg, Reg, Imm) \
    {                                       \
        SetFalseResult();                   \
    }

// Encode (dst, src0, src1)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BinaryShiftedRegisterOperation(opc)   \
    virtual void Encode##opc(Reg, Reg, Shift) \
    {                                         \
        SetFalseResult();                     \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, TYPE) TYPE(OPCODE)

    ENCODE_MATH_LIST(INST_DEF)

    ENCODE_INST_WITH_SHIFTED_OPERAND(INST_DEF)

#undef UnaryOperation
#undef BinaryOperation
#undef BinaryShiftedRegisterOperation
#undef INST_DEF

    virtual void EncodeNop()
    {
        SetFalseResult();
    }

    virtual void EncodeAddOverflow([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] Reg dst,
                                   [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeSubOverflow([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] Reg dst,
                                   [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeMulOverflow([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] Reg dst,
                                   [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeCast([[maybe_unused]] Reg dst, [[maybe_unused]] bool dst_signed, [[maybe_unused]] Reg src,
                            [[maybe_unused]] bool src_signed)
    {
        SetFalseResult();
    }
    virtual void EncodeCastToBool([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }
    virtual void EncodeMin([[maybe_unused]] Reg dst, [[maybe_unused]] bool dst_signed, [[maybe_unused]] Reg src0,
                           [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }
    virtual void EncodeDiv([[maybe_unused]] Reg dst, [[maybe_unused]] bool dst_signed, [[maybe_unused]] Reg src0,
                           [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }
    virtual void EncodeMod([[maybe_unused]] Reg dst, [[maybe_unused]] bool dst_signed, [[maybe_unused]] Reg src0,
                           [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }
    virtual void EncodeMax([[maybe_unused]] Reg dst, [[maybe_unused]] bool dst_signed, [[maybe_unused]] Reg src0,
                           [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }
    virtual void EncodeMov([[maybe_unused]] Reg dst, [[maybe_unused]] Imm src)
    {
        SetFalseResult();
    }

    virtual void EncodeLdr([[maybe_unused]] Reg dst, [[maybe_unused]] bool dst_signed, [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }
    virtual void EncodeLdrAcquire([[maybe_unused]] Reg dst, [[maybe_unused]] bool dst_signed,
                                  [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }
    virtual void EncodeStr([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }
    virtual void EncodeStrRelease([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }
    virtual void EncodeLdrExclusive([[maybe_unused]] Reg dst, [[maybe_unused]] Reg addr, [[maybe_unused]] bool acquire)
    {
        SetFalseResult();
    }
    virtual void EncodeStrExclusive([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Reg addr,
                                    [[maybe_unused]] bool release)
    {
        SetFalseResult();
    }

    // zerod high part: [reg.size, 64)
    virtual void EncodeStrz([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }
    virtual void Push([[maybe_unused]] Reg src, [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }
    virtual void EncodeSti([[maybe_unused]] Imm src, [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }
    // size must be 8, 16,32 or 64
    virtual void EncodeMemCopy([[maybe_unused]] MemRef mem_from, [[maybe_unused]] MemRef mem_to,
                               [[maybe_unused]] size_t size)
    {
        SetFalseResult();
    }
    // size must be 8, 16,32 or 64
    // zerod high part: [size, 64)
    virtual void EncodeMemCopyz([[maybe_unused]] MemRef mem_from, [[maybe_unused]] MemRef mem_to,
                                [[maybe_unused]] size_t size)
    {
        SetFalseResult();
    }

    virtual void EncodeCmp([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                           [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    // Additional check for isnan-comparison
    virtual void EncodeCompare([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                               [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeCompareTest([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                                   [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeCompressedStringCharAt([[maybe_unused]] Reg dst, [[maybe_unused]] Reg str,
                                              [[maybe_unused]] Reg idx, [[maybe_unused]] Reg length,
                                              [[maybe_unused]] Reg tmp, [[maybe_unused]] size_t data_offset,
                                              [[maybe_unused]] uint32_t shift)
    {
        SetFalseResult();
    }

    virtual void EncodeCompressedStringCharAtI([[maybe_unused]] Reg dst, [[maybe_unused]] Reg str,
                                               [[maybe_unused]] Reg length, [[maybe_unused]] size_t data_offset,
                                               [[maybe_unused]] uint32_t index, [[maybe_unused]] uint32_t shift)
    {
        SetFalseResult();
    }

    virtual void EncodeSelect([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                              [[maybe_unused]] Reg src2, [[maybe_unused]] Reg src3, [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeSelectTest([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                                  [[maybe_unused]] Reg src2, [[maybe_unused]] Reg src3, [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeIsInf([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0)
    {
        SetFalseResult();
    }

    virtual void EncodeReverseBytes([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeReverseBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeBitCount([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeRotate([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src1, [[maybe_unused]] Reg src2,
                              [[maybe_unused]] bool is_ror)
    {
        SetFalseResult();
    }

    virtual void EncodeSignum([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeCountLeadingZeroBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeCountTrailingZeroBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeCeil([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeFloor([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeRint([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeRound([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeStringEquals([[maybe_unused]] Reg dst, [[maybe_unused]] Reg str1, [[maybe_unused]] Reg str2,
                                    [[maybe_unused]] bool compression, [[maybe_unused]] uint32_t length_offset,
                                    [[maybe_unused]] uint32_t data_offset)
    {
        SetFalseResult();
    }

    virtual void EncodeStringIndexOfAfter([[maybe_unused]] Reg dst, [[maybe_unused]] Reg str,
                                          [[maybe_unused]] Reg character, [[maybe_unused]] Reg idx,
                                          [[maybe_unused]] Reg tmp, [[maybe_unused]] bool compression,
                                          [[maybe_unused]] uint32_t length_offset,
                                          [[maybe_unused]] uint32_t data_offset,
                                          [[maybe_unused]] int32_t char_const_value)
    {
        SetFalseResult();
    }

    virtual void EncodeSelect([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                              [[maybe_unused]] Reg src2, [[maybe_unused]] Imm imm, [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeSelectTest([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                                  [[maybe_unused]] Reg src2, [[maybe_unused]] Imm imm, [[maybe_unused]] Condition cc)
    {
        SetFalseResult();
    }

    virtual void EncodeGetTypeSize([[maybe_unused]] Reg size, [[maybe_unused]] Reg type)
    {
        SetFalseResult();
    }

    virtual void EncodeLdp([[maybe_unused]] Reg dst0, [[maybe_unused]] Reg dst1, [[maybe_unused]] bool dst_signed,
                           [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }

    virtual void EncodeStp([[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1, [[maybe_unused]] MemRef mem)
    {
        SetFalseResult();
    }

    virtual void EncodeMAdd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                            [[maybe_unused]] Reg src2)
    {
        SetFalseResult();
    }

    virtual void EncodeMSub([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1,
                            [[maybe_unused]] Reg src2)
    {
        SetFalseResult();
    }

    virtual void EncodeMNeg([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }

    virtual void EncodeOrNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }

    virtual void EncodeAndNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }

    virtual void EncodeXorNot([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src0, [[maybe_unused]] Reg src1)
    {
        SetFalseResult();
    }

    virtual void EncodeNeg([[maybe_unused]] Reg dst, [[maybe_unused]] Shift src)
    {
        SetFalseResult();
    }

    virtual void EncodeFpToBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeMoveBitsRaw([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src)
    {
        SetFalseResult();
    }

    virtual void EncodeExtractBits([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Imm imm1,
                                   [[maybe_unused]] Imm imm2)
    {
        success_ = false;
    }

    virtual void EncodeCrc32Update([[maybe_unused]] Reg dst, [[maybe_unused]] Reg crc_reg, [[maybe_unused]] Reg val_reg)
    {
        SetFalseResult();
    }

    /**
     * Encode dummy load from the address [sp + offset].
     * @param offset offset from the stack pointer register
     */
    virtual void EncodeStackOverflowCheck([[maybe_unused]] ssize_t offset)
    {
        SetFalseResult();
    }

    virtual bool IsValid() const
    {
        return false;
    }

    virtual bool GetResult() const
    {
        return success_;
    }

    void SetFalseResult()
    {
        success_ = false;
    }

    // Encoder builder - implement in target.cpp
    static Encoder *Create(ArenaAllocator *arena_allocator, Arch arch, bool print_asm, bool js_number_cast = false);

    // For now it is one function for Add/Sub and Cmp, it suits all considered targets (x86, amd64, arm32, arm64).
    // We probably should revisit this if we add new targets, like Thumb-2 or others.
    virtual bool CanEncodeImmAddSubCmp([[maybe_unused]] int64_t imm, [[maybe_unused]] uint32_t size,
                                       [[maybe_unused]] bool signed_compare)
    {
        return false;
    }

    virtual bool CanEncodeImmMulDivMod([[maybe_unused]] uint64_t imm, [[maybe_unused]] uint32_t size)
    {
        return false;
    }

    virtual bool CanEncodeImmLogical([[maybe_unused]] uint64_t imm, [[maybe_unused]] uint32_t size)
    {
        return false;
    }

    virtual bool CanEncodeScale([[maybe_unused]] uint64_t imm, [[maybe_unused]] uint32_t size)
    {
        return false;
    }

    virtual bool CanEncodeShift([[maybe_unused]] uint32_t size)
    {
        return true;
    }

    virtual bool CanEncodeBitCount()
    {
        return false;
    }

    virtual bool CanEncodeMAdd()
    {
        return false;
    }

    virtual bool CanEncodeMSub()
    {
        return false;
    }

    virtual bool CanEncodeMNeg()
    {
        return false;
    }

    virtual bool CanEncodeOrNot()
    {
        return false;
    }

    virtual bool CanEncodeAndNot()
    {
        return false;
    }

    virtual bool CanEncodeXorNot()
    {
        return false;
    }

    // Check if encoder is capable of encoding operations where an operand is a register with
    // a value shifted by shift operation with specified type by some immediate value.
    virtual bool CanEncodeShiftedOperand([[maybe_unused]] ShiftOpcode opcode, [[maybe_unused]] ShiftType shift_type)
    {
        return false;
    }

    virtual bool CanEncodeCompressedStringCharAt()
    {
        return false;
    }

    virtual bool CanEncodeCompressedStringCharAtI()
    {
        return false;
    }

    virtual void EncodeCompareAndSwap([[maybe_unused]] Reg dst, [[maybe_unused]] Reg obj, [[maybe_unused]] Reg offset,
                                      [[maybe_unused]] Reg val, [[maybe_unused]] Reg newval)
    {
        SetFalseResult();
    }

    virtual void EncodeCompareAndSwap([[maybe_unused]] Reg dst, [[maybe_unused]] Reg addr, [[maybe_unused]] Reg val,
                                      [[maybe_unused]] Reg newval)
    {
        SetFalseResult();
    }

    virtual void EncodeUnsafeGetAndSet([[maybe_unused]] Reg dst, [[maybe_unused]] Reg obj, [[maybe_unused]] Reg offset,
                                       [[maybe_unused]] Reg val)
    {
        SetFalseResult();
    }

    virtual void EncodeUnsafeGetAndAdd([[maybe_unused]] Reg dst, [[maybe_unused]] Reg obj, [[maybe_unused]] Reg offset,
                                       [[maybe_unused]] Reg val, [[maybe_unused]] Reg tmp)
    {
        SetFalseResult();
    }

    virtual void EncodeMemoryBarrier([[maybe_unused]] MemoryOrder::Order order)
    {
        SetFalseResult();
    }

    virtual size_t GetCursorOffset() const
    {
        return 0;
    }

    virtual void SetCursorOffset([[maybe_unused]] size_t offset) {}

    virtual void SaveRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] ssize_t slot,
                               [[maybe_unused]] size_t start_reg, [[maybe_unused]] bool is_fp)
    {
        SetFalseResult();
    }
    virtual void LoadRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] ssize_t slot,
                               [[maybe_unused]] size_t start_reg, [[maybe_unused]] bool is_fp)
    {
        SetFalseResult();
    }

    /**
     * Save/load registers to/from the memory.
     *
     * If `mask` is empty (all bits are zero), then registers will be saved densely, otherwise place for each register
     * will be determined according to this mask.
     * Example: registers' bits = [1, 3, 10], mask's bits = [0, 1, 2, 3, 8, 9, 10, 11]
     * We can see that mask has the gap in 4-7 bits. So, registers will be saved in the following slots:
     *      slots: 0   1   2   3   4   5   6   7
     *      regs :     1       3          10
     * If the mask would be zero, then the following layout will be used:
     *      slots: 0   1   2
     *      regs : 1   3  10
     *
     * @param registers mask of registers to be saved
     * @param is_fp if true, registers are floating point registers
     * @param slot offset from the `base` register to the destination address (in words)
     * @param base base register
     * @param mask determine memory layout for the registers
     */
    virtual void SaveRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool is_fp,
                               [[maybe_unused]] ssize_t slot, [[maybe_unused]] Reg base, [[maybe_unused]] RegMask mask)
    {
        SetFalseResult();
    }
    virtual void LoadRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool is_fp,
                               [[maybe_unused]] ssize_t slot, [[maybe_unused]] Reg base, [[maybe_unused]] RegMask mask)
    {
        SetFalseResult();
    }
    virtual void PushRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool is_fp,
                               [[maybe_unused]] bool align)
    {
        SetFalseResult();
    }

    virtual void PopRegisters([[maybe_unused]] RegMask registers, [[maybe_unused]] bool is_fp,
                              [[maybe_unused]] bool align)
    {
        SetFalseResult();
    }

    RegistersDescription *GetRegfile() const
    {
        ASSERT(regfile_ != nullptr);
        return regfile_;
    }

    void SetRegfile(RegistersDescription *regfile)
    {
        regfile_ = regfile;
    }

    virtual compiler::Reg AcquireScratchRegister([[maybe_unused]] compiler::TypeInfo type)
    {
        return compiler::Reg();
    }

    virtual void AcquireScratchRegister([[maybe_unused]] compiler::Reg reg)
    {
        SetFalseResult();
    }

    virtual void ReleaseScratchRegister([[maybe_unused]] compiler::Reg reg)
    {
        SetFalseResult();
    }

    virtual bool IsScratchRegisterReleased([[maybe_unused]] compiler::Reg reg)
    {
        return false;
    }

    size_t GetScratchRegistersCount() const
    {
        return GetScratchRegistersMask().Count();
    }

    virtual RegMask GetScratchRegistersMask() const
    {
        return 0;
    }

    size_t GetScratchFPRegistersCount() const
    {
        return GetScratchFpRegistersMask().Count();
    }

    virtual RegMask GetScratchFpRegistersMask() const
    {
        return 0;
    }

    // Get Scratch registers, that currently are not allocated
    virtual RegMask GetAvailableScratchRegisters() const
    {
        return 0;
    }

    // Get Floating Point Scratch registers, that currently are not allocated
    virtual VRegMask GetAvailableScratchFpRegisters() const
    {
        return 0;
    }

    virtual size_t MaxArchInstPerEncoded()
    {
        static constexpr size_t MAX_ARCH_INST_PER_ENCODE = 32;
        return MAX_ARCH_INST_PER_ENCODE;
    }

    virtual void SetRegister(RegMask *mask, VRegMask *vmask, Reg reg)
    {
        SetRegister(mask, vmask, reg, true);
    }

    virtual void SetRegister(RegMask *mask, VRegMask *vmask, Reg reg, bool val) const
    {
        if (!reg.IsValid()) {
            return;
        }
        if (reg.IsScalar()) {
            ASSERT(mask != nullptr);
            mask->set(reg.GetId(), val);
        } else {
            ASSERT(vmask != nullptr);
            ASSERT(reg.IsFloat());
            if (vmask != nullptr) {
                vmask->set(reg.GetId(), val);
            }
        }
    }

    virtual compiler::TypeInfo GetRefType()
    {
        return compiler::TypeInfo();
    }

    virtual void Finalize() = 0;

public:
    /**
     *   Label-holder interfaces
     */
    LabelHolder::LabelId CreateLabel()
    {
        auto labels = GetLabels();
        ASSERT(labels != nullptr);
        return labels->CreateLabel();
    }

    void BindLabel(LabelHolder::LabelId id)
    {
        auto labels = GetLabels();
        ASSERT(labels != nullptr);
        ASSERT(labels->Size() > id);
        labels->BindLabel(id);
    }

    virtual LabelHolder *GetLabels() const
    {
        return nullptr;
    }
    virtual size_t GetLabelAddress(LabelHolder::LabelId label) = 0;

    virtual bool LabelHasLinks(LabelHolder::LabelId label) = 0;

public:
    virtual void MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
    {
        SetFalseResult();
    }

    virtual void MakeCall([[maybe_unused]] compiler::LabelHolder::LabelId id)
    {
        SetFalseResult();
    }

    virtual void MakeCall([[maybe_unused]] const void *entry_point)
    {
        SetFalseResult();
    }

    virtual void MakeCall([[maybe_unused]] Reg reg)
    {
        SetFalseResult();
    }

    virtual void MakeCall([[maybe_unused]] compiler::MemRef entry_point)
    {
        SetFalseResult();
    }

    virtual void MakeCallAot([[maybe_unused]] intptr_t offset)
    {
        SetFalseResult();
    }

    virtual bool CanMakeCallByOffset([[maybe_unused]] intptr_t offset)
    {
        return false;
    }

    virtual void MakeCallByOffset([[maybe_unused]] intptr_t offset)
    {
        SetFalseResult();
    }

    virtual void MakeLoadAotTable([[maybe_unused]] intptr_t offset, [[maybe_unused]] compiler::Reg reg)
    {
        SetFalseResult();
    }

    virtual void MakeLoadAotTableAddr([[maybe_unused]] intptr_t offset, [[maybe_unused]] compiler::Reg addr,
                                      [[maybe_unused]] compiler::Reg val)
    {
        SetFalseResult();
    }

    // Encode unconditional branch
    virtual void EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id)
    {
        SetFalseResult();
    }

    // Encode jump with compare to zero
    virtual void EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg reg,
                            [[maybe_unused]] compiler::Condition cond)
    {
        SetFalseResult();
    }

    // Compare reg and immediate and branch
    virtual void EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg reg,
                            [[maybe_unused]] compiler::Imm imm, [[maybe_unused]] compiler::Condition c)
    {
        SetFalseResult();
    }

    // Compare two regs and branch
    virtual void EncodeJump([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg r,
                            [[maybe_unused]] compiler::Reg reg, [[maybe_unused]] compiler::Condition c)
    {
        SetFalseResult();
    }

    // Compare reg and immediate and branch
    virtual void EncodeJumpTest([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg reg,
                                [[maybe_unused]] compiler::Imm imm, [[maybe_unused]] compiler::Condition c)
    {
        SetFalseResult();
    }

    // Compare two regs and branch
    virtual void EncodeJumpTest([[maybe_unused]] compiler::LabelHolder::LabelId id, [[maybe_unused]] compiler::Reg r,
                                [[maybe_unused]] compiler::Reg reg, [[maybe_unused]] compiler::Condition c)
    {
        SetFalseResult();
    }

    // Encode jump by register value
    virtual void EncodeJump([[maybe_unused]] compiler::Reg reg)
    {
        SetFalseResult();
    }

    virtual void EncodeJump([[maybe_unused]] RelocationInfo *relocation)
    {
        SetFalseResult();
    }

    virtual void EncodeBitTestAndBranch([[maybe_unused]] compiler::LabelHolder::LabelId id,
                                        [[maybe_unused]] compiler::Reg reg, [[maybe_unused]] uint32_t bit_pos,
                                        [[maybe_unused]] bool bit_value)
    {
        SetFalseResult();
    }

    virtual void EncodeAbort()
    {
        SetFalseResult();
    }

    virtual void EncodeReturn()
    {
        SetFalseResult();
    }

    void SetFrameLayout(CFrameLayout fl)
    {
        frame_layout_ = fl;
    }

    const CFrameLayout &GetFrameLayout() const
    {
        return frame_layout_;
    }

    RegMask GetLiveTmpRegMask()
    {
        return live_tmp_regs_;
    }

    VRegMask GetLiveTmpFpRegMask()
    {
        return live_tmp_fp_regs_;
    }

    void AddRegInLiveMask(Reg reg)
    {
        if (!reg.IsValid()) {
            return;
        }
        if (reg.IsScalar()) {
            live_tmp_regs_.set(reg.GetId(), true);
        } else {
            ASSERT(reg.IsFloat());
            live_tmp_fp_regs_.set(reg.GetId(), true);
        }
    }

    void RemoveRegFromLiveMask(Reg reg)
    {
        if (!reg.IsValid()) {
            return;
        }
        if (reg.IsScalar()) {
            live_tmp_regs_.set(reg.GetId(), false);
        } else {
            ASSERT(reg.IsFloat());
            live_tmp_fp_regs_.set(reg.GetId(), false);
        }
    }

    void SetCodeOffset(size_t offset)
    {
        code_offset_ = offset;
    }

    size_t GetCodeOffset() const
    {
        return code_offset_;
    }

    void EnableLrAsTempReg(bool value)
    {
        enable_lr_as_temp_reg_ = value;
    }

    bool IsLrAsTempRegEnabled() const
    {
        return enable_lr_as_temp_reg_;
    }

    NO_COPY_SEMANTIC(Encoder);
    NO_MOVE_SEMANTIC(Encoder);

protected:
    void SetFrameSize(size_t size)
    {
        frame_size_ = size;
    }

    size_t GetFrameSize() const
    {
        return frame_size_;
    }

    static constexpr size_t INVALID_OFFSET = std::numeric_limits<size_t>::max();

private:
    ArenaAllocator *allocator_;
    RegistersDescription *regfile_ {nullptr};
    size_t frame_size_ {0};

    CFrameLayout frame_layout_;

    RegMask live_tmp_regs_;
    VRegMask live_tmp_fp_regs_;

    // In case of AOT compilation, this variable specifies offset from the start of the AOT file.
    // It is needed for accessing to the entrypoints table and AOT table, that lie right before code.
    size_t code_offset_ {INVALID_OFFSET};

    Target target_ {Arch::NONE};

    bool success_ {true};
    bool js_number_cast_ {false};
    // If true, then ScopedTmpReg can use LR as a temp register.
    bool enable_lr_as_temp_reg_ {false};
};  // Encoder

/**
 * This class is using to acquire/release temp register using RAII technique.
 *
 * @tparam lazy if true, temp register will be acquired in the constructor, otherwise user should acquire it explicitly.
 */
template <bool lazy>
class ScopedTmpRegImpl {
public:
    explicit ScopedTmpRegImpl(Encoder *encoder) : ScopedTmpRegImpl(encoder, false) {}
    ScopedTmpRegImpl(Encoder *encoder, bool with_lr) : encoder_(encoder)
    {
        if constexpr (!lazy) {  // NOLINT
            auto link_reg = encoder->GetTarget().GetLinkReg();
            with_lr &= encoder->IsLrAsTempRegEnabled();
            if (with_lr && encoder->IsScratchRegisterReleased(link_reg)) {
                reg_ = link_reg;
                encoder->AcquireScratchRegister(link_reg);
            } else {
                reg_ = encoder->AcquireScratchRegister(Is64BitsArch(encoder->GetArch()) ? INT64_TYPE : INT32_TYPE);
            }
        }
    }

    ScopedTmpRegImpl(Encoder *encoder, TypeInfo type) : encoder_(encoder), reg_(encoder->AcquireScratchRegister(type))
    {
        static_assert(!lazy);
    }

    ScopedTmpRegImpl(Encoder *encoder, Reg reg) : encoder_(encoder), reg_(reg)
    {
        static_assert(!lazy);
        encoder->AcquireScratchRegister(reg);
    }

    ScopedTmpRegImpl(ScopedTmpRegImpl &&other) noexcept
    {
        reg_ = other.reg_;
        other.reg_ = Reg();
        ASSERT(!other.reg_.IsValid());
    }

    virtual ~ScopedTmpRegImpl()
    {
        if (reg_.IsValid()) {
            encoder_->ReleaseScratchRegister(reg_);
        }
    }

    NO_COPY_SEMANTIC(ScopedTmpRegImpl);
    NO_MOVE_OPERATOR(ScopedTmpRegImpl);

    Reg GetReg() const
    {
        return reg_;
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator Reg() const
    {
        return reg_;
    }

    void ChangeType(TypeInfo tp)
    {
        ASSERT(tp.IsScalar() == reg_.IsScalar());
        reg_ = Reg(reg_.GetId(), tp);
    }

    void Release()
    {
        if (reg_.IsValid()) {
            encoder_->ReleaseScratchRegister(reg_);
            reg_ = INVALID_REGISTER;
        }
    }

    void Acquire()
    {
        ASSERT(!reg_.IsValid());
        reg_ = encoder_->AcquireScratchRegister(Is64BitsArch(encoder_->GetArch()) ? INT64_TYPE : INT32_TYPE);
        ASSERT(reg_.IsValid());
    }

    void AcquireIfInvalid()
    {
        if (!reg_.IsValid()) {
            reg_ = encoder_->AcquireScratchRegister(Is64BitsArch(encoder_->GetArch()) ? INT64_TYPE : INT32_TYPE);
            ASSERT(reg_.IsValid());
        }
    }

protected:
    Encoder *GetEncoder()
    {
        return encoder_;
    }

private:
    Encoder *encoder_ {nullptr};
    Reg reg_;
};

struct ScopedTmpReg : public ScopedTmpRegImpl<false> {
    using ScopedTmpRegImpl<false>::ScopedTmpRegImpl;
};

struct ScopedTmpRegLazy : public ScopedTmpRegImpl<true> {
    using ScopedTmpRegImpl<true>::ScopedTmpRegImpl;
};

struct ScopedTmpRegU16 : public ScopedTmpReg {
    explicit ScopedTmpRegU16(Encoder *encoder) : ScopedTmpReg(encoder, INT16_TYPE) {}
};

struct ScopedTmpRegU32 : public ScopedTmpReg {
    explicit ScopedTmpRegU32(Encoder *encoder) : ScopedTmpReg(encoder, INT32_TYPE) {}
};

struct ScopedTmpRegU64 : public ScopedTmpReg {
    explicit ScopedTmpRegU64(Encoder *encoder) : ScopedTmpReg(encoder, INT64_TYPE) {}
};

struct ScopedTmpRegF32 : public ScopedTmpReg {
    explicit ScopedTmpRegF32(Encoder *encoder) : ScopedTmpReg(encoder, FLOAT32_TYPE) {}
};

struct ScopedTmpRegF64 : public ScopedTmpReg {
    explicit ScopedTmpRegF64(Encoder *encoder) : ScopedTmpReg(encoder, FLOAT64_TYPE) {}
};

struct ScopedTmpRegRef : public ScopedTmpReg {
    explicit ScopedTmpRegRef(Encoder *encoder) : ScopedTmpReg(encoder, encoder->GetRefType()) {}
};

class ScopedLiveTmpReg : public ScopedTmpReg {
public:
    explicit ScopedLiveTmpReg(Encoder *encoder) : ScopedTmpReg(encoder)
    {
        encoder->AddRegInLiveMask(GetReg());
    }
    ScopedLiveTmpReg(Encoder *encoder, TypeInfo type) : ScopedTmpReg(encoder, type)
    {
        encoder->AddRegInLiveMask(GetReg());
    }
    ~ScopedLiveTmpReg() override
    {
        GetEncoder()->RemoveRegFromLiveMask(GetReg());
    }
    NO_COPY_SEMANTIC(ScopedLiveTmpReg);
    NO_MOVE_SEMANTIC(ScopedLiveTmpReg);
};

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_ENCODE_H_
