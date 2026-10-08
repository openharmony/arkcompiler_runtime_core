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

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <utility>

#include "dynamic_cse.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "libpandabase/mem/arena_allocator.h"
#include "libpandabase/mem/mem.h"
#include "libpandabase/mem/pool_manager.h"

namespace panda::bytecodeopt::test {

using compiler::BasicBlock;
using compiler::Graph;
using compiler::Inst;
using compiler::RuntimeInterface;
namespace DataType = compiler::DataType;

class DynamicCseTest : public testing::Test {
public:
    DynamicCseTest()
    {
        mem::MemConfig::Initialize(128_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_INTERNAL);
        localAllocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_INTERNAL);
    }

    ~DynamicCseTest() override
    {
        delete allocator_;
        delete localAllocator_;
        PoolManager::Finalize();
        mem::MemConfig::Finalize();
    }

protected:
    Graph *CreateGraph()
    {
        auto *graph = allocator_->New<Graph>(allocator_, localAllocator_, Arch::NONE, true, true);
        auto *start = graph->CreateStartBlock();
        body_ = graph->CreateEmptyBlock();
        auto *end = graph->CreateEndBlock();
        start->AddSucc(body_);
        body_->AddSucc(end);
        return graph;
    }

    Graph *CreateLinearGraph(BasicBlock **first, BasicBlock **second)
    {
        auto *graph = allocator_->New<Graph>(allocator_, localAllocator_, Arch::NONE, true, true);
        auto *start = graph->CreateStartBlock();
        *first = graph->CreateEmptyBlock();
        *second = graph->CreateEmptyBlock();
        auto *end = graph->CreateEndBlock();
        start->AddSucc(*first);
        (*first)->AddSucc(*second);
        (*second)->AddSucc(end);
        body_ = *first;
        return graph;
    }

    Inst *AddParameter(Graph *graph, int id, uint16_t argNumber)
    {
        auto *param = graph->AddNewParameter(argNumber);
        param->SetId(id);
        param->SetType(DataType::ANY);
        return param;
    }

    Inst *AddIntrinsic(Graph *graph, int id, RuntimeInterface::IntrinsicId intrinsicId,
                       std::initializer_list<Inst *> inputs)
    {
        return AddIntrinsicToBlock(graph, body_, id, intrinsicId, inputs);
    }

    Inst *AddIntrinsicToBlock(Graph *graph, BasicBlock *block, int id, RuntimeInterface::IntrinsicId intrinsicId,
                              std::initializer_list<Inst *> inputs)
    {
        auto *inst = graph->CreateInstIntrinsic(DataType::ANY, compiler::INVALID_PC, intrinsicId);
        inst->ClearFlag(compiler::inst_flags::REQUIRE_STATE);
        inst->ReserveInputs(inputs.size());
        inst->AllocateInputTypes(graph->GetAllocator(), inputs.size());
        for (auto *input : inputs) {
            inst->AppendInput(input);
            inst->AddInputType(DataType::ANY);
        }
        inst->SetId(id);
        block->AppendInst(inst);
        return inst;
    }

    Inst *AddSuperCheckToBlock(BasicBlock *block, int id, RuntimeInterface::IntrinsicId intrinsicId, Inst *input,
                               uint32_t immediate)
    {
        auto *graph = block->GetGraph();
        auto *inst = graph->CreateInstIntrinsic(DataType::VOID, compiler::INVALID_PC, intrinsicId);
        inst->ClearFlag(compiler::inst_flags::REQUIRE_STATE);
        inst->ReserveInputs(1U);
        inst->AllocateInputTypes(graph->GetAllocator(), 1U);
        inst->AppendInput(input);
        inst->AddInputType(DataType::ANY);
        inst->CastToIntrinsic()->AddImm(graph->GetAllocator(), immediate);
        inst->SetFlag(compiler::inst_flags::ACC_READ);
        inst->SetId(id);
        block->AppendInst(inst);
        return inst;
    }

    Inst *AddSuperCheck(int id, RuntimeInterface::IntrinsicId intrinsicId, Inst *input, uint32_t immediate)
    {
        return AddSuperCheckToBlock(body_, id, intrinsicId, input, immediate);
    }

    Inst *AddReturn(Graph *graph, int id, Inst *input)
    {
        return AddReturnToBlock(graph, body_, id, input);
    }

    Inst *AddReturnToBlock(Graph *graph, BasicBlock *block, int id, Inst *input)
    {
        auto *ret = graph->CreateInstReturn(DataType::ANY, compiler::INVALID_PC);
        ret->SetId(id);
        ret->SetInput(0, input);
        block->AppendInst(ret);
        return ret;
    }

    Inst *AddIf(Graph *graph, BasicBlock *block, int id, Inst *condition)
    {
        auto *ifInst = graph->CreateInstIfImm();
        ifInst->SetId(id);
        ifInst->SetInput(0, condition);
        ifInst->SetOperandsType(DataType::BOOL);
        ifInst->SetCc(compiler::ConditionCode::CC_NE);
        ifInst->SetImm(0);
        block->AppendInst(ifInst);
        return ifInst;
    }

    void RunDynamicCse(Graph *graph, bool expectedChanged)
    {
        EXPECT_FALSE(graph->RunPass<compiler::ValNum>());
        EXPECT_EQ(graph->RunPass<DynamicCse>(), expectedChanged);
    }

    void CheckEliminatesRepeatedIntrinsic(RuntimeInterface::IntrinsicId intrinsicId, bool unary)
    {
        auto *graph = CreateGraph();
        auto *left = AddParameter(graph, 0, 0);
        auto *right = AddParameter(graph, 1, 1);
        auto *producer =
            unary ? AddIntrinsic(graph, 2, intrinsicId, {left}) : AddIntrinsic(graph, 2, intrinsicId, {left, right});
        auto *current =
            unary ? AddIntrinsic(graph, 3, intrinsicId, {left}) : AddIntrinsic(graph, 3, intrinsicId, {left, right});
        auto *producerUser =
            AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
        auto *currentUser =
            AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
        AddReturn(graph, 6, currentUser);

        RunDynamicCse(graph, true);

        EXPECT_EQ(currentUser->GetInput(0).GetInst(), producer);
        EXPECT_FALSE(current->HasUsers());
        EXPECT_EQ(producerUser->GetInput(0).GetInst(), producer);
    }

    ArenaAllocator *allocator_ {nullptr};
    ArenaAllocator *localAllocator_ {nullptr};
    BasicBlock *body_ {nullptr};
};

TEST_F(DynamicCseTest, EliminatesRepeatedStrictEqWhenProducerIsAlreadyLiveAcrossUse)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *producerUser = AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::ISFALSE, {producer});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::ISTRUE, {current});
    AddReturn(graph, 6, currentUser);

    RunDynamicCse(graph, true);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), producer);
    EXPECT_FALSE(current->HasUsers());
    EXPECT_EQ(producerUser->GetInput(0).GetInst(), producer);
}

TEST_F(DynamicCseTest, EliminatesRepeatedStrictNotEq)
{
    CheckEliminatesRepeatedIntrinsic(RuntimeInterface::IntrinsicId::STRICTNOTEQ_IMM8_V8, false);
}

TEST_F(DynamicCseTest, EliminatesRepeatedIsTrue)
{
    CheckEliminatesRepeatedIntrinsic(RuntimeInterface::IntrinsicId::ISTRUE, true);
}

TEST_F(DynamicCseTest, EliminatesRepeatedIsFalse)
{
    CheckEliminatesRepeatedIntrinsic(RuntimeInterface::IntrinsicId::ISFALSE, true);
}

TEST_F(DynamicCseTest, IgnoresInlineCacheSlotImmediate)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    producer->CastToIntrinsic()->AddImm(graph->GetAllocator(), 1U);
    auto *current = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    current->CastToIntrinsic()->AddImm(graph->GetAllocator(), 99U);
    AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 6, currentUser);

    RunDynamicCse(graph, true);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), producer);
}

TEST_F(DynamicCseTest, DoesNotExtendProducerLifetime)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *current = AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 6, currentUser);

    RunDynamicCse(graph, false);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, DoesNotMergeDifferentOperands)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *other = AddParameter(graph, 2, 2);
    auto *producer = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current = AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, other});
    AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 6, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 7, currentUser);

    RunDynamicCse(graph, false);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, DoesNotMergeWhenInputValueNumbersAreUnavailable)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 6, currentUser);

    EXPECT_FALSE(graph->RunPass<DynamicCse>());

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, DoesNotMergeCallRuntimeBooleanConversion)
{
    auto *graph = CreateGraph();
    auto *value = AddParameter(graph, 0, 0);
    auto *producer = AddIntrinsic(graph, 1, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {value});
    auto *current = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {value});
    AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {current});
    AddReturn(graph, 5, currentUser);

    RunDynamicCse(graph, false);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, DoesNotMergeArithmeticIntrinsic)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::ADD2_IMM8_V8, {left, right});
    auto *current = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::ADD2_IMM8_V8, {left, right});
    AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 6, currentUser);

    RunDynamicCse(graph, false);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, DoesNotMergeDifferentIntrinsicIds)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::STRICTNOTEQ_IMM8_V8, {left, right});
    AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 6, currentUser);

    RunDynamicCse(graph, false);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, DoesNotMergeDifferentOutputTypes)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    current->SetType(DataType::BOOL);
    AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 6, currentUser);

    RunDynamicCse(graph, false);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, IgnoresDifferentSaveStateInputs)
{
    auto *graph = CreateGraph();
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *producerState = graph->CreateInstSaveState();
    producerState->SetId(3);
    producer->InsertBefore(producerState);
    producer->AppendInput(producerState);
    producer->CastToIntrinsic()->AddInputType(DataType::NO_TYPE);
    producer->SetFlag(compiler::inst_flags::REQUIRE_STATE);
    auto *current = AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *currentState = graph->CreateInstSaveState();
    currentState->SetId(5);
    current->InsertBefore(currentState);
    current->AppendInput(currentState);
    current->CastToIntrinsic()->AddInputType(DataType::NO_TYPE);
    current->SetFlag(compiler::inst_flags::REQUIRE_STATE);
    AddIntrinsic(graph, 6, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 7, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 8, currentUser);

    RunDynamicCse(graph, true);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), producer);
}

TEST_F(DynamicCseTest, EliminatesAcrossDominatedNonEhBlocks)
{
    BasicBlock *first = nullptr;
    BasicBlock *second = nullptr;
    auto *graph = CreateLinearGraph(&first, &second);
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer =
        AddIntrinsicToBlock(graph, first, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current =
        AddIntrinsicToBlock(graph, second, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddIntrinsicToBlock(graph, second, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser =
        AddIntrinsicToBlock(graph, second, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturnToBlock(graph, second, 6, currentUser);

    RunDynamicCse(graph, true);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), producer);
}

TEST_F(DynamicCseTest, DoesNotEliminateAcrossEhBlocks)
{
    BasicBlock *first = nullptr;
    BasicBlock *second = nullptr;
    auto *graph = CreateLinearGraph(&first, &second);
    first->SetTry(true);
    second->SetTry(true);
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer =
        AddIntrinsicToBlock(graph, first, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current =
        AddIntrinsicToBlock(graph, second, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddIntrinsicToBlock(graph, second, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser =
        AddIntrinsicToBlock(graph, second, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturnToBlock(graph, second, 6, currentUser);

    RunDynamicCse(graph, false);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), current);
}

TEST_F(DynamicCseTest, EliminatesWithinOneEhBlock)
{
    auto *graph = CreateGraph();
    body_->SetTry(true);
    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *producer = AddIntrinsic(graph, 2, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current = AddIntrinsic(graph, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddIntrinsic(graph, 4, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8, {producer});
    auto *currentUser = AddIntrinsic(graph, 5, RuntimeInterface::IntrinsicId::CALLRUNTIME_ISTRUE_PREF_IMM8, {current});
    AddReturn(graph, 6, currentUser);

    RunDynamicCse(graph, true);

    EXPECT_EQ(currentUser->GetInput(0).GetInst(), producer);
}

TEST_F(DynamicCseTest, DoesNotReuseExpressionFromOneDiamondBranchAtMerge)
{
    auto *graph = allocator_->New<Graph>(allocator_, localAllocator_, Arch::NONE, true, true);
    auto *start = graph->CreateStartBlock();
    auto *branch = graph->CreateEmptyBlock();
    auto *leftBlock = graph->CreateEmptyBlock();
    auto *rightBlock = graph->CreateEmptyBlock();
    auto *merge = graph->CreateEmptyBlock();
    auto *end = graph->CreateEndBlock();
    start->AddSucc(branch);
    branch->AddSucc(leftBlock);
    branch->AddSucc(rightBlock);
    leftBlock->AddSucc(merge);
    rightBlock->AddSucc(merge);
    merge->AddSucc(end);

    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *condition = AddParameter(graph, 2, 2);
    condition->SetType(DataType::BOOL);
    AddIf(graph, branch, 3, condition);
    AddIntrinsicToBlock(graph, leftBlock, 4, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    auto *current =
        AddIntrinsicToBlock(graph, merge, 5, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddReturnToBlock(graph, merge, 6, current);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(current->HasUsers());
}

TEST_F(DynamicCseTest, DoesNotUseLoopBodyExpressionForFirstHeaderExecution)
{
    auto *graph = allocator_->New<Graph>(allocator_, localAllocator_, Arch::NONE, true, true);
    auto *start = graph->CreateStartBlock();
    auto *header = graph->CreateEmptyBlock();
    auto *body = graph->CreateEmptyBlock();
    auto *exit = graph->CreateEmptyBlock();
    auto *end = graph->CreateEndBlock();
    start->AddSucc(header);
    header->AddSucc(body);
    header->AddSucc(exit);
    body->AddSucc(header);
    exit->AddSucc(end);

    auto *left = AddParameter(graph, 0, 0);
    auto *right = AddParameter(graph, 1, 1);
    auto *condition = AddParameter(graph, 2, 2);
    condition->SetType(DataType::BOOL);
    auto *current =
        AddIntrinsicToBlock(graph, header, 3, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddIf(graph, header, 4, condition);
    AddIntrinsicToBlock(graph, body, 5, RuntimeInterface::IntrinsicId::STRICTEQ_IMM8_V8, {left, right});
    AddReturnToBlock(graph, exit, 6, current);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(current->HasUsers());
}

TEST_F(DynamicCseTest, EliminatesRepeatedSuperCheckWithSameSsaAndMode)
{
    auto *graph = CreateGraph();
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheck(1, id, value, 0U);
    auto *current = AddSuperCheck(2, id, value, 0U);
    AddReturn(graph, 3, value);

    RunDynamicCse(graph, true);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_FALSE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, EliminatesRepeatedModeOneSuperCheck)
{
    auto *graph = CreateGraph();
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheck(1, id, value, 1U);
    auto *current = AddSuperCheck(2, id, value, 1U);
    AddReturn(graph, 3, value);

    RunDynamicCse(graph, true);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_FALSE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, EliminatesEquivalentImm8AndImm16SuperChecks)
{
    auto *graph = CreateGraph();
    auto *value = AddParameter(graph, 0, 0);
    auto *producer =
        AddSuperCheck(1, RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8, value, 0U);
    auto *current =
        AddSuperCheck(2, RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16, value, 0U);
    AddReturn(graph, 3, value);

    RunDynamicCse(graph, true);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_FALSE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, KeepsSuperChecksWithDifferentModes)
{
    auto *graph = CreateGraph();
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *beforeSuper = AddSuperCheck(1, id, value, 1U);
    auto *afterSuper = AddSuperCheck(2, id, value, 0U);
    AddReturn(graph, 3, value);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(beforeSuper->IsNotRemovable());
    EXPECT_TRUE(afterSuper->IsNotRemovable());
}

TEST_F(DynamicCseTest, KeepsSuperChecksWithDifferentSsaInputs)
{
    auto *graph = CreateGraph();
    auto *oldThis = AddParameter(graph, 0, 0);
    auto *newThis = AddParameter(graph, 1, 1);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *beforeSuper = AddSuperCheck(2, id, oldThis, 0U);
    auto *afterSuper = AddSuperCheck(3, id, newThis, 0U);
    AddReturn(graph, 4, newThis);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(beforeSuper->IsNotRemovable());
    EXPECT_TRUE(afterSuper->IsNotRemovable());
}

TEST_F(DynamicCseTest, KeepsSuperChecksAcrossThisInitializationTransition)
{
    auto *graph = CreateGraph();
    auto *oldThis = AddParameter(graph, 0, 0);
    auto *newThis = AddParameter(graph, 1, 1);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *beforeSuper = AddSuperCheck(2, id, oldThis, 1U);
    auto *afterSuper = AddSuperCheck(3, id, newThis, 0U);
    AddReturn(graph, 4, newThis);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(beforeSuper->IsNotRemovable());
    EXPECT_TRUE(afterSuper->IsNotRemovable());
}

TEST_F(DynamicCseTest, KeepsSuperCheckWithUnsupportedImmediate)
{
    auto *graph = CreateGraph();
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheck(1, id, value, 2U);
    auto *current = AddSuperCheck(2, id, value, 2U);
    AddReturn(graph, 3, value);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_TRUE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, DoesNotEliminateOtherConditionalThrowIntrinsic)
{
    auto *graph = CreateGraph();
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_UNDEFINEDIFHOLEWITHNAME_PREF_ID16;
    auto *producer = AddSuperCheck(1, id, value, 0U);
    auto *current = AddSuperCheck(2, id, value, 0U);
    AddReturn(graph, 3, value);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_TRUE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, EliminatesSuperCheckAcrossDominatedNonEhBlocks)
{
    BasicBlock *first = nullptr;
    BasicBlock *second = nullptr;
    auto *graph = CreateLinearGraph(&first, &second);
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheckToBlock(first, 1, id, value, 0U);
    auto *current = AddSuperCheckToBlock(second, 2, id, value, 0U);
    AddReturnToBlock(graph, second, 3, value);

    RunDynamicCse(graph, true);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_FALSE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, KeepsSuperCheckAcrossEhBlocks)
{
    BasicBlock *first = nullptr;
    BasicBlock *second = nullptr;
    auto *graph = CreateLinearGraph(&first, &second);
    first->SetTry(true);
    second->SetTry(true);
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheckToBlock(first, 1, id, value, 0U);
    auto *current = AddSuperCheckToBlock(second, 2, id, value, 0U);
    AddReturnToBlock(graph, second, 3, value);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_TRUE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, EliminatesRepeatedSuperCheckWithinOneEhBlock)
{
    auto *graph = CreateGraph();
    body_->SetTry(true);
    auto *value = AddParameter(graph, 0, 0);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheck(1, id, value, 0U);
    auto *current = AddSuperCheck(2, id, value, 0U);
    AddReturn(graph, 3, value);

    RunDynamicCse(graph, true);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_FALSE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, KeepsSuperCheckAtMergeWhenProducerIsOnOneDiamondBranch)
{
    auto *graph = allocator_->New<Graph>(allocator_, localAllocator_, Arch::NONE, true, true);
    auto *start = graph->CreateStartBlock();
    auto *branch = graph->CreateEmptyBlock();
    auto *leftBlock = graph->CreateEmptyBlock();
    auto *rightBlock = graph->CreateEmptyBlock();
    auto *merge = graph->CreateEmptyBlock();
    auto *end = graph->CreateEndBlock();
    start->AddSucc(branch);
    branch->AddSucc(leftBlock);
    branch->AddSucc(rightBlock);
    leftBlock->AddSucc(merge);
    rightBlock->AddSucc(merge);
    merge->AddSucc(end);

    auto *value = AddParameter(graph, 0, 0);
    auto *condition = AddParameter(graph, 1, 1);
    condition->SetType(DataType::BOOL);
    AddIf(graph, branch, 2, condition);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheckToBlock(leftBlock, 3, id, value, 0U);
    auto *current = AddSuperCheckToBlock(merge, 4, id, value, 0U);
    AddReturnToBlock(graph, merge, 5, value);

    RunDynamicCse(graph, false);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_TRUE(current->IsNotRemovable());
}

TEST_F(DynamicCseTest, EliminatesLoopBodySuperCheckDominatedByLoopHeader)
{
    auto *graph = allocator_->New<Graph>(allocator_, localAllocator_, Arch::NONE, true, true);
    // This hand-built loop has no compiler-inserted safepoint; DynamicCse does not depend on that managed-code check.
    auto mode = graph->GetMode();
    mode.SetNative(true);
    graph->SetMode(mode);
    auto *start = graph->CreateStartBlock();
    auto *header = graph->CreateEmptyBlock();
    auto *body = graph->CreateEmptyBlock();
    auto *exit = graph->CreateEmptyBlock();
    auto *end = graph->CreateEndBlock();
    start->AddSucc(header);
    header->AddSucc(body);
    header->AddSucc(exit);
    body->AddSucc(header);
    exit->AddSucc(end);

    auto *value = AddParameter(graph, 0, 0);
    auto *condition = AddParameter(graph, 1, 1);
    condition->SetType(DataType::BOOL);
    auto id = RuntimeInterface::IntrinsicId::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8;
    auto *producer = AddSuperCheckToBlock(header, 2, id, value, 0U);
    AddIf(graph, header, 3, condition);
    auto *current = AddSuperCheckToBlock(body, 4, id, value, 0U);
    AddReturnToBlock(graph, exit, 5, value);

    RunDynamicCse(graph, true);

    EXPECT_TRUE(producer->IsNotRemovable());
    EXPECT_FALSE(current->IsNotRemovable());
}

}  // namespace panda::bytecodeopt::test
