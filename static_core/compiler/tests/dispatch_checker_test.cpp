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

#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/ir_constructor.h"
#include "unit_test.h"
#include <vector>

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class DispatchCheckerTest : public AsmTest {
public:
    DispatchCheckerTest() = default;
    ~DispatchCheckerTest() override = default;
    NO_COPY_SEMANTIC(DispatchCheckerTest);
    NO_MOVE_SEMANTIC(DispatchCheckerTest);

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

    void BuildDispatchGraph(Graph *graph);
    void BuildDispatchInvalidInputTypeGraph(Graph *graph);
    void BuildDispatchMultipleInputsGraph(Graph *graph);
    void BuildDispatchNotInDedicatedBlockGraph(Graph *graph);
    void BuildDispatchDirectlyAfterStartGraph(Graph *graph);

    void SetupTryCatch(uint32_t tryBeginId, uint32_t tryEndId, uint32_t catchId,
                       const std::vector<uint32_t> &tryBlockIds);
};

#ifdef COMPILER_DEBUG_CHECKS
static bool CheckGraph(Graph *graph)
{
    graph->SetInliningComplete();
    GraphChecker checker(graph);
    return checker.Check();
}
#endif

void DispatchCheckerTest::SetupTryCatch(uint32_t tryBeginId, uint32_t tryEndId, uint32_t catchId,
                                        const std::vector<uint32_t> &tryBlockIds)
{
    for (auto blockId : tryBlockIds) {
        BB(blockId).SetTry(true);
        BB(blockId).SetTryId(0U);
    }
    BB(tryBeginId).SetTryBegin(true);
    BB(tryBeginId).SetTryId(0U);
    BB(tryEndId).SetTryEnd(true);
    BB(tryEndId).SetTryId(0U);
    BB(catchId).SetCatchBegin(true);
    BB(catchId).SetCatch(true);
}

void DispatchCheckerTest::BuildDispatchGraph(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 7U)  // Try-begin
        {
            INST(1U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(2U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U) {}  // Try-end
        BASIC_BLOCK(5U, -1L)
        {
            INST(3U, Opcode::Return).ref().Inputs(0U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(4U, Opcode::Dispatch).Inputs(0U);
        }
        BASIC_BLOCK(7U, -1L)  // Catch
        {
            INST(5U, Opcode::CatchPhi).ref().Inputs();
            INST(6U, Opcode::Return).ref().Inputs(0U);
        }

        SetupTryCatch(2U, 4U, 7U, {3U, 6U});
    }
}

void DispatchCheckerTest::BuildDispatchInvalidInputTypeGraph(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 7U)  // Try-begin
        {
            INST(1U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(2U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U) {}  // Try-end
        BASIC_BLOCK(5U, -1L)
        {
            INST(3U, Opcode::Return).s32().Inputs(0U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(4U, Opcode::Dispatch).Inputs(0U);
        }
        BASIC_BLOCK(7U, -1L)  // Catch
        {
            INST(5U, Opcode::CatchPhi).ref().Inputs();
            INST(6U, Opcode::Return).s32().Inputs(0U);
        }

        SetupTryCatch(2U, 4U, 7U, {3U, 6U});
    }
}

void DispatchCheckerTest::BuildDispatchMultipleInputsGraph(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U, 7U)  // Try-begin
        {
            INST(2U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U) {}  // Try-end
        BASIC_BLOCK(5U, -1L)
        {
            INST(4U, Opcode::Return).ref().Inputs(0U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(5U, Opcode::Dispatch).Inputs(0U, 1U);
        }
        BASIC_BLOCK(7U, -1L)  // Catch
        {
            INST(6U, Opcode::CatchPhi).ref().Inputs();
            INST(7U, Opcode::Return).ref().Inputs(0U);
        }

        SetupTryCatch(2U, 4U, 7U, {3U, 6U});
    }
}

void DispatchCheckerTest::BuildDispatchNotInDedicatedBlockGraph(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 7U)  // Try-begin
        {
            INST(1U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(2U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U) {}  // Try-end
        BASIC_BLOCK(5U, -1L)
        {
            INST(3U, Opcode::Return).ref().Inputs(0U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(4U, Opcode::Dispatch).Inputs(0U);
            INST(5U, Opcode::Return).ref().Inputs(0U);
        }
        BASIC_BLOCK(7U, -1L)  // Catch
        {
            INST(6U, Opcode::CatchPhi).ref().Inputs();
            INST(7U, Opcode::Return).ref().Inputs(0U);
        }

        SetupTryCatch(2U, 4U, 7U, {3U, 6U});
    }
}

void DispatchCheckerTest::BuildDispatchDirectlyAfterStartGraph(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 4U)  // Try-begin
        {
            INST(1U, Opcode::Try).CatchTypeIds({0x0U});
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(2U, Opcode::Dispatch).Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)  // Catch
        {
            INST(3U, Opcode::CatchPhi).ref().Inputs();
            INST(4U, Opcode::Return).ref().Inputs(0U);
        }

        SetupTryCatch(2U, 3U, 4U, {3U});
    }
}

/*
 * Test Dispatch in a dedicated terminator block from prologue branch.
 */
TEST_F(DispatchCheckerTest, Dispatch)
{
#ifdef COMPILER_DEBUG_CHECKS
    BuildDispatchGraph(GetGraph());
    ASSERT_TRUE(CheckGraph(GetGraph()));
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test that Dispatch with non-reference input fails validation.
 */
TEST_F(DispatchCheckerTest, DispatchInvalidInputType)
{
#ifdef COMPILER_DEBUG_CHECKS
#ifndef NDEBUG
    ASSERT_DEATH(
        {
            BuildDispatchInvalidInputTypeGraph(GetGraph());
            static_cast<void>(CheckGraph(GetGraph()));
        },
        "");  // Wrong input 0 type 'i32' for inst
#else
    BuildDispatchInvalidInputTypeGraph(GetGraph());
    ASSERT_FALSE(СheckGraph(GetGraph()));
#endif
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}
/*
 * Test that Dispatch with multiple inputs fails validation.
 */
TEST_F(DispatchCheckerTest, DispatchMultipleInputs)
{
#ifdef COMPILER_DEBUG_CHECKS
#ifndef NDEBUG
    ASSERT_DEATH(
        {
            BuildDispatchMultipleInputsGraph(GetGraph());
            static_cast<void>(CheckGraph(GetGraph()));
        },
        "");  // index < GetStaticInputsCapacity()
#else
    BuildDispatchMultipleInputsGraph(GetGraph());
    ASSERT_FALSE(СheckGraph(GetGraph()));
#endif
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test that Dispatch must be the only instruction in its block.
 */
TEST_F(DispatchCheckerTest, DispatchNotInDedicatedBlock)
{
#ifdef COMPILER_DEBUG_CHECKS
#ifndef NDEBUG
    ASSERT_DEATH(
        {
            BuildDispatchNotInDedicatedBlockGraph(GetGraph());
            static_cast<void>(CheckGraph(GetGraph()));
        },
        "");  // Dispatch must be the only executable instruction in its basic block
#else
    BuildDispatchNotInDedicatedBlockGraph(GetGraph());
    ASSERT_FALSE(СheckGraph(GetGraph()));
#endif
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/*
 * Test that Dispatch must be reached from post-start prologue branch block.
 */
TEST_F(DispatchCheckerTest, DispatchDirectlyAfterStart)
{
#ifdef COMPILER_DEBUG_CHECKS
#ifndef NDEBUG
    ASSERT_DEATH(
        {
            BuildDispatchDirectlyAfterStartGraph(GetGraph());
            static_cast<void>(CheckGraph(GetGraph()));
        },
        "");  // Dispatch prologue block must not be a try-begin block
#else
    BuildDispatchDirectlyAfterStartGraph(GetGraph());
    ASSERT_FALSE(СheckGraph(GetGraph()));
#endif
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

}  // namespace ark::compiler
