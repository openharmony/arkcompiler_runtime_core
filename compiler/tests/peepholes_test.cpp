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
#include "optimizer/optimizations/peepholes.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/cleanup.h"

namespace panda::compiler {
class PeepholesTest : public CommonTest {
public:
    PeepholesTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

    void CheckCompare(DataType::Type param_type, ConditionCode orig_cc, ConditionCode cc)
    {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0, 0);
            PARAMETER(1, 0);
            INS(1).SetType(param_type);
            PARAMETER(2, 1);
            INS(2).SetType(param_type);
            BASIC_BLOCK(2, 1)
            {
                INST(3, Opcode::Cmp).s32().Inputs(1, 2);
                INST(4, Opcode::Compare).b().CC(orig_cc).Inputs(3, 0);
                INST(5, Opcode::Return).b().Inputs(4);
            }
        }
        graph1->RunPass<Peepholes>();
        GraphChecker(graph1).Check();

        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            CONSTANT(0, 0);
            PARAMETER(1, 0);
            INS(1).SetType(param_type);
            PARAMETER(2, 1);
            INS(2).SetType(param_type);
            BASIC_BLOCK(2, 1)
            {
                INST(3, Opcode::Cmp).s32().Inputs(1, 2);
                INST(4, Opcode::Compare).b().CC(cc).Inputs(1, 2);
                INST(5, Opcode::Return).b().Inputs(4);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    void CheckCompare(ConditionCode cc, int64_t cst, std::optional<uint64_t> exp_cst, bool exp_inv)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0, cst);
            PARAMETER(1, 0).b();
            BASIC_BLOCK(2, 3, 4)
            {
                INST(3, Opcode::Compare).b().CC(cc).Inputs(1, 0);
                INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
            }
            BASIC_BLOCK(3, -1)
            {
                INST(5, Opcode::ReturnVoid);
            }
            BASIC_BLOCK(4, -1)
            {
                INST(6, Opcode::ReturnVoid);
            }
        }
        graph->RunPass<Peepholes>();
        GraphChecker(graph).Check();

        EXPECT_FALSE(INS(3).HasUsers());
        if (exp_cst.has_value()) {
            auto inst = INS(4).GetInput(0).GetInst();
            EXPECT_EQ(inst->GetOpcode(), Opcode::Constant);
            EXPECT_EQ(inst->CastToConstant()->GetIntValue(), *exp_cst);
        } else {
            EXPECT_EQ(INS(4).GetInput(0).GetInst(), &INS(1));
        }
        EXPECT_EQ(INS(4).CastToIfImm()->GetImm(), 0);
        EXPECT_EQ(INS(4).CastToIfImm()->GetCc() == CC_EQ, exp_inv);
    }

    void CheckCast(DataType::Type src_type, DataType::Type tgt_type, bool applied)
    {
        if (src_type == DataType::REFERENCE || tgt_type == DataType::REFERENCE) {
            return;
        }
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0);
            INS(0).SetType(src_type);
            BASIC_BLOCK(2, 1)
            {
                INST(1, Opcode::Cast).SrcType(src_type).Inputs(0);
                INS(1).SetType(tgt_type);
                INST(2, Opcode::Return).Inputs(1);
                INS(2).SetType(tgt_type);
            }
        }
        auto graph2 = CreateEmptyGraph();
        if (applied) {
            GRAPH(graph2)
            {
                PARAMETER(0, 0);
                INS(0).SetType(src_type);
                BASIC_BLOCK(2, 1)
                {
                    INST(1, Opcode::Cast).SrcType(src_type).Inputs(0);
                    INS(1).SetType(tgt_type);
                    INST(2, Opcode::Return).Inputs(0);
                    INS(2).SetType(src_type);
                }
            }
        } else {
            GRAPH(graph2)
            {
                PARAMETER(0, 0);
                INS(0).SetType(src_type);
                BASIC_BLOCK(2, 1)
                {
                    INST(1, Opcode::Cast).SrcType(src_type).Inputs(0);
                    INS(1).SetType(tgt_type);
                    INST(2, Opcode::Return).Inputs(1);
                    INS(2).SetType(tgt_type);
                }
            }
        }
        ASSERT_EQ(graph1->RunPass<Peepholes>(), applied);
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    void CheckCast(DataType::Type src_type, DataType::Type mdl_type, DataType::Type tgt_type, bool applied,
                   bool join_cast)
    {
        if (src_type == DataType::REFERENCE || tgt_type == DataType::REFERENCE || mdl_type == DataType::REFERENCE) {
            return;
        }
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0);
            INS(0).SetType(src_type);
            BASIC_BLOCK(2, 1)
            {
                INST(1, Opcode::Cast).SrcType(src_type).Inputs(0);
                INS(1).SetType(mdl_type);
                INST(2, Opcode::Cast).SrcType(mdl_type).Inputs(1);
                INS(2).SetType(tgt_type);
                INST(3, Opcode::Return).Inputs(2);
                INS(3).SetType(tgt_type);
            }
        }
        auto graph2 = CreateEmptyGraph();
        if (applied) {
            if (join_cast) {
                GRAPH(graph2)
                {
                    PARAMETER(0, 0);
                    INS(0).SetType(src_type);
                    BASIC_BLOCK(2, 1)
                    {
                        INST(1, Opcode::Cast).SrcType(src_type).Inputs(0);
                        INS(1).SetType(mdl_type);
                        INST(2, Opcode::Cast).SrcType(src_type).Inputs(0);
                        INS(2).SetType(tgt_type);
                        INST(3, Opcode::Return).Inputs(2);
                        INS(3).SetType(tgt_type);
                    }
                }
            } else {
                GRAPH(graph2)
                {
                    PARAMETER(0, 0);
                    INS(0).SetType(src_type);
                    BASIC_BLOCK(2, 1)
                    {
                        INST(1, Opcode::Cast).SrcType(src_type).Inputs(0);
                        INS(1).SetType(mdl_type);
                        INST(2, Opcode::Cast).SrcType(mdl_type).Inputs(1);
                        INS(2).SetType(tgt_type);
                        INST(3, Opcode::Return).Inputs(0);
                        INS(3).SetType(src_type);
                    }
                }
            }
        } else {
            GRAPH(graph2)
            {
                PARAMETER(0, 0);
                INS(0).SetType(src_type);
                BASIC_BLOCK(2, 1)
                {
                    INST(1, Opcode::Cast).SrcType(src_type).Inputs(0);
                    INS(1).SetType(mdl_type);
                    INST(2, Opcode::Cast).SrcType(mdl_type).Inputs(1);
                    INS(2).SetType(tgt_type);
                    INST(3, Opcode::Return).Inputs(2);
                    INS(3).SetType(tgt_type);
                }
            }
        }
        ASSERT_EQ(graph1->RunPass<Peepholes>(), applied);
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    void CheckCast(int i, int j, int k)
    {
        if (i == j || j == k) {
            return;
        }
        auto src_type = static_cast<DataType::Type>(i);
        auto mdl_type = static_cast<DataType::Type>(j);
        auto tgt_type = static_cast<DataType::Type>(k);
        auto join_cast = DataType::GetTypeSize(src_type, GetArch()) > DataType::GetTypeSize(mdl_type, GetArch()) &&
                         DataType::GetTypeSize(mdl_type, GetArch()) > DataType::GetTypeSize(tgt_type, GetArch());
        CheckCast(src_type, mdl_type, tgt_type,
                  (src_type == tgt_type &&
                   DataType::GetTypeSize(mdl_type, GetArch()) > DataType::GetTypeSize(tgt_type, GetArch())) ||
                      join_cast,
                  join_cast);
    }

    void CheckCompareFoldIntoTest(uint64_t constant, ConditionCode cc, bool success, ConditionCode expected_cc = CC_EQ)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, constant);

            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::And).u64().Inputs(0, 1);
                INST(4, Opcode::Compare).b().CC(cc).Inputs(3, 2);
                INST(5, Opcode::Return).b().Inputs(4);
            }
        }

        ASSERT_EQ(graph->RunPass<Peepholes>(), success);
        if (!success) {
            return;
        }

        graph->RunPass<Cleanup>();

        auto expected_graph = CreateEmptyGraph();
        GRAPH(expected_graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();

            BASIC_BLOCK(2, -1)
            {
                INST(4, Opcode::Compare).b().CC(expected_cc).Inputs(0, 1);
                INST(5, Opcode::Return).b().Inputs(4);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
    }

    void CheckIfAndZeroFoldIntoIfTest(uint64_t constant, ConditionCode cc, bool success,
                                      ConditionCode expected_cc = CC_EQ)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(2, constant);
            CONSTANT(3, -1);
            CONSTANT(4, -2);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(5, Opcode::And).u64().Inputs(0, 1);
                INST(6, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(5, 2);
            }

            BASIC_BLOCK(3, 4) {}

            BASIC_BLOCK(4, -1)
            {
                INST(7, Opcode::Phi).s64().Inputs(3, 4);
                INST(8, Opcode::Return).s64().Inputs(7);
            }
        }

        ASSERT_EQ(graph->RunPass<Peepholes>(), success);
        if (!success) {
            return;
        }

        graph->RunPass<Cleanup>();

        auto expected_graph = CreateEmptyGraph();
        GRAPH(expected_graph)
        {
            PARAMETER(0, 0).u64();
            PARAMETER(1, 1).u64();
            CONSTANT(3, -1);
            CONSTANT(4, -2);

            BASIC_BLOCK(2, 3, 4)
            {
                INST(6, Opcode::If).SrcType(DataType::UINT64).CC(expected_cc).Inputs(0, 1);
            }

            BASIC_BLOCK(3, 4) {}

            BASIC_BLOCK(4, -1)
            {
                INST(7, Opcode::Phi).s64().Inputs(3, 4);
                INST(8, Opcode::Return).s64().Inputs(7);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
    }

    void CheckCompareLenArrayWithZeroTest(int64_t constant, ConditionCode cc, std::optional<bool> expected_value,
                                          bool swap = false)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            CONSTANT(1, constant);
            BASIC_BLOCK(2, -1)
            {
                INST(3, Opcode::LenArray).s32().Inputs(0);
                if (swap) {
                    INST(4, Opcode::Compare).b().CC(cc).Inputs(1, 3);
                } else {
                    INST(4, Opcode::Compare).b().CC(cc).Inputs(3, 1);
                }
                INST(5, Opcode::Return).b().Inputs(4);
            }
        }

        ASSERT_EQ(graph->RunPass<Peepholes>(), expected_value.has_value());
        if (!expected_value.has_value()) {
            return;
        }

        graph->RunPass<Cleanup>();

        auto expected_graph = CreateEmptyGraph();
        GRAPH(expected_graph)
        {
            CONSTANT(1, *expected_value);
            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Return).b().Inputs(1);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, expected_graph));
    }

protected:
    Graph *graph_ {nullptr};
};

TEST_F(PeepholesTest, TestAnd1)
{
    // case 1:
    // 0.i64 Const ...
    // 1.i64 AND v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 AND v5, v0
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 2);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).u64().Inputs(1, 0);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(2).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(2).GetInput(1).GetInst(), &INS(1));
    ASSERT_EQ(INS(1).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestAnd2)
{
    // case 2:
    // 0.i64 Const 0xFFF..FF
    // 1.i64 AND v5, v0
    // 2.i64 INS which use v1
    // ===>
    // 0.i64 Const 0xFFF..FF
    // 1.i64 AND v5, v0
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, -1);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).u64().Inputs(0, 1);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(0));
}

TEST_F(PeepholesTest, TestAnd2_1)
{
    // Case 1 + Case 2
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, -1);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).u64().Inputs(1, 0);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(1).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestAnd3)
{
    // case 3:
    // 1.i64 AND v5, v5
    // 2.i64 INS which use v1
    // ===>
    // 1.i64 AND v5, v5
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).u64().Inputs(0, 0);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(0));
}

TEST_F(PeepholesTest, TestAnd3_1)
{
    // case 3:
    // but input's type not equal with and-inst's type
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::And).s16().Inputs(0, 0);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(2));
}

TEST_F(PeepholesTest, TestAnd4)
{
    // case 4: De Morgan rules
    // 2.i64 not v0 -> {4}
    // 3.i64 not v1 -> {4}
    // 4.i64 AND v2, v3
    // ===>
    // 5.i64 OR v0, v1
    // 6.i64 not v5
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Not).u64().Inputs(0);
            INST(3, Opcode::Not).u64().Inputs(1);
            INST(4, Opcode::And).u64().Inputs(2, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto not_inst = INS(5).GetInput(0).GetInst();
    ASSERT_EQ(not_inst->GetOpcode(), Opcode::Not);
    auto or_inst = not_inst->GetInput(0).GetInst();
    ASSERT_EQ(or_inst->GetOpcode(), Opcode::Or);
    ASSERT_EQ(or_inst->GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(or_inst->GetInput(1).GetInst(), &INS(1));
}

TEST_F(PeepholesTest, TestAnd4_1)
{
    // Case 4, but NOT-inst have more than 1 user
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Not).u64().Inputs(0);
            INST(3, Opcode::Not).u64().Inputs(1);
            INST(4, Opcode::And).u64().Inputs(2, 3);
            INST(5, Opcode::Not).u64().Inputs(2);
            INST(6, Opcode::Not).u64().Inputs(3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).u64().InputsAutoType(4, 5, 6, 20);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto and_inst = INS(7).GetInput(0).GetInst();
    ASSERT_EQ(and_inst->GetOpcode(), Opcode::And);
}

TEST_F(PeepholesTest, TestOr1)
{
    // case 1:
    // 0.i64 Const ...
    // 1.i64 Or v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 Or v5, v0
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 2);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Or).u64().Inputs(1, 0);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(2).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(2).GetInput(1).GetInst(), &INS(1));
    ASSERT_EQ(INS(1).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestOr2)
{
    // case 2:
    // 0.i64 Const 0x000..00
    // 1.i64 OR v5, v0
    // 2.i64 INS which use v1
    // ===>
    // 0.i64 Const 0x000..00
    // 1.i64 OR v5, v0
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Or).u64().Inputs(1, 0);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(0));
}

TEST_F(PeepholesTest, AddConstantTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 1);
        CONSTANT(3, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).u64().Inputs(2, 0);
            INST(5, Opcode::Add).u64().Inputs(3, 1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).u64().InputsAutoType(4, 5, 20);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_TRUE(CheckInputs(INS(4), {0, 2}));
    ASSERT_TRUE(CheckInputs(INS(5), {1, 3}));
    ASSERT_TRUE(CheckInputs(INS(6), {4, 1, 20}));

    ASSERT_TRUE(CheckUsers(INS(0), {4}));
    ASSERT_TRUE(CheckUsers(INS(2), {4}));
    ASSERT_TRUE(CheckUsers(INS(3), {5}));
    ASSERT_TRUE(INS(5).GetUsers().Empty());
}

TEST_F(PeepholesTest, AddConstantTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 1);
            INST(4, Opcode::Sub).u64().Inputs(0, 1);

            INST(5, Opcode::Add).u64().Inputs(2, 3);
            INST(6, Opcode::Add).u64().Inputs(2, 4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).u64().InputsAutoType(5, 6, 20);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    auto const3 = INS(2).GetNext();
    ASSERT_TRUE(const3 != nullptr);
    auto const4 = const3->GetNext();
    ASSERT_TRUE(const4 != nullptr && const4->GetNext() == nullptr);

    ASSERT_TRUE(INS(5).GetInput(0).GetInst() == &INS(0) && INS(5).GetInput(1).GetInst() == const3);
    ASSERT_TRUE(INS(6).GetInput(0).GetInst() == &INS(0) && INS(6).GetInput(1).GetInst() == const4);

    ASSERT_TRUE(CheckUsers(INS(0), {3, 4, 5, 6}));
    ASSERT_TRUE(CheckUsers(INS(1), {3, 4}));
    ASSERT_TRUE(CheckUsers(*const3, {5}));
    ASSERT_TRUE(CheckUsers(*const4, {6}));
    ASSERT_TRUE(INS(2).GetUsers().Empty());
}

TEST_F(PeepholesTest, AddConstantTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u16().Inputs(0, 1);
            INST(4, Opcode::Sub).u16().Inputs(0, 1);

            INST(5, Opcode::Add).u32().Inputs(2, 3);
            INST(6, Opcode::Add).u32().Inputs(2, 4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).u64().InputsAutoType(5, 6, 20);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(2).GetNext() == nullptr);

    ASSERT_TRUE(CheckInputs(INS(3), {0, 1}));
    ASSERT_TRUE(CheckInputs(INS(4), {0, 1}));
    ASSERT_TRUE(CheckInputs(INS(5), {3, 2}));
    ASSERT_TRUE(CheckInputs(INS(6), {4, 2}));

    ASSERT_TRUE(CheckUsers(INS(0), {3, 4}));
    ASSERT_TRUE(CheckUsers(INS(1), {3, 4}));
    ASSERT_TRUE(CheckUsers(INS(2), {5, 6}));
    ASSERT_TRUE(CheckUsers(INS(3), {5}));
    ASSERT_TRUE(CheckUsers(INS(4), {6}));
}

TEST_F(PeepholesTest, AddNegTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(3, Opcode::Neg).u64().Inputs(1);

            INST(4, Opcode::Add).u64().Inputs(2, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    auto new_neg = INS(4).GetNext();

    ASSERT_TRUE(CheckUsers(INS(0), {2, 4}));
    ASSERT_TRUE(CheckUsers(INS(1), {3, 4}));
    ASSERT_TRUE(INS(2).GetUsers().Empty());
    ASSERT_TRUE(INS(3).GetUsers().Empty());

    ASSERT(CheckInputs(INS(4), {0, 1}));

    auto user = INS(4).GetUsers().begin();
    ASSERT_TRUE(user->GetInst() == new_neg);
    ASSERT_FALSE(++user != INS(4).GetUsers().end());

    ASSERT_TRUE(new_neg->GetInput(0).GetInst() == &INS(4));
    ASSERT_TRUE(CheckUsers(*new_neg, {5}));

    ASSERT_TRUE(INS(5).GetInput(0).GetInst() == new_neg);
}

TEST_F(PeepholesTest, AddNegTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(3, Opcode::Neg).u64().Inputs(1);

            INST(4, Opcode::Add).u64().Inputs(2, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).u64().InputsAutoType(2, 4, 20);
            INST(5, Opcode::Return).u64().Inputs(6);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckUsers(INS(0), {2}));
    ASSERT_TRUE(CheckUsers(INS(1), {3}));

    ASSERT_TRUE(CheckInputs(INS(2), {0}));
    ASSERT_TRUE(CheckUsers(INS(2), {4, 6}));
    ASSERT_TRUE(CheckInputs(INS(3), {1}));
    ASSERT_TRUE(CheckUsers(INS(3), {4}));

    ASSERT_TRUE(CheckInputs(INS(4), {2, 3}));
}

TEST_F(PeepholesTest, AddNegTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        CONSTANT(8, 1);
        CONSTANT(9, 2);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(3, Opcode::Neg).u64().Inputs(1);

            INST(4, Opcode::Add).u64().Inputs(2, 8);
            INST(5, Opcode::Add).u64().Inputs(3, 9);

            INST(6, Opcode::Add).u64().Inputs(2, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(10, Opcode::CallStatic).u64().InputsAutoType(4, 5, 6, 20);
            INST(7, Opcode::Return).u64().Inputs(10);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_TRUE(CheckUsers(INS(0), {2}));
    ASSERT_TRUE(CheckUsers(INS(1), {3}));
    ASSERT_TRUE(CheckUsers(INS(2), {4, 6}));
    ASSERT_TRUE(CheckUsers(INS(3), {5, 6}));

    ASSERT(CheckInputs(INS(4), {2, 8}));
    ASSERT(CheckInputs(INS(5), {3, 9}));

    ASSERT_TRUE(CheckInputs(INS(6), {2, 3}));
    ASSERT_TRUE(CheckUsers(INS(6), {10}));
    ASSERT_TRUE(INS(6).GetNext()->GetNext() == &INS(10));

    ASSERT_TRUE(CheckInputs(INS(7), {10}));
}

TEST_F(PeepholesTest, AddNegTest4)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(3, Opcode::Neg).u64().Inputs(1);

            INST(4, Opcode::Add).u64().Inputs(0, 3);
            INST(5, Opcode::Add).u64().Inputs(2, 1);
            INST(20, Opcode::SaveState).NoVregs();
            INST(7, Opcode::CallStatic).u64().InputsAutoType(4, 5, 20);
            INST(6, Opcode::Return).u64().Inputs(7);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckUsers(INS(0), {2, 4, 5}));
    ASSERT_TRUE(CheckUsers(INS(1), {3, 4, 5}));

    ASSERT_TRUE(INS(2).GetUsers().Empty());
    ASSERT_TRUE(INS(3).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(4), {0, 1}));
    ASSERT_TRUE(INS(4).GetOpcode() == Opcode::Sub);

    ASSERT_TRUE(CheckInputs(INS(5), {1, 0}));
    ASSERT_TRUE(INS(5).GetOpcode() == Opcode::Sub);
}

TEST_F(PeepholesTest, SameAddAndSubTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).u64().Inputs(0, 1);
            INST(3, Opcode::Add).u64().Inputs(2, 1);
            INST(4, Opcode::Add).u64().Inputs(1, 2);
            INST(5, Opcode::Add).u64().Inputs(3, 4);

            INST(6, Opcode::Add).u32().Inputs(1, 2);
            INST(7, Opcode::Add).u32().Inputs(2, 1);
            INST(8, Opcode::Add).u32().Inputs(6, 7);
            INST(20, Opcode::SaveState).NoVregs();
            INST(10, Opcode::CallStatic).u64().InputsAutoType(5, 8, 20);
            INST(9, Opcode::Return).u64().Inputs(10);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3).GetUsers().Empty());
    ASSERT_TRUE(INS(4).GetUsers().Empty());
    ASSERT_TRUE(CheckInputs(INS(5), {0, 0}));

    ASSERT_TRUE(CheckInputs(INS(6), {1, 2}));
    ASSERT_TRUE(CheckInputs(INS(7), {2, 1}));
    ASSERT_TRUE(CheckInputs(INS(8), {6, 7}));
}

// (a - b) + (c + b) -> a + c
TEST_F(PeepholesTest, AddSubAndAddSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(0, 1);
            INST(4, Opcode::Add).u64().Inputs(2, 1);
            INST(5, Opcode::Add).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5), {0, 2}));
}

// (a - b) + (b + c) -> a + c
TEST_F(PeepholesTest, AddSubAndAddSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(0, 1);
            INST(4, Opcode::Add).u64().Inputs(1, 2);
            INST(5, Opcode::Add).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5), {0, 2}));
}

// (a + b) + (c - b) -> c + a
TEST_F(PeepholesTest, AddAddAndSubSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(2, 1);
            INST(4, Opcode::Add).u64().Inputs(0, 1);
            INST(5, Opcode::Add).u64().Inputs(4, 3);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5), {2, 0}));
}

// (b + a) + (c - b) -> c + a
TEST_F(PeepholesTest, AddAddAndSubSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(2, 1);
            INST(4, Opcode::Add).u64().Inputs(1, 0);
            INST(5, Opcode::Add).u64().Inputs(4, 3);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5), {2, 0}));
}

// (a - b) + (b - c) -> a - c
TEST_F(PeepholesTest, AddSubAndSubSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(0, 1);
            INST(4, Opcode::Sub).u64().Inputs(1, 2);
            INST(5, Opcode::Add).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5), {0, 2}));
}

// (a - b) + (c - a) -> c - b
TEST_F(PeepholesTest, AddSubAndSubSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(0, 1);
            INST(4, Opcode::Sub).u64().Inputs(2, 0);
            INST(5, Opcode::Add).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5), {2, 1}));
}

// a + (0 - b) -> a - b
TEST_F(PeepholesTest, AddSubLeftZeroOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        PARAMETER(2, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(1, 2);
            INST(4, Opcode::Add).u64().Inputs(0, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(4), {0, 2}));
}

// (0 - a) + b -> b - a
TEST_F(PeepholesTest, AddSubLeftZeroOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        PARAMETER(2, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Sub).u64().Inputs(1, 0);
            INST(4, Opcode::Add).u64().Inputs(3, 2);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(4), {2, 0}));
}

// (x + a) - (x + b) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 1);
            INST(4, Opcode::Add).u64().Inputs(0, 2);
            INST(5, Opcode::Sub).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5), {1, 2}));
}

// (a + x) - (x + b) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(1, 0);
            INST(4, Opcode::Add).u64().Inputs(0, 2);
            INST(5, Opcode::Sub).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5), {1, 2}));
}

// (a + x) - (b + x) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(1, 0);
            INST(4, Opcode::Add).u64().Inputs(2, 0);
            INST(5, Opcode::Sub).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5), {1, 2}));
}

// (x + a) - (b + x) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest4)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 1);
            INST(4, Opcode::Add).u64().Inputs(2, 0);
            INST(5, Opcode::Sub).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5), {1, 2}));
}

TEST_F(PeepholesTest, SubZeroConstantTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(3, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Sub).u64().Inputs(0, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(9, Opcode::CallStatic).s32().InputsAutoType(6, 20);
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(6).GetUsers().Empty());
    ASSERT_TRUE(INS(9).GetInput(0).GetInst() == &INS(0));
}

TEST_F(PeepholesTest, SubFromZeroConstantTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).s64().Inputs(1, 0);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).s64().Inputs(0);
            INST(3, Opcode::Return).s64().Inputs(2);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, SubFromZeroConstantFPTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).f64();
        CONSTANT(1, 0.0);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).f64().Inputs(1, 0);
            INST(3, Opcode::Return).f64().Inputs(2);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
}

TEST_F(PeepholesTest, SubConstantTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 1);
            INST(4, Opcode::Sub).u64().Inputs(0, 1);

            INST(5, Opcode::Sub).u64().Inputs(3, 2);
            INST(6, Opcode::Sub).u64().Inputs(4, 2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).u64().InputsAutoType(5, 6, 20);
            INST(7, Opcode::Return).u64().Inputs(8);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    auto const3 = static_cast<ConstantInst *>(INS(2).GetNext());
    ASSERT_TRUE(const3 != nullptr);
    auto const4 = static_cast<ConstantInst *>(const3->GetNext());
    ASSERT_TRUE(const4 != nullptr && const4->GetNext() == nullptr);
    ASSERT_TRUE(const3->IsEqualConst(1));
    ASSERT_TRUE(const4->IsEqualConst(11));

    ASSERT_TRUE(INS(5).GetInput(0).GetInst() == &INS(0) && INS(5).GetInput(1).GetInst() == const3);
    ASSERT_TRUE(INS(6).GetInput(0).GetInst() == &INS(0) && INS(6).GetInput(1).GetInst() == const4);

    ASSERT_TRUE(CheckUsers(INS(0), {3, 4, 5, 6}));
    ASSERT_TRUE(CheckUsers(INS(1), {3, 4}));
    ASSERT_TRUE(INS(2).GetUsers().Empty());
    ASSERT_TRUE(CheckUsers(*const3, {5}));
    ASSERT_TRUE(CheckUsers(*const4, {6}));
}

TEST_F(PeepholesTest, SubConstantTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u64().Inputs(0, 1);
            INST(4, Opcode::Sub).u64().Inputs(0, 1);

            INST(5, Opcode::Sub).u32().Inputs(3, 2);
            INST(6, Opcode::Sub).u32().Inputs(4, 2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).u64().InputsAutoType(5, 6, 20);
            INST(7, Opcode::Return).u64().Inputs(8);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(2).GetNext() == nullptr);

    ASSERT_TRUE(CheckInputs(INS(3), {0, 1}));
    ASSERT_TRUE(CheckInputs(INS(4), {0, 1}));
    ASSERT_TRUE(CheckInputs(INS(5), {3, 2}));
    ASSERT_TRUE(CheckInputs(INS(6), {4, 2}));

    ASSERT_TRUE(CheckUsers(INS(0), {3, 4}));
    ASSERT_TRUE(CheckUsers(INS(1), {3, 4}));
    ASSERT_TRUE(CheckUsers(INS(2), {5, 6}));
    ASSERT_TRUE(CheckUsers(INS(3), {5}));
    ASSERT_TRUE(CheckUsers(INS(4), {6}));
}

TEST_F(PeepholesTest, SubNegTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Neg).u64().Inputs(0);
            INST(3, Opcode::Neg).u64().Inputs(1);

            INST(4, Opcode::Sub).u64().Inputs(0, 3);
            INST(5, Opcode::Sub).u64().Inputs(2, 0);
            INST(6, Opcode::Sub).u64().Inputs(2, 3);
            INST(7, Opcode::Sub).u64().Inputs(3, 2);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).u64().InputsAutoType(4, 5, 6, 7, 20);
            INST(9, Opcode::Return).u64().Inputs(8);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckInputs(INS(4), {0, 1}));
    ASSERT_TRUE(INS(4).GetOpcode() == Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5), {2, 0}));
    ASSERT_TRUE(INS(5).GetOpcode() == Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(6), {1, 0}));
    ASSERT_TRUE(INS(6).GetOpcode() == Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(7), {0, 1}));
    ASSERT_TRUE(INS(7).GetOpcode() == Opcode::Sub);
}

TEST_F(PeepholesTest, SameSubAndAddTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);

            INST(4, Opcode::Sub).u64().Inputs(2, 0);
            INST(5, Opcode::Sub).u64().Inputs(2, 1);
            INST(6, Opcode::Add).u64().Inputs(4, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());

    ASSERT_TRUE(INS(4).GetUsers().Empty());
    ASSERT_TRUE(INS(5).GetUsers().Empty());
    ASSERT_TRUE(CheckInputs(INS(6), {1, 0}));
}

TEST_F(PeepholesTest, SameSubAndAddTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).u64().Inputs(0, 1);
            INST(3, Opcode::Sub).u64().Inputs(1, 0);

            INST(4, Opcode::Sub).u64().Inputs(0, 2);
            INST(5, Opcode::Sub).u64().Inputs(3, 1);
            INST(6, Opcode::Add).u64().Inputs(4, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(4).GetUsers().Empty());
    ASSERT_TRUE(CheckUsers(INS(5), {6}));
    ASSERT_TRUE(CheckInputs(INS(6), {1, 5}));
}

TEST_F(PeepholesTest, TestOr2_1)
{
    // Case 1 + Case 2
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        CONSTANT(1, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Or).u64().Inputs(0, 1);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(1).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestOr3)
{
    // case 3:
    // 1.i64 OR v5, v5
    // 2.i64 INS which use v1
    // ===>
    // 1.i64 OR v5, v5
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Or).u64().Inputs(0, 0);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(0));
}

TEST_F(PeepholesTest, TestOr4)
{
    // case 4: De Morgan rules
    // 2.i64 not v0 -> {4}
    // 3.i64 not v1 -> {4}
    // 4.i64 OR v2, v3
    // ===>
    // 5.i64 AND v0, v1
    // 6.i64 not v5
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Not).u64().Inputs(0);
            INST(3, Opcode::Not).u64().Inputs(1);
            INST(4, Opcode::Or).u64().Inputs(2, 3);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    auto not_inst = INS(5).GetInput(0).GetInst();
    ASSERT_EQ(not_inst->GetOpcode(), Opcode::Not);
    auto and_inst = not_inst->GetInput(0).GetInst();
    ASSERT_EQ(and_inst->GetOpcode(), Opcode::And);
    ASSERT_EQ(and_inst->GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(and_inst->GetInput(1).GetInst(), &INS(1));
}

TEST_F(PeepholesTest, TestOr4_1)
{
    // Case 4, but NOT-inst have more than 1 user
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Not).u64().Inputs(0);
            INST(3, Opcode::Not).u64().Inputs(1);
            INST(4, Opcode::Or).u64().Inputs(2, 3);
            INST(5, Opcode::Not).u64().Inputs(2);
            INST(6, Opcode::Not).u64().Inputs(3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).u64().InputsAutoType(4, 5, 6, 20);
            INST(7, Opcode::Return).u64().Inputs(8);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto or_inst = INS(8).GetInput(0).GetInst();
    ASSERT_EQ(or_inst->GetOpcode(), Opcode::Or);
}

TEST_F(PeepholesTest, NegTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Neg).s64().Inputs(0);
            INST(5, Opcode::Neg).s64().Inputs(4);
            INST(6, Opcode::Add).s64().Inputs(1, 5);
            INST(20, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).u32().InputsAutoType(6, 20);
            INST(10, Opcode::Return).u32().Inputs(11);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckInputs(INS(5), {4}));
    ASSERT_TRUE(INS(5).GetUsers().Empty());
    ASSERT_TRUE(CheckInputs(INS(6), {1, 0}));
    ASSERT_EQ(INS(6).GetOpcode(), Opcode::Add);
}

TEST_F(PeepholesTest, NegTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).s64().Inputs(0, 1);
            INST(3, Opcode::Neg).s64().Inputs(2);
            INST(4, Opcode::Return).s64().Inputs(3);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3).GetUsers().Empty());
    ASSERT_NE(INS(3).GetNext(), &INS(4));
    auto sub = INS(3).GetNext();
    ASSERT_EQ(sub->GetOpcode(), Opcode::Sub);
    ASSERT_EQ(sub->GetType(), INS(3).GetType());

    ASSERT_EQ(sub->GetInput(0).GetInst(), &INS(1));
    ASSERT_EQ(sub->GetInput(1).GetInst(), &INS(0));

    ASSERT_EQ(INS(4).GetInput(0).GetInst(), sub);
}

// Checking the shift with zero constant
TEST_F(PeepholesTest, ShlZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shl).u64().Inputs(0, 1);
            INST(4, Opcode::Shl).u64().Inputs(0, 2);

            INST(5, Opcode::Add).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(5), {0, 4}));
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Add);
}

// Checking repeated shifts for constants with the same types
TEST_F(PeepholesTest, ShlRepeatConstTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shl).u64().Inputs(0, 1);
            INST(4, Opcode::Shl).u64().Inputs(3, 2);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(2).GetNext(), nullptr);
    auto const3 = static_cast<ConstantInst *>(INS(2).GetNext());
    ASSERT_TRUE(const3->IsEqualConst(11));

    ASSERT_TRUE(INS(3).GetUsers().Empty());
    ASSERT_EQ(INS(3).GetOpcode(), Opcode::Shl);
    ASSERT_TRUE(CheckUsers(INS(4), {5}));
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::Shl);
    ASSERT_EQ(INS(4).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(4).GetInput(1).GetInst(), const3);

    ASSERT_TRUE(CheckInputs(INS(5), {4}));
}

// Checking repeated shifts for constants with different types
TEST_F(PeepholesTest, ShlRepeatConstTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shl).u16().Inputs(0, 1);
            INST(4, Opcode::Shl).u32().Inputs(3, 2);
            INST(5, Opcode::Return).u32().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(2).GetNext(), nullptr);

    ASSERT_TRUE(CheckInputs(INS(3), {0, 1}));
    ASSERT_TRUE(CheckUsers(INS(3), {4}));
    ASSERT_EQ(INS(3).GetOpcode(), Opcode::Shl);
    ASSERT_TRUE(CheckInputs(INS(4), {3, 2}));
    ASSERT_TRUE(CheckUsers(INS(4), {5}));
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::Shl);

    ASSERT_TRUE(CheckInputs(INS(5), {4}));
}

// Checking the shift for a constant greater than the type size
TEST_F(PeepholesTest, ShlBigConstTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u32();
        PARAMETER(2, 2).u16();
        PARAMETER(3, 3).u8();
        CONSTANT(4, 127);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::Shl).u64().Inputs(0, 4);
            INST(6, Opcode::Shl).u32().Inputs(1, 4);
            INST(7, Opcode::Shl).u16().Inputs(2, 4);
            INST(8, Opcode::Shl).u8().Inputs(3, 4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(10, Opcode::CallStatic).s64().InputsAutoType(5, 6, 7, 8, 20);
            INST(9, Opcode::Return).s64().Inputs(10);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(4).GetNext(), nullptr);
    auto const64 = static_cast<ConstantInst *>(INS(4).GetNext());
    ASSERT_TRUE(const64->IsEqualConst(63));

    ASSERT_NE(const64->GetNext(), nullptr);
    auto const32 = static_cast<ConstantInst *>(const64->GetNext());
    ASSERT_TRUE(const32->IsEqualConst(31));

    ASSERT_NE(const32->GetNext(), nullptr);
    auto const16 = static_cast<ConstantInst *>(const32->GetNext());
    ASSERT_TRUE(const16->IsEqualConst(15));

    ASSERT_NE(const16->GetNext(), nullptr);
    auto const8 = static_cast<ConstantInst *>(const16->GetNext());
    ASSERT_TRUE(const8->IsEqualConst(7));

    ASSERT_EQ(INS(5).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(5).GetInput(1).GetInst(), const64);
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Shl);

    ASSERT_EQ(INS(6).GetInput(0).GetInst(), &INS(1));
    ASSERT_EQ(INS(6).GetInput(1).GetInst(), const32);
    ASSERT_EQ(INS(6).GetOpcode(), Opcode::Shl);

    ASSERT_EQ(INS(7).GetInput(0).GetInst(), &INS(2));
    ASSERT_EQ(INS(7).GetInput(1).GetInst(), const16);
    ASSERT_EQ(INS(7).GetOpcode(), Opcode::Shl);

    ASSERT_EQ(INS(8).GetInput(0).GetInst(), &INS(3));
    ASSERT_EQ(INS(8).GetInput(1).GetInst(), const8);
    ASSERT_EQ(INS(8).GetOpcode(), Opcode::Shl);
}

TEST_F(PeepholesTest, ShlPlusShrTest)
{
    // applied
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(5, 0xff);

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::And).s32().Inputs(0, 5);
            INST(4, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusShrTest1)
{
    // not applied, different constants
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);
        CONSTANT(5, 0x10);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 5);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);
        CONSTANT(5, 0x10);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 5);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusShrTest2)
{
    // not applied, same inputs but not a constant
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checking the shift with zero constant
TEST_F(PeepholesTest, ShrZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shr).u64().Inputs(0, 1);
            INST(4, Opcode::Shr).u64().Inputs(0, 2);

            INST(5, Opcode::Add).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(5), {0, 4}));
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Add);
}

// Checking repeated shifts for constants with the same types
TEST_F(PeepholesTest, ShrRepeatConstTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shr).u64().Inputs(0, 1);
            INST(4, Opcode::Shr).u64().Inputs(3, 2);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(2).GetNext(), nullptr);
    auto const3 = static_cast<ConstantInst *>(INS(2).GetNext());
    ASSERT_TRUE(const3->IsEqualConst(11));

    ASSERT_TRUE(INS(3).GetUsers().Empty());
    ASSERT_EQ(INS(3).GetOpcode(), Opcode::Shr);
    ASSERT_TRUE(CheckUsers(INS(4), {5}));
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::Shr);
    ASSERT_EQ(INS(4).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(4).GetInput(1).GetInst(), const3);

    ASSERT_TRUE(CheckInputs(INS(5), {4}));
}

// Checking repeated shifts for constants with different types
TEST_F(PeepholesTest, ShrRepeatConstTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shr).u16().Inputs(0, 1);
            INST(4, Opcode::Shr).u32().Inputs(3, 2);
            INST(5, Opcode::Return).u32().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(2).GetNext(), nullptr);

    ASSERT_TRUE(CheckInputs(INS(3), {0, 1}));
    ASSERT_TRUE(CheckUsers(INS(3), {4}));
    ASSERT_EQ(INS(3).GetOpcode(), Opcode::Shr);
    ASSERT_TRUE(CheckInputs(INS(4), {3, 2}));
    ASSERT_TRUE(CheckUsers(INS(4), {5}));
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::Shr);

    ASSERT_TRUE(CheckInputs(INS(5), {4}));
}

// Checking the shift for a constant greater than the type size
TEST_F(PeepholesTest, ShrBigConstTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u32();
        PARAMETER(2, 2).u16();
        PARAMETER(3, 3).u8();
        CONSTANT(4, 127);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::Shr).u64().Inputs(0, 4);
            INST(6, Opcode::Shr).u32().Inputs(1, 4);
            INST(7, Opcode::Shr).u16().Inputs(2, 4);
            INST(8, Opcode::Shr).u8().Inputs(3, 4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(9, Opcode::CallStatic).u64().InputsAutoType(5, 6, 7, 8, 20);
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(4).GetNext(), nullptr);
    auto const64 = static_cast<ConstantInst *>(INS(4).GetNext());
    ASSERT_TRUE(const64->IsEqualConst(63));

    ASSERT_NE(const64->GetNext(), nullptr);
    auto const32 = static_cast<ConstantInst *>(const64->GetNext());
    ASSERT_TRUE(const32->IsEqualConst(31));

    ASSERT_NE(const32->GetNext(), nullptr);
    auto const16 = static_cast<ConstantInst *>(const32->GetNext());
    ASSERT_TRUE(const16->IsEqualConst(15));

    ASSERT_NE(const16->GetNext(), nullptr);
    auto const8 = static_cast<ConstantInst *>(const16->GetNext());
    ASSERT_TRUE(const8->IsEqualConst(7));

    ASSERT_EQ(INS(5).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(5).GetInput(1).GetInst(), const64);
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Shr);

    ASSERT_EQ(INS(6).GetInput(0).GetInst(), &INS(1));
    ASSERT_EQ(INS(6).GetInput(1).GetInst(), const32);
    ASSERT_EQ(INS(6).GetOpcode(), Opcode::Shr);

    ASSERT_EQ(INS(7).GetInput(0).GetInst(), &INS(2));
    ASSERT_EQ(INS(7).GetInput(1).GetInst(), const16);
    ASSERT_EQ(INS(7).GetOpcode(), Opcode::Shr);

    ASSERT_EQ(INS(8).GetInput(0).GetInst(), &INS(3));
    ASSERT_EQ(INS(8).GetInput(1).GetInst(), const8);
    ASSERT_EQ(INS(8).GetOpcode(), Opcode::Shr);
}

// Checking the shift with zero constant
TEST_F(PeepholesTest, AShrZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);
        CONSTANT(2, 1);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::AShr).u64().Inputs(0, 1);
            INST(4, Opcode::AShr).u64().Inputs(0, 2);

            INST(5, Opcode::Add).u64().Inputs(3, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(5), {0, 4}));
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::Add);
}

// Checking repeated shifts for constants with the same types
TEST_F(PeepholesTest, AShrRepeatConstTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::AShr).u64().Inputs(0, 1);
            INST(4, Opcode::AShr).u64().Inputs(3, 2);
            INST(5, Opcode::Return).u64().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(2).GetNext(), nullptr);
    auto const3 = static_cast<ConstantInst *>(INS(2).GetNext());
    ASSERT_TRUE(const3->IsEqualConst(11));

    ASSERT_TRUE(INS(3).GetUsers().Empty());
    ASSERT_EQ(INS(3).GetOpcode(), Opcode::AShr);
    ASSERT_TRUE(CheckUsers(INS(4), {5}));
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::AShr);
    ASSERT_EQ(INS(4).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(4).GetInput(1).GetInst(), const3);

    ASSERT_TRUE(CheckInputs(INS(5), {4}));
}

// Checking repeated shifts for constants with different types
TEST_F(PeepholesTest, AShrRepeatConstTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        CONSTANT(1, 5);
        CONSTANT(2, 6);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::AShr).u16().Inputs(0, 1);
            INST(4, Opcode::AShr).u32().Inputs(3, 2);
            INST(5, Opcode::Return).u32().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(2).GetNext(), nullptr);

    ASSERT_TRUE(CheckInputs(INS(3), {0, 1}));
    ASSERT_TRUE(CheckUsers(INS(3), {4}));
    ASSERT_EQ(INS(3).GetOpcode(), Opcode::AShr);
    ASSERT_TRUE(CheckInputs(INS(4), {3, 2}));
    ASSERT_TRUE(CheckUsers(INS(4), {5}));
    ASSERT_EQ(INS(4).GetOpcode(), Opcode::AShr);

    ASSERT_TRUE(CheckInputs(INS(5), {4}));
}

// Checking the shift for a constant greater than the type size
TEST_F(PeepholesTest, AShrBigConstTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u32();
        PARAMETER(2, 2).u16();
        PARAMETER(3, 3).u8();
        CONSTANT(4, 127);

        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::AShr).u64().Inputs(0, 4);
            INST(6, Opcode::AShr).u32().Inputs(1, 4);
            INST(7, Opcode::AShr).u16().Inputs(2, 4);
            INST(8, Opcode::AShr).u8().Inputs(3, 4);
            INST(20, Opcode::SaveState).NoVregs();
            INST(10, Opcode::CallStatic).u64().InputsAutoType(5, 6, 7, 8, 20);
            INST(9, Opcode::Return).u64().Inputs(10);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(4).GetNext(), nullptr);
    auto const64 = static_cast<ConstantInst *>(INS(4).GetNext());
    ASSERT_TRUE(const64->IsEqualConst(63));

    ASSERT_NE(const64->GetNext(), nullptr);
    auto const32 = static_cast<ConstantInst *>(const64->GetNext());
    ASSERT_TRUE(const32->IsEqualConst(31));

    ASSERT_NE(const32->GetNext(), nullptr);
    auto const16 = static_cast<ConstantInst *>(const32->GetNext());
    ASSERT_TRUE(const16->IsEqualConst(15));

    ASSERT_NE(const16->GetNext(), nullptr);
    auto const8 = static_cast<ConstantInst *>(const16->GetNext());
    ASSERT_TRUE(const8->IsEqualConst(7));

    ASSERT_EQ(INS(5).GetInput(0).GetInst(), &INS(0));
    ASSERT_EQ(INS(5).GetInput(1).GetInst(), const64);
    ASSERT_EQ(INS(5).GetOpcode(), Opcode::AShr);

    ASSERT_EQ(INS(6).GetInput(0).GetInst(), &INS(1));
    ASSERT_EQ(INS(6).GetInput(1).GetInst(), const32);
    ASSERT_EQ(INS(6).GetOpcode(), Opcode::AShr);

    ASSERT_EQ(INS(7).GetInput(0).GetInst(), &INS(2));
    ASSERT_EQ(INS(7).GetInput(1).GetInst(), const16);
    ASSERT_EQ(INS(7).GetOpcode(), Opcode::AShr);

    ASSERT_EQ(INS(8).GetInput(0).GetInst(), &INS(3));
    ASSERT_EQ(INS(8).GetInput(1).GetInst(), const8);
    ASSERT_EQ(INS(8).GetOpcode(), Opcode::AShr);
}

TEST_F(PeepholesTest, ShlPlusAShrTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::AShr).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Cast).s8().SrcType(DataType::INT32).Inputs(0);
            INST(4, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x10);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::AShr).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Cast).s16().SrcType(DataType::INT32).Inputs(0);
            INST(4, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        CONSTANT(1, 0x8);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s64().Inputs(0, 1);
            INST(3, Opcode::AShr).s64().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(0);
            INST(4, Opcode::Return).s32().Inputs(6);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest4)
{
    // not applied, different constants
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);
        CONSTANT(5, 0x10);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::AShr).s32().Inputs(2, 5);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);
        CONSTANT(5, 0x10);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::AShr).s32().Inputs(2, 5);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest5)
{
    // not applied, same inputs but not a constant
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestMulCase1)
{
    // case 1:
    // 0. Const 1
    // 1. MUL v5, v0 -> {v2, ...}
    // 2. INS v1
    // ===>
    // 0. Const 1
    // 1. MUL v5, v0
    // 2. INS v5
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(3, 1);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Mul).u64().Inputs(0, 3);
            INST(12, Opcode::Mul).u16().Inputs(0, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).u64().InputsAutoType(6, 12, 20);
            INST(14, Opcode::Return).u64().Inputs(21);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(21).GetInput(0).GetInst(), &INS(0));
    ASSERT_TRUE(INS(6).GetUsers().Empty());

    ASSERT_EQ(INS(21).GetInput(1).GetInst(), &INS(0));
    ASSERT_TRUE(INS(12).GetUsers().Empty());
}

TEST_F(PeepholesTest, TestMulCase2)
{
    // case 2:
    // 0. Const -1
    // 1. MUL v5, v0
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. MUL v5, v0
    // 3. NEG v5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(3, -1);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Mul).u64().Inputs(0, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).u64().InputsAutoType(6, 20);
            INST(12, Opcode::Return).u64().Inputs(13);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Neg).u64().Inputs(0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).u64().InputsAutoType(6, 20);
            INST(12, Opcode::Return).u64().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestMulCase3)
{
    // case 3:
    // 0. Const 2
    // 1. MUL v5, v0
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. MUL v5, v0
    // 3. ADD v5 , V5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(3, 2);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Mul).u64().Inputs(0, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).u64().InputsAutoType(6, 20);
            INST(12, Opcode::Return).u64().Inputs(13);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).u64().InputsAutoType(6, 20);
            INST(12, Opcode::Return).u64().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestMulCase4)
{
    // case 4:
    // 0. Const 2^N
    // 1. MUL v5, v0
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. MUL v5, v0
    // 3. SHL v5 , N -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).u16();
        PARAMETER(13, 3).f32();
        PARAMETER(14, 4).f64();
        CONSTANT(3, 4);
        CONSTANT(4, 16);
        CONSTANT(5, 512);
        CONSTANT(15, 4.0F);
        CONSTANT(16, 16.0);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Mul).u64().Inputs(0, 3);
            INST(8, Opcode::Mul).s64().Inputs(1, 4);
            INST(10, Opcode::Mul).u16().Inputs(2, 5);
            INST(17, Opcode::Mul).f32().Inputs(13, 15);
            INST(19, Opcode::Mul).f64().Inputs(14, 16);
            INST(20, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).u64().InputsAutoType(6, 8, 10, 17, 19, 20);
            INST(22, Opcode::Return).u64().Inputs(21);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).u16();
        PARAMETER(13, 3).f32();
        PARAMETER(14, 4).f64();
        CONSTANT(3, 4);
        CONSTANT(15, 4.0F);
        CONSTANT(16, 16.0);
        CONSTANT(23, 2);
        CONSTANT(24, 9);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Shl).u64().Inputs(0, 23);
            INST(8, Opcode::Shl).s64().Inputs(1, 3);
            INST(10, Opcode::Shl).u16().Inputs(2, 24);
            INST(17, Opcode::Mul).f32().Inputs(13, 15);
            INST(19, Opcode::Mul).f64().Inputs(14, 16);
            INST(20, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).u64().InputsAutoType(6, 8, 10, 17, 19, 20);
            INST(22, Opcode::Return).u64().Inputs(21);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestDivCase1)
{
    // case 1:
    // 0. Const 1
    // 1. DIV v5, v0 -> {v2, ...}
    // 2. INS v1
    // ===>
    // 0. Const 1
    // 1. DIV v5, v0
    // 3. MOV v5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(3, 1);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Div).u64().Inputs(0, 3);
            INST(12, Opcode::Div).u16().Inputs(0, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).u64().InputsAutoType(6, 12, 20);
            INST(22, Opcode::Return).u64().Inputs(21);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();

    ASSERT_EQ(INS(21).GetInput(0).GetInst(), &INS(0));
    ASSERT_TRUE(INS(6).GetUsers().Empty());

    ASSERT_EQ(INS(21).GetInput(1).GetInst(), &INS(0));
    ASSERT_TRUE(INS(12).GetUsers().Empty());
}

TEST_F(PeepholesTest, TestDivCase2)
{
    // case 2:
    // 0. Const -1
    // 1. DIV v5, v0 -> {v2, ...}
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. DIV v5, v0
    // 3. NEG v5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(3, -1);
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Div).u64().Inputs(0, 3);
            INST(20, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).u64().InputsAutoType(6, 20);
            INST(22, Opcode::Return).u64().Inputs(21);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Neg).u64().Inputs(0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).u64().InputsAutoType(6, 20);
            INST(12, Opcode::Return).u64().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestDivCase3Unsigned)
{
    for (auto type : {DataType::UINT8, DataType::UINT16, DataType::UINT32, DataType::UINT64}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(1, 4);
            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Div).Inputs(0, 1);
                INS(2).SetType(type);
                INST(3, Opcode::Return).Inputs(2);
                INS(3).SetType(type);
            }
        }
        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(1, 2);
            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Shr).Inputs(0, 1);
                INS(2).SetType(type);
                INST(3, Opcode::Return).Inputs(2);
                INS(3).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, TestDivCase3SignedPositive)
{
    for (auto type : {DataType::INT8, DataType::INT16, DataType::INT32, DataType::INT64}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(1, 4);
            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Div).Inputs(0, 1);
                INS(2).SetType(type);
                INST(3, Opcode::Return).Inputs(2);
                INS(3).SetType(type);
            }
        }

        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();

        auto type_size = DataType::GetTypeSize(type, graph1->GetArch());
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(1, type_size - 1);
            CONSTANT(5, type_size - 2);  // type size - log2(4)
            CONSTANT(8, 2);              // log2(4)
            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::AShr).Inputs(0, 1);
                INS(2).SetType(type);
                INST(4, Opcode::Shr).Inputs(2, 5);
                INS(4).SetType(type);
                INST(6, Opcode::Add).Inputs(4, 0);
                INS(6).SetType(type);
                INST(7, Opcode::AShr).Inputs(6, 8);
                INS(7).SetType(type);
                INST(3, Opcode::Return).Inputs(7);
                INS(3).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, TestDivCase3SignedNegative)
{
    for (auto type : {DataType::INT8, DataType::INT16, DataType::INT32, DataType::INT64}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(1, -16);
            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Div).Inputs(0, 1);
                INS(2).SetType(type);
                INST(3, Opcode::Return).Inputs(2);
                INS(3).SetType(type);
            }
        }

        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();

        auto type_size = DataType::GetTypeSize(type, graph1->GetArch());
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(1, type_size - 1);
            CONSTANT(5, type_size - 4);  // type size - log2(16)
            CONSTANT(8, 4);              // log2(16)
            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::AShr).Inputs(0, 1);
                INS(2).SetType(type);
                INST(4, Opcode::Shr).Inputs(2, 5);
                INS(4).SetType(type);
                INST(6, Opcode::Add).Inputs(4, 0);
                INS(6).SetType(type);
                INST(7, Opcode::AShr).Inputs(6, 8);
                INS(7).SetType(type);
                INST(9, Opcode::Neg).Inputs(7);
                INS(9).SetType(type);
                INST(3, Opcode::Return).Inputs(9);
                INS(3).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, TestLenArray1)
{
    // 1. .... ->{v2}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2 ->{v4, v5...}
    // ===>
    // 1. .... ->{v2, v4, v5, ...}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(1, Opcode::NewArray).ref().Inputs(44, 0);
            INST(2, Opcode::LenArray).s32().Inputs(1);
            INST(3, Opcode::Return).s32().Inputs(2);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto cnst = INS(3).GetInput(0).GetInst();
    ASSERT_EQ(cnst->GetOpcode(), Opcode::Constant);
    ASSERT_TRUE(INS(2).GetUsers().Empty());
}

TEST_F(PeepholesTest, TestLenArray2)
{
    // 1. .... ->{v2}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2 ->{v4, v5...}
    // ===>
    // 1. .... ->{v2, v4, v5, ...}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2
    GRAPH(GetGraph())
    {
        CONSTANT(0, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(10, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(1, Opcode::NegativeCheck).s32().Inputs(0, 10);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(10).TypeId(68);
            INST(2, Opcode::NewArray).ref().Inputs(44, 1);
            INST(3, Opcode::NullCheck).ref().Inputs(2, 10);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto cnst = INS(5).GetInput(0).GetInst();
    ASSERT_EQ(cnst->GetOpcode(), Opcode::Constant);
    ASSERT_TRUE(INS(4).GetUsers().Empty());
}

TEST_F(PeepholesTest, TestPhi1)
{
    // Users isn't intersect
    // 2.type  Phi v0, v1 -> v4
    // 3.type  Phi v0, v1 -> v5
    // ===>
    // 2.type  Phi v0, v1 -> v4, v5
    // 3.type  Phi v0, v1
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Sub).s32().Inputs(1, 0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(9, Opcode::Phi).s32().Inputs(5, 7);
            INST(10, Opcode::Phi).s32().Inputs(5, 7);
            INST(11, Opcode::Add).s32().Inputs(9, 0);
            INST(12, Opcode::Add).s32().Inputs(10, 1);
            INST(13, Opcode::Add).s32().Inputs(11, 12);
            INST(14, Opcode::Return).s32().Inputs(13);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Sub).s32().Inputs(1, 0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(9, Opcode::Phi).s32().Inputs(5, 7);
            INST(10, Opcode::Phi).s32().Inputs(5, 7);
            INST(11, Opcode::Add).s32().Inputs(9, 0);
            INST(12, Opcode::Add).s32().Inputs(9, 1);
            INST(13, Opcode::Add).s32().Inputs(11, 12);
            INST(14, Opcode::Return).s32().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestPhi2)
{
    // Users is intersect
    // 2.type  Phi v0, v1 -> v4, v5
    // 3.type  Phi v0, v1 -> v5, v6
    // ===>
    // 2.type  Phi v0, v1 -> v4, v5
    // 3.type  Phi v0, v1
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Sub).s32().Inputs(1, 0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(9, Opcode::Phi).s32().Inputs(5, 7);
            INST(10, Opcode::Phi).s32().Inputs(5, 7);
            INST(11, Opcode::Add).s32().Inputs(9, 0);
            INST(12, Opcode::Add).s32().Inputs(9, 10);
            INST(13, Opcode::Add).s32().Inputs(10, 1);
            INST(14, Opcode::Add).s32().Inputs(11, 12);
            INST(15, Opcode::Add).s32().Inputs(13, 14);
            INST(20, Opcode::SaveState).NoVregs();
            INST(16, Opcode::CallStatic).v0id().InputsAutoType(9, 10, 15, 20);
            ;
            INST(17, Opcode::Return).s32().Inputs(15);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(7, Opcode::Sub).s32().Inputs(1, 0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(9, Opcode::Phi).s32().Inputs(5, 7);
            INST(10, Opcode::Phi).s32().Inputs(5, 7);
            INST(11, Opcode::Add).s32().Inputs(9, 0);
            INST(12, Opcode::Add).s32().Inputs(9, 9);
            INST(13, Opcode::Add).s32().Inputs(9, 1);
            INST(14, Opcode::Add).s32().Inputs(11, 12);
            INST(15, Opcode::Add).s32().Inputs(13, 14);
            INST(20, Opcode::SaveState).NoVregs();
            INST(16, Opcode::CallStatic).v0id().InputsAutoType(9, 9, 15, 20);
            INST(17, Opcode::Return).s32().Inputs(15);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestPhi3)
{
    // Peephole rule isn't applied.
    // Same types, different inputs.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);  // Phi 11
            INST(6, Opcode::Add).s32().Inputs(0, 1);  // Phi 12
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Sub).s32().Inputs(1, 0);  // Phi 11
            INST(9, Opcode::Add).s32().Inputs(0, 1);  // Phi 12
        }
        BASIC_BLOCK(5, 1)
        {
            INST(11, Opcode::Phi).s32().Inputs(5, 8);
            INST(12, Opcode::Phi).s32().Inputs(6, 9);
            INST(13, Opcode::Add).s32().Inputs(11, 12);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(11, 12, 13, 20);
            INST(15, Opcode::Return).s32().Inputs(13);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);  // Phi 11
            INST(6, Opcode::Add).s32().Inputs(0, 1);  // Phi 12
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Sub).s32().Inputs(1, 0);  // Phi 11
            INST(9, Opcode::Add).s32().Inputs(0, 1);  // Phi 12
        }
        BASIC_BLOCK(5, 1)
        {
            INST(11, Opcode::Phi).s32().Inputs(5, 8);
            INST(12, Opcode::Phi).s32().Inputs(6, 9);
            INST(13, Opcode::Add).s32().Inputs(11, 12);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(11, 12, 13, 20);
            INST(15, Opcode::Return).s32().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestPhi4)
{
    // Peephole rule isn't applied.
    // Different types, same inputs.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Sub).s32().Inputs(1, 0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(11, Opcode::Phi).s16().Inputs(5, 8);
            INST(12, Opcode::Phi).s32().Inputs(5, 8);
            INST(13, Opcode::Add).s32().Inputs(11, 12);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(11, 12, 13, 20);
            INST(15, Opcode::Return).s32().Inputs(13);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Sub).s32().Inputs(1, 0);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(11, Opcode::Phi).s16().Inputs(5, 8);
            INST(12, Opcode::Phi).s32().Inputs(5, 8);
            INST(13, Opcode::Add).s32().Inputs(11, 12);
            INST(20, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).v0id().InputsAutoType(11, 12, 13, 20);
            INST(15, Opcode::Return).s32().Inputs(13);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestPhi5)
{
    // Peephole rule isn't applied.
    // Different types, different inputs.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(17, 2).f32();
        PARAMETER(18, 3).f32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);    // Phi 11
            INST(6, Opcode::Add).f32().Inputs(18, 17);  // Phi 12
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Sub).s32().Inputs(1, 0);    // Phi 11
            INST(9, Opcode::Add).f32().Inputs(18, 17);  // Phi 12
        }
        BASIC_BLOCK(5, 1)
        {
            INST(11, Opcode::Phi).s32().Inputs(5, 8);
            INST(12, Opcode::Phi).f32().Inputs(6, 9);
            INST(13, Opcode::Cast).f32().SrcType(DataType::INT32).Inputs(11);
            INST(14, Opcode::Add).f32().Inputs(13, 12);
            INST(20, Opcode::SaveState).NoVregs();
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(11, 12, 13, 14, 20);
            INST(16, Opcode::Return).f32().Inputs(14);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(17, 2).f32();
        PARAMETER(18, 3).f32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);    // Phi 11
            INST(6, Opcode::Add).f32().Inputs(18, 17);  // Phi 12
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Sub).s32().Inputs(1, 0);    // Phi 11
            INST(9, Opcode::Add).f32().Inputs(18, 17);  // Phi 12
        }
        BASIC_BLOCK(5, 1)
        {
            INST(11, Opcode::Phi).s32().Inputs(5, 8);
            INST(12, Opcode::Phi).f32().Inputs(6, 9);
            INST(13, Opcode::Cast).f32().SrcType(DataType::INT32).Inputs(11);
            INST(14, Opcode::Add).f32().Inputs(13, 12);
            INST(20, Opcode::SaveState).NoVregs();
            INST(15, Opcode::CallStatic).v0id().InputsAutoType(11, 12, 13, 14, 20);
            INST(16, Opcode::Return).f32().Inputs(14);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, MultiplePeepholeTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 100);
        CONSTANT(1, -46);
        CONSTANT(2, 0);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(1, 2);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(5, Opcode::Sub).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Add).s32().Inputs(0, 1);
        }
        BASIC_BLOCK(5, 1)
        {
            INST(11, Opcode::Phi).s32().Inputs(5, 8);
            INST(16, Opcode::Return).s32().Inputs(11);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GetGraph()->RunPass<BranchElimination>();
    GetGraph()->RunPass<Cleanup>();
    GetGraph()->RunPass<Peepholes>();
#ifndef NDEBUG
    GetGraph()->SetLowLevelInstructionsEnabled();
#endif
    GetGraph()->RunPass<Lowering>();
    GetGraph()->RunPass<Cleanup>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(5, 1)
        {
            INST(16, Opcode::ReturnI).s32().Imm(146);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest)
{
    // test case 2
    CheckCompare(DataType::UINT32, ConditionCode::CC_LT, ConditionCode::CC_B);
    CheckCompare(DataType::UINT32, ConditionCode::CC_LE, ConditionCode::CC_BE);
    CheckCompare(DataType::UINT32, ConditionCode::CC_GT, ConditionCode::CC_A);
    CheckCompare(DataType::UINT32, ConditionCode::CC_GE, ConditionCode::CC_AE);
    CheckCompare(DataType::UINT32, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::UINT32, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT32, ConditionCode::CC_LT, ConditionCode::CC_LT);
    CheckCompare(DataType::INT32, ConditionCode::CC_LE, ConditionCode::CC_LE);
    CheckCompare(DataType::INT32, ConditionCode::CC_GT, ConditionCode::CC_GT);
    CheckCompare(DataType::INT32, ConditionCode::CC_GE, ConditionCode::CC_GE);
    CheckCompare(DataType::INT32, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::INT32, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT32, ConditionCode::CC_B, ConditionCode::CC_B);
    CheckCompare(DataType::INT32, ConditionCode::CC_BE, ConditionCode::CC_BE);
    CheckCompare(DataType::INT32, ConditionCode::CC_A, ConditionCode::CC_A);
    CheckCompare(DataType::INT32, ConditionCode::CC_AE, ConditionCode::CC_AE);

    CheckCompare(DataType::UINT64, ConditionCode::CC_LT, ConditionCode::CC_B);
    CheckCompare(DataType::UINT64, ConditionCode::CC_LE, ConditionCode::CC_BE);
    CheckCompare(DataType::UINT64, ConditionCode::CC_GT, ConditionCode::CC_A);
    CheckCompare(DataType::UINT64, ConditionCode::CC_GE, ConditionCode::CC_AE);
    CheckCompare(DataType::UINT64, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::UINT64, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT64, ConditionCode::CC_LT, ConditionCode::CC_LT);
    CheckCompare(DataType::INT64, ConditionCode::CC_LE, ConditionCode::CC_LE);
    CheckCompare(DataType::INT64, ConditionCode::CC_GT, ConditionCode::CC_GT);
    CheckCompare(DataType::INT64, ConditionCode::CC_GE, ConditionCode::CC_GE);
    CheckCompare(DataType::INT64, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::INT64, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT64, ConditionCode::CC_B, ConditionCode::CC_B);
    CheckCompare(DataType::INT64, ConditionCode::CC_BE, ConditionCode::CC_BE);
    CheckCompare(DataType::INT64, ConditionCode::CC_A, ConditionCode::CC_A);
    CheckCompare(DataType::INT64, ConditionCode::CC_AE, ConditionCode::CC_AE);
}

TEST_F(PeepholesTest, CompareTest1)
{
    // applied case 2
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).u32();
        PARAMETER(2, 1).u32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(3, 0);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).u32();
        PARAMETER(2, 1).u32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_B).Inputs(1, 2);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1_0)
{
    // cmp with zero and inputs are signed, compare operands are in normal order (constant is the second operand)
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        PARAMETER(1, 1).u32();
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::Cmp).s32().Inputs(0, 1);
            INST(3, Opcode::Compare).b().CC(CC_LT).Inputs(2, 0);
            INST(4, Opcode::Return).b().Inputs(3);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        PARAMETER(1, 1).u32();
        BASIC_BLOCK(2, 1)
        {
            INST(2, Opcode::Cmp).s32().Inputs(0, 1);
            INST(3, Opcode::Compare).b().CC(CC_B).Inputs(0, 1);
            INST(4, Opcode::Return).b().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1_1)
{
    // cmp inputs are unsigned, compare operands are in reverse order (constant is the first operand)
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).u32();
        PARAMETER(2, 1).u32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).u32();
        PARAMETER(2, 1).u32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_A).Inputs(1, 2);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1_2)
{
    // cmp inputs are signed and compare operands are in normal order
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(3, 0);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(1, 2);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1_3)
{
    // cmp inputs are signed and compare operands are in reverse order
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(0, 3);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_GT).Inputs(1, 2);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest2)
{
    // not applied, Compare with non zero constant
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(3, 0);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(3, 0);
            INST(5, Opcode::Return).b().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest3)
{
    // not applied, cmp have more than 1 users.
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(3, 0);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 1)
        {
            INST(6, Opcode::Return).s32().Inputs(3);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(7, Opcode::Return).s32().Inputs(3);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1);
        PARAMETER(1, 0).s32();
        PARAMETER(2, 1).s32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Cmp).s32().Inputs(1, 2);
            INST(4, Opcode::Compare).b().CC(CC_LT).Inputs(3, 0);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 1)
        {
            INST(6, Opcode::Return).s32().Inputs(3);
        }
        BASIC_BLOCK(4, 1)
        {
            INST(7, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest4)
{
    for (auto cc : {CC_EQ, CC_NE, CC_LT, CC_LE, CC_GT, CC_GE, CC_B, CC_BE, CC_A, CC_AE}) {
        for (auto cst : {-2, -1, 0, 1, 2}) {
            bool input_true = true;
            bool input_false = false;
            switch (cc) {
                case CC_EQ:
                    input_true = input_true == cst;
                    input_false = input_false == cst;
                    break;
                case CC_NE:
                    input_true = input_true != cst;
                    input_false = input_false != cst;
                    break;
                case CC_LT:
                    input_true = input_true < cst;
                    input_false = input_false < cst;
                    break;
                case CC_LE:
                    input_true = input_true <= cst;
                    input_false = input_false <= cst;
                    break;
                case CC_GT:
                    input_true = input_true > cst;
                    input_false = input_false > cst;
                    break;
                case CC_GE:
                    input_true = input_true >= cst;
                    input_false = input_false >= cst;
                    break;
                case CC_B:
                    input_true = input_true < static_cast<uint64_t>(cst);
                    input_false = input_false < static_cast<uint64_t>(cst);
                    break;
                case CC_BE:
                    input_true = input_true <= static_cast<uint64_t>(cst);
                    input_false = input_false <= static_cast<uint64_t>(cst);
                    break;
                case CC_A:
                    input_true = input_true > static_cast<uint64_t>(cst);
                    input_false = input_false > static_cast<uint64_t>(cst);
                    break;
                case CC_AE:
                    input_true = input_true >= static_cast<uint64_t>(cst);
                    input_false = input_false >= static_cast<uint64_t>(cst);
                    break;
                default:
                    UNREACHABLE();
            }
            if (input_true && input_false) {
                CheckCompare(cc, cst, {1}, false);
            } else if (!input_true && !input_false) {
                CheckCompare(cc, cst, {0}, false);
            } else if (input_true && !input_false) {
                CheckCompare(cc, cst, std::nullopt, false);
            } else {
                CheckCompare(cc, cst, std::nullopt, true);
            }
        }
    }
}

// cast case 1
TEST_F(PeepholesTest, CastTest1)
{
    for (int i = 1; i < DataType::ANY; ++i) {
        for (int j = 1; j < DataType::ANY; ++j) {
            if ((i == DataType::FLOAT32 || i == DataType::FLOAT64) && j >= DataType::BOOL && j <= DataType::INT16)
                continue;
            CheckCast(static_cast<DataType::Type>(i), static_cast<DataType::Type>(j), i == j);
        }
    }
}

// cast case 2
TEST_F(PeepholesTest, CastTest2_1)
{
    for (int i = 1; i < DataType::LAST - 5; ++i) {
        for (int j = 1; j < DataType::LAST - 5; ++j) {
            for (int k = 1; k < DataType::LAST - 5; ++k) {
                if ((i == DataType::FLOAT32 || i == DataType::FLOAT64) && j >= DataType::BOOL && j <= DataType::INT16)
                    continue;
                CheckCast(i, j, k);
            }
        }
    }
}
// cast case 2
TEST_F(PeepholesTest, CastTest2_2)
{
    for (int i = DataType::LAST - 5; i < DataType::ANY; ++i) {
        for (int j = DataType::LAST - 5; j < DataType::ANY; ++j) {
            for (int k = DataType::LAST - 5; k < DataType::ANY; ++k) {
                CheckCast(i, j, k);
            }
        }
    }
}

// cast case 1, several casts
TEST_F(PeepholesTest, CastTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0);
            INST(2, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(1);
            INST(3, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(2);
            INST(4, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0);
            INST(2, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0);
            INST(3, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0);
            INST(4, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0);
            INST(5, Opcode::Return).s32().Inputs(0);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Casts from float32/64 to int8/16 don't support.
// cast case 2, several casts
TEST_F(PeepholesTest, DISABLED_CastTest4)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s16();
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::Cast).f32().SrcType(DataType::INT16).Inputs(0);
            INST(2, Opcode::Cast).s16().SrcType(DataType::FLOAT32).Inputs(1);
            INST(3, Opcode::Cast).f64().SrcType(DataType::INT16).Inputs(2);
            INST(4, Opcode::Cast).s16().SrcType(DataType::FLOAT64).Inputs(3);
            INST(5, Opcode::Return).s16().Inputs(4);
        }
    }
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s16();
        BASIC_BLOCK(2, 1)
        {
            INST(1, Opcode::Cast).f32().SrcType(DataType::INT16).Inputs(0);
            INST(2, Opcode::Cast).s16().SrcType(DataType::FLOAT32).Inputs(1);
            INST(3, Opcode::Cast).f64().SrcType(DataType::INT16).Inputs(0);
            INST(4, Opcode::Cast).s16().SrcType(DataType::FLOAT64).Inputs(3);
            INST(5, Opcode::Return).s16().Inputs(0);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// cast case 3
TEST_F(PeepholesTest, CastTest5)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 1);
            INST(4, Opcode::Shl).s32().Inputs(3, 1);
            INST(5, Opcode::AShr).s32().Inputs(4, 1);
            INST(6, Opcode::Return).s32().Inputs(5);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0x18);
        CONSTANT(7, 0xff);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Shl).s32().Inputs(0, 1);
            INST(3, Opcode::Shr).s32().Inputs(2, 1);
            INST(8, Opcode::And).s32().Inputs(0, 7);

            INST(4, Opcode::Shl).s32().Inputs(8, 1);
            INST(5, Opcode::AShr).s32().Inputs(4, 1);
            INST(9, Opcode::Cast).s8().SrcType(DataType::INT32).Inputs(0);

            INST(6, Opcode::Return).s32().Inputs(9);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TrySwapInputsTest)
{
    for (Opcode opc : {Opcode::Add, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::Min, Opcode::Max, Opcode::Mul}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0).s32();
            CONSTANT(1, 23);
            BASIC_BLOCK(2, 1)
            {
                INST(2, opc).s32().Inputs(1, 0);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }
        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0, 0).s32();
            CONSTANT(1, 23);
            BASIC_BLOCK(2, 1)
            {
                INST(2, opc).s32().Inputs(0, 1);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, ReplaceXorWithNot)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, -1);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Xor).s32().Inputs(0, 1);
            INST(3, Opcode::Xor).s32().Inputs(1, 0);
            INST(4, Opcode::Add).s32().Inputs(2, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Not).s32().Inputs(0);
            INST(2, Opcode::Not).s32().Inputs(0);
            INST(3, Opcode::Add).s32().Inputs(1, 2);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, XorWithZero)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Xor).s32().Inputs(0, 1);
            INST(3, Opcode::Xor).s32().Inputs(1, 0);
            INST(4, Opcode::Add).s32().Inputs(2, 3);
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).s32().Inputs(0, 0);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CleanupTrigger)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);
        CONSTANT(3, 3);
        CONSTANT(4, 4);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(1, 4);
            INST(6, Opcode::Add).u64().Inputs(2, 3);
            INST(7, Opcode::Compare).b().Inputs(0, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 6}, {3, 5}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 2);
        CONSTANT(3, 3);
        CONSTANT(4, 4);
        CONSTANT(11, 5);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(1, 4);
            INST(6, Opcode::Add).u64().Inputs(2, 3);
            INST(7, Opcode::Compare).b().Inputs(0, 3);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs({{2, 11}, {3, 11}});
            INST(10, Opcode::Return).u64().Inputs(9);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestAbsUnsigned)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Abs).u64().Inputs(0);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3).GetInput(0).GetInst(), &INS(0));
}

TEST_F(PeepholesTest, SafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).s32();
        PARAMETER(2, 3).f64();
        PARAMETER(3, 4).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SafePoint).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::CallStatic).u64().InputsAutoType(0, 1, 2, 3, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }
    Graph *graph_et = CreateEmptyGraph();
    GRAPH(graph_et)
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).s32();
        PARAMETER(2, 3).f64();
        PARAMETER(3, 4).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SafePoint).Inputs(3).SrcVregs({3});
            INST(5, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(6, Opcode::CallStatic).u64().InputsAutoType(0, 1, 2, 3, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_et));
}

TEST_F(PeepholesTest, ShlShlAddAdd)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }
    Graph *graph_peepholed = CreateEmptyGraph();
    GRAPH(graph_peepholed)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Add).i64().Inputs(1, 4);
            INST(7, Opcode::Add).i64().Inputs(6, 5);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_peepholed));

    // Not optimizable cases
    Graph *graphNotOptimizable = CreateEmptyGraph();
    GRAPH(graphNotOptimizable)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            // shl4 !HasSingleUser()
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Add).i64().Inputs(4, 7);
            INST(9, Opcode::Return).i64().Inputs(8);
        }
    }
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    GRAPH(graphNotOptimizable)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            // shl5 !HasSingleUser()
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Add).i64().Inputs(5, 7);
            INST(9, Opcode::Return).i64().Inputs(8);
        }
    }
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    GRAPH(graphNotOptimizable)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            // add6 !HasSingleUser()
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Add).i64().Inputs(0, 6);
            INST(9, Opcode::Return).i64().Inputs(8);
        }
    }
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    GRAPH(graphNotOptimizable)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            // shl4 and shl5 have different input(0).
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(1, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    GRAPH(graphNotOptimizable)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        PARAMETER(2, 2).i64();
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            // shl4->GetInput(1) is not constant.
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(1, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    GRAPH(graphNotOptimizable)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        PARAMETER(3, 2).i64();
        BASIC_BLOCK(2, -1)
        {
            // shl5->GetInput(1) is not constant.
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(1, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    GRAPH(graphNotOptimizable)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, 3)
        {
            INST(20, Opcode::Shl).i64().Inputs(0, 2);
            INST(21, Opcode::Shl).i64().Inputs(0, 3);
            INST(22, Opcode::Add).i64().Inputs(20, 21);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(30, Opcode::Add).i64().Inputs(0, 1);
        }
        BASIC_BLOCK(4, -1)
        {
            // add40: input(0) does not dominate input(1)
            INST(40, Opcode::Add).i64().Inputs(30, 22);
            INST(41, Opcode::Return).i64().Inputs(40);
        }
    }
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());
}

TEST_F(PeepholesTest, ShlShlAddSub)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Add).i64().Inputs(4, 5);
            INST(7, Opcode::Sub).i64().Inputs(1, 6);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }
    Graph *graph_peepholed = CreateEmptyGraph();
    GRAPH(graph_peepholed)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Sub).i64().Inputs(1, 4);
            INST(7, Opcode::Sub).i64().Inputs(6, 5);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_peepholed));
}

TEST_F(PeepholesTest, ShlShlSubAdd)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Sub).i64().Inputs(4, 5);
            INST(7, Opcode::Add).i64().Inputs(1, 6);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }
    Graph *graph_peepholed = CreateEmptyGraph();
    GRAPH(graph_peepholed)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Add).i64().Inputs(1, 4);
            INST(7, Opcode::Sub).i64().Inputs(6, 5);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_peepholed));
}

TEST_F(PeepholesTest, ShlShlSubSub)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Sub).i64().Inputs(4, 5);
            INST(7, Opcode::Sub).i64().Inputs(1, 6);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }
    Graph *graph_peepholed = CreateEmptyGraph();
    GRAPH(graph_peepholed)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i64();
        CONSTANT(2, 16);
        CONSTANT(3, 17);
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Shl).i64().Inputs(0, 2);
            INST(5, Opcode::Shl).i64().Inputs(0, 3);
            INST(6, Opcode::Sub).i64().Inputs(1, 4);
            INST(7, Opcode::Add).i64().Inputs(6, 5);
            INST(8, Opcode::Return).i64().Inputs(7);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_peepholed));
}

TEST_F(PeepholesTest, CompareAndWithZero)
{
    CheckCompareFoldIntoTest(0, CC_EQ, true, CC_TST_EQ);
    CheckCompareFoldIntoTest(0, CC_NE, true, CC_TST_NE);

    CheckCompareFoldIntoTest(1, CC_EQ, false);
    CheckCompareFoldIntoTest(2, CC_NE, false);

    // check comparision with zero for all CCs except CC_EQ and CC_NE
    for (int cc = CC_LT; cc <= CC_LAST; ++cc) {
        CheckCompareFoldIntoTest(0, static_cast<ConditionCode>(cc), false);
    }
}

TEST_F(PeepholesTest, IfAndComparedWithZero)
{
    CheckIfAndZeroFoldIntoIfTest(0, CC_EQ, true, CC_TST_EQ);
    CheckIfAndZeroFoldIntoIfTest(0, CC_NE, true, CC_TST_NE);

    CheckIfAndZeroFoldIntoIfTest(1, CC_EQ, false);
    CheckIfAndZeroFoldIntoIfTest(2, CC_NE, false);

    // check comparision with zero for all CCs except CC_EQ and CC_NE
    for (int cc = CC_LT; cc <= CC_LAST; ++cc) {
        CheckCompareFoldIntoTest(0, static_cast<ConditionCode>(cc), false);
    }
}

TEST_F(PeepholesTest, CompareLenArrayWithZero)
{
    CheckCompareLenArrayWithZeroTest(0, CC_GE, true);
    CheckCompareLenArrayWithZeroTest(0, CC_LT, false);
    CheckCompareLenArrayWithZeroTest(0, CC_LE, std::nullopt);
    CheckCompareLenArrayWithZeroTest(1, CC_GE, std::nullopt);
    CheckCompareLenArrayWithZeroTest(1, CC_LT, std::nullopt);
}

TEST_F(PeepholesTest, CompareLenArrayWithZeroSwapped)
{
    CheckCompareLenArrayWithZeroTest(0, CC_LE, true, true);
    CheckCompareLenArrayWithZeroTest(0, CC_GT, false, true);
    CheckCompareLenArrayWithZeroTest(0, CC_GE, std::nullopt, true);
    CheckCompareLenArrayWithZeroTest(1, CC_LE, std::nullopt, true);
    CheckCompareLenArrayWithZeroTest(1, CC_GT, std::nullopt, true);
}

TEST_F(PeepholesTest, TestEqualInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Compare).b().CC(CC_TST_EQ).Inputs(0, 0);
            INST(2, Opcode::Compare).b().CC(CC_TST_NE).Inputs(0, 0);
            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::CallStatic).b().InputsAutoType(1, 2, 3);
            INST(5, Opcode::ReturnVoid);
        }
    }
    Graph *graph_peepholed = CreateEmptyGraph();
    GRAPH(graph_peepholed)
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(3, Opcode::Compare).b().CC(CC_NE).Inputs(0, 1);
            INST(4, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallStatic).b().InputsAutoType(2, 3, 4);
            INST(6, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_peepholed));
}

TEST_F(PeepholesTest, AndWithCast)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0xFFFFFFFF);
        CONSTANT(3, 0xFFFF);
        CONSTANT(4, 0xFF);
        CONSTANT(5, 0xF7);

        BASIC_BLOCK(2, -1)
        {
            INST(10, Opcode::And).i32().Inputs(1, 2);
            INST(11, Opcode::And).i32().Inputs(1, 3);
            INST(12, Opcode::And).i32().Inputs(1, 4);
            INST(13, Opcode::And).i32().Inputs(1, 5);

            INST(22, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(10);
            INST(23, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(11);
            INST(24, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(12);
            INST(25, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(13);

            INST(26, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(10);
            INST(27, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(11);
            INST(28, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(12);
            INST(29, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(13);

            INST(30, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(10);
            INST(31, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(11);
            INST(32, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(12);
            INST(33, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(13);

            INST(34, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(10);
            INST(35, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(11);
            INST(36, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(12);
            INST(37, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(13);

            INST(38, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph_expected = CreateEmptyGraph();
    GRAPH(graph_expected)
    {
        PARAMETER(0, 0).i64();
        PARAMETER(1, 1).i32();
        CONSTANT(2, 0xFFFFFFFF);
        CONSTANT(3, 0xFFFF);
        CONSTANT(4, 0xFF);
        CONSTANT(5, 0xF7);

        BASIC_BLOCK(2, -1)
        {
            INST(10, Opcode::And).i32().Inputs(1, 2);
            INST(11, Opcode::And).i32().Inputs(1, 3);
            INST(12, Opcode::And).i32().Inputs(1, 4);
            INST(13, Opcode::And).i32().Inputs(1, 5);

            INST(22, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(1);
            INST(23, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(1);
            INST(24, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(12);
            INST(25, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(13);

            INST(26, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(1);
            INST(27, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(1);
            INST(28, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(12);
            INST(29, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(13);

            INST(30, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(1);
            INST(31, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(1);
            INST(32, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(1);
            INST(33, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(13);

            INST(34, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(1);
            INST(35, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(1);
            INST(36, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(1);
            INST(37, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(13);

            INST(38, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
}

TEST_F(PeepholesTest, AndWithStore)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 0xFFFFFFFF);
        CONSTANT(3, 0xFFFF);
        CONSTANT(4, 0xFF);
        CONSTANT(5, 0xF7);

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::And).i32().Inputs(1, 2);
            INST(7, Opcode::And).i32().Inputs(1, 3);
            INST(8, Opcode::And).i32().Inputs(1, 4);
            INST(9, Opcode::And).i32().Inputs(1, 5);

            INST(10, Opcode::Store).i32().Inputs(0, 1, 6);
            INST(11, Opcode::Store).i32().Inputs(0, 1, 7);
            INST(12, Opcode::Store).i32().Inputs(0, 1, 8);
            INST(13, Opcode::Store).i32().Inputs(0, 1, 9);

            INST(14, Opcode::Store).u32().Inputs(0, 1, 6);
            INST(15, Opcode::Store).u32().Inputs(0, 1, 7);
            INST(16, Opcode::Store).u32().Inputs(0, 1, 8);
            INST(17, Opcode::Store).u32().Inputs(0, 1, 9);

            INST(18, Opcode::Store).i16().Inputs(0, 1, 6);
            INST(19, Opcode::Store).i16().Inputs(0, 1, 7);
            INST(20, Opcode::Store).i16().Inputs(0, 1, 8);
            INST(21, Opcode::Store).i16().Inputs(0, 1, 9);

            INST(22, Opcode::Store).u16().Inputs(0, 1, 6);
            INST(23, Opcode::Store).u16().Inputs(0, 1, 7);
            INST(24, Opcode::Store).u16().Inputs(0, 1, 8);
            INST(25, Opcode::Store).u16().Inputs(0, 1, 9);

            INST(26, Opcode::Store).i8().Inputs(0, 1, 6);
            INST(27, Opcode::Store).i8().Inputs(0, 1, 7);
            INST(28, Opcode::Store).i8().Inputs(0, 1, 8);
            INST(29, Opcode::Store).i8().Inputs(0, 1, 9);

            INST(30, Opcode::Store).u8().Inputs(0, 1, 6);
            INST(31, Opcode::Store).u8().Inputs(0, 1, 7);
            INST(32, Opcode::Store).u8().Inputs(0, 1, 8);
            INST(33, Opcode::Store).u8().Inputs(0, 1, 9);

            INST(34, Opcode::ReturnVoid).v0id();
        }
    }
    auto graph_expected = CreateEmptyGraph();
    GRAPH(graph_expected)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 0xFFFFFFFF);
        CONSTANT(3, 0xFFFF);
        CONSTANT(4, 0xFF);
        CONSTANT(5, 0xF7);

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::And).i32().Inputs(1, 2);
            INST(7, Opcode::And).i32().Inputs(1, 3);
            INST(8, Opcode::And).i32().Inputs(1, 4);
            INST(9, Opcode::And).i32().Inputs(1, 5);

            INST(10, Opcode::Store).i32().Inputs(0, 1, 1);
            INST(11, Opcode::Store).i32().Inputs(0, 1, 7);
            INST(12, Opcode::Store).i32().Inputs(0, 1, 8);
            INST(13, Opcode::Store).i32().Inputs(0, 1, 9);

            INST(14, Opcode::Store).u32().Inputs(0, 1, 1);
            INST(15, Opcode::Store).u32().Inputs(0, 1, 7);
            INST(16, Opcode::Store).u32().Inputs(0, 1, 8);
            INST(17, Opcode::Store).u32().Inputs(0, 1, 9);

            INST(18, Opcode::Store).i16().Inputs(0, 1, 1);
            INST(19, Opcode::Store).i16().Inputs(0, 1, 1);
            INST(20, Opcode::Store).i16().Inputs(0, 1, 8);
            INST(21, Opcode::Store).i16().Inputs(0, 1, 9);

            INST(22, Opcode::Store).u16().Inputs(0, 1, 1);
            INST(23, Opcode::Store).u16().Inputs(0, 1, 1);
            INST(24, Opcode::Store).u16().Inputs(0, 1, 8);
            INST(25, Opcode::Store).u16().Inputs(0, 1, 9);

            INST(26, Opcode::Store).i8().Inputs(0, 1, 1);
            INST(27, Opcode::Store).i8().Inputs(0, 1, 1);
            INST(28, Opcode::Store).i8().Inputs(0, 1, 1);
            INST(29, Opcode::Store).i8().Inputs(0, 1, 9);

            INST(30, Opcode::Store).u8().Inputs(0, 1, 1);
            INST(31, Opcode::Store).u8().Inputs(0, 1, 1);
            INST(32, Opcode::Store).u8().Inputs(0, 1, 1);
            INST(33, Opcode::Store).u8().Inputs(0, 1, 9);

            INST(34, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
}

TEST_F(PeepholesTest, CastWithStore)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).i64();
        PARAMETER(2, 2).i32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(1);
            INST(7, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(2);
            INST(8, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(2);

            INST(9, Opcode::Store).i32().Inputs(0, 1, 6);
            INST(10, Opcode::Store).i32().Inputs(0, 1, 7);
            INST(11, Opcode::Store).i32().Inputs(0, 1, 8);

            INST(12, Opcode::Store).u32().Inputs(0, 1, 6);
            INST(13, Opcode::Store).u32().Inputs(0, 1, 7);
            INST(14, Opcode::Store).u32().Inputs(0, 1, 8);

            INST(15, Opcode::Store).i16().Inputs(0, 1, 6);
            INST(16, Opcode::Store).i16().Inputs(0, 1, 7);
            INST(17, Opcode::Store).i16().Inputs(0, 1, 8);

            INST(18, Opcode::Store).u16().Inputs(0, 1, 6);
            INST(19, Opcode::Store).u16().Inputs(0, 1, 7);
            INST(20, Opcode::Store).u16().Inputs(0, 1, 8);

            INST(21, Opcode::Store).i8().Inputs(0, 1, 6);
            INST(22, Opcode::Store).i8().Inputs(0, 1, 7);
            INST(23, Opcode::Store).i8().Inputs(0, 1, 8);

            INST(24, Opcode::Store).u8().Inputs(0, 1, 6);
            INST(25, Opcode::Store).u8().Inputs(0, 1, 7);
            INST(26, Opcode::Store).u8().Inputs(0, 1, 8);

            INST(27, Opcode::ReturnVoid).v0id();
        }
    }
    auto graph_expected = CreateEmptyGraph();
    GRAPH(graph_expected)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).i64();
        PARAMETER(2, 2).i32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(1);
            INST(7, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(2);
            INST(8, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(2);

            INST(9, Opcode::Store).i32().Inputs(0, 1, 1);
            INST(10, Opcode::Store).i32().Inputs(0, 1, 7);
            INST(11, Opcode::Store).i32().Inputs(0, 1, 8);

            INST(12, Opcode::Store).u32().Inputs(0, 1, 1);
            INST(13, Opcode::Store).u32().Inputs(0, 1, 7);
            INST(14, Opcode::Store).u32().Inputs(0, 1, 8);

            INST(15, Opcode::Store).i16().Inputs(0, 1, 1);
            INST(16, Opcode::Store).i16().Inputs(0, 1, 2);
            INST(17, Opcode::Store).i16().Inputs(0, 1, 8);

            INST(18, Opcode::Store).u16().Inputs(0, 1, 1);
            INST(19, Opcode::Store).u16().Inputs(0, 1, 2);
            INST(20, Opcode::Store).u16().Inputs(0, 1, 8);

            INST(21, Opcode::Store).i8().Inputs(0, 1, 1);
            INST(22, Opcode::Store).i8().Inputs(0, 1, 2);
            INST(23, Opcode::Store).i8().Inputs(0, 1, 2);

            INST(24, Opcode::Store).u8().Inputs(0, 1, 1);
            INST(25, Opcode::Store).u8().Inputs(0, 1, 2);
            INST(26, Opcode::Store).u8().Inputs(0, 1, 2);

            INST(27, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
}

TEST_F(PeepholesTest, CastWithStoreSignMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).i64();
        PARAMETER(2, 2).u8();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Cast).i32().SrcType(DataType::INT8).Inputs(2);
            INST(4, Opcode::Store).i32().Inputs(0, 1, 3);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, CastWithStoreTypeMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ptr();
        PARAMETER(1, 1).i64();
        PARAMETER(2, 2).i16();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Cast).i32().SrcType(DataType::INT8).Inputs(2);
            INST(4, Opcode::Store).i32().Inputs(0, 1, 3);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }
#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, CastI64ToU16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(2, 0xFFFF);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0);
            INST(4, Opcode::And).i32().Inputs(3, 2);
            INST(5, Opcode::Return).i32().Inputs(4);
        }
    }

    auto graph_expected = CreateEmptyGraph();
    GRAPH(graph_expected)
    {
        PARAMETER(0, 0).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Cast).u16().SrcType(DataType::INT64).Inputs(0);
            INST(5, Opcode::Return).i32().Inputs(6);
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
}

TEST_F(PeepholesTest, CastI8ToU16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i8();
        CONSTANT(2, 0xFFFF);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Cast).i32().SrcType(DataType::INT8).Inputs(0);
            INST(4, Opcode::And).i32().Inputs(3, 2);
            INST(5, Opcode::Return).i32().Inputs(4);
        }
    }

    auto graph_expected = CreateEmptyGraph();
    GRAPH(graph_expected)
    {
        PARAMETER(0, 0).i8();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Cast).u16().SrcType(DataType::INT8).Inputs(0);
            INST(5, Opcode::Return).i32().Inputs(6);
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
}

TEST_F(PeepholesTest, CastI64ToB)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0);
            INST(2, Opcode::Cast).b().SrcType(DataType::INT32).Inputs(1);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }

    auto graph_expected = CreateEmptyGraph();
    GRAPH(graph_expected)
    {
        PARAMETER(0, 0).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Cast).b().SrcType(DataType::INT64).Inputs(0);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph, graph_expected));
}

TEST_F(PeepholesTest, CastI64ToBSignMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0);
            INST(2, Opcode::Cast).b().SrcType(DataType::UINT32).Inputs(1);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }

#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, CastI64ToBTypeMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).i64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0);
            INST(2, Opcode::Cast).b().SrcType(DataType::INT16).Inputs(1);
            INST(3, Opcode::Return).b().Inputs(2);
        }
    }

#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, ConstCombineAdd)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(1, 3);
        CONSTANT(2, 7);
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Add).s64().Inputs(0, 1);
            INST(4, Opcode::Add).s64().Inputs(3, 2);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(6, 10);
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::Add).s64().Inputs(0, 6);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, ConstCombineAddSub)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(1, 3);
        CONSTANT(2, 7);
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Sub).s64().Inputs(0, 1);
            INST(4, Opcode::Add).s64().Inputs(3, 2);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(6, 4);
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::Add).s64().Inputs(0, 6);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, ConstCombineSubAdd)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(1, 3);
        CONSTANT(2, 7);
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Add).s64().Inputs(0, 1);
            INST(4, Opcode::Sub).s64().Inputs(3, 2);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(6, 4);
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::Sub).s64().Inputs(0, 6);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, ConstCombineShift)
{
    for (auto opcode : {Opcode::Shr, Opcode::Shl, Opcode::AShr}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0, 0).i64();
            CONSTANT(1, 3);
            CONSTANT(2, 7);
            BASIC_BLOCK(2, 1)
            {
                INST(3, opcode).s64().Inputs(0, 1);
                INST(4, opcode).s64().Inputs(3, 2);
                INST(5, Opcode::Return).s64().Inputs(4);
            }
        }
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0, 0).i64();
            CONSTANT(6, 10);
            BASIC_BLOCK(2, 1)
            {
                INST(4, opcode).s64().Inputs(0, 6);
                INST(5, Opcode::Return).s64().Inputs(4);
            }
        }
        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, ConstCombineMul)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(1, 3);
        CONSTANT(2, 7);
        BASIC_BLOCK(2, 1)
        {
            INST(3, Opcode::Mul).s64().Inputs(0, 1);
            INST(4, Opcode::Mul).s64().Inputs(3, 2);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0, 0).i64();
        CONSTANT(6, 21);
        BASIC_BLOCK(2, 1)
        {
            INST(4, Opcode::Mul).s64().Inputs(0, 6);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}
}  // namespace panda::compiler
