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
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/lse.h"
#include "optimizer/optimizations/peepholes.h"

namespace panda::compiler {
class LSETest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
/// Two consecutive loads of the same array element with leading store
TEST_F(LSETest, SimpleLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({2, 5});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 2, 3);
            INST(7, Opcode::StoreArray).u32().Inputs(4, 6, 1);
            INST(8, Opcode::SaveState).Inputs(0, 2).SrcVregs({2, 5});
            INST(11, Opcode::BoundsCheck).s32().Inputs(5, 2, 8);
            INST(12, Opcode::LoadArray).s32().Inputs(4, 11);
            INST(13, Opcode::SaveState).Inputs(12, 2, 0).SrcVregs({0, 5, 1});
            INST(16, Opcode::BoundsCheck).s32().Inputs(5, 2, 13);
            INST(17, Opcode::LoadArray).s32().Inputs(4, 16);
            INST(18, Opcode::Add).s32().Inputs(12, 17);
            INST(19, Opcode::Return).s32().Inputs(18);
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({2, 5});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 2, 3);
            INST(7, Opcode::StoreArray).u32().Inputs(4, 6, 1);
            INST(18, Opcode::Add).s32().Inputs(1, 1);
            INST(19, Opcode::Return).s32().Inputs(18);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Two consecutive stores to the same array element
TEST_F(LSETest, SimpleStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 2, 3);
            INST(7, Opcode::StoreArray).u32().Inputs(4, 6, 1);
            INST(8, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 3});
            INST(11, Opcode::BoundsCheck).s32().Inputs(5, 2, 8);
            INST(12, Opcode::StoreArray).u32().Inputs(4, 11, 1);
            INST(13, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 3});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::LenArray).s32().Inputs(4);
            INST(6, Opcode::BoundsCheck).s32().Inputs(5, 2, 3);
            INST(7, Opcode::StoreArray).u32().Inputs(4, 6, 1);
            INST(13, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Store comes from previous basic block
TEST_F(LSETest, PreviousBlocks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 1);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(15, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(20, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(21, Opcode::Add).s32().Inputs(15, 20);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(22, Opcode::Phi).s32().Inputs({{2, 1}, {4, 21}});
            INST(23, Opcode::Return).s32().Inputs(22);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 1);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(21, Opcode::Add).s32().Inputs(1, 1);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(22, Opcode::Phi).s32().Inputs({{2, 1}, {4, 21}});
            INST(23, Opcode::Return).s32().Inputs(22);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Loading unknown value twice
TEST_F(LSETest, LoadWithoutStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::LoadArray).s64().Inputs(0, 1);
            INST(11, Opcode::LoadArray).s64().Inputs(0, 1);
            INST(12, Opcode::Add).s32().Inputs(6, 11);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::LoadArray).s64().Inputs(0, 1);
            INST(12, Opcode::Add).s32().Inputs(6, 6);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Empty basic block after elimination
TEST_F(LSETest, EmptyBB)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadArray).s64().Inputs(0, 1);
            INST(7, Opcode::Compare).b().CC(CC_GT).Inputs(6, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(13, Opcode::LoadArray).s64().Inputs(0, 1);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(14, Opcode::Phi).s64().Inputs({{2, 6}, {4, 13}});
            INST(15, Opcode::Return).s64().Inputs(14);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadArray).s64().Inputs(0, 1);
            INST(7, Opcode::Compare).b().CC(CC_GT).Inputs(6, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 3) {}
        BASIC_BLOCK(3, -1)
        {
            INST(14, Opcode::Phi).s64().Inputs({{2, 6}, {4, 6}});
            INST(15, Opcode::Return).s64().Inputs(14);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Do not load constant multiple times
TEST_F(LSETest, DISABLED_PoolConstants)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(9, Opcode::SaveState).NoVregs();
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(9);

            INST(12, Opcode::SaveState).NoVregs();
            INST(0, Opcode::LoadString).ref().Inputs(12).TypeId(42);
            INST(1, Opcode::StoreStatic).ref().Inputs(6, 0);

            INST(13, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadString).ref().Inputs(13).TypeId(27);
            INST(3, Opcode::StoreStatic).ref().Inputs(6, 2);

            INST(14, Opcode::SaveState).Inputs(2).SrcVregs({0});
            INST(4, Opcode::LoadString).ref().Inputs(14).TypeId(42);
            INST(5, Opcode::StoreStatic).ref().Inputs(6, 4);

            INST(21, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(9, Opcode::SaveState).NoVregs();
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(9);

            INST(12, Opcode::SaveState).NoVregs();
            INST(0, Opcode::LoadString).ref().Inputs(12).TypeId(42);
            INST(1, Opcode::StoreStatic).ref().Inputs(6, 0);

            INST(13, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadString).ref().Inputs(13).TypeId(27);
            INST(3, Opcode::StoreStatic).ref().Inputs(6, 2);

            INST(5, Opcode::StoreStatic).ref().Inputs(6, 0);

            INST(21, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Eliminate object field's operations.
TEST_F(LSETest, InstanceFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::StoreObject).s64().Inputs(3, 1).TypeId(122);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
            INST(6, Opcode::NullCheck).ref().Inputs(0, 5);
            INST(7, Opcode::LoadObject).s64().Inputs(6).TypeId(122);
            INST(8, Opcode::Return).s64().Inputs(7);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::StoreObject).s64().Inputs(3, 1).TypeId(122);
            INST(8, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// A store before branch eliminates a load after branch
TEST_F(LSETest, DominationHere)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(122);
            INST(7, Opcode::Compare).b().CC(CC_GT).Inputs(1, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(11, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(136);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(14, Opcode::LoadObject).s64().Inputs(0).TypeId(122);
            INST(15, Opcode::Return).s64().Inputs(14);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(122);
            INST(7, Opcode::Compare).b().CC(CC_GT).Inputs(1, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(11, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(136);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(15, Opcode::Return).s64().Inputs(1);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Infer the stored value through the heap
TEST_F(LSETest, TransitiveValues)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            INST(7, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(10, Opcode::StoreObject).s64().Inputs(0, 7).TypeId(243);
            INST(13, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(16, Opcode::StoreObject).s64().Inputs(0, 13).TypeId(243);
            INST(17, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            INST(17, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/**
 * Tests that store of an eliminated value and store of an replacement for this
 * eliminated value are treated as same stores.
 */
TEST_F(LSETest, StoreEliminableDominatesStoreReplacement)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            // Should be replaced with v1
            INST(7, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            // v5-v6 to invalidate the whole heap
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
            INST(6, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0}, {DataType::NO_TYPE, 5}});
            // Here v7 and v1 are the same values. v1 is a replacement for v7
            INST(10, Opcode::StoreObject).s64().Inputs(2, 7).TypeId(243);
            INST(11, Opcode::StoreObject).s64().Inputs(2, 1).TypeId(243);
            INST(17, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
            INST(6, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0}, {DataType::NO_TYPE, 5}});
            INST(10, Opcode::StoreObject).s64().Inputs(2, 1).TypeId(243);
            INST(17, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/**
 * Tests that store of an eliminated value and store of an replacement for this
 * eliminated value are treated as same stores.
 */
TEST_F(LSETest, StoreReplacementDominatesStoreEliminable)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            // Should be replaced with v1
            INST(7, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            // v5-v6 to invalidate the whole heap
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
            INST(6, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0}, {DataType::NO_TYPE, 5}});
            // Here v7 and v1 are the same values. v1 is a replacement for v7
            INST(10, Opcode::StoreObject).s64().Inputs(2, 1).TypeId(243);
            INST(11, Opcode::StoreObject).s64().Inputs(2, 7).TypeId(243);
            INST(17, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
            INST(6, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0}, {DataType::NO_TYPE, 5}});
            INST(10, Opcode::StoreObject).s64().Inputs(2, 1).TypeId(243);
            INST(17, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Load elimination in loop by means of a dominated load
TEST_F(LSETest, LoopElimination)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0x0).s64();
        CONSTANT(28, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(242);
            INST(6, Opcode::LenArray).s32().Inputs(3);
            INST(30, Opcode::Cmp).s32().Inputs(7, 6);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(30, 7);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        // for (v10 = 0, v10 < lenarr(v3), ++v10)
        //     v11 += v3[v10]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(10, Opcode::Phi).s32().Inputs({{2, 7}, {4, 27}});
            INST(11, Opcode::Phi).s32().Inputs({{2, 7}, {4, 26}});

            INST(20, Opcode::LoadObject).ref().Inputs(0).TypeId(242);  // Eliminable due to checked access above

            INST(25, Opcode::LoadArray).s32().Inputs(20, 10);

            INST(26, Opcode::Add).s32().Inputs(25, 11);
            INST(27, Opcode::Add).s32().Inputs(10, 28);
            INST(16, Opcode::Compare).b().CC(CC_GE).Inputs(27, 6);
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(16);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(35, Opcode::Phi).s32().Inputs({{4, 26}, {2, 7}});
            INST(29, Opcode::Return).s32().Inputs(35);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0x0).s64();
        CONSTANT(28, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(242);
            INST(6, Opcode::LenArray).s32().Inputs(3);
            INST(30, Opcode::Cmp).s32().Inputs(7, 6);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(30, 7);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        // for (v10 = 0, v10 < lenarr(v3), ++v10)
        //     v11 += v3[v10]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(10, Opcode::Phi).s32().Inputs({{2, 7}, {4, 27}});
            INST(11, Opcode::Phi).s32().Inputs({{2, 7}, {4, 26}});

            INST(25, Opcode::LoadArray).s32().Inputs(3, 10);

            INST(26, Opcode::Add).s32().Inputs(25, 11);
            INST(27, Opcode::Add).s32().Inputs(10, 28);
            INST(16, Opcode::Compare).b().CC(CC_GE).Inputs(27, 6);
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(16);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(35, Opcode::Phi).s32().Inputs({{4, 26}, {2, 7}});
            INST(29, Opcode::Return).s32().Inputs(35);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Loop of multiple blocks
TEST_F(LSETest, LoopBranches)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0x0).s64();
        CONSTANT(8, 0x2).s64();
        CONSTANT(37, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(242);
            INST(6, Opcode::LenArray).s32().Inputs(3);
            INST(48, Opcode::Compare).b().CC(CC_GE).Inputs(7, 6);
            INST(49, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(48);
        }
        // for (v11 = 0, v11 < lenarr(v3), ++v11)
        BASIC_BLOCK(4, 5, 6)
        {
            INST(11, Opcode::Phi).s32().Inputs({{2, 7}, {7, 46}});
            INST(12, Opcode::Phi).s32().Inputs({{2, 7}, {7, 45}});
            INST(22, Opcode::Mod).s32().Inputs(11, 8);
            INST(23, Opcode::Compare).b().CC(CC_EQ).Inputs(22, 7);
            INST(24, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(23);
        }
        //     if (v11 % 2 == 1)
        //          v44 = v3[v11]
        BASIC_BLOCK(6, 7)
        {
            INST(27, Opcode::LoadObject).ref().Inputs(0).TypeId(242);  // Eliminable due to checked access above
            INST(32, Opcode::LoadArray).s32().Inputs(27, 11);
        }
        //     else
        //          v44 = v3[v11 + 1]
        BASIC_BLOCK(5, 7)
        {
            INST(35, Opcode::LoadObject).ref().Inputs(0).TypeId(242);  // Eliminable due to checked access above
            INST(36, Opcode::Add).s32().Inputs(11, 37);
            INST(42, Opcode::LoadArray).s32().Inputs(35, 36);
        }
        //     v12 += v44
        BASIC_BLOCK(7, 3, 4)
        {
            INST(44, Opcode::Phi).s32().Inputs({{6, 32}, {5, 42}});
            INST(45, Opcode::Add).s32().Inputs(44, 12);
            INST(46, Opcode::Add).s32().Inputs(11, 37);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(46, 6);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(51, Opcode::Phi).s32().Inputs({{7, 45}, {2, 7}});
            INST(47, Opcode::Return).s32().Inputs(51);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0x0).s64();
        CONSTANT(8, 0x2).s64();
        CONSTANT(37, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(242);
            INST(6, Opcode::LenArray).s32().Inputs(3);
            INST(48, Opcode::Compare).b().CC(CC_GE).Inputs(7, 6);
            INST(49, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(48);
        }
        // for (v11 = 0, v11 < lenarr(v3), ++v11)
        BASIC_BLOCK(4, 5, 6)
        {
            INST(11, Opcode::Phi).s32().Inputs({{2, 7}, {7, 46}});
            INST(12, Opcode::Phi).s32().Inputs({{2, 7}, {7, 45}});
            INST(22, Opcode::Mod).s32().Inputs(11, 8);
            INST(23, Opcode::Compare).b().CC(CC_EQ).Inputs(22, 7);
            INST(24, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(23);
        }
        //     if (v11 % 2 == 1)
        //          v44 = v3[v11]
        BASIC_BLOCK(6, 7)
        {
            INST(32, Opcode::LoadArray).s32().Inputs(3, 11);
        }
        //     else
        //          v44 = v3[v11 + 1]
        BASIC_BLOCK(5, 7)
        {
            INST(36, Opcode::Add).s32().Inputs(11, 37);
            INST(42, Opcode::LoadArray).s32().Inputs(3, 36);
        }
        //     v12 += v44
        BASIC_BLOCK(7, 3, 4)
        {
            INST(44, Opcode::Phi).s32().Inputs({{6, 32}, {5, 42}});
            INST(45, Opcode::Add).s32().Inputs(44, 12);
            INST(46, Opcode::Add).s32().Inputs(11, 37);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(46, 6);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(51, Opcode::Phi).s32().Inputs({{7, 45}, {2, 7}});
            INST(47, Opcode::Return).s32().Inputs(51);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Nested loop elimination
TEST_F(LSETest, NestedLoopElimination)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(11, 0x0).s64();
        CONSTANT(62, 0x1).s64();
        BASIC_BLOCK(2, 8)
        {
            INST(4, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(181);
            INST(7, Opcode::LoadObject).ref().Inputs(0).TypeId(195);
            INST(10, Opcode::LenArray).s32().Inputs(7);
        }
        // for (v14 = 0, v14 < lenarr(v7), v14++)
        BASIC_BLOCK(8, 3, 4)
        {
            INST(14, Opcode::Phi).s32().Inputs({{2, 11}, {5, 63}});
            INST(15, Opcode::Phi).s32().Inputs({{2, 11}, {5, 69}});
            INST(20, Opcode::Cmp).s32().Inputs(14, 10);
            INST(21, Opcode::Compare).b().CC(CC_GE).Inputs(20, 11);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(25, Opcode::LoadObject).ref().Inputs(0).TypeId(209);
            INST(28, Opcode::LenArray).s32().Inputs(25);
            INST(65, Opcode::Cmp).s32().Inputs(11, 28);
            INST(66, Opcode::Compare).b().CC(CC_GE).Inputs(65, 11);
            INST(67, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(66);
        }
        //     for (v35 = 0, v35 < lenarr(v25), v35++)
        //         v32 += (v7[v14] * v25[v35])
        BASIC_BLOCK(6, 5, 6)
        {
            INST(32, Opcode::Phi).s32().Inputs({{4, 15}, {6, 60}});
            INST(35, Opcode::Phi).s32().Inputs({{4, 11}, {6, 61}});
            INST(45, Opcode::LoadObject).ref().Inputs(0).TypeId(195);  // Already loaded in BB2
            INST(50, Opcode::LoadArray).s32().Inputs(45, 14);
            INST(53, Opcode::LoadObject).ref().Inputs(0).TypeId(209);  // Already loaded in BB4
            INST(58, Opcode::LoadArray).s32().Inputs(53, 35);
            INST(59, Opcode::Mul).s32().Inputs(50, 58);
            INST(60, Opcode::Add).s32().Inputs(59, 32);
            INST(61, Opcode::Add).s32().Inputs(35, 62);
            INST(40, Opcode::Cmp).s32().Inputs(61, 28);
            INST(41, Opcode::Compare).b().CC(CC_GE).Inputs(40, 11);
            INST(42, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(41);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(69, Opcode::Phi).s32().Inputs({{6, 60}, {4, 15}});
            INST(63, Opcode::Add).s32().Inputs(14, 62);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(64, Opcode::Return).s32().Inputs(15);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(11, 0x0).s64();
        CONSTANT(62, 0x1).s64();
        BASIC_BLOCK(2, 8)
        {
            INST(4, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(181);
            INST(7, Opcode::LoadObject).ref().Inputs(0).TypeId(195);
            INST(10, Opcode::LenArray).s32().Inputs(7);
        }
        // for (v14 = 0, v14 < lenarr(v7), v14++)
        BASIC_BLOCK(8, 3, 4)
        {
            INST(14, Opcode::Phi).s32().Inputs({{2, 11}, {5, 63}});
            INST(15, Opcode::Phi).s32().Inputs({{2, 11}, {5, 69}});
            INST(20, Opcode::Cmp).s32().Inputs(14, 10);
            INST(21, Opcode::Compare).b().CC(CC_GE).Inputs(20, 11);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(25, Opcode::LoadObject).ref().Inputs(0).TypeId(209);
            INST(28, Opcode::LenArray).s32().Inputs(25);
            INST(65, Opcode::Cmp).s32().Inputs(11, 28);
            INST(66, Opcode::Compare).b().CC(CC_GE).Inputs(65, 11);
            INST(67, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(66);
        }
        //     for (v35 = 0, v35 < lenarr(v25), v35++)
        //         v32 += (v7[v14] * v25[v35])
        BASIC_BLOCK(6, 5, 6)
        {
            INST(32, Opcode::Phi).s32().Inputs({{4, 15}, {6, 60}});
            INST(35, Opcode::Phi).s32().Inputs({{4, 11}, {6, 61}});
            INST(50, Opcode::LoadArray).s32().Inputs(7, 14);
            INST(58, Opcode::LoadArray).s32().Inputs(25, 35);
            INST(59, Opcode::Mul).s32().Inputs(50, 58);
            INST(60, Opcode::Add).s32().Inputs(59, 32);
            INST(61, Opcode::Add).s32().Inputs(35, 62);
            INST(40, Opcode::Cmp).s32().Inputs(61, 28);
            INST(41, Opcode::Compare).b().CC(CC_GE).Inputs(40, 11);
            INST(42, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(41);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(69, Opcode::Phi).s32().Inputs({{6, 60}, {4, 15}});
            INST(63, Opcode::Add).s32().Inputs(14, 62);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(64, Opcode::Return).s32().Inputs(15);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Replace only MUST_ALIASed accesses
TEST_F(LSETest, LoopWithMayAliases)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // i32[][]
        CONSTANT(1, 0x0).s64();
        CONSTANT(30, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(9, Opcode::LenArray).s32().Inputs(6);
            INST(45, Opcode::Compare).b().CC(CC_GE).Inputs(1, 9);
            INST(46, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(45);
        }
        // for (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v0[0][0] + v0[1][0]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(12, Opcode::Phi).s32().Inputs({{2, 1}, {4, 43}});
            INST(13, Opcode::Phi).s32().Inputs({{2, 1}, {4, 42}});
            INST(24, Opcode::LoadArray).ref().Inputs(0, 1);  // Eliminated due to v6
            INST(29, Opcode::LoadArray).s32().Inputs(24, 1);
            INST(35, Opcode::LoadArray).ref().Inputs(0, 30);
            INST(40, Opcode::LoadArray).s32().Inputs(35, 1);
            INST(41, Opcode::Add).s32().Inputs(40, 29);
            INST(42, Opcode::Add).s32().Inputs(41, 13);
            INST(43, Opcode::Add).s32().Inputs(12, 30);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(43, 9);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(48, Opcode::Phi).s32().Inputs({{4, 42}, {2, 1}});
            INST(44, Opcode::Return).s32().Inputs(48);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();  // i32[][]
        CONSTANT(1, 0x0).s64();
        CONSTANT(30, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(9, Opcode::LenArray).s32().Inputs(6);
            INST(45, Opcode::Compare).b().CC(CC_GE).Inputs(1, 9);
            INST(46, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(45);
        }
        // for (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v0[0][0] + v0[1][0]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(12, Opcode::Phi).s32().Inputs({{2, 1}, {4, 43}});
            INST(13, Opcode::Phi).s32().Inputs({{2, 1}, {4, 42}});
            INST(29, Opcode::LoadArray).s32().Inputs(6, 1);
            INST(35, Opcode::LoadArray).ref().Inputs(0, 30);
            INST(40, Opcode::LoadArray).s32().Inputs(35, 1);
            INST(41, Opcode::Add).s32().Inputs(40, 29);
            INST(42, Opcode::Add).s32().Inputs(41, 13);
            INST(43, Opcode::Add).s32().Inputs(12, 30);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(43, 9);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(48, Opcode::Phi).s32().Inputs({{4, 42}, {2, 1}});
            INST(44, Opcode::Return).s32().Inputs(48);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Loop elimination combined with regular elimination
TEST_F(LSETest, CombinedWithLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // i32[][]
        CONSTANT(1, 0x0).s64();
        CONSTANT(43, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(9, Opcode::LenArray).s32().Inputs(6);
            INST(45, Opcode::Compare).b().CC(CC_GE).Inputs(1, 9);
            INST(46, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(45);
        }
        // for (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v0[0][0] + v0[0][0]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(12, Opcode::Phi).s32().Inputs({{2, 1}, {4, 42}});
            INST(13, Opcode::Phi).s32().Inputs({{2, 1}, {4, 41}});
            INST(24, Opcode::LoadArray).ref().Inputs(0, 1);  // Eliminated due to v6
            INST(29, Opcode::LoadArray).s32().Inputs(24, 1);
            INST(34, Opcode::LoadArray).ref().Inputs(0, 1);   // Eliminated due to v24
            INST(39, Opcode::LoadArray).s32().Inputs(34, 1);  // Eliminated due to v29
            INST(40, Opcode::Add).s32().Inputs(39, 29);
            INST(41, Opcode::Add).s32().Inputs(40, 13);
            INST(42, Opcode::Add).s32().Inputs(12, 43);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(42, 9);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(48, Opcode::Phi).s32().Inputs({{4, 41}, {2, 1}});
            INST(44, Opcode::Return).s32().Inputs(48);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();  // i32[][]
        CONSTANT(1, 0x0).s64();
        CONSTANT(43, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(9, Opcode::LenArray).s32().Inputs(6);
            INST(45, Opcode::Compare).b().CC(CC_GE).Inputs(1, 9);
            INST(46, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(45);
        }
        // for (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v0[0][0] + v0[0][0]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(12, Opcode::Phi).s32().Inputs({{2, 1}, {4, 42}});
            INST(13, Opcode::Phi).s32().Inputs({{2, 1}, {4, 41}});
            INST(29, Opcode::LoadArray).s32().Inputs(6, 1);
            INST(40, Opcode::Add).s32().Inputs(29, 29);
            INST(41, Opcode::Add).s32().Inputs(40, 13);
            INST(42, Opcode::Add).s32().Inputs(12, 43);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(42, 9);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(48, Opcode::Phi).s32().Inputs({{4, 41}, {2, 1}});
            INST(44, Opcode::Return).s32().Inputs(48);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Phi candidates are the origins of values on the heap
TEST_F(LSETest, OverwrittenPhiCandidates)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(2, 0x0).s64();
        CONSTANT(50, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::LoadArray).ref().Inputs(0, 2);
            INST(12, Opcode::StoreArray).ref().Inputs(0, 2, 1);
            INST(15, Opcode::LenArray).s32().Inputs(7);
            INST(52, Opcode::Compare).b().CC(CC_GE).Inputs(2, 15);
            INST(53, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(52);
        }
        BASIC_BLOCK(4, 3, 4)
        {
            INST(18, Opcode::Phi).s32().Inputs({{2, 2}, {4, 49}});
            INST(19, Opcode::Phi).s32().Inputs({{2, 2}, {4, 48}});
            // Must be replaced with a1 not v7
            INST(31, Opcode::LoadArray).ref().Inputs(0, 2);
            INST(36, Opcode::LoadArray).s32().Inputs(31, 2);
            // Must be replaced with a1 not v7
            INST(41, Opcode::LoadArray).ref().Inputs(0, 2);
            INST(46, Opcode::LoadArray).s32().Inputs(41, 2);
            INST(47, Opcode::Add).s32().Inputs(46, 36);
            INST(48, Opcode::Add).s32().Inputs(47, 19);
            INST(49, Opcode::Add).s32().Inputs(18, 50);
            INST(25, Opcode::Compare).b().CC(CC_GE).Inputs(49, 15);
            INST(26, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(25);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(55, Opcode::Phi).s32().Inputs({{4, 48}, {2, 2}});
            INST(51, Opcode::Return).s32().Inputs(55);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(2, 0x0).s64();
        CONSTANT(50, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::LoadArray).ref().Inputs(0, 2);
            INST(12, Opcode::StoreArray).ref().Inputs(0, 2, 1);
            INST(15, Opcode::LenArray).s32().Inputs(7);
            INST(52, Opcode::Compare).b().CC(CC_GE).Inputs(2, 15);
            INST(53, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(52);
        }
        BASIC_BLOCK(4, 3, 4)
        {
            INST(18, Opcode::Phi).s32().Inputs({{2, 2}, {4, 49}});
            INST(19, Opcode::Phi).s32().Inputs({{2, 2}, {4, 48}});
            INST(36, Opcode::LoadArray).s32().Inputs(1, 2);
            INST(46, Opcode::LoadArray).s32().Inputs(1, 2);
            INST(47, Opcode::Add).s32().Inputs(46, 36);
            INST(48, Opcode::Add).s32().Inputs(47, 19);
            INST(49, Opcode::Add).s32().Inputs(18, 50);
            INST(25, Opcode::Compare).b().CC(CC_GE).Inputs(49, 15);
            INST(26, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(25);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(55, Opcode::Phi).s32().Inputs({{4, 48}, {2, 2}});
            INST(51, Opcode::Return).s32().Inputs(55);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/**
 * Do not use a deleted instruction as a valid phi candidate
 *    [entry]
 *       |
 *      [2]
 *       |
 *      [3]--\
 *       |   [7]<--\
 *      [6]<-/ \---/
 *       |
 *     [exit]
 */
TEST_F(LSETest, DeletedPhiCandidate)
{
    GRAPH(GetGraph())
    {
        CONSTANT(9, 0x0).s64();
        CONSTANT(47, 0x1).s64();
        BASIC_BLOCK(2, 6, 7)
        {
            INST(10, Opcode::SaveState).Inputs(9).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(10).TypeId(0U);
            INST(1, Opcode::LoadStatic).s32().Inputs(3).TypeId(3059);  // A valid phi candidate for 3059
            INST(97, Opcode::SaveState);
            INST(4, Opcode::NewArray).ref().Inputs(3, 1, 97).TypeId(273);
            INST(5, Opcode::StoreStatic).ref().Inputs(3, 4).TypeId(3105);
            INST(65, Opcode::LoadStatic).s32().Inputs(3).TypeId(3059);  // Would be a deleted phi candidate
            INST(94, Opcode::SaveState);
            INST(68, Opcode::NewArray).ref().Inputs(3, 65, 94).TypeId(273);
            INST(69, Opcode::StoreStatic).ref().Inputs(3, 68).TypeId(3105);
            INST(89, Opcode::LoadStatic).s32().Inputs(3).TypeId(3059);  // Would be a deleted phi candidate
            INST(90, Opcode::Compare).b().CC(CC_GE).Inputs(9, 89);
            INST(91, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(90);
        }
        BASIC_BLOCK(7, 6, 7)
        {
            INST(72, Opcode::Phi).s32().Inputs({{2, 9}, {7, 87}});
            INST(79, Opcode::LoadStatic).ref().Inputs(3).TypeId(3105);
            INST(87, Opcode::Add).s32().Inputs(72, 47);
            INST(76, Opcode::LoadStatic).s32().Inputs(3).TypeId(3059);
            INST(77, Opcode::Compare).b().CC(CC_GE).Inputs(87, 76);
            INST(78, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(77);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(88, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        CONSTANT(9, 0x0).s64();
        CONSTANT(47, 0x1).s64();
        BASIC_BLOCK(2, 6, 7)
        {
            INST(10, Opcode::SaveState).Inputs(9).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(10).TypeId(0U);
            INST(1, Opcode::LoadStatic).s32().Inputs(3).TypeId(3059);
            INST(97, Opcode::SaveState);
            INST(4, Opcode::NewArray).ref().Inputs(3, 1, 97).TypeId(273);
            INST(5, Opcode::StoreStatic).ref().Inputs(3, 4).TypeId(3105);
            INST(94, Opcode::SaveState);
            INST(68, Opcode::NewArray).ref().Inputs(3, 1, 94).TypeId(273);
            INST(69, Opcode::StoreStatic).ref().Inputs(3, 68).TypeId(3105);
            INST(90, Opcode::Compare).b().CC(CC_GE).Inputs(9, 1);
            INST(91, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(90);
        }
        BASIC_BLOCK(7, 6, 7)
        {
            INST(72, Opcode::Phi).s32().Inputs({{2, 9}, {7, 87}});
            INST(87, Opcode::Add).s32().Inputs(72, 47);
            INST(77, Opcode::Compare).b().CC(CC_GE).Inputs(87, 1);
            INST(78, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(77);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(88, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Stored value might be casted therefore we should cast if types are different
TEST_F(LSETest, PrimitiveTypeCasting)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).s64().Inputs(1, 2);
            INST(6, Opcode::StoreObject).s32().Inputs(0, 3).TypeId(157);
            INST(9, Opcode::LoadObject).s32().Inputs(0).TypeId(157);
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).s64().Inputs(1, 2);
            INST(6, Opcode::StoreObject).s32().Inputs(0, 3).TypeId(157);
            // Types of a stored value (v3) and loaded (v9) are different. Cast is needed.
            INST(11, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(3);
            INST(10, Opcode::Return).s32().Inputs(11);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/**
 * Stored value might be casted therefore we should cast if types are different
 * To avoid inappropriate Cast, skip the elimination of some loadobj inst
 */
TEST_F(LSETest, PrimitiveInt8TypeCasting)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();

        PARAMETER(3, 3).ref();
        PARAMETER(4, 4).s8();
        PARAMETER(5, 5).s8();

        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::Add).s64().Inputs(1, 2);
            INST(9, Opcode::StoreObject).s32().Inputs(0, 7).TypeId(159);
            INST(11, Opcode::LoadObject).s32().Inputs(0).TypeId(159);

            INST(16, Opcode::Add).s8().Inputs(4, 5);
            INST(17, Opcode::StoreObject).s32().Inputs(3, 16).TypeId(163);
            INST(18, Opcode::LoadObject).s32().Inputs(3).TypeId(163);
            INST(23, Opcode::Add).s32().Inputs(11, 18);
            INST(19, Opcode::Return).s32().Inputs(23);
        }
    }
    auto graph_lsed = CreateEmptyBytecodeGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();

        PARAMETER(3, 3).ref();
        PARAMETER(4, 4).s8();
        PARAMETER(5, 5).s8();

        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::Add).s64().Inputs(1, 2);
            INST(9, Opcode::StoreObject).s32().Inputs(0, 7).TypeId(159);
            // Types of stored value (v4, INT8) and loaded (v9, INT32) are different and
            // legal for cast. We can use Cast to take place of LoadObject.
            INST(11, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(7);

            INST(16, Opcode::Add).s8().Inputs(4, 5);
            INST(17, Opcode::StoreObject).s32().Inputs(3, 16).TypeId(163);
            // if eliminate the loadobj inst, inappropriate inst 'i32 Cast i8' will be created.
            INST(18, Opcode::LoadObject).s32().Inputs(3).TypeId(163);
            INST(23, Opcode::Add).s32().Inputs(11, 18);
            INST(19, Opcode::Return).s32().Inputs(23);
        }
    }
    ASSERT_TRUE(graph->RunPass<Lse>());
    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_lsed));
}

/// Overwritten load in loop
TEST_F(LSETest, LoopWithOverwrite)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0x0).s64();
        CONSTANT(28, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(242);
            INST(6, Opcode::LenArray).s32().Inputs(3);
            INST(30, Opcode::Cmp).s32().Inputs(7, 6);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(30, 7);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        // for (v10 = 0, v10 < lenarr(v3), v10++)
        //     v3 = v3[v10]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(10, Opcode::Phi).s32().Inputs({{2, 7}, {4, 27}});

            INST(20, Opcode::LoadObject).ref().Inputs(0).TypeId(242);
            INST(25, Opcode::LoadArray).ref().Inputs(20, 10);

            INST(27, Opcode::Add).s32().Inputs(10, 28);

            INST(33, Opcode::StoreObject).ref().Inputs(0, 25).TypeId(242);

            INST(15, Opcode::Cmp).s32().Inputs(27, 6);
            INST(16, Opcode::Compare).b().CC(CC_GE).Inputs(15, 7);
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(16);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(29, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0x0).s64();
        CONSTANT(28, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(242);
            INST(6, Opcode::LenArray).s32().Inputs(3);
            INST(30, Opcode::Cmp).s32().Inputs(7, 6);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(30, 7);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        // for (v10 = 0, v10 < lenarr(v3), v10++)
        //     v3 = v3[v10]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(10, Opcode::Phi).s32().Inputs({{2, 7}, {4, 27}});
            INST(11, Opcode::Phi).ref().Inputs({{2, 3}, {4, 25}});

            INST(25, Opcode::LoadArray).ref().Inputs(11, 10);

            INST(27, Opcode::Add).s32().Inputs(10, 28);

            INST(33, Opcode::StoreObject).ref().Inputs(0, 25).TypeId(242);

            INST(15, Opcode::Cmp).s32().Inputs(27, 6);
            INST(16, Opcode::Compare).b().CC(CC_GE).Inputs(15, 7);
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(16);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(29, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Eliminate loads saved in SaveState and SafePoint.
TEST_F(LSETest, SavedLoadInSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(6, Opcode::StoreObject).ref().Inputs(0, 3).TypeId(122);

            INST(7, Opcode::SafePoint).Inputs(0, 3).SrcVregs({0, 2});

            // Eliminable because of v3 saved in SafePoint
            INST(8, Opcode::LoadObject).ref().Inputs(0).TypeId(100);

            INST(12, Opcode::Return).ref().Inputs(8);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(6, Opcode::StoreObject).ref().Inputs(0, 3).TypeId(122);

            INST(7, Opcode::SafePoint).Inputs(0, 3).SrcVregs({0, 2});

            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// SafePoint may relocate collected references making them invalid
TEST_F(LSETest, LoopWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, 8)
        {
            INST(5, Opcode::LoadObject).ref().Inputs(0).TypeId(657);
            INST(8, Opcode::LenArray).s32().Inputs(5);
        }
        BASIC_BLOCK(8, 8, 4)
        {
            INST(16, Opcode::SafePoint).Inputs(0).SrcVregs({0});
            INST(22, Opcode::LoadObject).ref().Inputs(0).TypeId(657);
            INST(23, Opcode::LenArray).s32().Inputs(22);
            INST(17, Opcode::Compare).b().CC(CC_GE).Inputs(23, 8);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(17);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(19, Opcode::ReturnVoid);
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, 8)
        {
            INST(5, Opcode::LoadObject).ref().Inputs(0).TypeId(657);
            INST(8, Opcode::LenArray).s32().Inputs(5);
        }
        BASIC_BLOCK(8, 8, 4)
        {
            INST(16, Opcode::SafePoint).Inputs(0, 5).SrcVregs({0, VirtualRegister::BRIDGE});
            INST(23, Opcode::LenArray).s32().Inputs(5);
            INST(17, Opcode::Compare).b().CC(CC_GE).Inputs(23, 8);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(17);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(19, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Call generally invalidates all heap.
TEST_F(LSETest, MemEscaping)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s64();
            BASIC_BLOCK(2, -1)
            {
                INST(4, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(122);
                INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 2});
                INST(6, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0}, {DataType::NO_TYPE, 5}});
                INST(9, Opcode::LoadObject).s64().Inputs(0).TypeId(122);
                INST(10, Opcode::Return).s64().Inputs(9);
            }
        }
    }
    ASSERT_FALSE(equal_graphs[0]->RunPass<Lse>());
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

/**
 * Use a dominated access after elimination
 *     [entry]
 *        |
 *    /--[2]--\
 *   [4]      |
 *    \----->[3]------\
 *            |       |
 *        /->[7]--\   |
 *        \---|   |   |
 *           [5]<-----/
 *            |
 *          [exit]
 */
TEST_F(LSETest, ReplaceByDominated)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x0).s64();
        CONSTANT(6, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
            INST(3, Opcode::Compare).b().CC(CC_GT).Inputs(1, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(3);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(5, Opcode::LoadStatic).ref().Inputs(30).TypeId(83);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 6, 1, 5).SrcVregs({5, 4, 3, 2, 1, 0});
            INST(8, Opcode::NullCheck).ref().Inputs(5, 7);
            INST(11, Opcode::StoreArray).s8().Inputs(5, 1, 6);
        }
        BASIC_BLOCK(3, 5, 7)
        {
            INST(13, Opcode::Phi).s32().Inputs({{2, 0}, {4, 1}});
            INST(15, Opcode::LoadStatic).ref().Inputs(30).TypeId(83);
            INST(20, Opcode::LoadArray).s32().Inputs(15, 13);
            INST(21, Opcode::Compare).b().CC(CC_EQ).Inputs(20, 2);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(7, 5, 7)
        {
            // Replace by dominated v15 not v5
            INST(33, Opcode::LoadStatic).ref().Inputs(30).TypeId(83);
            INST(38, Opcode::StoreArray).s32().Inputs(33, 13, 6);
            INST(31, Opcode::Compare).b().CC(CC_GT).Inputs(6, 13);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(45, Opcode::Phi).s32().Inputs({{3, 20}, {7, 6}});
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x0).s64();
        CONSTANT(6, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
            INST(3, Opcode::Compare).b().CC(CC_GT).Inputs(1, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(3);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(5, Opcode::LoadStatic).ref().Inputs(30).TypeId(83);
            INST(7, Opcode::SaveState).Inputs(0, 1, 2, 6, 1, 5).SrcVregs({5, 4, 3, 2, 1, 0});
            INST(8, Opcode::NullCheck).ref().Inputs(5, 7);
            INST(11, Opcode::StoreArray).s8().Inputs(5, 1, 6);
        }
        BASIC_BLOCK(3, 5, 7)
        {
            INST(13, Opcode::Phi).s32().Inputs({{2, 0}, {4, 1}});
            INST(15, Opcode::LoadStatic).ref().Inputs(30).TypeId(83);
            INST(20, Opcode::LoadArray).s32().Inputs(15, 13);
            INST(21, Opcode::Compare).b().CC(CC_EQ).Inputs(20, 2);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(7, 5, 7)
        {
            INST(38, Opcode::StoreArray).s32().Inputs(15, 13, 6);
            INST(31, Opcode::Compare).b().CC(CC_GT).Inputs(6, 13);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(45, Opcode::Phi).s32().Inputs({{3, 20}, {7, 6}});
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Normal loads and stores can be eliminated after volatile store
TEST_F(LSETest, ReorderableVolatileStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).ref().Inputs(2).TypeId(152);
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::StoreObject).ref().Inputs(5, 3).TypeId(138);
            INST(7, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(8, Opcode::NullCheck).ref().Inputs(0, 7);
            INST(9, Opcode::LoadObject).ref().Inputs(8).TypeId(152);
            INST(10, Opcode::Return).ref().Inputs(9);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).ref().Inputs(2).TypeId(152);
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, VirtualRegister::BRIDGE});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::StoreObject).ref().Inputs(5, 3).TypeId(138);
            INST(10, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

TEST_F(LSETest, ReorderableVolatileStoreWithOutBridge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).s32().Inputs(2).TypeId(152);
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::StoreObject).s32().Inputs(5, 3).TypeId(138);
            INST(7, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(8, Opcode::NullCheck).ref().Inputs(0, 7);
            INST(9, Opcode::LoadObject).s32().Inputs(8).TypeId(152);
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadObject).s32().Inputs(2).TypeId(152);
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::StoreObject).s32().Inputs(5, 3).TypeId(138);
            INST(10, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// v9 and v12 MAY_ALIAS each other. But after elimination of v11 they have NO_ALIAS.
TEST_F(LSETest, PhiCandidatesWithUpdatedAA)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).f32();
        CONSTANT(1, 0x2);
        CONSTANT(3, 0x0).f32();
        BASIC_BLOCK(2, 3)
        {
            INST(5, Opcode::SaveState).Inputs(3, 1).SrcVregs({1, 5});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);

            INST(23, Opcode::SaveState).Inputs(7).SrcVregs({0});
            INST(8, Opcode::NewArray).ref().Inputs(6, 1, 23).TypeId(20);
            INST(9, Opcode::StoreArray).ref().Inputs(8, 1, 7);
            INST(10, Opcode::LoadArray).ref().Inputs(8, 1);
            INST(11, Opcode::StoreObject).f32().Inputs(10, 0).TypeId(2726);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(14, Opcode::Phi).f32().Inputs({{2, 3}, {3, 15}});
            INST(12, Opcode::LoadObject).ref().Inputs(10).TypeId(2740);
            INST(13, Opcode::LoadArray).f32().Inputs(12, 1);
            INST(15, Opcode::Add).f32().Inputs(14, 13);
            INST(16, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(3, 15);
            INST(19, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0x0).Inputs(16);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(20, Opcode::Return).f32().Inputs(15);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).f32();
        CONSTANT(1, 0x2);
        CONSTANT(3, 0x0).f32();
        BASIC_BLOCK(2, 3)
        {
            INST(5, Opcode::SaveState).Inputs(3, 1).SrcVregs({1, 5});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);

            INST(23, Opcode::SaveState).Inputs(7).SrcVregs({0});
            INST(8, Opcode::NewArray).ref().Inputs(6, 1, 23).TypeId(20);
            INST(9, Opcode::StoreArray).ref().Inputs(8, 1, 7);
            INST(11, Opcode::StoreObject).f32().Inputs(7, 0).TypeId(2726);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(14, Opcode::Phi).f32().Inputs({{2, 3}, {3, 15}});
            INST(12, Opcode::LoadObject).ref().Inputs(7).TypeId(2740);
            INST(13, Opcode::LoadArray).f32().Inputs(12, 1);
            INST(15, Opcode::Add).f32().Inputs(14, 13);
            INST(16, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(3, 15);
            INST(19, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0x0).Inputs(16);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(20, Opcode::Return).f32().Inputs(15);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/**
 * The test relies on LSE debug internal check. The check aimed to control
 * that none of eliminated instructions are replaced by other eliminated
 * instructions.
 *
 * Here v25 may be erroneously replaced by v14.
 */
TEST_F(LSETest, EliminationOrderMatters)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(9, Opcode::StoreArray).s64().Inputs(1, 4, 2);
            // V14 is eliminated due to v9
            INST(14, Opcode::LoadArray).s64().Inputs(1, 4);
            // v19 MAY_ALIAS v9 and v14 therefore pops them from block_heap
            INST(19, Opcode::StoreArray).s64().Inputs(1, 3, 3);

            INST(22, Opcode::StoreObject).s64().Inputs(0, 14).TypeId(382);
            // v25 is eliminated due to v22 and should be replaced with v2 because v14 is eliminated too
            INST(25, Opcode::LoadObject).s64().Inputs(0).TypeId(382);
            INST(29, Opcode::Return).s64().Inputs(25);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(9, Opcode::StoreArray).s64().Inputs(1, 4, 2);
            INST(19, Opcode::StoreArray).s64().Inputs(1, 3, 3);

            INST(22, Opcode::StoreObject).s64().Inputs(0, 2).TypeId(382);
            INST(29, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/**
 * Similar to the test above but chain of elimination is across loops.
 *
 * Here v19 may be erroneously replaced by v15.
 */
TEST_F(LSETest, EliminationOrderMattersLoops)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(10, 0x0).s64();
        CONSTANT(11, 0xffffffffffffffff).s64();
        BASIC_BLOCK(5, 6)
        {
            INST(12, Opcode::Cast).s16().SrcType(DataType::INT64).Inputs(11);
            INST(13, Opcode::LoadObject).ref().Inputs(0).TypeId(947545);
            INST(14, Opcode::LenArray).s32().Inputs(13);
        }
        BASIC_BLOCK(6, 7, 6)
        {
            // v15 is eliminated due to v13
            INST(15, Opcode::LoadObject).ref().Inputs(0).TypeId(947545);
            INST(16, Opcode::StoreArray).s16().Inputs(15, 14, 12);
            INST(17, Opcode::Compare).b().CC(CC_EQ).Inputs(14, 10);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(17);
        }
        BASIC_BLOCK(7, 8, 7)
        {
            // v19 is eliminated due to v15 (a valid heap value after the last iteration of previous loop)
            // and should be replaced with v13 because v15 is eliminated too
            INST(19, Opcode::LoadObject).ref().Inputs(0).TypeId(947545);
            INST(20, Opcode::StoreArray).s16().Inputs(19, 14, 12);
            INST(21, Opcode::Compare).b().CC(CC_NE).Inputs(14, 10);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(23, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(10, 0x0).s64();
        CONSTANT(11, 0xffffffffffffffff).s64();
        BASIC_BLOCK(5, 6)
        {
            INST(12, Opcode::Cast).s16().SrcType(DataType::INT64).Inputs(11);
            INST(13, Opcode::LoadObject).ref().Inputs(0).TypeId(947545);
            INST(14, Opcode::LenArray).s32().Inputs(13);
        }
        BASIC_BLOCK(6, 7, 6)
        {
            INST(16, Opcode::StoreArray).s16().Inputs(13, 14, 12);
            INST(17, Opcode::Compare).b().CC(CC_EQ).Inputs(14, 10);
            INST(18, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(17);
        }
        BASIC_BLOCK(7, 8, 7)
        {
            INST(20, Opcode::StoreArray).s16().Inputs(13, 14, 12);
            INST(21, Opcode::Compare).b().CC(CC_NE).Inputs(14, 10);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(23, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/*
 * We can eliminate over SafePoints if the reference is listed in arguments
 */
TEST_F(LSETest, EliminationWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(6, Opcode::StoreObject).ref().Inputs(0, 3).TypeId(122);

            INST(7, Opcode::SafePoint).Inputs(3, 0).SrcVregs({0, 1});
            INST(8, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(9, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(10, Opcode::SaveState).Inputs(8, 9).SrcVregs({0, 1});
            INST(11, Opcode::NullCheck).ref().Inputs(9, 10);
            INST(12, Opcode::Return).ref().Inputs(8);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(6, Opcode::StoreObject).ref().Inputs(0, 3).TypeId(122);

            INST(7, Opcode::SafePoint).Inputs(3, 0).SrcVregs({0, 1});
            INST(10, Opcode::SaveState).Inputs(3, 3).SrcVregs({0, 1});
            INST(11, Opcode::NullCheck).ref().Inputs(3, 10);
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/*
 * We should be able to eliminate over SafePoints if the reference listed in
 * arguments was eliminated
 */
TEST_F(LSETest, EliminatedWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
            // v4 would be replaced with v1
            INST(4, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            // v1 should be alive after replacements
            INST(5, Opcode::SafePoint).Inputs(0, 4).SrcVregs({0, 1});
            // v6 would be replaced with v1
            INST(6, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(7, Opcode::Return).ref().Inputs(6);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
            // bridge is not required
            INST(5, Opcode::SafePoint).Inputs(0, 1, 1).SrcVregs({0, 1, VirtualRegister::BRIDGE});
            INST(7, Opcode::Return).ref().Inputs(1);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

TEST_F(LSETest, EliminatedWithSafePointNeedBridge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(5, Opcode::SafePoint).Inputs(0).SrcVregs({0});
            INST(6, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(7, Opcode::Return).ref().Inputs(6);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(5, Opcode::SafePoint).Inputs(0, 4).SrcVregs({0, VirtualRegister::BRIDGE});
            INST(7, Opcode::Return).ref().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Not aliased array acceses, since array cannot overlap each other
TEST_F(LSETest, SameArrayAccesses)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(5, 0x1).s64();
        CONSTANT(9, 0x0).s64();
        CONSTANT(32, 0x2).s64();
        CONSTANT(58, 0x3).s64();
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::LoadObject).ref().TypeId(302).Inputs(0);
            INST(8, Opcode::LoadObject).ref().TypeId(312).Inputs(0);

            INST(14, Opcode::LoadArray).s64().Inputs(8, 9);
            INST(22, Opcode::LoadArray).s64().Inputs(8, 5);
            INST(23, Opcode::Or).s64().Inputs(14, 22);
            INST(28, Opcode::StoreArray).s64().Inputs(4, 5, 23);

            // Same to 14, in spite of possible aliasing of v4 and v8, they
            // have been accessed at different indices
            INST(40, Opcode::LoadArray).s64().Inputs(8, 9);
            INST(48, Opcode::LoadArray).s64().Inputs(8, 32);
            INST(49, Opcode::Or).s64().Inputs(40, 48);
            INST(54, Opcode::StoreArray).s64().Inputs(4, 32, 49);

            // Same to 14 and 40, in spite of possible aliasing of v4 and v8,
            // they have been accessed at different indices
            INST(66, Opcode::LoadArray).s64().Inputs(8, 9);
            INST(74, Opcode::LoadArray).s64().Inputs(8, 58);
            INST(75, Opcode::Or).s64().Inputs(66, 74);
            INST(80, Opcode::StoreArray).s64().Inputs(4, 58, 75);

            INST(81, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(5, 0x1).s64();
        CONSTANT(9, 0x0).s64();
        CONSTANT(32, 0x2).s64();
        CONSTANT(58, 0x3).s64();
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::LoadObject).ref().TypeId(302).Inputs(0);
            INST(8, Opcode::LoadObject).ref().TypeId(312).Inputs(0);

            INST(14, Opcode::LoadArray).s64().Inputs(8, 9);
            INST(22, Opcode::LoadArray).s64().Inputs(8, 5);
            INST(23, Opcode::Or).s64().Inputs(14, 22);
            INST(28, Opcode::StoreArray).s64().Inputs(4, 5, 23);

            INST(48, Opcode::LoadArray).s64().Inputs(8, 32);
            INST(49, Opcode::Or).s64().Inputs(14, 48);
            INST(54, Opcode::StoreArray).s64().Inputs(4, 32, 49);

            INST(74, Opcode::LoadArray).s64().Inputs(8, 58);
            INST(75, Opcode::Or).s64().Inputs(14, 74);
            INST(80, Opcode::StoreArray).s64().Inputs(4, 58, 75);

            INST(81, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Eliminate in case of inlined virtual calls
TEST_F(LSETest, OverInlinedVirtualCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).i32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::StoreObject).i32().Inputs(1, 2).TypeId(122);
            INST(4, Opcode::SaveState).Inputs(1).SrcVregs({0});
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(6, Opcode::CallVirtual).v0id().InputsAutoType(1, 2, 4).Inlined();
            INST(7, Opcode::LoadObject).i32().Inputs(1).TypeId(122);
            INST(8, Opcode::ReturnInlined).v0id().Inputs(4);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(9, Opcode::CallVirtual).v0id().InputsAutoType(1, 2, 4).Inlined();
            INST(10, Opcode::LoadObject).i32().Inputs(1).TypeId(122);
            INST(11, Opcode::ReturnInlined).v0id().Inputs(4);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(12, Opcode::LoadObject).i32().Inputs(1).TypeId(122);
            INST(13, Opcode::Return).i32().Inputs(12);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).i32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::StoreObject).i32().Inputs(1, 2).TypeId(122);
            INST(4, Opcode::SaveState).Inputs(1).SrcVregs({0});
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(6, Opcode::CallVirtual).v0id().InputsAutoType(1, 2, 4).Inlined();
            INST(8, Opcode::ReturnInlined).v0id().Inputs(4);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(9, Opcode::CallVirtual).v0id().InputsAutoType(1, 2, 4).Inlined();
            INST(11, Opcode::ReturnInlined).v0id().Inputs(4);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(13, Opcode::Return).i32().Inputs(2);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Counter case with aliased store
TEST_F(LSETest, SameArrayAccessesWithOverwrite)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(5, 0x1).s64();
        CONSTANT(9, 0x0).s64();
        CONSTANT(32, 0x2).s64();
        CONSTANT(58, 0x3).s64();
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::LoadObject).ref().TypeId(302).Inputs(0);
            INST(8, Opcode::LoadObject).ref().TypeId(312).Inputs(0);

            INST(14, Opcode::LoadArray).s64().Inputs(8, 9);
            INST(22, Opcode::LoadArray).s64().Inputs(8, 5);
            INST(23, Opcode::Or).s64().Inputs(14, 22);
            INST(28, Opcode::StoreArray).s64().Inputs(4, 9, 23);

            // Same to 14 but v4 and v8 may be aliased and v28 may overwrite
            // previous load
            INST(40, Opcode::LoadArray).s64().Inputs(8, 9);
            INST(48, Opcode::LoadArray).s64().Inputs(8, 32);
            INST(49, Opcode::Or).s64().Inputs(40, 48);
            INST(54, Opcode::StoreArray).s64().Inputs(4, 32, 49);

            INST(81, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Counter case with unknown index
TEST_F(LSETest, SameArrayAccessesWithUnknownIndices)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        CONSTANT(5, 0x1).s64();
        CONSTANT(32, 0x2).s64();
        CONSTANT(58, 0x3).s64();
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::LoadObject).ref().TypeId(302).Inputs(0);
            INST(8, Opcode::LoadObject).ref().TypeId(312).Inputs(0);

            INST(14, Opcode::LoadArray).s64().Inputs(8, 1);
            INST(22, Opcode::LoadArray).s64().Inputs(8, 5);
            INST(23, Opcode::Or).s64().Inputs(14, 22);
            INST(28, Opcode::StoreArray).s64().Inputs(4, 5, 23);

            // Same to v14 but index is unknown, it may be equal to v5
            INST(40, Opcode::LoadArray).s64().Inputs(8, 1);
            INST(48, Opcode::LoadArray).s64().Inputs(8, 32);
            INST(49, Opcode::Or).s64().Inputs(40, 48);
            INST(54, Opcode::StoreArray).s64().Inputs(4, 32, 49);

            INST(81, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// A store does not dominate a load
TEST_F(LSETest, NoDominationHere)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::Compare).b().CC(CC_GT).Inputs(1, 1);
                INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
            }
            BASIC_BLOCK(4, 3)
            {
                INST(8, Opcode::StoreArray).s64().Inputs(0, 1, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(14, Opcode::LoadArray).s64().Inputs(0, 1);
                INST(15, Opcode::Return).s64().Inputs(14);
            }
        }
    }
    ASSERT_FALSE(equal_graphs[0]->RunPass<Lse>());
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

TEST_F(LSETest, EliminateMonitoredStores)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        CONSTANT(2, 1);
        BASIC_BLOCK(2, -1)
        {
            INST(15, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            INST(4, Opcode::Monitor).v0id().Entry().Inputs(0, 15);
            INST(5, Opcode::Add).s64().Inputs(1, 2);
            INST(6, Opcode::StoreObject).s64().Inputs(0, 5).TypeId(243);
            INST(7, Opcode::Add).s64().Inputs(5, 2);
            INST(8, Opcode::StoreObject).s64().Inputs(0, 7).TypeId(243);
            INST(16, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Monitor).v0id().Exit().Inputs(0, 16);
            INST(10, Opcode::Add).s64().Inputs(7, 2);
            INST(11, Opcode::StoreObject).s64().Inputs(0, 10).TypeId(243);
            INST(12, Opcode::Add).s64().Inputs(10, 2);
            INST(13, Opcode::StoreObject).s64().Inputs(0, 12).TypeId(243);
            INST(14, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        CONSTANT(2, 1);
        BASIC_BLOCK(2, -1)
        {
            INST(15, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
            INST(4, Opcode::Monitor).v0id().Entry().Inputs(0, 15);
            INST(5, Opcode::Add).s64().Inputs(1, 2);
            INST(7, Opcode::Add).s64().Inputs(5, 2);
            INST(8, Opcode::StoreObject).s64().Inputs(0, 7).TypeId(243);
            INST(16, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Monitor).v0id().Exit().Inputs(0, 16);
            INST(10, Opcode::Add).s64().Inputs(7, 2);
            INST(12, Opcode::Add).s64().Inputs(10, 2);
            INST(13, Opcode::StoreObject).s64().Inputs(0, 12).TypeId(243);
            INST(14, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

TEST_F(LSETest, EliminateMonitoredLoads)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(11, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(1, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(2, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(3, Opcode::Monitor).v0id().Entry().Inputs(0, 11);
            INST(4, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(5, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(12, Opcode::SaveState).Inputs(0, 1, 2, 4, 5).SrcVregs({0, 1, 2, 4, 5});
            INST(6, Opcode::Monitor).v0id().Exit().Inputs(0, 12);
            INST(7, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(8, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(13, Opcode::SaveState).Inputs(0, 1, 2, 4, 5, 7, 8).SrcVregs({0, 1, 2, 4, 5, 7, 8});
            INST(9, Opcode::CallStatic).b().InputsAutoType(1, 2, 4, 5, 7, 8, 13);
            INST(10, Opcode::Return).b().Inputs(9);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(11, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(1, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(3, Opcode::Monitor).v0id().Entry().Inputs(0, 11);
            INST(4, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
            INST(12, Opcode::SaveState).Inputs(0, 1, 1, 4, 4).SrcVregs({0, 1, 2, 4, 5});
            INST(6, Opcode::Monitor).v0id().Exit().Inputs(0, 12);
            INST(13, Opcode::SaveState).Inputs(0, 1, 1, 4, 4, 4, 4).SrcVregs({0, 1, 2, 4, 5, 7, 8});
            INST(9, Opcode::CallStatic).b().InputsAutoType(1, 1, 4, 4, 4, 4, 13);
            INST(10, Opcode::Return).b().Inputs(9);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Stores that cannot be eliminated due to monitors
TEST_F(LSETest, NotEliminableMonitoredStores)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s64();
            BASIC_BLOCK(2, -1)
            {
                INST(9, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
                INST(7, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
                INST(2, Opcode::Monitor).v0id().Entry().Inputs(0, 7);
                INST(5, Opcode::StoreObject).s64().Inputs(0, 1).TypeId(243);
                INST(8, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
                INST(6, Opcode::Monitor).v0id().Exit().Inputs(0, 8);
                INST(10, Opcode::ReturnVoid).v0id();
            }
        }
    }
    ASSERT_TRUE(equal_graphs[0]->RunPass<MonitorAnalysis>());
    ASSERT_FALSE(equal_graphs[0]->RunPass<Lse>());
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

TEST_F(LSETest, NotEliminableMonitoredLoadStore)
{
    std::vector<Graph *> equal_graphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equal_graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            BASIC_BLOCK(2, -1)
            {
                INST(9, Opcode::LoadObject).s64().Inputs(0).TypeId(243);
                INST(7, Opcode::SaveState).Inputs(0).SrcVregs({0});
                INST(2, Opcode::Monitor).v0id().Entry().Inputs(0, 7);
                INST(5, Opcode::StoreObject).s64().Inputs(0, 9).TypeId(243);
                INST(8, Opcode::SaveState).Inputs(0).SrcVregs({0});
                INST(6, Opcode::Monitor).v0id().Exit().Inputs(0, 8);
                INST(10, Opcode::ReturnVoid).v0id();
            }
        }
    }
    ASSERT_TRUE(equal_graphs[0]->RunPass<MonitorAnalysis>());
    ASSERT_FALSE(equal_graphs[0]->RunPass<Lse>());
    GraphChecker(equal_graphs[0]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equal_graphs[0], equal_graphs[1]));
}

/// Inner loop overwrites outer loop reference. No elimination
TEST_F(LSETest, InnerOverwrite)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(8, 0x0).s64();
        CONSTANT(53, 0x1).s64();
        BASIC_BLOCK(2, 8)
        {
            INST(4, Opcode::LoadObject).ref().Inputs(0).TypeId(194);
            INST(7, Opcode::LenArray).s32().Inputs(4);
        }
        // for (v11 = 0, v11 < lenarr(v4), v11++)
        BASIC_BLOCK(8, 3, 4)
        {
            INST(11, Opcode::Phi).s32().Inputs({{2, 8}, {5, 63}});
            INST(12, Opcode::Phi).s32().Inputs({{2, 8}, {5, 62}});
            INST(17, Opcode::Cmp).s32().Inputs(11, 7);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(17, 8);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(22, Opcode::LenArray).s32().Inputs(1);
            INST(65, Opcode::Cmp).s32().Inputs(8, 22);
            INST(66, Opcode::Compare).b().CC(CC_GE).Inputs(65, 8);
            INST(67, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(66);
        }
        //     for (v28 = 0, v28 < lenarr(v1), v28++)
        //         v4 = v28[v4[v11]]
        BASIC_BLOCK(6, 5, 6)
        {
            INST(28, Opcode::Phi).s32().Inputs({{4, 8}, {6, 52}});
            INST(38, Opcode::LoadObject).ref().Inputs(0).TypeId(194);  // Cannot eliminate due to v51
            INST(43, Opcode::LoadArray).s32().Inputs(38, 11);
            INST(48, Opcode::LoadArray).ref().Inputs(1, 43);
            INST(51, Opcode::StoreObject).ref().Inputs(0, 48).TypeId(194);
            INST(52, Opcode::Add).s32().Inputs(28, 53);
            INST(33, Opcode::Cmp).s32().Inputs(52, 22);
            INST(34, Opcode::Compare).b().CC(CC_GE).Inputs(33, 8);
            INST(35, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(34);
        }
        //     v12 += v4[v11]
        BASIC_BLOCK(5, 8)
        {
            INST(56, Opcode::LoadObject).ref().Inputs(0).TypeId(194);  // Cannot eliminate due to v51
            INST(61, Opcode::LoadArray).s32().Inputs(56, 11);
            INST(62, Opcode::Add).s32().Inputs(61, 12);
            INST(63, Opcode::Add).s32().Inputs(11, 53);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(64, Opcode::Return).s32().Inputs(12);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Outer loop overwrites inner loop reference.
TEST_F(LSETest, OuterOverwrite)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(8, 0x0).s64();
        CONSTANT(46, 0x1).s64();
        BASIC_BLOCK(2, 8)
        {
            INST(4, Opcode::LoadObject).ref().Inputs(0).TypeId(194);
            INST(7, Opcode::LenArray).s32().Inputs(4);
        }
        // for (v11 = 0, v11 < lenarr(v4), v11++)
        BASIC_BLOCK(8, 3, 4)
        {
            INST(11, Opcode::Phi).s32().Inputs({{2, 8}, {5, 55}});
            INST(12, Opcode::Phi).s32().Inputs({{2, 8}, {5, 62}});
            INST(17, Opcode::Cmp).s32().Inputs(11, 7);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(17, 8);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(57, Opcode::Cmp).s32().Inputs(8, 7);
            INST(58, Opcode::Compare).b().CC(CC_GE).Inputs(57, 8);
            INST(59, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(58);
        }
        //     for (v28 = 0, v28 < lenarr(v4), v28++)
        //         v12 += v4[v28]
        BASIC_BLOCK(6, 5, 6)
        {
            INST(26, Opcode::Phi).s32().Inputs({{4, 12}, {6, 44}});
            INST(28, Opcode::Phi).s32().Inputs({{4, 8}, {6, 45}});
            INST(38, Opcode::LoadObject).ref().Inputs(0).TypeId(194);
            INST(43, Opcode::LoadArray).s32().Inputs(38, 28);
            INST(44, Opcode::Add).s32().Inputs(43, 26);
            INST(45, Opcode::Add).s32().Inputs(28, 46);
            INST(33, Opcode::Cmp).s32().Inputs(45, 7);
            INST(34, Opcode::Compare).b().CC(CC_GE).Inputs(33, 8);
            INST(35, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(34);
        }
        //     v4 = v1[v11]
        BASIC_BLOCK(5, 8)
        {
            INST(62, Opcode::Phi).s32().Inputs({{6, 44}, {4, 12}});
            INST(51, Opcode::LoadArray).ref().Inputs(1, 11);
            INST(54, Opcode::StoreObject).ref().Inputs(0, 51).TypeId(194);
            INST(55, Opcode::Add).s32().Inputs(11, 46);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(56, Opcode::Return).s32().Inputs(12);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        CONSTANT(8, 0x0).s64();
        CONSTANT(46, 0x1).s64();
        BASIC_BLOCK(2, 8)
        {
            INST(4, Opcode::LoadObject).ref().Inputs(0).TypeId(194);
            INST(7, Opcode::LenArray).s32().Inputs(4);
        }
        // for (v11 = 0, v11 < lenarr(v4), v11++)
        BASIC_BLOCK(8, 3, 4)
        {
            INST(11, Opcode::Phi).s32().Inputs({{2, 8}, {5, 55}});
            INST(12, Opcode::Phi).s32().Inputs({{2, 8}, {5, 62}});
            INST(13, Opcode::Phi).ref().Inputs({{2, 4}, {5, 51}});
            INST(17, Opcode::Cmp).s32().Inputs(11, 7);
            INST(18, Opcode::Compare).b().CC(CC_GE).Inputs(17, 8);
            INST(19, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(18);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(57, Opcode::Cmp).s32().Inputs(8, 7);
            INST(58, Opcode::Compare).b().CC(CC_GE).Inputs(57, 8);
            INST(59, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(58);
        }
        //     for (v28 = 0, v28 < lenarr(v4), v28++)
        //         v12 += v4[v28]
        BASIC_BLOCK(6, 5, 6)
        {
            INST(26, Opcode::Phi).s32().Inputs({{4, 12}, {6, 44}});
            INST(28, Opcode::Phi).s32().Inputs({{4, 8}, {6, 45}});
            INST(43, Opcode::LoadArray).s32().Inputs(13, 28);
            INST(44, Opcode::Add).s32().Inputs(43, 26);
            INST(45, Opcode::Add).s32().Inputs(28, 46);
            INST(33, Opcode::Cmp).s32().Inputs(45, 7);
            INST(34, Opcode::Compare).b().CC(CC_GE).Inputs(33, 8);
            INST(35, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(34);
        }
        //     v4 = v1[v11]
        BASIC_BLOCK(5, 8)
        {
            INST(62, Opcode::Phi).s32().Inputs({{6, 44}, {4, 12}});
            INST(51, Opcode::LoadArray).ref().Inputs(1, 11);
            INST(54, Opcode::StoreObject).ref().Inputs(0, 51).TypeId(194);
            INST(55, Opcode::Add).s32().Inputs(11, 46);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(56, Opcode::Return).s32().Inputs(12);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Invalidating phi candidates
TEST_F(LSETest, PhiCandOverCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(7, 0x0).s64();
        CONSTANT(28, 0x1).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::LoadObject).ref().Inputs(0).TypeId(130);
            INST(3, Opcode::LoadObject).ref().Inputs(2).TypeId(242);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(30, Opcode::Cmp).s32().Inputs(7, 4);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(30, 7);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        // for (v10 = 0, v10 < lenarr(v3), ++v10)
        //     v11 += v3[v10]
        BASIC_BLOCK(4, 3, 4)
        {
            INST(10, Opcode::Phi).s32().Inputs({{2, 7}, {4, 27}});
            INST(11, Opcode::Phi).s32().Inputs({{2, 7}, {4, 26}});
            INST(40, Opcode::SaveState).NoVregs();
            INST(18, Opcode::CallStatic).v0id().InputsAutoType(2, 40);
            // Can't eliminated v19 because array may be overwritted in v4
            INST(19, Opcode::LoadObject).ref().Inputs(0).TypeId(130);
            INST(20, Opcode::LoadObject).ref().Inputs(19).TypeId(242);

            INST(25, Opcode::LoadArray).s32().Inputs(20, 10);

            INST(26, Opcode::Add).s32().Inputs(25, 11);
            INST(27, Opcode::Add).s32().Inputs(10, 28);
            INST(16, Opcode::Compare).b().CC(CC_GE).Inputs(27, 4);
            INST(17, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(16);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(35, Opcode::Phi).s32().Inputs({{4, 26}, {2, 7}});
            INST(29, Opcode::Return).s32().Inputs(35);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Not dominated volatile load
TEST_F(LSETest, NotDominatedVolatileLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(7, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::LoadObject).s32().Inputs(0).TypeId(152);
            INST(6, Opcode::Compare).b().CC(CC_EQ).Inputs(2, 7);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(6);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(11, Opcode::LoadObject).s32().Volatile().Inputs(0).TypeId(138);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(12, Opcode::Phi).s32().Inputs({{2, 1}, {4, 11}});
            INST(14, Opcode::SaveState).Inputs(2, 1, 0, 12, 5).SrcVregs({4, 3, 2, 1, 0});
            INST(15, Opcode::NullCheck).ref().Inputs(0, 14);
            // v16 is not eliminable due to volatile load v11
            INST(16, Opcode::LoadObject).s32().Inputs(0).TypeId(152);
            INST(17, Opcode::Add).s32().Inputs(16, 12);
            INST(18, Opcode::Return).s32().Inputs(17);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// If we eliminate load over safepoint, check that it is correctly bridged
TEST_F(LSETest, LoadsWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(6, Opcode::StoreObject).ref().Inputs(0, 3).TypeId(122);

            INST(7, Opcode::SafePoint).Inputs(0).SrcVregs({0});
            // Eliminable because of v6 but v6 can be relocated by GC
            INST(8, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            // Eliminable because of v3 but v3 can be relocated by GC
            INST(9, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(10, Opcode::SaveState).Inputs(8).SrcVregs({0});
            INST(11, Opcode::NullCheck).ref().Inputs(9, 10);
            INST(12, Opcode::Return).ref().Inputs(8);
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(6, Opcode::StoreObject).ref().Inputs(0, 3).TypeId(122);

            INST(7, Opcode::SafePoint).Inputs(0, 3).SrcVregs({0, VirtualRegister::BRIDGE});
            INST(10, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(11, Opcode::NullCheck).ref().Inputs(3, 10);
            INST(12, Opcode::Return).ref().Inputs(3);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Pool constants can be relocated as well and should be bridged.
TEST_F(LSETest, RelocatablePoolConstants)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0);

            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::LoadString).ref().Inputs(2).TypeId(42);
            INST(4, Opcode::StoreStatic).ref().Inputs(1, 3);

            INST(7, Opcode::SafePoint).NoVregs();

            INST(8, Opcode::SaveState).NoVregs();
            INST(9, Opcode::LoadString).ref().Inputs(8).TypeId(42);
            INST(10, Opcode::StoreStatic).ref().Inputs(1, 9);

            INST(11, Opcode::ReturnVoid);
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0);

            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::LoadString).ref().Inputs(2).TypeId(42);
            INST(4, Opcode::StoreStatic).ref().Inputs(1, 3);

            INST(7, Opcode::SafePoint).Inputs(3).SrcVregs({VirtualRegister::BRIDGE});

            INST(10, Opcode::StoreStatic).ref().Inputs(1, 3);

            INST(11, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// If we replace load in loop by value stored before loop, check that it's correctly bridged in any loop safepoints
TEST_F(LSETest, StoreAndLoadLoopWithSafepoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 3)
        {
            INST(6, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(7, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(8, Opcode::SafePoint).Inputs(0).SrcVregs({0});
            INST(9, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Return).ref().Inputs(7);
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 3)
        {
            INST(6, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(8, Opcode::SafePoint).Inputs(0, 1).SrcVregs({0, VirtualRegister::BRIDGE});
            INST(9, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Return).ref().Inputs(1);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// If we hoist load from loop, check that it's correctly bridged in any loop safepoints
TEST_F(LSETest, HoistFromLoopWithSafepoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 3, 4)
        {
            INST(7, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(8, Opcode::SafePoint).Inputs(0).SrcVregs({0});
            INST(9, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Return).ref().Inputs(7);
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, 3)
        {
            INST(7, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(8, Opcode::SafePoint).Inputs(0, 7).SrcVregs({0, VirtualRegister::BRIDGE});
            INST(9, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Return).ref().Inputs(7);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Store that is not read, but overwritten on all paths is removed
TEST_F(LSETest, RemoveShadowedStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 1);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::Add).s32().Inputs(1, 1);
            INST(17, Opcode::StoreArray).s32().Inputs(0, 2, 21);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(22, Opcode::Mul).s32().Inputs(1, 1);
            INST(18, Opcode::StoreArray).s32().Inputs(0, 2, 22);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(23, Opcode::Return).s32().Inputs(2);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::Add).s32().Inputs(1, 1);
            INST(17, Opcode::StoreArray).s32().Inputs(0, 2, 21);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(22, Opcode::Mul).s32().Inputs(1, 1);
            INST(18, Opcode::StoreArray).s32().Inputs(0, 2, 22);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(23, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Store that is not read, but not overwritten on all paths is not removed
TEST_F(LSETest, DontRemoveStoreIfPathWithoutShadowExists)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 1);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(21, Opcode::Add).s32().Inputs(1, 1);
            INST(17, Opcode::StoreArray).s32().Inputs(0, 2, 21);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(23, Opcode::Return).s32().Inputs(2);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(LSETest, DISABLED_ShadowInInnerLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 1);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::Add).s32().Inputs(1, 1);
            INST(17, Opcode::StoreArray).s32().Inputs(0, 2, 21);
        }
        BASIC_BLOCK(4, 5, 4)
        {
            INST(6, Opcode::Phi).s32().Inputs({{2, 1}, {4, 22}});
            INST(22, Opcode::Add).s32().Inputs(6, 1);
            INST(18, Opcode::StoreArray).s32().Inputs(0, 2, 22);
            INST(11, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(23, Opcode::Return).s32().Inputs(2);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::Add).s32().Inputs(1, 1);
            INST(17, Opcode::StoreArray).s32().Inputs(0, 2, 21);
        }
        BASIC_BLOCK(4, 5, 4)
        {
            INST(6, Opcode::Phi).s32().Inputs({{2, 1}, {4, 22}});
            INST(22, Opcode::Add).s32().Inputs(6, 1);
            INST(18, Opcode::StoreArray).s32().Inputs(0, 2, 22);
            INST(11, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(23, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Stores are not removed from loops, unless they're shadowed in the same loop
TEST_F(LSETest, ShadowedStoresInLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 4, 3)
        {
            INST(7, Opcode::Phi).s32().Inputs({{2, 1}, {3, 21}});
            INST(21, Opcode::Add).s32().Inputs(7, 1);
            INST(17, Opcode::StoreArray).s32().Inputs(0, 2, 21);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 5, 4)
        {
            INST(27, Opcode::Phi).s32().Inputs({{3, 1}, {4, 21}});
            INST(35, Opcode::StoreArray).s32().Inputs(0, 2, 27);
            INST(41, Opcode::Mul).s32().Inputs(27, 1);
            INST(37, Opcode::StoreArray).s32().Inputs(0, 2, 41);
            INST(30, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(28, Opcode::Phi).s32().Inputs({{4, 1}, {6, 41}});
            INST(36, Opcode::StoreArray).s32().Inputs(0, 2, 28);
        }
        BASIC_BLOCK(6, 7, 5)
        {
            INST(42, Opcode::Mul).s32().Inputs(27, 1);
            INST(38, Opcode::StoreArray).s32().Inputs(0, 2, 42);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(23, Opcode::Return).s32().Inputs(2);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3) {}
        BASIC_BLOCK(3, 4, 3)
        {
            INST(7, Opcode::Phi).s32().Inputs({{2, 1}, {3, 21}});
            INST(21, Opcode::Add).s32().Inputs(7, 1);
            INST(17, Opcode::StoreArray).s32().Inputs(0, 2, 21);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(1, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 5, 4)
        {
            INST(27, Opcode::Phi).s32().Inputs({{3, 1}, {4, 21}});
            INST(41, Opcode::Mul).s32().Inputs(27, 1);
            INST(37, Opcode::StoreArray).s32().Inputs(0, 2, 41);
            INST(30, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(28, Opcode::Phi).s32().Inputs({{4, 1}, {6, 41}});
        }
        BASIC_BLOCK(6, 7, 5)
        {
            INST(42, Opcode::Mul).s32().Inputs(27, 1);
            INST(38, Opcode::StoreArray).s32().Inputs(0, 2, 42);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(23, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Store that can be read is not removed
TEST_F(LSETest, DontRemoveShadowedStoreIfRead)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::StoreArray).u32().Inputs(0, 2, 1);
            INST(8, Opcode::NullCheck).ref().Inputs(3);
            INST(10, Opcode::Add).s32().Inputs(1, 1);
            INST(11, Opcode::StoreArray).u32().Inputs(0, 2, 10);
            INST(13, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Check that shadows store search correctly handles irreducible loops
TEST_F(LSETest, ShadowStoreWithIrreducibleLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        PARAMETER(4, 4).ref();
        CONSTANT(5, 0x2a).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::StoreArray).s32().Inputs(4, 2, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(4, 5, 7)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(5, 6, 9)
        {
            INST(20, Opcode::Phi).s32().Inputs({{4, 5}, {7, 11}});
            INST(26, Opcode::Mul).s32().Inputs(20, 5);
            INST(28, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(30, Opcode::SaveState).Inputs(4).SrcVregs({0});
        }
        BASIC_BLOCK(7, 5, 9)
        {
            INST(11, Opcode::Phi).s32().Inputs({{4, 5}, {6, 26}});
            INST(19, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 9)
        {
            INST(12, Opcode::StoreArray).s32().Inputs(4, 2, 3);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(34, Opcode::Phi).s32().Inputs({{3, 1}, {5, 2}, {7, 3}});
            INST(35, Opcode::Return).s32().Inputs(34);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not eliminate loads over runtime calls due to GC relocations
TEST_F(LSETest, LoadsWithRuntimeCalls)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(6, Opcode::StoreObject).ref().Inputs(0, 3).TypeId(122);

            INST(13, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(14, Opcode::LoadAndInitClass).ref().Inputs(13);
            INST(15, Opcode::StoreStatic).ref().Inputs(14, 3);

            // Eliminable because of v6 but v6 can be relocated by GC
            INST(8, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            // Eliminable because of v3 but v3 can be relocated by GC
            INST(9, Opcode::LoadObject).ref().Inputs(0).TypeId(100);
            INST(10, Opcode::SaveState).Inputs(8).SrcVregs({0});
            INST(11, Opcode::NullCheck).ref().Inputs(9, 10);
            INST(12, Opcode::Return).ref().Inputs(8);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not eliminate through static constructors
TEST_F(LSETest, OverClassInitialization)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::StoreStatic).ref().Inputs(6, 0).TypeId(42);

            INST(9, Opcode::InitClass).Inputs(5).TypeId(200);

            INST(11, Opcode::LoadStatic).ref().Inputs(6).TypeId(42);
            INST(21, Opcode::Return).ref().Inputs(11);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist from irreducible loop
TEST_F(LSETest, DontHoistWithIrreducibleLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        PARAMETER(4, 4).ref();
        CONSTANT(5, 0x2a).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(7, Opcode::StoreArray).s32().Inputs(4, 2, 1);
            INST(8, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(4, 5, 7)
        {
            INST(10, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(1);
        }
        BASIC_BLOCK(5, 6, 9)
        {
            INST(20, Opcode::Phi).s32().Inputs({{4, 5}, {7, 11}});
            INST(21, Opcode::LoadArray).s32().Inputs(4, 2);
            INST(26, Opcode::Mul).s32().Inputs(20, 21);
            INST(28, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(6, 7)
        {
            INST(30, Opcode::SaveState).Inputs(4).SrcVregs({0});
        }
        BASIC_BLOCK(7, 5, 9)
        {
            INST(11, Opcode::Phi).s32().Inputs({{4, 5}, {6, 26}});
            INST(19, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 9)
        {
            INST(12, Opcode::StoreArray).s32().Inputs(4, 2, 3);
        }
        BASIC_BLOCK(9, -1)
        {
            INST(34, Opcode::Phi).s32().Inputs({{3, 1}, {5, 2}, {7, 3}});
            INST(35, Opcode::Return).s32().Inputs(34);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist values not alive on backedge
TEST_F(LSETest, DontHoistIfNotAlive)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(45, Opcode::Phi).s64().Inputs({{2, 1}, {6, 35}});
            INST(21, Opcode::Compare).b().CC(CC_LT).Inputs(45, 6);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(25, Opcode::Add).s64().Inputs(45, 45);
            INST(26, Opcode::StoreStatic).s64().Inputs(30, 25).TypeId(83);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(17, Opcode::Mul).s64().Inputs(16, 45);
        }
        BASIC_BLOCK(6, 3, 7)
        {
            INST(35, Opcode::Phi).s64().Inputs({{4, 25}, {5, 17}});
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(35, 2);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist values not alive on backedge
TEST_F(LSETest, DontHoistIfNotAliveTriangle)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(25, Opcode::Phi).s64().Inputs({{2, 1}, {5, 35}});
            INST(21, Opcode::Compare).b().CC(CC_LT).Inputs(25, 6);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(17, Opcode::Mul).s64().Inputs(16, 25);
        }
        BASIC_BLOCK(5, 3, 6)
        {
            INST(35, Opcode::Phi).s64().Inputs({{3, 25}, {4, 17}});
            INST(33, Opcode::StoreStatic).s64().Inputs(30, 35).TypeId(83);
            INST(34, Opcode::Add).s64().Inputs(35, 1);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(34, 2);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(46, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Double hoist
TEST_F(LSETest, HoistInnerLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(45, Opcode::Phi).s64().Inputs({{2, 1}, {4, 25}});
            INST(21, Opcode::Compare).b().CC(CC_LT).Inputs(45, 6);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 4, 3)
        {
            INST(35, Opcode::Phi).s64().Inputs({{3, 45}, {4, 25}});
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(25, Opcode::Add).s64().Inputs(45, 16);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(16, 2);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(45, Opcode::Phi).s64().Inputs({{2, 1}, {4, 25}});
            INST(21, Opcode::Compare).b().CC(CC_LT).Inputs(45, 6);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 4, 3)
        {
            INST(35, Opcode::Phi).s64().Inputs({{3, 45}, {4, 25}});
            INST(25, Opcode::Add).s64().Inputs(45, 16);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(16, 2);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Single hoist
TEST_F(LSETest, HoistInnerLoop2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
        }
        BASIC_BLOCK(3, 4, 6)
        {
            INST(45, Opcode::Phi).s64().Inputs({{2, 1}, {5, 25}});
            INST(21, Opcode::Compare).b().CC(CC_LT).Inputs(45, 6);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 4, 5)
        {
            INST(35, Opcode::Phi).s64().Inputs({{3, 45}, {4, 25}});
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(25, Opcode::Add).s64().Inputs(45, 16);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(16, 2);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, 3)
        {
            INST(17, Opcode::StoreStatic).s64().Inputs(30, 25).TypeId(83);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
        }
        BASIC_BLOCK(3, 4, 6)
        {
            INST(45, Opcode::Phi).s64().Inputs({{2, 1}, {5, 25}});
            INST(21, Opcode::Compare).b().CC(CC_LT).Inputs(45, 6);
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 4, 5)
        {
            INST(35, Opcode::Phi).s64().Inputs({{3, 45}, {4, 25}});
            INST(25, Opcode::Add).s64().Inputs(45, 16);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(16, 2);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, 3)
        {
            INST(17, Opcode::StoreStatic).s64().Inputs(30, 25).TypeId(83);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Don't hoist due to inner store
TEST_F(LSETest, DontHoistOuterLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
        }
        BASIC_BLOCK(3, 4, 6)
        {
            INST(45, Opcode::Phi).s64().Inputs({{2, 1}, {4, 25}});
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(21, Opcode::Compare).b().CC(CC_LT).Inputs(45, 6);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 4, 3)
        {
            INST(35, Opcode::Phi).s64().Inputs({{3, 45}, {4, 25}});
            INST(25, Opcode::Add).s64().Inputs(45, 16);
            INST(17, Opcode::StoreStatic).s64().Inputs(30, 25).TypeId(83);
            INST(31, Opcode::Compare).b().CC(CC_GE).Inputs(16, 2);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist from OSR loop
TEST_F(LSETest, DontHoistFromOsrLoop)
{
    GetGraph()->SetMode(GraphMode::Osr());
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s32();
        BASIC_BLOCK(2, 3)
        {
            INST(6, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
            INST(11, Opcode::StoreObject).s32().Inputs(0, 2).TypeId(123);
            INST(12, Opcode::LoadObject).s32().Inputs(0).TypeId(124);
            INST(13, Opcode::LoadObject).s32().Inputs(0).TypeId(125);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(7, Opcode::SaveStateOsr).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::LoadObject).ref().Inputs(0).TypeId(122);       // store-load pair
            INST(14, Opcode::StoreObject).s32().Inputs(0, 2).TypeId(123);  // store-store pair
            INST(15, Opcode::LoadObject).s32().Inputs(0).TypeId(124);      // load-load pair
            INST(9, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::LoadObject).s32().Inputs(0).TypeId(125);  // load-load pair, before and after OSR loop
            INST(10, Opcode::Return).s32().Inputs(16);
        }
    }

    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(LSETest, AliveLoadInBackedge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x0).s64();
        CONSTANT(6, 0x1).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
            INST(5, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(15, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(21, Opcode::Compare).b().CC(CC_EQ).Inputs(15, 2);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(24, Opcode::Add).s64().Inputs(15, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(23, Opcode::Mul).s64().Inputs(15, 15);
            INST(7, Opcode::StoreStatic).s64().Inputs(30, 23).TypeId(83);
        }
        BASIC_BLOCK(6, 7, 3)
        {
            INST(45, Opcode::Phi).s64().Inputs({{4, 24}, {5, 23}});
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(31, Opcode::Compare).b().CC(CC_EQ).Inputs(16, 6);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x0).s64();
        CONSTANT(6, 0x1).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
            INST(5, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(15, Opcode::Phi).s64().Inputs({{2, 5}, {6, 16}});
            INST(21, Opcode::Compare).b().CC(CC_EQ).Inputs(15, 2);
            INST(22, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(21);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(24, Opcode::Add).s64().Inputs(15, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(23, Opcode::Mul).s64().Inputs(15, 15);
            INST(7, Opcode::StoreStatic).s64().Inputs(30, 23).TypeId(83);
        }
        BASIC_BLOCK(6, 7, 3)
        {
            INST(45, Opcode::Phi).s64().Inputs({{4, 24}, {5, 23}});
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(31, Opcode::Compare).b().CC(CC_EQ).Inputs(16, 6);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

TEST_F(LSETest, AliveLoadInBackedgeInnerLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
            INST(5, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(15, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(24, Opcode::Add).s64().Inputs(15, 1);
            INST(7, Opcode::StoreStatic).s64().Inputs(30, 24).TypeId(83);
        }
        BASIC_BLOCK(4, 4, 5)
        {
            INST(45, Opcode::Phi).s64().Inputs({{3, 1}, {4, 25}});
            INST(25, Opcode::Mul).s64().Inputs(45, 45);
            INST(31, Opcode::Compare).b().CC(CC_EQ).Inputs(25, 6);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, 3, 6)
        {
            INST(16, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
            INST(27, Opcode::Compare).b().CC(CC_EQ).Inputs(16, 2);
            INST(28, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(27);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x2).s64();
        CONSTANT(2, 0x1).s64();
        CONSTANT(6, 0x10).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(40, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(30, Opcode::LoadAndInitClass).ref().Inputs(40).TypeId(0U);
            INST(5, Opcode::LoadStatic).s64().Inputs(30).TypeId(83);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(55, Opcode::Phi).s64().Inputs({{2, 5}, {5, 24}});
            INST(24, Opcode::Add).s64().Inputs(55, 1);
            INST(7, Opcode::StoreStatic).s64().Inputs(30, 24).TypeId(83);
        }
        BASIC_BLOCK(4, 4, 5)
        {
            INST(45, Opcode::Phi).s64().Inputs({{3, 1}, {4, 25}});
            INST(25, Opcode::Mul).s64().Inputs(45, 45);
            INST(31, Opcode::Compare).b().CC(CC_EQ).Inputs(25, 6);
            INST(32, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(31);
        }
        BASIC_BLOCK(5, 3, 6)
        {
            INST(27, Opcode::Compare).b().CC(CC_EQ).Inputs(24, 2);
            INST(28, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0).Inputs(27);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(46, Opcode::Return).s32().Inputs(45);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Do not eliminate loads over runtime calls due to GC relocations
TEST_F(LSETest, PhiOverSaveStates)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(6, Opcode::SafePoint).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(7, Opcode::LoadObject).ref().Inputs(0).TypeId(122);
            INST(8, Opcode::SaveState).Inputs(0, 2, 7).SrcVregs({0, 2, 3});

            INST(9, Opcode::LoadObject).ref().Inputs(7).TypeId(100);

            INST(10, Opcode::SaveState).Inputs(0, 2, 9).SrcVregs({0, 2, 3});
            INST(15, Opcode::LoadObject).s64().Inputs(9).TypeId(133);
            INST(11, Opcode::StoreObject).ref().Inputs(0, 9).TypeId(122);
            INST(12, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0x0).Inputs(15);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(23, Opcode::ReturnVoid).v0id();
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        BASIC_BLOCK(2, 3)
        {
            INST(4, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(25, Opcode::Phi).ref().Inputs(1, 9);
            INST(6, Opcode::SafePoint).Inputs(0, 1, 2, 25).SrcVregs({0, 1, 2, VirtualRegister::BRIDGE});
            INST(8, Opcode::SaveState).Inputs(0, 2, 25).SrcVregs({0, 2, 3});

            INST(9, Opcode::LoadObject).ref().Inputs(25).TypeId(100);

            INST(10, Opcode::SaveState).Inputs(0, 2, 9).SrcVregs({0, 2, 3});
            INST(15, Opcode::LoadObject).s64().Inputs(9).TypeId(133);
            INST(11, Opcode::StoreObject).ref().Inputs(0, 9).TypeId(122);
            INST(12, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0x0).Inputs(15);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(23, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Simple condition phi
TEST_F(LSETest, SimpleConditionPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(5, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(21, Opcode::Add).s32().Inputs(5, 1);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 21);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(22, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(23, Opcode::Return).s32().Inputs(22);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(5, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(21, Opcode::Add).s32().Inputs(5, 1);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 21);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(25, Opcode::Phi).s32().Inputs({{2, 5}, {3, 21}});
            INST(23, Opcode::Return).s32().Inputs(25);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// Simple condition phi
TEST_F(LSETest, SimpleConditionPhi2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(5, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::Add).s32().Inputs(5, 1);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 3, 21);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(6, Opcode::LoadArray).s32().Inputs(0, 3);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(22, Opcode::LoadArray).s32().Inputs(0, 3);
            INST(23, Opcode::Return).s32().Inputs(22);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(5, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::Add).s32().Inputs(5, 1);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 3, 21);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(6, Opcode::LoadArray).s32().Inputs(0, 3);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(25, Opcode::Phi).s32().Inputs({{3, 21}, {4, 6}});
            INST(23, Opcode::Return).s32().Inputs(25);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

TEST_F(LSETest, SimpleConditionPhiMayAlias)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        CONSTANT(8, 0x8).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(5, 8);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(21, Opcode::Add).s32().Inputs(5, 1);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 2, 21);
            INST(6, Opcode::StoreArray).s32().Inputs(0, 1, 1);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(22, Opcode::LoadArray).s32().Inputs(0, 2);
            INST(23, Opcode::Return).s32().Inputs(22);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Simple condition phi, check bridges
TEST_F(LSETest, SimpleConditionPhiWithRefInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        PARAMETER(3, 3).ref();
        CONSTANT(4, 0x4).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::StoreArray).ref().Inputs(1, 0, 2);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(0, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::StoreArray).ref().Inputs(1, 0, 3);
            INST(8, Opcode::SafePoint).Inputs(1).SrcVregs({1});
        }
        BASIC_BLOCK(3, -1)
        {
            INST(21, Opcode::SafePoint).Inputs(1).SrcVregs({1});
            INST(22, Opcode::LoadArray).ref().Inputs(1, 0);
            INST(23, Opcode::SaveState).Inputs(22).SrcVregs({22});
            INST(24, Opcode::CallStatic).v0id().InputsAutoType(22, 23);
            INST(25, Opcode::Return).ref().Inputs(22);
        }
    }
    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        PARAMETER(3, 3).ref();
        CONSTANT(4, 0x4).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::StoreArray).ref().Inputs(1, 0, 2);
            INST(9, Opcode::Compare).b().CC(CC_GT).Inputs(0, 4);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::StoreArray).ref().Inputs(1, 0, 3);
            INST(8, Opcode::SafePoint).Inputs(1, 3).SrcVregs({1, VirtualRegister::BRIDGE});
        }
        BASIC_BLOCK(3, -1)
        {
            INST(26, Opcode::Phi).ref().Inputs({{2, 2}, {4, 3}});
            INST(21, Opcode::SafePoint).Inputs(1, 26).SrcVregs({1, VirtualRegister::BRIDGE});
            INST(23, Opcode::SaveState).Inputs(26).SrcVregs({22});
            INST(24, Opcode::CallStatic).v0id().InputsAutoType(26, 23);
            INST(25, Opcode::Return).ref().Inputs(26);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

/// SaveStateDeoptimize does not prevent elimination and does not need bridges
TEST_F(LSETest, EliminateOverSaveStateDeoptimize)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
            INST(11, Opcode::StoreObject).s32().Inputs(0, 2).TypeId(123);
            INST(12, Opcode::LoadObject).ref().Inputs(0).TypeId(124);

            INST(7, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(8, Opcode::LoadObject).ref().Inputs(0).TypeId(122);       // store-load pair
            INST(14, Opcode::StoreObject).s32().Inputs(0, 3).TypeId(123);  // store-store pair
            INST(15, Opcode::LoadObject).ref().Inputs(0).TypeId(124);      // load-load pair

            INST(16, Opcode::SaveState).Inputs(12, 8, 15).SrcVregs({0, 1, 2});
            INST(17, Opcode::CallStatic).ref().InputsAutoType(12, 8, 15, 16);

            INST(10, Opcode::Return).ref().Inputs(17);
        }
    }

    Graph *graph_lsed = CreateEmptyGraph();
    GRAPH(graph_lsed)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::StoreObject).ref().Inputs(0, 1).TypeId(122);
            INST(12, Opcode::LoadObject).ref().Inputs(0).TypeId(124);

            INST(7, Opcode::SaveStateDeoptimize).Inputs(0).SrcVregs({0});
            INST(14, Opcode::StoreObject).s32().Inputs(0, 3).TypeId(123);

            INST(16, Opcode::SaveState).Inputs(12, 1, 12).SrcVregs({0, 1, 2});
            INST(17, Opcode::CallStatic).ref().InputsAutoType(12, 1, 12, 16);

            INST(10, Opcode::Return).ref().Inputs(17);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_lsed));
}

// NOLINTEND(readability-magic-numbers)

}  //  namespace panda::compiler
