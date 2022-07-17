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

#include "common.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"

namespace panda::bytecodeopt {

uint8_t AccReadIndex(const compiler::Inst *inst)
{
    // For calls we cannot tell static index for acc position, thus
    // ensure that we don't invoke this for calls
    ASSERT(!inst->IsCall());

    switch (inst->GetOpcode()) {
        case compiler::Opcode::LoadArray:
        case compiler::Opcode::StoreObject:
        case compiler::Opcode::StoreStatic:
        case compiler::Opcode::NewArray:
            return 1U;
        case compiler::Opcode::StoreArray:
            return 2U;
        default: {
            if (inst->IsIntrinsic() && inst->IsAccRead()) {
                ASSERT(inst->GetBasicBlock()->GetGraph()->IsDynamicMethod());
                ASSERT(inst->GetInputsCount() >= 2U);
                return inst->GetInputsCount() - 2U;
            }
            return 0;
        }
    }
}

// This method is used by bytecode optimizer's codegen.
bool CanConvertToIncI(const compiler::BinaryImmOperation *binop)
{
    ASSERT(binop->GetBasicBlock()->GetGraph()->IsRegAllocApplied());
    ASSERT(binop->GetOpcode() == compiler::Opcode::AddI || binop->GetOpcode() == compiler::Opcode::SubI);

    // IncI works on the same register.
    if (binop->GetSrcReg(0) != binop->GetDstReg()) {
        return false;
    }

    // IncI cannot write accumulator.
    if (binop->GetSrcReg(0) == compiler::ACC_REG_ID) {
        return false;
    }

    // IncI users cannot read from accumulator.
    // While Addi/SubI stores the output in accumulator, IncI works directly on registers.
    for (const auto &user : binop->GetUsers()) {
        const auto *uinst = user.GetInst();

        if (uinst->IsCall()) {
            continue;
        }

        const uint8_t index = AccReadIndex(uinst);
        if (uinst->GetInput(index).GetInst() == binop && uinst->GetSrcReg(index) == compiler::ACC_REG_ID) {
            return false;
        }
    }

    constexpr uint64_t bitmask = 0xffffffff;
    // Define min and max values of i4 type.
    constexpr int32_t min = -8;
    constexpr int32_t max = 7;

    int32_t imm = binop->GetImm() & bitmask;
    // Note: subi 3 is the same as inci v2, -3.
    if (binop->GetOpcode() == compiler::Opcode::SubI) {
        imm = -imm;
    }

    // IncI works only with 4 bits immediates.
    return imm >= min && imm <= max;
}

}  // namespace panda::bytecodeopt
