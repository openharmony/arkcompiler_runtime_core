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

#include "libarkbase/macros.h"
#include "libarkbase/utils/utils.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/ir_constructor.h"
#include "tests/graph_comparator.h"
#include "unit_test.h"
#include "optimizer/optimizations/cleanup.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class SaveStateSuspendCheckerTest : public AsmTest {
public:
    SaveStateSuspendCheckerTest() = default;
    ~SaveStateSuspendCheckerTest() override = default;
    NO_COPY_SEMANTIC(SaveStateSuspendCheckerTest);
    NO_MOVE_SEMANTIC(SaveStateSuspendCheckerTest);

    // Override to handle null method in GraphChecker
    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        if (method == nullptr) {
            return "TestClass";
        }
        return AsmTest::GetClassNameFromMethod(method);
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        if (method == nullptr) {
            return "testMethod";
        }
        return AsmTest::GetMethodName(method);
    }
};

/*
 * Test that SaveStateSuspend with all live values included passes validation.
 */
TEST_F(SaveStateSuspendCheckerTest, AllLiveValuesPresent)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 42U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(4U, Opcode::SaveStateSuspend).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    GetGraph()->SetInliningComplete();
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test that missing a live value causes validation failure.
 * Value v3 dominates SaveStateSuspend v4 and v4 dominates its user v5,
 * but v3 is not an input of v4.
 */
TEST_F(SaveStateSuspendCheckerTest, MissingLiveValue)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            // Missing v3 in SaveStateSuspend inputs
            INST(4U, Opcode::SaveStateSuspend).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    GetGraph()->SetInliningComplete();
    GraphChecker checker(GetGraph());
#ifndef NDEBUG
    ASSERT_DEATH(checker.Check(), "");
#else
    ASSERT_FALSE(checker.Check());
#endif
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test that a value that does not dominate SaveStateSuspend is not required.
 * v3 is defined after SaveStateSuspend, so it's not live.
 */
TEST_F(SaveStateSuspendCheckerTest, ValueAfterSuspend)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    GetGraph()->SetInliningComplete();
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test that a value whose users are not dominated by SaveStateSuspend is not required.
 * v3 has user v5, but v4 does not dominate v5 (v5 is before v4).
 * However, v5 itself is used after v4 (by v6), so v5 must be included.
 */
TEST_F(SaveStateSuspendCheckerTest, UserNotDominated)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Add).s32().Inputs(3U, 3U);  // user of v3
            INST(4U, Opcode::SaveStateSuspend).Inputs(0U, 1U, 5U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::Return).s32().Inputs(5U);
        }
    }
    GetGraph()->SetInliningComplete();
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test Phi instruction handling.
 * Phi v3 dominates SaveStateSuspend v4 and v4 dominates its user v9.
 * Phi must be included in SaveStateSuspend inputs.
 * v5 does not dominate v4 (defined in loop body), so it's not required.
 */
TEST_F(SaveStateSuspendCheckerTest, PhiLiveValue)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 0U).s32();
        CONSTANT(6U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(2U, 5U);
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(3U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(5U, Opcode::Add).s32().Inputs(3U, 6U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::Return).s32().Inputs(3U);
        }
    }
    GetGraph()->SetInliningComplete();
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test loop case: value defined in loop latch block with user in loop header.
 * This is the specific case requested by the user.
 * v5 does not dominate exit block, so it's not required as input.
 */
TEST_F(SaveStateSuspendCheckerTest, LoopLatchToHeader)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 0U).s32();
        CONSTANT(6U, 1U).s32();
        // Loop header
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(2U, 5U);  // v3 defined in header, uses v5 from latch
            INST(7U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(3U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        // Loop body
        BASIC_BLOCK(3U, 2U)
        {
            INST(5U, Opcode::Add).s32().Inputs(3U, 6U);  // v5 defined in latch, used by v3 in header
        }
        // Exit block with SaveStateSuspend
        BASIC_BLOCK(4U, -1L)
        {
            // v5 does not dominate v4, so not required. Phi v3 dominates v4 and is live.
            INST(4U, Opcode::SaveStateSuspend).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::Return).s32().Inputs(3U);
        }
    }
    GetGraph()->SetInliningComplete();
    GraphChecker checker(GetGraph());
    ASSERT_TRUE(checker.Check());
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

}  // namespace ark::compiler