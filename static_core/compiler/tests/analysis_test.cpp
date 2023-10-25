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
#include "optimizer/ir/analysis.h"

namespace panda::compiler {
class AnalysisTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(AnalysisTest, FixSSInBBFromBlockToOut)
{
    GRAPH(GetGraph())
    {
        // SS will be fixed ONLY in BB2
        BASIC_BLOCK(2, 3)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will fix
            INST(2, Opcode::SaveState).NoVregs();
        }
        BASIC_BLOCK(3, -1)
        {
            // This SS will not fix
            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, 3)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2, Opcode::SaveState).Inputs(1).SrcVregs({VirtualRegister::BRIDGE});
        }
        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(2));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixSSInBBFromOutToBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, 3)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(2, Opcode::SaveState).NoVregs();
        }
        // SS will be fixed ONLY in BB3
        BASIC_BLOCK(3, -1)
        {
            // This SS will fix
            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::Intrinsic)
                .v0id()
                .Inputs({{DataType::REFERENCE, 1}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, 3)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2, Opcode::SaveState).NoVregs();
        }
        BASIC_BLOCK(3, -1)
        {
            INST(3, Opcode::SaveState).Inputs(1).SrcVregs({VirtualRegister::BRIDGE});
            INST(4, Opcode::Intrinsic)
                .v0id()
                .Inputs({{DataType::REFERENCE, 1}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(3));
    ASSERT(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixSSInBBUnrealInput)
{
    builder_->EnableGraphChecker(false);
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            // After moving Intrinsic use before declare, so fix SaveState
            INST(1, Opcode::SaveState).Inputs(2).SrcVregs({10});
            INST(2, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(3, Opcode::ReturnVoid).v0id();
        }
    }

    builder_->EnableGraphChecker(true);
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).NoVregs();
            INST(2, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(3, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(2));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixSSInBBInside)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will fix
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2, Opcode::SaveState).Inputs(1).SrcVregs({VirtualRegister::BRIDGE});
            INST(3, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(2));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixBridgesInOptimizedGraph)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2, Opcode::SaveState);
            INST(3, Opcode::ReturnVoid).v0id();
        }
    }

    auto bridge_example = CreateEmptyGraph();
    GRAPH(bridge_example)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2, Opcode::SaveState).Inputs(1).SrcVregs({VirtualRegister::BRIDGE});
            INST(3, Opcode::ReturnVoid).v0id();
        }
    }

    Graph *graph_bc = CreateEmptyBytecodeGraph();
    graph_bc->SetRuntime(&runtime_);
    GRAPH(graph_bc)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).ref();
            INST(3, Opcode::ReturnVoid).v0id();
        }
    }

    Graph *example = CreateEmptyGraph();
    GRAPH(example)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).ref();
            INST(2, Opcode::ReturnVoid).v0id();
        }
    }

    SaveStateBridgesBuilder ssb;

    const BasicBlock *bb = GetGraph()->GetVectorBlocks().at(2);
    ssb.SearchAndCreateMissingObjInSaveState(GetGraph(), bb->GetFirstInst(), bb->GetLastInst());

    const BasicBlock *bb_bc = graph_bc->GetVectorBlocks().at(2);
    ssb.SearchAndCreateMissingObjInSaveState(graph_bc, bb_bc->GetFirstInst(), bb_bc->GetLastInst());

    ASSERT_TRUE(GraphComparator().Compare(graph_bc, example));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), bridge_example));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
