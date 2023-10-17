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
#include "optimizer/optimizations/licm.h"

namespace panda::compiler {
class LicmTest : public GraphTest {
public:
    static constexpr uint32_t HOST_LIMIT = 8;
};

// NOLINTBEGIN(readability-magic-numbers)
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

TEST_F(LicmTest, LicmResolver)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
            // We can safely hoist ResolveVirtual (INST[8]) into BLOCK[5] and link it to SaveState (INST[5])
            // as INST[6] produces a reference, which is never moved by GC (note that ResolveVirtual calls
            // runtime and may trigger GC).
        }
        BASIC_BLOCK(2, 2, 3)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 7);
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
    GetGraph()->RunPass<Cleanup>(HOST_LIMIT);

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 5);
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2, 2, 3)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LicmTest, LicmResolverIfHeaderIsNotExit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2, 3)
        {
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
        }
        BASIC_BLOCK(3, 2, 4)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 7);
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Licm>(HOST_LIMIT));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 5);
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2, 3)
        {
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
        }
        BASIC_BLOCK(3, 2, 4)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LicmTest, DontLicmResolverIfHeaderIsExit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2, 3, 4)
        {
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 7);
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
        }
        BASIC_BLOCK(4, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

TEST_F(LicmTest, DontLicmResolverIfNoAppropriateSaveState)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            // We must not hoist ResolveVirtual (INST[8]) into BLOCK[5] and link it to SaveState (INST[2]).
            // Otherwise a reference produced by INST[4] might be moved by GC as ResolveVirtual calls runtime.
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2, 2, 3)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 7);
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

TEST_F(LicmTest, DontLicmResolverThroughTry)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2, 2, 3)
        {
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 7);
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    BB(2).SetTry(true);
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

TEST_F(LicmTest, DontLicmResolverThroughInitClass)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(20, 0xa);

        BASIC_BLOCK(5, 2)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2).TypeId(0U);
            INST(1, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2, 2, 3)
        {
            INST(100, Opcode::SaveState).NoVregs();
            INST(101, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(100).TypeId(0U);
            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::ResolveVirtual).ptr().Inputs(4, 7);
            INST(9, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8}, {DataType::REFERENCE, 4}, {DataType::NO_TYPE, 7}});
            INST(10, Opcode::SaveState).NoVregs();
            INST(11, Opcode::NullCheck).ref().Inputs(0, 10);
            INST(12, Opcode::LoadObject).s32().Inputs(11).TypeId(122);
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12, 20);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

TEST_F(LicmTest, DontHoistLenArray)
{
    // If ChecksElimination removed NullCheck based on BoundsAnalysis, we can hoist LenArray
    // only if the array cannot be null in the loop preheader
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0);
        CONSTANT(2, nullptr).ref();

        BASIC_BLOCK(2, 3)
        {
            INST(7, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_NE).Inputs(0, 2);
            INST(3, Opcode::SaveStateDeoptimize).NoVregs();
        }

        BASIC_BLOCK(3, 4, 6)
        {
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).b().InputsAutoType(4);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(4, 3, 5)
        {
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(9, Opcode::SaveState).NoVregs();
            INST(10, Opcode::LenArray).s32().Inputs(0);
            INST(11, Opcode::BoundsCheck).s32().Inputs(10, 1, 9);
            INST(12, Opcode::StoreArray).s32().Inputs(0, 11, 1);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(13, Opcode::ReturnVoid).v0id();
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(LicmTest, HoistLenArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::SaveStateDeoptimize).NoVregs();
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3).SetFlag(inst_flags::CAN_DEOPTIMIZE);
        }

        BASIC_BLOCK(3, 3, 4)
        {
            INST(6, Opcode::SaveState).NoVregs();
            INST(7, Opcode::LenArray).s32().Inputs(4);
            INST(8, Opcode::BoundsCheck).s32().Inputs(7, 1, 6);
            INST(9, Opcode::StoreArray).s32().Inputs(0, 8, 1);
            INST(10, Opcode::CallStatic).b().InputsAutoType(6);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
    auto graph = CreateEmptyGraph();

    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::SaveStateDeoptimize).NoVregs();
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(7, Opcode::LenArray).s32().Inputs(4);
        }

        BASIC_BLOCK(3, 3, 4)
        {
            INST(6, Opcode::SaveState).NoVregs();
            INST(8, Opcode::BoundsCheck).s32().Inputs(7, 1, 6);
            INST(9, Opcode::StoreArray).s32().Inputs(0, 8, 1);
            INST(10, Opcode::CallStatic).b().InputsAutoType(6);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LicmTest, LoadImmediate)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1);
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(40, 0xa);
        CONSTANT(41, 0xb);
        BASIC_BLOCK(2, 5) {}
        BASIC_BLOCK(5, 4, 3)
        {
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40, 41);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(2, Opcode::LoadImmediate).ref().Class(class1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(15, Opcode::CallStatic).b().InputsAutoType(2, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnVoid);
        }
    }
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(40, 0xa);
        CONSTANT(41, 0xb);
        BASIC_BLOCK(2, 5)
        {
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40, 41);
            INST(2, Opcode::LoadImmediate).ref().Class(class1);
        }
        BASIC_BLOCK(5, 4, 3)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(15, Opcode::CallStatic).b().InputsAutoType(2, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}

TEST_F(LicmTest, LoadObjFromConst)
{
    auto obj1 = static_cast<uintptr_t>(1);
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(40, 0xa);
        CONSTANT(41, 0xb);
        BASIC_BLOCK(2, 5) {}
        BASIC_BLOCK(5, 4, 3)
        {
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40, 41);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(2, Opcode::LoadObjFromConst).ref().SetPtr(obj1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(15, Opcode::CallStatic).b().InputsAutoType(2, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnVoid);
        }
    }
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(40, 0xa);
        CONSTANT(41, 0xb);
        BASIC_BLOCK(2, 5)
        {
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40, 41);
            INST(2, Opcode::LoadObjFromConst).ref().SetPtr(obj1);
        }
        BASIC_BLOCK(5, 4, 3)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(20, Opcode::SaveState).Inputs(2).SrcVregs({VirtualRegister::BRIDGE});
            INST(15, Opcode::CallStatic).b().InputsAutoType(2, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}

TEST_F(LicmTest, FunctionImmediate)
{
    auto fptr = static_cast<uintptr_t>(1);
    auto graph = CreateEmptyGraph();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        CONSTANT(40, 0xa);
        CONSTANT(41, 0xb);
        BASIC_BLOCK(2, 5) {}
        BASIC_BLOCK(5, 4, 3)
        {
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40, 41);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(2, Opcode::FunctionImmediate).any().SetPtr(fptr);
            INST(20, Opcode::SaveState).NoVregs();
            INST(15, Opcode::CallStatic).b().InputsAutoType(2, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnVoid);
        }
    }
    auto graph1 = CreateEmptyGraph();
    graph1->SetDynamicMethod();
#ifndef NDEBUG
    graph1->SetDynUnitTestFlag();
#endif
    GRAPH(graph1)
    {
        CONSTANT(40, 0xa);
        CONSTANT(41, 0xb);
        BASIC_BLOCK(2, 5)
        {
            INST(13, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40, 41);
            INST(2, Opcode::FunctionImmediate).any().SetPtr(fptr);
        }
        BASIC_BLOCK(5, 4, 3)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(20, Opcode::SaveState).Inputs(2).SrcVregs({VirtualRegister::BRIDGE});
            INST(15, Opcode::CallStatic).b().InputsAutoType(2, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(17, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}

TEST_F(LicmTest, LoadFromConstantPool)
{
    auto graph = CreateEmptyGraph();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        PARAMETER(0, 0).any();
        PARAMETER(1, 1).any();
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::LoadConstantPool).any().Inputs(0);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(9, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 1}, {DataType::NO_TYPE, 3}});
            INST(4, Opcode::LoadFromConstantPool).any().Inputs(2).TypeId(1);
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 4, 9).SrcVregs({0, 1, 2, 3, 4});
            INST(6, Opcode::Intrinsic).b().Inputs({{DataType::ANY, 4}, {DataType::ANY, 9}, {DataType::NO_TYPE, 5}});
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));

    auto graph1 = CreateEmptyGraph();
    graph1->SetDynamicMethod();
#ifndef NDEBUG
    graph1->SetDynUnitTestFlag();
#endif
    GRAPH(graph1)
    {
        PARAMETER(0, 0).any();
        PARAMETER(1, 1).any();
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::LoadConstantPool).any().Inputs(0);
            INST(4, Opcode::LoadFromConstantPool).any().Inputs(2).TypeId(1);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2, 4).SrcVregs({0, 1, 2, VirtualRegister::BRIDGE});
            INST(9, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 1}, {DataType::NO_TYPE, 3}});
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 4, 9).SrcVregs({0, 1, 2, 3, 4});
            INST(6, Opcode::Intrinsic).b().Inputs({{DataType::ANY, 4}, {DataType::ANY, 9}, {DataType::NO_TYPE, 5}});
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
