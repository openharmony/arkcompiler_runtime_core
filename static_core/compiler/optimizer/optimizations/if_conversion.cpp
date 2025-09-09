/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "compiler_logger.h"
#include "if_conversion.h"

#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {
bool IfConversion::RunImpl()
{
    COMPILER_LOG(DEBUG, IFCONVERSION) << "Run " << GetPassName();
    bool result = false;
    // Post order (without reverse)
    for (auto it = GetGraph()->GetBlocksRPO().rbegin(); it != GetGraph()->GetBlocksRPO().rend(); it++) {
        auto block = *it;
        if (block->GetSuccsBlocks().size() == MAX_SUCCS_NUM) {
            result |= TryTriangle(block) || TryDiamond(block);
        }
    }
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        result |= TryReplaceSelectImmVariablesWithConstants();
    }
    COMPILER_LOG(DEBUG, IFCONVERSION) << GetPassName() << " complete";
    return result;
}

/*
   The methods handles the recognition of the following pattern that have two variations:
   BB -- basic block the recognition starts from
   TBB -- TrueSuccessor of BB
   FBB -- FalseSuccessor of BB

       P0          P1
      [BB]        [BB]
       |  \        |  \
       |  [TBB]    |  [FBB]
       |  /        |  /
      [FBB]       [TBB]
*/
bool IfConversion::TryTriangle(BasicBlock *bb)
{
    ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    BasicBlock *tbb;
    BasicBlock *fbb;
    bool swapped = false;
    // interpret P1 variation as P0
    if (bb->GetTrueSuccessor()->GetPredsBlocks().size() == 1) {
        tbb = bb->GetTrueSuccessor();
        fbb = bb->GetFalseSuccessor();
    } else if (bb->GetFalseSuccessor()->GetPredsBlocks().size() == 1) {
        tbb = bb->GetFalseSuccessor();
        fbb = bb->GetTrueSuccessor();
        swapped = true;
    } else {
        return false;
    }

    if (tbb->GetSuccsBlocks().size() > 1 || fbb->GetPredsBlocks().size() <= 1 || tbb->GetSuccessor(0) != fbb) {
        return false;
    }

    if (LoopInvariantPreventConversion(bb)) {
        return false;
    }

    uint32_t instCount = 0;
    uint32_t phiCount = 0;
    auto limit = GetIfcLimit(bb);
    if (IsConvertable(tbb, &instCount) && IsPhisAllowed(fbb, tbb, bb, &phiCount)) {
        COMPILER_LOG(DEBUG, IFCONVERSION)
            << "Triangle pattern was found in Block #" << bb->GetId() << " with " << instCount
            << " convertible instruction(s) and " << phiCount << " Phi(s) to process";
        if (instCount <= limit && phiCount <= limit) {
            // Joining tbb into bb
            bb->JoinBlocksUsingSelect(tbb, nullptr, swapped);

            if (fbb->GetPredsBlocks().size() == 1 && bb->IsTry() == fbb->IsTry()) {
                bb->JoinSuccessorBlock();  // join fbb
                COMPILER_LOG(DEBUG, IFCONVERSION) << "Merged blocks " << tbb->GetId() << " and " << fbb->GetId()
                                                  << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Triangle", tbb->GetId(),
                                                               fbb->GetId(), -1);
            } else {
                COMPILER_LOG(DEBUG, IFCONVERSION)
                    << "Merged block " << tbb->GetId() << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Triangle_Phi",
                                                               tbb->GetId(), -1, -1);
            }
            return true;
        }
    }

    return false;
}

uint32_t IfConversion::GetIfcLimit(BasicBlock *bb)
{
    ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    auto trueCounter = GetGraph()->GetBranchCounter(bb, true);
    auto falseCounter = GetGraph()->GetBranchCounter(bb, false);
    if (trueCounter == 0 && falseCounter == 0) {
        return limit_;
    }
    auto minCounter = std::min(trueCounter, falseCounter);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto percent = (minCounter * 100) / (trueCounter + falseCounter);
    if (percent < g_options.GetCompilerIfConversionIncraseLimitThreshold()) {
        return limit_;
    }
    // The limit is increased by 4 times for branch with a large number of mispredicts
    return limit_ << 2U;
}

/*
   The methods handles the recognition of the following pattern:
   BB -- basic block the recognition starts from
   TBB -- TrueSuccessor of BB
   FBB -- FalseSuccessor of BB
   JBB -- Joint basic block of branches

      [BB]
     /    \
   [TBB] [FBB]
     \    /
      [JBB]
*/
bool IfConversion::TryDiamond(BasicBlock *bb)
{
    ASSERT(bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    BasicBlock *tbb = bb->GetTrueSuccessor();
    if (tbb->GetPredsBlocks().size() != 1 || tbb->GetSuccsBlocks().size() != 1) {
        return false;
    }

    BasicBlock *fbb = bb->GetFalseSuccessor();
    if (fbb->GetPredsBlocks().size() != 1 || fbb->GetSuccsBlocks().size() != 1) {
        return false;
    }

    BasicBlock *jbb = tbb->GetSuccessor(0);
    if (jbb->GetPredsBlocks().size() <= 1 || fbb->GetSuccessor(0) != jbb) {
        return false;
    }

    if (LoopInvariantPreventConversion(bb)) {
        return false;
    }

    uint32_t tbbInst = 0;
    uint32_t fbbInst = 0;
    uint32_t phiCount = 0;
    auto limit = GetIfcLimit(bb);
    if (IsConvertable(tbb, &tbbInst) && IsConvertable(fbb, &fbbInst) && IsPhisAllowed(jbb, tbb, fbb, &phiCount)) {
        COMPILER_LOG(DEBUG, IFCONVERSION)
            << "Diamond pattern was found in Block #" << bb->GetId() << " with " << (tbbInst + fbbInst)
            << " convertible instruction(s) and " << phiCount << " Phi(s) to process";
        if (tbbInst + fbbInst <= limit && phiCount <= limit) {
            // Joining tbb into bb
            bb->JoinBlocksUsingSelect(tbb, fbb, false);

            bb->JoinSuccessorBlock();  // joining fbb

            if (jbb->GetPredsBlocks().size() == 1 && bb->IsTry() == jbb->IsTry()) {
                bb->JoinSuccessorBlock();  // joining jbb
                COMPILER_LOG(DEBUG, IFCONVERSION) << "Merged blocks " << tbb->GetId() << ", " << fbb->GetId() << " and "
                                                  << jbb->GetId() << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Diamond", tbb->GetId(),
                                                               fbb->GetId(), jbb->GetId());
            } else {
                COMPILER_LOG(DEBUG, IFCONVERSION) << "Merged blocks " << tbb->GetId() << " and " << fbb->GetId()
                                                  << " into " << bb->GetId() << " using Select";
                GetGraph()->GetEventWriter().EventIfConversion(bb->GetId(), bb->GetGuestPc(), "Diamond_Phi",
                                                               tbb->GetId(), fbb->GetId(), -1);
            }
            return true;
        }
    }
    return false;
}

bool IfConversion::LoopInvariantPreventConversion(BasicBlock *bb)
{
    if (!g_options.IsCompilerLicmConditions()) {
        // Need to investigate may be it is always better to avoid IfConv for loop invariant condition
        return false;
    }
    if (!bb->IsLoopValid()) {
        return false;
    }
    auto loop = bb->GetLoop();
    if (loop->IsRoot()) {
        return false;
    }
    auto lastInst = bb->GetLastInst();
    for (auto &input : lastInst->GetInputs()) {
        if (input.GetInst()->GetBasicBlock()->GetLoop() == loop) {
            return false;
        }
    }
    return true;
}

bool IfConversion::IsConvertable(BasicBlock *bb, uint32_t *instCount)
{
    uint32_t total = 0;
    for (auto inst : bb->AllInsts()) {
        ASSERT(inst->GetOpcode() != Opcode::Phi);
        if (!inst->IsIfConvertable()) {
            return false;
        }
        total += static_cast<uint32_t>(inst->HasUsers());
    }
    *instCount = total;
    return true;
}

bool IfConversion::IsPhisAllowed(BasicBlock *bb, BasicBlock *pred1, BasicBlock *pred2, uint32_t *phiCount)
{
    uint32_t total = 0;

    for (auto phi : bb->PhiInsts()) {
        size_t index1 = phi->CastToPhi()->GetPredBlockIndex(pred1);
        size_t index2 = phi->CastToPhi()->GetPredBlockIndex(pred2);
        ASSERT(index1 != index2);

        auto inst1 = phi->GetInput(index1).GetInst();
        auto inst2 = phi->GetInput(index2).GetInst();
        if (inst1 == inst2) {
            // Otherwise DCE should remove Phi
            [[maybe_unused]] constexpr auto IMM_2 = 2;
            ASSERT(bb->GetPredsBlocks().size() > IMM_2);
            // No select needed
            continue;
        }

        // Select can be supported for float operands on the specific architectures (arm64 now)
        if (DataType::IsFloatType(phi->GetType()) && !canEncodeFloatSelect_) {
            return false;
        }

        // NOTE: LICM conditions pass moves condition chain invariants outside the loop
        // and consider branch probability. IfConversion can replaces it with SelectInst
        // but their If inst users lose branch probability information. This code prevents
        // such conversion until IfConversion can estimate branch probability.
        if (IsConditionChainPhi(phi)) {
            return false;
        }

        // One more Select
        total++;
    }
    *phiCount = total;
    return true;
}

bool IfConversion::IsConditionChainPhi(Inst *phi)
{
    if (!g_options.IsCompilerLicmConditions()) {
        return false;
    }

    auto loop = phi->GetBasicBlock()->GetLoop();
    for (auto &user : phi->GetUsers()) {
        auto userBb = user.GetInst()->GetBasicBlock();
        if (!userBb->GetLoop()->IsInside(loop)) {
            return false;
        }
    }
    return true;
}

// It is optimized for special SelectImm insts, so that using ubfx replacing test and csel inst on arm64 target.
// 2.i32  AddI v1, 0x1
// 3.i32  SelectImm v1, v2, v0, 0x2, CC_TST_EQ
// 4.i32  AddI v3, 0x1
// 5.i32  SelectImm v3, v4, v0, 0x4, CC_TST_EQ
// ====>
// 2.i32  ExtractBitfield, v0, 1, 1 [ZeroExtension]
// 3.i32  Add v1, v2
// 4.i32  ExtractBitfield, v0, 2, 1 [ZeroExtension]
// 5.i32  Add v3, v4
bool IfConversion::TryReplaceSelectImmVariablesWithConstants()
{
    bool result = false;
    for (auto it : GetGraph()->GetBlocksRPO()) {
        for (auto inst : it->AllInsts()) {
            if (inst->GetOpcode() != Opcode::SelectImm) {
                continue;
            }
            auto selectImm = inst->CastToSelectImm();
            auto cc = selectImm->GetCc();
            auto imm = selectImm->GetImm();
            // Optimized Conditions:
            // (1) condition code is TST_CC_EQ
            // (2) the imm of SelectImm inst is the power of 2
            // (3) the input1 of SelectImm inst is an AddI inst, and the imm of AddI inst is 1
            // (4) the input0 of SelectImm inst is equal to the input0 of the AddI inst in (3)
            if (cc != CC_TST_EQ || static_cast<int64_t>(imm) <= 0 || !helpers::math::IsPowerOfTwo(imm)) {
                continue;
            }
            auto operand0 = inst->GetInput(0).GetInst();
            auto operand1 = inst->GetInput(1).GetInst();
            if (operand1->GetOpcode() != Opcode::AddI) {
                continue;
            }
            auto addImm = operand1->CastToAddI()->GetImm();
            if (addImm != 1ULL) {
                continue;
            }
            if (operand1->GetInput(0).GetInst() != operand0) {
                continue;
            }

            auto operand2 = inst->GetInput(2).GetInst();
            auto newUbfxInst =
                GetGraph()->CreateInstExtractBitfield(inst, operand2, helpers::math::GetIntLog2(imm), 1U);
            auto newAddInst = GetGraph()->CreateInstAdd(inst, operand0, newUbfxInst);
            inst->InsertBefore(newUbfxInst);
            inst->InsertBefore(newAddInst);
            inst->ReplaceUsers(newAddInst);
            result = true;
        }
    }
    return result;
}

void IfConversion::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
}
}  // namespace ark::compiler
