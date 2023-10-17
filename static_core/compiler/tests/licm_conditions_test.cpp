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
#include "optimizer/optimizations/licm_conditions.h"

namespace panda::compiler {
class LicmConditionsTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers,readability-function-size)

/// Graph is similar to TestUpdatePhiCorrectPredOrder but 14p prevents hoisting
TEST_F(LicmConditionsTest, TestPhiPreventsConditionHoisting)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(10, 3, 4)
        {
            INST(18, Opcode::Phi).i64().Inputs(4, 17);
            INST(6, Opcode::Phi).i64().Inputs(4, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(4, 24, 27)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(28, 24)
        {
            // anything
        }

        BASIC_BLOCK(27, 28, 24)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(24, 10)
        {
            INST(14, Opcode::Phi).i64().Inputs(4, 18, 5);
            INST(15, Opcode::Add).i64().Inputs(6, 5);
            INST(17, Opcode::Add).i64().Inputs(14, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<LicmConditions>());
}

TEST_F(LicmConditionsTest, TestConditionIsNotHoistable)
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
        BASIC_BLOCK(6, 5, 7)
        {
            INST(16, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 2);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(16);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(9, Opcode::Add).i64().Inputs(5, 6);
        }
        BASIC_BLOCK(7, 8)
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
    ASSERT_FALSE(GetGraph()->RunPass<LicmConditions>());
}

/*
 * Test graph:
 * - Loop contains condition combination BB4 and BB6.
 * - Branches BB5 and BB7 do not contain phis.
 *     /-----[2]<----------\
 *     |      |            |
 *     |      v            |
 *     |     [4]------\    |
 *     |      |       |    |
 *     |      |       v    |
 *     |      |<-----[6]   |
 *     |      |       |    |
 *     |      v       v    |
 *     |     [5]     [7]   |
 *     |      |       |    |
 *     |      \->[8]<-/    |
 *     |          |        |
 *     |          \--------/
 *     |
 *     \----->[3]
 */
TEST_F(LicmConditionsTest, TestBrachWithoutPhi)
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
        BASIC_BLOCK(6, 5, 7)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(9, Opcode::Add).i64().Inputs(5, 6);
        }
        BASIC_BLOCK(7, 8)
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
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        CONSTANT(2, 100).i64();
        CONSTANT(3, 0).i64();
        CONSTANT(4, 1).i64();

        BASIC_BLOCK(14, 4) {}

        BASIC_BLOCK(4, 16, 6)
        {
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(6, 16, 17)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(17, 16) {}

        BASIC_BLOCK(16, 2)
        {
            INST(16, Opcode::Phi).b().Inputs(4, 4, 3);
        }

        BASIC_BLOCK(2, 3, 15)
        {
            INST(5, Opcode::Phi).i64().Inputs(3, 11);
            INST(6, Opcode::Phi).i64().Inputs(3, 12);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 2);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(15, 5, 7)
        {
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(16);
        }

        BASIC_BLOCK(5, 8)
        {
            INST(9, Opcode::Add).i64().Inputs(5, 6);
        }

        BASIC_BLOCK(7, 8)
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
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LicmConditionsTest, TestExtraInstPreventsHoisting)
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
        BASIC_BLOCK(6, 5, 7)
        {
            INST(16, Opcode::SaveState);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(9, Opcode::Add).i64().Inputs(5, 6);
        }
        BASIC_BLOCK(7, 8)
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
    ASSERT_FALSE(GetGraph()->RunPass<LicmConditions>());
}

/*
 * Test graph:
 * - Loop contains condition combination BB12, BB14 and BB15.
 * - Longest chain should be processed
 */
TEST_F(LicmConditionsTest, TestProcessLongestChain)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).b();
        PARAMETER(3, 3).ptr();
        CONSTANT(4, 100).i64();
        CONSTANT(5, 0).i64();
        CONSTANT(6, 1).i64();

        BASIC_BLOCK(20, 3, 12)
        {
            INST(18, Opcode::Phi).i64().Inputs(5, 17);
            INST(7, Opcode::Phi).i64().Inputs(5, 15);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7, 4);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(12, 13, 14)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(14, 13, 15)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(15, 16, 13)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(13, 17)
        {
            INST(20, Opcode::Add).i64().Inputs(18, 6);
        }

        BASIC_BLOCK(17, 16)
        {
            // anything
        }

        BASIC_BLOCK(16, 20)
        {
            INST(14, Opcode::Phi).i64().Inputs(18, 20);
            INST(15, Opcode::Add).i64().Inputs(7, 6);
            INST(17, Opcode::Add).i64().Inputs(14, 6);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).b();
        PARAMETER(3, 3).ptr();
        CONSTANT(4, 100).i64();
        CONSTANT(5, 0).i64();
        CONSTANT(6, 1).i64();

        BASIC_BLOCK(42, 12) {}

        BASIC_BLOCK(12, 44, 14)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(14, 44, 15)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(15, 45, 44)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(45, 44)
        {
            // empty
        }

        BASIC_BLOCK(44, 20)
        {
            INST(21, Opcode::Phi).b().Inputs(6, 6, 6, 5);
        }

        BASIC_BLOCK(20, 3, 43)
        {
            INST(18, Opcode::Phi).i64().Inputs(5, 17);
            INST(7, Opcode::Phi).i64().Inputs(5, 15);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7, 4);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(43, 13, 16)
        {
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(21);
        }

        BASIC_BLOCK(13, 17)
        {
            INST(20, Opcode::Add).i64().Inputs(18, 6);
        }

        BASIC_BLOCK(17, 16)
        {
            // anything
        }

        BASIC_BLOCK(16, 20)
        {
            INST(14, Opcode::Phi).i64().Inputs(18, 20);
            INST(15, Opcode::Add).i64().Inputs(7, 6);
            INST(17, Opcode::Add).i64().Inputs(14, 6);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB12, BB14 and BB15.
 * - Longest chain should be processed but it has Phi with different inputs in multiple predecessors successor
 */
TEST_F(LicmConditionsTest, TestProcessLongestChainNotSuitable)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).b();
        PARAMETER(3, 3).ptr();
        CONSTANT(4, 100).i64();
        CONSTANT(5, 0).i64();
        CONSTANT(6, 1).i64();

        BASIC_BLOCK(20, 3, 12)
        {
            INST(18, Opcode::Phi).i64().Inputs(5, 17);
            INST(7, Opcode::Phi).i64().Inputs(5, 15);
            INST(23, Opcode::Load).i64().Inputs(3, 5);
            INST(24, Opcode::Load).i64().Inputs(3, 6);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7, 4);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(12, 13, 14)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(14, 13, 15)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(15, 16, 13)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(13, 17)
        {
            INST(21, Opcode::Phi).i64().Inputs(23, 24, 24);
            INST(20, Opcode::Add).i64().Inputs(18, 6);
            INST(22, Opcode::Add).i64().Inputs(20, 21);
        }

        BASIC_BLOCK(17, 16)
        {
            // anything
        }

        BASIC_BLOCK(16, 20)
        {
            INST(14, Opcode::Phi).i64().Inputs(18, 22);
            INST(15, Opcode::Add).i64().Inputs(7, 6);
            INST(17, Opcode::Add).i64().Inputs(14, 6);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).b();
        PARAMETER(3, 3).ptr();
        CONSTANT(4, 100).i64();
        CONSTANT(5, 0).i64();
        CONSTANT(6, 1).i64();

        BASIC_BLOCK(42, 14) {}

        BASIC_BLOCK(14, 44, 15)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(15, 45, 44)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(45, 44)
        {
            // empty
        }

        BASIC_BLOCK(44, 20)
        {
            INST(25, Opcode::Phi).b().Inputs(6, 6, 5);
        }

        BASIC_BLOCK(20, 3, 12)
        {
            INST(18, Opcode::Phi).i64().Inputs(5, 17);
            INST(7, Opcode::Phi).i64().Inputs(5, 15);
            INST(23, Opcode::Load).i64().Inputs(3, 5);
            INST(24, Opcode::Load).i64().Inputs(3, 6);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7, 4);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }

        BASIC_BLOCK(12, 13, 43)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(43, 13, 16)
        {
            INST(26, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(25);
        }

        BASIC_BLOCK(13, 17)
        {
            INST(21, Opcode::Phi).i64().Inputs(23, 24);
            INST(20, Opcode::Add).i64().Inputs(18, 6);
            INST(22, Opcode::Add).i64().Inputs(20, 21);
        }

        BASIC_BLOCK(17, 16)
        {
            // anything
        }

        BASIC_BLOCK(16, 20)
        {
            INST(14, Opcode::Phi).i64().Inputs(18, 22);
            INST(15, Opcode::Add).i64().Inputs(7, 6);
            INST(17, Opcode::Add).i64().Inputs(14, 6);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB2 and BB29.
 * - Block BB2 should be splitted
 * - Branch BB26 contains phi which requires input update
 *     /-----[6]<---------------\
 *     |      |                 |
 *     |      v                 |
 *     |     [2]----------\     |
 *     |      |           |     |
 *     |      |           v     |
 *     |      |<---------[29]   |
 *     |      |           |     |
 *     |      |           v     |
 *     |      |      /---[30]   |
 *     |      |      |     |    |
 *     |      |      v     v    |
 *     |      |    [32]   [31]  |
 *     |      |      |     |    |
 *     |      |------/-----/    |
 *     |      |                 |
 *     |      v                 |
 *     |     [26]               |
 *     |      |                 |
 *     |      \-----------------/
 *     |
 *     \----->[3]
 */
TEST_F(LicmConditionsTest, TestUpdatePhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(6, 3, 2)
        {
            INST(14, Opcode::Phi).i32().Inputs(4, 15);
            INST(6, Opcode::Phi).i64().Inputs(4, 13);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(2, 26, 29)
        {
            INST(9, Opcode::Load).b().Inputs(2, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(29, 30, 26)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(30, 31, 32)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }

        BASIC_BLOCK(31, 26)
        {
            // anything
        }

        BASIC_BLOCK(32, 26)
        {
            // anything
        }

        BASIC_BLOCK(26, 6)
        {
            INST(17, Opcode::Phi).i32().Inputs(14, 14, 9, 9);
            INST(15, Opcode::Add).i32().Inputs(17, 5);
            INST(13, Opcode::Add).i64().Inputs(6, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(16, Opcode::Return).i32().Inputs(14);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(60, 61) {}

        BASIC_BLOCK(61, 63, 29)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(29, 64, 63)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(64, 63)
        {
            // empty
        }

        BASIC_BLOCK(63, 6)
        {
            INST(18, Opcode::Phi).b().Inputs(5, 5, 4);
        }

        BASIC_BLOCK(6, 3, 2)
        {
            INST(14, Opcode::Phi).i32().Inputs(4, 15);
            INST(6, Opcode::Phi).i64().Inputs(4, 13);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(2, 62)
        {
            INST(9, Opcode::Load).b().Inputs(2, 4);
        }

        BASIC_BLOCK(62, 26, 30)
        {
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(18);
        }

        BASIC_BLOCK(30, 31, 32)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }

        BASIC_BLOCK(31, 26)
        {
            // anything
        }

        BASIC_BLOCK(32, 26)
        {
            // anything
        }

        BASIC_BLOCK(26, 6)
        {
            INST(17, Opcode::Phi).i32().Inputs(14, 9, 9);
            INST(15, Opcode::Add).i32().Inputs(17, 5);
            INST(13, Opcode::Add).i64().Inputs(6, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(16, Opcode::Return).i32().Inputs(14);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB50 and BB52.
 * - Branch BB51 contains phi which is hoisted
 */
TEST_F(LicmConditionsTest, TestHoistPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(53, 3, 2)
        {
            INST(18, Opcode::Phi).i64().Inputs(4, 17);
            INST(6, Opcode::Phi).i64().Inputs(4, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(2, 46, 50)
        {
            INST(9, Opcode::Load).b().Inputs(2, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }

        BASIC_BLOCK(50, 51, 52)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(52, 46, 51)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(51, 46)
        {
            INST(13, Opcode::Phi).i64().Inputs(5, 4);
        }

        BASIC_BLOCK(46, 53)
        {
            INST(14, Opcode::Phi).i64().Inputs(4, 5, 13);
            INST(15, Opcode::Add).i64().Inputs(6, 5);
            INST(17, Opcode::Add).i64().Inputs(14, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(16, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(108, 50) {}

        BASIC_BLOCK(50, 110, 52)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(52, 111, 110)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(111, 110)
        {
            // empty
        }

        BASIC_BLOCK(110, 53)
        {
            INST(13, Opcode::Phi).i64().Inputs(5, 4, 4);
            INST(19, Opcode::Phi).b().Inputs(5, 5, 4);
        }

        BASIC_BLOCK(53, 3, 2)
        {
            INST(18, Opcode::Phi).i64().Inputs(4, 17);
            INST(6, Opcode::Phi).i64().Inputs(4, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(2, 46, 109)
        {
            INST(9, Opcode::Load).b().Inputs(2, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }

        BASIC_BLOCK(109, 51, 46)
        {
            INST(20, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }

        BASIC_BLOCK(51, 46) {}

        BASIC_BLOCK(46, 53)
        {
            INST(14, Opcode::Phi).i64().Inputs(4, 5, 13);
            INST(15, Opcode::Add).i64().Inputs(6, 5);
            INST(17, Opcode::Add).i64().Inputs(14, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(16, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB50 and BB52.
 * - Branch BB51 contains phi 13 which cannot be hoist (input is not from chain)
 *   but we can update inputs
 */
TEST_F(LicmConditionsTest, TestCannotHoistPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(54, 53)
        {
            INST(20, Opcode::Load).b().Inputs(2, 5);
        }

        BASIC_BLOCK(53, 3, 2)
        {
            INST(18, Opcode::Phi).i32().Inputs(4, 17);
            INST(6, Opcode::Phi).i32().Inputs(4, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(2, 51, 50)
        {
            INST(9, Opcode::Load).b().Inputs(2, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }

        BASIC_BLOCK(50, 51, 52)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(52, 46, 51)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(51, 46)
        {
            INST(13, Opcode::Phi).i32().Inputs(20, 5, 5);
        }

        BASIC_BLOCK(46, 53)
        {
            INST(14, Opcode::Phi).i32().Inputs(5, 13);
            INST(15, Opcode::Add).i32().Inputs(6, 5);
            INST(17, Opcode::Add).i32().Inputs(14, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(16, Opcode::Return).i32().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(54, 50)
        {
            INST(20, Opcode::Load).b().Inputs(2, 5);
        }

        BASIC_BLOCK(50, 111, 52)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(52, 112, 111)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(112, 111)
        {
            // empty
        }

        BASIC_BLOCK(111, 53)
        {
            INST(21, Opcode::Phi).b().Inputs(5, 5, 4);
        }

        BASIC_BLOCK(53, 3, 2)
        {
            INST(18, Opcode::Phi).i32().Inputs(4, 17);
            INST(6, Opcode::Phi).i32().Inputs(4, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(2, 51, 110)
        {
            INST(9, Opcode::Load).b().Inputs(2, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }

        BASIC_BLOCK(110, 51, 46)
        {
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(21);
        }

        BASIC_BLOCK(51, 46)
        {
            INST(13, Opcode::Phi).i32().Inputs(20, 5);
        }

        BASIC_BLOCK(46, 53)
        {
            INST(14, Opcode::Phi).i32().Inputs(5, 13);
            INST(15, Opcode::Add).i32().Inputs(6, 5);
            INST(17, Opcode::Add).i32().Inputs(14, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(16, Opcode::Return).i32().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - Loop contains condition combination BB4 and BB27.
 * - Branch BB24 contains phi which requires new edge in correct predecessors order
 */
TEST_F(LicmConditionsTest, TestHoistPhiCorrectPredOrder)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(10, 3, 4)
        {
            INST(18, Opcode::Phi).i64().Inputs(4, 17);
            INST(6, Opcode::Phi).i64().Inputs(4, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(28, 24)
        {
            // anything
        }

        BASIC_BLOCK(4, 24, 27)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(27, 28, 24)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(24, 10)
        {
            INST(14, Opcode::Phi).i64().Inputs(3, 4, 5);
            INST(15, Opcode::Add).i64().Inputs(6, 5);
            INST(17, Opcode::Add).i64().Inputs(14, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).ptr();
        CONSTANT(3, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(58, 4) {}

        BASIC_BLOCK(4, 60, 27)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }

        BASIC_BLOCK(61, 60)
        {
            // empty
        }

        BASIC_BLOCK(27, 61, 60)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(60, 10)
        {
            INST(14, Opcode::Phi).i64().Inputs(4, 3, 5);
            INST(20, Opcode::Phi).b().Inputs(5, 4, 5);
        }

        BASIC_BLOCK(10, 3, 59)
        {
            INST(18, Opcode::Phi).i64().Inputs(4, 17);
            INST(6, Opcode::Phi).i64().Inputs(4, 15);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(59, 24, 28)
        {
            INST(21, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(20);
        }

        BASIC_BLOCK(28, 24)
        {
            // anything
        }

        BASIC_BLOCK(24, 10)
        {
            INST(15, Opcode::Add).i64().Inputs(6, 5);
            INST(17, Opcode::Add).i64().Inputs(14, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains with neighbour blocks
 */
TEST_F(LicmConditionsTest, TestProcessNeigbourChains)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        PARAMETER(2, 2).i32();
        PARAMETER(3, 3).i32();
        CONSTANT(20, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(18, Opcode::Phi).i64().Inputs(4, 16);
            INST(6, Opcode::Phi).i64().Inputs(4, 17);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 20);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(4, 6, 5)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, 6, 9)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(11).Inputs(1);
        }

        BASIC_BLOCK(6, 7, 8)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(12).Inputs(2);
        }

        BASIC_BLOCK(7, 8, 9)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(13).Inputs(3);
        }

        BASIC_BLOCK(8, 10)
        {
            INST(13, Opcode::Sub).i64().Inputs(18, 5);
        }

        BASIC_BLOCK(9, 10)
        {
            INST(14, Opcode::Add).i64().Inputs(18, 5);
        }

        BASIC_BLOCK(10, 2)
        {
            INST(15, Opcode::Phi).i64().Inputs(13, 14);
            INST(16, Opcode::Add).i64().Inputs(15, 5);
            INST(17, Opcode::Add).i64().Inputs(6, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        PARAMETER(2, 2).i32();
        PARAMETER(3, 3).i32();
        CONSTANT(20, 100).i64();
        CONSTANT(4, 0).i64();
        CONSTANT(5, 1).i64();

        BASIC_BLOCK(14, 6) {}

        BASIC_BLOCK(6, 7, 16)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(12).Inputs(2);
        }

        BASIC_BLOCK(7, 16, 17)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(13).Inputs(3);
        }

        BASIC_BLOCK(17, 16)
        {
            // empty
        }

        BASIC_BLOCK(16, 4)
        {
            INST(21, Opcode::Phi).b().Inputs(5, 5, 4);
        }

        BASIC_BLOCK(4, 19, 5)
        {
            INST(9, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(10).Inputs(0);
        }

        BASIC_BLOCK(5, 19, 20)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(11).Inputs(1);
        }

        BASIC_BLOCK(20, 19)
        {
            // empty
        }

        BASIC_BLOCK(19, 2)
        {
            INST(23, Opcode::Phi).b().Inputs(5, 5, 4);
        }

        BASIC_BLOCK(2, 3, 18)
        {
            INST(18, Opcode::Phi).i64().Inputs(4, 16);
            INST(6, Opcode::Phi).i64().Inputs(4, 17);
            INST(7, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(6, 20);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(18, 15, 9)
        {
            INST(24, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(23);
        }

        BASIC_BLOCK(15, 8, 9)
        {
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(21);
        }

        BASIC_BLOCK(8, 10)
        {
            INST(13, Opcode::Sub).i64().Inputs(18, 5);
        }

        BASIC_BLOCK(9, 10)
        {
            INST(14, Opcode::Add).i64().Inputs(18, 5);
        }

        BASIC_BLOCK(10, 2)
        {
            INST(15, Opcode::Phi).i64().Inputs(13, 14);
            INST(16, Opcode::Add).i64().Inputs(15, 5);
            INST(17, Opcode::Add).i64().Inputs(6, 5);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(19, Opcode::Return).i64().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains with common successors
 */
TEST_F(LicmConditionsTest, TestProcessChainsWithCommonSuccessors)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).b();
        PARAMETER(3, 3).ptr();
        CONSTANT(4, 100).i64();
        CONSTANT(5, 0).i64();
        CONSTANT(6, 1).i64();

        BASIC_BLOCK(4, 3, 5)
        {
            INST(19, Opcode::Phi).i64().Inputs(5, 18);
            INST(7, Opcode::Phi).i64().Inputs(5, 17);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7, 4);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, 6, 10)
        {
            INST(23, Opcode::Load).i64().Inputs(3, 5);
            INST(24, Opcode::Load).i64().Inputs(3, 6);
            INST(25, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0).Inputs(23);
        }
        BASIC_BLOCK(6, 8, 7)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(7, 8, 9)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(10, 8, 11)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(11, 8, 9)
        {
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(8, 12)
        {
            INST(14, Opcode::Phi).i64().Inputs(23, 23, 24, 24);
        }
        BASIC_BLOCK(9, 12)
        {
            INST(15, Opcode::Phi).i64().Inputs(24, 23);
        }
        BASIC_BLOCK(12, 4)
        {
            INST(16, Opcode::Phi).i64().Inputs(14, 15);
            INST(17, Opcode::Add).i64().Inputs(7, 6);
            INST(18, Opcode::Add).i64().Inputs(16, 6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(20, Opcode::Return).i64().Inputs(19);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();
        PARAMETER(2, 2).b();
        PARAMETER(3, 3).ptr();
        CONSTANT(4, 100).i64();
        CONSTANT(5, 0).i64();
        CONSTANT(6, 1).i64();

        BASIC_BLOCK(22, 6) {}
        BASIC_BLOCK(6, 24, 7)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(7, 24, 25)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(25, 24)
        {
            // empty
        }
        BASIC_BLOCK(24, 10)
        {
            INST(26, Opcode::Phi).b().Inputs(6, 6, 5);
        }
        BASIC_BLOCK(10, 27, 11)
        {
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(11, 27, 28)
        {
            INST(13, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(28, 27)
        {
            // empty
        }
        BASIC_BLOCK(27, 4)
        {
            INST(28, Opcode::Phi).b().Inputs(6, 6, 5);
        }
        BASIC_BLOCK(4, 3, 5)
        {
            INST(19, Opcode::Phi).i64().Inputs(5, 18);
            INST(7, Opcode::Phi).i64().Inputs(5, 17);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_GE).Inputs(7, 4);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(5, 23, 26)
        {
            INST(23, Opcode::Load).i64().Inputs(3, 5);
            INST(24, Opcode::Load).i64().Inputs(3, 6);
            INST(25, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0).Inputs(23);
        }
        BASIC_BLOCK(26, 8, 9)
        {
            INST(29, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(28);
        }
        BASIC_BLOCK(23, 8, 9)
        {
            INST(27, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }
        BASIC_BLOCK(8, 12)
        {
            INST(14, Opcode::Phi).i64().Inputs(24, 23);
        }
        BASIC_BLOCK(9, 12)
        {
            INST(15, Opcode::Phi).i64().Inputs(23, 24);
        }
        BASIC_BLOCK(12, 4)
        {
            INST(16, Opcode::Phi).i64().Inputs(14, 15);
            INST(17, Opcode::Add).i64().Inputs(7, 6);
            INST(18, Opcode::Add).i64().Inputs(16, 6);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(20, Opcode::Return).i64().Inputs(19);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains can be merged
 */
TEST_F(LicmConditionsTest, TestMergeChains)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        CONSTANT(3, 6).i64();
        CONSTANT(4, 5).i64();
        CONSTANT(5, 100).i64();
        CONSTANT(6, 0).i64();
        CONSTANT(7, 1).i64();
        CONSTANT(34, 2).i64();

        BASIC_BLOCK(11, 9)
        {
            INST(37, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1, 3);
            INST(26, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0, 4);
            INST(28, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1, 3);
        }

        BASIC_BLOCK(9, 10, 2)
        {
            INST(17, Opcode::Phi).i32().Inputs(6, 43);
            INST(18, Opcode::Phi).i32().Inputs(7, 41);
            INST(24, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17, 5);
            INST(25, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(2, 3, 4)
        {
            INST(27, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(4, 3, 5)
        {
            INST(29, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(28);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(30, Opcode::Add).i32().Inputs(17, 18);
        }

        BASIC_BLOCK(3, 6, 7)
        {
            INST(31, Opcode::Phi).i32().Inputs(18, 18, 30);
            INST(33, Opcode::Add).i32().Inputs(31, 34);
            INST(36, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(7, 8, 6)
        {
            INST(38, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(37);
        }

        BASIC_BLOCK(6, 8)
        {
            INST(40, Opcode::Sub).i32().Inputs(33, 17);
        }

        BASIC_BLOCK(8, 9)
        {
            INST(41, Opcode::Phi).i32().Inputs(33, 40);
            INST(43, Opcode::Add).i32().Inputs(17, 7);
        }

        BASIC_BLOCK(10, -1)
        {
            INST(20, Opcode::Return).i32().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        CONSTANT(3, 6).i64();
        CONSTANT(4, 5).i64();
        CONSTANT(5, 100).i64();
        CONSTANT(6, 0).i64();
        CONSTANT(7, 1).i64();
        CONSTANT(34, 2).i64();

        BASIC_BLOCK(11, 29, 7)
        {
            INST(37, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1, 3);
            INST(26, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0, 4);
            INST(36, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(7, 27, 29)
        {
            INST(38, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(37);
        }

        BASIC_BLOCK(27, 29)
        {
            // empty
        }

        BASIC_BLOCK(29, 9)
        {
            INST(44, Opcode::Phi).b().Inputs(7, 7, 6);
        }

        BASIC_BLOCK(9, 10, 28)
        {
            INST(17, Opcode::Phi).i32().Inputs(6, 43);
            INST(18, Opcode::Phi).i32().Inputs(7, 41);
            INST(24, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17, 5);
            INST(25, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(28, 5, 3)
        {
            INST(47, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(44);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(30, Opcode::Add).i32().Inputs(17, 18);
        }

        BASIC_BLOCK(3, 6, 8)
        {
            INST(31, Opcode::Phi).i32().Inputs(18, 30);
            INST(33, Opcode::Add).i32().Inputs(31, 34);
            INST(45, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(44);
        }

        BASIC_BLOCK(6, 8)
        {
            INST(40, Opcode::Sub).i32().Inputs(33, 17);
        }

        BASIC_BLOCK(8, 9)
        {
            INST(41, Opcode::Phi).i32().Inputs(33, 40);
            INST(43, Opcode::Add).i32().Inputs(17, 7);
        }

        BASIC_BLOCK(10, -1)
        {
            INST(20, Opcode::Return).i32().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/*
 * Test graph:
 * - two chains can be merged
 */
TEST_F(LicmConditionsTest, TestMergeChainsPhiHoisted)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        CONSTANT(3, 6).i64();
        CONSTANT(4, 5).i64();
        CONSTANT(5, 100).i64();
        CONSTANT(6, 0).i64();
        CONSTANT(7, 1).i64();
        CONSTANT(34, 2).i64();

        BASIC_BLOCK(11, 9)
        {
            INST(37, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1, 3);
            INST(26, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0, 4);
            INST(28, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1, 3);
        }

        BASIC_BLOCK(9, 10, 2)
        {
            INST(17, Opcode::Phi).i32().Inputs(6, 43);
            INST(18, Opcode::Phi).i32().Inputs(7, 41);
            INST(24, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17, 5);
            INST(25, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(2, 3, 4)
        {
            INST(27, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(28);
        }

        BASIC_BLOCK(4, 3, 5)
        {
            INST(29, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(30, Opcode::Add).i32().Inputs(17, 18);
        }

        BASIC_BLOCK(3, 6, 7)
        {
            INST(31, Opcode::Phi).i32().Inputs(18, 18, 30);
            INST(44, Opcode::Phi).i32().Inputs(34, 7, 4);
            INST(33, Opcode::Add).i32().Inputs(31, 44);
            INST(36, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(7, 8, 6)
        {
            INST(38, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(37);
        }

        BASIC_BLOCK(6, 8)
        {
            INST(40, Opcode::Sub).i32().Inputs(33, 17);
        }

        BASIC_BLOCK(8, 9)
        {
            INST(41, Opcode::Phi).i32().Inputs(33, 40);
            INST(43, Opcode::Add).i32().Inputs(17, 7);
        }

        BASIC_BLOCK(10, -1)
        {
            INST(20, Opcode::Return).i32().Inputs(18);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();
        CONSTANT(3, 6).i64();
        CONSTANT(4, 5).i64();
        CONSTANT(5, 100).i64();
        CONSTANT(6, 0).i64();
        CONSTANT(7, 1).i64();
        CONSTANT(34, 2).i64();

        BASIC_BLOCK(11, 2, 7)
        {
            INST(37, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GT).Inputs(1, 3);
            INST(26, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(0, 4);
            INST(28, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(1, 3);
            INST(36, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(7, 27, 2)
        {
            INST(38, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(37);
        }

        BASIC_BLOCK(27, 2)
        {
            // empty
        }

        BASIC_BLOCK(2, 29, 4)
        {
            INST(45, Opcode::Phi).b().Inputs(7, 7, 6);
            INST(27, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(28);
        }

        BASIC_BLOCK(4, 29, 30)
        {
            INST(29, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(26);
        }

        BASIC_BLOCK(30, 29)
        {
            // empty
        }

        BASIC_BLOCK(29, 9)
        {
            INST(44, Opcode::Phi).i32().Inputs(34, 7, 4);
        }

        BASIC_BLOCK(9, 10, 28)
        {
            INST(17, Opcode::Phi).i32().Inputs(6, 43);
            INST(18, Opcode::Phi).i32().Inputs(7, 41);
            INST(24, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(17, 5);
            INST(25, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(24);
        }

        BASIC_BLOCK(28, 5, 3)
        {
            INST(47, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(45);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(30, Opcode::Add).i32().Inputs(17, 18);
        }

        BASIC_BLOCK(3, 6, 8)
        {
            INST(31, Opcode::Phi).i32().Inputs(18, 30);
            INST(33, Opcode::Add).i32().Inputs(31, 44);
            INST(46, Opcode::IfImm).SrcType(DataType::Type::BOOL).CC(CC_NE).Imm(0).Inputs(45);
        }

        BASIC_BLOCK(6, 8)
        {
            INST(40, Opcode::Sub).i32().Inputs(33, 17);
        }

        BASIC_BLOCK(8, 9)
        {
            INST(41, Opcode::Phi).i32().Inputs(33, 40);
            INST(43, Opcode::Add).i32().Inputs(17, 7);
        }

        BASIC_BLOCK(10, -1)
        {
            INST(20, Opcode::Return).i32().Inputs(18);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers,readability-function-size)
}  // namespace panda::compiler
