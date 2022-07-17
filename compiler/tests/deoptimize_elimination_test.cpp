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
#include "optimizer/optimizations/deoptimize_elimination.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/runtime_interface.h"

namespace panda::compiler {
class DeoptimizeEliminationTest : public CommonTest {
public:
    DeoptimizeEliminationTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

protected:
    Graph *graph_ {nullptr};
};

TEST_F(DeoptimizeEliminationTest, DeoptimizeIfTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0);
        BASIC_BLOCK(2, 1)
        {
            // without users
            INST(2, Opcode::SaveStateDeoptimize).Inputs(1).SrcVregs({1});
            // cleanup delete this
            INST(10, Opcode::SaveState).Inputs(1).SrcVregs({1});

            INST(3, Opcode::SaveStateDeoptimize).Inputs(1).SrcVregs({1});
            INST(4, Opcode::Compare).b().Inputs(0, 1).CC(CC_GT);
            INST(5, Opcode::DeoptimizeIf).Inputs(4, 3);
            // 5 is dominate by 7
            INST(6, Opcode::SaveStateDeoptimize).Inputs(1).SrcVregs({1});
            INST(7, Opcode::DeoptimizeIf).Inputs(4, 6);

            // redundant
            INST(8, Opcode::DeoptimizeIf).Inputs(1, 6);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::SaveStateDeoptimize).Inputs(1).SrcVregs({1});
            INST(4, Opcode::Compare).b().Inputs(0, 1).CC(CC_GT);
            INST(5, Opcode::DeoptimizeIf).Inputs(4, 3);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);

            INST(3, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(4, Opcode::IsMustDeoptimize).b();
            INST(5, Opcode::DeoptimizeIf).Inputs(4, 3);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);

            INST(3, Opcode::NOP);
            INST(4, Opcode::NOP);
            INST(5, Opcode::NOP);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockCallTest)
{
    // Not applied. Call to runtime between guards.
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(20);

            INST(3, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(4, Opcode::IsMustDeoptimize).b();
            INST(5, Opcode::DeoptimizeIf).Inputs(4, 3);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockSeveralGuardsTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(20);

            INST(3, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(4, Opcode::IsMustDeoptimize).b();
            INST(5, Opcode::DeoptimizeIf).Inputs(4, 3);

            INST(11, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(12, Opcode::IsMustDeoptimize).b();
            INST(13, Opcode::DeoptimizeIf).Inputs(12, 11);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().InputsAutoType(20);

            INST(3, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(4, Opcode::IsMustDeoptimize).b();
            INST(5, Opcode::DeoptimizeIf).Inputs(4, 3);

            INST(11, Opcode::NOP);
            INST(12, Opcode::NOP);
            INST(13, Opcode::NOP);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockCallInlinedTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().Inlined().InputsAutoType(20);
            INST(7, Opcode::ReturnInlined).Inputs(0);

            INST(3, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(4, Opcode::IsMustDeoptimize).b();
            INST(5, Opcode::DeoptimizeIf).Inputs(4, 3);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10, 0);
        BASIC_BLOCK(2, 1)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10).SrcVregs({10});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().Inlined().InputsAutoType(20);
            INST(7, Opcode::ReturnInlined).Inputs(0);

            INST(3, Opcode::NOP);
            INST(4, Opcode::NOP);
            INST(5, Opcode::NOP);

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardIfTest)
{
    /*
     * Not applied.
     * bb1
     * guard
     * |   \
     * |   bb2
     * |   runtime call
     * |  /
     * bb3
     * guard
     */
    GRAPH(GetGraph())
    {
        PARAMETER(10, 0).s32();
        CONSTANT(11, 1);
        BASIC_BLOCK(2, 4, 5)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(3, Opcode::Compare).b().CC(CC_LT).Inputs(10, 11);
            INST(4, Opcode::IfImm).CC(CC_NE).Inputs(3).Imm(0);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(20);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(7, Opcode::IsMustDeoptimize).b();
            INST(8, Opcode::DeoptimizeIf).Inputs(7, 6);
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardIfSeveralGuardsTest)
{
    /*
     * bb1
     * guard
     * call
     * guard
     * |   \
     * |   bb2
     * |   some inst
     * |  /
     * bb3
     * guard
     */
    GRAPH(GetGraph())
    {
        PARAMETER(10, 0).s32();
        CONSTANT(11, 1);
        BASIC_BLOCK(2, 4, 5)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(12, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(13, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(14, Opcode::IsMustDeoptimize).b();
            INST(15, Opcode::DeoptimizeIf).Inputs(14, 13);
            INST(3, Opcode::Compare).b().CC(CC_LT).Inputs(10, 11);
            INST(4, Opcode::IfImm).CC(CC_NE).Inputs(3).Imm(0);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().Inlined().InputsAutoType(21);
            INST(16, Opcode::ReturnInlined).Inputs(0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(7, Opcode::IsMustDeoptimize).b();
            INST(8, Opcode::DeoptimizeIf).Inputs(7, 6);
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(10, 0).s32();
        CONSTANT(11, 1);
        BASIC_BLOCK(2, 4, 5)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(12, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(13, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(14, Opcode::IsMustDeoptimize).b();
            INST(15, Opcode::DeoptimizeIf).Inputs(14, 13);
            INST(3, Opcode::Compare).b().CC(CC_LT).Inputs(10, 11);
            INST(4, Opcode::IfImm).CC(CC_NE).Inputs(3).Imm(0);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().Inlined().InputsAutoType(21);
            INST(16, Opcode::ReturnInlined).Inputs(0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(6, Opcode::NOP);
            INST(7, Opcode::NOP);
            INST(8, Opcode::NOP);
            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        CONSTANT(11, 1);
        CONSTANT(12, 10);
        BASIC_BLOCK(2, 3)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).s32().Inputs(10, 7);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(3, 12);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(4, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(5, Opcode::IsMustDeoptimize).b();
            INST(6, Opcode::DeoptimizeIf).Inputs(5, 4);
            INST(7, Opcode::Add).s32().Inputs(3, 11);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(13, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10, 0);
        CONSTANT(11, 1);
        CONSTANT(12, 10);
        BASIC_BLOCK(2, 3)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).s32().Inputs(10, 7);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(3, 12);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(4, Opcode::NOP);
            INST(5, Opcode::NOP);
            INST(6, Opcode::NOP);
            INST(7, Opcode::Add).s32().Inputs(3, 11);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(13, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithCallAfterGuardTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        CONSTANT(11, 1);
        CONSTANT(12, 10);
        BASIC_BLOCK(2, 3)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).s32().Inputs(10, 7);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(3, 12);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(4, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(5, Opcode::IsMustDeoptimize).b();
            INST(6, Opcode::DeoptimizeIf).Inputs(5, 4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(7, Opcode::Add).s32().Inputs(3, 11);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(13, Opcode::Return).s32().Inputs(3);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithCallBeforeGuardTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        CONSTANT(11, 1);
        CONSTANT(12, 10);
        BASIC_BLOCK(2, 3)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).s32().Inputs(10, 7);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(3, 12);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(4, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(5, Opcode::IsMustDeoptimize).b();
            INST(6, Opcode::DeoptimizeIf).Inputs(5, 4);
            INST(7, Opcode::Add).s32().Inputs(3, 11);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(13, Opcode::Return).s32().Inputs(3);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithCallBetweenGuardsWithDomGuardTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10, 0);
        CONSTANT(11, 1);
        CONSTANT(12, 10);
        BASIC_BLOCK(2, 3)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).s32().Inputs(10, 7);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(3, 12);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(15, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(16, Opcode::IsMustDeoptimize).b();
            INST(17, Opcode::DeoptimizeIf).Inputs(16, 15);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(4, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(5, Opcode::IsMustDeoptimize).b();
            INST(6, Opcode::DeoptimizeIf).Inputs(5, 4);
            INST(7, Opcode::Add).s32().Inputs(3, 11);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(13, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10, 0);
        CONSTANT(11, 1);
        CONSTANT(12, 10);
        BASIC_BLOCK(2, 3)
        {
            INST(0, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(1, Opcode::IsMustDeoptimize).b();
            INST(2, Opcode::DeoptimizeIf).Inputs(1, 0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).s32().Inputs(10, 7);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(3, 12);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(15, Opcode::NOP);
            INST(16, Opcode::NOP);
            INST(17, Opcode::NOP);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(4, Opcode::SaveStateDeoptimize).Inputs(10, 11).SrcVregs({0, 1});
            INST(5, Opcode::IsMustDeoptimize).b();
            INST(6, Opcode::DeoptimizeIf).Inputs(5, 4);
            INST(7, Opcode::Add).s32().Inputs(3, 11);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(13, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithDuplicatedGuardsTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(2, 0).s32();
        BASIC_BLOCK(2, 3, 10)
        {
            INST(3, Opcode::Compare).b().CC(CC_LT).Inputs(0, 2);
            INST(4, Opcode::IfImm).CC(CC_NE).Inputs(3).Imm(0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(19, Opcode::Phi).s32().Inputs(0, 14);
            INST(5, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(6, Opcode::IsMustDeoptimize).b();
            INST(7, Opcode::DeoptimizeIf).Inputs(6, 5);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(0, 2);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(10, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(11, Opcode::IsMustDeoptimize).b();
            INST(12, Opcode::DeoptimizeIf).Inputs(11, 10);
        }
        BASIC_BLOCK(5, 3, 10)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(14, Opcode::Add).s32().Inputs(19, 1);
            INST(15, Opcode::Compare).b().CC(CC_LT).Inputs(14, 2);
            INST(16, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }

        BASIC_BLOCK(10, 1)
        {
            INST(17, Opcode::Phi).s32().Inputs(0, 14);
            INST(18, Opcode::Return).s32().Inputs(17);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        PARAMETER(2, 0).s32();
        BASIC_BLOCK(2, 3, 10)
        {
            INST(3, Opcode::Compare).b().CC(CC_LT).Inputs(0, 2);
            INST(4, Opcode::IfImm).CC(CC_NE).Inputs(3).Imm(0);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(19, Opcode::Phi).s32().Inputs(0, 14);
            INST(5, Opcode::SaveStateDeoptimize).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(6, Opcode::IsMustDeoptimize).b();
            INST(7, Opcode::DeoptimizeIf).Inputs(6, 5);
            INST(8, Opcode::Compare).b().CC(CC_LT).Inputs(0, 2);
            INST(9, Opcode::IfImm).CC(CC_NE).Inputs(8).Imm(0);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(10, Opcode::NOP);
            INST(11, Opcode::NOP);
            INST(12, Opcode::NOP);
        }
        BASIC_BLOCK(5, 3, 10)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(14, Opcode::Add).s32().Inputs(19, 1);
            INST(15, Opcode::Compare).b().CC(CC_LT).Inputs(14, 2);
            INST(16, Opcode::IfImm).CC(CC_NE).Inputs(15).Imm(0);
        }

        BASIC_BLOCK(10, 1)
        {
            INST(17, Opcode::Phi).s32().Inputs(0, 14);
            INST(18, Opcode::Return).s32().Inputs(17);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardNotDominateTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(3, Opcode::IfImm).CC(CC_NE).Inputs(2).Imm(0);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(4, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::IsMustDeoptimize).b();
            INST(6, Opcode::DeoptimizeIf).Inputs(5, 4);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(7, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::IsMustDeoptimize).b();
            INST(9, Opcode::DeoptimizeIf).Inputs(8, 7);
            INST(10, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardIfWithGuardsTest)
{
    /*
     *    some code
     *    /       \
     *  call     call
     * guard    guard
     * \          /
     *  \        /
     *    guard(*, deleted)
     */
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(6, Opcode::IfImm).CC(CC_NE).Inputs(5).Imm(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(8, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::IsMustDeoptimize).b();
            INST(10, Opcode::DeoptimizeIf).Inputs(9, 8);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(21);
            INST(12, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(13, Opcode::IsMustDeoptimize).b();
            INST(14, Opcode::DeoptimizeIf).Inputs(13, 12);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(15, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(16, Opcode::IsMustDeoptimize).b();
            INST(17, Opcode::DeoptimizeIf).Inputs(16, 15);
            INST(18, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(6, Opcode::IfImm).CC(CC_NE).Inputs(5).Imm(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(8, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::IsMustDeoptimize).b();
            INST(10, Opcode::DeoptimizeIf).Inputs(9, 8);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(21, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(21);
            INST(12, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(13, Opcode::IsMustDeoptimize).b();
            INST(14, Opcode::DeoptimizeIf).Inputs(13, 12);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(15, Opcode::NOP);
            INST(16, Opcode::NOP);
            INST(17, Opcode::NOP);
            INST(18, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ReplaceByDeoptimizeTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::DeoptimizeIf).Inputs(0, 2);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::INVALID).Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ReplaceByDeoptimizeInliningTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::CallStatic).v0id().InputsAutoType(2).Inlined();
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(7, Opcode::DeoptimizeIf).Inputs(0, 4);
            INST(5, Opcode::ReturnInlined).Inputs(2);
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }
    INS(4).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(3)));
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::CallStatic).v0id().InputsAutoType(2).Inlined();
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(5, Opcode::ReturnInlined).Inputs(2);
            INST(8, Opcode::Deoptimize).Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, RemoveSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        INST(2, Opcode::SafePoint).Inputs(0, 1).SrcVregs({0, 1});
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Add).s32().Inputs(0, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Add).s32().Inputs(0, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Not applied, have runtime calls
TEST_F(DeoptimizeEliminationTest, RemoveSafePoint1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        INST(2, Opcode::SafePoint).Inputs(0, 1).SrcVregs({0, 1});
        BASIC_BLOCK(2, 1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(20);
            INST(3, Opcode::Add).s32().Inputs(0, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Not applied, a lot of instructions
TEST_F(DeoptimizeEliminationTest, RemoveSafePoint2)
{
    uint64_t N = options.GetCompilerSafepointEliminationLimit();
    auto block = GetGraph()->CreateEmptyBlock();
    GetGraph()->GetStartBlock()->AddSucc(block);
    block->AddSucc(GetGraph()->GetEndBlock());
    auto param1 = GetGraph()->CreateInstParameter(0);
    auto param2 = GetGraph()->CreateInstParameter(1);
    param1->SetType(DataType::INT32);
    param2->SetType(DataType::INT32);
    GetGraph()->GetStartBlock()->AppendInst(param1);
    GetGraph()->GetStartBlock()->AppendInst(param2);
    auto sp = GetGraph()->CreateInstSafePoint();
    sp->AppendInput(param1);
    sp->AppendInput(param2);
    sp->SetVirtualRegister(0, VirtualRegister(0, false));
    sp->SetVirtualRegister(1, VirtualRegister(1, false));
    GetGraph()->GetStartBlock()->AppendInst(sp);
    ArenaVector<Inst *> insts(GetGraph()->GetLocalAllocator()->Adapter());
    insts.push_back(param1);
    insts.push_back(param2);
    for (uint64_t i = 2; i <= N + 1; i++) {
        auto inst = GetGraph()->CreateInstAdd();
        inst->SetType(DataType::INT32);
        inst->SetInput(0, insts[i - 2]);
        inst->SetInput(1, insts[i - 1]);
        block->AppendInst(inst);
        insts.push_back(inst);
    }
    auto ret = GetGraph()->CreateInstReturn();
    ret->SetType(DataType::INT32);
    ret->SetInput(0, insts[N + 1]);
    block->AppendInst(ret);
    GraphChecker(GetGraph()).Check();
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Applied, have CallStatic.Inlined
TEST_F(DeoptimizeEliminationTest, RemoveSafePoint3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        INST(2, Opcode::SafePoint).Inputs(0, 1).SrcVregs({0, 1});
        BASIC_BLOCK(2, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(8).Inlined();
            INST(3, Opcode::Add).s32().Inputs(0, 1);
            INST(7, Opcode::ReturnInlined).Inputs(8);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::CallStatic).v0id().InputsAutoType(8).Inlined();
            INST(3, Opcode::Add).s32().Inputs(0, 1);
            INST(7, Opcode::ReturnInlined).Inputs(8);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Removes numeric inputs from SS without Deoptimize and doen't remove with Deoptimize
TEST_F(DeoptimizeEliminationTest, RemovNumericInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).b();
        PARAMETER(1, 2).s32();
        PARAMETER(2, 3).f64();
        PARAMETER(3, 4).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::DeoptimizeIf).Inputs(0, 4);
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::CallStatic).u64().InputsAutoType(0, 1, 2, 3, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).b();
        PARAMETER(1, 2).s32();
        PARAMETER(2, 3).f64();
        PARAMETER(3, 4).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::DeoptimizeIf).Inputs(0, 4);
            INST(5, Opcode::SaveState).Inputs(3).SrcVregs({3});
            INST(6, Opcode::CallStatic).u64().InputsAutoType(0, 1, 2, 3, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

// Removes object inputs from SS without Deoptimize
TEST_F(DeoptimizeEliminationTest, RemovObjectInputs)
{
    // 1 don't removed because they used in SS with deoptimization
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).b();
        PARAMETER(1, 2).ref();
        PARAMETER(2, 3).ref();
        PARAMETER(3, 4).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::DeoptimizeIf).Inputs(0, 4);
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::CallStatic).u64().InputsAutoType(0, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).b();
        PARAMETER(1, 2).ref();
        PARAMETER(2, 3).ref();
        PARAMETER(3, 4).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::DeoptimizeIf).Inputs(0, 4);
            INST(5, Opcode::SaveState).Inputs(1).SrcVregs({1});
            INST(6, Opcode::CallStatic).u64().InputsAutoType(0, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}
}  // namespace panda::compiler
