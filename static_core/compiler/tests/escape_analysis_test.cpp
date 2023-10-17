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
#include "optimizer/optimizations/escape.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/liveness_analyzer.h"

namespace panda::compiler {
class EscapeAnalysisTest : public GraphTest {
public:
    static const RuntimeInterface::FieldPtr OBJ_FIELD;
    static const RuntimeInterface::FieldPtr INT64_FIELD;
    static const RuntimeInterface::FieldPtr UINT64_FIELD;
    static const RuntimeInterface::FieldPtr INT32_FIELD;
    static const RuntimeInterface::FieldPtr UINT8_FIELD;
    static const RuntimeInterface::FieldPtr INT8_FIELD;

    EscapeAnalysisTest()
    {
        RegisterFieldType(OBJ_FIELD, DataType::REFERENCE);
        RegisterFieldType(INT64_FIELD, DataType::INT64);
        RegisterFieldType(UINT64_FIELD, DataType::UINT64);
        RegisterFieldType(INT32_FIELD, DataType::INT32);
        RegisterFieldType(UINT8_FIELD, DataType::UINT8);
        RegisterFieldType(INT8_FIELD, DataType::INT8);
    }

    bool Run() const
    {
        EscapeAnalysis ea {GetGraph()};
        ea.RelaxClassRestrictions();
        auto res = ea.Run();
        if (res) {
            GetGraph()->RunPass<Cleanup>();
        }
        return res;
    }
};

const RuntimeInterface::FieldPtr EscapeAnalysisTest::OBJ_FIELD = reinterpret_cast<void *>(0xDEADBEEF);
const RuntimeInterface::FieldPtr EscapeAnalysisTest::INT64_FIELD = reinterpret_cast<void *>(0xDEADFEED);
const RuntimeInterface::FieldPtr EscapeAnalysisTest::UINT64_FIELD = reinterpret_cast<void *>(0xDEAD64BU);
const RuntimeInterface::FieldPtr EscapeAnalysisTest::INT32_FIELD = reinterpret_cast<void *>(0xDEAD5320);
const RuntimeInterface::FieldPtr EscapeAnalysisTest::UINT8_FIELD = reinterpret_cast<void *>(0xB00B00L);
const RuntimeInterface::FieldPtr EscapeAnalysisTest::INT8_FIELD = reinterpret_cast<void *>(0xB00B01L);

TEST_F(EscapeAnalysisTest, NewEmptyUnusedObject)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState);
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState);
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(4, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, NewUnusedObjectAndControlFlow)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4) {}

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).Inputs(0, 1).s32();
            INST(7, Opcode::Return).Inputs(6).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4) {}

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).Inputs(0, 1).s32();
            INST(7, Opcode::Return).Inputs(6).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, NewUnusedObjectsFromDifferentBranches)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);  // virt
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(6, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(7, Opcode::NewObject).ref().Inputs(3, 6);  // virt
        }

        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::Phi).Inputs(0, 1).s32();
            INST(9, Opcode::Return).Inputs(8).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4) {}

        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::Phi).Inputs(0, 1).s32();
            INST(9, Opcode::Return).Inputs(8).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MergeDifferentMaterializedStatesForSameObject)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, 3, 7)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);  // virt
            INST(5, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(0);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(7, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(4, 7);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(9, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(10, Opcode::CallStatic).v0id().InputsAutoType(4, 9);
        }

        BASIC_BLOCK(6, 7)
        {
            INST(11, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(12, Opcode::NullCheck).ref().Inputs(4, 11);
            INST(13, Opcode::LoadObject).u64().Inputs(12);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(14, Opcode::Phi).Inputs(1, 13).u64();
            INST(15, Opcode::Return).Inputs(14).u64();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, 3, 7)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(0);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(16, Opcode::SaveState);
            INST(4, Opcode::NewObject).ref().Inputs(3, 16);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(7, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(4, 7);
        }

        BASIC_BLOCK(5, 6)
        {
            INST(9, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(10, Opcode::CallStatic).v0id().InputsAutoType(4, 9);
        }

        BASIC_BLOCK(6, 7)
        {
            INST(11, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(12, Opcode::NullCheck).ref().Inputs(4, 11);
            INST(13, Opcode::LoadObject).u64().Inputs(12);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(14, Opcode::Phi).Inputs(1, 13).u64();
            INST(15, Opcode::Return).Inputs(14).u64();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, ObjectEscapement)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState);
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::Return).ref().Inputs(3);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, ObjectPartialEscapement)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(10, Opcode::NullCheck).ref().Inputs(4, 2);
            INST(11, Opcode::StoreObject).s32().Inputs(10, 1).ObjField(INT32_FIELD);
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(8, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(4, 8);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).Inputs(0, 1).s32();
            INST(7, Opcode::Return).Inputs(6).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(10, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(11, Opcode::NewObject).ref().Inputs(3, 10);
            INST(12, Opcode::StoreObject).s32().Inputs(11, 1).ObjField(INT32_FIELD);
            INST(8, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 11);
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(11, 8);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).Inputs(0, 1).s32();
            INST(7, Opcode::Return).Inputs(6).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, ObjectEscapementThroughPhi)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4);
            INST(6, Opcode::NewObject).ref().Inputs(5, 4);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::SaveState).SrcVregs({0}).Inputs(0);
            INST(8, Opcode::LoadAndInitClass).ref().Inputs(7);
            INST(9, Opcode::NewObject).ref().Inputs(8, 7);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::Phi).Inputs(6, 9).ref();
            INST(11, Opcode::Return).Inputs(10).ref();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, ReturnField)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::StoreObject).u64().Inputs(5, 0).ObjField(UINT64_FIELD);
            INST(7, Opcode::LoadObject).u64().Inputs(5).ObjField(UINT64_FIELD);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(8, Opcode::Return).u64().Inputs(0);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, EscapeThroughAlias)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);  // should escape
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(6, Opcode::NewObject).ref().Inputs(2, 4);  // should escape

            INST(7, Opcode::SaveState).Inputs(0, 3, 6).SrcVregs({0, 1, 2});

            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);  // should escape
            INST(9, Opcode::NullCheck).ref().Inputs(6, 7);  // should escape

            INST(10, Opcode::CallStatic).v0id().InputsAutoType(3, 9, 7);
            INST(11, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, VirtualObjectsChain)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);  // should not escape
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(6, Opcode::NewObject).ref().Inputs(2, 4);  // should not escape

            INST(7, Opcode::SaveState).Inputs(0, 3, 6).SrcVregs({0, 1, 2});

            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);

            INST(9, Opcode::StoreObject).ref().Inputs(8, 6).ObjField(OBJ_FIELD);
            INST(11, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(11, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, VirtualObjectsChainDereference)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);  // should not escape
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(6, Opcode::NewObject).ref().Inputs(2, 4);  // should not escape

            INST(7, Opcode::SaveState).Inputs(0, 3, 6).SrcVregs({0, 1, 2});

            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);

            INST(9, Opcode::StoreObject).ref().Inputs(8, 6).ObjField(OBJ_FIELD);
            INST(10, Opcode::LoadObject).ref().Inputs(8).ObjField(OBJ_FIELD);

            INST(11, Opcode::SaveState).Inputs(0, 3, 10).SrcVregs({0, 1, 2});
            INST(12, Opcode::NullCheck).ref().Inputs(10, 11);
            INST(13, Opcode::LoadObject).u64().Inputs(12).ObjField(UINT64_FIELD);
            INST(14, Opcode::Return).u64().Inputs(13);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(15, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(14, Opcode::Return).u64().Inputs(15);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, VirtualLoadEscape)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);  // should not escape
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(6, Opcode::NewObject).ref().Inputs(2, 4);  // escape through return after load

            INST(7, Opcode::SaveState).Inputs(0, 3, 6).SrcVregs({0, 1, 2});

            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);

            INST(9, Opcode::StoreObject).ref().Inputs(8, 6).ObjField(OBJ_FIELD);
            INST(10, Opcode::LoadObject).ref().Inputs(8).ObjField(OBJ_FIELD);
            INST(14, Opcode::Return).ref().Inputs(10);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::NewObject).ref().Inputs(2, 4);
            INST(14, Opcode::Return).ref().Inputs(6);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, EscapeThroughAliasOnCfgMerge)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        PARAMETER(9, 1).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::SaveState).Inputs(0, 9).SrcVregs({0, 1});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::SaveState).Inputs(0, 9, 3).SrcVregs({0, 1, 2});
            INST(6, Opcode::NewObject).ref().Inputs(2, 4);

            INST(7, Opcode::SaveState).Inputs(0, 9, 3, 6).SrcVregs({0, 1, 2, 3});
            INST(8, Opcode::NullCheck).ref().Inputs(3, 7);
            INST(100, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0).Imm(0);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(10, Opcode::StoreObject).ref().Inputs(8, 9).ObjField(OBJ_FIELD);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(11, Opcode::SaveState).Inputs(0, 3, 6).SrcVregs({0, 1, 2});
            INST(12, Opcode::NullCheck).ref().Inputs(6, 11);
            INST(13, Opcode::StoreObject).ref().Inputs(8, 12).ObjField(OBJ_FIELD);
        }

        BASIC_BLOCK(5, 1)
        {
            INST(16, Opcode::SaveState).Inputs(3).SrcVregs({1});
            INST(14, Opcode::NullCheck).ref().Inputs(3, 16);
            INST(15, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        PARAMETER(9, 1).ref();

        BASIC_BLOCK(2, 5, 4)
        {
            INST(1, Opcode::SaveState).Inputs(0, 9).SrcVregs({0, 1});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(100, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0).Imm(0);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(13, Opcode::SaveState);
            INST(14, Opcode::NewObject).ref().Inputs(2, 13);
        }

        BASIC_BLOCK(5, 1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, EscapeInsideLoop)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(5, Opcode::Phi).u64().Inputs(0, 7);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(5);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::Sub).u64().Inputs(5, 1);
            INST(8, Opcode::SaveState).Inputs(7, 4).SrcVregs({7, 4});
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(4, 7, 8);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, PartialEscapeInsideLoop)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::Phi).u64().Inputs(0, 11);
            INST(3, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(2);
        }

        BASIC_BLOCK(3, 4, 7)
        {
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4);
            INST(6, Opcode::NewObject).ref().Inputs(5, 4);
            INST(7, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(2);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Sub).u64().Inputs(2, 1);
            INST(9, Opcode::SaveState).Inputs(6).SrcVregs({1});
            INST(10, Opcode::CallStatic).v0id().InputsAutoType(6, 9);
        }

        BASIC_BLOCK(7, 5) {}

        BASIC_BLOCK(5, 2)
        {
            INST(11, Opcode::Phi).u64().Inputs(8, 0);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    GetGraph()->RunPass<LivenessAnalyzer>();

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3) {}

        BASIC_BLOCK(3, 4, 6)
        {
            INST(2, Opcode::Phi).u64().Inputs(0, 0, 8);
            INST(3, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(2);
        }

        BASIC_BLOCK(4, 5, 3)
        {
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(5, Opcode::LoadAndInitClass).ref().Inputs(4);
            INST(7, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(2);
        }

        BASIC_BLOCK(5, 3)
        {
            INST(8, Opcode::Sub).u64().Inputs(2, 1);
            INST(13, Opcode::SaveState);
            INST(14, Opcode::NewObject).ref().Inputs(5, 13);
            INST(9, Opcode::SaveState).Inputs(14).SrcVregs({1});
            INST(10, Opcode::CallStatic).v0id().InputsAutoType(14, 9);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(12, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MergeFieldInsideLoop)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(1, 1);
        CONSTANT(2, 100);

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::SaveState);
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(5, Opcode::NewObject).ref().Inputs(4, 3);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Phi).s32().Inputs(2, 11);
            INST(7, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_GE).Inputs(6);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(8, Opcode::SaveState).Inputs(5).SrcVregs({0});
            INST(9, Opcode::NullCheck).ref().Inputs(5, 8);
            INST(10, Opcode::StoreObject).s32().Inputs(9, 6).ObjField(INT32_FIELD);
            INST(11, Opcode::Sub).s32().Inputs(6, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(12, Opcode::LoadObject).s32().Inputs(5).ObjField(INT32_FIELD);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(1, 1);
        CONSTANT(2, 100);
        CONSTANT(5, 0);

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::SaveState);
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Phi).s32().Inputs(2, 11);
            INST(8, Opcode::Phi).s32().Inputs(5, 6);
            INST(7, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_GE).Inputs(6);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(11, Opcode::Sub).s32().Inputs(6, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(12, Opcode::Return).s32().Inputs(8);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MergeObjectsInLoopHeader)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(1, 1);
        CONSTANT(2, 100);

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::SaveState);
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(5, Opcode::NewObject).ref().Inputs(4, 3);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(6, Opcode::Phi).ref().Inputs(5, 10);
            INST(12, Opcode::Phi).s32().Inputs(2, 11);
            INST(7, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_GE).Inputs(12);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(8, Opcode::SaveState).Inputs(5, 6).SrcVregs({0, 1});
            INST(9, Opcode::LoadAndInitClass).ref().Inputs(8);
            INST(10, Opcode::NewObject).ref().Inputs(9, 8);
            INST(11, Opcode::Sub).s32().Inputs(12, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(13, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, LoadDefaultValue)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(7, Opcode::LoadObject).u64().Inputs(5).ObjField(UINT64_FIELD);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(9, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(8, Opcode::Return).u64().Inputs(9);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, VirtualLoadAfterMaterialization)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);  // escape through return
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(7, Opcode::LoadObject).u64().Inputs(5).ObjField(UINT64_FIELD);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs(0, 7);
            INST(10, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(8, 10);
            INST(9, Opcode::Return).ref().Inputs(3);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();

    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(12, 0);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs(0, 12);
            INST(10, Opcode::SaveState);
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(8, 10);
            INST(13, Opcode::SaveState);
            INST(3, Opcode::NewObject).ref().Inputs(2, 13);
            INST(9, Opcode::Return).ref().Inputs(3);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MergeVirtualFields)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(5, Opcode::NewObject).ref().Inputs(4, 3);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(7, Opcode::SaveState).Inputs(0, 5).SrcVregs({0, 1});
            INST(8, Opcode::NullCheck).ref().Inputs(5, 7);
            INST(9, Opcode::StoreObject).i64().Inputs(8, 1).ObjField(INT64_FIELD);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(10, Opcode::SaveState).Inputs(0, 5).SrcVregs({0, 1});
            INST(11, Opcode::NullCheck).ref().Inputs(5, 10);
            INST(12, Opcode::StoreObject).i64().Inputs(11, 2).ObjField(INT64_FIELD);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(13, Opcode::SaveState).Inputs(0, 5).SrcVregs({0, 1});
            INST(14, Opcode::NullCheck).ref().Inputs(5, 13);
            INST(15, Opcode::LoadObject).i64().Inputs(14).ObjField(INT64_FIELD);
            INST(16, Opcode::Return).i64().Inputs(15);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);

        BASIC_BLOCK(2, 5, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(17, Opcode::Phi).i64().Inputs(1, 2);
            INST(16, Opcode::Return).i64().Inputs(17);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MergeVirtualObjFields)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(5, Opcode::NewObject).ref().Inputs(4, 3);
            INST(17, Opcode::NewObject).ref().Inputs(4, 3);
            INST(18, Opcode::NewObject).ref().Inputs(4, 3);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(7, Opcode::SaveState).Inputs(0, 5, 17).SrcVregs({0, 1, 2});
            INST(8, Opcode::NullCheck).ref().Inputs(5, 7);
            INST(9, Opcode::StoreObject).ref().Inputs(8, 17).ObjField(OBJ_FIELD);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(10, Opcode::SaveState).Inputs(0, 5, 18).SrcVregs({0, 1, 3});
            INST(11, Opcode::NullCheck).ref().Inputs(5, 10);
            INST(12, Opcode::StoreObject).ref().Inputs(11, 18).ObjField(OBJ_FIELD);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(13, Opcode::SaveState).Inputs(0, 5).SrcVregs({0, 1});
            INST(14, Opcode::NullCheck).ref().Inputs(5, 13);
            INST(15, Opcode::LoadObject).ref().Inputs(14).ObjField(OBJ_FIELD);
            INST(16, Opcode::Return).ref().Inputs(15);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(21, Opcode::SaveState);
            INST(17, Opcode::NewObject).ref().Inputs(4, 21);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(20, Opcode::SaveState);
            INST(18, Opcode::NewObject).ref().Inputs(4, 20);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(19, Opcode::Phi).ref().Inputs(17, 18);
            INST(16, Opcode::Return).ref().Inputs(19);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MultipleIterationsOverCfg)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::NullPtr).ref();
            INST(3, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(5, Opcode::NewObject).ref().Inputs(4, 3);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(3, 5, 6)
        {
            INST(7, Opcode::SaveState).Inputs(0, 5).SrcVregs({0, 1});
            INST(8, Opcode::NullCheck).ref().Inputs(5, 7);
            INST(9, Opcode::StoreObject).u64().Inputs(8, 1).ObjField(UINT64_FIELD);
            INST(11, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1).CC(CC_EQ).Inputs(0);
        }
        BASIC_BLOCK(4, 5) {}

        BASIC_BLOCK(5, 7) {}

        BASIC_BLOCK(7, -1)
        {
            INST(16, Opcode::Return).ref().Inputs(5);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(17, Opcode::Return).ref().Inputs(2);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();

    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(22, 0);

        BASIC_BLOCK(2, 3, 7)
        {
            INST(2, Opcode::NullPtr).ref();
            INST(3, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(6, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 7, 6)
        {
            INST(11, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(18, Opcode::Phi).u64().Inputs(22, 1);
            INST(19, Opcode::SaveState);
            INST(5, Opcode::NewObject).ref().Inputs(4, 19);
            INST(20, Opcode::StoreObject).u64().Inputs(5, 18);
            INST(16, Opcode::Return).ref().Inputs(5);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(17, Opcode::Return).ref().Inputs(2);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, CarryNestedState)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).Inputs(0, 4).SrcVregs({0, 1});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);
            INST(8, Opcode::SaveState).Inputs(0, 4, 7).SrcVregs({0, 1, 2});
            INST(9, Opcode::NullCheck).ref().Inputs(7, 8);
            INST(10, Opcode::StoreObject).ref().Inputs(9, 0).ObjField(OBJ_FIELD);
            INST(11, Opcode::NullCheck).ref().Inputs(4, 8);
            INST(12, Opcode::StoreObject).ref().Inputs(11, 7).ObjField(OBJ_FIELD);
            INST(13, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(14, Opcode::SaveState).Inputs(4).SrcVregs({1});
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(14);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::SaveState).Inputs(4).SrcVregs({1});
            INST(17, Opcode::NullCheck).ref().Inputs(4, 16);
            INST(18, Opcode::LoadObject).ref().Inputs(17).ObjField(OBJ_FIELD);
            INST(19, Opcode::SaveState).Inputs(18).SrcVregs({0});
            INST(20, Opcode::NullCheck).ref().Inputs(18, 19);
            INST(21, Opcode::LoadObject).ref().Inputs(20).ObjField(OBJ_FIELD);
            INST(22, Opcode::Return).ref().Inputs(21);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(13, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(14, Opcode::SaveState).Inputs(0).SrcVregs({VirtualRegister::BRIDGE});
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(14);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(22, Opcode::Return).ref().Inputs(0);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, RemoveVirtualObjectFromSaveStates)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::SaveState);
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);

            INST(4, Opcode::IfImm).SrcType(DataType::UINT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::ReturnVoid).v0id();
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(3, 6).Inlined();
            INST(8, Opcode::SaveState).Inputs(3).SrcVregs({0}).Caller(7);
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(3, 8).Inlined();
            INST(10, Opcode::SaveState).Inputs(3).SrcVregs({0}).Caller(9);
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(3, 10);
            INST(12, Opcode::ReturnInlined).Inputs(8);
            INST(13, Opcode::ReturnInlined).Inputs(6);
            INST(14, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(15, Opcode::Throw).Inputs(3, 14);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u32();
        INST(18, Opcode::NullPtr).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(1, Opcode::SaveState);
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(4, Opcode::IfImm).SrcType(DataType::UINT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::ReturnVoid).v0id();
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::SaveState);
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(18, 6).Inlined();
            INST(8, Opcode::SaveState);
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(18, 8).Inlined();
            INST(41, Opcode::SaveState);
            INST(42, Opcode::NewObject).ref().Inputs(2, 41);
            INST(10, Opcode::SaveState).Inputs(42).SrcVregs({0});
            INST(11, Opcode::CallStatic).v0id().InputsAutoType(42, 10);
            INST(12, Opcode::ReturnInlined).Inputs(8);
            INST(13, Opcode::ReturnInlined).Inputs(6);
            INST(14, Opcode::SaveState).Inputs(42).SrcVregs({0});
            INST(15, Opcode::Throw).Inputs(42, 14);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, CorrectlyHandleSingleBlockLoops)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
        }

        BASIC_BLOCK(3, 3, 5)
        {
            INST(5, Opcode::Phi).u64().Inputs(0, 6);
            INST(6, Opcode::Sub).u64().Inputs(5, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(6);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(10, Opcode::Throw).Inputs(4, 9);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
        }

        BASIC_BLOCK(3, 3, 5)
        {
            INST(5, Opcode::Phi).u64().Inputs(0, 6);
            INST(6, Opcode::Sub).u64().Inputs(5, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(6);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::SaveState);
            INST(4, Opcode::NewObject).ref().Inputs(3, 11);
            INST(9, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(10, Opcode::Throw).Inputs(4, 9);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MaterializeObjOnPhiInsideLoop)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
        }

        BASIC_BLOCK(3, 3, 5)
        {
            INST(5, Opcode::Phi).u64().Inputs(0, 6);
            INST(11, Opcode::Phi).ref().Inputs(4, 14);
            INST(6, Opcode::Sub).u64().Inputs(5, 1);
            INST(12, Opcode::SaveState).Inputs(4, 11).SrcVregs({0, 1});
            INST(13, Opcode::LoadAndInitClass).ref().Inputs(12);
            INST(14, Opcode::NewObject).ref().Inputs(13, 12);
            INST(7, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0).CC(CC_NE).Inputs(6);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(15, Opcode::Return).ref().Inputs(11);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, NotMergingMaterializationPaths)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(10, Opcode::SaveState);
            INST(11, Opcode::LoadAndInitClass).ref().Inputs(10);
            INST(12, Opcode::NewObject).ref().Inputs(11, 10);
            INST(13, Opcode::IfImm).SrcType(DataType::INT64).Imm(0).CC(CC_LE).Inputs(0);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::INT64).Imm(-1).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(15, Opcode::SaveState).Inputs(12).SrcVregs({0});
            INST(16, Opcode::Throw).Inputs(12, 15);
        }

        BASIC_BLOCK(4, 7, 6)
        {
            INST(17, Opcode::IfImm).SrcType(DataType::INT64).Imm(1).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(18, Opcode::SaveState).Inputs(12).SrcVregs({0});
            INST(19, Opcode::Throw).Inputs(12, 18);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(20, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(10, Opcode::SaveState);
            INST(11, Opcode::LoadAndInitClass).ref().Inputs(10);
            INST(13, Opcode::IfImm).SrcType(DataType::INT64).Imm(0).CC(CC_LE).Inputs(0);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(14, Opcode::IfImm).SrcType(DataType::INT64).Imm(-1).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(21, Opcode::SaveState);
            INST(12, Opcode::NewObject).ref().Inputs(11, 21);
            INST(15, Opcode::SaveState).Inputs(12).SrcVregs({0});
            INST(16, Opcode::Throw).Inputs(12, 15);
        }

        BASIC_BLOCK(4, 7, 6)
        {
            INST(17, Opcode::IfImm).SrcType(DataType::INT64).Imm(1).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(22, Opcode::SaveState);
            INST(23, Opcode::NewObject).ref().Inputs(11, 22);
            INST(18, Opcode::SaveState).Inputs(23).SrcVregs({0});
            INST(19, Opcode::Throw).Inputs(23, 18);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(20, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MaterializeCrossReferencingObjects)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();

        // obj.4 field0 = obj.7
        // obj.7 field0 = obj.4
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);

            INST(5, Opcode::SaveState).Inputs(0, 4).SrcVregs({0, 1});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);

            INST(8, Opcode::SaveState).Inputs(0, 4, 7).SrcVregs({0, 1, 2});
            INST(9, Opcode::NullCheck).ref().Inputs(7, 8);
            INST(10, Opcode::StoreObject).ref().Inputs(9, 4).ObjField(OBJ_FIELD);
            INST(11, Opcode::NullCheck).ref().Inputs(4, 8);
            INST(12, Opcode::StoreObject).ref().Inputs(11, 7).ObjField(OBJ_FIELD);
            INST(13, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(14, Opcode::SaveState).Inputs(4, 7).SrcVregs({1, 2});
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(4, 7, 14);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(22, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(13, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(16, Opcode::SaveState);
            INST(4, Opcode::NewObject).ref().Inputs(3, 16);
            INST(17, Opcode::SaveState).Inputs(4).SrcVregs({1});
            INST(7, Opcode::NewObject).ref().Inputs(6, 17);
            INST(10, Opcode::StoreObject).ref().Inputs(7, 4).ObjField(OBJ_FIELD);
            INST(12, Opcode::StoreObject).ref().Inputs(4, 7).ObjField(OBJ_FIELD);
            INST(14, Opcode::SaveState).Inputs(4, 7).SrcVregs({1, 2});
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(4, 7, 14);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(22, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MaterializeCrossReferencingObjectsAtNewSaveState)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();

        // obj.4 field0 = obj.7
        // obj.7 field0 = obj.4
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);

            INST(5, Opcode::SaveState).Inputs(0, 4).SrcVregs({0, 1});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);

            INST(8, Opcode::SaveState).Inputs(0, 4, 7).SrcVregs({0, 1, 2});
            INST(9, Opcode::NullCheck).ref().Inputs(7, 8);
            INST(10, Opcode::StoreObject).ref().Inputs(9, 4).ObjField(OBJ_FIELD);
            INST(11, Opcode::NullCheck).ref().Inputs(4, 8);
            INST(12, Opcode::StoreObject).ref().Inputs(11, 7).ObjField(OBJ_FIELD);
            INST(13, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(14, Opcode::SaveState).Inputs(4, 7).SrcVregs({1, 2});
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(14);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(22, Opcode::Return).ref().Inputs(7);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(13, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(14, Opcode::SaveState);
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(14);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::SaveState);
            INST(7, Opcode::NewObject).ref().Inputs(6, 16);
            INST(17, Opcode::SaveState).Inputs(7).SrcVregs({VirtualRegister::BRIDGE});
            INST(4, Opcode::NewObject).ref().Inputs(3, 17);
            INST(12, Opcode::StoreObject).ref().Inputs(4, 7).ObjField(OBJ_FIELD);
            INST(10, Opcode::StoreObject).ref().Inputs(7, 4).ObjField(OBJ_FIELD);
            INST(22, Opcode::Return).ref().Inputs(7);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, HandleCompareObjects)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(3, Opcode::NewObject).ref().Inputs(2, 1);
            INST(4, Opcode::Compare).b().Inputs(0, 3).CC(CC_EQ).SrcType(DataType::REFERENCE);
            INST(5, Opcode::Return).b().Inputs(4);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        CONSTANT(6, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(5, Opcode::Return).b().Inputs(6);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MaterializeSaveStateInputsEscapingInSaveStateUser)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 5)
        {
            INST(2, Opcode::SaveState);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            // Inst 4 escapes only in block 3, but it should be
            // materialized before corresponding save state.
            INST(5, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(6, Opcode::CallStatic).u64().InputsAutoType(0, 5).Inlined();
            INST(7, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0).Imm(0);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(4, 5);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::Phi).u64().Inputs(0, 1);
            INST(11, Opcode::ReturnInlined).u64().Inputs(5);
            INST(12, Opcode::Return).u64().Inputs(10);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, EliminateStoreToNarrowIntField)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u32();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::StoreObject).u8().Inputs(4, 0).ObjField(UINT8_FIELD);
            INST(6, Opcode::LoadObject).u8().Inputs(4).ObjField(UINT8_FIELD);
            INST(7, Opcode::Add).u32().Inputs(6, 1);
            INST(8, Opcode::Return).u32().Inputs(7);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).u32();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::Cast).u8().SrcType(DataType::UINT32).Inputs(0);
            INST(7, Opcode::Add).u32().Inputs(4, 1);
            INST(8, Opcode::Return).u32().Inputs(7);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, PatchSaveStates)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, 5, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::UINT32).CC(CC_EQ).Inputs(1).Imm(0);
        }

        BASIC_BLOCK(4, 5) {}

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).ref().Inputs(0, 4);
            INST(7, Opcode::SaveState).Inputs(0, 6).SrcVregs({0, 1});
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(0, 6, 7);
            INST(9, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, 5, 4)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::IfImm).SrcType(DataType::UINT32).CC(CC_EQ).Inputs(1).Imm(0);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(10, Opcode::SaveState).Inputs(0).SrcVregs({VirtualRegister::BRIDGE});
            INST(4, Opcode::NewObject).ref().Inputs(3, 10);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).ref().Inputs(0, 4);
            INST(7, Opcode::SaveState).Inputs(0, 6).SrcVregs({0, 1});
            INST(8, Opcode::CallStatic).v0id().InputsAutoType(0, 6, 7);
            INST(9, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, InverseMaterializationOrder)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).b();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);
            INST(8, Opcode::StoreObject).ref().Inputs(7, 4).ObjField(OBJ_FIELD);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(10, Opcode::SaveState).Inputs(4, 7).SrcVregs({0, 1});
            INST(11, Opcode::LoadAndInitClass).ref().Inputs(10);
            INST(12, Opcode::NewObject).ref().Inputs(11, 10);
            INST(13, Opcode::StoreObject).ref().Inputs(12, 7).ObjField(OBJ_FIELD);
            INST(14, Opcode::SaveState).Inputs(4, 7, 12).SrcVregs({0, 1, 2});
            INST(15, Opcode::NullCheck).ref().Inputs(4, 14);
            INST(16, Opcode::CallStatic).v0id().InputsAutoType(15, 12, 14);
            INST(17, Opcode::ReturnVoid).v0id();
        }

        BASIC_BLOCK(4, -1)
        {
            INST(18, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    // Don't check transformed graph as the test intended to only reproduce a crash.
    ASSERT_TRUE(Run());
}

TEST_F(EscapeAnalysisTest, MaterializeObjectDuringItsFieldsMerge)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).b();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::StoreObject).u64().Inputs(4, 1).ObjField(UINT64_FIELD);
            INST(6, Opcode::StoreObject).s64().Inputs(4, 1).ObjField(INT64_FIELD);
            INST(7, Opcode::StoreObject).s32().Inputs(4, 1).ObjField(INT32_FIELD);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(9, Opcode::SaveState).Inputs(4).SrcVregs({1});
            INST(10, Opcode::LoadAndInitClass).ref().Inputs(9);
            INST(11, Opcode::NewObject).ref().Inputs(10, 9);
            INST(12, Opcode::StoreObject).ref().Inputs(11, 4).ObjField(OBJ_FIELD);
            INST(13, Opcode::StoreObject).ref().Inputs(4, 11).ObjField(OBJ_FIELD);
        }

        BASIC_BLOCK(4, -1)
        {
            // We need to merge fields of the object (4), but during the merge we'll need to materialize
            // the object (11) as values of corresponding field diverged and require a phi.
            // Materialization of the object (11) will require materialization of the object (4) as it is
            // stored in 11's field. As a result materialization of the object (4) will be requested
            // during its fields merge.
            INST(14, Opcode::LoadObject).ref().Inputs(4).ObjField(OBJ_FIELD);
            INST(15, Opcode::Return).ref().Inputs(14);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, MaterializeBeforeReferencedObjectMaterialization)
{
    static constexpr uint32_t CALL_INST_ID = 10;
    static constexpr uint32_t SS1_ID = 11;
    static constexpr uint32_t SS2_ID = 13;
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(7, Opcode::NewObject).ref().Inputs(6, 5);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1).Imm(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(9, Opcode::SaveState).Inputs(4, 7).SrcVregs({0, 1});
            INST(CALL_INST_ID, Opcode::CallStatic).v0id().InputsAutoType(9).Inlined();
            INST(SS1_ID, Opcode::SaveState).Inputs(4, 7).SrcVregs({0, 1});
            INST(12, Opcode::CallStatic).v0id().InputsAutoType(7, 11);
            INST(SS2_ID, Opcode::SaveState).Inputs(4, 7).SrcVregs({0, 1});
            // Due to deopt inst all its save state inputs should be materialized before the first inlined call.
            // So both inst 4 and 7 should be materialized before inst 10, but before we hit deopt inst the inst 7
            // will be materialized at save state 11 (due to call static 12).
            INST(14, Opcode::DeoptimizeIf).Inputs(0, 13);
            INST(15, Opcode::ReturnInlined).v0id().Inputs(9);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).b();
        PARAMETER(1, 1).b();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::SaveState);
            INST(6, Opcode::LoadAndInitClass).ref().Inputs(5);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1).Imm(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(17, Opcode::SaveState);
            INST(4, Opcode::NewObject).ref().Inputs(3, 17);
            INST(18, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(7, Opcode::NewObject).ref().Inputs(6, 18);

            INST(9, Opcode::SaveState).Inputs(4, 7).SrcVregs({0, 1});
            INST(10, Opcode::CallStatic).v0id().InputsAutoType(9).Inlined();
            INST(11, Opcode::SaveState).Inputs(4, 7).SrcVregs({0, 1});
            INST(12, Opcode::CallStatic).v0id().InputsAutoType(7, 11);
            INST(13, Opcode::SaveState).Inputs(4, 7).SrcVregs({0, 1});
            INST(14, Opcode::DeoptimizeIf).Inputs(0, 13);
            INST(15, Opcode::ReturnInlined).v0id().Inputs(9);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(16, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
TEST_F(EscapeAnalysisTest, FillSaveStateBetweenSaveState)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();
        PARAMETER(13, 2).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0, 2}).Inputs(0, 13);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(10, Opcode::NullCheck).ref().Inputs(4, 2);
            INST(11, Opcode::StoreObject).s32().Inputs(10, 1).ObjField(INT32_FIELD);
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            // Branch with BasicBlock 3 don't have 13(vr2) in SaveState
            INST(8, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 4);
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(4, 8);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).Inputs(0, 1).s32();
            INST(14, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 13);
            INST(7, Opcode::Return).Inputs(6).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).s32();
        CONSTANT(1, 42).s32();
        PARAMETER(13, 2).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0, 2}).Inputs(0, 13);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::IfImm).SrcType(DataType::INT32).Imm(0).CC(CC_EQ).Inputs(0);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(15, Opcode::SaveState).SrcVregs({0, VirtualRegister::BRIDGE}).Inputs(0, 13);
            INST(16, Opcode::NewObject).ref().Inputs(3, 15);
            INST(17, Opcode::StoreObject).s32().Inputs(16, 1).ObjField(INT32_FIELD);
            INST(8, Opcode::SaveState).SrcVregs({0, 1, VirtualRegister::BRIDGE}).Inputs(0, 16, 13);
            INST(9, Opcode::CallStatic).v0id().InputsAutoType(16, 8);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).Inputs(0, 1).s32();
            INST(7, Opcode::Return).Inputs(6).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/* if the original input for a newly created phi was a
 * load then we have to ensure the current phi's actual
 * input type matches the type of phi itself */
TEST_F(EscapeAnalysisTest, FixPhiInputTypes)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s8();
        CONSTANT(20, 64).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 1);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
            INST(5, Opcode::StoreObject).s8().Inputs(4, 1).ObjField(INT8_FIELD);
            INST(6, Opcode::IfImm).SrcType(DataType::INT8).Imm(0).CC(CC_EQ).Inputs(1);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(7, Opcode::LoadObject).s8().Inputs(4).ObjField(INT8_FIELD);
            INST(8, Opcode::Add).s32().Inputs(7, 20);
            INST(9, Opcode::StoreObject).s8().Inputs(4, 8).ObjField(INT8_FIELD);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::LoadObject).s8().Inputs(4).ObjField(INT8_FIELD);
            INST(11, Opcode::Return).Inputs(10).s8();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s8();
        CONSTANT(20, 64).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::SaveState).SrcVregs({0, 1}).Inputs(0, 1);
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(5, Opcode::IfImm).SrcType(DataType::INT8).Imm(0).CC(CC_EQ).Inputs(1);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(8, Opcode::Add).s32().Inputs(1, 20);
            INST(9, Opcode::Cast).s8().SrcType(DataType::INT32).Inputs(8);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Phi).Inputs(1, 9).s8();
            INST(11, Opcode::Return).Inputs(10).s8();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
}  // namespace panda::compiler
