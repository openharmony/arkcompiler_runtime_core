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

#include "reg_alloc.h"
#include "cleanup_empty_blocks.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/optimizations/cleanup.h"
#include "reg_alloc_graph_coloring.h"
#include "reg_alloc_linear_scan.h"
#include "reg_alloc_resolver.h"

namespace panda::compiler {
static constexpr size_t INST_LIMIT_FOR_GRAPH_COLORING = 5000;

bool IsGraphColoringEnable(const Graph *graph)
{
    if (graph->GetArch() == Arch::AARCH32 || !graph->IsAotMode() || !OPTIONS.IsCompilerAotRa()) {
        return false;
    }

    size_t inst_count = 0;
    for (auto bb : graph->GetBlocksRPO()) {
        inst_count += bb->CountInsts();
    }
    return inst_count < INST_LIMIT_FOR_GRAPH_COLORING;
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

    bool ra_passed = false;

    if (graph->IsBytecodeOptimizer()) {
        RegAllocResolver(graph).ResolveCatchPhis();
        ra_passed = graph->RunPass<RegAllocGraphColoring>(VIRTUAL_FRAME_SIZE);
    } else if (IsGraphColoringEnable(graph)) {
        ra_passed = graph->RunPass<RegAllocGraphColoring>();
    } else {
        ra_passed = graph->RunPass<RegAllocLinearScan>();
    }
    if (ra_passed) {
        CleanupEmptyBlocks(graph);
    }
    return ra_passed;
}
}  // namespace panda::compiler
