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
#include "operands.h"
#include "codegen.h"

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
    auto entrypoint = IsCompressedStringsEnabled() ? EntrypointId::STRING_BUILDER_APPEND_CHAR_COMPRESSED
                                                   : EntrypointId::STRING_BUILDER_APPEND_CHAR;
    SbAppendArgs args(dst, src[FIRST_OPERAND], src[SECOND_OPERAND]);
    GenerateSbAppendCall(this, inst, args, entrypoint);
}

void Codegen::CreateStringBuilderAppendBool(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    SbAppendArgs args(dst, src[FIRST_OPERAND], src[SECOND_OPERAND]);
    GenerateSbAppendCall(this, inst, args, EntrypointId::STRING_BUILDER_APPEND_BOOL);
}

}  // namespace ark::compiler
