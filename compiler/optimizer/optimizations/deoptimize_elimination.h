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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_DEOPTIMIZEELIMINATION_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_DEOPTIMIZEELIMINATION_H_

#include "compiler_logger.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/graph_visitor.h"

namespace panda::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class DeoptimizeElimination : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit DeoptimizeElimination(Graph *graph)
        : Optimization(graph),
          blocks_type_(graph->GetLocalAllocator()->Adapter()),
          deoptimize_must_throw_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(DeoptimizeElimination);
    NO_COPY_SEMANTIC(DeoptimizeElimination);
    ~DeoptimizeElimination() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "DeoptimizeElimination";
    }

    void InvalidateAnalyses() override;

    bool IsEnable() const override
    {
        return options.IsCompilerDeoptimizeElimination();
    }

    void ReplaceDeoptimizeIfByUnconditionalDeoptimize();

    void RemoveSafePoints();

    /*
     * By default all blocks have INVALID_TYPE.
     * GUARD - If block have IsMustDeootimize before runtime call inst(in reverse order)
     * RUNTIME_CALL - If block have runtime call inst before IsMustDeoptimize(in reverse order)
     * NOTHING - If block is preccessed, but it doesn't contain GUARD and RUNTIME_CALL
     */
    enum BlockType { INVALID, GUARD, RUNTIME_CALL, NOTHING };

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    void SetApplied()
    {
        is_applied_ = true;
    }

    bool IsApplied() const
    {
        return is_applied_;
    }

    bool HaveCalls()
    {
        return have_calls_;
    }

#include <deoptimize_elimination_call_visitors.inl>
    static void VisitSaveState(GraphVisitor *v, Inst *inst);
    static void VisitSaveStateDeoptimize(GraphVisitor *v, Inst *inst);
    static void VisitDeoptimizeIf(GraphVisitor *v, Inst *inst);
    void VisitDefault(Inst *inst) override;

#include "optimizer/ir/visitor.inc"

private:
    void SetHaveCalls()
    {
        have_calls_ = true;
    }

    void PushNewDeoptimizeIf(Inst *inst)
    {
        deoptimize_must_throw_.push_back(inst);
    }

    void PushNewBlockType(BasicBlock *block, BlockType type)
    {
        ASSERT(blocks_type_.find(block) == blocks_type_.end());
        blocks_type_.emplace(block, type);
    }

    BlockType GetBlockType(BasicBlock *block)
    {
        if (blocks_type_.find(block) != blocks_type_.end()) {
            return blocks_type_.at(block);
        }
        return BlockType::INVALID;
    }

    bool TryToRemoveRedundantSaveState(Inst *inst);
    bool CanRemoveGuard(Inst *guard);
    bool CanRemoveGuardRec(BasicBlock *block, Inst *guard, const Marker &mrk, const Marker &remove_mrk);
    void RemoveGuard(Inst *guard);
    void RemoveDeoptimizeIf(Inst *inst);
    bool RequireRegMap(Inst *inst);

private:
    bool have_calls_ {false};
    bool is_applied_ {false};
    ArenaUnorderedMap<BasicBlock *, BlockType> blocks_type_;
    InstVector deoptimize_must_throw_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_DEOPTIMIZEELIMINATION_H_
