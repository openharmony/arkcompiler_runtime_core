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

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/optimizations/if_merging.h"
#include "compiler_logger.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"

namespace panda::compiler {
IfMerging::IfMerging(Graph *graph) : Optimization(graph) {}

bool IfMerging::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    // Do not try to fix LoopAnalyzer during optimization
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    is_applied_ = false;
    true_branch_marker_ = GetGraph()->NewMarker();
    ArenaVector<BasicBlock *> blocks(GetGraph()->GetVectorBlocks(), GetGraph()->GetLocalAllocator()->Adapter());
    for (auto block : blocks) {
        if (block == nullptr) {
            continue;
        }
        if (TryMergeEquivalentIfs(block) || TrySimplifyConstantPhi(block)) {
            is_applied_ = true;
        }
    }
    GetGraph()->RemoveUnreachableBlocks();

#ifndef NDEBUG
    CheckDomTreeValid();
#endif
    GetGraph()->EraseMarker(true_branch_marker_);
    COMPILER_LOG(DEBUG, IF_MERGING) << "If merging complete";
    return is_applied_;
}

void IfMerging::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

// Splits block while it contains If comparing phi with constant inputs to another constant
// (After removing one If and joining blocks a new If may go to this block)
bool IfMerging::TrySimplifyConstantPhi(BasicBlock *block)
{
    auto is_applied = false;
    while (TryRemoveConstantPhiIf(block)) {
        is_applied = true;
    }
    return is_applied;
}

// Tries to remove If that compares phi with constant inputs to another constant
bool IfMerging::TryRemoveConstantPhiIf(BasicBlock *if_block)
{
    auto if_imm = GetIfImm(if_block);
    if (if_imm == nullptr || if_block->GetGraph() == nullptr || if_block->IsTry()) {
        return false;
    }
    auto input = if_imm->GetInput(0).GetInst();
    if (input->GetOpcode() != Opcode::Compare) {
        return false;
    }
    auto compare = input->CastToCompare();
    auto lhs = compare->GetInput(0).GetInst();
    auto rhs = compare->GetInput(1).GetInst();
    if (rhs->IsConst() && lhs->GetOpcode() == Opcode::Phi &&
        TryRemoveConstantPhiIf(if_imm, lhs->CastToPhi(), rhs->CastToConstant()->GetRawValue(), compare->GetCc())) {
        return true;
    }
    if (lhs->IsConst() && rhs->GetOpcode() == Opcode::Phi &&
        TryRemoveConstantPhiIf(if_imm, rhs->CastToPhi(), lhs->CastToConstant()->GetRawValue(),
                               SwapOperandsConditionCode(compare->GetCc()))) {
        return true;
    }
    return false;
}

// Returns IfImm instruction terminating the block or nullptr
IfImmInst *IfMerging::GetIfImm(BasicBlock *block)
{
    if (block->IsEmpty() || block->GetLastInst()->GetOpcode() != Opcode::IfImm) {
        return nullptr;
    }
    auto if_imm = block->GetLastInst()->CastToIfImm();
    if ((if_imm->GetCc() != CC_EQ && if_imm->GetCc() != CC_NE) || if_imm->GetImm() != 0) {
        return nullptr;
    }
    return if_imm;
}

// If bb and its dominator end with the same IfImm instruction, removes IfImm in bb and returns true
bool IfMerging::TryMergeEquivalentIfs(BasicBlock *bb)
{
    auto if_imm = GetIfImm(bb);
    if (if_imm == nullptr || bb->GetGraph() == nullptr || bb->IsTry()) {
        return false;
    }
    if (bb->GetPredsBlocks().size() != MAX_SUCCS_NUM) {
        return false;
    }
    auto dom = bb->GetDominator();
    auto dom_if = GetIfImm(dom);
    if (dom_if == nullptr || dom_if->GetInput(0).GetInst() != if_imm->GetInput(0).GetInst()) {
        return false;
    }
    auto inverted_if = IsIfInverted(bb, dom_if);
    if (inverted_if == std::nullopt) {
        // Not applied, true and false branches of dominate If intersect
        return false;
    }
    auto true_bb = if_imm->GetEdgeIfInputTrue();
    auto false_bb = if_imm->GetEdgeIfInputFalse();
    if (!MarkInstBranches(bb, true_bb, false_bb)) {
        // Not applied, there are instructions in bb used both in true and false branches
        return false;
    }
    COMPILER_LOG(DEBUG, IF_MERGING) << "Equivalent IfImm instructions were found. Dominant block id = " << dom->GetId()
                                    << ", dominated block id = " << bb->GetId();
    bb->RemoveInst(if_imm);
    SplitBlockWithEquivalentIf(bb, true_bb, *inverted_if);
    return true;
}

// In case If has constant phi input and can be removed, removes it and returns true
bool IfMerging::TryRemoveConstantPhiIf(IfImmInst *if_imm, PhiInst *phi, uint64_t constant, ConditionCode cc)
{
    auto bb = phi->GetBasicBlock();
    if (bb != if_imm->GetBasicBlock()) {
        return false;
    }
    for (auto input : phi->GetInputs()) {
        if (!input.GetInst()->IsConst()) {
            return false;
        }
    }
    auto true_bb = if_imm->GetEdgeIfInputTrue();
    auto false_bb = if_imm->GetEdgeIfInputFalse();
    if (!MarkInstBranches(bb, true_bb, false_bb)) {
        // Not applied, there are instructions in bb used both in true and false branches
        return false;
    }
    for (auto inst : bb->PhiInsts()) {
        if (inst != phi) {
            return false;
        }
    }

    COMPILER_LOG(DEBUG, IF_MERGING) << "Comparison of constant and Phi instruction with constant inputs was found. "
                                    << "Phi id = " << phi->GetId() << ", IfImm id = " << if_imm->GetId();
    bb->RemoveInst(if_imm);
    SplitBlockWithConstantPhi(bb, true_bb, phi, constant, cc);
    return true;
}

// Marks instructions in bb for moving to true or false branch
// Returns true if it was possible
bool IfMerging::MarkInstBranches(BasicBlock *bb, BasicBlock *true_bb, BasicBlock *false_bb)
{
    for (auto inst : bb->AllInstsSafeReverse()) {
        if (inst->IsNotRemovable() && inst != bb->GetLastInst()) {
            return false;
        }
        auto true_branch = false;
        auto false_branch = false;
        for (auto &user : inst->GetUsers()) {
            auto user_branch = GetUserBranch(user.GetInst(), bb, true_bb, false_bb);
            if (!user_branch) {
                // If instruction has users not in true or false branch, than splitting is
                // impossible (even if the instruction is Phi)
                return false;
            }
            if (*user_branch) {
                true_branch = true;
            } else {
                false_branch = true;
            }
        }
        if (inst->IsPhi()) {
            // Phi instruction will be replaced with one of its inputs
            continue;
        }
        if (true_branch && false_branch) {
            // If instruction is used in both branches, it would need to be copied -> not applied
            return false;
        }
        if (true_branch) {
            inst->SetMarker(true_branch_marker_);
        } else {
            inst->ResetMarker(true_branch_marker_);
        }
    }
    return true;
}

std::optional<bool> IfMerging::GetUserBranch(Inst *user_inst, BasicBlock *bb, BasicBlock *true_bb, BasicBlock *false_bb)
{
    auto user_bb = user_inst->GetBasicBlock();
    if (user_bb == bb) {
        return user_inst->IsMarked(true_branch_marker_);
    }
    if (IsDominateEdge(true_bb, user_bb)) {
        // user_inst can be executed only in true branch of If
        return true;
    }
    if (IsDominateEdge(false_bb, user_bb)) {
        // user_inst can be executed only in false branch of If
        return false;
    }
    // user_inst can be executed both in true and false branches
    return std::nullopt;
}

// Returns true if edge_bb is always the first block in the path
// from its predecessor to target_bb
bool IfMerging::IsDominateEdge(BasicBlock *edge_bb, BasicBlock *target_bb)
{
    return edge_bb->IsDominate(target_bb) && edge_bb->GetPredsBlocks().size() == 1;
}

void IfMerging::SplitBlockWithEquivalentIf(BasicBlock *bb, BasicBlock *true_bb, bool inverted_if)
{
    auto true_branch_bb = SplitBlock(bb);
    // Phi instructions are replaced with one of their inputs
    for (auto inst : bb->PhiInstsSafe()) {
        for (auto it = inst->GetUsers().begin(); it != inst->GetUsers().end(); it = inst->GetUsers().begin()) {
            auto user_inst = it->GetInst();
            auto user_bb = user_inst->GetBasicBlock();
            auto phi_input_block_index = (user_bb == true_branch_bb || IsDominateEdge(true_bb, user_bb)) ? 0 : 1;
            if (inverted_if) {
                phi_input_block_index = 1 - phi_input_block_index;
            }
            user_inst->ReplaceInput(inst, inst->CastToPhi()->GetPhiInput(bb->GetPredecessor(phi_input_block_index)));
        }
        bb->RemoveInst(inst);
    }

    // True branches of both Ifs are disconnected from bb and connected to the new block
    true_bb->ReplacePred(bb, true_branch_bb);
    bb->RemoveSucc(true_bb);
    auto true_pred = bb->GetPredecessor(inverted_if ? 1 : 0);
    true_pred->ReplaceSucc(bb, true_branch_bb);
    bb->RemovePred(true_pred);

    FixDominatorsTree(true_branch_bb, bb);
}

void IfMerging::SplitBlockWithConstantPhi(BasicBlock *bb, BasicBlock *true_bb, PhiInst *phi, uint64_t constant,
                                          ConditionCode cc)
{
    if (true_bb == bb) {
        // Avoid case when true_bb is bb itself
        true_bb = bb->InsertNewBlockToSuccEdge(true_bb);
    }
    auto true_branch_bb = SplitBlock(bb);
    // Move Phi inputs corresponding to true branch to a new Phi instruction
    auto true_branch_phi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
    for (auto i = phi->GetInputsCount(); i > 0U; --i) {
        auto inst = phi->GetInput(i - 1).GetInst();
        if (Compare(cc, inst->CastToConstant()->GetRawValue(), constant)) {
            auto bb_index = phi->GetPhiInputBbNum(i - 1);
            phi->RemoveInput(i - 1);
            bb->GetPredBlockByIndex(bb_index)->ReplaceSucc(bb, true_branch_bb);
            bb->RemovePred(bb_index);
            true_branch_phi->AppendInput(inst);
        }
    }
    for (auto it = phi->GetUsers().begin(); it != phi->GetUsers().end();) {
        auto user_inst = it->GetInst();
        auto user_bb = user_inst->GetBasicBlock();
        // Skip list nodes that can be deleted inside ReplaceInput
        while (it != phi->GetUsers().end() && it->GetInst() == user_inst) {
            ++it;
        }
        if (user_bb == true_branch_bb || IsDominateEdge(true_bb, user_bb)) {
            user_inst->ReplaceInput(phi, true_branch_phi);
        }
    }
    // Phi instructions with single inputs need to be removed
    if (true_branch_phi->GetInputsCount() == 1) {
        true_branch_phi->ReplaceUsers(true_branch_phi->GetInput(0).GetInst());
        true_branch_phi->RemoveInputs();
    } else {
        true_branch_bb->AppendPhi(true_branch_phi);
    }
    if (phi->GetInputsCount() == 1) {
        phi->ReplaceUsers(phi->GetInput(0).GetInst());
        bb->RemoveInst(phi);
    }

    // True branches of both Ifs are disconnected from bb and connected to the new block
    true_bb->ReplacePred(bb, true_branch_bb);
    bb->RemoveSucc(true_bb);
    FixDominatorsTree(true_branch_bb, bb);
}

// Moves instructions that should be in the true branch to a new block and returns it
BasicBlock *IfMerging::SplitBlock(BasicBlock *bb)
{
    auto true_branch_bb = GetGraph()->CreateEmptyBlock(bb->GetGuestPc());
    // Dominators tree will be fixed after connecting the new block
    GetGraph()->GetAnalysis<DominatorsTree>().SetValid(true);
    for (auto inst : bb->InstsSafeReverse()) {
        if (inst->IsMarked(true_branch_marker_)) {
            bb->EraseInst(inst);
            true_branch_bb->PrependInst(inst);
        }
    }
    return true_branch_bb;
}

// Makes dominator tree valid after false_branch_bb was split into two blocks
void IfMerging::FixDominatorsTree(BasicBlock *true_branch_bb, BasicBlock *false_branch_bb)
{
    auto &dom_tree = GetGraph()->GetAnalysis<DominatorsTree>();
    auto dom = false_branch_bb->GetDominator();
    // Dominator of bb will remain (maybe not immediate) dominator of blocks dominated by bb
    for (auto dominated_block : false_branch_bb->GetDominatedBlocks()) {
        dom_tree.SetDomPair(dom, dominated_block);
    }
    false_branch_bb->ClearDominatedBlocks();

    TryJoinSuccessorBlock(true_branch_bb);
    TryJoinSuccessorBlock(false_branch_bb);
    TryUpdateDominator(true_branch_bb);
    TryUpdateDominator(false_branch_bb);
}

void IfMerging::TryJoinSuccessorBlock(BasicBlock *bb)
{
    if (bb->GetSuccsBlocks().size() == 1 && bb->GetSuccessor(0)->GetPredsBlocks().size() == 1 &&
        !bb->GetSuccessor(0)->IsPseudoControlFlowBlock() && bb->IsTry() == bb->GetSuccessor(0)->IsTry() &&
        bb->GetSuccessor(0) != bb) {
        bb->JoinSuccessorBlock();
    }
}

// If bb has only one precessor, set it as dominator of bb
void IfMerging::TryUpdateDominator(BasicBlock *bb)
{
    if (bb->GetPredsBlocks().size() != 1) {
        return;
    }
    auto pred = bb->GetPredecessor(0);
    auto dom = bb->GetDominator();
    if (dom != pred) {
        if (dom != nullptr) {
            dom->RemoveDominatedBlock(bb);
        }
        GetGraph()->GetAnalysis<DominatorsTree>().SetDomPair(pred, bb);
    }
}

#ifndef NDEBUG
// Checks if dominators in dominators tree are indeed dominators (but not necessary immediate)
void IfMerging::CheckDomTreeValid()
{
    ArenaVector<BasicBlock *> dominators(GetGraph()->GetVectorBlocks().size(),
                                         GetGraph()->GetLocalAllocator()->Adapter());
    GetGraph()->GetAnalysis<DominatorsTree>().SetValid(true);
    for (auto block : GetGraph()->GetBlocksRPO()) {
        dominators[block->GetId()] = block->GetDominator();
    }
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->RunPass<DominatorsTree>();

    for (auto block : GetGraph()->GetBlocksRPO()) {
        auto dom = dominators[block->GetId()];
        ASSERT_DO(dom == nullptr || dom->IsDominate(block),
                  (std::cerr << "Basic block with id " << block->GetId() << " has incorrect dominator with id "
                             << dom->GetId() << std::endl));
    }
}
#endif
}  // namespace panda::compiler
