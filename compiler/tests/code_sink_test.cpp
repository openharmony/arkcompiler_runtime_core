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
#include "optimizer/optimizations/code_sink.h"

namespace panda::compiler {
class CodeSinkTest : public GraphTest {
};

TEST_F(CodeSinkTest, OperationPropagation)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Add).s64().Inputs(1, 2);
            INST(6, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Return).s64().Inputs(5);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(11, Opcode::Return).s64().Inputs(3);
        }
    }
    Graph *sunk_graph = CreateEmptyGraph();
    GRAPH(sunk_graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(5, Opcode::Add).s64().Inputs(1, 2);
            INST(9, Opcode::Return).s64().Inputs(5);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(11, Opcode::Return).s64().Inputs(3);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunk_graph));
}

/**
 * Move load but the NullCheck is still on its place:
 * exception should be thrown where it was initially.
 */
// TODO(Kudriashov Evgenii) enable the test after fixing CodeSink
TEST_F(CodeSinkTest, DISABLED_LoadWithOperationPropagation)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(3, 2).SrcVregs({0, 4});
            INST(5, Opcode::NullCheck).ref().Inputs(2, 4);
            INST(6, Opcode::LoadObject).s64().Inputs(5).TypeId(176);
            INST(7, Opcode::Add).s64().Inputs(6, 1);
            INST(8, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).s64().Inputs(7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Return).s64().Inputs(3);
        }
    }
    Graph *sunk_graph = CreateEmptyGraph();
    GRAPH(sunk_graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(3, 2).SrcVregs({0, 4});
            INST(5, Opcode::NullCheck).ref().Inputs(2, 4);
            INST(8, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::LoadObject).s64().Inputs(5).TypeId(176);
            INST(7, Opcode::Add).s64().Inputs(6, 1);
            INST(11, Opcode::Return).s64().Inputs(7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Return).s64().Inputs(3);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunk_graph));
}

/**
 * Do not move anything
 */
TEST_F(CodeSinkTest, NoDomination)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).s64();
            PARAMETER(1, 1).s64();
            PARAMETER(2, 2).s64();
            CONSTANT(3, 0x0).s64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(5, Opcode::Add).s64().Inputs(1, 2);
                INST(6, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
                INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(9, Opcode::Return).s64().Inputs(5);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(11, Opcode::Return).s64().Inputs(5);
            }
        }
    }
    equal_graphs[0]->RunPass<CodeSink>();
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

/**
 * Do not sink loads that may alias further stores in the block
 */
TEST_F(CodeSinkTest, LoadStoreAliasing)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).ref();
            PARAMETER(2, 2).s64();
            CONSTANT(3, 0x0).s64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(6, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
                INST(9, Opcode::StoreObject).s64().Inputs(1, 2).TypeId(243);
                INST(10, Opcode::Compare).b().CC(CC_NE).Inputs(2, 3);
                INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(13, Opcode::Return).s64().Inputs(6);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(15, Opcode::Return).s64().Inputs(3);
            }
        }
    }
    equal_graphs[0]->RunPass<CodeSink>();
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

TEST_F(CodeSinkTest, LoopSinking)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x1).s64();
        CONSTANT(4, 0x0).s64();
        CONSTANT(5, 0x42).s64();
        BASIC_BLOCK(2, 3)
        {
            // Do not sink into loop
            INST(6, Opcode::Add).s64().Inputs(1, 5);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(10, Opcode::Phi).s64().Inputs({{2, 4}, {3, 22}});
            INST(20, Opcode::LoadArray).s64().Inputs(2, 10);
            INST(21, Opcode::Add).s64().Inputs(20, 6);
            INST(22, Opcode::Add).s64().Inputs(21, 10);
            INST(23, Opcode::Add).s32().Inputs(10, 3);
            // Sink out of loop
            INST(26, Opcode::Add).s64().Inputs(21, 22);
            INST(24, Opcode::Compare).b().CC(CC_LT).Inputs(23, 0);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(24);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(27, Opcode::Return).s64().Inputs(26);
        }
    }
    Graph *sunk_graph = CreateEmptyGraph();
    GRAPH(sunk_graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x1).s64();
        CONSTANT(4, 0x0).s64();
        CONSTANT(5, 0x42).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(6, Opcode::Add).s64().Inputs(1, 5);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(10, Opcode::Phi).s64().Inputs({{2, 4}, {3, 22}});
            INST(20, Opcode::LoadArray).s64().Inputs(2, 10);
            INST(21, Opcode::Add).s64().Inputs(20, 6);
            INST(22, Opcode::Add).s64().Inputs(21, 10);
            INST(23, Opcode::Add).s32().Inputs(10, 3);
            INST(24, Opcode::Compare).b().CC(CC_LT).Inputs(23, 0);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(24);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(26, Opcode::Add).s64().Inputs(21, 22);
            INST(27, Opcode::Return).s64().Inputs(26);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunk_graph));
}

/**
 * Sink instruction over critical edge
 */
TEST_F(CodeSinkTest, CriticalEdgeSinking)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Add).s32().Inputs(3, 2);
            INST(8, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(9, Opcode::Compare).b().CC(CC_NE).Inputs(8, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(13, Opcode::StoreObject).s64().Inputs(1, 8).TypeId(243);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::Return).s32().Inputs(5);
        }
    }
    Graph *sunk_graph = CreateEmptyGraph();
    GRAPH(sunk_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(8, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(9, Opcode::Compare).b().CC(CC_NE).Inputs(8, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(13, Opcode::StoreObject).s64().Inputs(1, 8).TypeId(243);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Add).s32().Inputs(3, 2);
            INST(15, Opcode::Return).s32().Inputs(5);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunk_graph));
}

/**
 * Do not sink loads over monitor
 */
TEST_F(CodeSinkTest, LoadOverMonitor)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(2, 1, 0, 3).SrcVregs({4, 3, 2, 0});
            INST(5, Opcode::NullCheck).ref().Inputs(2, 4);
            // Do not move load
            INST(6, Opcode::LoadObject).s64().Inputs(5).TypeId(243);
            // Safely move arithmetic operations
            INST(7, Opcode::Add).s64().Inputs(6, 1);
            INST(15, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(8, Opcode::Monitor).v0id().Entry().Inputs(0, 15);
            INST(9, Opcode::Compare).b().CC(CC_NE).Inputs(1, 3);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(11, Opcode::Monitor).v0id().Exit().Inputs(0, 16);
            INST(12, Opcode::Return).s64().Inputs(7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(13, Opcode::Monitor).v0id().Exit().Inputs(0, 17);
            INST(14, Opcode::Return).s64().Inputs(3);
        }
    }
    Graph *sunk_graph = CreateEmptyGraph();
    GRAPH(sunk_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(2, 1, 0, 3).SrcVregs({4, 3, 2, 0});
            INST(5, Opcode::NullCheck).ref().Inputs(2, 4);
            INST(6, Opcode::LoadObject).s64().Inputs(5).TypeId(243);
            INST(15, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(8, Opcode::Monitor).v0id().Entry().Inputs(0, 15);
            INST(9, Opcode::Compare).b().CC(CC_NE).Inputs(1, 3);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::Add).s64().Inputs(6, 1);
            INST(16, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(11, Opcode::Monitor).v0id().Exit().Inputs(0, 16);
            INST(12, Opcode::Return).s64().Inputs(7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(13, Opcode::Monitor).v0id().Exit().Inputs(0, 17);
            INST(14, Opcode::Return).s64().Inputs(3);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunk_graph));
}

/**
 * Reordering of Normal Load and subsequent Volatile Load is allowed
 */
// TODO(Kudriashov Evgenii) enable the test after fixing CodeSink
TEST_F(CodeSinkTest, DISABLED_LoadOverVolatileLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(2).SrcVregs({0});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);
            INST(6, Opcode::LoadObject).s64().Inputs(2).TypeId(176);
            INST(7, Opcode::LoadStatic).s64().Inputs(5).Volatile().TypeId(103);
            INST(8, Opcode::Compare).b().CC(CC_NE).Inputs(0, 7);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).s64().Inputs(6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Return).s64().Inputs(3);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(2).SrcVregs({0});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);
            INST(7, Opcode::LoadStatic).s64().Inputs(5).Volatile().TypeId(103);
            INST(8, Opcode::Compare).b().CC(CC_NE).Inputs(0, 7);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::LoadObject).s64().Inputs(2).TypeId(176);
            INST(11, Opcode::Return).s64().Inputs(6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Return).s64().Inputs(3);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

/**
 *   /---[2]---\
 *   |         |
 *  [3]<------[4]
 *   |         |
 *   |        [5]
 *   |         |
 *   \---[6]---/
 *
 * Sink from BB 4 to BB 5
 */
TEST_F(CodeSinkTest, IntermediateSinking)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 4);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(4, 5, 3)
        {
            INST(6, Opcode::Add).s32().Inputs(1, 2);
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(2, 4);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(10, Opcode::SaveState).Inputs(6, 2, 1, 0, 6).SrcVregs({5, 4, 3, 2, 1});
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::StoreObject).s32().Inputs(11, 6).TypeId(271);
        }
        BASIC_BLOCK(3, 6) {}
        BASIC_BLOCK(6, -1)
        {
            INST(18, Opcode::Return).s32().Inputs(2);
        }
    }
    Graph *sunk_graph = CreateEmptyGraph();
    GRAPH(sunk_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, 6, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 4);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(2, 4);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(6, Opcode::Add).s32().Inputs(1, 2);
            INST(10, Opcode::SaveState).Inputs(6, 2, 1, 0, 6).SrcVregs({5, 4, 3, 2, 1});
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::StoreObject).s32().Inputs(11, 6).TypeId(271);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(18, Opcode::Return).s32().Inputs(2);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<CodeSink>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), sunk_graph));
}

/**
 * Do not sink object allocations
 */
TEST_F(CodeSinkTest, Allocations)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(1, 1).s64();
            CONSTANT(4, 0x0).s64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(18, Opcode::SaveState).Inputs(1).SrcVregs({3});
                INST(19, Opcode::LoadAndInitClass).TypeId(231).ref().Inputs(18);
                INST(2, Opcode::NewObject).ref().TypeId(231).Inputs(19, 18);
                INST(3, Opcode::Compare).b().CC(CC_GT).Inputs(1, 4);
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
            }
            BASIC_BLOCK(4, 5)
            {
                INST(8, Opcode::LoadObject).s64().Inputs(2).TypeId(243);
                INST(11, Opcode::LoadObject).s64().Inputs(2).TypeId(257);
                INST(12, Opcode::Add).s64().Inputs(11, 8);
            }
            BASIC_BLOCK(3, 5)
            {
                INST(14, Opcode::Neg).s64().Inputs(1);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(16, Opcode::Phi).s64().Inputs({{4, 12}, {3, 14}});
                INST(17, Opcode::Return).s64().Inputs(16);
            }
        }
    }
    equal_graphs[0]->RunPass<CodeSink>();
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

/**
 * Do not sink over PHI statement
 */
TEST_F(CodeSinkTest, PhiUsers)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).s64();
            CONSTANT(5, 0x0).s64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(12, Opcode::AddI).s64().Inputs(0).Imm(0x3);
                INST(4, Opcode::Compare).b().CC(CC_GT).Inputs(0, 5);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(4, 3)
            {
                INST(8, Opcode::Neg).s64().Inputs(0);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(9, Opcode::Phi).s64().Inputs({{2, 12}, {4, 8}});
                INST(11, Opcode::Return).s64().Inputs(9);
            }
        }
    }
    equal_graphs[0]->RunPass<CodeSink>();
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

/**
 * Do not sink volatile loads because other paths might be broken because of it
 */
TEST_F(CodeSinkTest, SinkableVolatileLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadObject).s64().Volatile().Inputs(2).TypeId(176);
            INST(8, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).s64().Inputs(6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Return).s64().Inputs(3);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/**
 * Do not sink loads over volatile store
 */
TEST_F(CodeSinkTest, LoadOverVolatileStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(1, 2).SrcVregs({0, 1});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);
            INST(6, Opcode::LoadObject).s64().Inputs(2).TypeId(176);
            INST(7, Opcode::StoreStatic).ref().Volatile().Inputs(5, 1).TypeId(103);
            INST(8, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).s64().Inputs(6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Return).s64().Inputs(3);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/**
 * Do not sink loads over GC barriered store.
 */
TEST_F(CodeSinkTest, LoadOverRefStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadArray).ref().Inputs(1, 0);
            INST(7, Opcode::StoreArray).ref().Inputs(1, 0, 2);
            INST(8, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Return).ref().Inputs(6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(13, Opcode::Return).ref().Inputs(2);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/**
 *  Do not sink into irreducible loops.
 */
TEST_F(CodeSinkTest, SinkIntoIrreducible)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        CONSTANT(5, 0x2a).s64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(8, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 7, 9)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(6, 10, 9)
        {
            INST(20, Opcode::Phi).s32().Inputs({{2, 5}, {7, 11}});
            INST(26, Opcode::Mul).s32().Inputs(20, 5);
            INST(28, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(2);
        }
        // BB 10 and BB 7 represent an irreducible loop. Do not sink v26 into it.
        BASIC_BLOCK(10, 7)
        {
            INST(30, Opcode::SaveState).NoVregs();
            INST(36, Opcode::CallStatic).s64().InputsAutoType(26, 30);
        }
        BASIC_BLOCK(7, 6, 9)
        {
            INST(11, Opcode::Phi).s32().Inputs({{3, 5}, {10, 26}});
            INST(19, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(34, Opcode::Phi).s32().Inputs({{3, 1}, {6, 2}, {7, 3}});
            INST(35, Opcode::Return).s32().Inputs(34);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/**
 *  Do not try to sink if an instruction has no uses. It's a waste of time.
 */
TEST_F(CodeSinkTest, UselessSinking)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        CONSTANT(3, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            // v5 is not used anywhere
            INST(5, Opcode::Add).s64().Inputs(1, 2);
            INST(6, Opcode::Compare).b().CC(CC_NE).Inputs(0, 3);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Return).s64().Inputs(0);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(11, Opcode::Return).s64().Inputs(3);
        }
    }

    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<CodeSink>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}
}  // namespace panda::compiler
