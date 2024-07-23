/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

void IfConversion::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
}
}  // namespace ark::compiler
