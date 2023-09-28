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
#include "optimizer/optimizations/if_conversion.h"

namespace panda::compiler {
class IfConversionTest : public GraphTest {
public:
    IfConversionTest()
    {
        SetGraphArch(panda::RUNTIME_ARCH);
#ifndef NDEBUG
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        GetGraph()->SetLowLevelInstructionsEnabled();
#endif
    }
};

// NOLINTBEGIN(readability-magic-numbers)
/*
 * Test Graph:
 *              [entry}
 *                 |
 *            /---[2]---\
 *            |         |
 *           [3]        |
 *            |         |
 *            \---[4]---/
 *                 |
 *               [exit]
 */
TEST_F(IfConversionTest, TriangleTrueImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_B).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs({{2, 0}, {3, 5}});
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Compare).b().CC(CC_B).Inputs(0, 1);
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
            INST(4, Opcode::SelectImm).u64().SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5, 0, 3);
            INST(7, Opcode::Return).u64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfConversionTest, TriangleTrue)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::If).SrcType(DataType::UINT64).CC(CC_NE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(5, Opcode::Phi).u64().Inputs({{2, 0}, {3, 4}});
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 2);
            INST(3, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_NE).Inputs(4, 0, 0, 1);
            INST(6, Opcode::Return).u64().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test Graph:
 *              [entry}
 *                 |
 *            /---[2]---\
 *            |         |
 *            |        [3]
 *            |         |
 *            \---[4]---/
 *                 |
 *               [exit]
 */
TEST_F(IfConversionTest, TriangleFalseImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 4, 3)
        {
            INST(3, Opcode::Compare).b().CC(CC_AE).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs({{2, 0}, {3, 5}});
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Compare).b().CC(CC_AE).Inputs(0, 1);
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
            INST(4, Opcode::SelectImm).u64().SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0, 5, 3);
            INST(7, Opcode::Return).u64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfConversionTest, TriangleFalse)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 4, 3)
        {
            INST(3, Opcode::If).SrcType(DataType::UINT64).CC(CC_LT).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(5, Opcode::Phi).u64().Inputs({{2, 0}, {3, 4}});
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 2);
            INST(3, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_LT).Inputs(0, 4, 0, 1);
            INST(6, Opcode::Return).u64().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test Graph:
 *              [entry}
 *                 |
 *            /---[2]---\
 *            |         |
 *        /--[3]--\     |
 *        |       |     |
 *       [4]      |     |
 *        |       |     |
 *        \------[5]----/
 *                |
 *              [exit]
 */
TEST_F(IfConversionTest, JointTriangleImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
            INST(6, Opcode::Compare).b().Inputs(5, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Mul).u64().Inputs(5, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 0}, {3, 5}, {4, 8}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
            INST(6, Opcode::Compare).b().Inputs(5, 1);
            INST(8, Opcode::Mul).u64().Inputs(5, 2);
            INST(7, Opcode::SelectImm).u64().SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8, 5, 6);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 0}, {3, 7}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfConversionTest, TriangleTwice)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, 3, 5)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 1);
            INST(4, Opcode::If).SrcType(DataType::UINT64).CC(CC_NE).Inputs(3, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(5, Opcode::If).SrcType(DataType::UINT64).CC(CC_NE).Inputs(3, 2);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(6, Opcode::Mul).u64().Inputs(3, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {3, 3}, {4, 6}});
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 1);
            INST(6, Opcode::Mul).u64().Inputs(3, 2);
            INST(5, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_NE).Inputs(6, 3, 3, 2);
            INST(4, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_NE).Inputs(5, 0, 3, 1);
            INST(8, Opcode::Return).u64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfConversionTest, JointTriangleWithTrickFloatPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).f64();
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(3, Opcode::If).SrcType(DataType::UINT64).CC(CC_NE).Inputs(0, 2);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Mul).f64().Inputs(1, 1);
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
            INST(6, Opcode::If).SrcType(DataType::UINT64).CC(CC_NE).Inputs(5, 2);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Mul).u64().Inputs(5, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs({{2, 0}, {3, 5}, {4, 7}});
            INST(9, Opcode::Phi).f64().Inputs({{2, 1}, {3, 4}, {4, 4}});
            INST(10, Opcode::Mul).f64().Inputs(9, 9);
            INST(20, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).u64().InputsAutoType(8, 10, 20);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }

    GetGraph()->RunPass<IfConversion>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).f64();
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(3, Opcode::If).SrcType(DataType::UINT64).CC(CC_NE).Inputs(0, 2);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Mul).f64().Inputs(1, 1);
            INST(5, Opcode::Mul).u64().Inputs(0, 2);
            INST(7, Opcode::Mul).u64().Inputs(5, 2);
            INST(6, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_NE).Inputs(7, 5, 5, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(9, Opcode::Phi).f64().Inputs({{2, 1}, {3, 4}});
            INST(10, Opcode::Mul).f64().Inputs(9, 9);
            INST(20, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).u64().InputsAutoType(8, 10, 20);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test Graph:
 *              [entry}
 *                 |
 *            /---[2]---\
 *            |         |
 *           [3]       [4]
 *            |         |
 *            \---[5]---/
 *                 |
 *               [exit]
 */
TEST_F(IfConversionTest, DiamondImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();
        CONSTANT(2, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(1, 2);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Add).u32().Inputs(0, 1);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(7, Opcode::Sub).u32().Inputs(0, 1);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Phi).u32().Inputs({{4, 5}, {3, 7}});
            INST(9, Opcode::Return).u32().Inputs(8);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();
        CONSTANT(2, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Compare).b().Inputs(1, 2);
            INST(7, Opcode::Sub).u32().Inputs(0, 1);
            INST(5, Opcode::Add).u32().Inputs(0, 1);
            INST(4, Opcode::SelectImm).u32().SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7, 5, 3);
            INST(9, Opcode::Return).u32().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test Graph:
 *              [entry}
 *                 |
 *            /---[2]---\
 *            |         |
 *        /--[3]--\     |
 *        |       |     |
 *       [4]     [5]    |
 *        |       |     |
 *        \------[6]----/
 *                |
 *              [exit]
 */
TEST_F(IfConversionTest, JointDiamondImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 0);
            INST(5, Opcode::Compare).b().Inputs(4, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(7, Opcode::Mul).u64().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(8, Opcode::Mul).u64().Inputs(4, 0);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 0}, {4, 7}, {5, 8}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 6)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 0);
            INST(5, Opcode::Compare).b().Inputs(4, 1);
            INST(7, Opcode::Mul).u64().Inputs(4, 1);
            INST(8, Opcode::Mul).u64().Inputs(4, 0);
            INST(6, Opcode::SelectImm).u64().SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7, 8, 5);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 0}, {3, 6}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfConversionTest, JointDiamond)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
            INST(4, Opcode::If).SrcType(DataType::UINT64).CC(CC_GT).Inputs(3, 1);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(5, Opcode::Mul).u64().Inputs(3, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(6, Opcode::Mul).u64().Inputs(3, 0);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {4, 5}, {5, 6}});
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 6)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
            INST(5, Opcode::Mul).u64().Inputs(3, 1);
            INST(6, Opcode::Mul).u64().Inputs(3, 0);
            INST(4, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_GT).Inputs(5, 6, 3, 1);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {3, 4}});
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfConversionTest, JointDiamondWithDroppedSelect)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
            INST(4, Opcode::If).SrcType(DataType::UINT64).CC(CC_GT).Inputs(3, 1);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(5, Opcode::Mul).u64().Inputs(3, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(6, Opcode::Mul).u64().Inputs(3, 0);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {4, 5}, {5, 6}});
            INST(8, Opcode::Phi).u64().Inputs({{2, 0}, {4, 3}, {5, 3}});
            INST(9, Opcode::Add).u64().Inputs(7, 8);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    GetGraph()->RunPass<IfConversion>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 6)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
            INST(5, Opcode::Mul).u64().Inputs(3, 1);
            INST(6, Opcode::Mul).u64().Inputs(3, 0);
            INST(4, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_GT).Inputs(5, 6, 3, 1);
            // Second Select not needed
        }
        BASIC_BLOCK(6, -1)
        {
            INST(7, Opcode::Phi).u64().Inputs({{2, 0}, {3, 4}});
            INST(8, Opcode::Phi).u64().Inputs({{2, 0}, {3, 3}});
            INST(9, Opcode::Add).u64().Inputs(7, 8);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IfConversionTest, JointDiamondRunTwice)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::If).SrcType(DataType::UINT64).CC(CC_GE).Inputs(0, 1);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
            INST(4, Opcode::Mul).u64().Inputs(1, 1);
            INST(5, Opcode::If).SrcType(DataType::UINT64).CC(CC_GT).Inputs(4, 1);
        }
        BASIC_BLOCK(4, 6) {}
        BASIC_BLOCK(5, 6) {}
        BASIC_BLOCK(6, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs({{2, 0}, {4, 3}, {5, 3}});
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<IfConversion>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Mul).u64().Inputs(0, 0);
            INST(2, Opcode::Select).u64().SrcType(DataType::UINT64).CC(CC_GE).Inputs(3, 0, 0, 1);
            INST(7, Opcode::Return).u64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 *  No conversion allowed cases below.
 */
TEST_F(IfConversionTest, TriangleWithCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 10);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).u64().InputsAutoType(0, 2, 20);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs({{2, 0}, {3, 5}});
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<IfConversion>());
}

TEST_F(IfConversionTest, DiamondThreeOperations)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();
        CONSTANT(2, 42);
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().Inputs(1, 3);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(6, Opcode::Add).u32().Inputs(0, 1);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(7, Opcode::Add).u32().Inputs(0, 2);
            INST(8, Opcode::Sub).u32().Inputs(7, 1);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Phi).u32().Inputs({{4, 6}, {3, 8}});
            INST(10, Opcode::Return).u32().Inputs(9);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<IfConversion>());
}

TEST_F(IfConversionTest, DiamondThreePhis)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();
        CONSTANT(2, 42);
        CONSTANT(3, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Add).u32().Inputs(0, 1);
            INST(7, Opcode::Add).u32().Inputs(0, 2);
            INST(8, Opcode::Sub).u32().Inputs(7, 1);
            INST(4, Opcode::Compare).b().Inputs(1, 3);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Phi).u32().Inputs({{4, 6}, {3, 8}});
            INST(10, Opcode::Phi).u32().Inputs({{4, 6}, {3, 7}});
            INST(11, Opcode::Phi).u32().Inputs({{4, 7}, {3, 8}});
            INST(12, Opcode::Add).u32().Inputs(9, 10);
            INST(13, Opcode::Add).u32().Inputs(11, 12);
            INST(14, Opcode::Return).u32().Inputs(13);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<IfConversion>());
}

TEST_F(IfConversionTest, TriangleFloat)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();
        PARAMETER(2, 2).f64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::Mul).f64().Inputs(0, 2);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).f64().Inputs({{2, 0}, {3, 5}});
            INST(7, Opcode::Return).f64().Inputs(6);
        }
    }

    ASSERT_EQ(GetGraph()->RunPass<IfConversion>(), GetGraph()->GetEncoder()->CanEncodeFloatSelect());
}

TEST_F(IfConversionTest, TrianglePhiFloat)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).f64();
        PARAMETER(1, 1).f64();
        PARAMETER(2, 2).f64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Mul).f64().Inputs(0, 2);
            INST(3, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 4) {}  // Instructions 5 moved up manually
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).f64().Inputs({{2, 0}, {3, 5}});
            INST(7, Opcode::Return).f64().Inputs(6);
        }
    }

    ASSERT_EQ(GetGraph()->RunPass<IfConversion>(), GetGraph()->GetEncoder()->CanEncodeFloatSelect());
}

TEST_F(IfConversionTest, LoopInvariantPreventTriangle)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 0x64).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(30, 2).i64();

        BASIC_BLOCK(30, 2, 7)
        {
            INST(8, Opcode::Load).b().Inputs(1, 4);
            INST(11, Opcode::Load).b().Inputs(1, 5);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(7, 15, 2)
        {
            INST(34, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }

        BASIC_BLOCK(15, 2) {}

        BASIC_BLOCK(2, 20, 4)
        {
            INST(41, Opcode::Phi).b().Inputs(5, 5, 4);
            INST(23, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(4, 20, 18)
        {
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(11);
        }

        BASIC_BLOCK(18, 20) {}

        BASIC_BLOCK(20, 16)
        {
            INST(43, Opcode::Phi).b().Inputs(5, 5, 4);
        }

        BASIC_BLOCK(16, 3, 5)
        {
            INST(13, Opcode::Phi).i32().Inputs(4, 53);
            INST(14, Opcode::Phi).i32().Inputs(5, 37);
            INST(44, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(26, Opcode::Add).i32().Inputs(13, 14);
        }

        BASIC_BLOCK(3, 6, 8)
        {
            INST(27, Opcode::Phi).i32().Inputs(14, 26);
            INST(52, Opcode::AddI).i32().Imm(2).Inputs(27);
            INST(29, Opcode::Add).i32().Inputs(27, 30);
            INST(42, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(41);
        }

        BASIC_BLOCK(6, 8)
        {
            INST(36, Opcode::Sub).i32().Inputs(52, 13);
        }

        BASIC_BLOCK(8, 31, 16)
        {
            INST(37, Opcode::Phi).i32().Inputs(52, 36);
            INST(53, Opcode::AddI).i32().Imm(1).Inputs(13);
            INST(39, Opcode::Add).i32().Inputs(13, 5);
            INST(21, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_GE).Imm(0x64).Inputs(53);
        }

        BASIC_BLOCK(31, -1)
        {
            INST(40, Opcode::Return).i32().Inputs(37);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<IfConversion>());
}

TEST_F(IfConversionTest, LoopInvariantPreventDiamond)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 0x64).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(30, 2).i64();

        BASIC_BLOCK(30, 2, 7)
        {
            INST(8, Opcode::Load).b().Inputs(1, 4);
            INST(11, Opcode::Load).b().Inputs(1, 5);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(7, 15, 2)
        {
            INST(34, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }

        BASIC_BLOCK(15, 2) {}

        BASIC_BLOCK(2, 20, 4)
        {
            INST(41, Opcode::Phi).b().Inputs(5, 5, 4);
            INST(23, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(4, 20, 18)
        {
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(11);
        }

        BASIC_BLOCK(18, 20) {}

        BASIC_BLOCK(20, 16)
        {
            INST(43, Opcode::Phi).b().Inputs(5, 5, 4);
        }

        BASIC_BLOCK(16, 32, 5)
        {
            INST(13, Opcode::Phi).i32().Inputs(4, 53);
            INST(14, Opcode::Phi).i32().Inputs(5, 37);
            INST(44, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(26, Opcode::Add).i32().Inputs(13, 14);
        }

        BASIC_BLOCK(32, 3)
        {
            INST(126, Opcode::Sub).i32().Inputs(13, 14);
        }

        BASIC_BLOCK(3, 6, 33)
        {
            INST(27, Opcode::Phi).i32().Inputs(26, 126);
            INST(52, Opcode::AddI).i32().Imm(2).Inputs(27);
            INST(29, Opcode::Add).i32().Inputs(27, 30);
            INST(42, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(41);
        }

        BASIC_BLOCK(6, 8)
        {
            INST(36, Opcode::Sub).i32().Inputs(52, 13);
        }

        BASIC_BLOCK(33, 8)
        {
            INST(136, Opcode::Sub).i32().Inputs(13, 52);
        }

        BASIC_BLOCK(8, 31, 16)
        {
            INST(37, Opcode::Phi).i32().Inputs(36, 136);
            INST(53, Opcode::AddI).i32().Imm(1).Inputs(13);
            INST(39, Opcode::Add).i32().Inputs(13, 5);
            INST(21, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_GE).Imm(0x64).Inputs(53);
        }

        BASIC_BLOCK(31, -1)
        {
            INST(40, Opcode::Return).i32().Inputs(37);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<IfConversion>());
}

TEST_F(IfConversionTest, NonLoopInvariantNotPreventConversion)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1, 1).ptr();
        PARAMETER(2, 2).i64();
        PARAMETER(3, 3).i64();
        CONSTANT(12, 1).i64();
        CONSTANT(13, 0).i64();
        CONSTANT(80, 10).i64();

        BASIC_BLOCK(6, 31, 5)
        {
            INST(16, Opcode::Phi).i32().Inputs(12, 62);
            INST(21, Opcode::Load).i32().Inputs(1, 12);
            INST(22, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LE).Inputs(21, 16);
            INST(23, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(22);
        }

        BASIC_BLOCK(5, 3, 4)
        {
            INST(28, Opcode::Phi).i64().Inputs(13, 49);
            INST(37, Opcode::Load).b().Inputs(1, 13);
            INST(39, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(37);
        }

        BASIC_BLOCK(4, 21, 20)
        {
            INST(93, Opcode::Load).i32().Inputs(1, 2);
            INST(94, Opcode::Compare).b().SrcType(DataType::INT32).Inputs(93, 80);
            INST(95, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(94);
        }

        BASIC_BLOCK(20, 21) {}

        BASIC_BLOCK(21, 22, 23)
        {
            INST(96, Opcode::Phi).i32().Inputs(12, 13);
            INST(99, Opcode::Load).i32().Inputs(1, 3);
            INST(101, Opcode::If).SrcType(DataType::INT32).CC(CC_NE).Inputs(80, 99);
        }

        BASIC_BLOCK(23, 22) {}

        BASIC_BLOCK(22, 18, 25)
        {
            INST(102, Opcode::Phi).i32().Inputs(12, 13);
            INST(103, Opcode::And).i32().Inputs(102, 96);
            INST(104, Opcode::Compare).b().SrcType(DataType::INT32).Inputs(103, 13);
            INST(105, Opcode::If).SrcType(DataType::INT32).CC(CC_NE).Inputs(13, 103);
        }

        BASIC_BLOCK(25, -1)
        {
            INST(106, Opcode::SaveState).Inputs(102, 96).SrcVregs({0, 1});
            INST(107, Opcode::LoadAndInitClass).ref().Inputs(106);
            INST(108, Opcode::NewObject).ref().Inputs(107, 106);
            INST(109, Opcode::SaveState).Inputs(108).SrcVregs({0});
            INST(113, Opcode::Throw).Inputs(108, 109);
        }

        BASIC_BLOCK(18, 5)
        {
            INST(49, Opcode::Add).i64().Inputs(28, 12);
        }

        BASIC_BLOCK(3, 6)
        {
            INST(62, Opcode::Add).i32().Inputs(16, 12);
        }

        BASIC_BLOCK(31, -1)
        {
            INST(58, Opcode::Return).i32().Inputs(21);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<IfConversion>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1, 1).ptr();
        PARAMETER(2, 2).i64();
        PARAMETER(3, 3).i64();
        CONSTANT(12, 1).i64();
        CONSTANT(13, 0).i64();
        CONSTANT(80, 10).i64();

        BASIC_BLOCK(42, 6) {}

        BASIC_BLOCK(6, 31, 5)
        {
            INST(16, Opcode::Phi).i32().Inputs(12, 62);
            INST(21, Opcode::Load).i32().Inputs(1, 12);
            INST(22, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LE).Inputs(21, 16);
            INST(23, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(22);
        }

        BASIC_BLOCK(5, 3, 4)
        {
            INST(28, Opcode::Phi).i64().Inputs(13, 49);
            INST(37, Opcode::Load).b().Inputs(1, 13);
            INST(39, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(37);
        }

        BASIC_BLOCK(4, 18, 25)
        {
            INST(93, Opcode::Load).i32().Inputs(1, 2);
            INST(94, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_EQ).Inputs(93, 80);
            INST(115, Opcode::SelectImm).i32().SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(12, 13, 94);
            INST(99, Opcode::Load).i32().Inputs(1, 3);
            INST(114, Opcode::Select).i32().SrcType(DataType::INT32).CC(CC_NE).Inputs(12, 13, 80, 99);
            INST(103, Opcode::And).i32().Inputs(114, 115);
            INST(104, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_EQ).Inputs(103, 13);
            INST(105, Opcode::If).SrcType(DataType::INT32).CC(CC_NE).Inputs(13, 103);
        }

        BASIC_BLOCK(25, -1)
        {
            INST(106, Opcode::SaveState).Inputs(114, 115).SrcVregs({0, 1});
            INST(107, Opcode::LoadAndInitClass).ref().Inputs(106);
            INST(108, Opcode::NewObject).ref().Inputs(107, 106);
            INST(109, Opcode::SaveState).Inputs(108).SrcVregs({0});
            INST(113, Opcode::Throw).Inputs(108, 109);
        }

        BASIC_BLOCK(18, 5)
        {
            INST(49, Opcode::Add).i64().Inputs(28, 12);
        }

        BASIC_BLOCK(3, 6)
        {
            INST(62, Opcode::Add).i32().Inputs(16, 12);
        }

        BASIC_BLOCK(31, -1)
        {
            INST(58, Opcode::Return).i32().Inputs(21);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
