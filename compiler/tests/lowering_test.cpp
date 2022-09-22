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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"

namespace panda::compiler {
class LoweringTest : public GraphTest {
public:
    LoweringTest()
    {
#ifndef NDEBUG
        graph_->SetLowLevelInstructionsEnabled();
#endif
    }
    template <class T>
    void ReturnTest(T val, DataType::Type type)
    {
        auto graph = CreateGraphStartEndBlocks();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        auto cnst = graph->FindOrCreateConstant(val);
        auto block = graph->CreateEmptyBlock();
        graph->GetStartBlock()->AddSucc(block);
        block->AddSucc(graph->GetEndBlock());
        auto ret = graph->CreateInstReturn();
        ret->SetType(type);
        ret->SetInput(0, cnst);
        block->AppendInst(ret);
        graph->RunPass<LoopAnalyzer>();
        GraphChecker(graph).Check();

        graph->RunPass<Lowering>();
        EXPECT_FALSE(cnst->HasUsers());
        GraphChecker(graph).Check();
#ifndef NDEBUG
        graph->SetRegAllocApplied();
#endif
        EXPECT_TRUE(graph->RunPass<Codegen>());
    }

    Graph *CreateEmptyLowLevelGraph()
    {
        auto graph = CreateEmptyGraph();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif
        return graph;
    }
};

TEST_F(LoweringTest, LoweringAddSub)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(11, 1).f64();
        PARAMETER(12, 2).f32();
        CONSTANT(1, 12);
        CONSTANT(2, -1);
        CONSTANT(3, 100000000);
        CONSTANT(21, 1.2);
        CONSTANT(22, 0.5f);

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
            INST(5, Opcode::Add).u64().Inputs(0, 2);
            INST(6, Opcode::Add).u64().Inputs(0, 3);
            INST(7, Opcode::Sub).u64().Inputs(0, 1);
            INST(8, Opcode::Sub).u64().Inputs(0, 2);
            INST(9, Opcode::Sub).u64().Inputs(0, 3);
            INST(13, Opcode::Add).f64().Inputs(11, 21);
            INST(14, Opcode::Sub).f64().Inputs(11, 21);
            INST(15, Opcode::Add).f32().Inputs(12, 22);
            INST(16, Opcode::Sub).f32().Inputs(12, 22);
            INST(17, Opcode::Add).u64().Inputs(0, 0);
            INST(18, Opcode::Sub).u64().Inputs(0, 0);
            INST(19, Opcode::Add).u16().Inputs(0, 1);
            INST(20, Opcode::Add).u16().Inputs(0, 2);
            INST(10, Opcode::SafePoint)
                .Inputs(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22)
                .SrcVregs({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21});
            INST(23, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<Lowering>();
    ASSERT_FALSE(INS(4).HasUsers());
    ASSERT_FALSE(INS(5).HasUsers());
    ASSERT_TRUE(INS(6).HasUsers());
    ASSERT_FALSE(INS(7).HasUsers());
    ASSERT_FALSE(INS(8).HasUsers());
    ASSERT_TRUE(INS(9).HasUsers());
    ASSERT_TRUE(INS(13).HasUsers());
    ASSERT_TRUE(INS(14).HasUsers());
    ASSERT_TRUE(INS(15).HasUsers());
    ASSERT_TRUE(INS(16).HasUsers());
    ASSERT_TRUE(INS(17).HasUsers());
    ASSERT_TRUE(INS(18).HasUsers());
    ASSERT_TRUE(INS(19).HasUsers());
    ASSERT_TRUE(INS(20).HasUsers());
    ASSERT_EQ(INS(4).GetPrev()->GetOpcode(), Opcode::AddI);
    ASSERT_EQ(INS(5).GetPrev()->GetOpcode(), Opcode::SubI);
    ASSERT_EQ(INS(6).GetPrev()->GetOpcode(), Opcode::Add);
    ASSERT_EQ(INS(7).GetPrev()->GetOpcode(), Opcode::SubI);
    ASSERT_EQ(INS(8).GetPrev()->GetOpcode(), Opcode::AddI);
    ASSERT_EQ(INS(9).GetPrev()->GetOpcode(), Opcode::Sub);
}

TEST_F(LoweringTest, AddSubCornerCase)
{
    auto graph = CreateEmptyBytecodeGraph();
    constexpr int MIN = std::numeric_limits<int8_t>::min();
    ASSERT_TRUE(MIN < 0);
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(2, MIN).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).s32().Inputs(0, 2);
            INST(4, Opcode::Sub).s32().Inputs(0, 2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).b().InputsAutoType(3, 4, 20);
            INST(6, Opcode::Return).b().Inputs(5);
        }
    }
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    graph->RunPass<compiler::Lowering>();
    graph->RunPass<compiler::Cleanup>();
    auto expected = CreateEmptyBytecodeGraph();
    GRAPH(expected)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            // As MIN = -128 and -MIN cannot be encoded with 8-bit, the opcodes will not be reversed.
            INST(7, Opcode::AddI).s32().Inputs(0).Imm(MIN);
            INST(8, Opcode::SubI).s32().Inputs(0).Imm(MIN);
            INST(20, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).b().InputsAutoType(7, 8, 20);
            INST(6, Opcode::Return).b().Inputs(5);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(LoweringTest, LoweringLogic)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 12);
        CONSTANT(2, 50);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Or).u32().Inputs(0, 1);
            INST(4, Opcode::Or).u64().Inputs(0, 1);
            INST(5, Opcode::Or).u64().Inputs(0, 2);
            INST(6, Opcode::And).u32().Inputs(0, 1);
            INST(7, Opcode::And).u64().Inputs(0, 1);
            INST(8, Opcode::And).u64().Inputs(0, 2);
            INST(9, Opcode::Xor).u32().Inputs(0, 1);
            INST(10, Opcode::Xor).u64().Inputs(0, 1);
            INST(11, Opcode::Xor).u64().Inputs(0, 2);
            INST(12, Opcode::Or).u8().Inputs(0, 1);
            INST(13, Opcode::And).u64().Inputs(0, 0);
            INST(14, Opcode::SafePoint)
                .Inputs(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13)
                .SrcVregs({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13});
            INST(23, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<Lowering>();
    if (GetGraph()->GetArch() != Arch::AARCH32) {
        ASSERT_FALSE(INS(3).HasUsers());
        ASSERT_FALSE(INS(4).HasUsers());
        ASSERT_TRUE(INS(5).HasUsers());
        ASSERT_FALSE(INS(6).HasUsers());
        ASSERT_FALSE(INS(7).HasUsers());
        ASSERT_TRUE(INS(8).HasUsers());
        ASSERT_FALSE(INS(9).HasUsers());
        ASSERT_FALSE(INS(10).HasUsers());
        ASSERT_TRUE(INS(11).HasUsers());
        ASSERT_TRUE(INS(12).HasUsers());
        ASSERT_TRUE(INS(13).HasUsers());
        ASSERT_EQ(INS(3).GetPrev()->GetOpcode(), Opcode::OrI);
        ASSERT_EQ(INS(4).GetPrev()->GetOpcode(), Opcode::OrI);
        ASSERT_EQ(INS(5).GetPrev()->GetOpcode(), Opcode::Or);
        ASSERT_EQ(INS(6).GetPrev()->GetOpcode(), Opcode::AndI);
        ASSERT_EQ(INS(7).GetPrev()->GetOpcode(), Opcode::AndI);
        ASSERT_EQ(INS(8).GetPrev()->GetOpcode(), Opcode::And);
        ASSERT_EQ(INS(9).GetPrev()->GetOpcode(), Opcode::XorI);
        ASSERT_EQ(INS(10).GetPrev()->GetOpcode(), Opcode::XorI);
        ASSERT_EQ(INS(11).GetPrev()->GetOpcode(), Opcode::Xor);
        return;
    }
    // Then check graph only for arm32
    ASSERT_FALSE(INS(3).HasUsers());
    ASSERT_FALSE(INS(4).HasUsers());
    ASSERT_FALSE(INS(5).HasUsers());
    ASSERT_FALSE(INS(6).HasUsers());
    ASSERT_FALSE(INS(7).HasUsers());
    ASSERT_FALSE(INS(8).HasUsers());
    ASSERT_FALSE(INS(9).HasUsers());
    ASSERT_FALSE(INS(10).HasUsers());
    ASSERT_FALSE(INS(11).HasUsers());
    ASSERT_TRUE(INS(12).HasUsers());
    ASSERT_TRUE(INS(13).HasUsers());
    ASSERT_EQ(INS(3).GetPrev()->GetOpcode(), Opcode::OrI);
    ASSERT_EQ(INS(4).GetPrev()->GetOpcode(), Opcode::OrI);
    ASSERT_EQ(INS(5).GetPrev()->GetOpcode(), Opcode::OrI);
    ASSERT_EQ(INS(6).GetPrev()->GetOpcode(), Opcode::AndI);
    ASSERT_EQ(INS(7).GetPrev()->GetOpcode(), Opcode::AndI);
    ASSERT_EQ(INS(8).GetPrev()->GetOpcode(), Opcode::AndI);
    ASSERT_EQ(INS(9).GetPrev()->GetOpcode(), Opcode::XorI);
    ASSERT_EQ(INS(10).GetPrev()->GetOpcode(), Opcode::XorI);
    ASSERT_EQ(INS(11).GetPrev()->GetOpcode(), Opcode::XorI);
}

TEST_F(LoweringTest, LoweringShift)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 12);
        CONSTANT(2, 64);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shr).u32().Inputs(0, 1);
            INST(4, Opcode::Shr).u64().Inputs(0, 1);
            INST(5, Opcode::Shr).u64().Inputs(0, 2);
            INST(6, Opcode::AShr).u32().Inputs(0, 1);
            INST(7, Opcode::AShr).u64().Inputs(0, 1);
            INST(8, Opcode::AShr).u64().Inputs(0, 2);
            INST(9, Opcode::Shl).u32().Inputs(0, 1);
            INST(10, Opcode::Shl).u64().Inputs(0, 1);
            INST(11, Opcode::Shl).u64().Inputs(0, 2);
            INST(12, Opcode::Shl).u8().Inputs(0, 1);
            INST(13, Opcode::Shr).u64().Inputs(0, 0);
            INST(14, Opcode::SafePoint)
                .Inputs(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13)
                .SrcVregs({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13});
            INST(23, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<Lowering>();
    ASSERT_FALSE(INS(3).HasUsers());
    ASSERT_FALSE(INS(4).HasUsers());
    ASSERT_TRUE(INS(5).HasUsers());
    ASSERT_FALSE(INS(6).HasUsers());
    ASSERT_FALSE(INS(7).HasUsers());
    ASSERT_TRUE(INS(8).HasUsers());
    ASSERT_FALSE(INS(9).HasUsers());
    ASSERT_FALSE(INS(10).HasUsers());
    ASSERT_TRUE(INS(11).HasUsers());
    ASSERT_TRUE(INS(12).HasUsers());
    ASSERT_TRUE(INS(13).HasUsers());
    ASSERT_EQ(INS(3).GetPrev()->GetOpcode(), Opcode::ShrI);
    ASSERT_EQ(INS(4).GetPrev()->GetOpcode(), Opcode::ShrI);
    ASSERT_EQ(INS(5).GetPrev()->GetOpcode(), Opcode::Shr);
    ASSERT_EQ(INS(6).GetPrev()->GetOpcode(), Opcode::AShrI);
    ASSERT_EQ(INS(7).GetPrev()->GetOpcode(), Opcode::AShrI);
    ASSERT_EQ(INS(8).GetPrev()->GetOpcode(), Opcode::AShr);
    ASSERT_EQ(INS(9).GetPrev()->GetOpcode(), Opcode::ShlI);
    ASSERT_EQ(INS(10).GetPrev()->GetOpcode(), Opcode::ShlI);
    ASSERT_EQ(INS(11).GetPrev()->GetOpcode(), Opcode::Shl);
}

TEST_F(LoweringTest, SaveStateTest)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP() << "The optimization for float isn't supported on Aarch32";
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);
        CONSTANT(3, 50);
        CONSTANT(4, 0.5);
        PARAMETER(5, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2, 3, 4, 5).SrcVregs({10, 11, 12, 13, 14, 15});
            INST(11, Opcode::SafePoint).Inputs(0, 1, 2, 3, 4, 5).SrcVregs({10, 11, 12, 13, 14, 15});
            INST(9, Opcode::CallStatic).u64().InputsAutoType(0, 1, 8);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    GetGraph()->RunPass<Lowering>();
    GraphChecker(GetGraph()).Check();
    EXPECT_TRUE(INS(0).HasUsers());
    EXPECT_TRUE(INS(1).HasUsers());
    EXPECT_FALSE(INS(2).HasUsers());
    EXPECT_FALSE(INS(3).HasUsers());
    EXPECT_FALSE(INS(4).HasUsers());
    EXPECT_TRUE(INS(5).HasUsers());

    auto save_state = INS(8).CastToSaveState();
    auto safe_point = INS(11).CastToSafePoint();

    EXPECT_EQ(save_state->GetInputsCount(), 2U);
    EXPECT_EQ(save_state->GetImmediatesCount(), 4U);
    EXPECT_EQ((*save_state->GetImmediates())[0].value, (bit_cast<uint64_t, uint64_t>(0)));
    EXPECT_EQ((*save_state->GetImmediates())[1].value, (bit_cast<uint64_t, uint64_t>(1)));
    EXPECT_EQ((*save_state->GetImmediates())[2].value, (bit_cast<uint64_t, double>(0.5)));
    EXPECT_EQ((*save_state->GetImmediates())[3].value, (bit_cast<uint64_t, uint64_t>(50)));
    EXPECT_EQ(save_state->GetInput(0).GetInst(), &INS(0));
    EXPECT_EQ(save_state->GetInput(1).GetInst(), &INS(5));
    EXPECT_EQ(save_state->GetVirtualRegister(0).Value(), 10);
    EXPECT_EQ(save_state->GetVirtualRegister(1).Value(), 15);

    EXPECT_EQ(safe_point->GetInputsCount(), 2U);
    EXPECT_EQ(safe_point->GetImmediatesCount(), 4U);
    EXPECT_EQ(safe_point->GetInput(0).GetInst(), &INS(0));
    EXPECT_EQ(safe_point->GetInput(1).GetInst(), &INS(5));
    EXPECT_EQ(safe_point->GetVirtualRegister(0).Value(), 10);
    EXPECT_EQ(safe_point->GetVirtualRegister(1).Value(), 15);

    GetGraph()->RunPass<LoopAnalyzer>();
    RegAlloc(GetGraph());
    SetNumVirtRegs(GetGraph()->GetVRegsCount());
    EXPECT_TRUE(GetGraph()->RunPass<Codegen>());
}

TEST_F(LoweringTest, BoundCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        CONSTANT(1, 10);        // index
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).u64().Inputs(3, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 6);
            INST(8, Opcode::StoreArray).u64().Inputs(3, 5, 7);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(7, 7);
            INST(13, Opcode::SaveState).Inputs(0, 10).SrcVregs({0, 1});
            INST(11, Opcode::CallStatic).u64().InputsAutoType(0, 10, 13);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }

    GetGraph()->RunPass<Lowering>();
    EXPECT_TRUE(INS(0).HasUsers());
    EXPECT_FALSE(INS(1).HasUsers());
    EXPECT_TRUE(INS(2).HasUsers());
    EXPECT_TRUE(INS(3).HasUsers());
    EXPECT_TRUE(INS(4).HasUsers());
    EXPECT_FALSE(INS(5).HasUsers());
    EXPECT_FALSE(INS(6).HasUsers());
    EXPECT_TRUE(INS(7).HasUsers());
    EXPECT_FALSE(INS(8).HasUsers());
    GraphChecker(GetGraph()).Check();
    // Run codegen
    GetGraph()->RunPass<LoopAnalyzer>();
    RegAlloc(GetGraph());
    SetNumVirtRegs(GetGraph()->GetVRegsCount());
    EXPECT_TRUE(GetGraph()->RunPass<Codegen>());
}

TEST_F(LoweringTest, LoadStoreArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        CONSTANT(1, 10);        // index
        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(4, Opcode::Add).u64().Inputs(3, 3);
            INST(5, Opcode::StoreArray).u64().Inputs(0, 1, 4);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(9, Opcode::SaveState).Inputs(0, 4).SrcVregs({0, 1});
            INST(7, Opcode::CallStatic).u64().InputsAutoType(0, 4, 9);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    GetGraph()->RunPass<Lowering>();
    EXPECT_TRUE(INS(0).HasUsers());
    EXPECT_FALSE(INS(1).HasUsers());
    EXPECT_FALSE(INS(2).HasUsers());
    EXPECT_FALSE(INS(3).HasUsers());
    EXPECT_TRUE(INS(4).HasUsers());
    EXPECT_FALSE(INS(5).HasUsers());
    GraphChecker(GetGraph()).Check();
    // Run codegen
    GetGraph()->RunPass<LoopAnalyzer>();
    RegAlloc(GetGraph());
    SetNumVirtRegs(GetGraph()->GetVRegsCount());
    EXPECT_TRUE(GetGraph()->RunPass<Codegen>());
}

TEST_F(LoweringTest, Return)
{
    ReturnTest<int64_t>(10, DataType::INT64);
    ReturnTest<float>(10.0F, DataType::FLOAT32);
    ReturnTest<double>(10.0, DataType::FLOAT64);
    ReturnTest<int64_t>(0, DataType::INT64);
    ReturnTest<float>(0.0F, DataType::FLOAT32);
    ReturnTest<double>(0.0, DataType::FLOAT64);
}

TEST_F(LoweringTest, If)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(6, Opcode::Return).b().Inputs(3);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(7, Opcode::Return).b().Inputs(2);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_if = CreateEmptyGraph();
        GRAPH(graph_if)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(3, Opcode::ReturnI).b().Imm(1);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnI).b().Imm(0);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graph_if));
    }
}

TEST_F(LoweringTest, If1)
{
    // Applied
    // Compare have several IfImm users.
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            CONSTANT(10, 2);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(3, 5, 6)
            {
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(7, Opcode::Return).s32().Inputs(2);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(8, Opcode::Return).s32().Inputs(3);
            }
            BASIC_BLOCK(6, -1)
            {
                INST(9, Opcode::Return).s32().Inputs(10);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_if = CreateEmptyGraph();
        GRAPH(graph_if)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0, 1);
            }
            BASIC_BLOCK(3, 5, 6)
            {
                INST(3, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0, 1);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnI).s32().Imm(0);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(5, Opcode::ReturnI).s32().Imm(1);
            }
            BASIC_BLOCK(6, -1)
            {
                INST(6, Opcode::ReturnI).s32().Imm(2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graph_if));
    }
}

TEST_F(LoweringTest, If2)
{
    // Not applied
    // Compare have several users, not only IfImm.
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);
            CONSTANT(10, 2);
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(3, 5, 6)
            {
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(20, Opcode::SaveState).NoVregs();
                INST(11, Opcode::CallStatic).b().InputsAutoType(4, 20);
                INST(7, Opcode::Return).s32().Inputs(2);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(8, Opcode::Return).s32().Inputs(3);
            }
            BASIC_BLOCK(6, -1)
            {
                INST(9, Opcode::Return).s32().Inputs(10);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_if = CreateEmptyGraph();
        GRAPH(graph_if)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(3, 5, 6)
            {
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(20, Opcode::SaveState).NoVregs();
                INST(11, Opcode::CallStatic).b().InputsAutoType(4, 20);
                INST(7, Opcode::ReturnI).s32().Imm(0);
            }
            BASIC_BLOCK(5, -1)
            {
                INST(8, Opcode::ReturnI).s32().Imm(1);
            }
            BASIC_BLOCK(6, -1)
            {
                INST(9, Opcode::ReturnI).s32().Imm(2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graph_if));
    }
}

TEST_F(LoweringTest, If_Fcmpl)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0, 1);
                INST(5, Opcode::Compare).b().CC(cc).Inputs(4, 2);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(7, Opcode::Return).b().Inputs(3);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(8, Opcode::Return).b().Inputs(2);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_if = CreateEmptyGraph();
        GRAPH(graph_if)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::If).SrcType(DataType::FLOAT64).CC(cc).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(3, Opcode::ReturnI).b().Imm(1);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnI).b().Imm(0);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graph_if));
    }
}

TEST_F(LoweringTest, If_Fcmpl_NoJoin)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0, 1);
                INST(5, Opcode::Compare).b().CC(cc).Inputs(4, 3);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(7, Opcode::Return).b().Inputs(3);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(8, Opcode::Return).b().Inputs(2);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_if = CreateEmptyGraph();
        GRAPH(graph_if)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0, 1);
                INST(5, Opcode::IfImm).SrcType(DataType::INT32).CC(cc).Imm(1).Inputs(4);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(7, Opcode::ReturnI).b().Imm(1);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(8, Opcode::ReturnI).b().Imm(0);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graph_if));
    }
}

TEST_F(LoweringTest, If_Fcmpl_NoJoin2)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0, 1);
                INST(10, Opcode::Add).s32().Inputs(4, 3);
                INST(5, Opcode::Compare).b().CC(cc).Inputs(4, 2);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(7, Opcode::Return).b().Inputs(3);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(8, Opcode::Return).b().Inputs(10);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_if = CreateEmptyGraph();
        GRAPH(graph_if)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0, 1);
                INST(10, Opcode::AddI).s32().Imm(1).Inputs(4);
                INST(5, Opcode::IfImm).SrcType(DataType::INT32).CC(cc).Imm(0).Inputs(4);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(7, Opcode::ReturnI).b().Imm(1);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(8, Opcode::Return).b().Inputs(10);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graph_if));
    }
}

TEST_F(LoweringTest, If_Fcmpg)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        ConditionCode cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();
            CONSTANT(2, 0);
            CONSTANT(3, 1);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(4, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(true).Inputs(0, 1);
                INST(5, Opcode::Compare).b().CC(cc).Inputs(4, 2);
                INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(7, Opcode::Return).b().Inputs(3);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(8, Opcode::Return).b().Inputs(2);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_if = CreateEmptyGraph();
        GRAPH(graph_if)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::If).SrcType(DataType::FLOAT64).CC(InverseSignednessConditionCode(cc)).Inputs(0, 1);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(3, Opcode::ReturnI).b().Imm(1);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnI).b().Imm(0);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graph_if));
    }
}

TEST_F(LoweringTest, MultiplyAddInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }

    std::initializer_list<std::array<DataType::Type, 4>> type_combinations = {
        {DataType::UINT32, DataType::UINT32, DataType::UINT32, DataType::UINT32},
        {DataType::INT32, DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::INT32, DataType::INT16, DataType::INT8, DataType::INT16},
        {DataType::UINT64, DataType::UINT64, DataType::UINT64, DataType::UINT64},
        {DataType::INT64, DataType::INT64, DataType::INT64, DataType::INT64}};

    for (auto &types : type_combinations) {
        auto type = types[0];
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).type(types[1]);
            PARAMETER(1, 1).type(types[2]);
            PARAMETER(2, 2).type(types[3]);

            BASIC_BLOCK(2, -1)
            {
                // a * b + c
                INST(3, Opcode::Mul).type(type).Inputs(0, 1);
                INST(4, Opcode::Add).type(type).Inputs(3, 2);

                // c + a * b
                INST(5, Opcode::Mul).type(type).Inputs(1, 2);
                INST(6, Opcode::Add).type(type).Inputs(0, 5);

                // a * b + c, but a * b is reused
                INST(7, Opcode::Mul).type(type).Inputs(0, 1);
                INST(8, Opcode::Add).type(type).Inputs(7, 2);

                INST(9, Opcode::Add).type(type).Inputs(4, 6);
                INST(10, Opcode::Add).type(type).Inputs(9, 8);
                INST(11, Opcode::Add).type(type).Inputs(10, 7);
                INST(12, Opcode::Return).type(type).Inputs(11);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_madd = CreateEmptyGraph();
        GRAPH(graph_madd)
        {
            PARAMETER(0, 0).type(types[1]);
            PARAMETER(1, 1).type(types[2]);
            PARAMETER(2, 2).type(types[3]);

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MAdd).type(type).Inputs(0, 1, 2);
                INST(4, Opcode::MAdd).type(type).Inputs(1, 2, 0);

                INST(5, Opcode::Mul).type(type).Inputs(0, 1);
                INST(6, Opcode::Add).type(type).Inputs(5, 2);

                INST(7, Opcode::Add).type(type).Inputs(3, 4);
                INST(8, Opcode::Add).type(type).Inputs(7, 6);
                INST(9, Opcode::Add).type(type).Inputs(8, 5);
                INST(10, Opcode::Return).type(type).Inputs(9);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graph_madd));
    }
}

TEST_F(LoweringTest, MultiplyAddWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u16();
        PARAMETER(2, 2).u16();

        BASIC_BLOCK(2, -1)
        {
            // a + b * c
            INST(3, Opcode::Mul).u16().Inputs(1, 2);
            INST(4, Opcode::Add).u32().Inputs(3, 0);
            INST(5, Opcode::Return).u32().Inputs(4);
        }
    }
    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoweringTest, MultiplySubInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    std::initializer_list<std::array<DataType::Type, 4>> type_combinations = {
        {DataType::INT32, DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::INT32, DataType::INT16, DataType::INT8, DataType::INT16},
        {DataType::INT64, DataType::INT64, DataType::INT64, DataType::INT64}};

    for (auto &types : type_combinations) {
        auto type = types[0];
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).type(types[1]);
            PARAMETER(1, 1).type(types[2]);
            PARAMETER(2, 2).type(types[3]);

            BASIC_BLOCK(2, -1)
            {
                // c - a * b
                INST(3, Opcode::Mul).type(type).Inputs(0, 1);
                INST(4, Opcode::Sub).type(type).Inputs(2, 3);

                // (- a * b) + c
                INST(5, Opcode::Mul).type(type).Inputs(0, 1);
                INST(6, Opcode::Neg).type(type).Inputs(5);
                INST(7, Opcode::Add).type(type).Inputs(6, 2);

                // c + (-a) * b
                INST(8, Opcode::Neg).type(type).Inputs(0);
                INST(9, Opcode::Mul).type(type).Inputs(8, 1);
                INST(10, Opcode::Add).type(type).Inputs(9, 2);

                // c + a * (-b)
                INST(11, Opcode::Neg).type(type).Inputs(1);
                INST(12, Opcode::Mul).type(type).Inputs(0, 11);
                INST(13, Opcode::Add).type(type).Inputs(12, 2);

                // c - a * b, but a * b is reused
                INST(14, Opcode::Mul).type(type).Inputs(0, 1);
                INST(15, Opcode::Sub).type(type).Inputs(2, 14);

                INST(16, Opcode::Add).type(type).Inputs(4, 7);
                INST(17, Opcode::Add).type(type).Inputs(16, 10);
                INST(18, Opcode::Add).type(type).Inputs(17, 13);
                INST(19, Opcode::Add).type(type).Inputs(18, 15);
                INST(20, Opcode::Add).type(type).Inputs(19, 14);
                INST(21, Opcode::Return).type(type).Inputs(20);
            }
        }
        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_msub = CreateEmptyGraph();
        GRAPH(graph_msub)
        {
            PARAMETER(0, 0).type(types[1]);
            PARAMETER(1, 1).type(types[2]);
            PARAMETER(2, 2).type(types[3]);

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MSub).type(type).Inputs(0, 1, 2);
                INST(4, Opcode::MSub).type(type).Inputs(0, 1, 2);
                INST(5, Opcode::MSub).type(type).Inputs(0, 1, 2);
                // add(mul(0, neg(1)), 2) -> add(mneg(1, 0), 2)
                INST(6, Opcode::MSub).type(type).Inputs(1, 0, 2);

                INST(7, Opcode::Mul).type(type).Inputs(0, 1);
                INST(8, Opcode::Sub).type(type).Inputs(2, 7);

                INST(9, Opcode::Add).type(type).Inputs(3, 4);
                INST(10, Opcode::Add).type(type).Inputs(9, 5);
                INST(11, Opcode::Add).type(type).Inputs(10, 6);
                INST(12, Opcode::Add).type(type).Inputs(11, 8);
                INST(13, Opcode::Add).type(type).Inputs(12, 7);
                INST(14, Opcode::Return).type(type).Inputs(13);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graph_msub));
    }
}

TEST_F(LoweringTest, MultiplySubIntegerWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u8();
        PARAMETER(1, 1).u8();
        PARAMETER(2, 2).u32();

        BASIC_BLOCK(2, -1)
        {
            // c - a * b
            INST(3, Opcode::Mul).u16().Inputs(0, 1);
            INST(4, Opcode::Sub).u32().Inputs(2, 3);

            // (- a * b) + c
            INST(5, Opcode::Mul).u16().Inputs(0, 1);
            INST(6, Opcode::Neg).u32().Inputs(5);
            INST(7, Opcode::Add).u32().Inputs(6, 2);

            // c + (-a) * b
            INST(8, Opcode::Neg).u8().Inputs(0);
            INST(9, Opcode::Mul).u16().Inputs(8, 1);
            INST(10, Opcode::Add).u32().Inputs(9, 2);

            // c + a * (-b)
            INST(11, Opcode::Neg).u8().Inputs(1);
            INST(12, Opcode::Mul).u16().Inputs(0, 11);
            INST(13, Opcode::Add).u32().Inputs(12, 2);

            // c - a * b, but a * b is reused
            INST(14, Opcode::Mul).u16().Inputs(0, 1);
            INST(15, Opcode::Sub).u32().Inputs(2, 14);

            INST(16, Opcode::Add).u32().Inputs(4, 7);
            INST(17, Opcode::Add).u32().Inputs(16, 10);
            INST(18, Opcode::Add).u32().Inputs(17, 13);
            INST(19, Opcode::Add).u32().Inputs(18, 15);
            INST(20, Opcode::Add).u32().Inputs(19, 14);
            INST(21, Opcode::Return).u32().Inputs(20);
        }
    }

    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoweringTest, MultiplyAddSubFloat)
{
    std::array<Graph *, 2> graphs {CreateEmptyLowLevelGraph(), CreateEmptyLowLevelGraph()};
    for (auto &graph : graphs) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).f64();
            PARAMETER(1, 1).f64();
            PARAMETER(2, 2).f64();

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::Mul).f64().Inputs(0, 1);
                INST(4, Opcode::Add).f64().Inputs(3, 2);

                INST(5, Opcode::Mul).f64().Inputs(0, 1);
                INST(6, Opcode::Sub).f64().Inputs(2, 5);

                INST(7, Opcode::Add).f64().Inputs(4, 6);
                INST(8, Opcode::Return).f64().Inputs(7);
            }
        }
    }

    EXPECT_TRUE(graphs[0]->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graphs[0], graphs[1]));
}

TEST_F(LoweringTest, MultiplyNegate)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    std::initializer_list<std::array<DataType::Type, 3>> type_combinations = {
        {DataType::FLOAT32, DataType::FLOAT32, DataType::FLOAT32},
        {DataType::FLOAT64, DataType::FLOAT64, DataType::FLOAT64},
        {DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::INT32, DataType::INT16, DataType::INT8},
        {DataType::INT64, DataType::INT64, DataType::INT64}};

    for (auto &types : type_combinations) {
        auto type = types[0];
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).type(types[1]);
            PARAMETER(1, 1).type(types[2]);

            BASIC_BLOCK(2, -1)
            {
                // - (a * b)
                INST(3, Opcode::Mul).type(type).Inputs(0, 1);
                INST(4, Opcode::Neg).type(type).Inputs(3);

                // a * (-b)
                INST(5, Opcode::Neg).type(type).Inputs(1);
                INST(6, Opcode::Mul).type(type).Inputs(0, 5);

                // (-a) * b
                INST(7, Opcode::Neg).type(type).Inputs(0);
                INST(8, Opcode::Mul).type(type).Inputs(7, 1);

                // - (a * b), but mul result is reused
                INST(9, Opcode::Mul).type(type).Inputs(0, 1);
                INST(10, Opcode::Neg).type(type).Inputs(9);

                // (-a) * b, but neg result is reused
                INST(11, Opcode::Neg).type(type).Inputs(0);
                INST(12, Opcode::Mul).type(type).Inputs(11, 1);

                INST(13, Opcode::Max).type(type).Inputs(4, 6);
                INST(14, Opcode::Max).type(type).Inputs(13, 8);
                INST(15, Opcode::Max).type(type).Inputs(14, 10);
                INST(16, Opcode::Max).type(type).Inputs(15, 12);
                INST(17, Opcode::Max).type(type).Inputs(16, 9);
                INST(18, Opcode::Max).type(type).Inputs(17, 11);
                INST(19, Opcode::Return).type(type).Inputs(18);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graph_expected = CreateEmptyGraph();
        GRAPH(graph_expected)
        {
            PARAMETER(0, 0).type(types[1]);
            PARAMETER(1, 1).type(types[2]);

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::MNeg).type(type).Inputs(0, 1);
                INST(4, Opcode::MNeg).type(type).Inputs(1, 0);
                INST(5, Opcode::MNeg).type(type).Inputs(0, 1);
                INST(6, Opcode::Mul).type(type).Inputs(0, 1);
                INST(7, Opcode::Neg).type(type).Inputs(6);
                INST(8, Opcode::Neg).type(type).Inputs(0);
                INST(9, Opcode::Mul).type(type).Inputs(8, 1);

                INST(10, Opcode::Max).type(type).Inputs(3, 4);
                INST(11, Opcode::Max).type(type).Inputs(10, 5);
                INST(12, Opcode::Max).type(type).Inputs(11, 7);
                INST(13, Opcode::Max).type(type).Inputs(12, 9);
                INST(14, Opcode::Max).type(type).Inputs(13, 6);
                INST(15, Opcode::Max).type(type).Inputs(14, 8);
                INST(16, Opcode::Return).type(type).Inputs(15);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
    }
}

TEST_F(LoweringTest, MultiplyNegateWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s16();
        PARAMETER(1, 1).s16();

        BASIC_BLOCK(2, -1)
        {
            // - (a * b)
            INST(3, Opcode::Mul).s16().Inputs(0, 1);
            INST(4, Opcode::Neg).s32().Inputs(3);

            // a * (-b)
            INST(5, Opcode::Neg).s16().Inputs(1);
            INST(6, Opcode::Mul).s32().Inputs(0, 5);

            // (-a) * b
            INST(7, Opcode::Neg).s16().Inputs(0);
            INST(8, Opcode::Mul).s32().Inputs(7, 1);

            // - (a * b), but mul result is reused
            INST(9, Opcode::Mul).s16().Inputs(0, 1);
            INST(10, Opcode::Neg).s32().Inputs(9);

            // (-a) * b, but neg result is reused
            INST(11, Opcode::Neg).s16().Inputs(0);
            INST(12, Opcode::Mul).s32().Inputs(11, 1);

            INST(13, Opcode::Max).s32().Inputs(4, 6);
            INST(14, Opcode::Max).s32().Inputs(13, 8);
            INST(15, Opcode::Max).s32().Inputs(14, 10);
            INST(16, Opcode::Max).s32().Inputs(15, 12);
            INST(17, Opcode::Max).s32().Inputs(16, 9);
            INST(18, Opcode::Max).s32().Inputs(17, 11);
            INST(19, Opcode::Return).s32().Inputs(18);
        }
    }

    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoweringTest, BitwiseBinaryOpWithInvertedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "xor-not, or-not and and-not instructions are only supported on Aarch64";
    }

    std::initializer_list<std::pair<Opcode, Opcode>> opcodes = {
        {Opcode::And, Opcode::AndNot}, {Opcode::Or, Opcode::OrNot}, {Opcode::Xor, Opcode::XorNot}};
    std::initializer_list<std::array<DataType::Type, 3>> type_combinations = {
        {DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::UINT32, DataType::UINT32, DataType::UINT32},
        {DataType::UINT32, DataType::UINT16, DataType::UINT8},
        {DataType::INT64, DataType::INT64, DataType::INT64},
        {DataType::UINT64, DataType::UINT64, DataType::UINT64}};
    for (auto &types : type_combinations) {
        for (auto &ops : opcodes) {
            auto type = types[0];
            auto graph = CreateEmptyLowLevelGraph();
            GRAPH(graph)
            {
                PARAMETER(0, 0).type(types[1]);
                PARAMETER(1, 1).type(types[2]);

                BASIC_BLOCK(2, -1)
                {
                    // ~a op b
                    INST(3, Opcode::Not).type(type).Inputs(0);
                    INST(4, ops.first).type(type).Inputs(3, 1);
                    // a op ~b
                    INST(5, Opcode::Not).type(type).Inputs(1);
                    INST(6, ops.first).type(type).Inputs(0, 5);
                    // ~a op b, but ~a is reused
                    INST(7, Opcode::Not).type(type).Inputs(0);
                    INST(8, ops.first).type(type).Inputs(7, 1);

                    INST(9, Opcode::Add).type(type).Inputs(4, 6);
                    INST(10, Opcode::Add).type(type).Inputs(9, 8);
                    INST(11, Opcode::Add).type(type).Inputs(10, 7);
                    INST(12, Opcode::Return).type(type).Inputs(11);
                }
            }

            EXPECT_TRUE(graph->RunPass<Lowering>());
            EXPECT_TRUE(graph->RunPass<Cleanup>());

            auto graph_expected = CreateEmptyGraph();
            GRAPH(graph_expected)
            {
                PARAMETER(0, 0).type(types[1]);
                PARAMETER(1, 1).type(types[2]);

                BASIC_BLOCK(2, -1)
                {
                    INST(3, ops.second).type(type).Inputs(1, 0);
                    INST(4, ops.second).type(type).Inputs(0, 1);
                    INST(5, Opcode::Not).type(type).Inputs(0);
                    INST(6, ops.first).type(type).Inputs(5, 1);

                    INST(7, Opcode::Add).type(type).Inputs(3, 4);
                    INST(8, Opcode::Add).type(type).Inputs(7, 6);
                    INST(9, Opcode::Add).type(type).Inputs(8, 5);
                    INST(10, Opcode::Return).type(type).Inputs(9);
                }
            }

            ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
        }
    }
}

TEST_F(LoweringTest, BitwiseBinaryOpWithInvertedOperandWitnIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "xor-not, or-not and and-not instructions are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::And, Opcode::Or, Opcode::Xor};
    for (auto &ops : opcodes) {
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s16();
            PARAMETER(1, 1).s16();

            BASIC_BLOCK(2, -1)
            {
                // ~a op b
                INST(3, Opcode::Not).s16().Inputs(0);
                INST(4, ops).s32().Inputs(3, 1);
                // a op ~b
                INST(5, Opcode::Not).s16().Inputs(1);
                INST(6, ops).s32().Inputs(0, 5);
                // ~a op b, but ~a is reused
                INST(7, Opcode::Not).s16().Inputs(0);
                INST(8, ops).s32().Inputs(7, 1);

                INST(9, Opcode::Add).s32().Inputs(4, 6);
                INST(10, Opcode::Add).s32().Inputs(9, 8);
                INST(11, Opcode::Add).s32().Inputs(10, 7);
                INST(12, Opcode::Return).s32().Inputs(11);
            }
        }

        auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_TRUE(GraphComparator().Compare(graph, clone));
    }
}

TEST_F(LoweringTest, CommutativeBinaryOpWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<std::pair<Opcode, Opcode>> opcodes = {{Opcode::Add, Opcode::AddSR},
                                                                {Opcode::And, Opcode::AndSR},
                                                                {Opcode::Or, Opcode::OrSR},
                                                                {Opcode::Xor, Opcode::XorSR}};

    std::initializer_list<std::tuple<Opcode, Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
        {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
        {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    std::initializer_list<std::array<DataType::Type, 3>> type_combinations = {
        {DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::UINT32, DataType::UINT32, DataType::UINT32},
        {DataType::UINT32, DataType::UINT16, DataType::UINT8},
        {DataType::INT64, DataType::INT64, DataType::INT64},
        {DataType::UINT64, DataType::UINT64, DataType::UINT64}};

    for (auto &types : type_combinations) {
        for (auto &shift_op : shift_ops) {
            for (auto &ops : opcodes) {
                auto type = types[0];
                auto graph = CreateEmptyLowLevelGraph();
                GRAPH(graph)
                {
                    PARAMETER(0, 0).type(types[1]);
                    PARAMETER(1, 1).type(types[2]);
                    CONSTANT(2, 5);
                    CONSTANT(3, 3);

                    BASIC_BLOCK(2, -1)
                    {
                        INST(4, std::get<0>(shift_op)).type(type).Inputs(0, 2);
                        INST(5, ops.first).type(type).Inputs(1, 4);

                        INST(6, std::get<0>(shift_op)).type(type).Inputs(1, 2);
                        INST(7, ops.first).type(type).Inputs(6, 1);

                        INST(8, std::get<0>(shift_op)).type(type).Inputs(1, 2);
                        INST(9, ops.first).type(type).Inputs(8, 3);

                        INST(10, std::get<0>(shift_op)).type(type).Inputs(0, 2);
                        INST(11, ops.first).type(type).Inputs(1, 10);

                        INST(12, Opcode::Max).type(type).Inputs(5, 7);
                        INST(13, Opcode::Max).type(type).Inputs(12, 9);
                        INST(14, Opcode::Max).type(type).Inputs(13, 11);
                        INST(15, Opcode::Max).type(type).Inputs(14, 10);
                        INST(16, Opcode::Return).type(type).Inputs(15);
                    }
                }

                EXPECT_TRUE(graph->RunPass<Lowering>());
                EXPECT_TRUE(graph->RunPass<Cleanup>());

                auto graph_expected = CreateEmptyGraph();
                GRAPH(graph_expected)
                {
                    PARAMETER(0, 0).type(types[1]);
                    PARAMETER(1, 1).type(types[2]);
                    CONSTANT(2, 3);

                    BASIC_BLOCK(2, -1)
                    {
                        INST(3, ops.second).Shift(std::get<2>(shift_op), 5).type(type).Inputs(1, 0);
                        INST(4, ops.second).Shift(std::get<2>(shift_op), 5).type(type).Inputs(1, 1);
                        INST(5, ops.second).Shift(std::get<2>(shift_op), 5).type(type).Inputs(2, 1);

                        INST(6, std::get<1>(shift_op)).Imm(5).type(type).Inputs(0);
                        INST(7, ops.first).type(type).Inputs(1, 6);

                        INST(8, Opcode::Max).type(type).Inputs(3, 4);
                        INST(9, Opcode::Max).type(type).Inputs(8, 5);
                        INST(10, Opcode::Max).type(type).Inputs(9, 7);
                        INST(11, Opcode::Max).type(type).Inputs(10, 6);
                        INST(12, Opcode::Return).type(type).Inputs(11);
                    }
                }

                ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
            }
        }
    }
}

TEST_F(LoweringTest, CommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::Add, Opcode::And, Opcode::Or, Opcode::Xor};

    std::initializer_list<std::tuple<Opcode, Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
        {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
        {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    for (auto &shift_op : shift_ops) {
        for (auto &ops : opcodes) {
            auto graph = CreateEmptyLowLevelGraph();
            GRAPH(graph)
            {
                PARAMETER(0, 0).s16();
                PARAMETER(1, 1).s16();
                CONSTANT(2, 5);
                PARAMETER(3, 2).s16();

                BASIC_BLOCK(2, -1)
                {
                    INST(4, std::get<0>(shift_op)).s16().Inputs(0, 2);
                    INST(5, ops).s32().Inputs(1, 4);

                    INST(6, std::get<0>(shift_op)).s16().Inputs(1, 2);
                    INST(7, ops).s32().Inputs(6, 1);

                    INST(8, std::get<0>(shift_op)).s16().Inputs(1, 2);
                    INST(9, ops).s32().Inputs(8, 3);

                    INST(10, std::get<0>(shift_op)).s16().Inputs(0, 2);
                    INST(11, ops).s32().Inputs(1, 10);

                    INST(12, Opcode::Max).s32().Inputs(5, 7);
                    INST(13, Opcode::Max).s32().Inputs(12, 9);
                    INST(14, Opcode::Max).s32().Inputs(13, 11);
                    INST(15, Opcode::Max).s32().Inputs(14, 10);
                    INST(16, Opcode::Return).s32().Inputs(15);
                }
            }

            auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
            EXPECT_TRUE(graph->RunPass<Lowering>());
            ASSERT_TRUE(GraphComparator().Compare(graph, clone));
        }
    }
}

TEST_F(LoweringTest, SubWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<std::tuple<Opcode, Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
        {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
        {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    std::initializer_list<std::array<DataType::Type, 3>> type_combinations = {
        {DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::UINT32, DataType::UINT32, DataType::UINT32},
        {DataType::UINT32, DataType::UINT16, DataType::UINT8},
        {DataType::INT64, DataType::INT64, DataType::INT64},
        {DataType::UINT64, DataType::UINT64, DataType::UINT64}};

    for (auto &types : type_combinations) {
        for (auto &shift_op : shift_ops) {
            auto type = types[0];
            auto graph = CreateEmptyLowLevelGraph();
            GRAPH(graph)
            {
                PARAMETER(0, 0).type(types[1]);
                PARAMETER(1, 1).type(types[2]);
                CONSTANT(2, 5);
                CONSTANT(3, 2);

                BASIC_BLOCK(2, -1)
                {
                    INST(4, std::get<0>(shift_op)).type(type).Inputs(0, 2);
                    INST(5, Opcode::Sub).type(type).Inputs(1, 4);

                    INST(6, std::get<0>(shift_op)).type(type).Inputs(1, 2);
                    INST(7, Opcode::Sub).type(type).Inputs(6, 1);

                    INST(8, std::get<0>(shift_op)).type(type).Inputs(0, 2);
                    INST(9, Opcode::Sub).type(type).Inputs(1, 8);

                    INST(10, std::get<0>(shift_op)).type(type).Inputs(0, 2);
                    INST(11, Opcode::Sub).type(type).Inputs(3, 10);

                    INST(12, Opcode::Max).type(type).Inputs(5, 7);
                    INST(13, Opcode::Max).type(type).Inputs(12, 8);
                    INST(14, Opcode::Max).type(type).Inputs(13, 9);
                    INST(15, Opcode::Max).type(type).Inputs(14, 11);
                    INST(16, Opcode::Return).type(type).Inputs(15);
                }
            }

            EXPECT_TRUE(graph->RunPass<Lowering>());
            EXPECT_TRUE(graph->RunPass<Cleanup>());

            auto graph_expected = CreateEmptyGraph();
            GRAPH(graph_expected)
            {
                PARAMETER(0, 0).type(types[1]);
                PARAMETER(1, 1).type(types[2]);
                CONSTANT(2, 2);

                BASIC_BLOCK(2, -1)
                {
                    INST(3, Opcode::SubSR).Shift(std::get<2>(shift_op), 5).type(type).Inputs(1, 0);
                    INST(4, std::get<1>(shift_op)).Imm(5).type(type).Inputs(1);
                    INST(5, Opcode::Sub).type(type).Inputs(4, 1);
                    INST(6, std::get<1>(shift_op)).Imm(5).type(type).Inputs(0);
                    INST(7, Opcode::Sub).type(type).Inputs(1, 6);
                    INST(8, Opcode::SubSR).Shift(std::get<2>(shift_op), 5).type(type).Inputs(2, 0);

                    INST(9, Opcode::Max).type(type).Inputs(3, 5);
                    INST(10, Opcode::Max).type(type).Inputs(9, 6);
                    INST(11, Opcode::Max).type(type).Inputs(10, 7);
                    INST(12, Opcode::Max).type(type).Inputs(11, 8);
                    INST(13, Opcode::Return).type(type).Inputs(12);
                }
            }

            ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
        }
    }
}

TEST_F(LoweringTest, SubWithShiftedOperandWithIncompatibleTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<std::tuple<Opcode, Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
        {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
        {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};

    for (auto &shift_op : shift_ops) {
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s16();
            PARAMETER(1, 1).s16();
            CONSTANT(2, 5);
            CONSTANT(3, 2);

            BASIC_BLOCK(2, -1)
            {
                INST(4, std::get<0>(shift_op)).s16().Inputs(0, 2);
                INST(5, Opcode::Sub).s32().Inputs(1, 4);

                INST(6, std::get<0>(shift_op)).s16().Inputs(1, 2);
                INST(7, Opcode::Sub).s32().Inputs(6, 1);

                INST(8, std::get<0>(shift_op)).s16().Inputs(0, 2);
                INST(9, Opcode::Sub).s32().Inputs(1, 8);

                INST(10, std::get<0>(shift_op)).s16().Inputs(0, 2);
                INST(11, Opcode::Sub).s32().Inputs(3, 10);

                INST(12, Opcode::Max).s32().Inputs(5, 7);
                INST(13, Opcode::Max).s32().Inputs(12, 8);
                INST(14, Opcode::Max).s32().Inputs(13, 9);
                INST(15, Opcode::Max).s32().Inputs(14, 11);
                INST(16, Opcode::Return).s32().Inputs(15);
            }
        }

        auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_TRUE(GraphComparator().Compare(graph, clone));
    }
}

TEST_F(LoweringTest, NonCommutativeBinaryOpWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<std::pair<Opcode, Opcode>> opcodes = {{Opcode::Sub, Opcode::SubSR},
                                                                {Opcode::AndNot, Opcode::AndNotSR},
                                                                {Opcode::OrNot, Opcode::OrNotSR},
                                                                {Opcode::XorNot, Opcode::XorNotSR}};

    std::initializer_list<std::tuple<Opcode, Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
        {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
        {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    std::initializer_list<std::array<DataType::Type, 3>> type_combinations = {
        {DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::UINT32, DataType::UINT32, DataType::UINT32},
        {DataType::UINT32, DataType::UINT16, DataType::UINT8},
        {DataType::INT64, DataType::INT64, DataType::INT64},
        {DataType::UINT64, DataType::UINT64, DataType::UINT64}};

    for (auto &types : type_combinations) {
        for (auto &shift_op : shift_ops) {
            auto type = types[0];
            for (auto &ops : opcodes) {
                auto graph = CreateEmptyLowLevelGraph();
                GRAPH(graph)
                {
                    PARAMETER(0, 0).type(types[1]);
                    PARAMETER(1, 1).type(types[2]);
                    CONSTANT(2, 5);

                    BASIC_BLOCK(2, -1)
                    {
                        INST(3, std::get<0>(shift_op)).type(type).Inputs(0, 2);
                        INST(4, ops.first).type(type).Inputs(1, 3);

                        INST(5, std::get<0>(shift_op)).type(type).Inputs(1, 2);
                        INST(6, ops.first).type(type).Inputs(5, 1);

                        INST(7, std::get<0>(shift_op)).type(type).Inputs(0, 2);
                        INST(8, ops.first).type(type).Inputs(1, 7);

                        INST(9, Opcode::Max).type(type).Inputs(4, 6);
                        INST(10, Opcode::Max).type(type).Inputs(9, 8);
                        INST(11, Opcode::Max).type(type).Inputs(10, 7);
                        INST(12, Opcode::Return).type(type).Inputs(11);
                    }
                }

                EXPECT_TRUE(graph->RunPass<Lowering>());
                EXPECT_TRUE(graph->RunPass<Cleanup>());

                auto graph_expected = CreateEmptyGraph();
                GRAPH(graph_expected)
                {
                    PARAMETER(0, 0).type(types[1]);
                    PARAMETER(1, 1).type(types[2]);

                    BASIC_BLOCK(2, -1)
                    {
                        INST(2, ops.second).Shift(std::get<2>(shift_op), 5).type(type).Inputs(1, 0);
                        INST(3, std::get<1>(shift_op)).Imm(5).type(type).Inputs(1);
                        INST(4, ops.first).type(type).Inputs(3, 1);
                        INST(5, std::get<1>(shift_op)).Imm(5).type(type).Inputs(0);
                        INST(6, ops.first).type(type).Inputs(1, 5);

                        INST(7, Opcode::Max).type(type).Inputs(2, 4);
                        INST(8, Opcode::Max).type(type).Inputs(7, 6);
                        INST(9, Opcode::Max).type(type).Inputs(8, 5);
                        INST(10, Opcode::Return).type(type).Inputs(9);
                    }
                }

                ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
            }
        }
    }
}

TEST_F(LoweringTest, NonCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::Sub, Opcode::AndNot, Opcode::OrNot, Opcode::XorNot};

    std::initializer_list<std::tuple<Opcode, Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
        {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
        {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};

    for (auto &shift_op : shift_ops) {
        for (auto &ops : opcodes) {
            auto graph = CreateEmptyLowLevelGraph();
            GRAPH(graph)
            {
                PARAMETER(0, 0).s16();
                PARAMETER(1, 1).s16();
                CONSTANT(2, 5);

                BASIC_BLOCK(2, -1)
                {
                    INST(3, std::get<0>(shift_op)).s16().Inputs(0, 2);
                    INST(4, ops).s32().Inputs(1, 3);

                    INST(5, std::get<0>(shift_op)).s16().Inputs(1, 2);
                    INST(6, ops).s32().Inputs(5, 1);

                    INST(7, std::get<0>(shift_op)).s16().Inputs(0, 2);
                    INST(8, ops).s32().Inputs(1, 7);

                    INST(9, Opcode::Max).s32().Inputs(4, 6);
                    INST(10, Opcode::Max).s32().Inputs(9, 8);
                    INST(11, Opcode::Max).s32().Inputs(10, 7);
                    INST(12, Opcode::Return).s32().Inputs(11);
                }
            }

            auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
            EXPECT_TRUE(graph->RunPass<Lowering>());
            ASSERT_TRUE(GraphComparator().Compare(graph, clone));
        }
    }
}

TEST_F(LoweringTest, BitwiseInstructionsWithInvertedShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<std::pair<Opcode, Opcode>> opcodes = {
        {Opcode::And, Opcode::AndNotSR}, {Opcode::Or, Opcode::OrNotSR}, {Opcode::Xor, Opcode::XorNotSR}};

    std::initializer_list<std::pair<Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};

    std::initializer_list<std::array<DataType::Type, 3>> type_combinations = {
        {DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::UINT32, DataType::UINT32, DataType::UINT32},
        {DataType::UINT32, DataType::UINT16, DataType::UINT8},
        {DataType::INT64, DataType::INT64, DataType::INT64},
        {DataType::UINT64, DataType::UINT64, DataType::UINT64}};

    for (auto &types : type_combinations) {
        for (auto &shift_op : shift_ops) {
            auto type = types[0];
            for (auto &ops : opcodes) {
                auto graph = CreateEmptyLowLevelGraph();
                GRAPH(graph)
                {
                    PARAMETER(0, 0).type(types[1]);
                    PARAMETER(1, 1).type(types[2]);
                    CONSTANT(2, 5);

                    BASIC_BLOCK(2, -1)
                    {
                        INST(3, shift_op.first).type(type).Inputs(0, 2);
                        INST(4, Opcode::Not).type(type).Inputs(3);
                        INST(5, ops.first).type(type).Inputs(1, 4);
                        INST(6, Opcode::Return).type(type).Inputs(5);
                    }
                }

                EXPECT_TRUE(graph->RunPass<Lowering>());
                EXPECT_TRUE(graph->RunPass<Cleanup>());

                auto graph_expected = CreateEmptyGraph();
                GRAPH(graph_expected)
                {
                    PARAMETER(0, 0).type(types[1]);
                    PARAMETER(1, 1).type(types[2]);

                    BASIC_BLOCK(2, -1)
                    {
                        INST(2, ops.second).Shift(shift_op.second, 5).type(type).Inputs(1, 0);
                        INST(3, Opcode::Return).type(type).Inputs(2);
                    }
                }

                ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
            }
        }
    }
}

TEST_F(LoweringTest, BitwiseInstructionsWithInvertedShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::And, Opcode::Or, Opcode::Xor};

    std::initializer_list<std::pair<Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};

    for (auto &shift_op : shift_ops) {
        for (auto &ops : opcodes) {
            auto graph = CreateEmptyLowLevelGraph();
            GRAPH(graph)
            {
                PARAMETER(0, 0).s16();
                PARAMETER(1, 1).s16();
                CONSTANT(2, 5);

                BASIC_BLOCK(2, -1)
                {
                    INST(3, shift_op.first).s16().Inputs(0, 2);
                    INST(4, Opcode::Not).s16().Inputs(3);
                    INST(5, ops).s32().Inputs(1, 4);
                    INST(6, Opcode::Return).s32().Inputs(5);
                }
            }

            auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
            EXPECT_TRUE(graph->RunPass<Lowering>());
            ASSERT_TRUE(GraphComparator().Compare(graph, clone));
        }
    }
}

TEST_F(LoweringTest, NegWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operand are only supported on Aarch64";
    }

    std::initializer_list<std::pair<Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};
    std::initializer_list<std::pair<DataType::Type, DataType::Type>> type_combinations = {
        {DataType::INT32, DataType::INT32},
        {DataType::UINT32, DataType::UINT32},
        {DataType::UINT32, DataType::UINT16},
        {DataType::INT64, DataType::INT64},
        {DataType::UINT64, DataType::UINT64}};
    for (auto &types : type_combinations) {
        for (auto &shift_op : shift_ops) {
            auto type = types.first;
            auto graph = CreateEmptyLowLevelGraph();
            GRAPH(graph)
            {
                PARAMETER(0, 0).type(types.second);
                CONSTANT(1, 5);

                BASIC_BLOCK(2, -1)
                {
                    INST(2, shift_op.first).type(type).Inputs(0, 1);
                    INST(3, Opcode::Neg).type(type).Inputs(2);
                    INST(4, Opcode::Return).type(type).Inputs(3);
                }
            }

            EXPECT_TRUE(graph->RunPass<Lowering>());
            EXPECT_TRUE(graph->RunPass<Cleanup>());

            auto graph_expected = CreateEmptyGraph();
            GRAPH(graph_expected)
            {
                PARAMETER(0, 0).type(types.second);

                BASIC_BLOCK(2, -1)
                {
                    INST(1, Opcode::NegSR).Shift(shift_op.second, 5).type(type).Inputs(0);
                    INST(2, Opcode::Return).type(type).Inputs(1);
                }
            }

            ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
        }
    }
}

TEST_F(LoweringTest, NegWithShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operand are only supported on Aarch64";
    }

    std::initializer_list<std::pair<Opcode, ShiftType>> shift_ops = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};

    for (auto &shift_op : shift_ops) {
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).s16();
            CONSTANT(1, 5);

            BASIC_BLOCK(2, -1)
            {
                INST(2, shift_op.first).s16().Inputs(0, 1);
                INST(3, Opcode::Neg).s32().Inputs(2);
                INST(4, Opcode::Return).s32().Inputs(3);
            }
        }

        auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_TRUE(GraphComparator().Compare(graph, clone));
    }
}

TEST_F(LoweringTest, AddSwapInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        CONSTANT(1, 5);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).s64().Inputs(1, 0);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    EXPECT_TRUE(GetGraph()->RunPass<Lowering>());
    EXPECT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph_expected = CreateEmptyGraph();
    GRAPH(graph_expected)
    {
        PARAMETER(0, 0).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::AddI).s64().Inputs(0).Imm(5);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_expected));
}

// Not applied, sub isn't commutative inst
TEST_F(LoweringTest, SubSwapInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        CONSTANT(1, 5);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).s64().Inputs(1, 0);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_TRUE(GetGraph()->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(LoweringTest, DeoptimizeCompare)
{
    // Check if Compare + DeoptimizeIf ===> DeoptimizeCompare/DeoptimizeCompareImm transformations are applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        CONSTANT(2, 10).s32();
        CONSTANT(3, nullptr).ref();
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(7, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(8, Opcode::Compare).b().CC(ConditionCode::CC_EQ).Inputs(5, 3);
            INST(9, Opcode::DeoptimizeIf).Inputs(8, 7);
            INST(10, Opcode::LenArray).s32().Inputs(5);
            INST(11, Opcode::Compare).b().CC(ConditionCode::CC_GT).Inputs(2, 10);
            INST(12, Opcode::DeoptimizeIf).Inputs(11, 7);
            INST(13, Opcode::Return).ref().Inputs(5);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lowering>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        CONSTANT(2, 10).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(5, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(7, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
            INST(10, Opcode::DeoptimizeCompareImm).CC(ConditionCode::CC_EQ).Imm(0).Inputs(5, 7);
            INST(11, Opcode::LenArray).s32().Inputs(5);
            INST(14, Opcode::DeoptimizeCompare).CC(ConditionCode::CC_GT).Inputs(2, 11, 7);
            INST(15, Opcode::Return).ref().Inputs(5);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LoweringTest, DeoptimizeCompareImmDoesNotFit)
{
    auto graph = CreateEmptyGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    if (graph->GetArch() == Arch::AARCH32 || graph->GetArch() == Arch::AARCH64) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s32();
            CONSTANT(2, INT_MAX).s32();
            BASIC_BLOCK(2, 1)
            {
                INST(5, Opcode::LoadArray).ref().Inputs(0, 1);
                INST(6, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
                INST(7, Opcode::LenArray).s32().Inputs(5);
                INST(8, Opcode::Compare).b().CC(ConditionCode::CC_GT).Inputs(7, 2);
                INST(9, Opcode::DeoptimizeIf).Inputs(8, 6);
                INST(10, Opcode::Return).ref().Inputs(5);
            }
        }
    } else if (graph->GetArch() == Arch::X86_64) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s32();
            CONSTANT(2, LONG_MAX).s64();
            BASIC_BLOCK(2, 1)
            {
                INST(5, Opcode::LoadArray).ref().Inputs(0, 1);
                INST(6, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
                INST(7, Opcode::LenArray).s32().Inputs(5);
                INST(8, Opcode::Cast).s64().SrcType(compiler::DataType::INT32).Inputs(7);
                INST(9, Opcode::Compare).b().CC(ConditionCode::CC_GT).Inputs(8, 2);
                INST(10, Opcode::DeoptimizeIf).Inputs(9, 6);
                INST(11, Opcode::Return).ref().Inputs(5);
            }
        }
    } else {
        UNREACHABLE();
    }
    ASSERT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyGraph(graph->GetArch());
    if (graphExpected->GetArch() == Arch::AARCH32 || graphExpected->GetArch() == Arch::AARCH64) {
        GRAPH(graphExpected)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s32();
            CONSTANT(2, INT_MAX).s32();
            BASIC_BLOCK(2, 1)
            {
                INST(5, Opcode::LoadArray).ref().Inputs(0, 1);
                INST(6, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
                INST(7, Opcode::LenArray).s32().Inputs(5);
                INST(9, Opcode::DeoptimizeCompare).CC(ConditionCode::CC_GT).Inputs(7, 2, 6);
                INST(10, Opcode::Return).ref().Inputs(5);
            }
        }
    } else if (graphExpected->GetArch() == Arch::X86_64) {
        GRAPH(graphExpected)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).s32();
            CONSTANT(2, LONG_MAX).s64();
            BASIC_BLOCK(2, 1)
            {
                INST(5, Opcode::LoadArray).ref().Inputs(0, 1);
                INST(6, Opcode::SaveStateDeoptimize).Inputs(0, 1).SrcVregs({0, 1});
                INST(7, Opcode::LenArray).s32().Inputs(5);
                INST(8, Opcode::Cast).s64().SrcType(compiler::DataType::INT32).Inputs(7);
                INST(10, Opcode::DeoptimizeCompare).CC(ConditionCode::CC_GT).Inputs(8, 2, 6);
                INST(11, Opcode::Return).ref().Inputs(5);
            }
        }
    } else {
        UNREACHABLE();
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(LoweringTest, LowerMoveScaleInLoadStore)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    if (graph->GetArch() == Arch::AARCH32) {
        GTEST_SKIP() << "The optimization isn't supported on Aarch32";
    }
    GRAPH(graph)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).ptr();
        PARAMETER(2, 2).u64();
        PARAMETER(80, 3).u64();
        PARAMETER(3, 4).u32();
        CONSTANT(4, 1);
        CONSTANT(5, 2);
        CONSTANT(6, 3);
        CONSTANT(7, 4);

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Shl).u64().Inputs(2, 4);
            INST(9, Opcode::Load).u64().Inputs(0, 8);
            INST(10, Opcode::Store).u64().Inputs(1, 8, 9);
            INST(11, Opcode::Shl).u64().Inputs(2, 5);
            INST(12, Opcode::Load).u64().Inputs(0, 11);
            INST(13, Opcode::Store).u64().Inputs(1, 11, 12);
            INST(14, Opcode::Shl).u64().Inputs(2, 6);
            INST(15, Opcode::Load).u64().Inputs(0, 14);
            INST(16, Opcode::Store).u64().Inputs(1, 14, 15);
            INST(17, Opcode::Shl).u64().Inputs(2, 7);
            INST(18, Opcode::Load).u64().Inputs(0, 17);
            INST(19, Opcode::Store).u64().Inputs(1, 17, 18);
            INST(20, Opcode::Shl).u64().Inputs(80, 4);
            INST(21, Opcode::Load).u16().Inputs(0, 20);
            INST(22, Opcode::Store).u16().Inputs(1, 20, 21);
            INST(23, Opcode::Shl).u64().Inputs(80, 5);
            INST(24, Opcode::Load).u32().Inputs(0, 23);
            INST(25, Opcode::Store).u32().Inputs(1, 23, 24);
            INST(26, Opcode::Shl).u32().Inputs(3, 6);
            INST(27, Opcode::Load).u64().Inputs(0, 26);
            INST(28, Opcode::Store).u64().Inputs(1, 26, 27);
            INST(90, Opcode::ReturnVoid).v0id();
        }
    }
    EXPECT_TRUE(graph->RunPass<Lowering>());
    EXPECT_TRUE(graph->RunPass<Cleanup>());

    auto graph_expected = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    if (graph->GetArch() == Arch::AARCH64) {
        GRAPH(graph_expected)
        {
            PARAMETER(0, 0).ptr();
            PARAMETER(1, 1).ptr();
            PARAMETER(2, 2).u64();
            PARAMETER(80, 3).u64();
            PARAMETER(3, 4).u32();

            BASIC_BLOCK(2, -1)
            {
                INST(8, Opcode::ShlI).u64().Inputs(2).Imm(1);
                INST(9, Opcode::Load).u64().Inputs(0, 8);
                INST(10, Opcode::Store).u64().Inputs(1, 8, 9);
                INST(11, Opcode::ShlI).u64().Inputs(2).Imm(2);
                INST(12, Opcode::Load).u64().Inputs(0, 11);
                INST(13, Opcode::Store).u64().Inputs(1, 11, 12);
                INST(15, Opcode::Load).u64().Inputs(0, 2).Scale(3);
                INST(16, Opcode::Store).u64().Inputs(1, 2, 15).Scale(3);
                INST(17, Opcode::ShlI).u64().Inputs(2).Imm(4);
                INST(18, Opcode::Load).u64().Inputs(0, 17);
                INST(19, Opcode::Store).u64().Inputs(1, 17, 18);
                INST(21, Opcode::Load).u16().Inputs(0, 80).Scale(1);
                INST(22, Opcode::Store).u16().Inputs(1, 80, 21).Scale(1);
                INST(24, Opcode::Load).u32().Inputs(0, 80).Scale(2);
                INST(25, Opcode::Store).u32().Inputs(1, 80, 24).Scale(2);
                INST(26, Opcode::ShlI).u32().Inputs(3).Imm(3);
                INST(27, Opcode::Load).u64().Inputs(0, 26);
                INST(28, Opcode::Store).u64().Inputs(1, 26, 27);
                INST(90, Opcode::ReturnVoid).v0id();
            }
        }
    } else {
        ASSERT(graph->GetArch() == Arch::X86_64);
        GRAPH(graph_expected)
        {
            PARAMETER(0, 0).ptr();
            PARAMETER(1, 1).ptr();
            PARAMETER(2, 2).u64();
            PARAMETER(80, 3).u64();
            PARAMETER(3, 4).u32();

            BASIC_BLOCK(2, -1)
            {
                INST(9, Opcode::Load).u64().Inputs(0, 2).Scale(1);
                INST(10, Opcode::Store).u64().Inputs(1, 2, 9).Scale(1);
                INST(12, Opcode::Load).u64().Inputs(0, 2).Scale(2);
                INST(13, Opcode::Store).u64().Inputs(1, 2, 12).Scale(2);
                INST(15, Opcode::Load).u64().Inputs(0, 2).Scale(3);
                INST(16, Opcode::Store).u64().Inputs(1, 2, 15).Scale(3);
                INST(17, Opcode::ShlI).u64().Inputs(2).Imm(4);
                INST(18, Opcode::Load).u64().Inputs(0, 17);
                INST(19, Opcode::Store).u64().Inputs(1, 17, 18);
                INST(21, Opcode::Load).u16().Inputs(0, 80).Scale(1);
                INST(22, Opcode::Store).u16().Inputs(1, 80, 21).Scale(1);
                INST(24, Opcode::Load).u32().Inputs(0, 80).Scale(2);
                INST(25, Opcode::Store).u32().Inputs(1, 80, 24).Scale(2);
                INST(26, Opcode::ShlI).u32().Inputs(3).Imm(3);
                INST(27, Opcode::Load).u64().Inputs(0, 26);
                INST(28, Opcode::Store).u64().Inputs(1, 26, 27);
                INST(90, Opcode::ReturnVoid).v0id();
            }
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());
}

TEST_F(LoweringTest, LowerUnsignedCast)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graph)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Load).u8().Inputs(0, 1);
            INST(3, Opcode::Cast).u64().SrcType(DataType::UINT8).Inputs(2);
            INST(4, Opcode::Load).u16().Inputs(0, 3);
            INST(5, Opcode::Cast).u64().SrcType(DataType::UINT16).Inputs(4);
            INST(6, Opcode::Load).u32().Inputs(0, 5);
            INST(7, Opcode::Cast).u64().SrcType(DataType::UINT32).Inputs(6);
            INST(8, Opcode::Load).f32().Inputs(0, 7);
            INST(9, Opcode::Cast).u64().SrcType(DataType::FLOAT32).Inputs(8);
            INST(10, Opcode::Load).s8().Inputs(0, 9);
            INST(11, Opcode::Cast).u64().SrcType(DataType::INT8).Inputs(10);
            INST(12, Opcode::Load).u8().Inputs(0, 11);
            INST(13, Opcode::Cast).u32().SrcType(DataType::UINT8).Inputs(12);
            INST(14, Opcode::Load).u64().Inputs(0, 13);
            INST(15, Opcode::Cast).u32().SrcType(DataType::UINT64).Inputs(14);
            INST(16, Opcode::Mul).u8().Inputs(15, 15);
            INST(17, Opcode::Cast).u64().SrcType(DataType::UINT8).Inputs(16);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }

    auto graph_expected = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    if (graph->GetArch() == Arch::AARCH64) {
        GRAPH(graph_expected)
        {
            PARAMETER(0, 0).ptr();
            PARAMETER(1, 1).u64();

            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Load).u8().Inputs(0, 1);
                INST(4, Opcode::Load).u16().Inputs(0, 2);
                INST(6, Opcode::Load).u32().Inputs(0, 4);
                INST(8, Opcode::Load).f32().Inputs(0, 6);
                INST(9, Opcode::Cast).u64().SrcType(DataType::FLOAT32).Inputs(8);
                INST(10, Opcode::Load).s8().Inputs(0, 9);
                INST(11, Opcode::Cast).u64().SrcType(DataType::INT8).Inputs(10);
                INST(12, Opcode::Load).u8().Inputs(0, 11);
                INST(14, Opcode::Load).u64().Inputs(0, 12);
                INST(15, Opcode::Cast).u32().SrcType(DataType::UINT64).Inputs(14);
                INST(16, Opcode::Mul).u8().Inputs(15, 15);
                INST(17, Opcode::Cast).u64().SrcType(DataType::UINT8).Inputs(16);
                INST(18, Opcode::Return).u64().Inputs(17);
            }
        }
    } else {
        graph_expected = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    }
    EXPECT_TRUE(graph->RunPass<Lowering>());
    if (graph->GetArch() == Arch::AARCH64) {
        EXPECT_TRUE(graph->RunPass<Cleanup>());
    } else {
        EXPECT_FALSE(graph->RunPass<Cleanup>());
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());
}
}  // namespace panda::compiler
