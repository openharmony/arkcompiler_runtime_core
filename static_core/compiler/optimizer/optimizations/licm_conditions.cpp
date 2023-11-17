/*
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
#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"
#include "licm_conditions.h"

namespace panda::compiler {
bool LicmConditions::RunImpl()
{
    condition_chain_manager_.Reset();
    MarkerHolder hoistable_inst_mh(GetGraph());
    MarkerHolder processed_blocks_mh(GetGraph());
    hoistable_inst_marker_ = hoistable_inst_mh.GetMarker();
    processed_blocks_marker_ = processed_blocks_mh.GetMarker();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    GetGraph()->RunPass<DominatorsTree>();
    MarkHoistableInst();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    return is_applied_;
}

void LicmConditions::MarkHoistableInst()
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (!bb->IsLoopValid()) {
            continue;
        }
        auto loop = bb->GetLoop();
        if (loop->IsRoot() || loop->IsIrreducible()) {
            continue;
        }
        for (auto inst : bb->PhiInsts()) {
            if (AllInputsDominate(inst, loop)) {
                inst->SetMarker(hoistable_inst_marker_);
            }
        }
        if (bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM) {
            auto last_inst = bb->GetLastInst();
            if (AllInputsDominate(last_inst, loop)) {
                last_inst->SetMarker(hoistable_inst_marker_);
            }
        }
    }
}

bool LicmConditions::TransformLoop(Loop *loop)
{
    same_phi_input_.clear();
    condition_chains_cache_.Clear();
    FindHoistableConditionChains(loop);
    HoistConditionChains(loop);
    return true;
}

void LicmConditions::FindHoistableConditionChains(Loop *loop)
{
    condition_chains_ctx_.clear();
    for (auto block : loop->GetBlocks()) {
        auto chain = condition_chain_manager_.FindConditionChain(block);
        if (chain == nullptr) {
            continue;
        }
        auto multiple_preds_succ = chain->GetMultiplePredecessorsSuccessor();
        auto single_pred_succ = chain->GetSinglePredecessorSuccessor();
        COMPILER_LOG(DEBUG, LICM_COND_OPT)
            << "Found conditions chain " << chain->GetFirstBlock()->GetId() << "->" << chain->GetLastBlock()->GetId()
            << ", succs: " << multiple_preds_succ->GetId() << ", " << single_pred_succ->GetId();
        if (!IsHoistable(chain)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Skip not hoistable chain";
            continue;
        }
        auto hoist_phi_available = IsHoistPhiAvailable(chain, multiple_preds_succ->GetPredsBlocks(), single_pred_succ);
        if (!AllPhiAllowConditionChainHoisting(chain, multiple_preds_succ, hoist_phi_available)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Skip not all phi are suitable";
            continue;
        }
        condition_chains_ctx_.emplace_back(chain, multiple_preds_succ, single_pred_succ, hoist_phi_available);
    }
    std::sort(condition_chains_ctx_.begin(), condition_chains_ctx_.end(),
              [](auto a, auto b) { return a.GetChain()->GetSize() > b.GetChain()->GetSize(); });
}

bool LicmConditions::IsHoistable(const ConditionChain *chain)
{
    auto last = chain->GetEnd();
    for (auto bb_it = chain->GetBegin(); bb_it != last; ++bb_it) {
        auto bb = *bb_it;
        auto last_inst = bb->GetLastInst();
        if (last_inst->GetOpcode() != Opcode::IfImm) {
            return false;
        }
        if (!last_inst->IsMarked(hoistable_inst_marker_)) {
            return false;
        }
    }
    return true;
}

// After LICM pass all hoistable inputs should be moved out from loop
bool LicmConditions::AllInputsDominate(const Inst *inst, const Loop *loop)
{
    auto preheader = loop->GetPreHeader();
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        if (!inst->GetInput(i).GetInst()->GetBasicBlock()->IsDominate(preheader)) {
            return false;
        }
    }
    return true;
}

bool LicmConditions::IsHoistPhiAvailable(const ConditionChain *chain, ArenaVector<BasicBlock *> &preds,
                                         const BasicBlock *single_pred_succ)
{
    for (auto pred : preds) {
        if (!chain->Contains(pred) && pred != single_pred_succ) {
            return false;
        }
    }
    return true;
}

bool LicmConditions::AllPhiAllowConditionChainHoisting(const ConditionChain *chain,
                                                       const BasicBlock *multiple_preds_succ, bool hoist_phi_available)
{
    for (auto phi : multiple_preds_succ->PhiInsts()) {
        if (hoist_phi_available && phi->IsMarked(hoistable_inst_marker_)) {
            continue;
        }
        auto same_input = SamePhiInputFromChain(phi, chain);
        if (same_input == nullptr) {
            return false;
        }
        same_phi_input_.insert({std::make_pair(chain, phi), same_input});
    }
    return true;
}

Inst *LicmConditions::SamePhiInputFromChain(Inst *phi, const ConditionChain *chain)
{
    Inst *saved_input = nullptr;
    auto bb = phi->GetBasicBlock();
    for (auto pred : bb->GetPredsBlocks()) {
        if (!chain->Contains(pred)) {
            continue;
        }
        auto pred_index = bb->GetPredBlockIndex(pred);
        auto input = phi->GetInput(pred_index).GetInst();
        if (saved_input == nullptr) {
            saved_input = input;
        } else if (saved_input != input) {
            return nullptr;
        } else {
            continue;
        }
    }
    return saved_input;
}

/*
 * Move condition chains which look like
 *
 *          | |
 *          v v
 *          [A]------\
 *           |       |
 *           |       v
 *           |<-----[B]
 *           |       |
 *           v       v
 *       -->[S0]    [S1]<--
 *           |       |
 *           v       v
 *
 * out of the loop if possible.
 * After whole chain is moved out it looks like
 *
 *           |
 *           v
 *          [A]------\
 *           |       |
 *           |       v
 *           |<-----[B]
 *           |       |
 *           |       v
 *           |    [empty_block]
 *           |       |
 *           v       v
 *          [phi_block]
 *               |
 *               v
 *          [loop preheader]
 *
 * phi_block contains either new phi instructions which is result of condition chain
 * (single_condition_block uses it) or phi which was hoisted from `multiple_predecessors successor.
 * Condition chain is replaced in loop with single_condition_block
 *
 *              |
 *              v
 *     [single_condition_block]
 *           |       |
 *           v       v
 *       -->[S0]    [S1]<--
 *           |       |
 *           v       v
 */
void LicmConditions::HoistConditionChains(Loop *loop)
{
    auto append_bb = loop->GetPreHeader();
    for (auto chain_ctx : condition_chains_ctx_) {
        auto chain = chain_ctx.GetChain();
        auto chain_first_block = chain->GetFirstBlock();
        if (chain_first_block->IsMarked(processed_blocks_marker_)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT)
                << "Skip chain with first block #" << chain_first_block->GetId() << ", longer chain was processed";
            continue;
        }

        auto multiple_preds_succ = chain->GetMultiplePredecessorsSuccessor();
        auto single_pred_succ = chain->GetSinglePredecessorSuccessor();

        COMPILER_LOG(DEBUG, LICM_COND_OPT)
            << "Process conditions chain " << chain_first_block->GetId() << "->" << chain->GetLastBlock()->GetId()
            << ", succs: " << multiple_preds_succ->GetId() << ", " << single_pred_succ->GetId();

        SaveProcessedBlocks(chain);
        SplitChainFirstBasicBlock(chain);

        // update chain successors because they can be changed during previous transformations
        chain_ctx.SetMultiplePredecessorsSuccessor(multiple_preds_succ);
        chain_ctx.SetSingleSPredecessorSuccessor(single_pred_succ);

        append_bb = ReplaceChainWithSingleBlock(append_bb, chain_ctx);

        is_applied_ = true;
    }
}

void LicmConditions::SaveProcessedBlocks(const ConditionChain *chain)
{
    std::for_each(chain->GetBegin(), chain->GetEnd(),
                  [this](BasicBlock *bb) { bb->SetMarker(processed_blocks_marker_); });
}

void LicmConditions::SaveMulitplePredecessorsSuccessorPreds(const BasicBlock *bb)
{
    multiple_predecessors_successor_preds_.clear();
    std::copy(bb->GetPredsBlocks().begin(), bb->GetPredsBlocks().end(),
              std::back_inserter(multiple_predecessors_successor_preds_));
}

void LicmConditions::SplitChainFirstBasicBlock(ConditionChain *chain)
{
    auto chain_first_bb = chain->GetFirstBlock();
    auto prelast_inst = chain_first_bb->GetLastInst()->GetPrev();
    if (prelast_inst == nullptr) {
        return;
    }
    auto new_first_bb = chain_first_bb->SplitBlockAfterInstruction(prelast_inst, true);
    chain->SetFirstBlock(new_first_bb);
    COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Split first chain block " << chain_first_bb->GetId() << "->"
                                       << new_first_bb->GetId();
}

BasicBlock *LicmConditions::ReplaceChainWithSingleBlock(BasicBlock *append_bb, const ConditionChainContext &chain_ctx)
{
    auto chain = chain_ctx.GetChain();
    // try to find condition chain which is looking like this
    auto cached_phi = condition_chains_cache_.FindPhi(chain);
    auto multiple_preds_succ = chain_ctx.GetMultiplePredecessorsSuccessor();
    auto single_pred_succ = chain_ctx.GetSinglePredecessorSuccessor();
    SaveMulitplePredecessorsSuccessorPreds(multiple_preds_succ);

    auto single_condition_block = GetGraph()->CreateEmptyBlock();
    AdjustPredecessorEdges(chain->GetFirstBlock(), single_condition_block);

    single_condition_block->AddSucc(multiple_preds_succ);
    // NOTE: multiple_preds_succ preds are not fixed
    auto chain_last_block = chain->GetLastBlock();
    single_pred_succ->ReplacePred(chain_last_block, single_condition_block);

    auto phi_block = GetGraph()->CreateEmptyBlock();
    auto append_bb_succ = append_bb->GetSuccessor(0);
    append_bb->ReplaceSucc(append_bb_succ, chain->GetFirstBlock());
    append_bb_succ->ReplacePred(append_bb, phi_block);

    auto empty_block = GetGraph()->CreateEmptyBlock();
    chain_last_block->ReplaceSucc(single_pred_succ, empty_block, true);

    UpdateMultiplePredecessorsSuccessorsPreds(chain_ctx, phi_block, empty_block);
    UpdatePhis(chain, multiple_preds_succ, phi_block, chain_ctx.IsHoistPhiAvailable());

    Inst *phi_inst;
    if (cached_phi != nullptr) {
        phi_inst = cached_phi;
    } else {
        phi_inst = AddPhiInst(phi_block, chain);
        condition_chains_cache_.Insert(chain, phi_inst);
    }
    AddSingleIfImmInst(single_condition_block, chain, phi_inst);
    return phi_block;
}

PhiInst *LicmConditions::AddPhiInst(BasicBlock *bb, const ConditionChain *chain)
{
    auto graph = bb->GetGraph();
    auto one_cnst = graph->FindOrCreateConstant(1);
    auto zero_cnst = graph->FindOrCreateConstant(0);
    auto phi_inst = graph->CreateInstPhi(DataType::BOOL, INVALID_PC);
    bb->AppendPhi(phi_inst);
    for (auto pred : bb->GetPredsBlocks()) {
        phi_inst->AppendInput(chain->Contains(pred) ? one_cnst : zero_cnst);
    }
    return phi_inst;
}

void LicmConditions::AddSingleIfImmInst(BasicBlock *bb, const ConditionChain *chain, Inst *input)
{
    auto orig_if_inst = chain->GetLastBlock()->GetLastInst()->CastToIfImm();
    auto if_inst = bb->GetGraph()->CreateInstIfImm(DataType::NO_TYPE, 0, input, 0, DataType::BOOL, ConditionCode::CC_NE,
                                                   orig_if_inst->GetMethod());
    bb->AppendInst(if_inst);
}

void LicmConditions::AdjustPredecessorEdges(BasicBlock *chain_first_bb, BasicBlock *bb)
{
    for (auto pred : chain_first_bb->GetPredsBlocks()) {
        pred->ReplaceSucc(chain_first_bb, bb);
    }
    chain_first_bb->GetPredsBlocks().clear();
}

void LicmConditions::UpdateMultiplePredecessorsSuccessorsPreds(const ConditionChainContext &chain_ctx,
                                                               BasicBlock *phi_block, BasicBlock *empty_block)
{
    auto chain = chain_ctx.GetChain();
    auto multiple_preds_succ = chain_ctx.GetMultiplePredecessorsSuccessor();
    auto single_pred_succ = chain_ctx.GetSinglePredecessorSuccessor();
    multiple_predecessors_successor_removed_pred_indices_.clear();
    // keep predecessors order in phi_block
    for (auto bb : multiple_predecessors_successor_preds_) {
        if (chain->Contains(bb)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT)
                << "Update chain block " << bb->GetId() << " successor: " << multiple_preds_succ->GetId() << "->"
                << phi_block->GetId();
            bb->ReplaceSucc(multiple_preds_succ, phi_block, true);
            auto pred_index = multiple_preds_succ->GetPredBlockIndex(bb);
            multiple_preds_succ->RemovePred(pred_index);
            multiple_predecessors_successor_removed_pred_indices_.push_back(pred_index);
        } else if (bb == single_pred_succ) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT)
                << "Add new edge to phi_block corresponding to edge between chain successors";
            empty_block->AddSucc(phi_block, true);
        } else {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Skip predecessor " << bb->GetId();
        }
    }
    if (empty_block->GetSuccsBlocks().empty()) {
        COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Add last edge to phi_block";
        empty_block->AddSucc(phi_block, true);
    }
}

void LicmConditions::UpdatePhis(const ConditionChain *chain, BasicBlock *multiple_preds_succ, BasicBlock *phi_block,
                                bool hoist_phi_available)
{
    for (auto phi : multiple_preds_succ->PhiInstsSafe()) {
        if (hoist_phi_available && phi->IsMarked(hoistable_inst_marker_)) {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Hoist phi " << phi->GetId();
            multiple_preds_succ->EraseInst(phi);
            // preds order was preserved
            phi_block->AppendPhi(phi);
            if (phi->GetInputsCount() >= phi_block->GetPredsBlocks().size()) {
                ASSERT(phi->GetInputsCount() == phi_block->GetPredsBlocks().size());
                continue;
            }
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Add dummy input";
            if (DataType::IsReference(phi->GetType())) {
                phi->AppendInput(GetGraph()->GetOrCreateNullPtr());
            } else {
                phi->AppendInput(GetGraph()->FindOrCreateConstant(0));
            }
            ASSERT(phi->GetInputsCount() == phi_block->GetPredsBlocks().size());
        } else {
            COMPILER_LOG(DEBUG, LICM_COND_OPT) << "Update inputs for phi " << phi->GetId();
            auto key = std::make_pair(chain, phi);
            ASSERT(same_phi_input_.count(key) != 0);
            phi->AppendInput(same_phi_input_[key]);
            for (size_t i : multiple_predecessors_successor_removed_pred_indices_) {
                phi->RemoveInput(i);
            }
        }
    }
}

void LicmConditions::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}
}  // namespace panda::compiler
