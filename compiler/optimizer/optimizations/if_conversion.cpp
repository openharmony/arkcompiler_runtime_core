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

#include "compiler_logger.h"
#include "if_conversion.h"

#include "optimizer/ir/basicblock.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"

namespace panda::compiler {
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

    uint32_t inst_count = 0;
    uint32_t phi_count = 0;
    if (IsConvertable(tbb, &inst_count) && IsPhisAllowed(fbb, tbb, bb, &phi_count)) {
        COMPILER_LOG(DEBUG, IFCONVERSION)
            << "Triangle pattern was found in Block #" << bb->GetId() << " with " << inst_count
            << " convertible instruction(s) and " << phi_count << " Phi(s) to process";
        if (inst_count <= limit_ && phi_count <= limit_) {
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

    uint32_t tbb_inst = 0;
    uint32_t fbb_inst = 0;
    uint32_t phi_count = 0;
    if (IsConvertable(tbb, &tbb_inst) && IsConvertable(fbb, &fbb_inst) && IsPhisAllowed(jbb, tbb, fbb, &phi_count)) {
        COMPILER_LOG(DEBUG, IFCONVERSION)
            << "Diamond pattern was found in Block #" << bb->GetId() << " with " << (tbb_inst + fbb_inst)
            << " convertible instruction(s) and " << phi_count << " Phi(s) to process";
        if (tbb_inst + fbb_inst <= limit_ && phi_count <= limit_) {
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

bool IfConversion::IsConvertable(BasicBlock *bb, uint32_t *inst_count)
{
    uint32_t total = 0;
    for (auto inst : bb->AllInsts()) {
        ASSERT(inst->GetOpcode() != Opcode::Phi);
        if (!inst->IsIfConvertable()) {
            return false;
        }
        total += static_cast<uint32_t>(inst->HasUsers());
    }
    *inst_count = total;
    return true;
}

bool IfConversion::IsPhisAllowed(BasicBlock *bb, BasicBlock *pred1, BasicBlock *pred2, uint32_t *phi_count)
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

        // Select can't support float operands
        if (DataType::IsFloatType(phi->GetType())) {
            return false;
        }

        // One more Select
        total++;
    }
    *phi_count = total;
    return true;
}

void IfConversion::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
}
}  // namespace panda::compiler
