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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "compiler_logger.h"
#include "monitor_analysis.h"

namespace panda::compiler {
void MonitorAnalysis::MarkedMonitorRec(BasicBlock *bb, int32_t num_monitors)
{
    ASSERT(num_monitors >= 0);
    if (num_monitors > 0) {
        bb->SetMonitorBlock(true);
        if (bb->IsEndBlock()) {
            COMPILER_LOG(DEBUG, MONITOR_ANALYSIS) << "There is MonitorEntry without MonitorExit";
            incorrect_monitors_ = true;
            return;
        }
    }
    for (auto inst : bb->Insts()) {
        if (inst->GetOpcode() == Opcode::Throw) {
            // The Monitor.Exit is removed from the compiled code after explicit Throw instruction
            // in the syncronized block because the execution is switching to the interpreting mode
            num_monitors = 0;
        }
        if (inst->GetOpcode() == Opcode::Monitor) {
            bb->SetMonitorBlock(true);
            if (inst->CastToMonitor()->IsEntry()) {
                bb->SetMonitorEntryBlock(true);
                ++num_monitors;
            } else {
                ASSERT(inst->CastToMonitor()->IsExit());
                if (num_monitors <= 0) {
                    COMPILER_LOG(DEBUG, MONITOR_ANALYSIS) << "There is MonitorExit without MonitorEntry";
                    incorrect_monitors_ = true;
                    return;
                }
                bb->SetMonitorExitBlock(true);
                --num_monitors;
            }
        }
    }
    enteredMonitorsCount_->at(bb->GetId()) = num_monitors;
    if (num_monitors == 0) {
        return;
    }
    for (auto succ : bb->GetSuccsBlocks()) {
        if (succ->SetMarker(marker_)) {
            continue;
        }
        MarkedMonitorRec(succ, num_monitors);
    }
}

bool MonitorAnalysis::RunImpl()
{
    auto allocator = GetGraph()->GetLocalAllocator();
    incorrect_monitors_ = false;
    enteredMonitorsCount_ = allocator->New<ArenaVector<uint32_t>>(allocator->Adapter());
    enteredMonitorsCount_->resize(GetGraph()->GetVectorBlocks().size());
    marker_ = GetGraph()->NewMarker();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->SetMarker(marker_)) {
            continue;
        }
        MarkedMonitorRec(bb, 0);
        if (incorrect_monitors_) {
            return false;
        }
    }
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        const uint32_t UNINITIALIZED = 0xFFFFFFFF;
        uint32_t count = UNINITIALIZED;
        for (auto prev : bb->GetPredsBlocks()) {
            if (count == UNINITIALIZED) {
                count = enteredMonitorsCount_->at(prev->GetId());
            } else if (count != enteredMonitorsCount_->at(prev->GetId())) {
                COMPILER_LOG(DEBUG, MONITOR_ANALYSIS)
                    << "There is an inconsistent MonitorEntry counters in parent basic blocks";
                return false;
            }
        }
    }
    GetGraph()->EraseMarker(marker_);
    return true;
}
}  // namespace panda::compiler
