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

#include "bytecodeopt_peepholes.h"
#include "common.h"
#include "compiler/optimizer/optimizations/cleanup.h"

namespace panda::bytecodeopt::test {

TEST_F(IrBuilderTest, PeepholesTryBlockInstBetween)
{
    auto source = R"(
    .record E {}
    .record R {
        u1 field
    }

    .function void R.ctor(R a0) <ctor> {
        newobj v0, E
        throw v0
    }

    .function u8 main() {
    try_begin:
        movi v1, 0x1
        newobj v0, R
        movi v1, 0x2
        call.short R.ctor, v0
    try_end:
        ldai 0x0
        return
    catch_all:
        lda v1
        return
    .catchall try_begin, try_end, catch_all
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    EXPECT_FALSE(GetGraph()->RunPass<BytecodeOptPeepholes>());
}

TEST_F(IrBuilderTest, PeepholesTryBlockNoInstBetween)
{
    auto source = R"(
    .record E {}
    .record R {
        u1 field
    }

    .function void R.ctor(R a0) <ctor> {
        newobj v0, E
        throw v0
    }

    .function u8 main() {
    try_begin:
        movi v1, 0x1
        newobj v0, R
        call.short R.ctor, v0
    try_end:
        ldai 0x0
        return
    catch_all:
        lda v1
        return
    .catchall try_begin, try_end, catch_all
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    EXPECT_TRUE(GetGraph()->RunPass<BytecodeOptPeepholes>());
}

// TODO(aromanov): enable
TEST_F(CommonTest, DISABLED_NoNullCheck)
{
    RuntimeInterfaceMock runtime(0);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        using namespace compiler::DataType;
        CONSTANT(6, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0).TypeId(68);
            INST(2, Opcode::NewObject).ref().Inputs(1, 0).TypeId(68);
            INST(3, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2}, {NO_TYPE, 3}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    EXPECT_TRUE(graph->RunPass<BytecodeOptPeepholes>());
    EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());

    auto after = CreateEmptyGraph();
    GRAPH(after)
    {
        using namespace compiler::DataType;
        CONSTANT(6, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0).TypeId(68);
            INST(3, Opcode::SaveState).NoVregs();
            INST(8, Opcode::InitObject).ref().Inputs({{REFERENCE, 1}, {NO_TYPE, 3}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, after));
}

// TODO(aromanov): enable
TEST_F(CommonTest, DISABLED_NotRelatedNullCheck)
{
    RuntimeInterfaceMock runtime(1);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        using namespace compiler::DataType;
        PARAMETER(10, 0).ref();
        CONSTANT(6, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0);
            INST(2, Opcode::NewObject).ref().Inputs(1, 0);
            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::NullCheck).ref().Inputs(10, 3);
            INST(5, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2}, {NO_TYPE, 3}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

TEST_F(CommonTest, CallStaticOtherBasicBlock)
{
    RuntimeInterfaceMock runtime(1);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        using namespace compiler::DataType;
        PARAMETER(10, 0).ref();
        CONSTANT(6, 0);
        BASIC_BLOCK(2, 3)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0);
            INST(2, Opcode::NewObject).ref().Inputs(1, 0);
            INST(3, Opcode::SaveState).NoVregs();
        }
        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2}, {NO_TYPE, 3}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

// TODO(aromanov): enable
TEST_F(CommonTest, DISABLED_NoSaveStateNullCheckAfterNewObject)
{
    RuntimeInterfaceMock runtime(0);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        using namespace compiler::DataType;
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0);
            INST(2, Opcode::NewObject).ref().Inputs(1, 0);
            CONSTANT(3, 0).s32();
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2}, {NO_TYPE, 4}});
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

TEST_F(CommonTest, CallConstructorOtherClass)
{
    RuntimeInterfaceMock runtime(1, false);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        using namespace compiler::DataType;
        PARAMETER(10, 0).ref();
        CONSTANT(6, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadAndInitClass).ref().Inputs(0);
            INST(2, Opcode::NewObject).ref().Inputs(1, 0);
            INST(3, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2}, {NO_TYPE, 3}});
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

}  // namespace panda::bytecodeopt::test
