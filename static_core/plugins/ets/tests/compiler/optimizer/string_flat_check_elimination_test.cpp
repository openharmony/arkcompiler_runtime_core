/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "tests/unit_test.h"

#include "intrinsic_string_flat_check.inl"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/savestate_optimization.h"
#include "optimizer/optimizations/string_flat_check_elimination.h"

#include <gtest/gtest.h>

namespace ark::compiler {
class StringFlatCheckEliminationTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(HasTrueUser, Graph *graph, bool requiresBoth)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(2U, Opcode::NullPtr).ref();
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::StringFlatCheck).ref().Inputs(2U, 3U);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(6U, Opcode::StringFlatCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::SaveState).Inputs(4U, 6U).SrcVregs({VirtualRegister::BRIDGE, VirtualRegister::BRIDGE});
            // INTRINSIC_STD_CORE_STRING_EQUALS requires a flat string for the first argument ('this') only.
            // INTRINSIC_STD_CORE_STRING_TO_LOCALE_LOWER_CASE requires a flat string for both arguments.
            INST(8U, Opcode::Intrinsic)
                .b()
                .IntrinsicId(requiresBoth
                                 ? RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_TO_LOCALE_LOWER_CASE
                                 : RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                .InputsAutoType(4U, 6U, 7U);
            INST(9U, Opcode::Return).b().Inputs(8U);
        }
    }
}

OUT_GRAPH(HasTrueUser, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(2U, Opcode::NullPtr).ref();
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::StringFlatCheck).ref().Inputs(2U, 3U);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(10U, Opcode::NOP);
            INST(7U, Opcode::SaveState).Inputs(4U, 2U).SrcVregs({VirtualRegister::BRIDGE, VirtualRegister::BRIDGE});
            INST(8U, Opcode::Intrinsic)
                .b()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                .InputsAutoType(4U, 2U, 7U);
            INST(9U, Opcode::Return).b().Inputs(8U);
        }
    }
}

TEST_F(StringFlatCheckEliminationTest, HasTrueUser)
{
    src_graph::HasTrueUser ::CREATE(GetGraph(), false);
    Graph *clone = CreateEmptyGraph();
    src_graph::HasTrueUser ::CREATE(clone, false);
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheckElimination>());

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::HasTrueUser::CREATE(expectedGraph);
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(StringFlatCheckEliminationTest, HasBothTrueUsers)
{
    src_graph::HasTrueUser ::CREATE(GetGraph(), true);
    Graph *clone = CreateEmptyGraph();
    src_graph::HasTrueUser ::CREATE(clone, true);
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheckElimination>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// CC-OFFNXT(huge_method, G.FUD.05) graph creation
SRC_GRAPH(HasIndirectUser, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 42U);
        BASIC_BLOCK(2U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(5U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_LT).b();
            INST(6U, Opcode::IfImm).Inputs(5U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(7U, Opcode::LenArray).i32().Inputs(0U);
            INST(8U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(9U, Opcode::StringFlatCheck).ref().Inputs(0U, 8U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(10U, Opcode::LenArray).i32().Inputs(0U);
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(12U, Opcode::StringFlatCheck).ref().Inputs(0U, 11U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(13U, Opcode::Phi).i32().Inputs(7U, 10U);
            INST(14U, Opcode::Phi).ref().Inputs(9U, 12U);
            INST(15U, Opcode::SaveState).Inputs(14U, 0U).SrcVregs({2U, 4U});
            INST(16U, Opcode::Intrinsic)
                .b()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                .InputsAutoType(14U, 0U, 15U);
            INST(17U, Opcode::Return).i32().Inputs(13U);
        }
    }
}

TEST_F(StringFlatCheckEliminationTest, HasIndirectUser)
{
    src_graph::HasIndirectUser ::CREATE(GetGraph());
    Graph *clone = CreateEmptyGraph();
    src_graph::HasIndirectUser ::CREATE(clone);
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheckElimination>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// CC-OFFNXT(huge_method, G.FUD.05) graph creation
SRC_GRAPH(HasIndirectUserSecondLevel, Graph *graph, bool secondStringFlatCheck)
{
    GRAPH(graph)
    {
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).i32();
            CONSTANT(2U, 42U);
            BASIC_BLOCK(2U, 4U)
            {
                INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            }
            BASIC_BLOCK(4U, 5U, 6U)
            {
                INST(5U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_LT).b();
                INST(6U, Opcode::IfImm).Inputs(5U).Imm(0U).CC(ConditionCode::CC_NE);
            }
            BASIC_BLOCK(5U, 7U, 8U)
            {
                INST(7U, Opcode::LenArray).i32().Inputs(0U);
                INST(8U, Opcode::Compare).Inputs(7U, 2U).CC(ConditionCode::CC_LT).b();
                INST(9U, Opcode::IfImm).Inputs(8U).Imm(0U).CC(ConditionCode::CC_NE);
            }
            BASIC_BLOCK(7U, 9U)
            {
                INST(10U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                INST(11U, Opcode::StringFlatCheck).ref().Inputs(0U, 10U);
                if (secondStringFlatCheck) {
                    INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                    INST(13U, Opcode::StringFlatCheck).ref().Inputs(0U, 12U);
                }
            }
            BASIC_BLOCK(8U, 9U)
            {
                INST(14U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                INST(15U, Opcode::StringFlatCheck).ref().Inputs(0U, 14U);
                if (secondStringFlatCheck) {
                    INST(16U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                    INST(17U, Opcode::StringFlatCheck).ref().Inputs(0U, 16U);
                }
            }
            BASIC_BLOCK(9U, 10U)
            {
                INST(18U, Opcode::Phi).ref().Inputs(11U, 15U);
                if (secondStringFlatCheck) {
                    INST(19U, Opcode::Phi).ref().Inputs(13U, 17U);
                }
            }
            BASIC_BLOCK(6U, 11U, 12U)
            {
                INST(20U, Opcode::LenArray).i32().Inputs(0U);
                INST(21U, Opcode::Compare).Inputs(20U, 2U).CC(ConditionCode::CC_LT).b();
                INST(22U, Opcode::IfImm).Inputs(21U).Imm(0U).CC(ConditionCode::CC_NE);
            }
            BASIC_BLOCK(11U, 13U)
            {
                INST(23U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                INST(24U, Opcode::StringFlatCheck).ref().Inputs(0U, 23U);
                if (secondStringFlatCheck) {
                    INST(25U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                    INST(26U, Opcode::StringFlatCheck).ref().Inputs(0U, 25U);
                }
            }
            BASIC_BLOCK(12U, 13U)
            {
                INST(27U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                INST(28U, Opcode::StringFlatCheck).ref().Inputs(0U, 27U);
                if (secondStringFlatCheck) {
                    INST(29U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
                    INST(30U, Opcode::StringFlatCheck).ref().Inputs(0U, 29U);
                }
            }
            // A loop just to test a self-looped Phi
            BASIC_BLOCK(13U, 13U, 10U)
            {
                INST(31U, Opcode::Phi).ref().Inputs(24U, 28U, 31U);
                if (secondStringFlatCheck) {
                    INST(32U, Opcode::Phi).ref().Inputs(26U, 30U, 32U);
                }
                INST(33U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_LT).b();
                INST(34U, Opcode::IfImm).Inputs(33U).Imm(0U).CC(ConditionCode::CC_NE);
            }
            BASIC_BLOCK(10U, -1L)
            {
                INST(35U, Opcode::Phi).i32().Inputs(7U, 20U);
                INST(36U, Opcode::Phi).ref().Inputs(18U, 31U);
                if (secondStringFlatCheck) {
                    INST(37U, Opcode::Phi).ref().Inputs(19U, 32U);
                }
                INST(38U, Opcode::SaveState).Inputs(36U, secondStringFlatCheck ? 37U : 0U).SrcVregs({2U, 4U});
                INST(39U, Opcode::NullCheck).ref().Inputs(36U, 38U);
                INST(40U, Opcode::SaveState).Inputs(36U, secondStringFlatCheck ? 37U : 0U).SrcVregs({2U, 4U});
                INST(41U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                    .InputsAutoType(39U, secondStringFlatCheck ? 37U : 0U, 40U);
                INST(42U, Opcode::Return).i32().Inputs(35U);
            }
        }
    }
}

// CC-OFFNXT(huge_method, G.FUD.05) graph creation
OUT_GRAPH(HasIndirectUserSecondLevel, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 42U);
        BASIC_BLOCK(2U, 4U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(5U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_LT).b();
            INST(6U, Opcode::IfImm).Inputs(5U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(5U, 7U, 8U)
        {
            INST(7U, Opcode::LenArray).i32().Inputs(0U);
            INST(8U, Opcode::Compare).Inputs(7U, 2U).CC(ConditionCode::CC_LT).b();
            INST(9U, Opcode::IfImm).Inputs(8U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(7U, 9U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(11U, Opcode::StringFlatCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(46U, Opcode::NOP);
        }
        BASIC_BLOCK(8U, 9U)
        {
            INST(14U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(15U, Opcode::StringFlatCheck).ref().Inputs(0U, 14U);
            INST(16U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(45U, Opcode::NOP);
        }
        BASIC_BLOCK(9U, 10U)
        {
            INST(18U, Opcode::Phi).ref().Inputs(11U, 15U);
            INST(19U, Opcode::Phi).ref().Inputs(0U, 0U);
        }
        BASIC_BLOCK(6U, 11U, 12U)
        {
            INST(20U, Opcode::LenArray).i32().Inputs(0U);
            INST(21U, Opcode::Compare).Inputs(20U, 2U).CC(ConditionCode::CC_LT).b();
            INST(22U, Opcode::IfImm).Inputs(21U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(11U, 13U)
        {
            INST(23U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(24U, Opcode::StringFlatCheck).ref().Inputs(0U, 23U);
            INST(25U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(44U, Opcode::NOP);
        }
        BASIC_BLOCK(12U, 13U)
        {
            INST(27U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(28U, Opcode::StringFlatCheck).ref().Inputs(0U, 27U);
            INST(29U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(43U, Opcode::NOP);
        }
        // A loop just to test a self-looped Phi
        BASIC_BLOCK(13U, 13U, 10U)
        {
            INST(31U, Opcode::Phi).ref().Inputs(24U, 28U, 31U);
            INST(32U, Opcode::Phi).ref().Inputs(0U, 0U, 32U);
            INST(33U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_LT).b();
            INST(34U, Opcode::IfImm).Inputs(33U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(10U, -1L)
        {
            INST(35U, Opcode::Phi).i32().Inputs(7U, 20U);
            INST(36U, Opcode::Phi).ref().Inputs(18U, 31U);
            INST(37U, Opcode::Phi).ref().Inputs(19U, 32U);
            INST(38U, Opcode::SaveState).Inputs(36U, 37U).SrcVregs({2U, 4U});
            INST(39U, Opcode::NullCheck).ref().Inputs(36U, 38U);
            INST(40U, Opcode::SaveState).Inputs(36U, 37U).SrcVregs({2U, 4U});
            INST(41U, Opcode::Intrinsic)
                .b()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                .InputsAutoType(39U, 37U, 40U);
            INST(42U, Opcode::Return).i32().Inputs(35U);
        }
    }
}

TEST_F(StringFlatCheckEliminationTest, HasIndirectUserSecondLevel)
{
    src_graph::HasIndirectUserSecondLevel ::CREATE(GetGraph(), false);
    Graph *clone = CreateEmptyGraph();
    src_graph::HasIndirectUserSecondLevel ::CREATE(clone, false);
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheckElimination>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(StringFlatCheckEliminationTest, HasIndirectUserSecondLevelWithDifferentUsers)
{
    src_graph::HasIndirectUserSecondLevel ::CREATE(GetGraph(), true);
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheckElimination>());

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::HasIndirectUserSecondLevel ::CREATE(expectedGraph);
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

SRC_GRAPH(UserInTheDiamondPattern, Graph *graph, bool requiresFlatString)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 42U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(5U, Opcode::StringFlatCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_LT).b();
            INST(7U, Opcode::IfImm).Inputs(6U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(4U, 6U) {}
        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::Phi).ref().Inputs(5U, 5U);
            INST(9U, Opcode::SaveState).Inputs(8U, 0U).SrcVregs({2U, 4U});
            INST(10U, Opcode::NullCheck).ref().Inputs(8U, 9U);
            INST(11U, Opcode::SaveState).Inputs(8U, 0U).SrcVregs({2U, 4U});
            INST(12U, Opcode::NullCheck).ref().Inputs(0U, 11U);
            INST(13U, Opcode::SaveState).Inputs(8U, 0U).SrcVregs({2U, 4U});
            if (requiresFlatString) {
                INST(14U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                    .InputsAutoType(10U, 12U, 13U);
            } else {
                INST(14U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                    .InputsAutoType(12U, 10U, 13U);
            }
            INST(15U, Opcode::Return).b().Inputs(14U);
        }
    }
}

OUT_GRAPH(UserInTheDiamondPattern, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 42U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(16U, Opcode::NOP);
            INST(6U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_LT).b();
            INST(7U, Opcode::IfImm).Inputs(6U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(4U, 6U) {}
        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::Phi).ref().Inputs(0U, 0U);
            INST(9U, Opcode::SaveState).Inputs(8U, 0U).SrcVregs({2U, 4U});
            INST(10U, Opcode::NullCheck).ref().Inputs(8U, 9U);
            INST(11U, Opcode::SaveState).Inputs(8U, 0U).SrcVregs({2U, 4U});
            INST(12U, Opcode::NullCheck).ref().Inputs(0U, 11U);
            INST(13U, Opcode::SaveState).Inputs(8U, 0U).SrcVregs({2U, 4U});
            INST(14U, Opcode::Intrinsic)
                .b()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS)
                .InputsAutoType(12U, 10U, 13U);
            INST(15U, Opcode::Return).b().Inputs(14U);
        }
    }
}

TEST_F(StringFlatCheckEliminationTest, UserInTheDiamondPatternRequiresFlatString)
{
    src_graph::UserInTheDiamondPattern ::CREATE(GetGraph(), true);
    Graph *clone = CreateEmptyGraph();
    src_graph::UserInTheDiamondPattern ::CREATE(clone, true);
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheckElimination>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(StringFlatCheckEliminationTest, UserInTheDiamondPattern)
{
    src_graph::UserInTheDiamondPattern ::CREATE(GetGraph(), false);
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheckElimination>());

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::UserInTheDiamondPattern ::CREATE(expectedGraph);
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

SRC_GRAPH(HasMultipleUsers, Graph *graph, bool firstTrueUser)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(4U, Opcode::NullPtr).ref();
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({4U});
            INST(6U, Opcode::StringFlatCheck).ref().Inputs(4U, 5U);
            INST(7U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_GE).b();
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(ConditionCode::CC_NE);
        }
        BASIC_BLOCK(4U, 6U)
        {
            if (firstTrueUser) {
                INST(9U, Opcode::SaveState).Inputs(6U).SrcVregs({1U});
                INST(10U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_TO_LOCALE_LOWER_CASE)
                    .InputsAutoType(6U, 4U, 9U);
            } else {
                INST(10U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::REFERENCE).Inputs(6U, 4U);
            }
            INST(11U, Opcode::AddI).i32().Imm(1U).Inputs(1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(12U, Opcode::SaveState).Inputs(6U).SrcVregs({1U});
            INST(13U, Opcode::Intrinsic)
                .b()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_TO_LOCALE_LOWER_CASE)
                .InputsAutoType(4U, 6U, 12U);
            INST(14U, Opcode::AddI).i32().Imm(2U).Inputs(1U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(15U, Opcode::Phi).i32().Inputs(11U, 14U);
            INST(16U, Opcode::Return).i32().Inputs(15U);
        }
    }
}

TEST_F(StringFlatCheckEliminationTest, HasSingleTrueUser)
{
    src_graph::HasMultipleUsers ::CREATE(GetGraph(), false);
    Graph *clone = CreateEmptyGraph();
    src_graph::HasMultipleUsers ::CREATE(clone, false);
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheckElimination>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(StringFlatCheckEliminationTest, HasMultipleTrueUsers)
{
    src_graph::HasMultipleUsers ::CREATE(GetGraph(), true);
    Graph *clone = CreateEmptyGraph();
    src_graph::HasMultipleUsers ::CREATE(clone, true);
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheckElimination>());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// CC-OFFNXT(G.FUD.05) solid test logic
SRC_GRAPH(NoExpectedUsers, Graph *graph, bool intrinsicUser, bool needReversedUser)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(2U, Opcode::NullPtr).ref();
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({4U, 0U});
            INST(4U, Opcode::StringFlatCheck).Inputs(0U, 3U).ref();
            if (intrinsicUser && needReversedUser) {
                INST(9U, Opcode::SaveState).Inputs(0U, 2U, 4U).SrcVregs({4U, 0U, 1U});
                // we believe that intrinsic with ID -1 has no StringFlatCheckMask
                INST(5U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(static_cast<RuntimeInterface::IntrinsicId>(-1L))
                    .InputsAutoType(2U, 4U, 9U);
                INST(6U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(static_cast<RuntimeInterface::IntrinsicId>(-1L))
                    .InputsAutoType(4U, 2U, 9U);
                INST(7U, Opcode::And).b().Inputs(5U, 6U);
            } else if (intrinsicUser && !needReversedUser) {
                INST(9U, Opcode::SaveState).Inputs(0U, 2U, 4U).SrcVregs({4U, 0U, 1U});
                // we believe that intrinsic with ID -1 has no StringFlatCheckMask
                INST(5U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(static_cast<RuntimeInterface::IntrinsicId>(-1L))
                    .InputsAutoType(2U, 4U, 9U);
            } else if (!intrinsicUser && needReversedUser) {
                INST(5U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::REFERENCE).Inputs(4U, 2U);
                INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::REFERENCE).Inputs(2U, 4U);
                INST(7U, Opcode::And).b().Inputs(5U, 6U);
            } else {
                INST(5U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::REFERENCE).Inputs(4U, 2U);
            }
            INST(8U, Opcode::Return).Inputs(needReversedUser ? 7U : 5U).b();
        }
    }
}

// CC-OFFNXT(G.FUD.05) solid test logic
OUT_GRAPH(NoExpectedInUsers, Graph *graph, bool intrinsicUser, bool needReversedUser)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SafePoint).Inputs(0U).SrcVregs({4U});
            INST(2U, Opcode::NullPtr).ref();
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({4U, 0U});
            INST(intrinsicUser ? 10U : 9U, Opcode::NOP);
            if (intrinsicUser && needReversedUser) {
                INST(9U, Opcode::SaveState).Inputs(0U, 2U, 0U).SrcVregs({4U, 0U, 1U});
                INST(5U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(static_cast<RuntimeInterface::IntrinsicId>(-1L))
                    .InputsAutoType(2U, 0U, 9U);
                INST(6U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(static_cast<RuntimeInterface::IntrinsicId>(-1L))
                    .InputsAutoType(0U, 2U, 9U);
                INST(7U, Opcode::And).b().Inputs(5U, 6U);
            } else if (intrinsicUser && !needReversedUser) {
                INST(9U, Opcode::SaveState).Inputs(0U, 2U, 0U).SrcVregs({4U, 0U, 1U});
                INST(5U, Opcode::Intrinsic)
                    .b()
                    .IntrinsicId(static_cast<RuntimeInterface::IntrinsicId>(-1L))
                    .InputsAutoType(2U, 0U, 9U);
            } else if (!intrinsicUser && needReversedUser) {
                INST(5U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::REFERENCE).Inputs(0U, 2U);
                INST(6U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::REFERENCE).Inputs(2U, 0U);
                INST(7U, Opcode::And).b().Inputs(5U, 6U);
            } else {
                INST(5U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::REFERENCE).Inputs(0U, 2U);
            }
            INST(8U, Opcode::Return).Inputs(needReversedUser ? 7U : 5U).b();
        }
    }
}

TEST_F(StringFlatCheckEliminationTest, NoIntrinsicInSingleUser)
{
    src_graph::NoExpectedUsers ::CREATE(GetGraph(), false, false);
    GetGraph()->SetInliningComplete();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheckElimination>());

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::NoExpectedInUsers ::CREATE(expectedGraph, false, false);
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(StringFlatCheckEliminationTest, NoIntrinsicsInMultipleUsers)
{
    src_graph::NoExpectedUsers ::CREATE(GetGraph(), false, true);
    GetGraph()->SetInliningComplete();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheckElimination>());

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::NoExpectedInUsers ::CREATE(expectedGraph, false, true);
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(StringFlatCheckEliminationTest, EmptyStringFlatCheckMaskInSingleUser)
{
    src_graph::NoExpectedUsers ::CREATE(GetGraph(), true, false);
    ASSERT_EQ(GetStringFlatCheckArgMask(INS(5U).CastToIntrinsic()->GetIntrinsicId()), 0U);
    GetGraph()->SetInliningComplete();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheckElimination>());

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::NoExpectedInUsers ::CREATE(expectedGraph, true, false);
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

TEST_F(StringFlatCheckEliminationTest, EmptyStringFlatCheckMaskInMultipleUsers)
{
    src_graph::NoExpectedUsers ::CREATE(GetGraph(), true, true);
    ASSERT_EQ(GetStringFlatCheckArgMask(INS(5U).CastToIntrinsic()->GetIntrinsicId()), 0U);
    GetGraph()->SetInliningComplete();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheckElimination>());

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::NoExpectedInUsers ::CREATE(expectedGraph, true, true);
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
