/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "optimizer/optimizations/select_optimization.h"

namespace ark::compiler {
class SelectOptimizationTest : public GraphTest {
public:
    SelectOptimizationTest()
    {
        SetGraphArch(ark::RUNTIME_ARCH);
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(SelectOptimizationTest, SelectEqCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmEqCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectEqCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmEqCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectNeCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmNeCostantBoolOperands01X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).i32();
        CONSTANT(1U, 1U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectNeCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();
        CONSTANT(3U, 1U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Select).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U, 3U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmNeCostantBoolOperands10X1)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).i32();
        CONSTANT(1U, 0U).i32();
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());
    GraphChecker(graph).Check();

    Graph *expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(2U, 0U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::XorI).b().Inputs(2U).Imm(0x1);
            INST(7U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(SelectOptimizationTest, SelectImmDuplicateWithIntermediateSelect)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::SelectImm).i32().SrcType(DataType::INT32).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(5U, Opcode::SelectImm).i32().SrcType(DataType::INT32).CC(CC_NE).Inputs(1U, 0U, 2U).Imm(0x0);
            INST(6U, Opcode::Add).i32().Inputs(4U, 5U);
            INST(7U, Opcode::SelectImm).i32().SrcType(DataType::INT32).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(8U, Opcode::Add).i32().Inputs(6U, 7U);
            INST(9U, Opcode::Return).i32().Inputs(8U);
        }
    }

    ASSERT_TRUE(graph->RunPass<SelectOptimization>());
    GraphChecker(graph).Check();
    ASSERT_EQ(INS(8U).GetInput(1U).GetInst(), &INS(4U));
}

TEST_F(SelectOptimizationTest, SelectImmDuplicateWithDifferentResultTypes)
{
    Graph *graph = GetGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        PARAMETER(2U, 2U).b();

        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::SelectImm).i32().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(5U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(6U, Opcode::SelectImm).u8().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(7U, Opcode::SelectImm).i8().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(8U, Opcode::SelectImm).u16().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(9U, Opcode::SelectImm).i16().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(10U, Opcode::SelectImm).u32().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 1U, 2U).Imm(0x0);
            INST(11U, Opcode::SaveState).Inputs(4U, 5U, 6U, 7U, 8U, 9U, 10U).SrcVregs({0U, 1U, 2U, 3U, 4U, 5U, 6U});
            INST(12U, Opcode::ReturnVoid);
        }
    }

    ASSERT_FALSE(graph->RunPass<SelectOptimization>());
    GraphChecker(graph).Check();
    ASSERT_EQ(INS(11U).GetInput(0U).GetInst(), &INS(4U));
    ASSERT_EQ(INS(11U).GetInput(1U).GetInst(), &INS(5U));
    ASSERT_EQ(INS(11U).GetInput(2U).GetInst(), &INS(6U));
    ASSERT_EQ(INS(11U).GetInput(3U).GetInst(), &INS(7U));
    ASSERT_EQ(INS(11U).GetInput(4U).GetInst(), &INS(8U));
    ASSERT_EQ(INS(11U).GetInput(5U).GetInst(), &INS(9U));
    ASSERT_EQ(INS(11U).GetInput(6U).GetInst(), &INS(10U));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
