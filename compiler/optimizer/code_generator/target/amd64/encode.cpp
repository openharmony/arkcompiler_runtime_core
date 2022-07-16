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

#include <iomanip>

#include "compiler/optimizer/code_generator/relocations.h"
#include "target/amd64/target.h"

#include "lib_helpers.inl"

#include "Zydis/Zydis.h"

#ifndef PANDA_TARGET_MACOS
#include "elf.h"
#endif  // PANDA_TARGET_MACOS

namespace panda::compiler::amd64 {
LabelHolder::LabelId Amd64LabelHolder::CreateLabel()
{
    ++id_;

    auto masm = (static_cast<Amd64Encoder *>(GetEncoder()))->GetMasm();
    auto label = masm->newLabel();

    auto allocator = GetEncoder()->GetAllocator();
    labels_.push_back(allocator->New<LabelType>(std::move(label)));
    ASSERT(labels_.size() == id_);
    return id_ - 1;
}

void Amd64LabelHolder::BindLabel(LabelId id)
{
    static_cast<Amd64Encoder *>(GetEncoder())->GetMasm()->bind(*labels_[id]);
}

Amd64Encoder::Amd64Encoder(ArenaAllocator *allocator) : Encoder(allocator, Arch::X86_64, false) {}

Amd64Encoder::~Amd64Encoder()
{
    if (masm_ != nullptr) {
        masm_->~Assembler();
        masm_ = nullptr;
    }

    if (code_holder_ != nullptr) {
        code_holder_->~CodeHolder();
        code_holder_ = nullptr;
    }

    if (error_handler_ != nullptr) {
        error_handler_->~ErrorHandler();
        error_handler_ = nullptr;
    }

    if (labels_ != nullptr) {
        labels_->~Amd64LabelHolder();
        labels_ = nullptr;
    }
}

bool Amd64Encoder::InitMasm()
{
    if (masm_ == nullptr) {
        labels_ = GetAllocator()->New<Amd64LabelHolder>(this);
        if (labels_ == nullptr) {
            SetFalseResult();
            return false;
        }

        asmjit::Environment env;
        env.setArch(asmjit::Environment::kArchX64);

        code_holder_ = GetAllocator()->New<asmjit::CodeHolder>(GetAllocator());
        if (code_holder_ == nullptr) {
            SetFalseResult();
            return false;
        }
        code_holder_->init(env, 0U);

        masm_ = GetAllocator()->New<asmjit::x86::Assembler>(code_holder_);
        if (masm_ == nullptr) {
            SetFalseResult();
            return false;
        }

        // Enable strict validation.
        masm_->addValidationOptions(asmjit::BaseEmitter::kValidationOptionAssembler);
        error_handler_ = GetAllocator()->New<AsmJitErrorHandler>(this);
        if (error_handler_ == nullptr) {
            SetFalseResult();
            return false;
        }
        masm_->setErrorHandler(error_handler_);

        // Make sure that the compiler uses the same scratch registers as the assembler
        CHECK_EQ(compiler::arch_info::x86_64::TEMP_REGS, GetTarget().GetTempRegsMask());
        CHECK_EQ(compiler::arch_info::x86_64::TEMP_FP_REGS, GetTarget().GetTempVRegsMask());
    }
    return true;
}

void Amd64Encoder::Finalize()
{
    auto code = GetMasm()->code();
    auto code_size = code->codeSize();

    code->flatten();
    code->resolveUnresolvedLinks();

    auto code_buffer = GetAllocator()->Alloc(code_size);

    code->relocateToBase(reinterpret_cast<uintptr_t>(code_buffer));
    code->copyFlattenedData(code_buffer, code_size, asmjit::CodeHolder::kCopyPadSectionBuffer);
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id)
{
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->jmp(*label);
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsScalar()) {
        if (src0.GetSize() == src1.GetSize()) {
            GetMasm()->cmp(ArchReg(src0), ArchReg(src1));
        } else if (src0.GetSize() > src1.GetSize()) {
            ScopedTmpReg tmp_reg(this, src0.GetType());
            EncodeCast(tmp_reg, false, src1, false);
            GetMasm()->cmp(ArchReg(src0), ArchReg(tmp_reg));
        } else {
            ScopedTmpReg tmp_reg(this, src1.GetType());
            EncodeCast(tmp_reg, false, src0, false);
            GetMasm()->cmp(ArchReg(tmp_reg), ArchReg(src1));
        }
    } else if (src0.GetType() == FLOAT32_TYPE) {
        GetMasm()->comiss(ArchVReg(src0), ArchVReg(src1));
    } else {
        GetMasm()->comisd(ArchVReg(src0), ArchVReg(src1));
    }

    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    if (src0.IsScalar()) {
        GetMasm()->j(ArchCc(cc, src0.IsFloat()), *label);
        return;
    }

    if (CcMatchesNan(cc)) {
        GetMasm()->jp(*label);
        GetMasm()->j(ArchCc(cc, src0.IsFloat()), *label);
    } else {
        auto end = GetMasm()->newLabel();

        GetMasm()->jp(end);
        GetMasm()->j(ArchCc(cc, src0.IsFloat()), *label);
        GetMasm()->bind(end);
    }
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    auto imm_val = ImmToSignedInt(imm);
    if (imm_val == 0) {
        EncodeJump(id, src, cc);
        return;
    }

    if (ImmFitsSize(imm_val, src.GetSize())) {
        auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);

        GetMasm()->cmp(ArchReg(src), asmjit::imm(imm_val));
        GetMasm()->j(ArchCc(cc), *label);
    } else {
        ScopedTmpReg tmp_reg(this, src.GetType());
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
        EncodeJump(id, src, tmp_reg, cc);
    }
}

void Amd64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar());
    if (src0.GetSize() == src1.GetSize()) {
        GetMasm()->test(ArchReg(src0), ArchReg(src1));
    } else if (src0.GetSize() > src1.GetSize()) {
        ScopedTmpReg tmp_reg(this, src0.GetType());
        EncodeCast(tmp_reg, false, src1, false);
        GetMasm()->test(ArchReg(src0), ArchReg(tmp_reg));
    } else {
        ScopedTmpReg tmp_reg(this, src1.GetType());
        EncodeCast(tmp_reg, false, src0, false);
        GetMasm()->test(ArchReg(tmp_reg), ArchReg(src1));
    }

    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->j(ArchCcTest(cc), *label);
}

void Amd64Encoder::EncodeJumpTest(LabelHolder::LabelId id, Reg src, Imm imm, Condition cc)
{
    ASSERT(src.IsScalar());

    auto imm_val = ImmToSignedInt(imm);
    if (ImmFitsSize(imm_val, src.GetSize())) {
        auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);

        GetMasm()->test(ArchReg(src), asmjit::imm(imm_val));
        GetMasm()->j(ArchCcTest(cc), *label);
    } else {
        ScopedTmpReg tmp_reg(this, src.GetType());
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
        EncodeJumpTest(id, src, tmp_reg, cc);
    }
}

void Amd64Encoder::EncodeJump(LabelHolder::LabelId id, Reg src, Condition cc)
{
    if (src.IsScalar()) {
        auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);

        GetMasm()->cmp(ArchReg(src), asmjit::imm(0));
        GetMasm()->j(ArchCc(cc), *label);
        return;
    }

    ScopedTmpReg tmp_reg(this, src.GetType());
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->xorps(ArchVReg(tmp_reg), ArchVReg(tmp_reg));
    } else {
        GetMasm()->xorpd(ArchVReg(tmp_reg), ArchVReg(tmp_reg));
    }
    EncodeJump(id, src, tmp_reg, cc);
}

void Amd64Encoder::EncodeJump(Reg dst)
{
    GetMasm()->jmp(ArchReg(dst));
}

void Amd64Encoder::EncodeJump(RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    // NOLINTNEXTLINE(readability-magic-numbers)
    std::array<uint8_t, 5U> data = {0xe9, 0, 0, 0, 0};
    GetMasm()->embed(data.data(), data.size());

    constexpr int ADDEND = 4;
    relocation->offset = GetCursorOffset() - ADDEND;
    relocation->addend = -ADDEND;
    relocation->type = R_X86_64_PLT32;
#endif
}

void Amd64Encoder::EncodeBitTestAndBranch(LabelHolder::LabelId id, compiler::Reg reg, uint32_t bit_pos, bool bit_value)
{
    ASSERT(reg.IsScalar() && reg.GetSize() > bit_pos);
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    if (reg.GetSize() == DOUBLE_WORD_SIZE) {
        ScopedTmpRegU64 tmp_reg(this);
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(static_cast<uint64_t>(1) << bit_pos));
        GetMasm()->test(ArchReg(reg), ArchReg(tmp_reg));
    } else {
        GetMasm()->test(ArchReg(reg), asmjit::imm(1U << bit_pos));
    }
    if (bit_value) {
        GetMasm()->j(ArchCc(Condition::NE), *label);
    } else {
        GetMasm()->j(ArchCc(Condition::EQ), *label);
    }
}

void Amd64Encoder::MakeCall([[maybe_unused]] compiler::RelocationInfo *relocation)
{
#ifdef PANDA_TARGET_MACOS
    LOG(FATAL, COMPILER) << "Not supported in Macos build";
#else
    // NOLINTNEXTLINE(readability-magic-numbers)
    const size_t LEN = 5;
    std::array<uint8_t, LEN> data = {0xe8, 0, 0, 0, 0};
    GetMasm()->embed(data.data(), data.size());

    constexpr int APPEND = 4;
    relocation->offset = GetCursorOffset() - APPEND;
    relocation->addend = -APPEND;
    relocation->type = R_X86_64_PLT32;
#endif
}

void Amd64Encoder::MakeCall(LabelHolder::LabelId id)
{
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->call(*label);
}

void Amd64Encoder::MakeCall(const void *entry_point)
{
    ScopedTmpRegU64 tmp_reg(this);
    GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(entry_point));
    GetMasm()->call(ArchReg(tmp_reg));
}

void Amd64Encoder::MakeCall(Reg reg)
{
    GetMasm()->call(ArchReg(reg));
}

void Amd64Encoder::MakeCall(MemRef entry_point)
{
    ScopedTmpRegU64 tmp_reg(this);
    EncodeLdr(tmp_reg, false, entry_point);
    GetMasm()->call(ArchReg(tmp_reg));
}

template <typename Func>
void Amd64Encoder::EncodeRelativePcMov(Reg reg, intptr_t offset, Func encode_instruction)
{
    auto pos = GetMasm()->offset();
    encode_instruction(reg, offset);
    offset -= (GetMasm()->offset() - pos);
    GetMasm()->setOffset(pos);
    encode_instruction(reg, offset);
}

void Amd64Encoder::MakeCallAot(intptr_t offset)
{
    ScopedTmpRegU64 tmp_reg(this);
    EncodeRelativePcMov(tmp_reg, offset, [this](Reg reg, intptr_t offset) {
        GetMasm()->long_().mov(ArchReg(reg), asmjit::x86::ptr(asmjit::x86::rip, offset));
    });
    GetMasm()->call(ArchReg(tmp_reg));
}

bool Amd64Encoder::CanMakeCallByOffset(intptr_t offset)
{
    return offset == static_cast<intptr_t>(static_cast<int32_t>(offset));
}

void Amd64Encoder::MakeCallByOffset(intptr_t offset)
{
    GetMasm()->call(GetCursorOffset() + int32_t(offset));
}

void Amd64Encoder::MakeLoadAotTable(intptr_t offset, Reg reg)
{
    EncodeRelativePcMov(reg, offset, [this](Reg reg, intptr_t offset) {
        GetMasm()->long_().mov(ArchReg(reg), asmjit::x86::ptr(asmjit::x86::rip, offset));
    });
}

void Amd64Encoder::MakeLoadAotTableAddr([[maybe_unused]] intptr_t offset, [[maybe_unused]] Reg addr,
                                        [[maybe_unused]] Reg val)
{
    EncodeRelativePcMov(addr, offset, [this](Reg reg, intptr_t offset) {
        GetMasm()->long_().lea(ArchReg(reg), asmjit::x86::ptr(asmjit::x86::rip, offset));
    });
    GetMasm()->mov(ArchReg(val), asmjit::x86::ptr(ArchReg(addr)));
}

void Amd64Encoder::EncodeAbort()
{
    GetMasm()->int3();
}

void Amd64Encoder::EncodeReturn()
{
    GetMasm()->ret();
}

void Amd64Encoder::EncodeMul([[maybe_unused]] Reg dst, [[maybe_unused]] Reg src, [[maybe_unused]] Imm imm)
{
    SetFalseResult();
}

void Amd64Encoder::EncodeNop()
{
    GetMasm()->nop();
}

void Amd64Encoder::EncodeMov(Reg dst, Reg src)
{
    if (dst == src) {
        return;
    }

    if (dst.IsFloat() != src.IsFloat()) {
        ASSERT(src.GetSize() == dst.GetSize());
        if (dst.GetSize() == WORD_SIZE) {
            if (dst.IsFloat()) {
                GetMasm()->movd(ArchVReg(dst), ArchReg(src));
            } else {
                GetMasm()->movd(ArchReg(dst), ArchVReg(src));
            }
        } else {
            ASSERT(dst.GetSize() == DOUBLE_WORD_SIZE);
            if (dst.IsFloat()) {
                GetMasm()->movq(ArchVReg(dst), ArchReg(src));
            } else {
                GetMasm()->movq(ArchReg(dst), ArchVReg(src));
            }
        }
        return;
    }

    if (dst.IsFloat()) {
        ASSERT(src.IsFloat());
        if (dst.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        return;
    }

    if (dst.GetSize() < WORD_SIZE && dst.GetSize() == src.GetSize()) {
        GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));
    }

    if (dst.GetSize() == src.GetSize()) {
        GetMasm()->mov(ArchReg(dst), ArchReg(src));
    } else {
        EncodeCast(dst, false, src, false);
    }
}

void Amd64Encoder::EncodeNeg(Reg dst, Reg src)
{
    if (dst.IsScalar()) {
        EncodeMov(dst, src);
        GetMasm()->neg(ArchReg(dst));
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF32 tmp(this);
        CopyImmToXmm(tmp, -0.0F);

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->xorps(ArchVReg(dst), ArchVReg(tmp));
    } else {
        ScopedTmpRegF64 tmp(this);
        CopyImmToXmm(tmp, -0.0);

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->xorps(ArchVReg(dst), ArchVReg(tmp));
    }
}

void Amd64Encoder::EncodeAbs(Reg dst, Reg src)
{
    if (dst.IsScalar()) {
        auto size = std::max<uint8_t>(src.GetSize(), WORD_SIZE);

        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), ArchReg(src));
            GetMasm()->neg(ArchReg(dst));
            GetMasm()->cmovl(ArchReg(dst, size), ArchReg(src, size));
        } else if (GetScratchRegistersCount() > 0) {
            ScopedTmpReg tmp_reg(this, dst.GetType());

            GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src));
            GetMasm()->neg(ArchReg(tmp_reg));

            GetMasm()->cmovl(ArchReg(tmp_reg, size), ArchReg(src, size));
            GetMasm()->mov(ArchReg(dst), ArchReg(tmp_reg));
        } else {
            auto end = GetMasm()->newLabel();

            GetMasm()->test(ArchReg(dst), ArchReg(dst));
            GetMasm()->jns(end);

            GetMasm()->neg(ArchReg(dst));
            GetMasm()->bind(end);
        }
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegF32 tmp(this);
        // NOLINTNEXTLINE(readability-magic-numbers)
        CopyImmToXmm(tmp, uint32_t(0x7fffffff));

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->andps(ArchVReg(dst), ArchVReg(tmp));
    } else {
        ScopedTmpRegF64 tmp(this);
        // NOLINTNEXTLINE(readability-magic-numbers)
        CopyImmToXmm(tmp, uint64_t(0x7fffffffffffffff));

        if (dst.GetId() != src.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        GetMasm()->andps(ArchVReg(dst), ArchVReg(tmp));
    }
}

void Amd64Encoder::EncodeNot(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar());

    EncodeMov(dst, src);
    GetMasm()->not_(ArchReg(dst));
}

void Amd64Encoder::EncodeSqrt(Reg dst, Reg src)
{
    ASSERT(dst.IsFloat());
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->sqrtps(ArchVReg(dst), ArchVReg(src));
    } else {
        GetMasm()->sqrtpd(ArchVReg(dst), ArchVReg(src));
    }
}

void Amd64Encoder::EncodeCastFloatToScalar(Reg dst, bool dst_signed, Reg src)
{
    // We DON'T support casts from float32/64 to int8/16 and bool, because this caste is not declared anywhere
    // in other languages and architecture, we do not know what the behavior should be.
    ASSERT(dst.GetSize() >= WORD_SIZE);
    auto end = GetMasm()->newLabel();

    // if src is NaN, then dst = 0
    EncodeCastFloatCheckNan(dst, src, end);

    // For JS number cast we treat Infinity as a zero integer value
    if (IsJsNumberCast() && src.GetType() == FLOAT64_TYPE) {
        static constexpr uint64_t EXP_BIT_MASK = 0x7FF0000000000000L;
        ScopedTmpReg cmp_reg(this, src.GetType());
        ScopedTmpReg tmp_reg(this, src.GetType() == FLOAT64_TYPE ? INT64_TYPE : INT32_TYPE);
        GetMasm()->mov(ArchReg(tmp_reg, DOUBLE_WORD_SIZE), asmjit::imm(EXP_BIT_MASK));
        GetMasm()->movq(ArchVReg(cmp_reg), ArchReg(tmp_reg));
        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(cmp_reg));
        GetMasm()->je(end);
    }

    if (dst_signed) {
        EncodeCastFloatSignCheckRange(dst, src, end);
    } else {
        EncodeCastFloatUnsignCheckRange(dst, src, end);
    }

    if (src.GetType() == FLOAT32_TYPE) {
        if (dst.GetSize() == DOUBLE_WORD_SIZE) {
            EncodeCastFloat32ToUint64(dst, src);
        } else {
            GetMasm()->cvttss2si(ArchReg(dst, DOUBLE_WORD_SIZE), ArchVReg(src));
        }
    } else {
        if (dst.GetSize() == DOUBLE_WORD_SIZE) {
            EncodeCastFloat64ToUint64(dst, src);
        } else {
            GetMasm()->cvttsd2si(ArchReg(dst, DOUBLE_WORD_SIZE), ArchVReg(src));
        }
    }

    GetMasm()->bind(end);
}

void Amd64Encoder::EncodeCastFloat32ToUint64(Reg dst, Reg src)
{
    auto big_number_label = GetMasm()->newLabel();
    auto end_label = GetMasm()->newLabel();
    ScopedTmpReg tmp_reg(this, src.GetType());
    ScopedTmpReg tmp_num(this, dst.GetType());

    // It is max number with max degree that we can load in sign int64
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(dst, WORD_SIZE), asmjit::imm(0x5F000000));
    GetMasm()->movd(ArchVReg(tmp_reg), ArchReg(dst, WORD_SIZE));
    GetMasm()->comiss(ArchVReg(src), ArchVReg(tmp_reg));
    GetMasm()->jnb(big_number_label);

    GetMasm()->cvttss2si(ArchReg(dst), ArchVReg(src));
    GetMasm()->jmp(end_label);

    GetMasm()->bind(big_number_label);
    GetMasm()->subss(ArchVReg(src), ArchVReg(tmp_reg));
    GetMasm()->cvttss2si(ArchReg(dst), ArchVReg(src));
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(tmp_num), asmjit::imm(0x8000000000000000));
    GetMasm()->xor_(ArchReg(dst), ArchReg(tmp_num));
    GetMasm()->bind(end_label);
}

void Amd64Encoder::EncodeCastFloat64ToUint64(Reg dst, Reg src)
{
    auto big_number_label = GetMasm()->newLabel();
    auto end_label = GetMasm()->newLabel();
    ScopedTmpReg tmp_reg(this, src.GetType());
    ScopedTmpReg tmp_num(this, dst.GetType());

    // It is max number with max degree that we can load in sign int64
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(dst), asmjit::imm(0x43E0000000000000));
    GetMasm()->movq(ArchVReg(tmp_reg), ArchReg(dst));
    GetMasm()->comisd(ArchVReg(src), ArchVReg(tmp_reg));
    GetMasm()->jnb(big_number_label);

    GetMasm()->cvttsd2si(ArchReg(dst), ArchVReg(src));
    GetMasm()->jmp(end_label);

    GetMasm()->bind(big_number_label);
    GetMasm()->subsd(ArchVReg(src), ArchVReg(tmp_reg));
    GetMasm()->cvttsd2si(ArchReg(dst), ArchVReg(src));
    // NOLINTNEXTLINE (readability-magic-numbers)
    GetMasm()->mov(ArchReg(tmp_num), asmjit::imm(0x8000000000000000));
    GetMasm()->xor_(ArchReg(dst), ArchReg(tmp_num));
    GetMasm()->bind(end_label);
}

void Amd64Encoder::EncodeCastFloatCheckNan(Reg dst, Reg src, const asmjit::Label &end)
{
    GetMasm()->xor_(ArchReg(dst, DOUBLE_WORD_SIZE), ArchReg(dst, DOUBLE_WORD_SIZE));
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(src));
    } else {
        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(src));
    }
    GetMasm()->jp(end);
}

void Amd64Encoder::EncodeCastFloatSignCheckRange(Reg dst, Reg src, const asmjit::Label &end)
{
    // if src < INT_MIN, then dst = INT_MIN
    // if src >= (INT_MAX + 1), then dst = INT_MAX
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        EncodeCastFloatCheckRange(dst, src, end, INT64_MIN, INT64_MAX);
    } else {
        EncodeCastFloatCheckRange(dst, src, end, INT32_MIN, INT32_MAX);
    }
}

void Amd64Encoder::EncodeCastFloatCheckRange(Reg dst, Reg src, const asmjit::Label &end, const int64_t min_value,
                                             const uint64_t max_value)
{
    ScopedTmpReg cmp_reg(this, src.GetType());
    ScopedTmpReg tmp_reg(this, src.GetType() == FLOAT64_TYPE ? INT64_TYPE : INT32_TYPE);

    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(min_value));
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(bit_cast<uint32_t>(float(min_value))));
        GetMasm()->movd(ArchVReg(cmp_reg), ArchReg(tmp_reg));
        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(cmp_reg));
    } else {
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(bit_cast<uint64_t>(double(min_value))));
        GetMasm()->movq(ArchVReg(cmp_reg), ArchReg(tmp_reg));
        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(cmp_reg));
    }
    GetMasm()->jb(end);

    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(max_value));
    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(bit_cast<uint32_t>(float(max_value) + 1U)));
        GetMasm()->movd(ArchVReg(cmp_reg), ArchReg(tmp_reg));
        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(cmp_reg));
    } else {
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(bit_cast<uint64_t>(double(max_value) + 1U)));
        GetMasm()->movq(ArchVReg(cmp_reg), ArchReg(tmp_reg));
        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(cmp_reg));
    }
    GetMasm()->jae(end);
}

void Amd64Encoder::EncodeCastFloatUnsignCheckRange(Reg dst, Reg src, const asmjit::Label &end)
{
    // if src < 0, then dst = 0
    // if src >= (UINT_MAX + 1), then dst = UINT_MAX
    if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        EncodeCastFloatCheckRange(dst, src, end, 0, UINT64_MAX);
    } else {
        EncodeCastFloatCheckRange(dst, src, end, 0, UINT32_MAX);
    }
}

void Amd64Encoder::EncodeCastScalarToFloatUnsignDouble(Reg dst, Reg src)
{
    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegU64 int1_reg(this);
        ScopedTmpRegU64 int2_reg(this);

        auto sgn = GetMasm()->newLabel();
        auto end = GetMasm()->newLabel();

        GetMasm()->test(ArchReg(src), ArchReg(src));
        GetMasm()->js(sgn);
        GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(src));
        GetMasm()->jmp(end);

        GetMasm()->bind(sgn);
        GetMasm()->mov(ArchReg(int1_reg), ArchReg(src));
        GetMasm()->mov(ArchReg(int2_reg), ArchReg(src));
        GetMasm()->shr(ArchReg(int2_reg), asmjit::imm(1));
        GetMasm()->and_(ArchReg(int1_reg, WORD_SIZE), asmjit::imm(1));
        GetMasm()->or_(ArchReg(int1_reg), ArchReg(int2_reg));
        GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(int1_reg));
        GetMasm()->addss(ArchVReg(dst), ArchVReg(dst));

        GetMasm()->bind(end);
    } else {
        static constexpr std::array<uint32_t, 4> ARR1 = {uint32_t(0x43300000), uint32_t(0x45300000), 0x0, 0x0};
        static constexpr std::array<uint64_t, 2> ARR2 = {uint64_t(0x4330000000000000), uint64_t(0x4530000000000000)};

        ScopedTmpReg float1_reg(this, dst.GetType());
        ScopedTmpRegF64 tmp(this);

        GetMasm()->movq(ArchVReg(float1_reg), ArchReg(src));
        CopyArrayToXmm(tmp, ARR1);
        GetMasm()->punpckldq(ArchVReg(float1_reg), ArchVReg(tmp));
        CopyArrayToXmm(tmp, ARR2);
        GetMasm()->subpd(ArchVReg(float1_reg), ArchVReg(tmp));
        GetMasm()->movapd(ArchVReg(dst), ArchVReg(float1_reg));
        GetMasm()->unpckhpd(ArchVReg(dst), ArchVReg(float1_reg));
        GetMasm()->addsd(ArchVReg(dst), ArchVReg(float1_reg));
    }
}

void Amd64Encoder::EncodeCastScalarToFloat(Reg dst, Reg src, bool src_signed)
{
    if (!src_signed && src.GetSize() == DOUBLE_WORD_SIZE) {
        EncodeCastScalarToFloatUnsignDouble(dst, src);
        return;
    }

    if (src.GetSize() < WORD_SIZE || (src_signed && src.GetSize() == WORD_SIZE)) {
        if (dst.GetType() == FLOAT32_TYPE) {
            GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(src, WORD_SIZE));
        } else {
            GetMasm()->cvtsi2sd(ArchVReg(dst), ArchReg(src, WORD_SIZE));
        }
        return;
    }

    if (!src_signed && src.GetSize() == WORD_SIZE) {
        ScopedTmpRegU64 int1_reg(this);

        GetMasm()->mov(ArchReg(int1_reg, WORD_SIZE), ArchReg(src, WORD_SIZE));
        if (dst.GetType() == FLOAT32_TYPE) {
            GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(int1_reg));
        } else {
            GetMasm()->cvtsi2sd(ArchVReg(dst), ArchReg(int1_reg));
        }
        return;
    }

    ASSERT(src_signed && src.GetSize() == DOUBLE_WORD_SIZE);
    if (dst.GetType() == FLOAT32_TYPE) {
        GetMasm()->cvtsi2ss(ArchVReg(dst), ArchReg(src));
    } else {
        GetMasm()->cvtsi2sd(ArchVReg(dst), ArchReg(src));
    }
}

void Amd64Encoder::EncodeCastToBool(Reg dst, Reg src)
{
    // In ISA says that we only support casts:
    // i32tou1, i64tou1, u32tou1, u64tou1
    ASSERT(src.IsScalar());
    ASSERT(dst.IsScalar());

    // In our ISA minimal type is 32-bit, so bool in 32bit
    GetMasm()->test(ArchReg(src), ArchReg(src));
    // One "mov" will be better, then 2 jump. Else other instructions will overwrite the flags.
    GetMasm()->mov(ArchReg(dst, WORD_SIZE), asmjit::imm(0));
    GetMasm()->setne(ArchReg(dst));
}

void Amd64Encoder::EncodeCast(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    if (src.IsFloat() && dst.IsScalar()) {
        EncodeCastFloatToScalar(dst, dst_signed, src);
        return;
    }

    if (src.IsScalar() && dst.IsFloat()) {
        EncodeCastScalarToFloat(dst, src, src_signed);
        return;
    }

    if (src.IsFloat() && dst.IsFloat()) {
        if (src.GetSize() != dst.GetSize()) {
            if (src.GetType() == FLOAT32_TYPE) {
                GetMasm()->cvtss2sd(ArchVReg(dst), ArchVReg(src));
            } else {
                GetMasm()->cvtsd2ss(ArchVReg(dst), ArchVReg(src));
            }
            return;
        }

        if (src.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src));
        }
        return;
    }

    ASSERT(src.IsScalar() && dst.IsScalar());
    EncodeCastScalar(dst, dst_signed, src, src_signed);
}

void Amd64Encoder::EncodeCastScalar(Reg dst, bool dst_signed, Reg src, bool src_signed)
{
    auto extend_to_32bit = [this](Reg reg, bool is_signed) {
        if (reg.GetSize() < WORD_SIZE) {
            if (is_signed) {
                GetMasm()->movsx(ArchReg(reg, WORD_SIZE), ArchReg(reg));
            } else {
                GetMasm()->movzx(ArchReg(reg, WORD_SIZE), ArchReg(reg));
            }
        }
    };

    if (src.GetSize() >= dst.GetSize()) {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), ArchReg(src, dst.GetSize()));
        }
        extend_to_32bit(dst, dst_signed);
        return;
    }

    if (src_signed) {
        if (dst.GetSize() < DOUBLE_WORD_SIZE) {
            GetMasm()->movsx(ArchReg(dst), ArchReg(src));
            extend_to_32bit(dst, dst_signed);
        } else if (src.GetSize() == WORD_SIZE) {
            GetMasm()->movsxd(ArchReg(dst), ArchReg(src));
        } else {
            GetMasm()->movsx(ArchReg(dst, WORD_SIZE), ArchReg(src));
            GetMasm()->movsxd(ArchReg(dst), ArchReg(dst, WORD_SIZE));
        }
        return;
    }

    if (src.GetSize() == WORD_SIZE) {
        GetMasm()->mov(ArchReg(dst, WORD_SIZE), ArchReg(src));
    } else if (dst.GetSize() == DOUBLE_WORD_SIZE) {
        GetMasm()->movzx(ArchReg(dst, WORD_SIZE), ArchReg(src));
    } else {
        GetMasm()->movzx(ArchReg(dst), ArchReg(src));
        extend_to_32bit(dst, dst_signed);
    }
}

Reg Amd64Encoder::MakeShift(Shift shift)
{
    Reg reg = shift.GetBase();
    ASSERT(reg.IsValid());
    if (reg.IsScalar()) {
        ASSERT(shift.GetType() != ShiftType::INVALID_SHIFT);
        switch (shift.GetType()) {
            case ShiftType::LSL:
                GetMasm()->shl(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            case ShiftType::LSR:
                GetMasm()->shr(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            case ShiftType::ASR:
                GetMasm()->sar(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            case ShiftType::ROR:
                GetMasm()->ror(ArchReg(reg), asmjit::imm(shift.GetScale()));
                break;
            default:
                UNREACHABLE();
        }

        return reg;
    }

    // Invalid register type
    UNREACHABLE();
}

void Amd64Encoder::EncodeAdd(Reg dst, Reg src0, Shift src1)
{
    if (dst.IsFloat()) {
        SetFalseResult();
        return;
    }

    ASSERT(dst.GetSize() >= src0.GetSize());

    auto shift_reg = MakeShift(src1);

    if (src0.GetSize() < WORD_SIZE) {
        EncodeAdd(dst, src0, shift_reg);
        return;
    }

    if (src0.GetSize() == DOUBLE_WORD_SIZE && shift_reg.GetSize() < DOUBLE_WORD_SIZE) {
        GetMasm()->movsxd(ArchReg(shift_reg, DOUBLE_WORD_SIZE), ArchReg(shift_reg));
    }

    GetMasm()->lea(ArchReg(dst), asmjit::x86::ptr(ArchReg(src0), ArchReg(shift_reg, src0.GetSize())));
}

void Amd64Encoder::EncodeAdd(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());
        GetMasm()->lea(ArchReg(dst, size), asmjit::x86::ptr(ArchReg(src0, size), ArchReg(src1, size)));
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->addss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->addss(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->addss(ArchVReg(dst), ArchVReg(src1));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->addsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->addsd(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->addsd(ArchVReg(dst), ArchVReg(src1));
        }
    }
}

void Amd64Encoder::EncodeSub(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->sub(ArchReg(dst), ArchReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->sub(ArchReg(dst), ArchReg(src0));
            GetMasm()->neg(ArchReg(dst));
        } else {
            GetMasm()->mov(ArchReg(dst), ArchReg(src0));
            GetMasm()->sub(ArchReg(dst), ArchReg(src1));
        }
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->subss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->subss(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpReg tmp_reg(this, dst.GetType());
            GetMasm()->movss(ArchVReg(tmp_reg), ArchVReg(src0));
            GetMasm()->subss(ArchVReg(tmp_reg), ArchVReg(src1));
            GetMasm()->movss(ArchVReg(dst), ArchVReg(tmp_reg));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->subsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->subsd(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpReg tmp_reg(this, dst.GetType());
            GetMasm()->movsd(ArchVReg(tmp_reg), ArchVReg(src0));
            GetMasm()->subsd(ArchVReg(tmp_reg), ArchVReg(src1));
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeMul(Reg dst, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());

        if (dst.GetId() == src0.GetId()) {
            GetMasm()->imul(ArchReg(dst, size), ArchReg(src1, size));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->imul(ArchReg(dst, size), ArchReg(src0, size));
        } else {
            GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
            GetMasm()->imul(ArchReg(dst, size), ArchReg(src1, size));
        }
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->mulss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->mulss(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->mulss(ArchVReg(dst), ArchVReg(src1));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->mulsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() == src1.GetId()) {
            GetMasm()->mulsd(ArchVReg(dst), ArchVReg(src0));
        } else {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->mulsd(ArchVReg(dst), ArchVReg(src1));
        }
    }
}

void Amd64Encoder::EncodeAddOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    auto size = dst.GetSize();
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->add(ArchReg(dst, size), ArchReg(src1, size));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->add(ArchReg(dst, size), ArchReg(src0, size));
    } else {
        GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
        GetMasm()->add(ArchReg(dst, size), ArchReg(src1, size));
    }
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->j(ArchCc(cc, false), *label);
}

void Amd64Encoder::EncodeSubOverflow(compiler::LabelHolder::LabelId id, Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(!dst.IsFloat() && !src0.IsFloat() && !src1.IsFloat());
    ASSERT(cc == Condition::VS || cc == Condition::VC);
    auto size = dst.GetSize();
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->sub(ArchReg(dst, size), ArchReg(src1, size));
    } else if (dst.GetId() == src1.GetId()) {
        ScopedTmpReg tmp_reg(this, dst.GetType());
        GetMasm()->mov(ArchReg(tmp_reg, size), ArchReg(src1, size));
        GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
        GetMasm()->sub(ArchReg(dst, size), ArchReg(tmp_reg, size));
    } else {
        GetMasm()->mov(ArchReg(dst, size), ArchReg(src0, size));
        GetMasm()->sub(ArchReg(dst, size), ArchReg(src1, size));
    }
    auto label = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(id);
    GetMasm()->j(ArchCc(cc, false), *label);
}

void Amd64Encoder::EncodeDivFloat(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsFloat());
    if (dst.GetType() == FLOAT32_TYPE) {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->divss(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movss(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->divss(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpRegF32 tmp(this);
            GetMasm()->movss(ArchVReg(tmp), ArchVReg(src0));
            GetMasm()->divss(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movss(ArchVReg(dst), ArchVReg(tmp));
        }
    } else {
        if (dst.GetId() == src0.GetId()) {
            GetMasm()->divsd(ArchVReg(dst), ArchVReg(src1));
        } else if (dst.GetId() != src1.GetId()) {
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(src0));
            GetMasm()->divsd(ArchVReg(dst), ArchVReg(src1));
        } else {
            ScopedTmpRegF64 tmp(this);
            GetMasm()->movsd(ArchVReg(tmp), ArchVReg(src0));
            GetMasm()->divsd(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movsd(ArchVReg(dst), ArchVReg(tmp));
        }
    }
}

void Amd64Encoder::EncodeDiv(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeDivFloat(dst, src0, src1);
        return;
    }

    auto neg_path = GetMasm()->newLabel();
    auto crossroad = GetMasm()->newLabel();

    GetMasm()->cmp(ArchReg(src1), asmjit::imm(-1));
    GetMasm()->je(neg_path);

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->push(asmjit::x86::rdx);
    }
    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->push(asmjit::x86::rax);
    }

    ScopedTmpReg tmp_reg(this, dst.GetType());
    Reg op1 {src1};
    if (src1.GetId() == ConvertRegNumber(asmjit::x86::rax.id()) ||
        src1.GetId() == ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src1));
        op1 = Reg(tmp_reg);
    }

    if (src0.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(src0, DOUBLE_WORD_SIZE));
    }

    if (dst_signed) {
        if (dst.GetSize() <= WORD_SIZE) {
            GetMasm()->cdq();
        } else {
            GetMasm()->cqo();
        }
        GetMasm()->idiv(ArchReg(op1));
    } else {
        GetMasm()->xor_(asmjit::x86::rdx, asmjit::x86::rdx);
        GetMasm()->div(ArchReg(op1));
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::x86::rax);
        GetMasm()->pop(asmjit::x86::rax);
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->pop(asmjit::x86::rdx);
    }
    GetMasm()->jmp(crossroad);

    GetMasm()->bind(neg_path);
    if (dst.GetId() != src0.GetId()) {
        GetMasm()->mov(ArchReg(dst), ArchReg(src0));
    }
    GetMasm()->neg(ArchReg(dst));

    GetMasm()->bind(crossroad);
}

void Amd64Encoder::EncodeModFloat(Reg dst, Reg src0, Reg src1)
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

void Amd64Encoder::EncodeMod(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsFloat()) {
        EncodeModFloat(dst, src0, src1);
        return;
    }

    auto zero_path = GetMasm()->newLabel();
    auto crossroad = GetMasm()->newLabel();

    GetMasm()->cmp(ArchReg(src1), asmjit::imm(-1));
    GetMasm()->je(zero_path);

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->push(asmjit::x86::rax);
    }
    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->push(asmjit::x86::rdx);
    }

    ScopedTmpReg tmp_reg(this, dst.GetType());
    Reg op1 {src1};
    if (src1.GetId() == ConvertRegNumber(asmjit::x86::rax.id()) ||
        src1.GetId() == ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src1));
        op1 = Reg(tmp_reg);
    }

    if (src0.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(src0, DOUBLE_WORD_SIZE));
    }

    if (dst_signed) {
        if (dst.GetSize() <= WORD_SIZE) {
            GetMasm()->cdq();
        } else {
            GetMasm()->cqo();
        }
        GetMasm()->idiv(ArchReg(op1));
    } else {
        GetMasm()->xor_(asmjit::x86::rdx, asmjit::x86::rdx);
        GetMasm()->div(ArchReg(op1));
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rdx.id())) {
        GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::x86::rdx);
        GetMasm()->pop(asmjit::x86::rdx);
    }

    if (dst.GetId() != ConvertRegNumber(asmjit::x86::rax.id())) {
        GetMasm()->pop(asmjit::x86::rax);
    }
    GetMasm()->jmp(crossroad);

    GetMasm()->bind(zero_path);
    GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));

    GetMasm()->bind(crossroad);
}

void Amd64Encoder::EncodeMin(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        ScopedTmpReg tmp_reg(this, dst.GetType());
        GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src1));
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));

        auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
        if (dst_signed) {
            GetMasm()->cmovle(ArchReg(tmp_reg, size), ArchReg(src0, size));
        } else {
            GetMasm()->cmovb(ArchReg(tmp_reg, size), ArchReg(src0, size));
        }
        EncodeMov(dst, tmp_reg);
        return;
    }

    EncodeMinMaxFp<false>(dst, src0, src1);
}

void Amd64Encoder::EncodeMax(Reg dst, bool dst_signed, Reg src0, Reg src1)
{
    if (dst.IsScalar()) {
        ScopedTmpReg tmp_reg(this, dst.GetType());
        GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src1));
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));

        auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
        if (dst_signed) {
            GetMasm()->cmovge(ArchReg(tmp_reg, size), ArchReg(src0, size));
        } else {
            GetMasm()->cmova(ArchReg(tmp_reg, size), ArchReg(src0, size));
        }
        EncodeMov(dst, tmp_reg);
        return;
    }

    EncodeMinMaxFp<true>(dst, src0, src1);
}

template <bool is_max>
void Amd64Encoder::EncodeMinMaxFp(Reg dst, Reg src0, Reg src1)
{
    auto end = GetMasm()->newLabel();
    auto not_equal = GetMasm()->newLabel();
    auto got_nan = GetMasm()->newLabel();
    auto &src_a = dst.GetId() != src1.GetId() ? src0 : src1;
    auto &src_b = src_a.GetId() == src0.GetId() ? src1 : src0;
    if (dst.GetType() == FLOAT32_TYPE) {
        GetMasm()->movaps(ArchVReg(dst), ArchVReg(src_a));
        GetMasm()->ucomiss(ArchVReg(src_b), ArchVReg(src_a));
        GetMasm()->jne(not_equal);
        GetMasm()->jp(got_nan);
        // calculate result for positive/negative zero operands
        if (is_max) {
            GetMasm()->andps(ArchVReg(dst), ArchVReg(src_b));
        } else {
            GetMasm()->orps(ArchVReg(dst), ArchVReg(src_b));
        }
        GetMasm()->jmp(end);
        GetMasm()->bind(got_nan);
        // if any operand is NaN result is NaN
        GetMasm()->por(ArchVReg(dst), ArchVReg(src_b));
        GetMasm()->jmp(end);
        GetMasm()->bind(not_equal);
        if (is_max) {
            GetMasm()->maxss(ArchVReg(dst), ArchVReg(src_b));
        } else {
            GetMasm()->minss(ArchVReg(dst), ArchVReg(src_b));
        }
        GetMasm()->bind(end);
    } else {
        GetMasm()->movapd(ArchVReg(dst), ArchVReg(src_a));
        GetMasm()->ucomisd(ArchVReg(src_b), ArchVReg(src_a));
        GetMasm()->jne(not_equal);
        GetMasm()->jp(got_nan);
        // calculate result for positive/negative zero operands
        if (is_max) {
            GetMasm()->andpd(ArchVReg(dst), ArchVReg(src_b));
        } else {
            GetMasm()->orpd(ArchVReg(dst), ArchVReg(src_b));
        }
        GetMasm()->jmp(end);
        GetMasm()->bind(got_nan);
        // if any operand is NaN result is NaN
        GetMasm()->por(ArchVReg(dst), ArchVReg(src_b));
        GetMasm()->jmp(end);
        GetMasm()->bind(not_equal);
        if (is_max) {
            GetMasm()->maxsd(ArchVReg(dst), ArchVReg(src_b));
        } else {
            GetMasm()->minsd(ArchVReg(dst), ArchVReg(src_b));
        }
        GetMasm()->bind(end);
    }
}

void Amd64Encoder::EncodeShl(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    ScopedTmpReg tmp_reg(this, dst.GetType());
    Reg rcx(ConvertRegNumber(asmjit::x86::rcx.id()), dst.GetType());
    GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src0));
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->push(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(rcx), ArchReg(src1));
    GetMasm()->shl(ArchReg(tmp_reg), asmjit::x86::cl);
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->pop(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(dst), ArchReg(tmp_reg));
}

void Amd64Encoder::EncodeShr(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    ScopedTmpReg tmp_reg(this, dst.GetType());
    Reg rcx(ConvertRegNumber(asmjit::x86::rcx.id()), dst.GetType());
    GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src0));
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->push(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(rcx), ArchReg(src1));
    GetMasm()->shr(ArchReg(tmp_reg), asmjit::x86::cl);
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->pop(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(dst), ArchReg(tmp_reg));
}

void Amd64Encoder::EncodeAShr(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    ScopedTmpReg tmp_reg(this, dst.GetType());
    Reg rcx(ConvertRegNumber(asmjit::x86::rcx.id()), dst.GetType());
    GetMasm()->mov(ArchReg(tmp_reg), ArchReg(src0));
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->push(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(rcx), ArchReg(src1));
    GetMasm()->sar(ArchReg(tmp_reg), asmjit::x86::cl);
    if (dst.GetId() != rcx.GetId()) {
        GetMasm()->pop(ArchReg(rcx, DOUBLE_WORD_SIZE));
    }
    GetMasm()->mov(ArchReg(dst), ArchReg(tmp_reg));
}

void Amd64Encoder::EncodeAnd(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->and_(ArchReg(dst), ArchReg(src1));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->and_(ArchReg(dst), ArchReg(src0));
    } else {
        GetMasm()->mov(ArchReg(dst), ArchReg(src0));
        GetMasm()->and_(ArchReg(dst), ArchReg(src1));
    }
}

void Amd64Encoder::EncodeOr(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->or_(ArchReg(dst), ArchReg(src1));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->or_(ArchReg(dst), ArchReg(src0));
    } else {
        GetMasm()->mov(ArchReg(dst), ArchReg(src0));
        GetMasm()->or_(ArchReg(dst), ArchReg(src1));
    }
}

void Amd64Encoder::EncodeXor(Reg dst, Reg src0, Reg src1)
{
    ASSERT(dst.IsScalar());
    if (dst.GetId() == src0.GetId()) {
        GetMasm()->xor_(ArchReg(dst), ArchReg(src1));
    } else if (dst.GetId() == src1.GetId()) {
        GetMasm()->xor_(ArchReg(dst), ArchReg(src0));
    } else {
        GetMasm()->mov(ArchReg(dst), ArchReg(src0));
        GetMasm()->xor_(ArchReg(dst), ArchReg(src1));
    }
}

void Amd64Encoder::EncodeAdd(Reg dst, Reg src, Imm imm)
{
    if (dst.IsFloat()) {
        SetFalseResult();
        return;
    }

    auto imm_val = ImmToSignedInt(imm);
    auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());
    if (ImmFitsSize(imm_val, size)) {
        GetMasm()->lea(ArchReg(dst, size), asmjit::x86::ptr(ArchReg(src, size), imm_val));
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(imm_val));
            GetMasm()->add(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmp_reg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
            GetMasm()->add(ArchReg(dst), ArchReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeSub(Reg dst, Reg src, Imm imm)
{
    if (dst.IsFloat()) {
        SetFalseResult();
        return;
    }

    auto imm_val = -ImmToSignedInt(imm);
    auto size = std::max<uint8_t>(WORD_SIZE, dst.GetSize());
    if (ImmFitsSize(imm_val, size)) {
        GetMasm()->lea(ArchReg(dst, size), asmjit::x86::ptr(ArchReg(src, size), imm_val));
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(imm_val));
            GetMasm()->add(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmp_reg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
            GetMasm()->add(ArchReg(dst), ArchReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeShl(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    EncodeMov(dst, src);
    GetMasm()->shl(ArchReg(dst), ArchImm(imm));
}

void Amd64Encoder::EncodeShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());

    EncodeMov(dst, src);
    GetMasm()->shr(ArchReg(dst), ArchImm(imm));
}

void Amd64Encoder::EncodeAShr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    EncodeMov(dst, src);
    GetMasm()->sar(ArchReg(dst), ArchImm(imm));
}

void Amd64Encoder::EncodeAnd(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    auto imm_val = ImmToUnsignedInt(imm);

    switch (imm.GetSize()) {
        case BYTE_SIZE:
            imm_val |= ~uint64_t(0xFF);  // NOLINT
            break;
        case HALF_SIZE:
            imm_val |= ~uint64_t(0xFFFF);  // NOLINT
            break;
        case WORD_SIZE:
            imm_val |= ~uint64_t(0xFFFFFFFF);  // NOLINT
            break;
        default:
            break;
    }

    if (dst.GetSize() != DOUBLE_WORD_SIZE) {
        // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
        imm_val &= (uint64_t(1) << dst.GetSize()) - 1;
    }

    if (ImmFitsSize(imm_val, dst.GetSize())) {
        EncodeMov(dst, src);
        GetMasm()->and_(ArchReg(dst), imm_val);
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(imm_val));
            GetMasm()->and_(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmp_reg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
            GetMasm()->and_(ArchReg(dst), ArchReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeOr(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    auto imm_val = ImmToUnsignedInt(imm);

    if (ImmFitsSize(imm_val, dst.GetSize())) {
        EncodeMov(dst, src);
        GetMasm()->or_(ArchReg(dst), imm_val);
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(imm_val));
            GetMasm()->or_(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmp_reg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
            GetMasm()->or_(ArchReg(dst), ArchReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeXor(Reg dst, Reg src, Imm imm)
{
    ASSERT(dst.IsScalar());
    auto imm_val = ImmToUnsignedInt(imm);

    if (ImmFitsSize(imm_val, dst.GetSize())) {
        EncodeMov(dst, src);
        GetMasm()->xor_(ArchReg(dst), imm_val);
    } else {
        if (dst.GetId() != src.GetId()) {
            GetMasm()->mov(ArchReg(dst), asmjit::imm(imm_val));
            GetMasm()->xor_(ArchReg(dst), ArchReg(src));
        } else {
            ScopedTmpReg tmp_reg(this, dst.GetType());
            GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
            GetMasm()->xor_(ArchReg(dst), ArchReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeMov(Reg dst, Imm src)
{
    if (dst.IsScalar()) {
        if (dst.GetSize() < WORD_SIZE) {
            GetMasm()->xor_(ArchReg(dst, WORD_SIZE), ArchReg(dst, WORD_SIZE));
        }
        GetMasm()->mov(ArchReg(dst), ArchImm(src));
        return;
    }

    if (dst.GetType() == FLOAT32_TYPE) {
        ScopedTmpRegU32 tmp_reg(this);
        auto val = bit_cast<uint32_t>(src.GetValue<float>());
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(val));
        GetMasm()->movd(ArchVReg(dst), ArchReg(tmp_reg));
    } else {
        ScopedTmpRegU64 tmp_reg(this);
        auto val = bit_cast<uint64_t>(src.GetValue<double>());
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(val));
        GetMasm()->movq(ArchVReg(dst), ArchReg(tmp_reg));
    }
}

void Amd64Encoder::EncodeLdr(Reg dst, bool dst_signed, MemRef mem)
{
    auto m = ArchMem(mem).Prepare(GetMasm());

    if (dst.GetType() == FLOAT32_TYPE) {
        GetMasm()->movss(ArchVReg(dst), m);
        return;
    }
    if (dst.GetType() == FLOAT64_TYPE) {
        GetMasm()->movsd(ArchVReg(dst), m);
        return;
    }

    m.setSize(dst.GetSize() / BITS_PER_BYTE);

    if (dst_signed && dst.GetSize() < DOUBLE_WORD_SIZE) {
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->movsxd(ArchReg(dst, DOUBLE_WORD_SIZE), m);
        } else {
            GetMasm()->movsx(ArchReg(dst, DOUBLE_WORD_SIZE), m);
        }
        return;
    }
    if (!dst_signed && dst.GetSize() < WORD_SIZE) {
        GetMasm()->movzx(ArchReg(dst, WORD_SIZE), m);
        return;
    }

    GetMasm()->mov(ArchReg(dst), m);
}

void Amd64Encoder::EncodeLdrAcquire(Reg dst, bool dst_signed, MemRef mem)
{
    EncodeLdr(dst, dst_signed, mem);
    // LoadLoad and LoadStore barrier should be here, but this is no-op in amd64 memory model
}

void Amd64Encoder::EncodeStr(Reg src, MemRef mem)
{
    auto m = ArchMem(mem).Prepare(GetMasm());

    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->movss(m, ArchVReg(src));
        return;
    }
    if (src.GetType() == FLOAT64_TYPE) {
        GetMasm()->movsd(m, ArchVReg(src));
        return;
    }

    m.setSize(src.GetSize() / BITS_PER_BYTE);
    GetMasm()->mov(m, ArchReg(src));
}

void Amd64Encoder::EncodeStrRelease(Reg src, MemRef mem)
{
    // StoreStore barrier should be here, but this is no-op in amd64 memory model
    EncodeStr(src, mem);
    // this is StoreLoad barrier (which is also full memory barrier in amd64 memory model)
    GetMasm()->lock().add(asmjit::x86::dword_ptr(asmjit::x86::rsp), asmjit::imm(0));
}

void Amd64Encoder::EncodeStrz(Reg src, MemRef mem)
{
    if (src.IsScalar()) {
        if (src.GetSize() == DOUBLE_WORD_SIZE) {
            GetMasm()->mov(ArchMem(mem).Prepare(GetMasm()), ArchReg(src));
        } else {
            ScopedTmpRegU64 tmp_reg(this);
            GetMasm()->xor_(ArchReg(tmp_reg), ArchReg(tmp_reg));
            GetMasm()->mov(ArchReg(tmp_reg, src.GetSize()), ArchReg(src));
            GetMasm()->mov(ArchMem(mem).Prepare(GetMasm()), ArchReg(tmp_reg));
        }
    } else {
        if (src.GetType() == FLOAT64_TYPE) {
            GetMasm()->movsd(ArchMem(mem).Prepare(GetMasm()), ArchVReg(src));
        } else {
            ScopedTmpRegF64 tmp_reg(this);

            GetMasm()->xorpd(ArchVReg(tmp_reg), ArchVReg(tmp_reg));
            GetMasm()->movss(ArchVReg(tmp_reg), ArchVReg(src));
            GetMasm()->movsd(ArchMem(mem).Prepare(GetMasm()), ArchVReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeSti(Imm src, MemRef mem)
{
    if (src.IsFloat()) {
        if (src.GetType() == FLOAT32_TYPE) {
            EncodeSti(Imm(bit_cast<int32_t>(src.GetValue<float>())), mem);
        } else {
            EncodeSti(Imm(bit_cast<int64_t>(src.GetValue<double>())), mem);
        }
        return;
    }

    auto m = ArchMem(mem).Prepare(GetMasm());
    if (src.GetSize() <= HALF_SIZE) {
        m.setSize(src.GetSize() / BITS_PER_BYTE);
        GetMasm()->mov(m, ArchImm(src));
    } else {
        m.setSize(DOUBLE_WORD_SIZE_BYTE);

        auto imm_val = ImmToSignedInt(src);
        if (ImmFitsSize(imm_val, DOUBLE_WORD_SIZE)) {
            GetMasm()->mov(m, asmjit::imm(imm_val));
        } else {
            ScopedTmpRegU64 tmp_reg(this);
            GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
            GetMasm()->mov(m, ArchReg(tmp_reg));
        }
    }
}

void Amd64Encoder::EncodeMemCopy(MemRef mem_from, MemRef mem_to, size_t size)
{
    ScopedTmpRegU64 tmp_reg(this);
    GetMasm()->mov(ArchReg(tmp_reg, size), ArchMem(mem_from).Prepare(GetMasm()));
    GetMasm()->mov(ArchMem(mem_to).Prepare(GetMasm()), ArchReg(tmp_reg, size));
}

void Amd64Encoder::EncodeMemCopyz(MemRef mem_from, MemRef mem_to, size_t size)
{
    ScopedTmpRegU64 tmp_reg(this);
    if (size < DOUBLE_WORD_SIZE) {
        GetMasm()->xor_(ArchReg(tmp_reg), ArchReg(tmp_reg));
    }
    GetMasm()->mov(ArchReg(tmp_reg, size), ArchMem(mem_from).Prepare(GetMasm()));
    GetMasm()->mov(ArchMem(mem_to).Prepare(GetMasm()), ArchReg(tmp_reg));
}

void Amd64Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc)
{
    if (src0.IsScalar()) {
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));
    } else {
        if (src0.GetType() == FLOAT32_TYPE) {
            GetMasm()->ucomiss(ArchVReg(src0), ArchVReg(src1));
        } else {
            GetMasm()->ucomisd(ArchVReg(src0), ArchVReg(src1));
        }
    }
    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(0));

    if (src0.IsScalar()) {
        GetMasm()->set(ArchCc(cc), ArchReg(dst, BYTE_SIZE));
        return;
    }

    auto end = GetMasm()->newLabel();

    if (CcMatchesNan(cc)) {
        GetMasm()->setp(ArchReg(dst, BYTE_SIZE));
    }
    GetMasm()->jp(end);
    GetMasm()->set(ArchCc(cc, true), ArchReg(dst, BYTE_SIZE));

    GetMasm()->bind(end);
}

void Amd64Encoder::EncodeCompareTest(Reg dst, Reg src0, Reg src1, Condition cc)
{
    ASSERT(src0.IsScalar());

    GetMasm()->test(ArchReg(src0), ArchReg(src1));

    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(0));
    GetMasm()->set(ArchCcTest(cc), ArchReg(dst, BYTE_SIZE));
}

void Amd64Encoder::EncodeCmp(Reg dst, Reg src0, Reg src1, Condition cc)
{
    auto end = GetMasm()->newLabel();

    if (src0.IsFloat()) {
        ASSERT(src1.IsFloat());
        ASSERT(cc == Condition::MI || cc == Condition::LT);

        if (src0.GetType() == FLOAT32_TYPE) {
            GetMasm()->ucomiss(ArchVReg(src0), ArchVReg(src1));
        } else {
            GetMasm()->ucomisd(ArchVReg(src0), ArchVReg(src1));
        }

        GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), cc == Condition::LT ? asmjit::imm(-1) : asmjit::imm(1));
        cc = Condition::LO;

        GetMasm()->jp(end);
    } else {
        ASSERT(src0.IsScalar() && src1.IsScalar());
        ASSERT(cc == Condition::LO || cc == Condition::LT);
        GetMasm()->cmp(ArchReg(src0), ArchReg(src1));
    }
    GetMasm()->mov(ArchReg(dst, DOUBLE_WORD_SIZE), asmjit::imm(0));
    GetMasm()->setne(ArchReg(dst, BYTE_SIZE));

    GetMasm()->j(asmjit::x86::Condition::negate(ArchCc(cc)), end);
    GetMasm()->neg(ArchReg(dst));

    GetMasm()->bind(end);
}

void Amd64Encoder::EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat());
    if (src2.IsScalar()) {
        GetMasm()->cmp(ArchReg(src2), ArchReg(src3));
    } else if (src2.GetType() == FLOAT32_TYPE) {
        GetMasm()->comiss(ArchVReg(src2), ArchVReg(src3));
    } else {
        GetMasm()->comisd(ArchVReg(src2), ArchVReg(src3));
    }

    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dst_aliased = dst.GetId() == src0.GetId();
    ScopedTmpReg tmp_reg(this, dst.GetType());
    auto dst_reg = dst_aliased ? ArchReg(tmp_reg, size) : ArchReg(dst, size);

    GetMasm()->mov(dst_reg, ArchReg(src1, size));

    if (src2.IsScalar()) {
        GetMasm()->cmov(ArchCc(cc), dst_reg, ArchReg(src0, size));
    } else if (CcMatchesNan(cc)) {
        GetMasm()->cmovp(dst_reg, ArchReg(src0, size));
        GetMasm()->cmov(ArchCc(cc, src2.IsFloat()), dst_reg, ArchReg(src0, size));
    } else {
        auto end = GetMasm()->newLabel();

        GetMasm()->jp(end);
        GetMasm()->cmov(ArchCc(cc, src2.IsFloat()), dst_reg, ArchReg(src0, size));

        GetMasm()->bind(end);
    }
    if (dst_aliased) {
        EncodeMov(dst, tmp_reg);
    }
}

void Amd64Encoder::EncodeSelect(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    auto imm_val = ImmToSignedInt(imm);
    if (ImmFitsSize(imm_val, src2.GetSize())) {
        GetMasm()->cmp(ArchReg(src2), asmjit::imm(imm_val));
    } else {
        ScopedTmpReg tmp_reg(this, src2.GetType());
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
        GetMasm()->cmp(ArchReg(src2), ArchReg(tmp_reg));
    }

    ScopedTmpReg tmp_reg(this, dst.GetType());
    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dst_aliased = dst.GetId() == src0.GetId();
    auto dst_reg = dst_aliased ? ArchReg(tmp_reg, size) : ArchReg(dst, size);

    GetMasm()->mov(dst_reg, ArchReg(src1, size));
    GetMasm()->cmov(ArchCc(cc), dst_reg, ArchReg(src0, size));
    if (dst_aliased) {
        EncodeMov(dst, tmp_reg);
    }
}

void Amd64Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Reg src3, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    GetMasm()->test(ArchReg(src2), ArchReg(src3));

    ScopedTmpReg tmp_reg(this, dst.GetType());
    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dst_aliased = dst.GetId() == src0.GetId();
    auto dst_reg = dst_aliased ? ArchReg(tmp_reg, size) : ArchReg(dst, size);

    GetMasm()->mov(dst_reg, ArchReg(src1, size));
    GetMasm()->cmov(ArchCcTest(cc), dst_reg, ArchReg(src0, size));
    if (dst_aliased) {
        EncodeMov(dst, tmp_reg);
    }
}

void Amd64Encoder::EncodeSelectTest(Reg dst, Reg src0, Reg src1, Reg src2, Imm imm, Condition cc)
{
    ASSERT(!src0.IsFloat() && !src1.IsFloat() && !src2.IsFloat());

    auto imm_val = ImmToSignedInt(imm);
    if (ImmFitsSize(imm_val, src2.GetSize())) {
        GetMasm()->test(ArchReg(src2), asmjit::imm(imm_val));
    } else {
        ScopedTmpReg tmp_reg(this, src2.GetType());
        GetMasm()->mov(ArchReg(tmp_reg), asmjit::imm(imm_val));
        GetMasm()->test(ArchReg(src2), ArchReg(tmp_reg));
    }

    ScopedTmpReg tmp_reg(this, dst.GetType());
    auto size = std::max<uint8_t>(src0.GetSize(), WORD_SIZE);
    bool dst_aliased = dst.GetId() == src0.GetId();
    auto dst_reg = dst_aliased ? ArchReg(tmp_reg, size) : ArchReg(dst, size);

    GetMasm()->mov(dst_reg, ArchReg(src1, size));
    GetMasm()->cmov(ArchCcTest(cc), dst_reg, ArchReg(src0, size));
    if (dst_aliased) {
        EncodeMov(dst, tmp_reg);
    }
}

void Amd64Encoder::EncodeLdp(Reg dst0, Reg dst1, bool dst_signed, MemRef mem)
{
    ASSERT(dst0.IsFloat() == dst1.IsFloat());
    ASSERT(dst0.GetSize() == dst1.GetSize());

    auto m = ArchMem(mem).Prepare(GetMasm());

    if (dst0.IsFloat()) {
        if (dst0.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(ArchVReg(dst0), m);

            m.addOffset(WORD_SIZE_BYTE);
            GetMasm()->movss(ArchVReg(dst1), m);
        } else {
            GetMasm()->movsd(ArchVReg(dst0), m);

            m.addOffset(DOUBLE_WORD_SIZE_BYTE);
            GetMasm()->movsd(ArchVReg(dst1), m);
        }
        return;
    }

    if (dst_signed && dst0.GetSize() == WORD_SIZE) {
        m.setSize(WORD_SIZE_BYTE);
        GetMasm()->movsxd(ArchReg(dst0, DOUBLE_WORD_SIZE), m);

        m.addOffset(WORD_SIZE_BYTE);
        GetMasm()->movsxd(ArchReg(dst1, DOUBLE_WORD_SIZE), m);
        return;
    }

    GetMasm()->mov(ArchReg(dst0), m);

    m.addOffset(dst0.GetSize() / BITS_PER_BYTE);
    GetMasm()->mov(ArchReg(dst1), m);
}

void Amd64Encoder::EncodeStp(Reg src0, Reg src1, MemRef mem)
{
    ASSERT(src0.IsFloat() == src1.IsFloat());
    ASSERT(src0.GetSize() == src1.GetSize());

    auto m = ArchMem(mem).Prepare(GetMasm());

    if (src0.IsFloat()) {
        if (src0.GetType() == FLOAT32_TYPE) {
            GetMasm()->movss(m, ArchVReg(src0));

            m.addOffset(WORD_SIZE_BYTE);
            GetMasm()->movss(m, ArchVReg(src1));
        } else {
            GetMasm()->movsd(m, ArchVReg(src0));

            m.addOffset(DOUBLE_WORD_SIZE_BYTE);
            GetMasm()->movsd(m, ArchVReg(src1));
        }
        return;
    }

    GetMasm()->mov(m, ArchReg(src0));

    m.addOffset(src0.GetSize() / BITS_PER_BYTE);
    GetMasm()->mov(m, ArchReg(src1));
}

void Amd64Encoder::EncodeReverseBytes(Reg dst, Reg src)
{
    ASSERT(src.GetSize() > BYTE_SIZE);
    ASSERT(src.GetSize() == dst.GetSize());

    if (src != dst) {
        GetMasm()->mov(ArchReg(dst), ArchReg(src));
    }

    if (src.GetSize() == HALF_SIZE) {
        GetMasm()->rol(ArchReg(dst), BYTE_SIZE);
        GetMasm()->movsx(ArchReg(dst, WORD_SIZE), ArchReg(dst));
    } else {
        GetMasm()->bswap(ArchReg(dst));
    }
}

bool Amd64Encoder::CanEncodeImmAddSubCmp(int64_t imm, uint32_t size, [[maybe_unused]] bool signed_compare)
{
    return ImmFitsSize(imm, size);
}

void Amd64Encoder::EncodeBitCount(Reg dst0, Reg src0)
{
    ASSERT(src0.GetSize() == WORD_SIZE || src0.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(dst0.GetSize() == WORD_SIZE);
    ASSERT(src0.IsScalar() && dst0.IsScalar());

    GetMasm()->popcnt(ArchReg(dst0, src0.GetSize()), ArchReg(src0));
}

void Amd64Encoder::EncodeCountLeadingZeroBits(Reg dst, Reg src)
{
    auto end = CreateLabel();
    auto zero = CreateLabel();
    EncodeJump(zero, src, Condition::EQ);
    GetMasm()->bsr(ArchReg(dst), ArchReg(src));
    GetMasm()->xor_(ArchReg(dst), asmjit::imm(dst.GetSize() - 1));
    EncodeJump(end);

    BindLabel(zero);
    GetMasm()->mov(ArchReg(dst), asmjit::imm(dst.GetSize()));

    BindLabel(end);
}

void Amd64Encoder::EncodeCountTrailingZeroBits(Reg dst, Reg src)
{
    ScopedTmpReg tmp(this, src.GetType());
    GetMasm()->bsf(ArchReg(tmp), ArchReg(src));
    GetMasm()->mov(ArchReg(dst), asmjit::imm(dst.GetSize()));
    GetMasm()->cmovne(ArchReg(dst), ArchReg(tmp));
}

void Amd64Encoder::EncodeCeil(Reg dst, Reg src)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    GetMasm()->roundsd(ArchVReg(dst), ArchVReg(src), asmjit::imm(2));
}

void Amd64Encoder::EncodeFloor(Reg dst, Reg src)
{
    GetMasm()->roundsd(ArchVReg(dst), ArchVReg(src), asmjit::imm(1));
}

void Amd64Encoder::EncodeRint(Reg dst, Reg src)
{
    GetMasm()->roundsd(ArchVReg(dst), ArchVReg(src), asmjit::imm(0));
}

void Amd64Encoder::EncodeRound(Reg dst, Reg src)
{
    ScopedTmpReg t1(this, src.GetType());
    ScopedTmpReg t2(this, src.GetType());
    ScopedTmpReg t3(this, src.GetType());
    ScopedTmpReg t4(this, dst.GetType());

    auto skip_incr_id = CreateLabel();
    auto done_id = CreateLabel();

    auto skip_incr = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(skip_incr_id);
    auto done = static_cast<Amd64LabelHolder *>(GetLabels())->GetLabel(done_id);

    if (src.GetType() == FLOAT32_TYPE) {
        GetMasm()->movss(ArchVReg(t2), ArchVReg(src));
        GetMasm()->roundss(ArchVReg(t1), ArchVReg(src), asmjit::imm(1));
        GetMasm()->subss(ArchVReg(t2), ArchVReg(t1));
        // NOLINTNEXTLINE(readability-magic-numbers)
        const auto HALF_F = 0.5F;
        GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int32_t, float>(HALF_F)));
        GetMasm()->movd(ArchVReg(t3), ArchReg(t4));
        GetMasm()->comiss(ArchVReg(t2), ArchVReg(t3));
        GetMasm()->j(asmjit::x86::Condition::Code::kB, *skip_incr);
        // NOLINTNEXTLINE(readability-magic-numbers)
        const auto ONE_F = 1.0F;
        GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int32_t, float>(ONE_F)));
        GetMasm()->movd(ArchVReg(t3), ArchReg(t4));
        GetMasm()->addss(ArchVReg(t1), ArchVReg(t3));
        BindLabel(skip_incr_id);

        // NOLINTNEXTLINE(readability-magic-numbers)
        GetMasm()->mov(ArchReg(dst), asmjit::imm(0x7FFFFFFF));
        GetMasm()->cvtsi2ss(ArchVReg(t2), ArchReg(dst));
        GetMasm()->comiss(ArchVReg(t1), ArchVReg(t2));
        GetMasm()->j(asmjit::x86::Condition::Code::kAE,
                     *done);                           // clipped to max (already in dst), does not jump on unordered
        GetMasm()->mov(ArchReg(dst), asmjit::imm(0));  // does not change flags
        GetMasm()->j(asmjit::x86::Condition::Code::kParityEven, *done);  // NaN mapped to 0 (just moved in dst)
        GetMasm()->cvttss2si(ArchReg(dst), ArchVReg(t1));
        BindLabel(done_id);
    } else if (src.GetType() == FLOAT64_TYPE) {
        GetMasm()->movsd(ArchVReg(t2), ArchVReg(src));
        GetMasm()->roundsd(ArchVReg(t1), ArchVReg(src), asmjit::imm(1));
        GetMasm()->subsd(ArchVReg(t2), ArchVReg(t1));
        // NOLINTNEXTLINE(readability-magic-numbers)
        const auto HALF = 0.5;
        GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int64_t, double>(HALF)));
        GetMasm()->movq(ArchVReg(t3), ArchReg(t4));
        GetMasm()->comisd(ArchVReg(t2), ArchVReg(t3));
        GetMasm()->j(asmjit::x86::Condition::Code::kB, *skip_incr);
        // NOLINTNEXTLINE(readability-magic-numbers)
        const auto ONE = 1.0;
        GetMasm()->mov(ArchReg(t4), asmjit::imm(bit_cast<int64_t, double>(ONE)));
        GetMasm()->movq(ArchVReg(t3), ArchReg(t4));
        GetMasm()->addsd(ArchVReg(t1), ArchVReg(t3));
        BindLabel(skip_incr_id);

        // NOLINTNEXTLINE(readability-magic-numbers)
        GetMasm()->mov(ArchReg(dst), asmjit::imm(0x7FFFFFFFFFFFFFFFL));
        GetMasm()->cvtsi2sd(ArchVReg(t2), ArchReg(dst));
        GetMasm()->comisd(ArchVReg(t1), ArchVReg(t2));
        GetMasm()->j(asmjit::x86::Condition::Code::kAE,
                     *done);                           // clipped to max (already in dst), does not jump on unordered
        GetMasm()->mov(ArchReg(dst), asmjit::imm(0));  // does not change flags
        GetMasm()->j(asmjit::x86::Condition::Code::kParityEven, *done);  // NaN mapped to 0 (just moved in dst)
        GetMasm()->cvttsd2si(ArchReg(dst), ArchVReg(t1));
        BindLabel(done_id);
    } else {
        UNREACHABLE();
    }
}

template <typename T>
void Amd64Encoder::EncodeReverseBitsImpl(Reg dst0, Reg src0)
{
    ASSERT(std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed);
    [[maybe_unused]] constexpr auto IMM_8 = 8;
    ASSERT(sizeof(T) * IMM_8 == dst0.GetSize());
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr T MASKS[] = {static_cast<T>(UINT64_C(0x5555555555555555)),
                                  static_cast<T>(UINT64_C(0x3333333333333333)),
                                  static_cast<T>(UINT64_C(0x0f0f0f0f0f0f0f0f))};

    ScopedTmpReg tmp(this, dst0.GetType());
    ScopedTmpReg imm_holder(this, dst0.GetType());
    auto imm_holder_reg = ArchReg(imm_holder);

    GetMasm()->mov(ArchReg(dst0), ArchReg(src0));
    GetMasm()->mov(ArchReg(tmp), ArchReg(src0));
    constexpr auto MAX_ROUNDS = 3;
    for (uint64_t round = 0; round < MAX_ROUNDS; round++) {
        auto shift = 1U << round;
        auto mask = asmjit::imm(MASKS[round]);
        GetMasm()->shr(ArchReg(dst0), shift);
        if (dst0.GetSize() == DOUBLE_WORD_SIZE) {
            GetMasm()->mov(imm_holder_reg, mask);
            GetMasm()->and_(ArchReg(tmp), imm_holder_reg);
            GetMasm()->and_(ArchReg(dst0), imm_holder_reg);
        } else {
            GetMasm()->and_(ArchReg(tmp), mask);
            GetMasm()->and_(ArchReg(dst0), mask);
        }
        GetMasm()->shl(ArchReg(tmp), shift);
        GetMasm()->or_(ArchReg(dst0), ArchReg(tmp));
        constexpr auto ROUND_2 = 2;
        if (round != ROUND_2) {
            GetMasm()->mov(ArchReg(tmp), ArchReg(dst0));
        }
    }

    GetMasm()->bswap(ArchReg(dst0));
}

void Amd64Encoder::EncodeReverseBits(Reg dst0, Reg src0)
{
    ASSERT(src0.GetSize() == WORD_SIZE || src0.GetSize() == DOUBLE_WORD_SIZE);
    ASSERT(src0.GetSize() == dst0.GetSize());

    if (src0.GetSize() == WORD_SIZE) {
        EncodeReverseBitsImpl<uint32_t>(dst0, src0);
        return;
    }

    EncodeReverseBitsImpl<uint64_t>(dst0, src0);
}

bool Amd64Encoder::CanEncodeScale(uint64_t imm, [[maybe_unused]] uint32_t size)
{
    return imm <= 3U;
}

bool Amd64Encoder::CanEncodeImmLogical(uint64_t imm, uint32_t size)
{
    return ImmFitsSize(imm, size);
}

bool Amd64Encoder::CanEncodeBitCount()
{
    return asmjit::CpuInfo::host().hasFeature(asmjit::x86::Features::kPOPCNT);
}

void Amd64Encoder::EncodeIsInf(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    GetMasm()->xor_(ArchReg(dst, DOUBLE_WORD_SIZE), ArchReg(dst, DOUBLE_WORD_SIZE));

    if (src.GetSize() == WORD_SIZE) {
        constexpr auto INF_MASK = uint32_t(0x7f800000) << 1U;

        ScopedTmpRegU32 tmp_reg(this);
        ScopedTmpRegU32 tmp1_reg(this);
        auto tmp = ArchReg(tmp_reg);
        auto tmp1 = ArchReg(tmp1_reg);

        GetMasm()->movd(tmp1, ArchVReg(src));
        GetMasm()->shl(tmp1, 1);
        GetMasm()->mov(tmp, INF_MASK);
        GetMasm()->cmp(tmp, tmp1);
    } else {
        constexpr auto INF_MASK = uint64_t(0x7ff0000000000000) << 1U;

        ScopedTmpRegU64 tmp_reg(this);
        ScopedTmpRegU64 tmp1_reg(this);
        auto tmp = ArchReg(tmp_reg);
        auto tmp1 = ArchReg(tmp1_reg);

        GetMasm()->movq(tmp1, ArchVReg(src));
        GetMasm()->shl(tmp1, 1);

        GetMasm()->mov(tmp, INF_MASK);
        GetMasm()->cmp(tmp, tmp1);
    }

    GetMasm()->sete(ArchReg(dst, BYTE_SIZE));
}

/* Since NaNs have to be canonicalized we compare the
 * input with itself, if it is NaN the comparison will
 * set the parity flag (PF) */
void Amd64Encoder::EncodeFpToBits(Reg dst, Reg src)
{
    ASSERT(dst.IsScalar() && src.IsFloat());

    if (dst.GetType() == INT32_TYPE) {
        ASSERT(src.GetSize() == WORD_SIZE);

        constexpr auto FLOAT_NAN = uint32_t(0x7fc00000);

        ScopedTmpRegU32 tmp(this);

        GetMasm()->ucomiss(ArchVReg(src), ArchVReg(src));
        GetMasm()->mov(ArchReg(tmp), FLOAT_NAN);
        GetMasm()->movd(ArchReg(dst), ArchVReg(src));
        GetMasm()->cmovpe(ArchReg(dst), ArchReg(tmp));
    } else {
        ASSERT(src.GetSize() == DOUBLE_WORD_SIZE);

        constexpr auto DOUBLE_NAN = uint64_t(0x7ff8000000000000);
        ScopedTmpRegU64 tmp(this);

        GetMasm()->ucomisd(ArchVReg(src), ArchVReg(src));
        GetMasm()->mov(ArchReg(tmp), DOUBLE_NAN);
        GetMasm()->movq(ArchReg(dst), ArchVReg(src));
        GetMasm()->cmovpe(ArchReg(dst), ArchReg(tmp));
    }
}

void Amd64Encoder::EncodeMoveBitsRaw(Reg dst, Reg src)
{
    ASSERT((dst.IsFloat() && src.IsScalar()) || (src.IsFloat() && dst.IsScalar()));
    if (src.IsScalar()) {
        ASSERT((dst.GetSize() == src.GetSize()));
        if (src.GetSize() == WORD_SIZE) {
            GetMasm()->movd(ArchVReg(dst), ArchReg(src));
        } else {
            GetMasm()->movq(ArchVReg(dst), ArchReg(src));
        }
    } else {
        ASSERT((src.GetSize() == dst.GetSize()));
        if (dst.GetSize() == WORD_SIZE) {
            GetMasm()->movd(ArchReg(dst), ArchVReg(src));
        } else {
            GetMasm()->movq(ArchReg(dst), ArchVReg(src));
        }
    }
}

/* Unsafe intrinsics */
void Amd64Encoder::EncodeCompareAndSwap(Reg dst, Reg obj, const Reg *offset, Reg val, Reg newval)
{
    /*
     * movl    old, %eax
     * lock    cmpxchgl   new, addr
     * sete    %al
     */
    ScopedTmpRegU64 tmp1(this);
    ScopedTmpRegU64 tmp2(this);
    ScopedTmpRegU64 tmp3(this);
    Reg newvalue = newval;
    auto addr = ArchMem(MemRef(tmp2)).Prepare(GetMasm());
    auto addr_reg = ArchReg(tmp2);
    Reg rax(ConvertRegNumber(asmjit::x86::rax.id()), INT64_TYPE);

    /* TODO(ayodkev) this is a workaround for the failure of
     * jsr166.ScheduledExecutorTest, have to figure out if there
     * is less crude way to avoid this */
    if (newval.GetId() == rax.GetId()) {
        SetFalseResult();
        return;
    }

    if (offset != nullptr) {
        GetMasm()->lea(addr_reg, asmjit::x86::ptr(ArchReg(obj), ArchReg(*offset)));
    } else {
        GetMasm()->mov(addr_reg, ArchReg(obj));
    }

    /* the [er]ax register will be overwritten by cmpxchg instruction
     * save it unless it is set as a destination register */
    if (dst.GetId() != rax.GetId()) {
        GetMasm()->mov(ArchReg(tmp1), asmjit::x86::rax);
    }

    /* if the new value comes in [er]ax register we have to use a
     * different register as [er]ax will contain the current value */
    if (newval.GetId() == rax.GetId()) {
        GetMasm()->mov(ArchReg(tmp3, newval.GetSize()), ArchReg(newval));
        newvalue = tmp3;
    }

    if (val.GetId() != rax.GetId()) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(val).r64());
    }

    GetMasm()->lock().cmpxchg(addr, ArchReg(newvalue));
    GetMasm()->sete(ArchReg(dst));

    if (dst.GetId() != rax.GetId()) {
        GetMasm()->mov(asmjit::x86::rax, ArchReg(tmp1));
    }
}

void Amd64Encoder::EncodeUnsafeGetAndSet(Reg dst, Reg obj, Reg offset, Reg val)
{
    ScopedTmpRegU64 tmp(this);
    auto addr_reg = ArchReg(tmp);
    auto addr = ArchMem(MemRef(tmp)).Prepare(GetMasm());
    GetMasm()->lea(addr_reg, asmjit::x86::ptr(ArchReg(obj), ArchReg(offset)));
    GetMasm()->mov(ArchReg(dst), ArchReg(val));
    GetMasm()->lock().xchg(addr, ArchReg(dst));
}

void Amd64Encoder::EncodeUnsafeGetAndAdd(Reg dst, Reg obj, Reg offset, Reg val, [[maybe_unused]] Reg tmp)
{
    ScopedTmpRegU64 tmp1(this);
    auto addr_reg = ArchReg(tmp1);
    auto addr = ArchMem(MemRef(tmp1)).Prepare(GetMasm());
    GetMasm()->lea(addr_reg, asmjit::x86::ptr(ArchReg(obj), ArchReg(offset)));
    GetMasm()->mov(ArchReg(dst), ArchReg(val));
    GetMasm()->lock().xadd(addr, ArchReg(dst));
}

void Amd64Encoder::EncodeMemoryBarrier(MemoryOrder::Order order)
{
    if (order == MemoryOrder::Full) {
        /* does the same as mfence but faster, not applicable for NT-writes, though */
        GetMasm()->lock().add(asmjit::x86::dword_ptr(asmjit::x86::rsp), asmjit::imm(0));
    }
}

void Amd64Encoder::EncodeStackOverflowCheck(ssize_t offset)
{
    MemRef mem(GetTarget().GetStackReg(), offset);
    auto m = ArchMem(mem).Prepare(GetMasm());
    GetMasm()->test(m, ArchReg(GetTarget().GetParamReg(0)));
}

void Amd64Encoder::MakeLibCall(Reg dst, Reg src0, Reg src1, void *entry_point)
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

        if (src0.GetId() != asmjit::x86::xmm0.id() || src1.GetId() != asmjit::x86::xmm1.id()) {
            ScopedTmpRegF32 tmp(this);
            GetMasm()->movss(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movss(asmjit::x86::xmm0, ArchVReg(src0));
            GetMasm()->movss(asmjit::x86::xmm1, ArchVReg(tmp));
        }

        MakeCall(entry_point);

        if (dst.GetId() != asmjit::x86::xmm0.id()) {
            GetMasm()->movss(ArchVReg(dst), asmjit::x86::xmm0);
        }
    } else if (dst.GetType() == FLOAT64_TYPE) {
        if (!src0.IsFloat() || !src1.IsFloat()) {
            SetFalseResult();
            return;
        }

        if (src0.GetId() != asmjit::x86::xmm0.id() || src1.GetId() != asmjit::x86::xmm1.id()) {
            ScopedTmpRegF64 tmp(this);
            GetMasm()->movsd(ArchVReg(tmp), ArchVReg(src1));
            GetMasm()->movsd(asmjit::x86::xmm0, ArchVReg(src0));
            GetMasm()->movsd(asmjit::x86::xmm1, ArchVReg(tmp));
        }

        MakeCall(entry_point);

        if (dst.GetId() != asmjit::x86::xmm0.id()) {
            GetMasm()->movsd(ArchVReg(dst), asmjit::x86::xmm0);
        }
    } else {
        UNREACHABLE();
    }
}

template <bool is_store>
void Amd64Encoder::LoadStoreRegisters(RegMask registers, ssize_t slot, size_t start_reg, bool is_fp)
{
    for (size_t i {0}; i < registers.size(); ++i) {
        if (!registers.test(i)) {
            continue;
        }

        asmjit::x86::Mem mem = asmjit::x86::ptr(asmjit::x86::rsp, (slot + i - start_reg) * DOUBLE_WORD_SIZE_BYTE);

        if constexpr (is_store) {  // NOLINT
            if (is_fp) {
                GetMasm()->movsd(mem, asmjit::x86::xmm(i));
            } else {
                GetMasm()->mov(mem, asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        } else {  // NOLINT
            if (is_fp) {
                GetMasm()->movsd(asmjit::x86::xmm(i), mem);
            } else {
                GetMasm()->mov(asmjit::x86::gpq(ConvertRegNumber(i)), mem);
            }
        }
    }
}

template <bool is_store>
void Amd64Encoder::LoadStoreRegisters(RegMask registers, bool is_fp, int32_t slot, Reg base, RegMask mask)
{
    auto base_reg = ArchReg(base);
    bool has_mask = mask.any();
    int32_t index = has_mask ? static_cast<int32_t>(mask.GetMinRegister()) : 0;
    slot -= index;
    for (size_t i = index; i < registers.size(); ++i) {
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

        // `-1` because we've incremented `index` in advance
        asmjit::x86::Mem mem = asmjit::x86::ptr(base_reg, (slot + index - 1) * DOUBLE_WORD_SIZE_BYTE);

        if constexpr (is_store) {  // NOLINT
            if (is_fp) {
                GetMasm()->movsd(mem, asmjit::x86::xmm(i));
            } else {
                GetMasm()->mov(mem, asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        } else {  // NOLINT
            if (is_fp) {
                GetMasm()->movsd(asmjit::x86::xmm(i), mem);
            } else {
                GetMasm()->mov(asmjit::x86::gpq(ConvertRegNumber(i)), mem);
            }
        }
    }
}

void Amd64Encoder::PushRegisters(RegMask registers, bool is_fp, bool align)
{
    for (size_t i = 0; i < registers.size(); i++) {
        if (registers[i]) {
            if (is_fp) {
                GetMasm()->sub(asmjit::x86::rsp, DOUBLE_WORD_SIZE_BYTE);
                GetMasm()->movsd(asmjit::x86::ptr(asmjit::x86::rsp), ArchVReg(Reg(i, FLOAT64_TYPE)));
            } else {
                GetMasm()->push(asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        }
    }
    if (align && (registers.count() & 1U) != 0) {
        GetMasm()->sub(asmjit::x86::rsp, DOUBLE_WORD_SIZE_BYTE);
    }
}

void Amd64Encoder::PopRegisters(RegMask registers, bool is_fp, bool align)
{
    if (align && (registers.count() & 1U) != 0) {
        GetMasm()->add(asmjit::x86::rsp, DOUBLE_WORD_SIZE_BYTE);
    }
    for (ssize_t i = registers.size() - 1; i >= 0; i--) {
        if (registers[i]) {
            if (is_fp) {
                GetMasm()->movsd(ArchVReg(Reg(i, FLOAT64_TYPE)), asmjit::x86::ptr(asmjit::x86::rsp));
                GetMasm()->add(asmjit::x86::rsp, DOUBLE_WORD_SIZE_BYTE);
            } else {
                GetMasm()->pop(asmjit::x86::gpq(ConvertRegNumber(i)));
            }
        }
    }
}

template <typename T, size_t n>
void Amd64Encoder::CopyArrayToXmm(Reg xmm, const std::array<T, n> &arr)
{
    static constexpr auto SIZE {n * sizeof(T)};
    static_assert((SIZE == DOUBLE_WORD_SIZE_BYTE) || (SIZE == 2U * DOUBLE_WORD_SIZE_BYTE));
    ASSERT(xmm.GetType() == FLOAT64_TYPE);

    auto data {reinterpret_cast<const uint64_t *>(arr.data())};

    ScopedTmpRegU64 tmp_gpr(this);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    GetMasm()->mov(ArchReg(tmp_gpr), asmjit::imm(data[0]));
    GetMasm()->movq(ArchVReg(xmm), ArchReg(tmp_gpr));

    if constexpr (SIZE == 2U * DOUBLE_WORD_SIZE_BYTE) {
        ScopedTmpRegF64 tmp_xmm(this);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        GetMasm()->mov(ArchReg(tmp_gpr), asmjit::imm(data[1]));
        GetMasm()->movq(ArchVReg(tmp_xmm), ArchReg(tmp_gpr));
        GetMasm()->unpcklpd(ArchVReg(xmm), ArchVReg(tmp_xmm));
    }
}

template <typename T>
void Amd64Encoder::CopyImmToXmm(Reg xmm, T imm)
{
    static_assert((sizeof(imm) == WORD_SIZE_BYTE) || (sizeof(imm) == DOUBLE_WORD_SIZE_BYTE));
    ASSERT(xmm.GetSize() == BYTE_SIZE * sizeof(imm));

    if constexpr (sizeof(imm) == WORD_SIZE_BYTE) {  // NOLINT
        ScopedTmpRegU32 tmp_gpr(this);
        GetMasm()->mov(ArchReg(tmp_gpr), asmjit::imm(bit_cast<uint32_t>(imm)));
        GetMasm()->movd(ArchVReg(xmm), ArchReg(tmp_gpr));
    } else {  // NOLINT
        ScopedTmpRegU64 tmp_gpr(this);
        GetMasm()->mov(ArchReg(tmp_gpr), asmjit::imm(bit_cast<uint64_t>(imm)));
        GetMasm()->movq(ArchVReg(xmm), ArchReg(tmp_gpr));
    }
}

size_t Amd64Encoder::DisasmInstr(std::ostream &stream, size_t pc, ssize_t code_offset) const
{
    if (code_offset < 0) {
        (const_cast<Amd64Encoder *>(this))->Finalize();
    }
    Span code(GetMasm()->bufferData(), GetMasm()->offset());

    [[maybe_unused]] size_t data_left = code.Size() - pc;
    [[maybe_unused]] constexpr size_t LENGTH = ZYDIS_MAX_INSTRUCTION_LENGTH;  // 15 bytes is max inst length in amd64

    // Initialize decoder context
    ZydisDecoder decoder;
    [[maybe_unused]] bool res =
        ZYAN_SUCCESS(ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64));

    // Initialize formatter
    ZydisFormatter formatter;
    res &= ZYAN_SUCCESS(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_ATT));
    ASSERT(res);

    ZydisDecodedInstruction instruction;

    res &= ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, &code[pc], std::min(LENGTH, data_left), &instruction));

    // Format & print the binary instruction structure to human readable format
    char buffer[256];  // NOLINT (modernize-avoid-c-arrays, readability-identifier-naming, readability-magic-numbers)
    res &= ZYAN_SUCCESS(
        ZydisFormatterFormatInstruction(&formatter, &instruction, buffer, sizeof(buffer), uintptr_t(&code[pc])));

    ASSERT(res);

    // Print disassembly
    if (code_offset < 0) {
        stream << buffer;
    } else {
        stream << std::setw(0x8) << std::right << std::setfill('0') << std::hex << pc + code_offset << std::dec
               << std::setfill(' ') << ": " << buffer;
    }

    return pc + instruction.length;
}
}  // namespace panda::compiler::amd64
