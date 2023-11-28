/*
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

#include "reg_acc_alloc.h"
#include "common.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/inst.h"

namespace ark::bytecodeopt {

/// Decide if accumulator register gets dirty between two instructions.
bool IsAccWriteBetween(compiler::Inst *srcInst, compiler::Inst *dstInst)
{
    ASSERT(srcInst != dstInst);

    compiler::BasicBlock *block = srcInst->GetBasicBlock();
    compiler::Inst *inst = srcInst->GetNext();

    while (inst != dstInst) {
        if (UNLIKELY(inst == nullptr)) {
            do {
                // NOTE(rtakacs): visit all the successors to get information about the
                // accumulator usage. Only linear flow is supported right now.
                if (block->GetSuccsBlocks().size() > 1U) {
                    return true;
                }

                ASSERT(block->GetSuccsBlocks().size() == 1U);
                block = block->GetSuccessor(0U);
                // NOTE(rtakacs): only linear flow is supported right now.
                if (!dstInst->IsPhi() && block->GetPredsBlocks().size() > 1U) {
                    return true;
                }
            } while (block->IsEmpty() && !block->HasPhi());

            // Get first phi instruction if exist.
            // This is requred if dst_inst is a phi node.
            inst = *(block->AllInsts());
        } else {
            if (inst->IsAccWrite()) {
                if (!inst->IsConst()) {
                    return true;
                }
                if (inst->GetDstReg() == compiler::ACC_REG_ID) {
                    return true;
                }
            }

            if (inst->IsAccRead()) {
                compiler::Inst *input = inst->GetInput(AccReadIndex(inst)).GetInst();

                if (input != srcInst && input->GetDstReg() != compiler::ACC_REG_ID) {
                    return true;
                }
            }

            inst = inst->GetNext();
        }
    }

    return false;
}
/// Return true if Phi instruction is marked as optimizable.
inline bool RegAccAlloc::IsPhiOptimizable(compiler::Inst *phi) const
{
    ASSERT(phi->GetOpcode() == compiler::Opcode::Phi);
    return phi->IsMarked(accMarker_);
}

/// Return true if instruction can read the accumulator.
bool RegAccAlloc::IsAccRead(compiler::Inst *inst) const
{
    return UNLIKELY(inst->IsPhi()) ? IsPhiOptimizable(inst) : inst->IsAccRead();
}

static bool IsCommutative(compiler::Inst *inst)
{
    if (inst->GetOpcode() == compiler::Opcode::If) {
        auto cc = inst->CastToIf()->GetCc();
        return cc == compiler::ConditionCode::CC_EQ || cc == compiler::ConditionCode::CC_NE;
    }
    return inst->IsCommutative();
}

bool UserNeedSwapInputs(compiler::Inst *inst, compiler::Inst *user)
{
    if (!IsCommutative(user)) {
        return false;
    }
    return user->GetInput(AccReadIndex(user)).GetInst() != inst;
}

/// Return true if instruction can write the accumulator.
bool RegAccAlloc::IsAccWrite(compiler::Inst *inst) const
{
    return UNLIKELY(inst->IsPhi()) ? IsPhiOptimizable(inst) : inst->IsAccWrite();
}

/**
 * Decide if user can use accumulator as source.
 * Do modifications on the order of inputs if necessary.
 *
 * Return true, if user can be optimized.
 */
bool RegAccAlloc::CanUserReadAcc(compiler::Inst *inst, compiler::Inst *user) const
{
    if (user->IsPhi()) {
        return IsPhiOptimizable(user);
    }

    if (!IsAccRead(user) || IsAccWriteBetween(inst, user)) {
        return false;
    }

    bool found = false;
    // Check if the instrucion occures more times as input.
    // v2. SUB v0, v1
    // v3. Add v2, v2
    for (auto input : user->GetInputs()) {
        compiler::Inst *uinput = input.GetInst();

        if (uinput != inst) {
            continue;
        }

        if (!found) {
            found = true;
        } else {
            return false;
        }
    }

    for (auto &input : user->GetInputs()) {
        auto inputInst = input.GetInst();
        if (inputInst != user && inputInst->GetDstReg() == compiler::ACC_REG_ID) {
            return false;
        }
        if (inst->IsPhi() && inputInst->IsConst() &&
            (inputInst->GetBasicBlock()->GetId() != user->GetBasicBlock()->GetId())) {
            return false;
        }
    }

    if (user->IsLaunchCall()) {
        return false;
    }

    if (user->IsCallOrIntrinsic()) {
        return user->GetInputsCount() <= (MAX_NUM_NON_RANGE_ARGS + 1U);  // +1 for SaveState
    }

    return user->GetInput(AccReadIndex(user)).GetInst() == inst || IsCommutative(user);
}

/**
 * Check if all the Phi inputs and outputs can use the accumulator register.
 *
 * Return true, if Phi can be optimized.
 */
bool RegAccAlloc::IsPhiAccReady(compiler::Inst *phi) const
{
    ASSERT(phi->GetOpcode() == compiler::Opcode::Phi);

    // NOTE(rtakacs): there can be cases when the input/output of a Phi is an other Phi.
    // These cases are not optimized for accumulator.
    for (auto input : phi->GetInputs()) {
        compiler::Inst *phiInput = input.GetInst();

        if (!IsAccWrite(phiInput) || IsAccWriteBetween(phiInput, phi)) {
            return false;
        }
    }

    std::unordered_set<compiler::Inst *> usersThatRequiredSwapInputs;
    for (auto &user : phi->GetUsers()) {
        compiler::Inst *uinst = user.GetInst();

        if (!CanUserReadAcc(phi, uinst)) {
            return false;
        }
        if (UserNeedSwapInputs(phi, uinst)) {
            usersThatRequiredSwapInputs.insert(uinst);
        }
    }
    for (auto uinst : usersThatRequiredSwapInputs) {
        uinst->SwapInputs();
    }

    return true;
}

/**
 * For most insts we can use their src_reg on the acc-read position
 * to characterise whether we need lda in the codegen pass.
 */
void RegAccAlloc::SetNeedLda(compiler::Inst *inst, bool need)
{
    if (inst->IsPhi() || inst->IsCatchPhi()) {
        return;
    }
    if (!IsAccRead(inst)) {
        return;
    }
    if (inst->IsCallOrIntrinsic()) {  // we never need lda for calls
        return;
    }
    compiler::Register reg = need ? compiler::INVALID_REG : compiler::ACC_REG_ID;
    inst->SetSrcReg(AccReadIndex(inst), reg);
}

static inline bool MaybeRegDst(compiler::Inst *inst)
{
    compiler::Opcode opcode = inst->GetOpcode();
    return inst->IsConst() || inst->IsBinaryInst() || inst->IsBinaryImmInst() || opcode == compiler::Opcode::LoadObject;
}

/**
 * Determine the accumulator usage between instructions.
 * Eliminate unnecessary register allocations by applying
 * a special value (ACC_REG_ID) to the destination and
 * source registers.
 * This special value is a marker for the code generator
 * not to produce lda/sta instructions.
 */
bool RegAccAlloc::RunImpl()
{
    GetGraph()->InitDefaultLocations();
    // Initialize all source register of all instructions.
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (inst->IsSaveState() || inst->IsCatchPhi()) {
                continue;
            }
            if (inst->IsConst()) {
                inst->SetFlag(compiler::inst_flags::ACC_WRITE);
            }
            for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
                inst->SetSrcReg(i, compiler::INVALID_REG);
                if (MaybeRegDst(inst)) {
                    inst->SetDstReg(compiler::INVALID_REG);
                }
            }
        }
    }

    // Drop the pass if the function contains unsupported opcodes
    // NOTE(rtakacs): support these opcodes.
    if (!GetGraph()->IsDynamicMethod()) {
        for (auto block : GetGraph()->GetBlocksRPO()) {
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() == compiler::Opcode::Builtin) {
                    return false;
                }
            }
        }
    }

    // Mark Phi instructions if they can be optimized for acc.
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto phi : block->PhiInsts()) {
            if (IsPhiAccReady(phi)) {
                phi->SetMarker(accMarker_);
            }
        }
    }

    // Mark instructions if they can be optimized for acc.
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInsts()) {
            if (inst->NoDest() || !IsAccWrite(inst)) {
                continue;
            }

            bool useAccDstReg = true;

            std::unordered_set<compiler::Inst *> usersThatRequiredSwapInputs;
            for (auto &user : inst->GetUsers()) {
                compiler::Inst *uinst = user.GetInst();
                if (uinst->IsSaveState()) {
                    continue;
                }
                if (CanUserReadAcc(inst, uinst)) {
                    if (UserNeedSwapInputs(inst, uinst)) {
                        usersThatRequiredSwapInputs.insert(uinst);
                    }
                    SetNeedLda(uinst, false);
                } else {
                    useAccDstReg = false;
                }
            }
            for (auto uinst : usersThatRequiredSwapInputs) {
                uinst->SwapInputs();
            }

            if (useAccDstReg) {
                inst->SetDstReg(compiler::ACC_REG_ID);
            } else if (MaybeRegDst(inst)) {
                inst->ClearFlag(compiler::inst_flags::ACC_WRITE);
                for (auto &user : inst->GetUsers()) {
                    compiler::Inst *uinst = user.GetInst();
                    if (uinst->IsSaveState()) {
                        continue;
                    }
                    SetNeedLda(uinst, true);
                }
            }
        }

        for (auto inst : block->Insts()) {
            if (inst->GetInputsCount() == 0U) {
                continue;
            }

            if (inst->IsCallOrIntrinsic()) {
                continue;
            }

            compiler::Inst *input = inst->GetInput(AccReadIndex(inst)).GetInst();

            if (IsAccWriteBetween(input, inst)) {
                input->SetDstReg(compiler::INVALID_REG);
                SetNeedLda(inst, true);

                if (MaybeRegDst(input)) {
                    input->ClearFlag(compiler::inst_flags::ACC_WRITE);
                    for (auto &user : input->GetUsers()) {
                        compiler::Inst *uinst = user.GetInst();
                        SetNeedLda(uinst, true);
                    }
                }
            }
        }
    }

#ifndef NDEBUG
    GetGraph()->SetRegAccAllocApplied();
#endif  // NDEBUG

    return true;
}

}  // namespace ark::bytecodeopt
