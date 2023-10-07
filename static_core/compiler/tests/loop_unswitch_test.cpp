/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "optimizer/ir/analysis.h"
#include "optimizer/optimizations/loop_peeling.h"
#include "optimizer/optimizations/loop_unswitch.h"
#include "optimizer/optimizations/branch_elimination.h"

namespace panda::compiler {
// NOLINTBEGIN(readability-magic-numbers,readability-function-size)
class LoopUnswitchTest : public GraphTest {
public:
    void CreateIncLoopGraph(int count)
    {
        GRAPH(GetGraph())
        {
            PARAMETER(1, 1).ptr();
            CONSTANT(3, 0).i64();
            CONSTANT(4, 1).i64();
            CONSTANT(100, count).i64();
            CONSTANT(8, 15).i64();

            BASIC_BLOCK(20, 10)
            {
                INST(7, Opcode::Load).i64().Inputs(1, 4);
                INST(19, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(7, 8);
            }

            BASIC_BLOCK(10, 9, 6)
            {
                INST(27, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(3, 4);
                INST(28, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(27);
            }

            BASIC_BLOCK(6, 2)
            {
                INST(10, Opcode::Phi).i32().Inputs(3, 25);
                INST(13, Opcode::Phi).i32().Inputs(4, 23);
            }

            BASIC_BLOCK(2, 3, 4)
            {
                INST(20, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
            }

            BASIC_BLOCK(4, 5)
            {
                INST(21, Opcode::Add).i32().Inputs(10, 13);
            }

            BASIC_BLOCK(3, 5)
            {
                INST(22, Opcode::Sub).i32().Inputs(13, 10);
            }

            BASIC_BLOCK(5, 11)
            {
                INST(23, Opcode::Phi).i32().Inputs(21, 22);
                INST(25, Opcode::Add).i32().Inputs(10, 4);
            }

            BASIC_BLOCK(11, 9, 6)
            {
                INST(17, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(25, 100);
                INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(17);
            }

            BASIC_BLOCK(9, 21)
            {
                INST(32, Opcode::Phi).i32().Inputs(4, 23);
            }

            BASIC_BLOCK(21, -1)
            {
                INST(26, Opcode::Return).i32().Inputs(32);
            }
        }
    }

    void CreateDecLoopGraph(int count)
    {
        GRAPH(GetGraph())
        {
            PARAMETER(1, 1).ptr();
            CONSTANT(3, 1).i64();
            CONSTANT(7, 15).i64();
            CONSTANT(16, 0).i64();
            CONSTANT(25, -1).i64();
            CONSTANT(100, count).i64();

            BASIC_BLOCK(20, 10)
            {
                INST(6, Opcode::Load).i64().Inputs(1, 7);
                INST(18, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(6, 7);
            }

            BASIC_BLOCK(10, 9, 6)
            {
                INST(27, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LE).Inputs(3, 16);
                INST(28, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(27);
            }

            BASIC_BLOCK(6, 2)
            {
                INST(9, Opcode::Phi).i32().Inputs(3, 22);
                INST(11, Opcode::Phi).i32().Inputs(100, 24);
            }

            BASIC_BLOCK(2, 3, 4)
            {
                INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
            }

            BASIC_BLOCK(4, 5)
            {
                INST(20, Opcode::Add).i32().Inputs(11, 9);
            }

            BASIC_BLOCK(3, 5)
            {
                INST(21, Opcode::Sub).i32().Inputs(9, 11);
            }

            BASIC_BLOCK(5, 11)
            {
                INST(22, Opcode::Phi).i32().Inputs(20, 21);
                INST(24, Opcode::Sub).i32().Inputs(11, 3);
            }

            BASIC_BLOCK(11, 9, 6)
            {
                INST(15, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LE).Inputs(24, 16);
                INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(15);
            }

            BASIC_BLOCK(9, 21)
            {
                INST(31, Opcode::Phi).i32().Inputs(3, 22);
            }

            BASIC_BLOCK(21, -1)
            {
                INST(26, Opcode::Return).i32().Inputs(31);
            }
        }
    }
};

TEST_F(LoopUnswitchTest, TestSingleCondition)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        CONSTANT(2, 100).i64();
        CONSTANT(3, 0).i64();
        CONSTANT(4, 1).i64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Phi).i64().Inputs(3, 11);
            INST(6, Opcode::Phi).i64().Inputs(3, 12);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(9, Opcode::Add).i64().Inputs(5, 6);
        }
        BASIC_BLOCK(6, 8)
        {
            INST(10, Opcode::Sub).i64().Inputs(5, 6);
        }
        BASIC_BLOCK(8, 2)
        {
            INST(11, Opcode::Phi).i64().Inputs(9, 10);
            INST(12, Opcode::Add).i64().Inputs(6, 4);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::Return).i64().Inputs(5);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        CONSTANT(2, 100).i64();
        CONSTANT(3, 0).i64();
        CONSTANT(4, 1).i64();

        BASIC_BLOCK(14, 27) {}

        BASIC_BLOCK(27, 16, 18)
        {
            INST(35, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(18, 25, 19)
        {
            INST(22, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3, 2);
            INST(23, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(22);
        }
        BASIC_BLOCK(19, 23)
        {
            INST(24, Opcode::Phi).i64().Inputs(3, 32);
            INST(25, Opcode::Phi).i64().Inputs(3, 28);
        }
        BASIC_BLOCK(23, 22)
        {
            // empty
        }
        BASIC_BLOCK(22, 21)
        {
            INST(32, Opcode::Sub).i64().Inputs(24, 25);
        }
        BASIC_BLOCK(21, 20)
        {
            INST(28, Opcode::Add).i64().Inputs(25, 4);
        }
        BASIC_BLOCK(20, 25, 19)
        {
            INST(26, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(28, 2);
            INST(27, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }
        BASIC_BLOCK(25, 26)
        {
            INST(33, Opcode::Phi).i64().Inputs(3, 32);
        }
        BASIC_BLOCK(16, 15, 2)
        {
            INST(16, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3, 2);
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(16);
        }
        BASIC_BLOCK(2, 4)
        {
            INST(5, Opcode::Phi).i64().Inputs(3, 9);
            INST(6, Opcode::Phi).i64().Inputs(3, 12);
        }
        BASIC_BLOCK(4, 6)
        {
            // empty
        }
        BASIC_BLOCK(6, 8)
        {
            INST(9, Opcode::Add).i64().Inputs(5, 6);
        }
        BASIC_BLOCK(8, 17)
        {
            INST(12, Opcode::Add).i64().Inputs(6, 4);
        }
        BASIC_BLOCK(17, 15, 2)
        {
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(12, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(15, 26)
        {
            INST(20, Opcode::Phi).i64().Inputs(3, 9);
        }
        BASIC_BLOCK(26, 3)
        {
            INST(34, Opcode::Phi).i64().Inputs(33, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::Return).i64().Inputs(34);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopUnswitchTest, TestSameCondition)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        CONSTANT(2, 100).i64();
        CONSTANT(3, 0).i64();
        CONSTANT(4, 1).i64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Phi).i64().Inputs(3, 16);
            INST(6, Opcode::Phi).i64().Inputs(3, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(5, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(6, 5)
        {
            INST(10, Opcode::Add).i64().Inputs(5, 6);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(11, Opcode::Phi).i64().Inputs(5, 10);
            INST(12, Opcode::Add).i64().Inputs(11, 4);
        }
        BASIC_BLOCK(8, 9, 10)
        {
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(9, 10)
        {
            INST(14, Opcode::Sub).i64().Inputs(12, 6);
        }
        BASIC_BLOCK(10, 2)
        {
            INST(15, Opcode::Phi).i64().Inputs(12, 14);
            INST(16, Opcode::Add).i64().Inputs(5, 4);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::Return).i64().Inputs(6);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        CONSTANT(2, 100).i64();
        CONSTANT(3, 0).i64();
        CONSTANT(4, 1).i64();

        BASIC_BLOCK(14, 29) {}

        BASIC_BLOCK(29, 16, 18)
        {
            INST(40, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(18, 27, 19)
        {
            INST(24, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3, 2);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(19, 24)
        {
            INST(26, Opcode::Phi).i64().Inputs(3, 30);
            INST(27, Opcode::Phi).i64().Inputs(3, 33);
        }

        BASIC_BLOCK(24, 25) {}

        BASIC_BLOCK(25, 23)
        {
            INST(36, Opcode::Add).i64().Inputs(26, 27);
        }

        BASIC_BLOCK(23, 22)
        {
            INST(33, Opcode::Add).i64().Inputs(36, 4);
        }

        BASIC_BLOCK(22, 21) {}

        BASIC_BLOCK(21, 20)
        {
            INST(30, Opcode::Add).i64().Inputs(26, 4);
        }

        BASIC_BLOCK(20, 27, 19)
        {
            INST(28, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(30, 2);
            INST(29, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(28);
        }

        BASIC_BLOCK(27, 28)
        {
            INST(38, Opcode::Phi).i64().Inputs(3, 33);
        }

        BASIC_BLOCK(16, 15, 2)
        {
            INST(18, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(3, 2);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }

        BASIC_BLOCK(2, 4)
        {
            INST(5, Opcode::Phi).i64().Inputs(3, 16);
            INST(6, Opcode::Phi).i64().Inputs(3, 14);
        }

        BASIC_BLOCK(4, 5) {}

        BASIC_BLOCK(5, 8)
        {
            INST(12, Opcode::Add).i64().Inputs(5, 4);
        }

        BASIC_BLOCK(8, 9) {}

        BASIC_BLOCK(9, 10)
        {
            INST(14, Opcode::Sub).i64().Inputs(12, 6);
        }

        BASIC_BLOCK(10, 17)
        {
            INST(16, Opcode::Add).i64().Inputs(5, 4);
        }

        BASIC_BLOCK(17, 15, 2)
        {
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(16, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(15, 28)
        {
            INST(23, Opcode::Phi).i64().Inputs(3, 14);
        }

        BASIC_BLOCK(28, 3)
        {
            INST(39, Opcode::Phi).i64().Inputs(38, 23);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::Return).i64().Inputs(39);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopUnswitchTest, TestMultipleConditions)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(12, 15).i64();
        CONSTANT(37, 2).i64();

        BASIC_BLOCK(20, 15)
        {
            INST(8, Opcode::Load).i64().Inputs(1, 4);
            INST(11, Opcode::Load).i64().Inputs(1, 5);
            INST(43, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11, 12);
            INST(38, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8, 12);
            INST(31, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(11, 12);
            INST(26, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8, 12);
        }

        BASIC_BLOCK(15, 14, 11)
        {
            INST(50, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(51, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(50);
        }

        BASIC_BLOCK(11, 2)
        {
            INST(16, Opcode::Phi).i32().Inputs(4, 48);
            INST(17, Opcode::Phi).i32().Inputs(5, 46);
        }

        BASIC_BLOCK(2, 3, 4)
        {
            INST(27, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(28, Opcode::Add).i32().Inputs(16, 17);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(29, Opcode::Phi).i32().Inputs(17, 28);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(31);
        }

        BASIC_BLOCK(6, 5)
        {
            INST(33, Opcode::Add).i32().Inputs(29, 16);
        }

        BASIC_BLOCK(5, 7, 8)
        {
            INST(34, Opcode::Phi).i32().Inputs(29, 33);
            INST(36, Opcode::Add).i32().Inputs(34, 37);
            INST(39, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(8, 7)
        {
            INST(40, Opcode::Sub).i32().Inputs(36, 16);
        }

        BASIC_BLOCK(7, 9, 10)
        {
            INST(41, Opcode::Phi).i32().Inputs(36, 40);
            INST(44, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(10, 9)
        {
            INST(45, Opcode::Sub).i32().Inputs(41, 16);
        }

        BASIC_BLOCK(9, 16)
        {
            INST(46, Opcode::Phi).i32().Inputs(41, 45);
            INST(48, Opcode::Add).i32().Inputs(16, 5);
        }

        BASIC_BLOCK(16, 14, 11)
        {
            INST(24, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48, 3);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(24);
        }

        BASIC_BLOCK(14, 21)
        {
            INST(55, Opcode::Phi).i32().Inputs(5, 46);
        }

        BASIC_BLOCK(21, -1)
        {
            INST(49, Opcode::Return).i32().Inputs(55);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(12, 15).i64();
        CONSTANT(37, 2).i64();

        BASIC_BLOCK(20, 71, 86)
        {
            INST(8, Opcode::Load).i64().Inputs(1, 4);
            INST(11, Opcode::Load).i64().Inputs(1, 5);
            INST(43, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11, 12);
            INST(38, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8, 12);
            INST(78, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(86, 42, 72)
        {
            INST(124, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(72, 21, 77)
        {
            INST(102, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(103, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(102);
        }

        BASIC_BLOCK(77, 21, 77)
        {
            INST(104, Opcode::Phi).i32().Inputs(4, 108);
            INST(105, Opcode::Phi).i32().Inputs(5, 121);
            INST(112, Opcode::Add).i32().Inputs(105, 37);
            INST(120, Opcode::Sub).i32().Inputs(112, 104);
            INST(121, Opcode::Sub).i32().Inputs(120, 104);
            INST(108, Opcode::Add).i32().Inputs(104, 5);
            INST(106, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(108, 3);
            INST(107, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(106);
        }

        BASIC_BLOCK(42, 21, 50)
        {
            INST(56, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(57, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(56);
        }

        BASIC_BLOCK(50, 21, 50)
        {
            INST(58, Opcode::Phi).i32().Inputs(4, 62);
            INST(59, Opcode::Phi).i32().Inputs(5, 75);
            INST(72, Opcode::Add).i32().Inputs(58, 59);
            INST(66, Opcode::Add).i32().Inputs(72, 37);
            INST(75, Opcode::Sub).i32().Inputs(66, 58);
            INST(62, Opcode::Add).i32().Inputs(58, 5);
            INST(60, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(62, 3);
            INST(61, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(60);
        }

        BASIC_BLOCK(71, 15, 57)
        {
            INST(101, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(57, 21, 66)
        {
            INST(79, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(80, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(79);
        }

        BASIC_BLOCK(66, 21, 66)
        {
            INST(81, Opcode::Phi).i32().Inputs(4, 85);
            INST(82, Opcode::Phi).i32().Inputs(5, 97);
            INST(96, Opcode::Add).i32().Inputs(82, 81);
            INST(89, Opcode::Add).i32().Inputs(96, 37);
            INST(97, Opcode::Sub).i32().Inputs(89, 81);
            INST(85, Opcode::Add).i32().Inputs(81, 5);
            INST(83, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(85, 3);
            INST(84, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(83);
        }

        BASIC_BLOCK(15, 21, 4)
        {
            INST(50, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(51, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(50);
        }

        BASIC_BLOCK(4, 21, 4)
        {
            INST(16, Opcode::Phi).i32().Inputs(4, 48);
            INST(17, Opcode::Phi).i32().Inputs(5, 36);
            INST(28, Opcode::Add).i32().Inputs(16, 17);
            INST(33, Opcode::Add).i32().Inputs(28, 16);
            INST(36, Opcode::Add).i32().Inputs(33, 37);
            INST(48, Opcode::Add).i32().Inputs(16, 5);
            INST(24, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48, 3);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(21, -1)
        {
            INST(77, Opcode::Phi)
                .i32()
                .Inputs({{15, 5}, {42, 5}, {57, 5}, {4, 36}, {66, 97}, {72, 5}, {50, 75}, {77, 121}});
            INST(49, Opcode::Return).i32().Inputs(77);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopUnswitchTest, TestMultipleConditionsLimitLevel)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(12, 15).i64();
        CONSTANT(37, 2).i64();

        BASIC_BLOCK(20, 15)
        {
            INST(8, Opcode::Load).i64().Inputs(1, 4);
            INST(11, Opcode::Load).i64().Inputs(1, 5);
            INST(43, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11, 12);
            INST(38, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8, 12);
            INST(31, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(11, 12);
            INST(26, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8, 12);
        }

        BASIC_BLOCK(15, 14, 11)
        {
            INST(50, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(51, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(50);
        }

        BASIC_BLOCK(11, 2)
        {
            INST(16, Opcode::Phi).i32().Inputs(4, 48);
            INST(17, Opcode::Phi).i32().Inputs(5, 46);
        }

        BASIC_BLOCK(2, 3, 4)
        {
            INST(27, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(28, Opcode::Add).i32().Inputs(16, 17);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(29, Opcode::Phi).i32().Inputs(17, 28);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(31);
        }

        BASIC_BLOCK(6, 5)
        {
            INST(33, Opcode::Add).i32().Inputs(29, 16);
        }

        BASIC_BLOCK(5, 7, 8)
        {
            INST(34, Opcode::Phi).i32().Inputs(29, 33);
            INST(36, Opcode::Add).i32().Inputs(34, 37);
            INST(39, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(8, 7)
        {
            INST(40, Opcode::Sub).i32().Inputs(36, 16);
        }

        BASIC_BLOCK(7, 9, 10)
        {
            INST(41, Opcode::Phi).i32().Inputs(36, 40);
            INST(44, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(10, 9)
        {
            INST(45, Opcode::Sub).i32().Inputs(41, 16);
        }

        BASIC_BLOCK(9, 16)
        {
            INST(46, Opcode::Phi).i32().Inputs(41, 45);
            INST(48, Opcode::Add).i32().Inputs(16, 5);
        }

        BASIC_BLOCK(16, 14, 11)
        {
            INST(24, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48, 3);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(24);
        }

        BASIC_BLOCK(14, 21)
        {
            INST(55, Opcode::Phi).i32().Inputs(5, 46);
        }

        BASIC_BLOCK(21, -1)
        {
            INST(49, Opcode::Return).i32().Inputs(55);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(1, 100));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(12, 15).i64();
        CONSTANT(37, 2).i64();

        BASIC_BLOCK(20, 15, 42)
        {
            INST(8, Opcode::Load).i64().Inputs(1, 4);
            INST(11, Opcode::Load).i64().Inputs(1, 5);
            INST(43, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11, 12);
            INST(38, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8, 12);
            INST(26, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8, 12);
            INST(78, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(42, 21, 49)
        {
            INST(56, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(57, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(56);
        }

        BASIC_BLOCK(49, 47, 50)
        {
            INST(58, Opcode::Phi).i32().Inputs(4, 62);
            INST(59, Opcode::Phi).i32().Inputs(5, 75);
            INST(71, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(50, 47)
        {
            INST(72, Opcode::Add).i32().Inputs(58, 59);
        }

        BASIC_BLOCK(47, 53, 52)
        {
            INST(70, Opcode::Phi).i32().Inputs(59, 72);
            INST(66, Opcode::Add).i32().Inputs(70, 37);
            INST(67, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(52, 53)
        {
            INST(74, Opcode::Sub).i32().Inputs(66, 58);
        }

        BASIC_BLOCK(53, 21, 49)
        {
            INST(65, Opcode::Phi).i32().Inputs(66, 74);
            INST(75, Opcode::Sub).i32().Inputs(65, 58);
            INST(62, Opcode::Add).i32().Inputs(58, 5);
            INST(60, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(62, 3);
            INST(61, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(60);
        }

        BASIC_BLOCK(15, 21, 2)
        {
            INST(50, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(51, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(50);
        }

        BASIC_BLOCK(2, 6, 4)
        {
            INST(16, Opcode::Phi).i32().Inputs(4, 48);
            INST(17, Opcode::Phi).i32().Inputs(5, 41);
            INST(27, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(26);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(28, Opcode::Add).i32().Inputs(16, 17);
        }

        BASIC_BLOCK(6, 9, 8)
        {
            INST(29, Opcode::Phi).i32().Inputs(17, 28);
            INST(33, Opcode::Add).i32().Inputs(29, 16);
            INST(36, Opcode::Add).i32().Inputs(33, 37);
            INST(39, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(38);
        }

        BASIC_BLOCK(8, 9)
        {
            INST(40, Opcode::Sub).i32().Inputs(36, 16);
        }

        BASIC_BLOCK(9, 21, 2)
        {
            INST(41, Opcode::Phi).i32().Inputs(36, 40);
            INST(48, Opcode::Add).i32().Inputs(16, 5);
            INST(24, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48, 3);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(21, -1)
        {
            INST(77, Opcode::Phi).i32().Inputs({{15, 5}, {42, 5}, {9, 41}, {53, 75}});
            INST(49, Opcode::Return).i32().Inputs(77);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopUnswitchTest, TestMultipleConditionsLimitInstructions)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(12, 15).i64();
        CONSTANT(37, 2).i64();

        BASIC_BLOCK(20, 15)
        {
            INST(8, Opcode::Load).i64().Inputs(1, 4);
            INST(11, Opcode::Load).i64().Inputs(1, 5);
            INST(43, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11, 12);
            INST(38, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8, 12);
            INST(31, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(11, 12);
            INST(26, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8, 12);
        }

        BASIC_BLOCK(15, 14, 11)
        {
            INST(50, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(51, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(50);
        }

        BASIC_BLOCK(11, 2)
        {
            INST(16, Opcode::Phi).i32().Inputs(4, 48);
            INST(17, Opcode::Phi).i32().Inputs(5, 46);
        }

        BASIC_BLOCK(2, 3, 4)
        {
            INST(27, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(28, Opcode::Add).i32().Inputs(16, 17);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(29, Opcode::Phi).i32().Inputs(17, 28);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(31);
        }

        BASIC_BLOCK(6, 5)
        {
            INST(33, Opcode::Add).i32().Inputs(29, 16);
        }

        BASIC_BLOCK(5, 7, 8)
        {
            INST(34, Opcode::Phi).i32().Inputs(29, 33);
            INST(36, Opcode::Add).i32().Inputs(34, 37);
            INST(39, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(8, 7)
        {
            INST(40, Opcode::Sub).i32().Inputs(36, 16);
        }

        BASIC_BLOCK(7, 9, 10)
        {
            INST(41, Opcode::Phi).i32().Inputs(36, 40);
            INST(44, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(10, 9)
        {
            INST(45, Opcode::Sub).i32().Inputs(41, 16);
        }

        BASIC_BLOCK(9, 16)
        {
            INST(46, Opcode::Phi).i32().Inputs(41, 45);
            INST(48, Opcode::Add).i32().Inputs(16, 5);
        }

        BASIC_BLOCK(16, 14, 11)
        {
            INST(24, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48, 3);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(24);
        }

        BASIC_BLOCK(14, 21)
        {
            INST(55, Opcode::Phi).i32().Inputs(5, 46);
        }

        BASIC_BLOCK(21, -1)
        {
            INST(49, Opcode::Return).i32().Inputs(55);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2, 40));
    ASSERT_TRUE(GetGraph()->RunPass<BranchElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1, 1).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();
        CONSTANT(12, 15).i64();
        CONSTANT(37, 2).i64();

        BASIC_BLOCK(20, 71, 42)
        {
            INST(8, Opcode::Load).i64().Inputs(1, 4);
            INST(11, Opcode::Load).i64().Inputs(1, 5);
            INST(43, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(11, 12);
            INST(38, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(8, 12);
            INST(26, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LE).Inputs(8, 12);
            INST(78, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(43);
        }

        BASIC_BLOCK(42, 21, 49)
        {
            INST(56, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(57, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(56);
        }

        BASIC_BLOCK(49, 47, 50)
        {
            INST(58, Opcode::Phi).i32().Inputs(4, 62);
            INST(59, Opcode::Phi).i32().Inputs(5, 75);
            INST(71, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(50, 47)
        {
            INST(72, Opcode::Add).i32().Inputs(58, 59);
        }

        BASIC_BLOCK(47, 53, 52)
        {
            INST(70, Opcode::Phi).i32().Inputs(59, 72);
            INST(66, Opcode::Add).i32().Inputs(70, 37);
            INST(67, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(52, 53)
        {
            INST(74, Opcode::Sub).i32().Inputs(66, 58);
        }

        BASIC_BLOCK(53, 21, 49)
        {
            INST(65, Opcode::Phi).i32().Inputs(66, 74);
            INST(75, Opcode::Sub).i32().Inputs(65, 58);
            INST(62, Opcode::Add).i32().Inputs(58, 5);
            INST(60, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(62, 3);
            INST(61, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(60);
        }

        BASIC_BLOCK(71, 15, 57)
        {
            INST(101, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(38);
        }

        BASIC_BLOCK(57, 21, 66)
        {
            INST(79, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(80, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(79);
        }

        BASIC_BLOCK(66, 21, 66)
        {
            INST(81, Opcode::Phi).i32().Inputs(4, 85);
            INST(82, Opcode::Phi).i32().Inputs(5, 97);
            INST(96, Opcode::Add).i32().Inputs(82, 81);
            INST(89, Opcode::Add).i32().Inputs(96, 37);
            INST(97, Opcode::Sub).i32().Inputs(89, 81);
            INST(85, Opcode::Add).i32().Inputs(81, 5);
            INST(83, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(85, 3);
            INST(84, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(83);
        }

        BASIC_BLOCK(15, 21, 4)
        {
            INST(50, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(4, 3);
            INST(51, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(50);
        }

        BASIC_BLOCK(4, 21, 4)
        {
            INST(16, Opcode::Phi).i32().Inputs(4, 48);
            INST(17, Opcode::Phi).i32().Inputs(5, 36);
            INST(28, Opcode::Add).i32().Inputs(16, 17);
            INST(33, Opcode::Add).i32().Inputs(28, 16);
            INST(36, Opcode::Add).i32().Inputs(33, 37);
            INST(48, Opcode::Add).i32().Inputs(16, 5);
            INST(24, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(48, 3);
            INST(25, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(21, -1)
        {
            INST(77, Opcode::Phi).i32().Inputs({{15, 5}, {42, 5}, {53, 75}, {57, 5}, {4, 36}, {66, 97}});
            INST(49, Opcode::Return).i32().Inputs(77);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopInc0)
{
    CreateIncLoopGraph(0);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopInc1)
{
    CreateIncLoopGraph(1);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopInc2)
{
    CreateIncLoopGraph(2);
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopDec0)
{
    CreateDecLoopGraph(0);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopDec1)
{
    CreateDecLoopGraph(1);
    ASSERT_FALSE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
}

TEST_F(LoopUnswitchTest, TestNoUnswitchSmallIterationLoopDec2)
{
    CreateDecLoopGraph(2);
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnswitch>(2, 100));
}

class ConditionComparatorTest : public GraphTest {
public:
    ConditionComparatorTest()
    {
        GetGraph()->CreateStartBlock();
        first_input = GetGraph()->FindOrCreateConstant(10);
        second_input = GetGraph()->FindOrCreateConstant(20);
        third_input = GetGraph()->FindOrCreateConstant(30);
    }
    Inst *CreateInstIfImm(Inst *input, uint64_t imm, ConditionCode cc)
    {
        auto inst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, input, imm, input->GetType(), cc);
        return inst;
    }
    Inst *CreateInstIfImm(Inst *input0, Inst *input1, ConditionCode compare_cc, uint64_t imm, ConditionCode if_cc)
    {
        auto compare_inst =
            GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, input0, input1, input0->GetType(), compare_cc);
        auto inst = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, INVALID_PC, compare_inst, imm,
                                                compare_inst->GetType(), if_cc);
        return inst;
    }

    Inst *first_input;   // NOLINT(misc-non-private-member-variables-in-classes)
    Inst *second_input;  // NOLINT(misc-non-private-member-variables-in-classes)
    Inst *third_input;   // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ConditionComparatorTest, TestNoCompareInput)
{
    {
        auto if_imm0 = CreateInstIfImm(first_input, 10, ConditionCode::CC_EQ);
        auto if_imm1 = CreateInstIfImm(first_input, 10, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, 1, ConditionCode::CC_EQ);
        auto if_imm1 = CreateInstIfImm(second_input, 1, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, 1, ConditionCode::CC_EQ);
        auto if_imm1 = CreateInstIfImm(first_input, 1, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, 0, ConditionCode::CC_EQ);
        auto if_imm1 = CreateInstIfImm(first_input, 1, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, 1, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, 1, ConditionCode::CC_EQ);
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, true));
    }
}

TEST_F(ConditionComparatorTest, TestCompareInput)
{
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 10, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 10, ConditionCode::CC_NE);
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 1, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, third_input, ConditionCode::CC_LT, 1, ConditionCode::CC_NE);
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 0, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 0, ConditionCode::CC_NE);
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 0, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_GE, 0, ConditionCode::CC_NE);
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, true));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 0, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 0, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, true));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_GE, 0, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 0, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, false));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, true));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_GE, 0, ConditionCode::CC_NE);
        auto if_imm1 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_LT, 1, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, true));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
    }
    {
        auto if_imm0 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_GE, 0, ConditionCode::CC_EQ);
        auto if_imm1 = CreateInstIfImm(first_input, second_input, ConditionCode::CC_GE, 1, ConditionCode::CC_EQ);
        ASSERT_TRUE(IsConditionEqual(if_imm0, if_imm1, true));
        ASSERT_FALSE(IsConditionEqual(if_imm0, if_imm1, false));
    }
}
// NOLINTEND(readability-magic-numbers,readability-function-size)
}  // namespace panda::compiler
