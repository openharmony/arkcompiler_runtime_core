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

#include "reg_alloc.h"
#include "cleanup_empty_blocks.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/optimizations/cleanup.h"
#include "reg_alloc_graph_coloring.h"
#include "reg_alloc_linear_scan.h"
#include "reg_alloc_resolver.h"

namespace ark::compiler {

bool IsGraphColoringEnable(Graph *graph)
{
    if (graph->GetArch() == Arch::AARCH32 || !graph->IsAotMode() || !g_options.IsCompilerAotRa()) {
        return false;
    }

    size_t instCount = 0;
    for (auto bb : graph->GetBlocksRPO()) {
        instCount += bb->CountInsts();
    }
    return instCount < g_options.GetCompilerInstGraphColoringLimit();
}

bool ShouldSkipAllocation(Graph *graph)
{
#ifndef PANDA_TARGET_WINDOWS
    // Parameters spill-fills are empty
    return graph->GetCallingConvention() == nullptr && !graph->IsBytecodeOptimizer();
#else
    return !graph->IsBytecodeOptimizer();
#endif
}

bool RegAlloc(Graph *graph)
{
    if (ShouldSkipAllocation(graph)) {
        return false;
    }

    bool raPassed = false;

    if (graph->IsBytecodeOptimizer()) {
        RegAllocResolver(graph).ResolveCatchPhis();
        raPassed = graph->RunPass<RegAllocGraphColoring>(VIRTUAL_FRAME_SIZE);
    } else {
        if (IsGraphColoringEnable(graph)) {
            raPassed = graph->RunPass<RegAllocGraphColoring>();
            if (!raPassed) {
                LOG(WARNING, COMPILER) << "RA graph coloring algorithm failed. Linear scan will be used.";
                graph->InvalidateAnalysis<LivenessAnalyzer>();
            }
        }
        if (!raPassed) {
            raPassed = graph->RunPass<RegAllocLinearScan>();
        }
    }
    if (raPassed) {
        CleanupEmptyBlocks(graph);
    }
    return raPassed;
}
}  // namespace ark::compiler
