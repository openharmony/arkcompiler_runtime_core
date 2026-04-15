/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_FILL_SAVESTATE_SUSPEND_INPUTS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_FILL_SAVESTATE_SUSPEND_INPUTS_H

#include "optimizer/pass.h"
#include "optimizer/ir/graph.h"

namespace ark::compiler {

class LivenessAnalyzer;

/**
 * FillSaveStateSuspendInputs pass — automatically fills inputs for SaveStateSuspend instructions.
 *
 * This pass MUST run BEFORE Register Allocation because it uses LivenessAnalyzer to determine
 * which values are live at each SaveStateSuspend point, and modifies the instruction inputs.
 * The pass runs LivenessAnalyzer lazily when the first SaveStateSuspend instruction is found.
 *
 * Algorithm:
 * 1. Traverse the graph until the first SaveStateSuspend instruction is found
 * 2. Run LivenessAnalyzer once before processing that instruction
 * 3. Process each SaveStateSuspend instruction:
 *    a. Mark existing inputs with a Marker for O(1) duplicate detection
 *    b. Get all live values at that point using LivenessAnalyzer::GetLiveValuesAtPoint
 *    c. Add missing live values as bridge inputs (skip self, already-marked, CatchPhi).
 *       "Populating inputs" here means appending these bridge inputs; VReg inputs are set by the IR builder.
 *    d. Optimize constant bridge inputs to immediates via OptimizeSaveStateConstantInputs
 * 4. Set Graph flag FlagSaveStateSuspendInputsAllocated
 *
 * Invariants:
 * - The pass may only APPEND inputs, never remove them
 * - After the flag is set, no subsequent pass should remove inputs from SaveStateSuspend
 *
 * GraphChecker verifies (gated by the Graph flag):
 * - All instructions that dominate SaveStateSuspend and whose users are dominated by
 *   SaveStateSuspend must be present in SaveStateSuspend inputs (dominance-based check)
 */
class PANDA_PUBLIC_API FillSaveStateSuspendInputs : public Optimization {
public:
    explicit FillSaveStateSuspendInputs(Graph *graph)
        : Optimization(graph), liveValues_(graph->GetLocalAllocator()->Adapter())
    {
    }
    ~FillSaveStateSuspendInputs() override = default;

    NO_MOVE_SEMANTIC(FillSaveStateSuspendInputs);
    NO_COPY_SEMANTIC(FillSaveStateSuspendInputs);

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "FillSaveStateSuspendInputs";
    }

    bool IsEnable() const override
    {
        return true;
    }

    void InvalidateAnalyses() override;

private:
    void UpdateCounters(SaveStateInst *saveStateSuspend);
    bool ProcessSaveStateSuspend(SaveStateInst *saveStateSuspend);

    ArenaVector<Inst *> liveValues_;
    uint32_t maxRefCount_ {0};
    uint32_t maxPrimCount_ {0};
    uint32_t maxSuspendBridges_ {0};
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_FILL_SAVESTATE_SUSPEND_INPUTS_H
