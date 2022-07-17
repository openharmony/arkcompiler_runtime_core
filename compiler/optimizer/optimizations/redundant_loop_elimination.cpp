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
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/analysis/dominators_tree.h"
#include "redundant_loop_elimination.h"

#include "optimizer/ir/basicblock.h"
namespace panda::compiler {
bool RedundantLoopElimination::RunImpl()
{
    COMPILER_LOG(DEBUG, RLE_OPT) << "Run " << GetPassName();
    RunLoopsVisitor();

    if (is_applied_) {
        COMPILER_LOG(DEBUG, RLE_OPT) << GetPassName() << " is applied";
    }
    return is_applied_;
}

BasicBlock *RedundantLoopElimination::IsRedundant(Loop *loop) const
{
    BasicBlock *outside_succ = nullptr;
    for (auto block : loop->GetBlocks()) {
        // check that loop have only one exit and one outside blocks
        for (auto succ : block->GetSuccsBlocks()) {
            if (succ->GetLoop() != loop) {
                if (outside_succ == nullptr) {
                    outside_succ = succ;
                } else {
                    return nullptr;
                }
            }
        }
        // check that loop doesn't contains not redundant insts
        for (auto inst : block->AllInsts()) {
            if (inst->IsNotRemovable() && inst->GetOpcode() != Opcode::IfImm &&
                inst->GetOpcode() != Opcode::SafePoint) {
                return nullptr;
            }
            for (auto &user : inst->GetUsers()) {
                if (user.GetInst()->GetBasicBlock()->GetLoop() != loop) {
                    return nullptr;
                }
            }
        }
    }
    // Check that loop is finite.
    // We can remove only loops that is always finite.
    if (!CountableLoopParser(*loop).Parse().has_value()) {
        return nullptr;
    }
    return outside_succ;
}

void RedundantLoopElimination::DeleteLoop(Loop *loop, BasicBlock *outside_succ) const
{
    auto header = loop->GetHeader();
    auto pre_header = loop->GetPreHeader();
    ASSERT(loop->GetBackEdges().size() == 1);
    ASSERT(pre_header != nullptr);

    pre_header->ReplaceSucc(header, outside_succ, true);
    for (auto block : loop->GetBlocks()) {
        GetGraph()->DisconnectBlock(block, false, false);
    }
}

bool RedundantLoopElimination::TransformLoop(Loop *loop)
{
    COMPILER_LOG(DEBUG, RLE_OPT) << "Visit loop with id = " << loop->GetId();
    auto outside_succ = IsRedundant(loop);
    if (outside_succ != nullptr) {
        DeleteLoop(loop, outside_succ);
        is_applied_ = true;
        COMPILER_LOG(DEBUG, RLE_OPT) << "Loop with id = " << loop->GetId() << " is removed";
        GetGraph()->GetEventWriter().EventRedundantLoopElimination(loop->GetId(), loop->GetHeader()->GetGuestPc());
        return true;
    }
    return false;
}

void RedundantLoopElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}
}  // namespace panda::compiler
