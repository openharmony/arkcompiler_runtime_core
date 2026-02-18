/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include <iostream>
#include "libabckit/src/adapter_static/ir_static.h"

namespace libabckit::test::adapter_static {
class IrStaticTest : public ::testing::Test {};

// ========================================
// Null argument tests for constant creation
// ========================================

TEST_F(IrStaticTest, IrStaticTestInvalid)
{
    EXPECT_EQ(GfindOrCreateConstantI64Static(nullptr, 0), nullptr);
    EXPECT_EQ(GfindOrCreateConstantI32Static(nullptr, 0), nullptr);
    EXPECT_EQ(GfindOrCreateConstantU64Static(nullptr, 0), nullptr);
    EXPECT_EQ(GfindOrCreateConstantF64Static(nullptr, 0), nullptr);
}

// ========================================
// Null argument tests for Graph manipulation
// ========================================

TEST_F(IrStaticTest, GgetStartBasicBlockStatic_Nullptr)
{
    EXPECT_EQ(GgetStartBasicBlockStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, GgetEndBasicBlockStatic_Nullptr)
{
    EXPECT_EQ(GgetEndBasicBlockStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, GgetBasicBlockStatic_Nullptr)
{
    EXPECT_EQ(GgetBasicBlockStatic(nullptr, 0), nullptr);
}

TEST_F(IrStaticTest, GgetNumberOfBasicBlocksStatic_Nullptr)
{
    EXPECT_EQ(GgetNumberOfBasicBlocksStatic(nullptr), 0U);
}

TEST_F(IrStaticTest, GgetParameterStatic_Nullptr)
{
    EXPECT_EQ(GgetParameterStatic(nullptr, 0), nullptr);
}

TEST_F(IrStaticTest, GvisitBlocksRPOStatic_NullGraph)
{
    bool result = GvisitBlocksRPOStatic(nullptr, nullptr, [](AbckitBasicBlock *, void *) { return true; });
    EXPECT_FALSE(result);
}

TEST_F(IrStaticTest, GvisitBlocksRPOStatic_NullCallback)
{
    // cb is nullptr -> returns false
    bool result = GvisitBlocksRPOStatic(nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);
}

// ========================================
// Null argument tests for BasicBlock manipulation
// ========================================

TEST_F(IrStaticTest, BBcreateEmptyStatic_Nullptr)
{
    EXPECT_EQ(BBcreateEmptyStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, BBgetFirstInstStatic_Nullptr)
{
    EXPECT_EQ(BBgetFirstInstStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, BBgetLastInstStatic_Nullptr)
{
    EXPECT_EQ(BBgetLastInstStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, BBgetGraphStatic_Nullptr)
{
    EXPECT_EQ(BBgetGraphStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, BBgetSuccBlockStatic_Nullptr)
{
    EXPECT_EQ(BBgetSuccBlockStatic(nullptr, 0), nullptr);
}

TEST_F(IrStaticTest, BBgetSuccBlockCountStatic_Nullptr)
{
    EXPECT_EQ(BBgetSuccBlockCountStatic(nullptr), 0U);
}

TEST_F(IrStaticTest, BBgetPredBlockStatic_Nullptr)
{
    EXPECT_EQ(BBgetPredBlockStatic(nullptr, 0), nullptr);
}

TEST_F(IrStaticTest, BBgetPredBlockCountStatic_Nullptr)
{
    EXPECT_EQ(BBgetPredBlockCountStatic(nullptr), 0U);
}

TEST_F(IrStaticTest, BBgetIdStatic_Nullptr)
{
    EXPECT_EQ(BBgetIdStatic(nullptr), 0U);
}

TEST_F(IrStaticTest, BBgetNumberOfInstructionsStatic_Nullptr)
{
    EXPECT_EQ(BBgetNumberOfInstructionsStatic(nullptr), 0U);
}

TEST_F(IrStaticTest, BBisStartStatic_Nullptr)
{
    EXPECT_FALSE(BBisStartStatic(nullptr));
}

TEST_F(IrStaticTest, BBisEndStatic_Nullptr)
{
    EXPECT_FALSE(BBisEndStatic(nullptr));
}

TEST_F(IrStaticTest, BBisLoopHeadStatic_Nullptr)
{
    EXPECT_FALSE(BBisLoopHeadStatic(nullptr));
}

TEST_F(IrStaticTest, BBisLoopPreheadStatic_Nullptr)
{
    EXPECT_FALSE(BBisLoopPreheadStatic(nullptr));
}

TEST_F(IrStaticTest, BBisTryBeginStatic_Nullptr)
{
    EXPECT_FALSE(BBisTryBeginStatic(nullptr));
}

TEST_F(IrStaticTest, BBisTryStatic_Nullptr)
{
    EXPECT_FALSE(BBisTryStatic(nullptr));
}

TEST_F(IrStaticTest, BBisTryEndStatic_Nullptr)
{
    EXPECT_FALSE(BBisTryEndStatic(nullptr));
}

TEST_F(IrStaticTest, BBisCatchBeginStatic_Nullptr)
{
    EXPECT_FALSE(BBisCatchBeginStatic(nullptr));
}

TEST_F(IrStaticTest, BBisCatchStatic_Nullptr)
{
    EXPECT_FALSE(BBisCatchStatic(nullptr));
}

TEST_F(IrStaticTest, BBgetTrueBranchStatic_Nullptr)
{
    EXPECT_EQ(BBgetTrueBranchStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, BBgetFalseBranchStatic_Nullptr)
{
    EXPECT_EQ(BBgetFalseBranchStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, BBgetImmediateDominatorStatic_Nullptr)
{
    EXPECT_EQ(BBgetImmediateDominatorStatic(nullptr), nullptr);
}

TEST_F(IrStaticTest, BBcheckDominanceStatic_NullBasicBlock)
{
    EXPECT_FALSE(BBcheckDominanceStatic(nullptr, nullptr));
}

TEST_F(IrStaticTest, BBsplitBlockAfterInstructionStatic_NullBB)
{
    EXPECT_EQ(BBsplitBlockAfterInstructionStatic(nullptr, nullptr, false), nullptr);
}

TEST_F(IrStaticTest, BBsplitBlockAfterInstructionStatic_NullBBAndInst)
{
    // Both args are null - covers the null-check branch for basicBlock (first arg)
    EXPECT_EQ(BBsplitBlockAfterInstructionStatic(nullptr, nullptr, false), nullptr);
}

// ========================================
// Null argument tests for BasicBlock visitors
// ========================================

TEST_F(IrStaticTest, BBvisitSuccBlocksStatic_NullBB)
{
    EXPECT_FALSE(BBvisitSuccBlocksStatic(nullptr, nullptr, [](AbckitBasicBlock *, void *) { return true; }));
}

TEST_F(IrStaticTest, BBvisitSuccBlocksStatic_NullCallback)
{
    EXPECT_FALSE(BBvisitSuccBlocksStatic(nullptr, nullptr, nullptr));
}

TEST_F(IrStaticTest, BBvisitPredBlocksStatic_NullBB)
{
    EXPECT_FALSE(BBvisitPredBlocksStatic(nullptr, nullptr, [](AbckitBasicBlock *, void *) { return true; }));
}

TEST_F(IrStaticTest, BBvisitPredBlocksStatic_NullCallback)
{
    EXPECT_FALSE(BBvisitPredBlocksStatic(nullptr, nullptr, nullptr));
}

TEST_F(IrStaticTest, BBvisitDominatedBlocksStatic_NullBB)
{
    EXPECT_FALSE(BBvisitDominatedBlocksStatic(nullptr, nullptr, [](AbckitBasicBlock *, void *) { return true; }));
}

TEST_F(IrStaticTest, BBvisitDominatedBlocksStatic_NullCallback)
{
    EXPECT_FALSE(BBvisitDominatedBlocksStatic(nullptr, nullptr, nullptr));
}

// ========================================
// Null argument tests for BasicBlock modification
// ========================================

TEST_F(IrStaticTest, BBaddInstFrontStatic_NullBB)
{
    BBaddInstFrontStatic(nullptr, nullptr);
}

TEST_F(IrStaticTest, BBaddInstBackStatic_NullBB)
{
    BBaddInstBackStatic(nullptr, nullptr);
}

TEST_F(IrStaticTest, BBappendSuccBlockStatic_NullBB)
{
    BBappendSuccBlockStatic(nullptr, nullptr);
}

TEST_F(IrStaticTest, BBinsertSuccBlockStatic_NullBB)
{
    BBinsertSuccBlockStatic(nullptr, nullptr, 0);
}

TEST_F(IrStaticTest, BBdisconnectSuccBlockStatic_NullBB)
{
    BBdisconnectSuccBlockStatic(nullptr, 0);
}

TEST_F(IrStaticTest, BBdumpStatic_NullBB)
{
    BBdumpStatic(nullptr, 1);
}

// ========================================
// Null argument tests for Instruction manipulation
// ========================================

TEST_F(IrStaticTest, IinsertAfterStatic_NullInst)
{
    IinsertAfterStatic(nullptr, nullptr);
}

TEST_F(IrStaticTest, IinsertBeforeStatic_NullInst)
{
    IinsertBeforeStatic(nullptr, nullptr);
}

}  // namespace libabckit::test::adapter_static
