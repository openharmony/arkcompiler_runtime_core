/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_CLEANUP_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_CLEANUP_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace panda::compiler {
class Cleanup final : public Optimization {
public:
    explicit Cleanup(Graph *graph, bool light_mode = true)
        : Optimization(graph),
          empty1_(graph->GetLocalAllocator()->Adapter()),
          empty2_(graph->GetLocalAllocator()->Adapter()),
          saved_preds_(graph->GetLocalAllocator()->Adapter()),
          dead_(graph->GetLocalAllocator()->Adapter()),
          temp_(graph->GetLocalAllocator()->Adapter()),
          ancestors_(graph->GetLocalAllocator()->Adapter()),
          buckets_(graph->GetLocalAllocator()->Adapter()),
          idoms_(graph->GetLocalAllocator()->Adapter()),
          labels_(graph->GetLocalAllocator()->Adapter()),
          parents_(graph->GetLocalAllocator()->Adapter()),
          semi_(graph->GetLocalAllocator()->Adapter()),
          vertices_(graph->GetLocalAllocator()->Adapter()),
          map_(graph->GetLocalAllocator()->Adapter()),
          light_mode_(light_mode)
    {
    }

    NO_MOVE_SEMANTIC(Cleanup);
    NO_COPY_SEMANTIC(Cleanup);
    ~Cleanup() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "Cleanup";
    }

    void InvalidateAnalyses() override;
    static bool SkipBasicBlock(BasicBlock *bb);

private:
    bool CanBeMerged(BasicBlock *bb);

    bool RunOnce(ArenaSet<BasicBlock *> *empty_blocks, ArenaSet<BasicBlock *> *new_empty_blocks, bool simple_dce);
    void RemoveDeadPhi(BasicBlock *bb, ArenaSet<BasicBlock *> *new_empty_blocks);
    bool ProcessBB(BasicBlock *bb, Marker dead_mrk, ArenaSet<BasicBlock *> *new_empty_blocks);
    bool CheckSpecialTriangle(BasicBlock *bb);

    void MarkInlinedCaller(Marker live_mrk, Inst *save_state);
    bool IsRemovableCall(Inst *inst);
    template <bool LIGHT_MODE>
    void MarkLiveRec(Marker live_mrk, Inst *inst);
    template <bool LIGHT_MODE>
    void MarkLiveInstructions(Marker dead_mrk, Marker live_mrk);
    template <bool LIGHT_MODE>
    bool Dce(Marker dead_mrk, ArenaSet<BasicBlock *> *new_empty_blocks);

    void SetLiveRec(Inst *inst, Marker mrk, Marker live_mrk);
    void LiveUserSearchRec(Inst *inst, Marker mrk, Marker live_mrk, Marker dead_mrk);
    bool SimpleDce(Marker dead_mrk, ArenaSet<BasicBlock *> *new_empty_blocks);
    void Marking(Marker dead_mrk, Marker mrk, Marker live_mrk);
    bool Removal(ArenaSet<BasicBlock *> *new_empty_blocks);

    bool PhiChecker();
    bool PhiCheckerLight() const;
    void BuildDominators();

    ArenaSet<BasicBlock *> empty1_;
    ArenaSet<BasicBlock *> empty2_;
    ArenaVector<BasicBlock *> saved_preds_;
    InstVector dead_;
    InstVector temp_;

    InstVector ancestors_;
    ArenaVector<InstVector> buckets_;
    InstVector idoms_;
    InstVector labels_;
    InstVector parents_;
    ArenaVector<int32_t> semi_;
    InstVector vertices_;
    ArenaUnorderedMap<Inst *, uint32_t> map_;

    Inst *fake_root_ {nullptr};
    static constexpr int32_t DEFAULT_DFS_VAL = 0;
    // number of the inst according to the order it is reached during the DFS
    int32_t dfs_num_ {DEFAULT_DFS_VAL};

    bool light_mode_ {true};

    inline uint32_t GetInstId(Inst *inst) const
    {
        ASSERT(map_.count(inst) > 0);
        return map_.at(inst);
    }

    /*
     * The immediate dominator of 'inst'
     */
    void SetIdom(Inst *inst, Inst *idom)
    {
        idoms_[GetInstId(inst)] = idom;
    }
    Inst *GetIdom(Inst *inst) const
    {
        return idoms_[GetInstId(inst)];
    }

    /*
     * The ancestor of 'inst', fake_root_ for non-phi
     */
    void SetAncestor(Inst *inst, Inst *anc)
    {
        ancestors_[GetInstId(inst)] = anc;
    }
    Inst *GetAncestor(Inst *inst) const
    {
        return ancestors_[GetInstId(inst)];
    }

    /*
     * A set of instructions whose semidominator is 'inst'
     */
    InstVector &GetBucket(Inst *inst)
    {
        return buckets_[GetInstId(inst)];
    }

    /*
     * The inst in the ancestors chain with the minimal semidominator DFS-number for 'inst'
     */
    void SetLabel(Inst *inst, Inst *label)
    {
        labels_[GetInstId(inst)] = label;
    }
    Inst *GetLabel(Inst *inst) const
    {
        return labels_[GetInstId(inst)];
    }

    /*
     * The parent of 'inst' in the spanning tree generated by DFS
     */
    void SetParent(Inst *inst, Inst *parent)
    {
        parents_[GetInstId(inst)] = parent;
    }
    Inst *GetParent(Inst *inst) const
    {
        return parents_[GetInstId(inst)];
    }

    /*
     * The DFS-number of the semidominator of 'inst'
     */
    void SetSemi(Inst *inst, int32_t value)
    {
        semi_[GetInstId(inst)] = value;
    }
    int32_t GetSemi(Inst *inst) const
    {
        return semi_[GetInstId(inst)];
    }

    /*
     * The inst whose DFS-number is 'index'
     */
    void SetVertex(size_t index, Inst *inst)
    {
        vertices_[index] = inst;
    }
    Inst *GetVertex(size_t index) const
    {
        return vertices_[index];
    }

    void AdjustImmediateDominators(Inst *inst);
    void ComputeImmediateDominators(Inst *inst);
    void Compress(Inst *inst);
    void DfsNumbering(Inst *inst);
    Inst *Eval(Inst *inst);
    void Init(size_t count);
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_CLEANUP_H
