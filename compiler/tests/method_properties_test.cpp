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
#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/method_properties.h"

namespace panda::compiler {
class MethodPropertiesTest : public GraphTest {
public:
    void CheckCall(Opcode opcode)
    {
        GRAPH(GetGraph())
        {
            PARAMETER(0, 0).ref();
            BASIC_BLOCK(2, -1)
            {
                INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
                INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
                INST(5, Opcode::SaveState).NoVregs();
                INST(3, opcode).v0id().Inputs({{DataType::REFERENCE, 2}, {DataType::NO_TYPE, 5}});
                INST(4, Opcode::ReturnVoid).v0id();
            }
        }
        MethodProperties props(GetGraph());
        EXPECT_TRUE(props.GetHasCalls());
        EXPECT_TRUE(props.GetHasRuntimeCalls());
        EXPECT_TRUE(props.GetHasRequireState());
        EXPECT_FALSE(props.GetHasSafepoints());
        EXPECT_TRUE(props.GetCanThrow());  // due to null check
    }
};

TEST_F(MethodPropertiesTest, EmptyBlock)
{
    GetGraph()->CreateStartBlock();
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_FALSE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, SimpleMethod)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u32().Inputs(0, 1);
            INST(3, Opcode::Return).u32().Inputs(2);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_FALSE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, SafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u32().Inputs(0, 1);
            INST(3, Opcode::SafePoint).Inputs(2).SrcVregs({0});
            INST(4, Opcode::Return).u32().Inputs(2);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_TRUE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_TRUE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, StaticCall)
{
    CheckCall(Opcode::CallStatic);
}

TEST_F(MethodPropertiesTest, VirtualCall)
{
    CheckCall(Opcode::CallVirtual);
}

TEST_F(MethodPropertiesTest, Intrinsic)
{
    CheckCall(Opcode::Intrinsic);
}

TEST_F(MethodPropertiesTest, Builtin)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).f64();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Builtin).b().Inputs({{DataType::FLOAT64, 0}});
            INST(2, Opcode::Return).b().Inputs(1);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_TRUE(props.GetHasCalls());
    EXPECT_FALSE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, SaveState)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::SaveState).NoVregs();
            INST(1, Opcode::LoadString).ref().Inputs(0).TypeId(42);
            INST(2, Opcode::Return).ref().Inputs(1);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_TRUE(props.GetHasRuntimeCalls());
    EXPECT_TRUE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_TRUE(props.GetCanThrow());
}
}  // namespace panda::compiler
