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

#include "compiler_options.h"
#include "gmock/gmock.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/utils.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/ir_constructor.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "tests/graph_comparator.h"
#include "unit_test.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/optimizations/fill_savestate_suspend_inputs.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class FillSaveStateSuspendInputsTest : public AsmTest {
public:
    FillSaveStateSuspendInputsTest() = default;
    ~FillSaveStateSuspendInputsTest() override = default;
    NO_COPY_SEMANTIC(FillSaveStateSuspendInputsTest);
    NO_MOVE_SEMANTIC(FillSaveStateSuspendInputsTest);

    void RunFillSaveStateSuspendInputs()
    {
        ASSERT_TRUE(GetGraph()->RunPass<FillSaveStateSuspendInputs>());
        ASSERT_TRUE(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
        ASSERT_FALSE(GetGraph()->GetAnalysis<LivenessAnalyzer>().IsTargetSpecificComputed());
    }

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

    static Inst *GetInstById(Graph *graph, unsigned id)
    {
        for (auto *bb : graph->GetVectorBlocks()) {
            if (auto *inst = FindInstInBlock(bb, id)) {
                return inst;
            }
        }
        return nullptr;
    }

    static constexpr int BRIDGE_VREG = static_cast<int>(VirtualRegister::BRIDGE);

private:
    template <typename InstRange>
    static Inst *FindInstInRange(const InstRange &range, unsigned id)
    {
        for (auto *inst : range) {
            if (inst != nullptr && inst->GetId() == id) {
                return inst;
            }
        }
        return nullptr;
    }

    static Inst *FindInstInBlock(BasicBlock *bb, unsigned id)
    {
        if (auto *inst = FindInstInRange(bb->PhiInsts(), id)) {
            return inst;
        }
        return FindInstInRange(bb->AllInsts(), id);
    }
};

// NOLINTBEGIN(readability-magic-numbers)

/**
 * Test 1: Live primitive (INT32) values should be added as bridge inputs.
 * Linear CFG: entry -> block2 -> exit.
 */
TEST_F(FillSaveStateSuspendInputsTest, LivePrimitiveValuesAddedAsBridgeInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 0U, 1U).SrcVregs({4U, BRIDGE_VREG, BRIDGE_VREG});
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/**
 * Test 2: Live object (REFERENCE) values should be added as bridge inputs.
 * Only param0 is used after SSS → only param0 is live.
 */
TEST_F(FillSaveStateSuspendInputsTest, LiveObjectValuesAddedAsBridgeInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(5U, Opcode::Return).ref().Inputs(0U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 0U).SrcVregs({4U, BRIDGE_VREG});
            INST(5U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/**
 * Test 3: Non-live values should NOT be added.
 * Add is live (used in Return); param0 is only used in Add (before SSS) → not live at SSS.
 */
TEST_F(FillSaveStateSuspendInputsTest, NonLiveValuesNotAdded)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        CONSTANT(2U, 10U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 2U);
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        CONSTANT(2U, 10U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 2U);
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 3U).SrcVregs({4U, BRIDGE_VREG});
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/**
 * Test 4: Existing inputs should NOT be removed. New live values should be appended.
 * SaveStateSuspend has param0 as pre-existing input; param1 is live and must be added.
 */
TEST_F(FillSaveStateSuspendInputsTest, ExistingInputsNotRemovedNoDuplicates)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 0U).SrcVregs({4U, 0U});
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 0U, 1U).SrcVregs({4U, 0, BRIDGE_VREG});
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/// Test 5: Pass should return false if no SaveStateSuspend instructions.
TEST_F(FillSaveStateSuspendInputsTest, NoSaveStateSuspendReturnsFalse)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::Return).s32().Inputs(0U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<FillSaveStateSuspendInputs>());
    ASSERT_TRUE(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
    ASSERT_FALSE(GetGraph()->GetAnalysis<LivenessAnalyzer>().IsTargetSpecificComputed());

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::Return).s32().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/**
 * Test 6: Multiple SaveStateSuspend instructions with different live sets.
 * SSS1: param0 live (used in Add). SSS2: Add live (used in Return); param0 not live at SSS2.
 */
TEST_F(FillSaveStateSuspendInputsTest, MultipleSaveStateSuspendDifferentLiveSets)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        CONSTANT(2U, 10U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(3U, Opcode::Add).s32().Inputs(0U, 2U);
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        CONSTANT(2U, 10U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::SaveStateSuspend).Inputs(14U, 0U, 2U).SrcVregs({4U, BRIDGE_VREG, BRIDGE_VREG});
            INST(3U, Opcode::Add).s32().Inputs(0U, 2U);
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U, 3U).SrcVregs({4U, BRIDGE_VREG});
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    OptimizeSaveStateConstantInputs(GetInstById(expectedGraph, 6U)->CastToSaveStateSuspend());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/// Test that graph flag IsSaveStateSuspendInputsAllocated is set after the pass.
TEST_F(FillSaveStateSuspendInputsTest, GraphFlagSetAfterPass)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(5U, Opcode::Return).s32().Inputs(0U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 0U).SrcVregs({4U, BRIDGE_VREG});
            INST(5U, Opcode::Return).s32().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/// Test that GraphChecker passes after the fill pass runs.
TEST_F(FillSaveStateSuspendInputsTest, GraphCheckerPassesAfterFillPass)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 0U, 1U).SrcVregs({4U, BRIDGE_VREG, BRIDGE_VREG});
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

/// Test 9: Constants added as bridge inputs should be converted to immediates.
TEST_F(FillSaveStateSuspendInputsTest, ConstantsConvertedToImmediates)
{
    GRAPH(GetGraph())
    {
        PARAMETER(14U, 0U).ref();
        CONSTANT(0U, 42U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(5U, Opcode::Return).s64().Inputs(0U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(14U, 0U).ref();
        CONSTANT(0U, 42U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U, 0U).SrcVregs({4U, BRIDGE_VREG});
            INST(5U, Opcode::Return).s64().Inputs(0U);
        }
    }
    OptimizeSaveStateConstantInputs(GetInstById(expectedGraph, 4U)->CastToSaveStateSuspend());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

// --- CFG variation tests: if-branch ---

/**
 * Test: If-branch CFG. SaveStateSuspend in then-branch; then-branch returns, else-branch goes to merge.
 *   entry -> 2 (if) -> 3 (then: SSS, Add, Return) or 4 (else) -> 5 (merge) -> return.
 * Live at SSS in block 3: param0 and param1 (used in Add/Return after SSS).
 */
// CC-OFFNXT(huge_method, G.FUD.05) graph creation
TEST_F(FillSaveStateSuspendInputsTest, IfBranchSaveStateSuspendInThenBranch)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(8U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(10U);
            INST(12U, Opcode::Return).s32().Inputs(11U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U, 0U, 1U).SrcVregs({4U, BRIDGE_VREG, BRIDGE_VREG});
            INST(8U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Phi).s32().Inputs(10U);
            INST(12U, Opcode::Return).s32().Inputs(11U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/// Test: If-branch with SSS in false branch. Same liveness idea.
// CC-OFFNXT(huge_method, G.FUD.05) graph creation
TEST_F(FillSaveStateSuspendInputsTest, IfBranchSaveStateSuspendInElseBranch)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        CONSTANT(2U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(8U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::Phi).s32().Inputs(6U, 8U);
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        CONSTANT(2U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U, 0U, 1U).SrcVregs({4U, BRIDGE_VREG, BRIDGE_VREG});
            INST(8U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::Phi).s32().Inputs(6U, 8U);
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

// --- CFG variation tests: loop ---

/**
 * Test: Simple loop with SaveStateSuspend in loop body.
 * entry -> loop (Phi, SSS, Add, back-edge) -> exit. Phi and Add are live at SSS.
 */
// CC-OFFNXT(huge_method, G.FUD.05) graph creation
TEST_F(FillSaveStateSuspendInputsTest, LoopWithSaveStateSuspendInBody)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        CONSTANT(2U, 1U).s32();
        CONSTANT(3U, 100U).s32();
        BASIC_BLOCK(2U, 2U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Phi).s32().Inputs(1U, 9U);
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(8U, Opcode::Add).s32().Inputs(5U, 2U);
            INST(9U, Opcode::Add).s32().Inputs(6U, 2U);
            INST(10U, Opcode::Compare).b().CC(CC_GE).Inputs(8U, 3U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(12U, Opcode::Return).s32().Inputs(6U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        CONSTANT(2U, 1U).s32();
        CONSTANT(3U, 100U).s32();
        BASIC_BLOCK(2U, 2U, 4U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(0U, 8U);
            INST(6U, Opcode::Phi).s32().Inputs(1U, 9U);
            INST(7U, Opcode::SaveStateSuspend)
                .Inputs(14U, 6U, 5U, 2U, 3U)
                .SrcVregs({4U, BRIDGE_VREG, BRIDGE_VREG, BRIDGE_VREG, BRIDGE_VREG});
            INST(8U, Opcode::Add).s32().Inputs(5U, 2U);
            INST(9U, Opcode::Add).s32().Inputs(6U, 2U);
            INST(10U, Opcode::Compare).b().CC(CC_GE).Inputs(8U, 3U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(12U, Opcode::Return).s32().Inputs(6U);
        }
    }
    OptimizeSaveStateConstantInputs(GetInstById(expectedGraph, 7U)->CastToSaveStateSuspend());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

/**
 * Test: Loop with two exits; SaveStateSuspend in header.
 * Header 2 -> body 3 or exit 4; body 3 -> header 2 or exit 5. Values used in exits are live at SSS.
 */
// CC-OFFNXT(huge_method, G.FUD.05) graph creation
TEST_F(FillSaveStateSuspendInputsTest, LoopTwoExitsSaveStateSuspendInHeader)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        CONSTANT(2U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(8U, Opcode::Compare).b().CC(CC_EQ).Inputs(6U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(3U, 2U, 5U)
        {
            INST(10U, Opcode::Add).s32().Inputs(6U, 1U);
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s32().Inputs(6U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(12U, Opcode::Return).s32().Inputs(1U);
        }
    }
    RunFillSaveStateSuspendInputs();

    Graph *expectedGraph = CreateGraph();
    GRAPH(expectedGraph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(14U, 2U).ref();
        CONSTANT(2U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveStateSuspend)
                .Inputs(14U, 1U, 6U, 2U)
                .SrcVregs({4U, BRIDGE_VREG, BRIDGE_VREG, BRIDGE_VREG});
            INST(8U, Opcode::Compare).b().CC(CC_EQ).Inputs(6U, 2U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(3U, 2U, 5U)
        {
            INST(10U, Opcode::Add).s32().Inputs(6U, 1U);
            INST(13U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s32().Inputs(6U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(12U, Opcode::Return).s32().Inputs(1U);
        }
    }
    OptimizeSaveStateConstantInputs(GetInstById(expectedGraph, 7U)->CastToSaveStateSuspend());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

namespace {
[[maybe_unused]] auto GetVRegs(SaveStateInst *ss)
{
    std::vector<VirtualRegister::ValueType> vregs;
    for (size_t i = 0; i < ss->GetInputsCount(); i++) {
        vregs.push_back(ss->GetVirtualRegister(i).Value());
    }
    return vregs;
}
}  // namespace

/// Test that running FillSaveStateSuspendInputs pass in compilation pipeline works.
TEST_F(FillSaveStateSuspendInputsTest, CompilationPipeline)
{
#ifdef COMPILER_DEBUG_CHECKS
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(14U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveStateSuspend).Inputs(14U).SrcVregs({4U});
            INST(5U, Opcode::Return).s32().Inputs(0U);
        }
    }
    // Run FillSaveStateSuspendInputs as part of register allocation pipeline
    EXPECT_TRUE(RegAlloc(GetGraph()));
    EXPECT_TRUE(GetGraph()->IsSaveStateSuspendInputsAllocated());

    auto *saveStateSuspend = INS(4U).CastToSaveStateSuspend();

    using namespace ::testing;
    auto inputIdsAre = [](auto... ids) {
        return ElementsAre(Property(&Input::GetInst, Property(&Inst::GetId, ids))...);
    };
    EXPECT_THAT(saveStateSuspend->GetInputs(), inputIdsAre(14U, 0U));
    EXPECT_THAT(GetVRegs(saveStateSuspend), ElementsAre(4U, BRIDGE_VREG));
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

namespace {
class OpBuilder {
public:
    OpBuilder() {}

    void PushNextArg(size_t arg)
    {
        toUseNext_.push_back(arg);
    }

    std::optional<size_t> PopArg()
    {
        if (toUse_.empty()) {
            return {};
        }
        size_t arg = toUse_.back();
        toUse_.pop_back();
        return arg;
    }

    void Next()
    {
        toUse_.swap(toUseNext_);
        toUseNext_.clear();
    }

    const std::vector<size_t> &GetArgs()
    {
        return toUse_;
    }

    const std::vector<size_t> &GetNextArgs()
    {
        return toUseNext_;
    }

    void Reset()
    {
        toUse_.clear();
        toUseNext_.clear();
    }

private:
    std::vector<size_t> toUse_;
    std::vector<size_t> toUseNext_;
};

template <typename SourceCallback>
OpBuilder InitOps(size_t numSources, SourceCallback &&source)
{
    OpBuilder builder;
    for (size_t i = 0; i < numSources; i++) {
        builder.PushNextArg(source(i));
    }
    builder.Next();
    return builder;
}

template <typename OpCallback>
void BuildOps(OpBuilder &builder, OpCallback &&op)
{
    ASSERT_FALSE(builder.GetArgs().empty());
    auto fallback = builder.GetArgs().front();
    while (builder.GetArgs().size() > 1) {
        do {
            auto lhs = builder.PopArg();
            ASSERT_TRUE(lhs.has_value());
            auto rhs = builder.PopArg();
            builder.PushNextArg(op(*lhs, rhs.value_or(fallback)));
        } while (!builder.GetArgs().empty());
        builder.Next();
    }
}
}  // namespace

/*
 * Create a graph with the following binary tree structure (for 7 inputs):
 *          Return
 *            |
 *        ___Add___
 *       /         \
 *    Add           Add
 *   /   \         /   \
 * Add    Add    Add   Add
 * ||     / \    / \   / \
 * C1    C2 C3  C4 C5 C6 C7
 *
 * After each Add a SaveStateSuspend and a CallStatic is inserted.
 */
SRC_GRAPH(BigGraph, Graph *graph, size_t numSources)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        // Create many constants that will be live across the same range
        size_t id = 1;
        auto adder = InitOps(numSources, [this, &id](size_t i) {
            CONSTANT(id, i);
            return id++;
        });
        BASIC_BLOCK(2U, -1L)
        {
            BuildOps(adder, [this, &id](size_t lhs, size_t rhs) {
                INST(id, Opcode::Add).u64().Inputs(lhs, rhs);
                size_t arg = id++;
                INST(id++, Opcode::SaveStateSuspend).Inputs(0U).SrcVregs({4U});
                INST(id++, Opcode::SaveState).NoVregs();
                INST(id, Opcode::CallStatic).v0id().InputsAutoType(id - 1);
                id++;
                return arg;
            });
            INST(id++, Opcode::Return).u64().Inputs(*adder.PopArg());
        }
    }
}

/// Same, but with more complicated graph.
TEST_F(FillSaveStateSuspendInputsTest, BigGraph)
{
#ifdef COMPILER_DEBUG_CHECKS
    constexpr size_t NUM_CONSTANTS = 300U;
    EXPECT_GT(NUM_CONSTANTS, MAX_NUM_IMM_SLOTS);

    src_graph::BigGraph::CREATE(GetGraph(), NUM_CONSTANTS);
    EXPECT_TRUE(RegAlloc(GetGraph()));
    EXPECT_TRUE(GetGraph()->IsSaveStateSuspendInputsAllocated());

    size_t id = 1;  // skip Parameter
    auto adder = InitOps(NUM_CONSTANTS, [&id]([[maybe_unused]] size_t i) { return id++; });
    BuildOps(adder, [this, &adder, &id]([[maybe_unused]] size_t lhs, [[maybe_unused]] size_t rhs) {
        size_t arg = id++;  // skip Add
        auto &saveStateSuspend = INS(id++);
        EXPECT_TRUE(saveStateSuspend.IsSaveStateSuspend());
        for (auto live : adder.GetNextArgs()) {
            using namespace ::testing;
            EXPECT_THAT(saveStateSuspend.CastToSaveStateSuspend()->GetInputs(),
                        Contains(Property(&Input::GetInst, Property(&Inst::GetId, live))));
        }
        id += 2U;  // skip SaveState and CallStatic
        return arg;
    });
#else
    GTEST_SKIP() << "Test requires COMPILER_DEBUG_CHECKS";
#endif
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
