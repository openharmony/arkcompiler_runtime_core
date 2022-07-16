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
#include "optimizer/optimizations/redundant_loop_elimination.h"

namespace panda::compiler {
class RedundantLoopEliminationTest : public CommonTest {
public:
    RedundantLoopEliminationTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

protected:
    Graph *graph_ {nullptr};
};

TEST_F(RedundantLoopEliminationTest, SimpleLoopTest1)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(3, 4, 5)
        {
            INST(20, Opcode::SafePoint).Inputs().SrcVregs({});
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(10, Opcode::Add).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 5) {}
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, SimpleLoopTest2)
{
    // not applied, insts from loop have user outside the loop.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(10, Opcode::Add).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(10, Opcode::Add).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, SimpleLoopTest3)
{
    // not applied, loop contains not removable inst
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).s32().InputsAutoType(20);
            INST(10, Opcode::Add).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs(0, 10);
            INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).s32().InputsAutoType(20);
            INST(10, Opcode::Add).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, InnerLoopsTest1)
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(3, 0).s32();
        PARAMETER(4, 1).s32();
        BASIC_BLOCK(2, 5, 4)
        {
            INST(5, Opcode::Phi).s32().Inputs(0, 8);
            INST(6, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(5, 6, 3)
        {
            INST(10, Opcode::Phi).s32().Inputs(0, 13);
            INST(11, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10, 3);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(6, 5)
        {
            INST(13, Opcode::Add).s32().Inputs(10, 1);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(8, Opcode::Add).s32().Inputs(5, 1);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(3, 0).s32();
        PARAMETER(4, 1).s32();
        BASIC_BLOCK(2, 5) {}
        BASIC_BLOCK(5, 1)
        {
            INST(12, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, InnerLoopsTest2)
{
    // not applied, inner loop contains not removable inst
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(3, 0).s32();
        PARAMETER(4, 1).s32();
        BASIC_BLOCK(2, 5, 4)
        {
            INST(5, Opcode::Phi).s32().Inputs(0, 8);
            INST(6, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(5, 6, 3)
        {
            INST(10, Opcode::Phi).s32().Inputs(0, 13);
            INST(11, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10, 3);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(6, 5)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).s32().InputsAutoType(20);
            INST(13, Opcode::Add).s32().Inputs(10, 1);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(8, Opcode::Add).s32().Inputs(5, 1);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(3, 0).s32();
        PARAMETER(4, 1).s32();
        BASIC_BLOCK(2, 5, 4)
        {
            INST(5, Opcode::Phi).s32().Inputs(0, 8);
            INST(6, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(5, 6, 3)
        {
            INST(10, Opcode::Phi).s32().Inputs(0, 13);
            INST(11, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10, 3);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(6, 5)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).s32().InputsAutoType(20);
            INST(13, Opcode::Add).s32().Inputs(10, 1);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(8, Opcode::Add).s32().Inputs(5, 1);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, InnerLoopsTest3)
{
    // applied, outher loop contains not removable inst, but inner loop
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(3, 0).s32();
        PARAMETER(4, 1).s32();
        BASIC_BLOCK(2, 5, 4)
        {
            INST(5, Opcode::Phi).s32().Inputs(0, 8);
            INST(6, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(5, 6, 3)
        {
            INST(10, Opcode::Phi).s32().Inputs(0, 13);
            INST(11, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(10, 3);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(6, 5)
        {
            INST(13, Opcode::Add).s32().Inputs(10, 1);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).s32().InputsAutoType(20);
            INST(8, Opcode::Add).s32().Inputs(5, 1);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(3, 0).s32();
        PARAMETER(4, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Phi).s32().Inputs(0, 8);
            INST(6, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(5, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).s32().InputsAutoType(20);
            INST(8, Opcode::Add).s32().Inputs(5, 1);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(9, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, SimpleLoopTestIncAfterPeeling)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(4, Opcode::Phi).s32().Inputs({{2, 0}, {3, 10}});
            INST(10, Opcode::Add).s32().Inputs(4, 1);               // i++
            INST(13, Opcode::Compare).CC(CC_LT).b().Inputs(10, 2);  // i < len_array
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<RedundantLoopElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 2);
            INST(5, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0, 2);  // 0 < len_array
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, 1)
        {
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RedundantLoopEliminationTest, InfiniteLoop)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, 2) {}
    }
    auto clone_graph =
        GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone_graph));
}

TEST_F(RedundantLoopEliminationTest, LoadObjectTest)
{
    for (bool volat : {true, false}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0, 0);
            CONSTANT(1, 1);
            CONSTANT(2, 10);
            PARAMETER(3, 0).ref();
            BASIC_BLOCK(3, 4, 5)
            {
                INST(4, Opcode::Phi).s32().Inputs(0, 10);
                INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(4, 3)
            {
                INST(9, Opcode::LoadObject).s32().Inputs(3).Volatile(volat);
                INST(10, Opcode::Add).s32().Inputs(4, 1);
            }
            BASIC_BLOCK(5, 1)
            {
                INST(12, Opcode::ReturnVoid);
            }
        }
        ASSERT_TRUE(graph1->RunPass<RedundantLoopElimination>());
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            CONSTANT(0, 0);
            CONSTANT(1, 1);
            CONSTANT(2, 10);
            PARAMETER(3, 0).ref();
            BASIC_BLOCK(2, 5) {}
            BASIC_BLOCK(5, 1)
            {
                INST(12, Opcode::ReturnVoid);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(RedundantLoopEliminationTest, StoreObjectTest)
{
    for (bool volat : {true, false}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0, 0);
            CONSTANT(2, 10);
            PARAMETER(3, 0).ref();
            BASIC_BLOCK(3, 4, 5)
            {
                INST(4, Opcode::Phi).s32().Inputs(0, 10);
                INST(5, Opcode::Compare).CC(CC_LT).b().Inputs(4, 2);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(4, 3)
            {
                INST(9, Opcode::StoreObject).s32().Inputs(3, 2).Volatile(volat);
                INST(10, Opcode::Add).s32().Inputs(4, 2);
            }
            BASIC_BLOCK(5, 1)
            {
                INST(12, Opcode::ReturnVoid);
            }
        }
        auto graph2 = GraphCloner(graph1, graph1->GetAllocator(), graph1->GetLocalAllocator()).CloneGraph();
        ASSERT_FALSE(graph1->RunPass<RedundantLoopElimination>());
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(RedundantLoopEliminationTest, LoopWithSeveralExits)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::IfImm).b().CC(CC_NE).Inputs(0).Imm(0);
        }
        BASIC_BLOCK(4, 3, 6)
        {
            INST(5, Opcode::IfImm).b().CC(CC_NE).Inputs(1).Imm(0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::Return).s32().Inputs(0);
        }
        BASIC_BLOCK(6, 1)
        {
            INST(7, Opcode::Return).s32().Inputs(1);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(RedundantLoopEliminationTest, PotentiallyInfinitLoop)
{
    // This is potentially infinite loop. We can't remove it.
    // public static void loop1(boolean incoming) {
    //     while (incoming) {}
    // }
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).s32();
        BASIC_BLOCK(2, 3, 2)
        {
            INST(2, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 0);
            INST(3, Opcode::IfImm).CC(CC_NE).Inputs(2).Imm(0);
        }
        BASIC_BLOCK(3, 1)
        {
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<RedundantLoopElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}
}  // namespace panda::compiler
