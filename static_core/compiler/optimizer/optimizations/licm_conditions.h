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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_CONDITIONS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_CONDITIONS_H

#include "optimizer/optimizations/loop_transform.h"
#include "condition_chain_manager.h"
#include "condition_chain_cache.h"
#include "compiler_options.h"

namespace panda::compiler {

class ConditionChainContext {
public:
    ConditionChainContext(ConditionChain *chain, BasicBlock *multiple_predecessors_successor,
                          BasicBlock *single_predecessor_successor, bool hoist_phi_available)
        : chain_(chain),
          multiple_predecessors_successor_(multiple_predecessors_successor),
          single_predecessor_successor_(single_predecessor_successor),
          hoist_phi_available_(hoist_phi_available)
    {
    }

    ConditionChain *GetChain() const
    {
        return chain_;
    }

    BasicBlock *GetMultiplePredecessorsSuccessor() const
    {
        return multiple_predecessors_successor_;
    }

    BasicBlock *GetSinglePredecessorSuccessor() const
    {
        return single_predecessor_successor_;
    }

    void SetMultiplePredecessorsSuccessor(BasicBlock *multiple_predecessors_successor)
    {
        multiple_predecessors_successor_ = multiple_predecessors_successor;
    }

    void SetSingleSPredecessorSuccessor(BasicBlock *single_predecessor_successor)
    {
        single_predecessor_successor_ = single_predecessor_successor;
    }

    bool IsHoistPhiAvailable() const
    {
        return hoist_phi_available_;
    }

private:
    ConditionChain *chain_;
    BasicBlock *multiple_predecessors_successor_;
    BasicBlock *single_predecessor_successor_;
    bool hoist_phi_available_;
};

class LicmConditions : public LoopTransform<LoopExitPoint::LOOP_EXIT_HEADER> {
public:
    explicit LicmConditions(Graph *graph)
        : LoopTransform(graph),
          condition_chains_ctx_(graph->GetLocalAllocator()->Adapter()),
          same_phi_input_(graph->GetLocalAllocator()->Adapter()),
          multiple_predecessors_successor_preds_(graph->GetLocalAllocator()->Adapter()),
          multiple_predecessors_successor_removed_pred_indices_(graph->GetLocalAllocator()->Adapter()),
          condition_chains_cache_(graph->GetLocalAllocator()),
          condition_chain_manager_(graph->GetLocalAllocator())
    {
    }
    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "LICM_conditions";
    }

    bool IsEnable() const override
    {
        return OPTIONS.IsCompilerLicmConditions();
    }

    void InvalidateAnalyses() override;

private:
    static bool IsHoistPhiAvailable(const ConditionChain *chain, ArenaVector<BasicBlock *> &preds,
                                    const BasicBlock *single_pred_succ);
    bool TransformLoop(Loop *loop) override;
    void MarkHoistableInst();
    bool IsHoistable(const ConditionChain *chain);
    bool AllInputsDominate(const Inst *inst, const Loop *loop);
    void FindHoistableConditionChains(Loop *loop);
    bool AllPhiAllowConditionChainHoisting(const ConditionChain *chain, const BasicBlock *multiple_preds_successor,
                                           bool hoist_phi_available);
    Inst *SamePhiInputFromChain(Inst *phi, const ConditionChain *chain);
    void HoistConditionChains(Loop *loop);
    void SplitChainFirstBasicBlock(ConditionChain *chain);
    BasicBlock *ReplaceChainWithSingleBlock(BasicBlock *append_bb, const ConditionChainContext &chain);
    void SaveMulitplePredecessorsSuccessorPreds(const BasicBlock *bb);
    PhiInst *AddPhiInst(BasicBlock *bb, const ConditionChain *chain);
    void AddSingleIfImmInst(BasicBlock *bb, const ConditionChain *chain, Inst *input);
    void SaveProcessedBlocks(const ConditionChain *chain);
    void AdjustPredecessorEdges(BasicBlock *chain_first_bb, BasicBlock *bb);
    void UpdateMultiplePredecessorsSuccessorsPreds(const ConditionChainContext &chain_ctx, BasicBlock *phi_block,
                                                   BasicBlock *empty_block);
    void UpdatePhis(const ConditionChain *chain, BasicBlock *multiple_preds_succ, BasicBlock *phi_block,
                    bool hoist_phi_available);
    ArenaVector<ConditionChainContext> condition_chains_ctx_;
    ArenaUnorderedMap<std::pair<const ConditionChain *, Inst *>, Inst *, PairHash> same_phi_input_;
    ArenaVector<BasicBlock *> multiple_predecessors_successor_preds_;
    ArenaVector<size_t> multiple_predecessors_successor_removed_pred_indices_;
    ConditionChainCache condition_chains_cache_;
    Marker processed_blocks_marker_ {UNDEF_MARKER};
    Marker hoistable_inst_marker_ {UNDEF_MARKER};
    ConditionChainManager condition_chain_manager_;
    bool is_applied_ {false};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_CONDITIONS_H
