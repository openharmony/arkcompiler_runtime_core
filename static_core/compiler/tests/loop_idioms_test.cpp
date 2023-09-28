/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "optimizer/optimizations/loop_idioms.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
class LoopIdiomsTest : public GraphTest {
protected:
    bool CheckFillArrayFull(DataType::Type array_type, RuntimeInterface::IntrinsicId expected_intrinsic)
    {
        auto initial = CreateEmptyGraph();
        GRAPH(initial)
        {
            // NOLINTBEGIN(readability-magic-numbers)
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).type(array_type);
            CONSTANT(2, 0);
            CONSTANT(3, 1);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
                INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
                INST(6, Opcode::LenArray).i32().Inputs(5);
                INST(7, Opcode::Compare).b().Inputs(2, 6).CC(CC_LT).SrcType(DataType::INT32);
                INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(3, 4, 3)
            {
                INST(9, Opcode::Phi).i32().Inputs(2, 11);
                INST(10, Opcode::StoreArray).type(array_type).Inputs(5, 9, 1);
                INST(11, Opcode::Add).i32().Inputs(9, 3);
                INST(12, Opcode::Compare).b().Inputs(6, 11).CC(CC_LE).SrcType(DataType::INT32);
                INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(4, -1)
            {
                INST(14, Opcode::ReturnVoid).v0id();
            }
            // NOLINTEND(readability-magic-numbers)
        }

        if (!initial->RunPass<LoopIdioms>()) {
            return false;
        }
        initial->RunPass<Cleanup>();

        auto expected = CreateEmptyGraph();
        GRAPH(expected)
        {
            // NOLINTBEGIN(readability-magic-numbers)
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).type(array_type);
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            CONSTANT(20, 6);  // LoopIdioms::ITERATIONS_THRESHOLD

            BASIC_BLOCK(2, 5, 4)
            {
                INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
                INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
                INST(6, Opcode::LenArray).i32().Inputs(5);
                INST(7, Opcode::Compare).b().Inputs(2, 6).CC(CC_LT).SrcType(DataType::INT32);
                INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(5, 3, 6)
            {
                INST(15, Opcode::Sub).i32().Inputs(6, 2);
                INST(16, Opcode::Compare).b().Inputs(15, 20).SrcType(DataType::INT32).CC(CC_LE);
                INST(17, Opcode::IfImm).Inputs(16).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(3, 4, 3)
            {
                INST(9, Opcode::Phi).i32().Inputs(2, 11);
                INST(10, Opcode::StoreArray).type(array_type).Inputs(5, 9, 1);
                INST(11, Opcode::Add).i32().Inputs(9, 3);
                INST(12, Opcode::Compare).b().Inputs(6, 11).CC(CC_LE).SrcType(DataType::INT32);
                INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
            }

            BASIC_BLOCK(6, 4)
            {
                INST(18, Opcode::Intrinsic)
                    .v0id()
                    .Inputs({{DataType::REFERENCE, 5}, {array_type, 1}, {DataType::INT32, 2}, {DataType::INT32, 6}})
                    .IntrinsicId(expected_intrinsic)
                    .SetFlag(compiler::inst_flags::NO_HOIST)
                    .SetFlag(compiler::inst_flags::NO_DCE)
                    .SetFlag(compiler::inst_flags::NO_CSE)
                    .SetFlag(compiler::inst_flags::BARRIER)
                    .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                    .ClearFlag(compiler::inst_flags::RUNTIME_CALL);
            }

            BASIC_BLOCK(4, -1)
            {
                INST(14, Opcode::ReturnVoid).v0id();
            }
            // NOLINTEND(readability-magic-numbers)
        }

        return GraphComparator().Compare(initial, expected);
    }
};

TEST_F(LoopIdiomsTest, FillArray)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    EXPECT_TRUE(CheckFillArrayFull(DataType::BOOL, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT8, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT8, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT16, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_16));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT16, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_16));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT32, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT32, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32));
    EXPECT_TRUE(CheckFillArrayFull(DataType::INT64, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_64));
    EXPECT_TRUE(CheckFillArrayFull(DataType::UINT64, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_64));
    EXPECT_TRUE(CheckFillArrayFull(DataType::FLOAT32, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F32));
    EXPECT_TRUE(CheckFillArrayFull(DataType::FLOAT64, RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F64));

    EXPECT_FALSE(CheckFillArrayFull(DataType::REFERENCE, RuntimeInterface::IntrinsicId::COUNT));
    EXPECT_FALSE(CheckFillArrayFull(DataType::POINTER, RuntimeInterface::IntrinsicId::COUNT));
}

TEST_F(LoopIdiomsTest, IncorrectStep)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 10);  // incorrect step

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(2, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 11);
            INST(10, Opcode::StoreArray).i32().Inputs(5, 9, 1);
            INST(11, Opcode::Add).i32().Inputs(9, 3);
            INST(12, Opcode::Compare).b().Inputs(6, 11).CC(CC_LE).SrcType(DataType::INT32);
            INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, OtherInstructionsWithinLoop)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(2, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 11);
            INST(10, Opcode::StoreArray).i32().Inputs(5, 9, 1);
            // it should prevent optimization
            INST(15, Opcode::LoadArray).i32().Inputs(5, 9);
            INST(11, Opcode::Add).i32().Inputs(9, 3);
            INST(12, Opcode::Compare).b().Inputs(6, 11).CC(CC_LE).SrcType(DataType::INT32);
            INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, UseLoopInstructionsOutstideOfLoop)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(2, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 11);
            INST(10, Opcode::StoreArray).i32().Inputs(5, 9, 1);
            INST(11, Opcode::Add).i32().Inputs(9, 3);
            INST(12, Opcode::Compare).b().Inputs(6, 11).CC(CC_LE).SrcType(DataType::INT32);
            INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::Phi).i32().Inputs(2, 9);
            INST(15, Opcode::Return).i32().Inputs(14);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, FillTinyArray)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);
        CONSTANT(15, 3);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(15, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 11);
            INST(10, Opcode::StoreArray).i32().Inputs(5, 9, 1);
            INST(11, Opcode::Add).i32().Inputs(9, 3);
            INST(12, Opcode::Compare).b().Inputs(15, 11).CC(CC_LE).SrcType(DataType::INT32);
            INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, FillLargeArrayWithConstantIterationsCount)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);
        CONSTANT(15, 42);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(15, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 11);
            INST(10, Opcode::StoreArray).i32().Inputs(5, 9, 1);
            INST(11, Opcode::Add).i32().Inputs(9, 3);
            INST(12, Opcode::Compare).b().Inputs(15, 11).CC(CC_LE).SrcType(DataType::INT32);
            INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GetGraph()->RunPass<LoopIdioms>());
    GetGraph()->RunPass<Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(15, 42);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(15, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(17, Opcode::Intrinsic)
                .v0id()
                .Inputs({{DataType::REFERENCE, 5}, {DataType::INT32, 1}, {DataType::INT32, 2}, {DataType::INT32, 15}})
                .IntrinsicId(RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32)
                .SetFlag(compiler::inst_flags::NO_HOIST)
                .SetFlag(compiler::inst_flags::NO_DCE)
                .SetFlag(compiler::inst_flags::NO_CSE)
                .SetFlag(compiler::inst_flags::BARRIER)
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .ClearFlag(compiler::inst_flags::RUNTIME_CALL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

TEST_F(LoopIdiomsTest, MultipleAdds)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(2, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 12);
            INST(10, Opcode::StoreArray).i32().Inputs(5, 9, 1);
            INST(11, Opcode::Add).i32().Inputs(9, 3);
            INST(12, Opcode::Add).i32().Inputs(11, 3);
            INST(13, Opcode::Compare).b().Inputs(6, 12).CC(CC_LE).SrcType(DataType::INT32);
            INST(14, Opcode::IfImm).Inputs(13).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(15, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, PreIncrementIndex)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            INST(7, Opcode::Compare).b().Inputs(2, 6).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 10);
            INST(10, Opcode::Add).i32().Inputs(9, 3);
            INST(11, Opcode::StoreArray).i32().Inputs(5, 10, 1);
            INST(12, Opcode::Compare).b().Inputs(6, 10).CC(CC_LE).SrcType(DataType::INT32);
            INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
}

TEST_F(LoopIdiomsTest, MismatchingConditions)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP();
    }

    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0);
        CONSTANT(3, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(5, Opcode::NullCheck).ref().Inputs(0, 4);
            INST(6, Opcode::LenArray).i32().Inputs(5);
            // This condition don't make any sensce, but it is so intentionally.
            INST(7, Opcode::Compare).b().Inputs(3, 2).CC(CC_LT).SrcType(DataType::INT32);
            INST(8, Opcode::IfImm).Inputs(7).Imm(0).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3, 4, 3)
        {
            INST(9, Opcode::Phi).i32().Inputs(2, 11);
            INST(10, Opcode::StoreArray).i32().Inputs(5, 9, 1);
            INST(11, Opcode::Add).i32().Inputs(9, 3);
            INST(12, Opcode::Compare).b().Inputs(6, 11).CC(CC_LE).SrcType(DataType::INT32);
            INST(13, Opcode::IfImm).Inputs(12).Imm(0).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(14, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    // Diffrent condition before and within the loop should not prevent idiom recognition.
    ASSERT_TRUE(GetGraph()->RunPass<LoopIdioms>());
}

}  // namespace panda::compiler
