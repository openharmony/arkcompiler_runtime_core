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
#include "optimizer/optimizations/if_merging.h"
#include "optimizer/optimizations/branch_elimination.h"

namespace panda::compiler {
class IfMergingTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
// Duplicate Ifs dominating each other can be merged
TEST_F(IfMergingTest, SameIfs)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            CONSTANT(1, 0);
            CONSTANT(2, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
                INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, 5, 6)
            {
                INST(5, Opcode::Phi).u64().Inputs({{3, 1}, {2, 2}});
                INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0).Inputs(3);
            }
            BASIC_BLOCK(5, 7)
            {
                INST(8, Opcode::Add).u64().Inputs(5, 0);
            }
            BASIC_BLOCK(6, 7)
            {
                INST(9, Opcode::Sub).u64().Inputs(5, 0);
            }
            BASIC_BLOCK(7, -1)
            {
                INST(10, Opcode::Phi).u64().Inputs({{5, 8}, {6, 9}});
                INST(11, Opcode::Return).u64().Inputs(10);
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graph_expected = CreateEmptyGraph();
        GRAPH(graph_expected)
        {
            PARAMETER(0, 0).u64();
            CONSTANT(1, 0);
            CONSTANT(2, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
                INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
            }
            BASIC_BLOCK(3, 5)
            {
                INST(8, inverse ? Opcode::Sub : Opcode::Add).u64().Inputs(1, 0);
            }
            BASIC_BLOCK(4, 5)
            {
                INST(9, inverse ? Opcode::Add : Opcode::Sub).u64().Inputs(2, 0);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(10, Opcode::Phi).u64().Inputs({{3, 8}, {4, 9}});
                INST(11, Opcode::Return).u64().Inputs(10);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
    }
}

// If inputs are different, not applied
TEST_F(IfMergingTest, DifferentIfs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, 5, 6)
        {
            INST(5, Opcode::Phi).u64().Inputs({{3, 1}, {2, 2}});
            INST(12, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(12);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(8, Opcode::Add).u64().Inputs(5, 0);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(9, Opcode::Sub).u64().Inputs(5, 0);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(10, Opcode::Phi).u64().Inputs({{5, 8}, {6, 9}});
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// If an instruction from second If block is used in both branches, we would need to copy it.
// Not applied
TEST_F(IfMergingTest, InstUsedInBothBranches)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, 5, 6)
        {
            INST(5, Opcode::Phi).u64().Inputs({{3, 1}, {2, 2}});
            INST(6, Opcode::Mul).u64().Inputs(5, 5);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(8, Opcode::Add).u64().Inputs(6, 0);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(9, Opcode::Sub).u64().Inputs(6, 0);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(10, Opcode::Phi).u64().Inputs({{5, 8}, {6, 9}});
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Instruction in If block can be split to true and false branches
TEST_F(IfMergingTest, CheckInstsSplit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            INST(9, Opcode::Phi).u64().Inputs({{4, 1}, {3, 2}});
            INST(10, Opcode::Mul).u64().Inputs(9, 9);
            INST(11, Opcode::Add).u64().Inputs(10, 9);
            INST(12, Opcode::Sub).u64().Inputs(0, 9);
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(14, Opcode::Return).u64().Inputs(11);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(15, Opcode::Return).u64().Inputs(12);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
            INST(10, Opcode::Mul).u64().Inputs(2, 2);
            INST(12, Opcode::Add).u64().Inputs(10, 2);
            INST(15, Opcode::Return).u64().Inputs(12);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
            INST(11, Opcode::Sub).u64().Inputs(0, 1);
            INST(14, Opcode::Return).u64().Inputs(11);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   |     v    |
 *   \--->[4]---/
 *         |
 *         v
 *        [5]
 *
 * Transform to (before cleanup):
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   v     v    |
 *  [4']  [4]---/
 *   |
 *   v
 *  [5]
 *
 */
TEST_F(IfMergingTest, SameIfsLoopBackEdge)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            CONSTANT(1, 0);
            CONSTANT(2, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {4, 4}});
                INST(4, Opcode::Sub).u64().Inputs(3, 2);
                INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(4, 1);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, 5, 2)
            {
                INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {3, 1}});
                INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(9, Opcode::Return).u64().Inputs(7);
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graph_expected = CreateEmptyGraph();
        GRAPH(graph_expected)
        {
            PARAMETER(0, 0).u64();
            CONSTANT(1, 0);
            CONSTANT(2, 1);
            BASIC_BLOCK(2, 3, 2)
            {
                INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {2, 4}});
                INST(4, Opcode::Sub).u64().Inputs(3, 2);
                INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(4, 1);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(9, Opcode::Return).u64().Inputs(inverse ? 0 : 1);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
    }
}

/*
 * We cannot remove the second If because branches of the first If intersect
 *         [entry]
 *            |
 *            v
 *           [if]-->[4]
 *            |      |
 *            v      v
 *           [3]--->[5]
 *            |      |
 *            v      |
 *   [7]<----[if]<---/
 *    |       |
 *    v       v
 *  [exit]<--[8]
 */
TEST_F(IfMergingTest, SameIfsMixedBranches)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0, 2);
            INST(7, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(4, 5) {}

        BASIC_BLOCK(5, 6) {}

        BASIC_BLOCK(6, 7, 8)
        {
            INST(8, Opcode::Phi).b().Inputs(1, 2);
            INST(9, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(10, Opcode::Return).u64().Inputs(1);
        }

        BASIC_BLOCK(8, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(0);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Equivalent If instructions dominate each other, but block with the second If has
// more than two predecessors. Not applied
TEST_F(IfMergingTest, BlockWithThreePredecessors)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, 5, 3)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0, 2);
            INST(7, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(4, 5) {}

        BASIC_BLOCK(5, 6, 7)
        {
            INST(8, Opcode::Phi).u64().Inputs({{2, 0}, {3, 1}, {4, 2}});
            INST(9, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(10, Opcode::Return).u64().Inputs(8);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(11, Opcode::Return).u64().Inputs(1);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Equivalent If instructions dominate each other, but block with the second If has
// only one predecessor. Not applied. This is a case for branch elimination
TEST_F(IfMergingTest, BlockWithOnePredecessor)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Return).u64().Inputs(2);
        }

        BASIC_BLOCK(4, 5, 6)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Return).u64().Inputs(1);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(9, Opcode::Return).u64().Inputs(1);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(IfMergingTest, ConstPhiIf)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            INST(9, Opcode::Phi).u64().Inputs({{3, 1}, {4, 2}});
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(9, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(13, Opcode::Return).b().Inputs(2);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
            INST(13, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
            INST(12, Opcode::Return).b().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfMergingTest, ConstantEqualToAllPhiInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            INST(9, Opcode::Phi).u64().Inputs({{3, 1}, {4, 1}});
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(9, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(13, Opcode::Return).b().Inputs(2);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(12, Opcode::Return).b().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfMergingTest, ConstantNotEqualToPhiInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(14, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            INST(9, Opcode::Phi).u64().Inputs({{3, 1}, {4, 2}});
            INST(10, Opcode::Compare).b().CC(CC_EQ).Inputs(9, 14);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(13, Opcode::Return).b().Inputs(2);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(13, Opcode::Return).b().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfMergingTest, InstInPhiBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(1, 5);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(2, 7);
        }
        BASIC_BLOCK(5, 6, 7)
        {
            INST(9, Opcode::Phi).u64().Inputs({{3, 1}, {4, 2}});
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(0, 10);
            INST(12, Opcode::Compare).b().CC(CC_EQ).Inputs(9, 2);
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(12);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(14, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(15, Opcode::Return).b().Inputs(2);
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

/*
 *        [0]
 *         |
 *         v
 *        [2]--->[4]----\
 *         |      |     |
 *         v      v     v
 *        [3]    [5]   [6]
 *         |      |     |
 *         |<-----+-----/
 *         v
 *        [7]--->[9]----\
 *         |      |     |
 *         v      v     v
 *        [8]    [10]  [11]
 *
 * Transform to:
 *        [0]
 *         |
 *         v
 *        [2]--->[4]----\
 *         |      |     |
 *         v      v     v
 *        [3]    [5]   [6]
 *         |      |     |
 *         v      v     v
 *        [8]    [10]  [11]
 */
TEST_F(IfMergingTest, NestedIf)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(3, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(6, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(1, 6);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(8, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(2, 10);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(12, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).v0id().InputsAutoType(3, 12);
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(14, Opcode::Phi).u64().Inputs({{3, 1}, {5, 2}, {6, 3}});
            INST(15, Opcode::Compare).b().CC(CC_EQ).Inputs(14, 2);
            INST(16, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(17, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(9, 10, 11)
        {
            INST(18, Opcode::Compare).b().CC(CC_EQ).Inputs(14, 1);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }
        BASIC_BLOCK(10, -1)
        {
            INST(20, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(11, -1)
        {
            INST(21, Opcode::Return).b().Inputs(3);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(3, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(6, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(1, 6);
            INST(20, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(8, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(2, 10);
            INST(17, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).v0id().InputsAutoType(3, 12);
            INST(21, Opcode::Return).b().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfMergingTest, DuplicatePhiInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(3, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(6, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(1, 6);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(8, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, 7)
        {
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(2, 10);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(12, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).v0id().InputsAutoType(3, 12);
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(14, Opcode::Phi).u64().Inputs({{3, 1}, {5, 2}, {6, 1}});
            INST(15, Opcode::Compare).b().CC(CC_EQ).Inputs(14, 1);
            INST(16, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(17, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(18, Opcode::Return).b().Inputs(2);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(3, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 7)
        {
            INST(6, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(1, 6);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(8, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(2, 10);
            INST(18, Opcode::Return).b().Inputs(2);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(12, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).v0id().InputsAutoType(3, 12);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Return).b().Inputs(1);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *        [0]
 *         |
 *         v   0
 *        [2]-----\
 *         |      |
 *         v    2 |
 *        [3]-----+
 *         |      |
 *         v    1 |
 *        [4]-----+
 *         |      |
 *         v   3  v       F
 *        [5]--->[Phi < 2]-->[8]
 *                  T|
 *                   v
 *                  [7]
 *
 * Transform to:
 *           [0]
 *            |
 *            v   0
 *           [2]-----\
 *            |      |
 *        2   v      |
 *      /----[3]     |
 *      |     |      |
 *      |     v   1  v
 *      |    [4]--->[7]
 *      |     |
 *      v  3  v
 *     [8]<--[5]
 */
TEST_F(IfMergingTest, SplitPhiIntoTwo)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(3, 2);
        CONSTANT(4, 3);
        BASIC_BLOCK(2, 6, 3)
        {
            INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 6, 4)
        {
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 6, 5)
        {
            INST(9, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 3);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 6) {}
        BASIC_BLOCK(6, 7, 8)
        {
            INST(11, Opcode::Phi).u64().Inputs({{2, 1}, {3, 3}, {4, 2}, {5, 4}});
            INST(12, Opcode::Add).u64().Inputs(11, 0);
            INST(13, Opcode::Compare).b().CC(CC_LT).Inputs(11, 3);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(15, Opcode::Return).u64().Inputs(11);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(16, Opcode::Return).u64().Inputs(12);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(3, 2);
        CONSTANT(4, 3);
        BASIC_BLOCK(2, 7, 3)
        {
            INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 8, 4)
        {
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 7, 8)
        {
            INST(9, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 3);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(17, Opcode::Phi).u64().Inputs({{2, 1}, {4, 2}});
            INST(15, Opcode::Return).u64().Inputs(17);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(18, Opcode::Phi).u64().Inputs({{3, 3}, {4, 4}});
            INST(12, Opcode::Add).u64().Inputs(18, 0);
            INST(16, Opcode::Return).u64().Inputs(12);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   |     v    |
 *   \--->[4]---/
 *         |
 *         v
 *        [5]
 *
 * Transform to (before cleanup):
 *        [0]
 *         |
 *         v
 *  [3]<--[2]<--\
 *   |     |    |
 *   v     v    |
 *  [4']  [4]---/
 *   |
 *   v
 *  [5]
 *
 */
TEST_F(IfMergingTest, ConstantPhiLoopBackEdge)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            CONSTANT(1, 0);
            CONSTANT(2, 2);
            CONSTANT(3, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Phi).u64().Inputs({{0, 0}, {4, 5}});
                INST(5, Opcode::Sub).u64().Inputs(4, 3);
                INST(6, Opcode::Compare).b().CC(CC_EQ).Inputs(5, 1);
                INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, 5, 2)
            {
                INST(8, Opcode::Phi).u64().Inputs({{2, 1}, {3, 2}});
                INST(9, Opcode::Compare).b().CC(CC_LE).Inputs(4, 3);
                INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0).Inputs(6);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(11, Opcode::Return).u64().Inputs(8);
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graph_expected = CreateEmptyGraph();
        GRAPH(graph_expected)
        {
            PARAMETER(0, 0).u64();
            CONSTANT(1, 0);
            if (!inverse) {
                CONSTANT(2, 2);
            }
            CONSTANT(3, 1);
            BASIC_BLOCK(2, 3, 2)
            {
                INST(4, Opcode::Phi).u64().Inputs({{0, 0}, {2, 5}});
                INST(5, Opcode::Sub).u64().Inputs(4, 3);
                INST(6, Opcode::Compare).b().CC(CC_EQ).Inputs(5, 1);
                INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(11, Opcode::Return).u64().Inputs(inverse ? 1 : 2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
    }
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     v     v
 *      \----[3]  [exit]
 *
 * Transform to:
 *
 * [0] -> [3] -> [exit]
 *
 */
TEST_F(IfMergingTest, ConstantPhiUnrollLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 2}});
            INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(4, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(7);
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     v     v
 *      \----[3]  [exit]
 *
 * Transform to:
 *
 *           [0]
 *            |
 *            v
 *      /--->[2]
 *      |     |
 *      |     v
 *      \----[3]
 *
 */
TEST_F(IfMergingTest, ConstantPhiRemoveLoopExit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 2}});
            INST(5, Opcode::Compare).b().CC(CC_GE).Inputs(4, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, 3) {}

        BASIC_BLOCK(3, 3)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(7);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     v     v
 *      \----[3]  [exit]
 *
 * Transform to:
 *
 * [0] -> [2] -> [exit]
 *
 */
TEST_F(IfMergingTest, ConstantPhiRemoveLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 2}});
            INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(4, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     |     v
 *      \-----/  [exit]
 *
 * Transform to:
 *
 * [0] -> [2] -> [exit]
 *
 */
TEST_F(IfMergingTest, ConstantPhiRemoveEmptyLoop)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            CONSTANT(1, 1);
            CONSTANT(2, 2);

            BASIC_BLOCK(2, 2, 3)
            {
                INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {2, 2}});
                INST(5, Opcode::Compare).b().CC(inverse ? CC_NE : CC_EQ).Inputs(4, 2);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(7, Opcode::ReturnVoid).v0id();
            }
        }

        ASSERT_TRUE(graph->RunPass<IfMerging>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto graph_expected = CreateEmptyGraph();
        GRAPH(graph_expected)
        {
            BASIC_BLOCK(2, -1)
            {
                INST(7, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
    }
}

/*
 *           [0]
 *            |
 *            v
 *      /--->[2]----\
 *      |     |     |
 *      |     |     v
 *      \-----/  [exit]
 *
 * Could be transformed to:
 *
 * [0] -> [2] -> [2] -> [exit]
 *
 * but basic block 2 would be duplicated. Not applied
 *
 */
TEST_F(IfMergingTest, ConstantPhiDontRemoveLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 2, 3)
        {
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {2, 2}});
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(5);
            INST(7, Opcode::Compare).b().CC(CC_EQ).Inputs(4, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<IfMerging>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
