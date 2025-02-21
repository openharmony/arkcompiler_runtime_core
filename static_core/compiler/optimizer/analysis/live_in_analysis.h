/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef COMPILER_ANALYSIS_LIVE_IN_H
#define COMPILER_ANALYSIS_LIVE_IN_H

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "typed_ref_set.h"

namespace ark::compiler {

class LiveInAnalysis {
public:
    explicit LiveInAnalysis(Graph *graph)
        : graph_(graph),
          liveIn_(graph_->GetLocalAllocator()->Adapter()),
          allInsts_(graph_->GetLocalAllocator()->Adapter())
    {
    }
    ~LiveInAnalysis() = default;
    NO_COPY_SEMANTIC(LiveInAnalysis);
    NO_MOVE_SEMANTIC(LiveInAnalysis);

    static bool HasAllocs(Graph *graph);

    // Compute set of ref-typed instructions live at the beginning of each
    // basic block. If `checkAllocs` is true, return false if graph don't contain NewObject instructions.
    bool Run(bool checkAllocs);

    template <typename Callback>
    void VisitAlive(const BasicBlock *block, Callback &&cb)
    {
        auto &liveIns = liveIn_[block->GetId()];
        VisitBitVector(liveIns, [this, &cb](size_t liveIdx) {
            ASSERT(liveIdx < allInsts_.size());
            ASSERT(allInsts_[liveIdx] != nullptr);
            cb(allInsts_[liveIdx]);
        });
    }

    bool IsAlive(const BasicBlock *block, const Inst *inst) const;

    static bool IsAllocInst(const Inst *inst)
    {
        return inst->GetOpcode() == Opcode::NewObject || inst->GetOpcode() == Opcode::NewArray;
    }

private:
    Graph *graph_;
    // Live ins evaluation requires union of successor's live ins,
    // bit vector's union works faster than set's union.
    ArenaVector<ArenaBitVector> liveIn_;
    ArenaVector<Inst *> allInsts_;

    void AddInst(Inst *inst)
    {
        if (inst->GetId() >= allInsts_.size()) {
            allInsts_.resize(inst->GetId() + 1);
        }
        allInsts_[inst->GetId()] = inst;
    }

    void ProcessBlock(BasicBlock *block);
};

}  // namespace ark::compiler

#endif  // COMPILER_ANALYSIS_LIVE_IN_H
