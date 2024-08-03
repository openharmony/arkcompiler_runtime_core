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
#include "optimizer/optimizations/cleanup.h"
#include "reg_alloc_graph_coloring.h"
#include "reg_alloc_resolver.h"

namespace panda::compiler {
bool RegAlloc(Graph *graph)
{
    graph->RunPass<Cleanup>();

    RegAllocResolver(graph).ResolveCatchPhis();
    return graph->RunPass<RegAllocGraphColoring>(VIRTUAL_FRAME_SIZE);
}
}  // namespace panda::compiler
