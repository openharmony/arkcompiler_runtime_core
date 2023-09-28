/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "optimizer/optimizations/adjust_arefs.h"

namespace panda::compiler {
class AdjustRefsTest : public GraphTest {
public:
    AdjustRefsTest() = default;
};

// NOLINTBEGIN(readability-magic-numbers)
/* One block, continuous chain */
TEST_F(AdjustRefsTest, OneBlockContinuousChain)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();

        BASIC_BLOCK(3, 4, 5)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 40);
            INST(11, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(12, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(13, Opcode::StoreArray).u64().Inputs(0, 1, 2);
            INST(14, Opcode::StoreArray).u64().Inputs(0, 1, 2);
            INST(15, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(40, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(50, Opcode::ReturnVoid);
        }
    }

    Graph *graph_et = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();

        BASIC_BLOCK(3, 4, 5)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 40);
            INST(11, Opcode::AddI).ptr().Inputs(0).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12, Opcode::Load).u64().Inputs(11, 1);
            INST(13, Opcode::Load).u64().Inputs(11, 1);
            INST(14, Opcode::Store).u64().Inputs(11, 1, 2);
            INST(15, Opcode::Store).u64().Inputs(11, 1, 2);
            INST(16, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(16).Imm(0);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(40, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(50, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_et));
}

/* One block, broken chain */
TEST_F(AdjustRefsTest, OneBlockBrokenChain)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();

        BASIC_BLOCK(3, 4, 5)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 40);
            INST(11, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(12, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(13, Opcode::SafePoint).NoVregs();
            INST(14, Opcode::StoreArray).u64().Inputs(0, 1, 2);
            INST(15, Opcode::StoreArray).u64().Inputs(0, 1, 2);
            INST(16, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(16).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(40, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(50, Opcode::ReturnVoid);
        }
    }

    Graph *graph_et = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();

        BASIC_BLOCK(3, 4, 5)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 40);
            INST(11, Opcode::AddI).ptr().Inputs(0).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12, Opcode::Load).u64().Inputs(11, 1);
            INST(13, Opcode::Load).u64().Inputs(11, 1);

            INST(14, Opcode::SafePoint).NoVregs();

            INST(15, Opcode::AddI).ptr().Inputs(0).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(16, Opcode::Store).u64().Inputs(15, 1, 2);
            INST(17, Opcode::Store).u64().Inputs(15, 1, 2);

            INST(18, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(18).Imm(0);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(40, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(50, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_et));
}

/* one head, the chain spans across multiple blocks, not broken */
TEST_F(AdjustRefsTest, MultipleBlockContinuousChain)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();

        BASIC_BLOCK(3, 4, 10)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 90);
            INST(11, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(15, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(41, Opcode::Compare).b().Inputs(10, 1);
            INST(42, Opcode::IfImm).CC(CC_NE).Inputs(41).Imm(0);
        }
        BASIC_BLOCK(5, 9)
        {
            INST(51, Opcode::LoadArray).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(61, Opcode::LoadArray).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(9, 3)
        {
            INST(90, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(10, 1)
        {
            INST(100, Opcode::ReturnVoid);
        }
    }

    Graph *graph_et = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();
        BASIC_BLOCK(3, 4, 10)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 90);
            INST(11, Opcode::AddI).ptr().Inputs(0).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12, Opcode::Load).u64().Inputs(11, 1);
            INST(15, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(41, Opcode::Compare).b().Inputs(10, 1);
            INST(42, Opcode::IfImm).CC(CC_NE).Inputs(41).Imm(0);
        }
        BASIC_BLOCK(5, 9)
        {
            INST(51, Opcode::Load).u64().Inputs(11, 1);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(61, Opcode::Load).u64().Inputs(11, 1);
        }
        BASIC_BLOCK(9, 3)
        {
            INST(90, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(10, 1)
        {
            INST(100, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_et));
}

/* one head, the chain spans across multiple blocks,
 * broken in one of the dominated basic blocks */
TEST_F(AdjustRefsTest, MultipleBlockBrokenChain)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();

        BASIC_BLOCK(3, 4, 10)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 90);
            INST(11, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(15, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(41, Opcode::Compare).b().Inputs(10, 1);
            INST(42, Opcode::IfImm).CC(CC_NE).Inputs(41).Imm(0);
        }
        BASIC_BLOCK(5, 9)
        {
            INST(51, Opcode::LoadArray).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(13, Opcode::SafePoint).NoVregs();
            INST(61, Opcode::LoadArray).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(9, 3)
        {
            INST(90, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(10, 1)
        {
            INST(100, Opcode::ReturnVoid);
        }
    }

    Graph *graph_et = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10).s32();
        BASIC_BLOCK(3, 4, 10)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 90);
            INST(11, Opcode::AddI).ptr().Inputs(0).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12, Opcode::Load).u64().Inputs(11, 1);
            INST(15, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(41, Opcode::Compare).b().Inputs(10, 1);
            INST(42, Opcode::IfImm).CC(CC_NE).Inputs(41).Imm(0);
        }
        BASIC_BLOCK(5, 9)
        {
            INST(51, Opcode::Load).u64().Inputs(11, 1);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(13, Opcode::SafePoint).NoVregs();
            INST(61, Opcode::LoadArray).u64().Inputs(0, 1);
        }
        BASIC_BLOCK(9, 3)
        {
            INST(90, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(10, 1)
        {
            INST(100, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_et));
}

TEST_F(AdjustRefsTest, ProcessIndex)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    uint64_t offset1 = 10;
    uint64_t offset2 = 1;
    uint64_t offset3 = 20;
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u8();
        PARAMETER(4, 4).u32();
        BASIC_BLOCK(3, 1)
        {
            INST(6, Opcode::AddI).s32().Inputs(1).Imm(offset1);
            INST(7, Opcode::StoreArray).u64().Inputs(0, 6, 2);
            INST(8, Opcode::SubI).s32().Inputs(1).Imm(offset2);
            INST(9, Opcode::StoreArray).u8().Inputs(0, 8, 3);
            INST(10, Opcode::SubI).s32().Inputs(1).Imm(offset3);
            INST(11, Opcode::StoreArray).u32().Inputs(0, 10, 4);
            INST(100, Opcode::ReturnVoid);
        }
    }

    Graph *graph_et = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    auto arr_data = graph->GetRuntime()->GetArrayDataOffset(graph->GetArch());
    uint64_t new_offset1 = arr_data + (offset1 << 3U);
    uint64_t new_offset2 = arr_data - (offset2 << 0U);
    uint64_t new_offset3 = (offset3 << 2U) - arr_data;
    GRAPH(graph_et)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u8();
        PARAMETER(4, 4).u32();
        BASIC_BLOCK(3, 1)
        {
            INST(56, Opcode::AddI).ptr().Inputs(0).Imm(new_offset1);
            INST(57, Opcode::Store).u64().Inputs(56, 1, 2);
            INST(58, Opcode::AddI).ptr().Inputs(0).Imm(new_offset2);
            INST(59, Opcode::Store).u8().Inputs(58, 1, 3);
            INST(60, Opcode::SubI).ptr().Inputs(0).Imm(new_offset3);
            INST(61, Opcode::Store).u32().Inputs(60, 1, 4);
            INST(100, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_et));
}

// BB 4 dominates BB 9, but there is SafePoint in BB 6 and reference cannot be adjusted
TEST_F(AdjustRefsTest, TriangleBrokenChain)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).u64();
        CONSTANT(3, 10);

        BASIC_BLOCK(3, 4, 10)
        {
            INST(10, Opcode::Phi).s32().Inputs(1, 90);
            INST(11, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(15, Opcode::Compare).b().Inputs(10, 1);
            INST(19, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }
        BASIC_BLOCK(4, 9, 6)
        {
            INST(41, Opcode::Compare).b().Inputs(10, 1);
            INST(42, Opcode::IfImm).CC(CC_NE).Inputs(41).Imm(0);
        }
        BASIC_BLOCK(6, 9)
        {
            INST(13, Opcode::SafePoint).NoVregs();
        }
        BASIC_BLOCK(9, 3)
        {
            INST(61, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(90, Opcode::Add).s32().Inputs(10, 3);
        }
        BASIC_BLOCK(10, 1)
        {
            INST(100, Opcode::ReturnVoid);
        }
    }

    auto clone = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(graph->RunPass<AdjustRefs>());
    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
