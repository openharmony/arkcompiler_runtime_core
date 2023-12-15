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

namespace panda::compiler {

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
    auto entrypoint_id = EntrypointId::INVALID;

    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BOOL_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_COPY_TO:
            entrypoint_id = EntrypointId::ARRAY_COPY_TO_1B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_COPY_TO:
            entrypoint_id = EntrypointId::ARRAY_COPY_TO_2B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_COPY_TO:
            entrypoint_id = EntrypointId::ARRAY_COPY_TO_4B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_COPY_TO:
            entrypoint_id = EntrypointId::ARRAY_COPY_TO_8B;
            break;

        default:
            UNREACHABLE();
            break;
    }

    ASSERT(entrypoint_id != EntrypointId::COUNT);

    auto src_obj = src[FIRST_OPERAND];
    auto dst_obj = src[SECOND_OPERAND];
    auto dst_start = src[THIRD_OPERAND];
    auto src_start = src[FOURTH_OPERAND];
    auto src_end = src[FIFTH_OPERAND];
    CallFastPath(inst, entrypoint_id, INVALID_REGISTER, RegMask::GetZeroMask(), src_obj, dst_obj, dst_start, src_start,
                 src_end);
}
}  // namespace panda::compiler
