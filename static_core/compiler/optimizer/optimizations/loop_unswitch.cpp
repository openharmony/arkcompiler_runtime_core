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
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/loop_unswitcher.h"
#include "loop_unswitch.h"

namespace panda::compiler {
bool LoopUnswitch::RunImpl()
{
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Run " << GetPassName();
    RunLoopsVisitor();
    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << GetPassName() << " complete";
    return is_applied_;
}

void LoopUnswitch::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool LoopUnswitch::TransformLoop(Loop *loop)
{
    loops_.push(loop);

    int32_t budget = max_insns_;
    for (uint32_t level = 0; !loops_.empty() && level < max_level_ && budget > 0; ++level) {
        auto level_size = loops_.size();
        while (level_size-- != 0) {
            auto orig_loop = loops_.front();
            loops_.pop();

            if (LoopUnswitcher::IsSmallLoop(orig_loop)) {
                COMPILER_LOG(DEBUG, LOOP_UNSWITCH)
                    << "Level #" << level << ": estimated loop iterations < 2, skip loop " << loop->GetId();
                continue;
            }
            auto unswitch_inst = LoopUnswitcher::FindUnswitchInst(orig_loop);
            if (unswitch_inst == nullptr) {
                COMPILER_LOG(DEBUG, LOOP_UNSWITCH)
                    << "Level #" << level << ": cannot find unswitch instruction, skip loop " << loop->GetId();
                continue;
            }

            uint32_t loop_size = 0;
            uint32_t true_count = 0;
            uint32_t false_count = 0;
            LoopUnswitcher::EstimateInstructionsCount(loop, unswitch_inst, &loop_size, &true_count, &false_count);
            if (true_count + false_count >= budget + loop_size) {
                break;
            }

            auto loop_unswitcher =
                LoopUnswitcher(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator());
            auto new_loop = loop_unswitcher.UnswitchLoop(orig_loop, unswitch_inst);
            if (new_loop == nullptr) {
                continue;
            }

            if (true_count + false_count > loop_size) {
                budget -= true_count + false_count - loop_size;
            }

            COMPILER_LOG(DEBUG, LOOP_UNSWITCH)
                << "Level #" << level << ": unswitch loop " << orig_loop->GetId() << ", new loop " << new_loop->GetId();

            loops_.push(orig_loop);
            loops_.push(new_loop);

            is_applied_ = true;
        }
    }
    return true;
}
}  // namespace panda::compiler
