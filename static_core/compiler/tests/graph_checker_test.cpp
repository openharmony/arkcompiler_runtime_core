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

#include "optimizer/ir/graph_checker.h"

#include <gtest/gtest.h>

namespace ark::compiler {
class GraphCheckerTest : public GraphTest {
protected:
    void ExpectChecker(bool expected)
    {
        GraphChecker checker(GetGraph());
        if (expected) {
            EXPECT_TRUE(checker.Check());
        } else {
#ifndef NDEBUG
            EXPECT_DEATH(checker.Check(), "");
#else
            EXPECT_FALSE(checker.Check());
#endif
        }
    }
};

// NOLINTBEGIN(readability-magic-numbers)
#if defined(COMPILER_DEBUG_CHECKS)
SRC_GRAPH(SaveStateAndSplitUsers, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({2U, 3U});
            INST(4U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0x0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(6U, Opcode::LoadObject).i32().Inputs(5U).TypeId(122U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Phi).i32().Inputs(2U, 6U);
            INST(8U, Opcode::Return).i32().Inputs(7U);
        }
    }
}

SRC_GRAPH(SaveStateAndSameBlockUsers, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0x0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({2U, 3U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(6U, Opcode::LoadObject).i32().Inputs(5U).TypeId(122U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Phi).i32().Inputs(2U, 6U);
            INST(8U, Opcode::Return).i32().Inputs(7U);
        }
    }
}

TEST_F(GraphCheckerTest, SaveStateAndSplitUsers)
{
    src_graph::SaveStateAndSplitUsers::CREATE(GetGraph());
    GetGraph()->SetInliningComplete();
    ExpectChecker(false);
}

TEST_F(GraphCheckerTest, SaveStateAndSameBlockUsers)
{
    src_graph::SaveStateAndSameBlockUsers::CREATE(GetGraph());
    GetGraph()->SetInliningComplete();
    ExpectChecker(true);
}
#else
TEST_F(GraphCheckerTest, RequiresCompilerDebugChecks)
{
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
}
#endif
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
