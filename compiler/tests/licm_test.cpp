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
#include "optimizer/optimizations/licm.h"

namespace panda::compiler {
class LicmTest : public GraphTest {
public:
    static constexpr uint32_t HOST_LIMIT = 8;
};

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *        /-----[2]--------\
 *        |      ^         |
 *        |      |         |
 *        |     [6]        |
 *        |      ^         |
 *        |      |         |
 *        |     [5]------->|
 *        |      ^         |
 *        |      |         |
 *        |     [4]        |
 *        |      ^         |
 *        |      |         |
 *        \---->[3]        |
 *               |         |
 *               v         |
 *             [exit]<-----/
 */

TEST_F(LicmTest, LoopExits)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, 3, 7)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4, 7)
        {
            INST(4, Opcode::Compare).b().Inputs(0, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6, 7)
        {
            INST(6, Opcode::Compare).b().Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(6, 2) {}
        BASIC_BLOCK(7, -1)
        {
            INST(8, Opcode::ReturnVoid);
        }
    }

    auto licm = Licm(GetGraph(), HOST_LIMIT);
    licm.RunImpl();

    ASSERT_TRUE(licm.IsBlockLoopExit(&BB(3)));
    ASSERT_TRUE(licm.IsBlockLoopExit(&BB(5)));

    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(0)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(1)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(2)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(4)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(6)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(7)));
}

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *        /-----[2]<----\
 *        |      |      |
 *        |      v      |
 *        |     [3]-----/
 *        |
 *        \---->[4]
 *               |
 *               v
 *             [exit]
 */
TEST_F(LicmTest, OneLoop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        CONSTANT(1, 10);
        CONSTANT(2, 20);
        CONSTANT(12, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {3, 7}});
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 8}});
            INST(5, Opcode::Compare).b().Inputs(4, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::Mul).u64().Inputs(3, 4);
            INST(13, Opcode::Mul).u64().Inputs(12, 0);
            INST(8, Opcode::Sub).u64().Inputs(4, 13);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(15, Opcode::CallStatic).u64().InputsAutoType(3, 4, 20);
            INST(11, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<Licm>(HOST_LIMIT);
    ASSERT_EQ(INS(13).GetBasicBlock(), BB(3).GetLoop()->GetPreHeader());
    ASSERT_EQ(INS(7).GetBasicBlock(), &BB(3));
    ASSERT_EQ(INS(8).GetBasicBlock(), &BB(3));

    GraphChecker(GetGraph()).Check();
}

/*
 * TODO (a.popov) Improve Licm to support this test with updated DF: `INST(19, Opcode::Phi).u64().Inputs(1, 18)`
 *
 * Test Graph:
 *
 *              [0]
 *               |
 *               v
 *              [2]<----------\
 *               |            |
 *               v            |
 *        /-----[3]<----\     |
 *        |      |      |     |
 *        |      v      |     |
 *        |     [4]-----/    [6]
 *        |                   |
 *        \---->[5]-----------/
 *               |
 *               v
 *             [exit]
 */
TEST_F(LicmTest, DISABLED_TwoLoops)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 10).u64();
        PARAMETER(3, 100).u64();
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 4, 5)
        {
            INST(5, Opcode::Phi).u64().Inputs(2, 9);
            INST(6, Opcode::Phi).u64().Inputs(3, 11);
            INST(19, Opcode::Phi).u64().Inputs(1, 18);
            INST(7, Opcode::Compare).b().Inputs(5, 6);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(9, Opcode::Mul).u64().Inputs(5, 1);
            INST(10, Opcode::Mul).u64().Inputs(1, 2);
            INST(18, Opcode::Mul).u64().Inputs(10, 10);
            INST(11, Opcode::Sub).u64().Inputs(6, 1);
        }

        BASIC_BLOCK(5, 6, 7)
        {
            INST(13, Opcode::Compare).b().Inputs(6, 1);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(6, 2) {}
        BASIC_BLOCK(7, -1)
        {
            INST(16, Opcode::Add).u64().Inputs(19, 0);
            INST(17, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<Licm>(HOST_LIMIT);
    ASSERT_EQ(INS(10).GetBasicBlock(), BB(2).GetLoop()->GetPreHeader());
    ASSERT_EQ(INS(18).GetBasicBlock(), BB(2).GetLoop()->GetPreHeader());
    ASSERT_EQ(INS(9).GetBasicBlock(), &BB(4));
    ASSERT_EQ(INS(11).GetBasicBlock(), &BB(4));
    GraphChecker(GetGraph()).Check();
}

TEST_F(LicmTest, EmptyPreHeaderWithPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, 5)
        {
            INST(6, Opcode::Phi).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 5, 6)
        {
            INST(7, Opcode::Phi).u64().Inputs(0, 8);
            INST(8, Opcode::Add).u64().Inputs(7, 1);
            INST(9, Opcode::Mul).u64().Inputs(6, 6);
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::UINT64).CC(CC_LT).Inputs(8, 9);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::Return).u64().Inputs(8);
        }
    }

    GetGraph()->RunPass<Licm>(HOST_LIMIT);
    ASSERT_EQ(INS(9).GetBasicBlock(), BB(5).GetLoop()->GetPreHeader());
}

TEST_F(LicmTest, PreHeaderWithIf)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(7, Opcode::Phi).u64().Inputs(0, 8);
            INST(8, Opcode::Add).u64().Inputs(7, 1);
            INST(9, Opcode::Mul).u64().Inputs(0, 0);
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::UINT64).CC(CC_LT).Inputs(8, 9);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(12, Opcode::Phi).u64().Inputs(0, 8);
            INST(13, Opcode::Return).u64().Inputs(12);
        }
    }
    GetGraph()->RunPass<Licm>(HOST_LIMIT);

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 5, 4)
        {
            INST(5, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(5, 3)
        {
            INST(9, Opcode::Mul).u64().Inputs(0, 0);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(7, Opcode::Phi).u64().Inputs(0, 8);
            INST(8, Opcode::Add).u64().Inputs(7, 1);
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::UINT64).CC(CC_LT).Inputs(8, 9);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(12, Opcode::Phi).u64().Inputs(0, 8);
            INST(13, Opcode::Return).u64().Inputs(12);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
}  // namespace panda::compiler
