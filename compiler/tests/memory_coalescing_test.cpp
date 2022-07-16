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
#include "optimizer/optimizations/memory_coalescing.h"
#include "optimizer/optimizations/scheduler.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"

namespace panda::compiler {
class MemoryCoalescingTest : public GraphTest {
#ifndef NDEBUG
public:
    MemoryCoalescingTest()
    {
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        GetGraph()->SetLowLevelInstructionsEnabled();
    }
#endif
};

TEST_F(MemoryCoalescingTest, ImmidiateLoads)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0).TypeId(77);
            INST(41, Opcode::SaveState).Inputs(3).SrcVregs({7});
            INST(42, Opcode::NullCheck).ref().Inputs(3, 41);
            INST(225, Opcode::LoadArrayI).s64().Inputs(42).Imm(0x0);
            INST(227, Opcode::LoadArrayI).s64().Inputs(42).Imm(0x1);

            INST(51, Opcode::Add).s64().Inputs(225, 227);
            INST(229, Opcode::StoreArrayI).s64().Inputs(42, 51).Imm(0x0);
            INST(230, Opcode::StoreArrayI).s64().Inputs(42, 51).Imm(0x1);
            INST(40, Opcode::Return).s64().Inputs(51);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0).TypeId(77);
            INST(41, Opcode::SaveState).Inputs(3).SrcVregs({7});
            INST(42, Opcode::NullCheck).ref().Inputs(3, 41);
            INST(231, Opcode::LoadArrayPairI).s64().Inputs(42).Imm(0x0);
            INST(232, Opcode::LoadPairPart).s64().Inputs(231).Imm(0x0);
            INST(233, Opcode::LoadPairPart).s64().Inputs(231).Imm(0x1);

            INST(51, Opcode::Add).s64().Inputs(232, 233);
            INST(234, Opcode::StoreArrayPairI).s64().Inputs(42, 51, 51).Imm(0x0);
            INST(40, Opcode::Return).s64().Inputs(51);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, LoopLoadCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(5, 0).ref();
        CONSTANT(6, 0x0).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(35, Opcode::SaveState).Inputs(5).SrcVregs({1});
            INST(36, Opcode::NullCheck).ref().Inputs(5, 35);
        }
        BASIC_BLOCK(3, 3, 7)
        {
            INST(9, Opcode::Phi).s32().Inputs({{2, 6}, {3, 64}});
            INST(10, Opcode::Phi).s64().Inputs({{2, 6}, {3, 28}});
            INST(19, Opcode::LoadArray).s64().Inputs(36, 9);
            INST(63, Opcode::AddI).s32().Inputs(9).Imm(0x1);
            INST(26, Opcode::LoadArray).s64().Inputs(36, 63);
            INST(27, Opcode::Add).s64().Inputs(26, 19);
            INST(28, Opcode::Add).s64().Inputs(27, 10);
            INST(64, Opcode::AddI).s32().Inputs(9).Imm(0x2);
            INST(31, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_LT).Imm(0x4).Inputs(64);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(33, Opcode::Return).s32().Inputs(28);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(5, 0).ref();
        CONSTANT(6, 0x0).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(35, Opcode::SaveState).Inputs(5).SrcVregs({1});
            INST(36, Opcode::NullCheck).ref().Inputs(5, 35);
        }
        BASIC_BLOCK(3, 3, 7)
        {
            INST(9, Opcode::Phi).s32().Inputs({{2, 6}, {3, 64}});
            INST(10, Opcode::Phi).s64().Inputs({{2, 6}, {3, 28}});
            INST(65, Opcode::LoadArrayPair).s64().Inputs(36, 9);
            INST(66, Opcode::LoadPairPart).s64().Inputs(65).Imm(0x0);
            INST(67, Opcode::LoadPairPart).s64().Inputs(65).Imm(0x1);
            INST(63, Opcode::AddI).s32().Inputs(9).Imm(0x1);
            INST(27, Opcode::Add).s64().Inputs(67, 66);
            INST(28, Opcode::Add).s64().Inputs(27, 10);
            INST(64, Opcode::AddI).s32().Inputs(9).Imm(0x2);
            INST(31, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_LT).Imm(0x4).Inputs(64);
        }
        BASIC_BLOCK(7, -1)
        {
            INST(33, Opcode::Return).s32().Inputs(28);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, LoopStoreCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(4, 0x0).s64();
        CONSTANT(5, 0x1).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::LenArray).s32().Inputs(0);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(6, Opcode::Phi).s32().Inputs({{2, 4}, {3, 34}});
            INST(7, Opcode::Phi).s32().Inputs({{2, 5}, {3, 24}});
            INST(8, Opcode::Phi).s32().Inputs({{2, 5}, {3, 25}});
            INST(17, Opcode::StoreArray).s32().Inputs(0, 6, 7);
            INST(33, Opcode::AddI).s32().Inputs(6).Imm(0x1);
            INST(23, Opcode::StoreArray).s32().Inputs(0, 33, 8);
            INST(24, Opcode::Add).s32().Inputs(7, 8);
            INST(25, Opcode::Add).s32().Inputs(8, 24);
            INST(34, Opcode::AddI).s32().Inputs(6).Imm(0x2);
            INST(35, Opcode::If).SrcType(DataType::INT32).CC(CC_LT).Inputs(34, 3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(29, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(4, 0x0).s64();
        CONSTANT(5, 0x1).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::LenArray).s32().Inputs(0);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(6, Opcode::Phi).s32().Inputs({{2, 4}, {3, 34}});
            INST(7, Opcode::Phi).s32().Inputs({{2, 5}, {3, 24}});
            INST(8, Opcode::Phi).s32().Inputs({{2, 5}, {3, 25}});
            INST(33, Opcode::AddI).s32().Inputs(6).Imm(0x1);
            INST(36, Opcode::StoreArrayPair).s32().Inputs(0, 6, 7, 8);
            INST(24, Opcode::Add).s32().Inputs(7, 8);
            INST(25, Opcode::Add).s32().Inputs(8, 24);
            INST(34, Opcode::AddI).s32().Inputs(6).Imm(0x2);
            INST(35, Opcode::If).SrcType(DataType::INT32).CC(CC_LT).Inputs(34, 3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(29, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, AliasedAccess)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(26, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x0);
            INST(28, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);
            INST(21, Opcode::Add).s64().Inputs(28, 26);
            INST(22, Opcode::Return).s64().Inputs(21);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(29, Opcode::LoadArrayPairI).s64().Inputs(0).Imm(0x0);
            INST(30, Opcode::LoadPairPart).s64().Inputs(29).Imm(0x0);
            INST(31, Opcode::LoadPairPart).s64().Inputs(29).Imm(0x1);
            INST(21, Opcode::Add).s64().Inputs(31, 30);
            INST(22, Opcode::Return).s64().Inputs(21);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, OnlySingleCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(26, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x0);
            INST(28, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);

            INST(30, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);
            INST(21, Opcode::Add).s64().Inputs(28, 26);
            INST(22, Opcode::Add).s64().Inputs(30, 21);
            INST(23, Opcode::Return).s64().Inputs(22);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(31, Opcode::LoadArrayPairI).s64().Inputs(0).Imm(0x0);
            INST(33, Opcode::LoadPairPart).s64().Inputs(31).Imm(0x0);
            INST(32, Opcode::LoadPairPart).s64().Inputs(31).Imm(0x1);

            INST(30, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);
            INST(21, Opcode::Add).s64().Inputs(32, 33);
            INST(22, Opcode::Add).s64().Inputs(30, 21);
            INST(23, Opcode::Return).s64().Inputs(22);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, PseudoParts)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x0);
            INST(2, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);

            INST(3, Opcode::StoreArrayI).s64().Inputs(0, 2).Imm(0x0);
            INST(4, Opcode::StoreArrayI).s64().Inputs(0, 1).Imm(0x1);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::LoadArrayPairI).s64().Inputs(0).Imm(0x0);
            INST(7, Opcode::LoadPairPart).s64().Inputs(6).Imm(0x0);
            INST(8, Opcode::LoadPairPart).s64().Inputs(6).Imm(0x1);

            INST(9, Opcode::StoreArrayPairI).s64().Inputs(0, 8, 7).Imm(0x0);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GetGraph()->RunPass<Scheduler>();
    RegAlloc(GetGraph());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, UnalignedStores)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::AddI).s32().Inputs(1).Imm(0x4);
            INST(5, Opcode::StoreArray).s32().Inputs(0, 4, 2);
            INST(6, Opcode::AddI).s32().Inputs(1).Imm(0x5);
            INST(7, Opcode::StoreArray).s32().Inputs(0, 6, 3);

            INST(8, Opcode::StoreArray).s32().Inputs(0, 4, 3);
            INST(9, Opcode::AddI).s32().Inputs(4).Imm(0x1);
            INST(10, Opcode::StoreArray).s32().Inputs(0, 9, 2);
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::AddI).s32().Inputs(1).Imm(0x4);
            INST(6, Opcode::AddI).s32().Inputs(1).Imm(0x5);
            INST(12, Opcode::StoreArrayPair).s32().Inputs(0, 4, 2, 3);

            INST(9, Opcode::AddI).s32().Inputs(4).Imm(0x1);
            INST(13, Opcode::StoreArrayPair).s32().Inputs(0, 4, 3, 2);
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, NoAlignmentTestI)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x2);
            INST(2, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);
            INST(3, Opcode::Add).s64().Inputs(2, 1);
            INST(4, Opcode::Return).s64().Inputs(3);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::LoadArrayPairI).s64().Inputs(0).Imm(0x1);
            INST(6, Opcode::LoadPairPart).s64().Inputs(5).Imm(0x0);
            INST(7, Opcode::LoadPairPart).s64().Inputs(5).Imm(0x1);
            INST(3, Opcode::Add).s64().Inputs(6, 7);
            INST(4, Opcode::Return).s64().Inputs(3);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, StoresRoundedByLoads)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x2);
            INST(4, Opcode::StoreArrayI).s64().Inputs(0, 1).Imm(0x2);
            INST(5, Opcode::StoreArrayI).s64().Inputs(0, 2).Imm(0x3);
            INST(6, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x3);
            INST(7, Opcode::Add).s64().Inputs(3, 6);
            INST(8, Opcode::Return).s64().Inputs(7);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x2);
            INST(9, Opcode::StoreArrayPairI).s64().Inputs(0, 1, 2).Imm(0x2);
            INST(6, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x3);
            INST(7, Opcode::Add).s64().Inputs(3, 6);
            INST(8, Opcode::Return).s64().Inputs(7);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, UnalignedInLoop)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        CONSTANT(3, 0xa).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(6, Opcode::NewArray).ref().Inputs(44, 3).TypeId(77);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(7, Opcode::Phi).s32().Inputs({{2, 0}, {3, 33}});
            INST(19, Opcode::StoreArray).s32().Inputs(6, 7, 7);
            INST(33, Opcode::AddI).s32().Inputs(7).Imm(0x1);
            INST(25, Opcode::StoreArray).s32().Inputs(6, 33, 7);
            INST(34, Opcode::If).SrcType(DataType::INT32).CC(CC_GT).Inputs(1, 33);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(29, Opcode::Return).s32().Inputs(33);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        CONSTANT(3, 0xa).s64();
        BASIC_BLOCK(2, 3)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(6, Opcode::NewArray).ref().Inputs(44, 3).TypeId(77);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(7, Opcode::Phi).s32().Inputs({{2, 0}, {3, 33}});
            INST(33, Opcode::AddI).s32().Inputs(7).Imm(0x1);
            INST(35, Opcode::StoreArrayPair).s32().Inputs(6, 7, 7, 7);
            INST(34, Opcode::If).SrcType(DataType::INT32).CC(CC_GT).Inputs(1, 33);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(29, Opcode::Return).s32().Inputs(33);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, IndexInference)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::AddI).s32().Inputs(1).Imm(0x1);
            INST(7, Opcode::LoadArray).s32().Inputs(0, 1);
            INST(8, Opcode::LoadArray).s32().Inputs(0, 6);
            INST(9, Opcode::StoreArray).s32().Inputs(0, 1, 7);
            INST(10, Opcode::StoreArray).s32().Inputs(0, 6, 8);
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::AddI).s32().Inputs(1).Imm(0x1);
            INST(12, Opcode::LoadArrayPair).s32().Inputs(0, 1);
            INST(14, Opcode::LoadPairPart).s32().Inputs(12).Imm(0x0);
            INST(13, Opcode::LoadPairPart).s32().Inputs(12).Imm(0x1);
            INST(15, Opcode::StoreArrayPair).s32().Inputs(0, 1, 14, 13);
            INST(11, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, SimplePlaceFinding)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::LoadArray).s32().Inputs(0, 1);
            INST(6, Opcode::SubI).s32().Inputs(1).Imm(1);
            INST(8, Opcode::LoadArray).s32().Inputs(0, 6);
            INST(9, Opcode::Add).s32().Inputs(7, 8);
            INST(11, Opcode::Return).s32().Inputs(9);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::SubI).s32().Inputs(1).Imm(1);
            INST(12, Opcode::LoadArrayPair).s32().Inputs(0, 6);
            INST(14, Opcode::LoadPairPart).s32().Inputs(12).Imm(0x0);
            INST(13, Opcode::LoadPairPart).s32().Inputs(12).Imm(0x1);
            INST(9, Opcode::Add).s32().Inputs(13, 14);
            INST(11, Opcode::Return).s32().Inputs(9);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, ObjectAccessesCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0x2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(50, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(50).TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(44, 1, 50).TypeId(88);
            INST(38, Opcode::LoadArrayI).ref().Inputs(0).Imm(0x0);
            INST(40, Opcode::LoadArrayI).ref().Inputs(0).Imm(0x1);
            INST(41, Opcode::StoreArrayI).ref().Inputs(4, 38).Imm(0x0);
            INST(42, Opcode::StoreArrayI).ref().Inputs(4, 40).Imm(0x1);
            INST(27, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0x2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(50, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(444, Opcode::LoadAndInitClass).ref().Inputs(50).TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(444, 1, 50).TypeId(88);
            INST(43, Opcode::LoadArrayPairI).ref().Inputs(0).Imm(0x0);
            INST(45, Opcode::LoadPairPart).ref().Inputs(43).Imm(0x0);
            INST(44, Opcode::LoadPairPart).ref().Inputs(43).Imm(0x1);
            INST(46, Opcode::StoreArrayPairI).ref().Inputs(4, 45, 44).Imm(0x0);
            INST(27, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    options.SetCompilerMemoryCoalescingObjects(false);
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    options.SetCompilerMemoryCoalescingObjects(true);
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, AllowedVolatileReordering)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).SrcVregs({});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);

            INST(3, Opcode::NewArray).ref().Inputs(5, 0).TypeId(77);
            // Can reorder Volatile Store (v226) and Normal Load (v227)
            INST(225, Opcode::LoadArrayI).s64().Inputs(3).Imm(0x0);
            INST(226, Opcode::StoreStatic).s64().Volatile().Inputs(5, 225).TypeId(103);
            INST(227, Opcode::LoadArrayI).s64().Inputs(3).Imm(0x1);

            INST(51, Opcode::Add).s64().Inputs(225, 227);
            // Can reorder Normal Store (v229) and Volatile Load (v230)
            INST(229, Opcode::StoreArrayI).s64().Inputs(3, 51).Imm(0x0);
            INST(230, Opcode::LoadStatic).s64().Inputs(5).Volatile().TypeId(107);
            INST(231, Opcode::StoreArrayI).s64().Inputs(3, 230).Imm(0x1);
            INST(40, Opcode::Return).s64().Inputs(51);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).SrcVregs({});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);

            INST(3, Opcode::NewArray).ref().Inputs(5, 0).TypeId(77);
            INST(232, Opcode::LoadArrayPairI).s64().Inputs(3).Imm(0x0);
            INST(234, Opcode::LoadPairPart).s64().Inputs(232).Imm(0x0);
            INST(233, Opcode::LoadPairPart).s64().Inputs(232).Imm(0x1);
            INST(226, Opcode::StoreStatic).s64().Volatile().Inputs(5, 234).TypeId(103);

            INST(51, Opcode::Add).s64().Inputs(234, 233);
            INST(230, Opcode::LoadStatic).s64().Inputs(5).Volatile().TypeId(107);
            INST(235, Opcode::StoreArrayPairI).s64().Inputs(3, 51, 230).Imm(0x0);
            INST(40, Opcode::Return).s64().Inputs(51);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, AllowedVolatileReordering2)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).SrcVregs({});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);

            INST(3, Opcode::NewArray).ref().Inputs(5, 0).TypeId(77);
            // We can reorder v225 and v226 but not v226 and v227
            INST(225, Opcode::LoadArrayI).s64().Inputs(3).Imm(0x0);
            INST(226, Opcode::LoadStatic).s64().Inputs(5).Volatile().TypeId(103);
            INST(227, Opcode::LoadArrayI).s64().Inputs(3).Imm(0x1);

            INST(51, Opcode::Add).s64().Inputs(225, 227);
            // We can reorder v230 and v231 but not v229 and v230
            INST(229, Opcode::StoreArrayI).s64().Inputs(3, 51).Imm(0x0);
            INST(230, Opcode::StoreStatic).s64().Inputs(5).Volatile().Inputs(226).TypeId(105);
            INST(231, Opcode::StoreArrayI).s64().Inputs(3, 51).Imm(0x1);
            INST(40, Opcode::Return).s64().Inputs(51);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).SrcVregs({});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);

            INST(3, Opcode::NewArray).ref().Inputs(5, 0).TypeId(77);
            INST(226, Opcode::LoadStatic).s64().Inputs(5).Volatile().TypeId(103);
            INST(235, Opcode::LoadArrayPairI).s64().Inputs(3).Imm(0x0);
            INST(237, Opcode::LoadPairPart).s64().Inputs(235).Imm(0x0);
            INST(236, Opcode::LoadPairPart).s64().Inputs(235).Imm(0x1);

            INST(51, Opcode::Add).s64().Inputs(237, 236);
            INST(238, Opcode::StoreArrayPairI).s64().Inputs(3, 51, 51).Imm(0x0);
            INST(230, Opcode::StoreStatic).s64().Inputs(5).Volatile().Inputs(226).TypeId(105);
            INST(40, Opcode::Return).s64().Inputs(51);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, UnrolledLoop)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0x2a).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(50, Opcode::SaveState).Inputs(1).SrcVregs({0});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(50).TypeId(68);
            INST(2, Opcode::NewArray).ref().Inputs(44, 3, 50).TypeId(77);
            INST(10, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_LE).Imm(0x0).Inputs(0);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(11, Opcode::Phi).s32().Inputs({{2, 4}, {3, 17}});
            INST(12, Opcode::LoadArray).s32().Inputs(1, 11);
            INST(13, Opcode::StoreArray).s32().Inputs(2, 11, 12);
            INST(14, Opcode::AddI).s32().Inputs(11).Imm(1);

            INST(15, Opcode::LoadArray).s32().Inputs(1, 14);
            INST(16, Opcode::StoreArray).s32().Inputs(2, 14, 15);
            INST(17, Opcode::AddI).s32().Inputs(11).Imm(2);

            INST(30, Opcode::If).SrcType(DataType::INT32).CC(CC_GE).Inputs(17, 3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(40, Opcode::ReturnVoid);
        }
    }
    Graph *opt_graph = CreateEmptyGraph();
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).ref();
        CONSTANT(3, 0x2a).s64();
        CONSTANT(4, 0x0).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(50, Opcode::SaveState).Inputs(1).SrcVregs({0});
            INST(444, Opcode::LoadAndInitClass).ref().Inputs(50).TypeId(68);
            INST(2, Opcode::NewArray).ref().Inputs(444, 3, 50).TypeId(77);
            INST(10, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_LE).Imm(0x0).Inputs(0);
        }
        BASIC_BLOCK(3, 3, 4)
        {
            INST(11, Opcode::Phi).s32().Inputs({{2, 4}, {3, 17}});
            INST(44, Opcode::LoadArrayPair).s32().Inputs(1, 11);
            INST(46, Opcode::LoadPairPart).s32().Inputs(44).Imm(0x0);
            INST(45, Opcode::LoadPairPart).s32().Inputs(44).Imm(0x1);
            INST(14, Opcode::AddI).s32().Inputs(11).Imm(1);

            INST(47, Opcode::StoreArrayPair).s32().Inputs(2, 11, 46, 45);
            INST(17, Opcode::AddI).s32().Inputs(11).Imm(2);

            INST(30, Opcode::If).SrcType(DataType::INT32).CC(CC_GE).Inputs(17, 3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(40, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), opt_graph));
}

TEST_F(MemoryCoalescingTest, CoalescingOverSaveState)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({6});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);

            INST(41, Opcode::SaveState).Inputs(0).SrcVregs({6});
            INST(43, Opcode::LenArray).s32().Inputs(2);
            INST(241, Opcode::BoundsCheckI).s32().Inputs(43, 41).Imm(0x0);
            INST(242, Opcode::LoadArrayI).s64().Inputs(2).Imm(0x0);

            INST(47, Opcode::SaveState).Inputs(242, 0).SrcVregs({3, 6});
            INST(49, Opcode::LenArray).s32().Inputs(2);
            INST(244, Opcode::BoundsCheckI).s32().Inputs(49, 47).Imm(0x1);
            INST(245, Opcode::LoadArrayI).s64().Inputs(2).Imm(0x1);

            INST(53, Opcode::Add).s64().Inputs(242, 245);
            INST(40, Opcode::Return).s64().Inputs(53);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, AlignmentTest)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(26, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x2);
            INST(28, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);
            INST(21, Opcode::Add).s64().Inputs(28, 26);
            INST(22, Opcode::Return).s64().Inputs(21);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, AliasedStore)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(26, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x0);
            INST(13, Opcode::StoreArray).s64().Inputs(0, 1, 2);
            INST(28, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x5);
            INST(29, Opcode::LoadArrayI).s64().Inputs(0).Imm(0x1);
            INST(21, Opcode::Add).s64().Inputs(28, 26);
            INST(24, Opcode::Add).s64().Inputs(21, 29);
            INST(22, Opcode::Return).s64().Inputs(24);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, TypeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::AddI).s32().Inputs(1).Imm(0x1);
            INST(7, Opcode::LoadArray).s8().Inputs(0, 1);
            INST(8, Opcode::LoadArray).s8().Inputs(0, 6);
            INST(9, Opcode::LoadArrayI).s16().Inputs(0).Imm(0x4);
            INST(10, Opcode::LoadArrayI).s16().Inputs(0).Imm(0x5);
            INST(11, Opcode::StoreArray).s16().Inputs(0, 1, 9);
            INST(12, Opcode::StoreArray).s16().Inputs(0, 6, 10);
            INST(13, Opcode::StoreArrayI).s8().Inputs(0, 7).Imm(0x4);
            INST(14, Opcode::StoreArrayI).s8().Inputs(0, 8).Imm(0x5);
            INST(15, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, ProhibitedVolatileReordering)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).SrcVregs({});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(0);

            INST(3, Opcode::NewArray).ref().Inputs(5, 0).TypeId(77);
            INST(225, Opcode::LoadArrayI).s64().Inputs(3).Imm(0x0);
            // v50 is needed to prevent reordering v225 with v226
            INST(50, Opcode::Add).s64().Inputs(225, 225);
            INST(226, Opcode::LoadStatic).s64().Inputs(5).Volatile().TypeId(103);
            INST(227, Opcode::LoadArrayI).s64().Inputs(3).Imm(0x1);

            INST(51, Opcode::Add).s64().Inputs(50, 227);
            INST(229, Opcode::StoreArrayI).s64().Inputs(3, 51).Imm(0x0);
            INST(230, Opcode::StoreStatic).s64().Inputs(5).Volatile().Inputs(226).TypeId(105);
            // v51 is needed to prevent reordering v231 and v230
            INST(52, Opcode::Add).s64().Inputs(50, 51);
            INST(231, Opcode::StoreArrayI).s64().Inputs(3, 52).Imm(0x1);
            INST(40, Opcode::Return).s64().Inputs(51);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, LoweringDominance)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::SubI).s32().Inputs(1).Imm(1);

            INST(9, Opcode::LoadArray).s32().Inputs(0, 1);
            INST(10, Opcode::ShlI).s32().Inputs(9).Imm(24);
            INST(16, Opcode::StoreArray).s32().Inputs(0, 8, 10);
            INST(11, Opcode::AShrI).s32().Inputs(9).Imm(28);

            INST(12, Opcode::AddI).s64().Inputs(1).Imm(1);
            INST(13, Opcode::LoadArray).s32().Inputs(0, 12);

            INST(14, Opcode::Add).s32().Inputs(10, 11);
            INST(15, Opcode::Return).s32().Inputs(14);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, LoweringDominance2)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        PARAMETER(3, 3).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::SubI).s32().Inputs(3).Imm(1);

            INST(9, Opcode::LoadArrayI).s32().Inputs(0).Imm(0x0);
            INST(10, Opcode::StoreArray).s32().Inputs(1, 8, 9);

            INST(11, Opcode::LoadArrayI).s32().Inputs(0).Imm(0x1);
            INST(12, Opcode::StoreArray).s32().Inputs(2, 8, 11);

            INST(15, Opcode::Return).s32().Inputs(3);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, CoalescingLoadsOverSaveState)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(9, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(10, Opcode::SaveState).SrcVregs({9});
            INST(11, Opcode::AddI).s64().Inputs(1).Imm(1);
            INST(12, Opcode::LoadArray).ref().Inputs(0, 11);
            INST(13, Opcode::SaveState).SrcVregs({9, 12});
            INST(15, Opcode::Return).s32().Inputs(1);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, CoalescingPhiAsUser)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        BASIC_BLOCK(2, 3, 2)
        {
            INST(9, Opcode::Phi).s32().Inputs({{0, 3}, {2, 10}});
            INST(10, Opcode::LoadArray).s32().Inputs(0, 9);
            INST(11, Opcode::AddI).s32().Inputs(9).Imm(1);
            INST(12, Opcode::LoadArray).s32().Inputs(0, 11);
            INST(18, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0).Inputs(12);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(20, Opcode::Return).s32().Inputs(2);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
}
}  //  namespace panda::compiler
