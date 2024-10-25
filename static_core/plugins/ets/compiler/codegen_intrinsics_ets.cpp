/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "compiler/optimizer/code_generator/operands.h"
#include "compiler/optimizer/code_generator/codegen.h"
#include "compiler/optimizer/ir/analysis.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/array.h"

namespace ark::compiler {

class SbAppendArgs {
private:
    Reg dst_;
    Reg builder_;
    Reg value_;

public:
    SbAppendArgs() = delete;
    SbAppendArgs(Reg dst, Reg builder, Reg value) : dst_(dst), builder_(builder), value_(value)
    {
        ASSERT(dst_ != INVALID_REGISTER);
        ASSERT(builder_ != INVALID_REGISTER);
        ASSERT(value_ != INVALID_REGISTER);
    }
    Reg Dst() const
    {
        return dst_;
    }
    Reg Builder() const
    {
        return builder_;
    }
    Reg Value() const
    {
        return value_;
    }
    bool DstCanBeUsedAsTemp() const
    {
        return (dst_.GetId() != builder_.GetId() && dst_.GetId() != value_.GetId());
    }
    MemRef SbBufferAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbBufferOffset());
    }
    MemRef SbIndexAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbIndexOffset());
    }
    MemRef SbCompressAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbCompressOffset());
    }
    MemRef SbLengthAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbLengthOffset());
    }
};

void Codegen::CreateMathTrunc([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeTrunc(dst, src[0]);
}

void Codegen::CreateMathRoundAway([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeRoundAway(dst, src[0]);
}

void Codegen::CreateArrayCopyTo(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    auto entrypointId = EntrypointId::INVALID;

    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BOOL_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_1B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_2B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_4B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_8B;
            break;

        default:
            UNREACHABLE();
            break;
    }

    ASSERT(entrypointId != EntrypointId::COUNT);

    auto srcObj = src[FIRST_OPERAND];
    auto dstObj = src[SECOND_OPERAND];
    auto dstStart = src[THIRD_OPERAND];
    auto srcStart = src[FOURTH_OPERAND];
    auto srcEnd = src[FIFTH_OPERAND];
    CallFastPath(inst, entrypointId, INVALID_REGISTER, RegMask::GetZeroMask(), srcObj, dstObj, dstStart, srcStart,
                 srcEnd);
}

// Generates a call to StringBuilder.append() for values (EtsBool/Char/Bool/Short/Int/Long),
// which are translated to array of utf16 chars.
static inline void GenerateSbAppendCall(Codegen *cg, IntrinsicInst *inst, SbAppendArgs args,
                                        RuntimeInterface::EntrypointId entrypoint)
{
    auto *runtime = cg->GetGraph()->GetRuntime();
    if (cg->GetGraph()->IsAotMode()) {
        auto *enc = cg->GetEncoder();
        ScopedTmpReg klass(enc);
        enc->EncodeLdr(klass, false, MemRef(cg->ThreadReg(), runtime->GetArrayU16ClassPointerTlsOffset(cg->GetArch())));
        cg->CallFastPath(inst, entrypoint, args.Dst(), {}, args.Builder(), args.Value(), klass);
    } else {
        auto klass = TypedImm(reinterpret_cast<uintptr_t>(runtime->GetArrayU16Class(cg->GetGraph()->GetMethod())));
        cg->CallFastPath(inst, entrypoint, args.Dst(), {}, args.Builder(), args.Value(), klass);
    }
}

void Codegen::CreateStringBuilderAppendNumber(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto sb = src[FIRST_OPERAND];
    auto num = src[SECOND_OPERAND];
    auto type = ConvertDataType(DataType::INT64, GetArch());
    ScopedTmpReg tmp(GetEncoder(), type);

    if (num.GetType() != INT64_TYPE) {
        ASSERT(num.GetType() == INT32_TYPE || num.GetType() == INT16_TYPE || num.GetType() == INT8_TYPE);
        if (dst.GetId() != sb.GetId() && dst.GetId() != num.GetId()) {
            GetEncoder()->EncodeCast(dst.As(type), true, num, true);
            num = dst.As(type);
        } else {
            GetEncoder()->EncodeCast(tmp, true, num, true);
            num = tmp.GetReg();
        }
    }
    GenerateSbAppendCall(this, inst, SbAppendArgs(dst, sb, num), EntrypointId::STRING_BUILDER_APPEND_LONG);
}

void Codegen::CreateStringBuilderAppendChar(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto entrypoint = EntrypointId::STRING_BUILDER_APPEND_CHAR_COMPRESSED;
    SbAppendArgs args(dst, src[FIRST_OPERAND], src[SECOND_OPERAND]);
    GenerateSbAppendCall(this, inst, args, entrypoint);
}

void Codegen::CreateStringBuilderAppendBool(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    SbAppendArgs args(dst, src[FIRST_OPERAND], src[SECOND_OPERAND]);
    GenerateSbAppendCall(this, inst, args, EntrypointId::STRING_BUILDER_APPEND_BOOL);
}

static inline void EncodeSbAppendNullString(Codegen *cg, IntrinsicInst *inst, Reg dst, Reg builder)
{
    auto entrypoint = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_NULL_STRING;
    cg->CallRuntime(inst, entrypoint, dst, {}, builder);
}

static inline void EncodeSbInsertStringIntoSlot(Codegen *cg, IntrinsicInst *inst, Reg slot, SbAppendArgs args)
{
    ASSERT(slot.IsValid());
    auto slotMemRef = MemRef(slot.As(Codegen::ConvertDataType(DataType::REFERENCE, cg->GetArch())));
    RegMask preserved(MakeMask(args.Builder().GetId(), args.Value().GetId(), slot.GetId()));
    cg->CreatePreWRB(inst, slotMemRef, preserved);
    cg->GetEncoder()->EncodeStr(args.Value(), slotMemRef);
    preserved.Reset(slot.GetId());
    cg->CreatePostWRB(inst, slotMemRef, args.Value(), INVALID_REGISTER, preserved);
}

static void EncodeSbAppendString(Codegen *cg, IntrinsicInst *inst, const SbAppendArgs &args,
                                 LabelHolder::LabelId labelReturn, LabelHolder::LabelId labelSlowPath)
{
    auto *enc = cg->GetEncoder();
    ScopedTmpReg tmp1(enc);
    ScopedTmpRegLazy tmp2(enc, false);
    auto reg0 = cg->ConvertInstTmpReg(inst, DataType::REFERENCE);
    auto reg1 = tmp1.GetReg().As(INT32_TYPE);
    auto reg2 = INVALID_REGISTER;
    if (args.DstCanBeUsedAsTemp() && args.Dst().GetId() != reg0.GetId()) {
        reg2 = args.Dst().As(INT32_TYPE);
    } else {
        tmp2.Acquire();
        reg2 = tmp2.GetReg().As(INT32_TYPE);
    }
    auto labelInsertStringIntoSlot = enc->CreateLabel();
    auto labelFastPathDone = enc->CreateLabel();
    auto labelIncIndex = enc->CreateLabel();
    // Jump to slowPath if buffer is full and needs to be reallocated
    enc->EncodeLdr(reg0, false, args.SbBufferAddr());
    enc->EncodeLdr(reg1, false, MemRef(reg0, coretypes::Array::GetLengthOffset()));
    enc->EncodeLdr(reg2, false, args.SbIndexAddr());
    enc->EncodeJump(labelSlowPath, reg2, reg1, Condition::HS);
    // Compute an address of a free slot so as not to reload SbIndex again
    enc->EncodeShl(reg1, reg2, Imm(compiler::DataType::ShiftByType(compiler::DataType::REFERENCE, cg->GetArch())));
    enc->EncodeAdd(reg0, reg0, Imm(coretypes::Array::GetDataOffset()));
    enc->EncodeAdd(reg0, reg0, reg1);
    // Process string length and compression
    enc->EncodeLdr(reg1, false, MemRef(args.Value(), ark::coretypes::STRING_LENGTH_OFFSET));
    // Do nothing if length of string is equal to 0.
    // The least significant bit indicates COMPRESSED/UNCOMPRESSED,
    // thus if (packed length <= 1) then the actual length is equal to 0.
    enc->EncodeJump(labelFastPathDone, reg1, Imm(1), Condition::LS);
    // Skip setting 'compress' to false if the string is compressed.
    enc->EncodeJumpTest(labelIncIndex, reg1, Imm(1), Condition::TST_EQ);
    // Otherwise set 'compress' to false
    enc->EncodeSti(0, 1, args.SbCompressAddr());
    // Increment 'index' field
    enc->BindLabel(labelIncIndex);
    enc->EncodeAdd(reg2, reg2, Imm(1));
    enc->EncodeStr(reg2, args.SbIndexAddr());
    // Unpack length of string
    enc->EncodeShr(reg1, reg1, Imm(1));
    // Add length of string to the current length of StringBuilder
    enc->EncodeLdr(reg2, false, args.SbLengthAddr());
    enc->EncodeAdd(reg2, reg2, reg1);
    enc->EncodeStr(reg2, args.SbLengthAddr());
    // Insert the string into the slot:
    // - reg0 contains an address of the slot
    // - release temps for barriers
    enc->BindLabel(labelInsertStringIntoSlot);
    tmp1.Release();
    tmp2.Release();
    EncodeSbInsertStringIntoSlot(cg, inst, reg0, args);
    // Return the reference to StringBuilder
    enc->BindLabel(labelFastPathDone);
    enc->EncodeMov(args.Dst(), args.Builder());
    enc->EncodeJump(labelReturn);
}

void Codegen::CreateStringBuilderAppendString(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    using StringLengthType = std::result_of<decltype (&coretypes::String::GetLength)(coretypes::String)>::type;
    static_assert(TypeInfo::GetScalarTypeBySize(sizeof(ark::ArraySizeT) * CHAR_BIT) == INT32_TYPE);
    static_assert(TypeInfo::GetScalarTypeBySize(sizeof(StringLengthType) * CHAR_BIT) == INT32_TYPE);
    ASSERT(GetArch() != Arch::AARCH32);
    ASSERT(IsCompressedStringsEnabled());

    auto *enc = GetEncoder();
    auto builder = src[FIRST_OPERAND];
    auto *strInst = inst->GetInput(1).GetInst();
    if (strInst->IsNullPtr()) {
        EncodeSbAppendNullString(this, inst, dst, builder);
        return;
    }
    auto labelReturn = enc->CreateLabel();
    auto labelSlowPath = enc->CreateLabel();
    auto str = src[SECOND_OPERAND];
    if (IsInstNotNull(strInst)) {
        EncodeSbAppendString(this, inst, SbAppendArgs(dst, builder, str), labelReturn, labelSlowPath);
    } else {
        auto labelStrNotNull = enc->CreateLabel();
        enc->EncodeJump(labelStrNotNull, str, Condition::NE);
        EncodeSbAppendNullString(this, inst, dst, builder);
        enc->EncodeJump(labelReturn);
        enc->BindLabel(labelStrNotNull);
        EncodeSbAppendString(this, inst, SbAppendArgs(dst, builder, str), labelReturn, labelSlowPath);
    }
    // Slow path
    static constexpr auto ENTRYPOINT_ID = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_STRING;
    enc->BindLabel(labelSlowPath);
    CallRuntime(inst, ENTRYPOINT_ID, dst, {}, builder, str);
    // Return
    enc->BindLabel(labelReturn);
}

RuntimeInterface::EntrypointId GetStringBuilderAppendStringsEntrypointId(uint32_t numArgs, mem::BarrierType barrierType)
{
    using EntrypointId = RuntimeInterface::EntrypointId;
    switch (barrierType) {
        case mem::BarrierType::POST_INTERGENERATIONAL_BARRIER: {  // Gen GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_ASYNC_MANUAL,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_ASYNC_MANUAL,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_ASYNC_MANUAL,
            };
            return entrypoints[numArgs];
        }
        case mem::BarrierType::POST_INTERREGION_BARRIER: {  // G1 GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_ASYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_ASYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_ASYNC,
            };
            return entrypoints[numArgs];
        }
        default: {  // STW GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_SYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_SYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_SYNC,
            };
            return entrypoints[numArgs];
        }
    }
}

void Codegen::CreateStringBuilderAppendStrings(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto builder = src[FIRST_OPERAND];
    auto str0 = src[SECOND_OPERAND];
    auto str1 = src[THIRD_OPERAND];
    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2: {
            auto entrypoint = GetStringBuilderAppendStringsEntrypointId(2U, GetGraph()->GetRuntime()->GetPostType());
            CallFastPath(inst, entrypoint, dst, {}, builder, str0, str1);
            break;
        }

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3: {
            auto str2 = src[FOURTH_OPERAND];
            auto entrypoint = GetStringBuilderAppendStringsEntrypointId(3U, GetGraph()->GetRuntime()->GetPostType());
            CallFastPath(inst, entrypoint, dst, {}, builder, str0, str1, str2);
            break;
        }

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4: {
            auto str2 = src[FOURTH_OPERAND];
            auto str3 = src[FIFTH_OPERAND];
            auto entrypoint = GetStringBuilderAppendStringsEntrypointId(4U, GetGraph()->GetRuntime()->GetPostType());
            CallFastPath(inst, entrypoint, dst, {}, builder, str0, str1, str2, str3);
            break;
        }

        default:
            UNREACHABLE();
            break;
    }
}

void Codegen::CreateStringConcat([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2: {
            auto str1 = src[FIRST_OPERAND];
            auto str2 = src[SECOND_OPERAND];
            CallFastPath(inst, EntrypointId::STRING_CONCAT2_TLAB, dst, {}, str1, str2);
            break;
        }

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3: {
            auto str1 = src[FIRST_OPERAND];
            auto str2 = src[SECOND_OPERAND];
            auto str3 = src[THIRD_OPERAND];
            CallFastPath(inst, EntrypointId::STRING_CONCAT3_TLAB, dst, {}, str1, str2, str3);
            break;
        }

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4: {
            auto str1 = src[FIRST_OPERAND];
            auto str2 = src[SECOND_OPERAND];
            auto str3 = src[THIRD_OPERAND];
            auto str4 = src[FOURTH_OPERAND];
            CallFastPath(inst, EntrypointId::STRING_CONCAT4_TLAB, dst, {}, str1, str2, str3, str4);
            break;
        }

        default:
            UNREACHABLE();
            break;
    }
}

void Codegen::CreateStringBuilderToString(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    ASSERT(IsCompressedStringsEnabled());

    auto *enc = GetEncoder();
    auto entrypoint = EntrypointId::STRING_BUILDER_TO_STRING;
    auto sb = src[FIRST_OPERAND];
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klass(enc);
        enc->EncodeLdr(klass, false, MemRef(ThreadReg(), GetRuntime()->GetStringClassPointerTlsOffset(GetArch())));
        CallFastPath(inst, entrypoint, dst, {}, sb, klass);
    } else {
        auto klass =
            TypedImm(reinterpret_cast<uintptr_t>(GetRuntime()->GetStringClass(GetGraph()->GetMethod(), nullptr)));
        CallFastPath(inst, entrypoint, dst, {}, sb, klass);
    }
}

void Codegen::CreateDoubleToStringDecimal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    ASSERT(inst->GetInputsCount() == 4U && inst->RequireState());
    auto cache = src[FIRST_OPERAND];
    auto numAsInt = src[SECOND_OPERAND];
    auto unused = src[THIRD_OPERAND];
    auto entrypoint = EntrypointId::DOUBLE_TO_STRING_DECIMAL;
    CallFastPath(inst, entrypoint, dst, {}, cache, numAsInt, unused);
}

/* See utf::IsWhiteSpaceChar() for the details */
void Codegen::CreateCharIsWhiteSpace([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = RuntimeInterface::EntrypointId::CHAR_IS_WHITE_SPACE;
    auto ch = src[FIRST_OPERAND];
    CallFastPath(inst, entrypoint, dst, {}, ch.As(INT16_TYPE));
}

void Codegen::CreateStringTrimLeft(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto unused = TypedImm(0);
    // llvm backend needs unused args to call 3-args slow_path from 1-arg fast_path.
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_TRIM_LEFT, dst, {}, str, unused, unused);
}

void Codegen::CreateStringTrimRight(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto unused = TypedImm(0);
    // llvm backend needs unused args to call 3-args slow_path from 1-arg fast_path.
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_TRIM_RIGHT, dst, {}, str, unused, unused);
}

void Codegen::CreateStringTrim(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto unused = TypedImm(0);
    // llvm backend needs unused args to call 3-args slow_path from 1-arg fast_path.
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_TRIM, dst, {}, str, unused, unused);
}

void Codegen::CreateStringStartsWith(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto pfx = src[SECOND_OPERAND];
    auto idx = src[THIRD_OPERAND];
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_STARTS_WITH, dst, {}, str, pfx, idx);
}

void Codegen::CreateStringEndsWith(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto sfx = src[SECOND_OPERAND];
    auto idx = src[THIRD_OPERAND];
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_ENDS_WITH, dst, {}, str, sfx, idx);
}

void Codegen::CreateStringGetBytesTlab([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto entrypointId = EntrypointId::STRING_GET_BYTES_TLAB;
    auto runtime = GetGraph()->GetRuntime();
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(klassReg, false,
                                MemRef(ThreadReg(), runtime->GetArrayU8ClassPointerTlsOffset(GetArch())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassReg);
    } else {
        auto klassImm = TypedImm(reinterpret_cast<uintptr_t>(runtime->GetArrayU8Class(GetGraph()->GetMethod())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassImm);
    }
}

}  // namespace ark::compiler
