/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
}  // namespace ark::compiler
