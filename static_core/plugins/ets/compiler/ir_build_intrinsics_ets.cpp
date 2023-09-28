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

#include "optimizer/code_generator/encode.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "bytecode_instruction-inl.h"

namespace panda::compiler {
/*
    FMOV: f -> n
    return CMP(AND(SHR(n, FP_FRACT_SIZE), FP_EXP_MASK), FP_EXP_MASK)

    1. bitcast f1
    2. shr v1, FP_FRACT_SIZE
    3. and v2, FP_EXP_MASK
    4. compare v3, FP_EXP_MASK

    fraction size is 23 bits for floats and 52 bits for doubles
    exponent mask is 0xff (8 bits) for floats and 0x7ff (11 bits) for doubles
 */
void InstBuilder::BuildIsFiniteIntrinsic(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto method_index = bc_inst->GetId(0).AsIndex();
    auto method_id = GetRuntime()->ResolveMethodIndex(GetMethod(), method_index);
    auto type = GetMethodArgumentType(method_id, 0);
    auto itype = type == DataType::FLOAT32 ? DataType::INT32 : DataType::INT64;
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto fp_fract_size = type == DataType::FLOAT32 ? 23 : 52;
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto fp_exp_mask = type == DataType::FLOAT32 ? 0xff : 0x7ff;

    auto bitcast = GetGraph()->CreateInstBitcast(itype, GetPc(bc_inst->GetAddress()));
    auto shift = GetGraph()->CreateInstShr(itype, GetPc(bc_inst->GetAddress()));
    auto mask = GetGraph()->CreateInstAnd(itype, GetPc(bc_inst->GetAddress()));
    auto cmp = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), ConditionCode::CC_NE);

    bitcast->SetInput(0, GetArgDefinition(bc_inst, 0, acc_read));
    bitcast->SetOperandsType(type);
    shift->SetInput(0, bitcast);
    shift->SetInput(1, FindOrCreateConstant(fp_fract_size));
    mask->SetInput(0, shift);
    mask->SetInput(1, FindOrCreateConstant(fp_exp_mask));
    cmp->SetOperandsType(itype);
    cmp->SetInput(0, mask);
    cmp->SetInput(1, FindOrCreateConstant(fp_exp_mask));

    AddInstruction(bitcast, shift, mask, cmp);
    UpdateDefinitionAcc(cmp);
}

void InstBuilder::BuildStdRuntimeEquals(const BytecodeInstruction *bc_inst, bool acc_read)
{
    auto cmp = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bc_inst->GetAddress()), ConditionCode::CC_EQ);
    cmp->SetInput(0, GetArgDefinition(bc_inst, 1, acc_read));
    cmp->SetInput(1, GetArgDefinition(bc_inst, 2, acc_read));
    cmp->SetOperandsType(DataType::REFERENCE);

    AddInstruction(cmp);
    UpdateDefinitionAcc(cmp);
}

}  // namespace panda::compiler
