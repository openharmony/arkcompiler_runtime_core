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
#include "optimizer/optimizations/adjust_arefs.h"

namespace panda::compiler {
class AdjustRefsTest : public GraphTest {
public:
    AdjustRefsTest() {}
};

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

}  // namespace panda::compiler
