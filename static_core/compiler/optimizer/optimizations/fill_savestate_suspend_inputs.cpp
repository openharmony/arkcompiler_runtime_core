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

#include "fill_savestate_suspend_inputs.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/analysis.h"
#include "compiler_logger.h"
#include <algorithm>
#include <cstdint>

namespace ark::compiler {

bool FillSaveStateSuspendInputs::RunImpl()
{
    auto *graph = GetGraph();

    COMPILER_LOG(DEBUG, FILL_SS_SUSPEND) << "Running FillSaveStateSuspendInputs pass";

    if (!graph->RunPass<LivenessAnalyzer>()) {
        COMPILER_LOG(ERROR, FILL_SS_SUSPEND) << "LivenessAnalyzer failed";
        return false;
    }

    bool encountered = false;
    bool modified = false;
    for (auto *block : graph->GetBlocksRPO()) {
        for (auto *inst : block->Insts()) {
            if (!inst->IsSaveStateSuspend()) {
                continue;
            }

            encountered = true;
            modified |= ProcessSaveStateSuspend(static_cast<SaveStateInst *>(inst));
        }
    }

    if (!encountered) {
        COMPILER_LOG(DEBUG, FILL_SS_SUSPEND) << "No SaveStateSuspend instructions found, skipping pass";
        return false;
    }

#ifdef COMPILER_DEBUG_CHECKS
    graph->SetSaveStateSuspendInputsAllocated();
#endif  // COMPILER_DEBUG_CHECKS

    graph->SetMaxPrimCountAtSuspend(maxPrimCount_);
    graph->SetMaxRefCountAtSuspend(maxRefCount_);

    COMPILER_LOG(DEBUG, FILL_SS_SUSPEND) << "FillSaveStateSuspendInputs pass completed, modified=" << modified;

    return modified;
}

void FillSaveStateSuspendInputs::UpdateCounters(SaveStateInst *saveStateSuspend)
{
    uint32_t refCount = 0;
    uint32_t primCount = 0;
    for (auto input : saveStateSuspend->GetInputs()) {
        if (IsReference(input.GetInst()->GetType())) {
            refCount++;
        } else {
            primCount++;
        }
    }
    maxPrimCount_ = std::max(maxPrimCount_, primCount + static_cast<uint32_t>(saveStateSuspend->GetImmediatesCount()));
    maxRefCount_ = std::max(maxRefCount_, refCount);
}

bool FillSaveStateSuspendInputs::ProcessSaveStateSuspend(SaveStateInst *saveStateSuspend)
{
    ASSERT(saveStateSuspend->IsSaveStateSuspend());

    COMPILER_LOG(DEBUG, FILL_SS_SUSPEND) << "Processing SaveStateSuspend v" << saveStateSuspend->GetId();

    ASSERT(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
    const auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();

    MarkerHolder marker(GetGraph());
    for (size_t i = 0; i < saveStateSuspend->GetInputsCount(); ++i) {
        saveStateSuspend->GetInput(i).GetInst()->SetMarker(marker.GetMarker());
    }
    saveStateSuspend->SetMarker(marker.GetMarker());

    la.GetLiveValuesAtPoint(saveStateSuspend, liveValues_);

    COMPILER_LOG(DEBUG, FILL_SS_SUSPEND) << "Found " << liveValues_.size() << " live values at SaveStateSuspend v"
                                         << saveStateSuspend->GetId();

    size_t addedCount = 0;
    for (auto *liveValue : liveValues_) {
        if (liveValue->IsMarked(marker.GetMarker())) {
            continue;
        }
        ASSERT(!liveValue->IsCatchPhi());
        saveStateSuspend->AppendBridge(liveValue);
        liveValue->SetMarker(marker.GetMarker());
        addedCount++;
        COMPILER_LOG(DEBUG, FILL_SS_SUSPEND) << "Added live value v" << liveValue->GetId()
                                             << " as bridge input to SaveStateSuspend v" << saveStateSuspend->GetId();
    }

    OptimizeSaveStateConstantInputs(saveStateSuspend);
    UpdateCounters(saveStateSuspend);

    COMPILER_LOG(DEBUG, FILL_SS_SUSPEND) << "SaveStateSuspend v" << saveStateSuspend->GetId() << ": added "
                                         << addedCount
                                         << " bridge inputs, total inputs now: " << saveStateSuspend->GetInputsCount()
                                         << ", immediates: " << saveStateSuspend->GetImmediatesCount();

    return addedCount > 0;
}

void FillSaveStateSuspendInputs::InvalidateAnalyses()
{
    if (GetGraph()->GetAnalysis<LivenessAnalyzer>().IsTargetSpecificComputed()) {
        GetGraph()->InvalidateAnalysis<LivenessAnalyzer>();
    }
}

}  // namespace ark::compiler
