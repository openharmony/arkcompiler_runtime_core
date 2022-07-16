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
#include "optimizer/optimizations/cse.h"

namespace panda::compiler {
class CSETest : public GraphTest {
public:
    CSETest() : is_cse_enable_default_(options.IsCompilerCse())
    {
        options.SetCompilerCse(true);
    }

    ~CSETest()
    {
        options.SetCompilerCse(is_cse_enable_default_);
    }

private:
    bool is_cse_enable_default_;
};

TEST_F(CSETest, CSETestApply1)
{
    // Remove duplicate arithmetic instructions in one basicblock
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Sub).u32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::Div).f64().Inputs(3, 2);

            INST(10, Opcode::Sub).u32().Inputs(1, 0);
            INST(11, Opcode::Div).f64().Inputs(3, 2);
            INST(12, Opcode::Mul).f32().Inputs(4, 5);
            INST(13, Opcode::Add).u64().Inputs(0, 1);

            INST(14, Opcode::Mod).u64().Inputs(0, 1);
            INST(15, Opcode::Min).u64().Inputs(0, 1);
            INST(16, Opcode::Max).u64().Inputs(0, 1);
            INST(17, Opcode::Shl).u64().Inputs(0, 1);
            INST(18, Opcode::Shr).u64().Inputs(0, 1);
            INST(19, Opcode::AShr).u64().Inputs(0, 1);
            INST(20, Opcode::And).b().Inputs(0, 1);
            INST(21, Opcode::Or).b().Inputs(0, 1);
            INST(22, Opcode::Xor).b().Inputs(0, 1);

            INST(23, Opcode::Mod).u64().Inputs(0, 1);
            INST(24, Opcode::Min).u64().Inputs(0, 1);
            INST(25, Opcode::Max).u64().Inputs(0, 1);
            INST(26, Opcode::Shl).u64().Inputs(0, 1);
            INST(27, Opcode::Shr).u64().Inputs(0, 1);
            INST(28, Opcode::AShr).u64().Inputs(0, 1);
            INST(29, Opcode::And).b().Inputs(0, 1);
            INST(30, Opcode::Or).b().Inputs(0, 1);
            INST(31, Opcode::Xor).b().Inputs(0, 1);
            INST(35, Opcode::SaveState).NoVregs();
            INST(32, Opcode::CallStatic)
                .b()
                .InputsAutoType(6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                                29, 30, 31, 35);
            INST(33, Opcode::ReturnVoid);
        }
    }

    // delete insts 10, 11, 12, 13, 23, 24, 25, 26, 27, 28, 29, 30, 31
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Sub).u32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::Div).f64().Inputs(3, 2);

            INST(14, Opcode::Mod).u64().Inputs(0, 1);
            INST(15, Opcode::Min).u64().Inputs(0, 1);
            INST(16, Opcode::Max).u64().Inputs(0, 1);
            INST(17, Opcode::Shl).u64().Inputs(0, 1);
            INST(18, Opcode::Shr).u64().Inputs(0, 1);
            INST(19, Opcode::AShr).u64().Inputs(0, 1);
            INST(20, Opcode::And).b().Inputs(0, 1);
            INST(21, Opcode::Or).b().Inputs(0, 1);
            INST(22, Opcode::Xor).b().Inputs(0, 1);
            INST(35, Opcode::SaveState).NoVregs();
            INST(32, Opcode::CallStatic)
                .b()
                .InputsAutoType(6, 7, 8, 9, 7, 9, 8, 6, 14, 15, 16, 17, 18, 19, 20, 21, 22, 14, 15, 16, 17, 18, 19, 20,
                                21, 22, 35);
            INST(33, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<Cse>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}

TEST_F(CSETest, CSETestApply2)
{
    // Remove duplicate arithmetic instructions along dominator tree
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Add).s32().Inputs(0, 1);
            INST(7, Opcode::Sub).s32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::Div).f64().Inputs(3, 2);
            INST(35, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 35);
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(12, Opcode::Add).s32().Inputs(0, 1);
            INST(13, Opcode::Add).s32().Inputs(12, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(14, Opcode::Sub).s32().Inputs(1, 0);
            INST(15, Opcode::Sub).s32().Inputs(14, 1);
            INST(16, Opcode::Mul).f32().Inputs(4, 5);
            INST(17, Opcode::Mul).f32().Inputs(16, 5);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(18, Opcode::Div).f64().Inputs(3, 2);
            INST(19, Opcode::Div).f64().Inputs(18, 3);
            INST(20, Opcode::ReturnVoid);
        }
    }

    // Delete Insts 12, 14, 16, 18
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Add).s32().Inputs(0, 1);
            INST(7, Opcode::Sub).s32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::Div).f64().Inputs(3, 2);
            INST(35, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 35);
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(13, Opcode::Add).s32().Inputs(6, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(15, Opcode::Sub).s32().Inputs(7, 1);
            INST(17, Opcode::Mul).f32().Inputs(8, 5);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(19, Opcode::Div).f64().Inputs(9, 3);
            INST(20, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}

TEST_F(CSETest, CSETestApply3)
{
    // use Phi to replace the arithmetic instruction which has appeared in the two preds of its block.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(8, Opcode::Add).u64().Inputs(0, 1);
            INST(9, Opcode::Sub).u32().Inputs(1, 0);
            INST(10, Opcode::Mul).f32().Inputs(4, 5);
            INST(11, Opcode::Div).f64().Inputs(3, 2);

            INST(22, Opcode::Add).u64().Inputs(8, 1);
            INST(23, Opcode::Sub).u32().Inputs(9, 0);
            INST(24, Opcode::Mul).f32().Inputs(10, 5);
            INST(25, Opcode::Div).f64().Inputs(11, 2);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(12, Opcode::Add).u64().Inputs(0, 1);
            INST(13, Opcode::Sub).u32().Inputs(1, 0);
            INST(14, Opcode::Mul).f32().Inputs(4, 5);
            INST(15, Opcode::Div).f64().Inputs(3, 2);

            INST(26, Opcode::Add).u64().Inputs(12, 1);
            INST(27, Opcode::Sub).u32().Inputs(13, 0);
            INST(28, Opcode::Mul).f32().Inputs(14, 5);
            INST(29, Opcode::Div).f64().Inputs(15, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(16, Opcode::Add).u64().Inputs(0, 1);
            INST(17, Opcode::Sub).u32().Inputs(1, 0);
            INST(18, Opcode::Mul).f32().Inputs(4, 5);
            INST(19, Opcode::Div).f64().Inputs(3, 2);
            INST(35, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(16, 17, 18, 19, 35);
            INST(21, Opcode::ReturnVoid);
        }
    }

    // Add Phi 30, 31, 32, 33; Delete Inst 16, 17, 18, 19
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(8, Opcode::Add).u64().Inputs(0, 1);
            INST(9, Opcode::Sub).u32().Inputs(1, 0);
            INST(10, Opcode::Mul).f32().Inputs(4, 5);
            INST(11, Opcode::Div).f64().Inputs(3, 2);

            INST(22, Opcode::Add).u64().Inputs(8, 1);
            INST(23, Opcode::Sub).u32().Inputs(9, 0);
            INST(24, Opcode::Mul).f32().Inputs(10, 5);
            INST(25, Opcode::Div).f64().Inputs(11, 2);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(12, Opcode::Add).u64().Inputs(0, 1);
            INST(13, Opcode::Sub).u32().Inputs(1, 0);
            INST(14, Opcode::Mul).f32().Inputs(4, 5);
            INST(15, Opcode::Div).f64().Inputs(3, 2);

            INST(26, Opcode::Add).u64().Inputs(12, 1);
            INST(27, Opcode::Sub).u32().Inputs(13, 0);
            INST(28, Opcode::Mul).f32().Inputs(14, 5);
            INST(29, Opcode::Div).f64().Inputs(15, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(30, Opcode::Phi).u64().Inputs({{3, 8}, {4, 12}});
            INST(31, Opcode::Phi).u32().Inputs({{3, 9}, {4, 13}});
            INST(32, Opcode::Phi).f32().Inputs({{3, 10}, {4, 14}});
            INST(33, Opcode::Phi).f64().Inputs({{3, 11}, {4, 15}});
            INST(35, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(30, 31, 32, 33, 35);
            INST(21, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}

TEST_F(CSETest, CSETestApply4)
{
    // Remove duplicate arithmetic instructions in one basicblock (commutative)
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Mul).f32().Inputs(4, 5);

            INST(8, Opcode::Mul).f32().Inputs(5, 4);
            INST(9, Opcode::Add).u64().Inputs(1, 0);

            INST(10, Opcode::Min).u64().Inputs(0, 1);
            INST(11, Opcode::Max).u64().Inputs(0, 1);
            INST(12, Opcode::And).b().Inputs(0, 1);
            INST(13, Opcode::Or).b().Inputs(0, 1);
            INST(14, Opcode::Xor).b().Inputs(0, 1);

            INST(15, Opcode::Min).u64().Inputs(1, 0);
            INST(16, Opcode::Max).u64().Inputs(1, 0);
            INST(17, Opcode::And).b().Inputs(1, 0);
            INST(18, Opcode::Or).b().Inputs(1, 0);
            INST(19, Opcode::Xor).b().Inputs(1, 0);
            INST(35, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 35);
            INST(21, Opcode::ReturnVoid);
        }
    }

    // delete insts 8, 9, 15, 16, 17, 18, 19
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
            INST(7, Opcode::Mul).f32().Inputs(4, 5);

            INST(10, Opcode::Min).u64().Inputs(0, 1);
            INST(11, Opcode::Max).u64().Inputs(0, 1);
            INST(12, Opcode::And).b().Inputs(0, 1);
            INST(13, Opcode::Or).b().Inputs(0, 1);
            INST(14, Opcode::Xor).b().Inputs(0, 1);
            INST(35, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(6, 7, 7, 6, 10, 11, 12, 13, 14, 10, 11, 12, 13, 14, 35);
            INST(21, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<Cse>();
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}

TEST_F(CSETest, CSETestApply5)
{
    // Remove duplicate arithmetic instructions along dominator tree(commutative)
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Add).s32().Inputs(0, 1);
            INST(7, Opcode::Max).s32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::And).b().Inputs(0, 1);
            INST(35, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 35);
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(12, Opcode::Add).s32().Inputs(1, 0);
            INST(13, Opcode::Add).s32().Inputs(12, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(14, Opcode::Max).s32().Inputs(0, 1);
            INST(15, Opcode::Sub).s32().Inputs(14, 1);
            INST(16, Opcode::Mul).f32().Inputs(5, 4);
            INST(17, Opcode::Mul).f32().Inputs(16, 5);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(18, Opcode::And).b().Inputs(1, 0);
            INST(19, Opcode::Xor).b().Inputs(18, 1);
            INST(20, Opcode::ReturnVoid);
        }
    }

    // Delete Insts 12, 14, 16, 18
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Add).s32().Inputs(0, 1);
            INST(7, Opcode::Max).s32().Inputs(1, 0);
            INST(8, Opcode::Mul).f32().Inputs(4, 5);
            INST(9, Opcode::And).b().Inputs(0, 1);
            INST(35, Opcode::SaveState).NoVregs();
            INST(21, Opcode::CallStatic).b().InputsAutoType(6, 7, 8, 9, 35);
            INST(10, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(0, 1);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(13, Opcode::Add).s32().Inputs(6, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(15, Opcode::Sub).s32().Inputs(7, 1);
            INST(17, Opcode::Mul).f32().Inputs(8, 5);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(19, Opcode::Xor).b().Inputs(9, 1);
            INST(20, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}

TEST_F(CSETest, CSETestApply6)
{
    // use Phi to replace the arithmetic instruction which has appeared in the two preds of its block (commutative).
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).b();
        PARAMETER(5, 5).b();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(8, Opcode::Or).b().Inputs(0, 1);
            INST(9, Opcode::Min).u64().Inputs(1, 0);
            INST(10, Opcode::Xor).b().Inputs(4, 5);
            INST(11, Opcode::Max).u64().Inputs(3, 2);

            INST(22, Opcode::Or).b().Inputs(8, 1);
            INST(23, Opcode::Min).u64().Inputs(9, 0);
            INST(24, Opcode::Xor).b().Inputs(10, 5);
            INST(25, Opcode::Max).u64().Inputs(11, 2);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(12, Opcode::Or).b().Inputs(1, 0);
            INST(13, Opcode::Min).u64().Inputs(0, 1);
            INST(14, Opcode::Xor).b().Inputs(5, 4);
            INST(15, Opcode::Max).u64().Inputs(2, 3);

            INST(26, Opcode::Or).b().Inputs(12, 1);
            INST(27, Opcode::Min).u64().Inputs(13, 0);
            INST(28, Opcode::Xor).b().Inputs(14, 5);
            INST(29, Opcode::Max).u64().Inputs(15, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(16, Opcode::Or).b().Inputs(1, 0);
            INST(17, Opcode::Min).u64().Inputs(0, 1);
            INST(18, Opcode::Xor).b().Inputs(5, 4);
            INST(19, Opcode::Max).u64().Inputs(2, 3);
            INST(35, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(16, 17, 18, 19, 35);
            INST(21, Opcode::ReturnVoid);
        }
    }

    // Add Phi 30, 31, 32, 33; Delete Inst 16, 17, 18, 19
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        PARAMETER(3, 3).u64();
        PARAMETER(4, 4).b();
        PARAMETER(5, 5).b();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::UINT64).Inputs(0, 1);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(8, Opcode::Or).b().Inputs(0, 1);
            INST(9, Opcode::Min).u64().Inputs(1, 0);
            INST(10, Opcode::Xor).b().Inputs(4, 5);
            INST(11, Opcode::Max).u64().Inputs(3, 2);

            INST(22, Opcode::Or).b().Inputs(8, 1);
            INST(23, Opcode::Min).u64().Inputs(9, 0);
            INST(24, Opcode::Xor).b().Inputs(10, 5);
            INST(25, Opcode::Max).u64().Inputs(11, 2);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(12, Opcode::Or).b().Inputs(1, 0);
            INST(13, Opcode::Min).u64().Inputs(0, 1);
            INST(14, Opcode::Xor).b().Inputs(5, 4);
            INST(15, Opcode::Max).u64().Inputs(2, 3);

            INST(26, Opcode::Or).b().Inputs(12, 1);
            INST(27, Opcode::Min).u64().Inputs(13, 0);
            INST(28, Opcode::Xor).b().Inputs(14, 5);
            INST(29, Opcode::Max).u64().Inputs(15, 2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(30, Opcode::Phi).b().Inputs({{3, 8}, {4, 12}});
            INST(31, Opcode::Phi).u64().Inputs({{3, 9}, {4, 13}});
            INST(32, Opcode::Phi).b().Inputs({{3, 10}, {4, 14}});
            INST(33, Opcode::Phi).u64().Inputs({{3, 11}, {4, 15}});
            INST(35, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(30, 31, 32, 33, 35);
            INST(21, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Cse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}

TEST_F(CSETest, CSETestNotApply1)
{
    // Instructions has different type, inputs, opcodes
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();
        PARAMETER(6, 6).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::Add).u64().Inputs(0, 1);
            INST(8, Opcode::Sub).u32().Inputs(1, 0);
            INST(9, Opcode::Mul).f32().Inputs(4, 5);

            INST(10, Opcode::Sub).u64().Inputs(0, 1);  // different opcode
            INST(11, Opcode::Sub).u64().Inputs(1, 0);  // different type
            INST(12, Opcode::Mul).f32().Inputs(4, 6);  // different inputs
            INST(35, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).b().InputsAutoType(7, 8, 9, 10, 11, 12, 35);
            INST(14, Opcode::ReturnVoid);
        }
    }
    // graph does not change
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        PARAMETER(4, 4).f32();
        PARAMETER(5, 5).f32();
        PARAMETER(6, 6).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::Add).u64().Inputs(0, 1);
            INST(8, Opcode::Sub).u32().Inputs(1, 0);
            INST(9, Opcode::Mul).f32().Inputs(4, 5);

            INST(10, Opcode::Sub).u64().Inputs(0, 1);
            INST(11, Opcode::Sub).u64().Inputs(1, 0);
            INST(12, Opcode::Mul).f32().Inputs(4, 6);
            INST(35, Opcode::SaveState).NoVregs();
            INST(13, Opcode::CallStatic).b().InputsAutoType(7, 8, 9, 10, 11, 12, 35);
            INST(14, Opcode::ReturnVoid);
        }
    }

    GraphChecker(GetGraph()).Check();
    ASSERT_FALSE(GetGraph()->RunPass<Cse>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}

TEST_F(CSETest, CSETestNotApply2)
{
    // Instructions in different blocks between which there is no dominating relationship.
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_GE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Add).s32().Inputs(1, 0);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(1, 0);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_GE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(6, 5)
        {
            INST(8, Opcode::Add).s32().Inputs(1, 0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Phi).s32().Inputs({{4, 5}, {3, 2}, {6, 8}});
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }

    // Graph does not change
    Graph *graph_csed = CreateEmptyGraph();
    GRAPH(graph_csed)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_GE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Add).s32().Inputs(1, 0);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(6, Opcode::Compare).b().SrcType(DataType::Type::INT32).Inputs(1, 0);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_GE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(6, 5)
        {
            INST(8, Opcode::Add).s32().Inputs(1, 0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Phi).s32().Inputs({{4, 5}, {3, 2}, {6, 8}});
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }

    GraphChecker(GetGraph()).Check();
    ASSERT_FALSE(GetGraph()->RunPass<Cse>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph_csed));
}
}  // namespace panda::compiler
