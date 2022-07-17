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

#include "unit_test.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/loop_unroll.h"
#include "optimizer/optimizations/loop_peeling.h"

namespace panda::compiler {
class GraphCreationTest : public CommonTest {
};

TEST_F(GraphCreationTest, CreateEmptyGraph)
{
    Graph *graph = CreateEmptyGraph();
    EXPECT_NE(graph, nullptr);
}

TEST_F(GraphCreationTest, CreateGraphStartEndBlocks)
{
    Graph *graph = CreateGraphStartEndBlocks();
    EXPECT_NE(graph, nullptr);
    EXPECT_NE(graph->GetStartBlock(), nullptr);
    EXPECT_NE(graph->GetEndBlock(), nullptr);
    EXPECT_EQ(graph->GetAliveBlocksCount(), 2U);
}

TEST_F(GraphCreationTest, OsrModeGraph)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs(1, 5);
            INST(4, Opcode::Phi).u64().Inputs(2, 10);
            INST(5, Opcode::Sub).u64().Inputs(3, 2);
            INST(6, Opcode::SafePoint).Inputs(0, 3, 4).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(5, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(9, Opcode::And).u64().Inputs(4, 5);
            INST(10, Opcode::Add).u64().Inputs(9, 4);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(4);
        }
    }
    BB(2).SetOsrEntry(true);
    auto clone_graph = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();

    graph->RunPass<LoopPeeling>();
    graph->RunPass<LoopUnroll>(1000, 4);
    EXPECT_TRUE(GraphComparator().Compare(graph, clone_graph));
}
}  // namespace panda::compiler
