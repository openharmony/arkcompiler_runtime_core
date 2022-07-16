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
#include "optimizer/optimizations/loop_peeling.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
class LoopPeelingTest : public GraphTest {
protected:
    void BuildGraphTwoBackEdges(Graph *graph)
    {
        GRAPH(graph)
        {
            PARAMETER(0, 0).b();
            PARAMETER(1, 1).u64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::Phi).Inputs(1, 4, 6).u64();
                INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
            }
            BASIC_BLOCK(3, 2, 5)
            {
                INST(4, Opcode::Add).Inputs(1, 1).u64();
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
            }
            BASIC_BLOCK(5, 2)
            {
                INST(6, Opcode::Add).Inputs(4, 4).u64();
            }
            BASIC_BLOCK(4, -1)
            {
                INST(7, Opcode::Return).u64().Inputs(2);
            }
        }
    }

    void BuildGraphNotHeaderExit(Graph *graph)
    {
        GRAPH(graph)
        {
            PARAMETER(1, 1).u64();
            BASIC_BLOCK(2, 6, 7)
            {
                INST(2, Opcode::Phi).Inputs(1, 6).u64();
                INST(20, Opcode::SaveState).NoVregs();
                INST(8, Opcode::CallStatic).v0id().InputsAutoType(20);
                INST(0, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0).Inputs(1);
            }
            BASIC_BLOCK(6, 3)
            {
                INST(3, Opcode::Add).Inputs(1, 2).u64();
            }
            BASIC_BLOCK(7, 3)
            {
                INST(9, Opcode::Add).Inputs(2, 1).u64();
            }
            BASIC_BLOCK(3, 4, 5)
            {
                INST(10, Opcode::Phi).Inputs(3, 9).u64();
                INST(4, Opcode::Add).Inputs(1, 10).u64();
                INST(5, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0).Inputs(2);
            }
            BASIC_BLOCK(4, 2)
            {
                INST(6, Opcode::Add).Inputs(4, 4).u64();
            }
            BASIC_BLOCK(5, -1)
            {
                INST(7, Opcode::Return).u64().Inputs(4);
            }
        }
    }

    void BuildGraphHeaderAndBackEdgeExit(Graph *graph)
    {
        GRAPH(graph)
        {
            CONSTANT(0, 10);
            CONSTANT(1, 0);
            CONSTANT(2, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(3, Opcode::Phi).u64().Inputs(1, 9);
                INST(4, Opcode::Phi).u64().Inputs(1, 10);
                INST(6, Opcode::SafePoint).Inputs(0, 3, 4, 4).SrcVregs({0, 1, 2, 3});
                INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(4, 0);
                INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
            }
            BASIC_BLOCK(3, 2, 4)
            {
                INST(9, Opcode::And).u64().Inputs(4, 3);
                INST(10, Opcode::Add).u64().Inputs(4, 2);
                INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(12, Opcode::Phi).u64().Inputs(4, 10);
                INST(13, Opcode::Return).u64().Inputs(12);
            }
        }
    }

    void BuildGraphMultiExit(Graph *graph)
    {
        GRAPH(graph)
        {
            PARAMETER(0, 0).b();
            PARAMETER(1, 1).u64();
            BASIC_BLOCK(2, 3, 5)
            {
                INST(2, Opcode::Phi).Inputs(1, 6).u64();
                INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
            }
            BASIC_BLOCK(3, 4, 5)
            {
                INST(4, Opcode::Add).Inputs(1, 1).u64();
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
            }
            BASIC_BLOCK(4, 2)
            {
                INST(6, Opcode::Add).Inputs(4, 4).u64();
            }
            BASIC_BLOCK(5, -1)
            {
                INST(7, Opcode::Phi).Inputs(2, 4).u64();
                INST(8, Opcode::Return).u64().Inputs(7);
            }
        }
    }
};

/*
 *              [0]
 *               |
 *               v
 *        /---->[2]-----\
 *        |      |      |
 *        |      v      v
 *        \-----[3]    [4]
 *                      |
 *                    [exit]
 */
TEST_F(LoopPeelingTest, CloneBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs(1, 9).u64();
            INST(4, Opcode::Phi).u64().Inputs(2, 10).u64();
            INST(6, Opcode::SafePoint).Inputs(0, 3, 4).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(0, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(9, Opcode::And).u64().Inputs(4, 3);
            INST(10, Opcode::Add).u64().Inputs(4, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(13, Opcode::Div).Inputs(3, 4).u64();
            INST(14, Opcode::Return).u64().Inputs(13);
        }
    }
    GetGraph()->RunPass<LoopAnalyzer>();
    auto graph_cloner = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator());
    graph_cloner.CloneLoopHeader(&BB(2), &BB(4), BB(2).GetLoop()->GetPreHeader());
    GetGraph()->RunPass<Cleanup>();

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(8, 2, 4)
        {
            INST(15, Opcode::Compare).CC(CC_EQ).b().Inputs(0, 1);
            INST(16, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
        }
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{8, 1}, {3, 9}}).u64();
            INST(4, Opcode::Phi).u64().Inputs({{8, 2}, {3, 10}}).u64();
            INST(6, Opcode::SafePoint).Inputs(0, 3, 4).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(0, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(9, Opcode::And).u64().Inputs(4, 3);
            INST(10, Opcode::Add).u64().Inputs(4, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs({{2, 3}, {8, 1}}).u64();
            INST(18, Opcode::Phi).u64().Inputs({{2, 4}, {8, 2}}).u64();
            INST(13, Opcode::Div).Inputs(17, 18).u64();
            INST(14, Opcode::Return).u64().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

/*
 *              [0]
 *               |
 *               v
 *        /---->[2]-----\
 *        |      |      |
 *        |      v      v
 *        \-----[3]    [4]
 *                      |
 *                    [exit]
 *
 * Transform to:
 *
 *              [0]
 *               |
 *               v
 *           [pre-loop]---------\
 *               |              |
 *        /---->[2]             |
 *        |      |              |
 *        |      v              |
 *        |     [3]             |
 *        |      |              |
 *        |      v              v
 *        \--[loop-exit]--->[loop-outer]
 *                              |
 *                              v
 *                             [4]
 *                              |
 *                              v
 *                            [exit]
 */
TEST_F(LoopPeelingTest, SingleLoop)
{
    GRAPH(GetGraph())
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

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(5, 2, 4)
        {
            INST(12, Opcode::Sub).u64().Inputs(1, 2);
            INST(13, Opcode::Compare).CC(CC_EQ).b().Inputs(12, 0);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(2, 2, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{5, 12}, {2, 5}});
            INST(4, Opcode::Phi).u64().Inputs({{5, 2}, {2, 10}});
            INST(9, Opcode::And).u64().Inputs(4, 3);
            INST(10, Opcode::Add).u64().Inputs(9, 4);
            INST(5, Opcode::Sub).u64().Inputs(3, 2);
            INST(6, Opcode::SafePoint).Inputs(0, 3, 10).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(5, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::Phi).u64().Inputs({{5, 2}, {2, 10}});
            INST(11, Opcode::Return).u64().Inputs(16);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    // TODO(a.popov): remove these calls, see todo in LoopPeeling pass constructor
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

/*
 *              [0]
 *               |
 *               v
 *   /--------->[2]------------\
 *   |           |             |
 *   |           v             |
 *   |    /---->[3]-----\     [6]
 *   |    |      |      |      |
 *   |    |      v      v      |
 *   |    \-----[4]    [5]   [exit]
 *   |                  |
 *   |                  |
 *   \------------------/
 *
 * Transform to:
 *
 *              [0]
 *               |
 *               v
 *   /--------->[2]-------------------------\
 *   |           |                          |
 *   |           v                         [6]
 *   |       [pre-loop]---------\           |
 *   |           |              |           v
 *   |           v              |         [exit]
 *   |    /---->[3]             |
 *   |    |      |              |
 *   |    |      v              |
 *   |    |     [4]             |
 *   |    |      |              |
 *   |    |      v              v
 *   |    \--[loop-exit]-->[loop-outer]
 *   |                          |
 *   |                          v
 *   \-------------------------[5]
 *
 */
TEST_F(LoopPeelingTest, InnerLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();  // count
        PARAMETER(1, 1).u64();  // i
        CONSTANT(2, 100);
        CONSTANT(3, 1);
        BASIC_BLOCK(2, 3, 6)
        {
            INST(4, Opcode::Phi).u64().Inputs(0, 8);              // count
            INST(5, Opcode::Phi).u64().Inputs(1, 14);             // i
            INST(6, Opcode::Compare).CC(CC_LT).b().Inputs(5, 2);  // while i < 100
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(8, Opcode::Phi).u64().Inputs(4, 12);              // count
            INST(9, Opcode::Phi).u64().Inputs(5, 13);              // j = i
            INST(10, Opcode::Compare).CC(CC_LT).b().Inputs(9, 2);  // while j < 100
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(12, Opcode::Add).u64().Inputs(8, 3);  // j++
            INST(13, Opcode::Add).u64().Inputs(9, 3);  // count++
        }
        BASIC_BLOCK(5, 2)
        {
            INST(14, Opcode::Add).u64().Inputs(5, 3);  // i++
        }
        BASIC_BLOCK(6, -1)
        {
            INST(15, Opcode::Return).u64().Inputs(4);  // return count
        }
    }

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 100);
        CONSTANT(3, 1);
        BASIC_BLOCK(2, 7, 6)
        {
            INST(4, Opcode::Phi).u64().Inputs(0, 18);
            INST(5, Opcode::Phi).u64().Inputs(1, 14);
            INST(6, Opcode::Compare).CC(CC_LT).b().Inputs(5, 2);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(7, 3, 5)
        {
            INST(16, Opcode::Compare).CC(CC_LT).b().Inputs(5, 2);
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(16);
        }
        BASIC_BLOCK(3, 3, 5)
        {
            INST(8, Opcode::Phi).u64().Inputs(4, 12);
            INST(9, Opcode::Phi).u64().Inputs(5, 13);
            INST(12, Opcode::Add).u64().Inputs(8, 3);
            INST(13, Opcode::Add).u64().Inputs(9, 3);
            INST(10, Opcode::Compare).CC(CC_LT).b().Inputs(13, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }

        BASIC_BLOCK(5, 2)
        {
            INST(18, Opcode::Phi).u64().Inputs({{7, 4}, {3, 12}});
            INST(14, Opcode::Add).u64().Inputs(5, 3);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(15, Opcode::Return).u64().Inputs(4);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

/*
 *              [0]
 *               |
 *               v
 *      /------>[2]------\
 *      |        |       |
 *      |       [3]      |
 *      |       / \      |
 *      |      v   v     v
 *      |     [4] [5]   [7]
 *      |       \ /      |
 *      \-------[6]    [exit]
 *
 *  Transform to:
 *
 *              [0]
 *               |
 *               v
 *           [pre-loop]---------\
 *               |              |
 *               v              |
 *      /------>[2]             |
 *      |        |              |
 *      |       [3]             |
 *      |       / \             |
 *      |      v   v            |
 *      |     [4] [5]           |
 *      |       \ /             |
 *      |       [6]             |
 *      |        |              v
 *      \---[loop-exit]-->[loop-outer]
 *                             |
 *                            [7]
 *                             |
 *                           [exit]
 */
TEST_F(LoopPeelingTest, LoopWithBranch)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 7)
        {
            INST(3, Opcode::Phi).Inputs(1, 8).u64();
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(6, Opcode::Add).Inputs(3, 2).u64();
        }
        BASIC_BLOCK(5, 6)
        {
            INST(7, Opcode::Mul).Inputs(3, 2).u64();
        }
        BASIC_BLOCK(6, 2)
        {
            INST(8, Opcode::Phi).Inputs(6, 7).u64();
            INST(11, Opcode::SaveState).NoVregs();
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(11);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(10, Opcode::Return).Inputs(3).u64();
        }
    }

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 2);
        BASIC_BLOCK(8, 3, 7)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).Inputs(1, 8).u64();
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(6, Opcode::Add).Inputs(3, 2).u64();
        }
        BASIC_BLOCK(5, 6)
        {
            INST(7, Opcode::Mul).Inputs(3, 2).u64();
        }
        BASIC_BLOCK(6, 3, 7)
        {
            INST(8, Opcode::Phi).Inputs(6, 7).u64();
            INST(13, Opcode::SaveState).NoVregs();
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(13);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(12, Opcode::Phi).Inputs({{8, 1}, {6, 8}}).u64();
            INST(10, Opcode::Return).Inputs(12).u64();
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

/*
 *              [0]
 *               |
 *               v
 *      --/---->[2]-----\
 *      | |      |      |
 *      | |      v      v
 *      | \-----[3]    [4]
 *      |        |      |
 *      \-------[5]   [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, TwoBackEdges)
{
    BuildGraphTwoBackEdges(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphTwoBackEdges(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *              [0]
 *               |
 *               v
 *      /------>[2]
 *      |        |
 *      |        v
 *      |       [3]--->[5]
 *      |        |      |
 *      \-------[4]   [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, NotHeaderExit)
{
    BuildGraphNotHeaderExit(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphNotHeaderExit(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *              [0]
 *               |
 *               v
 *        /---->[2]-----\
 *        |      |      |
 *        |      v      v
 *        \-----[3]--->[4]
 *                      |
 *                      v
 *                    [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, HeaderAndBackEdgeExit)
{
    BuildGraphHeaderAndBackEdgeExit(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphHeaderAndBackEdgeExit(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *              [0]
 *               |
 *               v
 *      /------>[2]----\
 *      |        |      |
 *      |        v      v
 *      |       [3]--->[5]
 *      |        |      |
 *      \-------[4]   [exit]
 *
 * NotApplied
 */
TEST_F(LoopPeelingTest, MultiExit)
{
    BuildGraphMultiExit(GetGraph());
    auto graph = CreateEmptyGraph();
    BuildGraphMultiExit(graph);

    EXPECT_FALSE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopPeelingTest, RemoveDeadPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs(0, 1);
            INST(4, Opcode::Phi).u64().Inputs(2, 10);
            INST(6, Opcode::SafePoint).Inputs(3, 4).SrcVregs({0, 1});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(4, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(10, Opcode::Add).u64().Inputs(4, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(4);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_FALSE(INS(3).HasUsers());
    EXPECT_EQ(INS(3).GetBasicBlock(), nullptr);
}

TEST_F(LoopPeelingTest, SingleBlockLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 2, 3)
        {
            INST(3, Opcode::Phi).u64().Inputs(0, 6);
            INST(4, Opcode::SafePoint).Inputs(3).SrcVregs({0});
            INST(5, Opcode::Add).u64().Inputs(1, 3);
            INST(6, Opcode::Add).u64().Inputs(0, 3);
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(6, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(9, Opcode::Return).u64().Inputs(5);
        }
    }

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(6, 2, 7)
        {
            INST(12, Opcode::Add).u64().Inputs(1, 0);
            INST(13, Opcode::Add).u64().Inputs(0, 0);
            INST(14, Opcode::Compare).CC(CC_EQ).b().Inputs(13, 2);
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }

        BASIC_BLOCK(2, 2, 7)
        {
            INST(3, Opcode::Phi).u64().Inputs(13, 6);
            INST(4, Opcode::SafePoint).Inputs(3).SrcVregs({0});
            INST(5, Opcode::Add).u64().Inputs(1, 3);
            INST(6, Opcode::Add).u64().Inputs(0, 3);
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(6, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs({{2, 5}, {6, 12}});
            INST(9, Opcode::Return).u64().Inputs(17);
        }
    }

    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

// TODO (apopov) Fix GraphComparator and enable test
TEST_F(LoopPeelingTest, DISABLED_RepeatedCloneableInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).u32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::LoadObject).u32().Inputs(0);
            INST(3, Opcode::Sub).u32().Inputs(2, 1);
            INST(4, Opcode::Compare).CC(CC_EQ).b().Inputs(3, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(6, Opcode::SaveState).Inputs(1, 2, 2, 3).SrcVregs({0, 1, 2, 3});
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::Return).u32().Inputs(2);
        }
    }

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).u32();
        BASIC_BLOCK(6, 2, 7)
        {
            INST(9, Opcode::LoadObject).u32().Inputs(0);
            INST(10, Opcode::Sub).u32().Inputs(9, 1);
            INST(11, Opcode::Compare).CC(CC_EQ).b().Inputs(10, 1);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(2, 2, 7)
        {
            INST(13, Opcode::Phi).u32().Inputs(9, 2);
            INST(15, Opcode::Phi).u32().Inputs(10, 3);
            INST(6, Opcode::SaveState).Inputs(1, 13, 13, 15).SrcVregs({0, 1, 2, 3});
            INST(2, Opcode::LoadObject).u32().Inputs(0);
            INST(3, Opcode::Sub).u32().Inputs(2, 1);
            INST(4, Opcode::Compare).CC(CC_EQ).b().Inputs(3, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(14, Opcode::Phi).u32().Inputs({{2, 2}, {6, 9}});
            INST(7, Opcode::Return).u32().Inputs(14);
        }
    }
    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

TEST_F(LoopPeelingTest, InfiniteLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 2)
        {
            INST(2, Opcode::Phi).s32().Inputs(0, 3);
            INST(3, Opcode::Add).s32().Inputs(2, 1);
        }
    }
    EXPECT_FALSE(graph->RunPass<LoopPeeling>());
}

TEST_F(LoopPeelingTest, MultiSafePointsLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs(1, 5);
            INST(4, Opcode::Phi).u64().Inputs(2, 10);
            INST(5, Opcode::Sub).u64().Inputs(3, 2);
            INST(6, Opcode::SafePoint).Inputs(0, 5, 4).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(5, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(9, Opcode::And).u64().Inputs(4, 5);
            INST(10, Opcode::Add).u64().Inputs(9, 4);
            INST(12, Opcode::SafePoint).Inputs(0, 5, 10).SrcVregs({0, 1, 2});
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(4);
        }
    }

    auto expected_graph = CreateEmptyGraph();
    GRAPH(expected_graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(5, 2, 4)
        {
            INST(13, Opcode::Sub).u64().Inputs(1, 2);
            INST(14, Opcode::Compare).CC(CC_EQ).b().Inputs(13, 0);
            INST(15, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(14);
        }
        BASIC_BLOCK(2, 2, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs(13, 5);
            INST(4, Opcode::Phi).u64().Inputs(2, 10);
            INST(9, Opcode::And).u64().Inputs(4, 3);
            INST(10, Opcode::Add).u64().Inputs(9, 4);
            INST(12, Opcode::SafePoint).Inputs(0, 3, 10).SrcVregs({0, 1, 2});
            INST(5, Opcode::Sub).u64().Inputs(3, 2);
            INST(6, Opcode::SafePoint).Inputs(0, 5, 10).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(5, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::Phi).u64().Inputs({{2, 10}, {5, 2}});
            INST(11, Opcode::Return).u64().Inputs(16);
        }
    }
    EXPECT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected_graph));
}

TEST_F(LoopPeelingTest, LoopWithInlinedCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::SaveState).Inputs().SrcVregs({});
            INST(4, Opcode::CallStatic).v0id().Inlined().InputsAutoType(3);
            INST(5, Opcode::IfImm).SrcType(DataType::UINT32).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(6, Opcode::ReturnInlined).Inputs(3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::SaveState).Inputs().SrcVregs({});
            INST(8, Opcode::Throw).Inputs(1, 7);
        }
    }
    INS(7).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(4)));
    ASSERT_FALSE(GetGraph()->RunPass<LoopPeeling>());
}

}  // namespace panda::compiler
