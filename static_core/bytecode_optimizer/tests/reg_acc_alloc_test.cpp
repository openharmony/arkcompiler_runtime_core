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

#include "common.h"
#include "bytecode_optimizer/reg_acc_alloc.h"
#include "bytecode_optimizer/optimize_bytecode.h"

namespace panda::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

class RegAccAllocTest : public CommonTest {
public:
    void CheckInstructionsDestRegIsAcc(std::vector<int> &&inst_ids)
    {
        for (auto id : inst_ids) {
            ASSERT_EQ(INS(id).GetDstReg(), compiler::ACC_REG_ID);
        }
    }

    void CheckInstructionsSrcRegIsAcc(std::vector<int> &&inst_ids)
    {
        for (auto id : inst_ids) {
            uint8_t idx = 0;
            switch (INS(id).GetOpcode()) {
                case compiler::Opcode::LoadArray:
                case compiler::Opcode::StoreObject:
                case compiler::Opcode::StoreStatic:
                    idx = 1;
                    break;
                case compiler::Opcode::StoreArray:
                    idx = 2;
                    break;
                default:
                    break;
            }

            ASSERT_EQ(INS(id).GetSrcReg(idx), compiler::ACC_REG_ID);
        }
    }
};

/*
 * Test if two arithmetic instructions follow each other.
 */
TEST_F(RegAccAllocTest, ArithmeticInstructions)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 10).s32();
        CONSTANT(2, 20).s32();

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::Mul).s32().Inputs(0, 2);
            INST(4, Opcode::Add).s32().Inputs(3, 1);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({3, 4});
    CheckInstructionsSrcRegIsAcc({4, 5});
}

/*
 * Test if two arithmetic instructions follow each other.
 * Take the advantage of commutativity.
 */
TEST_F(RegAccAllocTest, Commutativity1)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 10).s32();
        CONSTANT(2, 20).s32();

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::Mul).s32().Inputs(0, 2);
            INST(4, Opcode::Add).s32().Inputs(1, 3);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({3, 4});
    CheckInstructionsSrcRegIsAcc({4, 5});
}

/*
 * Test if two arithmetic instructions follow each other.
 * Cannot take the advantage of commutativity at the Sub instruction.
 */
TEST_F(RegAccAllocTest, Commutativity2)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 10).s32();
        CONSTANT(2, 20).s32();

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::Mul).s32().Inputs(0, 2);
            INST(4, Opcode::Sub).s32().Inputs(1, 3);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(5, Opcode::Return).s32().Inputs(4);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4});
    CheckInstructionsSrcRegIsAcc({5});
}

/*
 * Test if the first arithmetic instruction has multiple users.
 * That (3, Opcode::Mul) is spilled out to register becasue the subsequent
 * instruction (4, Opcode::Add) makes the accumulator dirty.
 */
TEST_F(RegAccAllocTest, ArithmeticInstructionsWithDirtyAccumulator)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 10).s32();
        CONSTANT(2, 20).s32();

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::Mul).s32().Inputs(0, 2);
            INST(4, Opcode::Add).s32().Inputs(1, 3);
            INST(5, Opcode::Sub).s32().Inputs(3, 4);
            INST(6, Opcode::Mul).s32().Inputs(5, 3);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5, 6});
    CheckInstructionsSrcRegIsAcc({6, 7});
}

/*
 * Test if arithmetic instructions are used as Phi inputs.
 *
 * Test Graph:
 *              [0]
 *               |
 *               v
 *         /----[2]----\
 *         |           |
 *         v           v
 *        [3]         [4]
 *         |           |
 *         \--->[4]<---/
 *               |
 *               v
 *             [exit]
 */
TEST_F(RegAccAllocTest, SimplePhi)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 10).s32();
        CONSTANT(2, 20).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Add).s32().Inputs(1, 2);
            INST(5, Opcode::Compare).b().Inputs(4, 0);
            INST(6, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(8, Opcode::Sub).s32().Inputs(4, 0);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(9, Opcode::Add).s32().Inputs(4, 0);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::Phi).s32().Inputs({{3, 8}, {4, 9}});
            INST(11, Opcode::Add).s32().Inputs(4, 10);
            INST(7, Opcode::Return).s32().Inputs(11);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5, 8, 9, 11});
    CheckInstructionsSrcRegIsAcc({6, 7, 11});
}

/*
 * Test Cast instructions which follow each other.
 * Each instruction refers to the previos one.
 */
TEST_F(RegAccAllocTest, CastInstructions)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).f64().SrcType(compiler::DataType::UINT32).Inputs(0);
            INST(2, Opcode::Cast).s32().SrcType(compiler::DataType::FLOAT64).Inputs(1);
            INST(3, Opcode::Cast).s16().SrcType(compiler::DataType::INT32).Inputs(2);
            INST(4, Opcode::Return).s16().Inputs(3);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({1, 2, 3});
    CheckInstructionsSrcRegIsAcc({2, 3, 4});
}

/*
 * Test Abs and Sqrt instructions.
 * Each instruction refers to the previous instruction.
 */
TEST_F(RegAccAllocTest, AbsAndSqrtInstructions)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1.22).f64();
        PARAMETER(1, 10).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Sub).f64().Inputs(0, 1);
            INST(3, Opcode::Abs).f64().Inputs(2);
            INST(4, Opcode::Sqrt).f64().Inputs(3);
            INST(5, Opcode::Return).f64().Inputs(4);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4});
    CheckInstructionsSrcRegIsAcc({5});
}

/*
 * Test LoadArray instruction that reads accumulator as second input.
 * Note: most instructions read accumulator as first input.
 */
TEST_F(RegAccAllocTest, LoadArrayInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 1).s32();
        PARAMETER(1, 10).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1, 0).SrcVregs({0, 1, 2});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).s32().Inputs(3, 5);
            INST(7, Opcode::Cast).f64().SrcType(compiler::DataType::INT32).Inputs(6);
            INST(8, Opcode::Return).f64().Inputs(7);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4, 6, 7});
    CheckInstructionsSrcRegIsAcc({5, 7, 8});
}

/*
 * Test throw instruction.
 * Currently, just linear block-flow is supported.
 * Nothing happens in this test.
 */
TEST_F(RegAccAllocTest, ThrowInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 0).s32();

        BASIC_BLOCK(2, 3)
        {
            INST(2, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::NewObject).ref().Inputs(3, 2);
        }
        BASIC_BLOCK(3, 4, 5, 6)
        {
            INST(5, Opcode::Try).CatchTypeIds({0x0, 0xE1});
        }
        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::SaveState).Inputs(4).SrcVregs({0});
            INST(7, Opcode::Throw).Inputs(4, 6);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(8, Opcode::Return).b().Inputs(1);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(9, Opcode::Return).b().Inputs(0);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
}

/*
 * Test Phi instruction in loop.
 * This test is copied from reg_alloc_linear_scan_test.cpp file.
 */
TEST_F(RegAccAllocTest, PhiInstructionInLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 10).s32();
        CONSTANT(2, 20).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).s32().Inputs({{0, 0}, {3, 8}});
            INST(4, Opcode::Phi).s32().Inputs({{0, 1}, {3, 9}});
            INST(5, Opcode::SafePoint).Inputs(0, 3, 4).SrcVregs({0, 1, 2});
            INST(6, Opcode::Compare).b().Inputs(4, 0);
            INST(7, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(8, Opcode::Mul).s32().Inputs(3, 4);
            INST(9, Opcode::Sub).s32().Inputs(4, 0);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Add).s32().Inputs(2, 3);
            INST(11, Opcode::Return).s32().Inputs(10);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({6, 10});
    CheckInstructionsSrcRegIsAcc({7, 11});
}

/*
 * Test multiple branches.
 *
 * Test Graph:
 *
 *            /---[2]---\
 *            |         |
 *            v         v
 *           [3]<------[4]
 *                      |
 *                      v
 *                     [5]
 */
TEST_F(RegAccAllocTest, MultipleBranches)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u16();
        PARAMETER(1, 1).s64();
        CONSTANT(2, 0x20).s64();
        CONSTANT(3, 0x25).s64();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(5, Opcode::Compare).b().CC(compiler::CC_GT).Inputs(0, 2);
            INST(6, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0).Inputs(5);
        }
        // v0 <= 0x20
        BASIC_BLOCK(4, 3, 5)
        {
            INST(8, Opcode::Compare).b().CC(compiler::CC_GT).Inputs(1, 3);
            INST(9, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0).Inputs(8);
        }
        // v0 <= 0x20 && v1 <= 0x25
        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::Return).u16().Inputs(0);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(14, Opcode::Return).u16().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5, 8});
    CheckInstructionsSrcRegIsAcc({6, 9});
}

/*
 * Test Phi with multiple inputs.
 * Phi cannot be optimized because one of the inputs is a CONSTANT.
 *
 * Test Graph:
 *
 *          /---[2]---\
 *          |         |
 *          v         |
 *      /--[3]--\     |
 *      |       |     |
 *      v       v     |
 *     [4]     [5]    |
 *      |       |     |
 *      |       v     |
 *      \----->[6]<---/
 *              |
 *            [exit]
 */
TEST_F(RegAccAllocTest, PhiWithMultipleInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();
        BASIC_BLOCK(2, 3, 6)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Mul).s32().Inputs(0, 0);
            INST(5, Opcode::Compare).b().Inputs(4, 1);
            INST(6, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(4, 6)
        {
            INST(7, Opcode::Mul).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(8, Opcode::Add).s32().Inputs(4, 1);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(9, Opcode::Phi).s32().Inputs({{2, 0}, {4, 7}, {5, 8}});
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({2, 5});
    CheckInstructionsSrcRegIsAcc({3, 6});
}

/*
 * Test for Phi. Phi cannot be optimized because the subsequent
 * Compare instruction makes the accumulator dirty.
 */
TEST_F(RegAccAllocTest, PhiWithSubsequentCompareInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 10).s32();
        CONSTANT(2, 20).s32();

        BASIC_BLOCK(2, 3)
        {
            INST(3, Opcode::Add).s32().Inputs(0, 1);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(4, Opcode::Phi).s32().Inputs({{2, 3}, {4, 8}});
            INST(5, Opcode::SafePoint).Inputs(0, 4).SrcVregs({0, 1});
            INST(6, Opcode::Compare).b().Inputs(4, 0);
            INST(7, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(4, 3)
        {
            INST(8, Opcode::Mul).s32().Inputs(3, 2);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Add).s32().Inputs(2, 4);
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({6, 9});
    CheckInstructionsSrcRegIsAcc({7, 10});
}

/*
 * A switch-case example. There are different arithmetic instructions
 * in the case blocks. These instructions are the inputs of a Phi.
 */
TEST_F(RegAccAllocTest, SwitchCase)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0x1).s32();
        CONSTANT(5, 0xa).s32();
        CONSTANT(8, 0x2).s32();
        CONSTANT(10, 0x3).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(3, Opcode::SaveState).Inputs(2, 1, 0).SrcVregs({0, 1, 2});
            INST(4, Opcode::CallStatic)
                .s32()
                .Inputs({{compiler::DataType::REFERENCE, 2}, {compiler::DataType::NO_TYPE, 3}});
            INST(7, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(1, 4);
        }

        BASIC_BLOCK(4, 5, 6)
        {
            INST(9, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(8, 4);
        }

        BASIC_BLOCK(6, 7, 9)
        {
            INST(11, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(10, 4);
        }

        BASIC_BLOCK(9, 10)
        {
            INST(12, Opcode::Mod).s32().Inputs(5, 4);
        }

        BASIC_BLOCK(7, 10)
        {
            INST(13, Opcode::Mul).s32().Inputs(4, 5);
        }

        BASIC_BLOCK(5, 10)
        {
            INST(14, Opcode::Add).s32().Inputs(4, 5);
        }

        BASIC_BLOCK(3, 10)
        {
            INST(15, Opcode::Sub).s32().Inputs(5, 4);
        }

        BASIC_BLOCK(10, -1)
        {
            INST(16, Opcode::Phi).s32().Inputs({{3, 15}, {5, 14}, {7, 13}, {9, 12}});
            INST(17, Opcode::Return).s32().Inputs(16);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({12, 13, 14, 15, 16});
    CheckInstructionsSrcRegIsAcc({17});
}

/*
 * This test creates an array and does modifications in that.
 */
TEST_F(RegAccAllocTest, CreateArray)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0x2).s32();
        CONSTANT(4, 0x1).s32();
        CONSTANT(10, 0x0).s32();
        CONSTANT(11, 0xa).s32();
        CONSTANT(20, 0xc).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(1, 0).SrcVregs({0, 6});
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(2).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 1, 2);
            INST(5, Opcode::LoadArray).ref().Inputs(0, 4);
            INST(6, Opcode::SaveState).Inputs(5, 4, 3, 0).SrcVregs({0, 1, 4, 6});
            INST(7, Opcode::CallStatic)
                .s32()
                .Inputs({{compiler::DataType::REFERENCE, 5}, {compiler::DataType::NO_TYPE, 6}});
            INST(9, Opcode::Add).s32().Inputs(7, 1);
            INST(12, Opcode::StoreArray).s32().Inputs(5, 10, 11);
            INST(14, Opcode::Add).s32().Inputs(7, 20);
            INST(15, Opcode::StoreArray).s32().Inputs(3, 4, 14);
            INST(17, Opcode::Mul).s32().Inputs(14, 9);
            INST(18, Opcode::Return).s32().Inputs(17);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({17});
    CheckInstructionsSrcRegIsAcc({9, 15, 18});
}

/*
 * Test StoreObject instruction that reads accumulator as second input.
 * Note: most instructions read accumulator as first input.
 */
TEST_F(RegAccAllocTest, StoreObjectInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, nullptr).ref();
        CONSTANT(1, 0x2a).s64();
        CONSTANT(2, 0x1).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 2).SrcVregs({0, 1});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::Add).s64().Inputs(1, 2);
            INST(6, Opcode::StoreObject).s64().Inputs(4, 5);
            INST(7, Opcode::Return).s64().Inputs(1);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({5});
    CheckInstructionsSrcRegIsAcc({6});
}

/*
 * Test StoreStatic instruction that reads accumulator as second input.
 * Note: most instructions read accumulator as first input.
 */
TEST_F(RegAccAllocTest, StoreStaticInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::Add).s32().Inputs(0, 1);
            INST(5, Opcode::StoreStatic).s32().Inputs(3, 4).Volatile();
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    CheckInstructionsDestRegIsAcc({4});
    CheckInstructionsSrcRegIsAcc({5});
}

/*
 * Test if Phi uses Phi as input.
 * This case is not supported right now.
 */
TEST_F(RegAccAllocTest, PhiUsesPhiAsInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0x1).s32();
        CONSTANT(5, 0xa).s32();
        CONSTANT(8, 0x2).s32();
        CONSTANT(10, 0x3).s32();

        BASIC_BLOCK(2, 3, 12)
        {
            INST(2, Opcode::LoadArray).ref().Inputs(0, 1);
            INST(3, Opcode::SaveState).Inputs(2, 1, 0).SrcVregs({0, 1, 2});
            INST(4, Opcode::CallStatic)
                .s32()
                .Inputs({{compiler::DataType::REFERENCE, 2}, {compiler::DataType::NO_TYPE, 3}});
            INST(23, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(5, 4);
        }

        BASIC_BLOCK(3, 4, 5)
        {
            INST(7, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(1, 4);
        }

        BASIC_BLOCK(5, 6, 7)
        {
            INST(9, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(8, 4);
        }

        BASIC_BLOCK(7, 8, 10)
        {
            INST(11, Opcode::If).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Inputs(10, 4);
        }

        BASIC_BLOCK(10, 11)
        {
            INST(12, Opcode::Mod).s32().Inputs(5, 4);
        }

        BASIC_BLOCK(8, 11)
        {
            INST(13, Opcode::Mul).s32().Inputs(4, 5);
        }

        BASIC_BLOCK(6, 11)
        {
            INST(14, Opcode::Add).s32().Inputs(4, 5);
        }

        BASIC_BLOCK(4, 11)
        {
            INST(15, Opcode::Sub).s32().Inputs(5, 4);
        }

        BASIC_BLOCK(11, 13)
        {
            INST(16, Opcode::Phi).s32().Inputs({{4, 15}, {6, 14}, {8, 13}, {10, 12}});
        }

        BASIC_BLOCK(12, 13)
        {
            INST(17, Opcode::Sub).s32().Inputs(5, 1);
        }

        BASIC_BLOCK(13, -1)
        {
            INST(18, Opcode::Phi).s32().Inputs({{11, 16}, {12, 17}});
            INST(19, Opcode::Return).s32().Inputs(18);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
}

/*
 * This test covers case with LoadObject which user can not read from accumulator
 */
TEST_F(RegAccAllocTest, NotUseAccDstRegLoadObject)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).s32().Inputs(0);
            INST(6, Opcode::LoadObject).ref().Inputs(0);
            INST(7, Opcode::SaveState).NoVregs();
            INST(9, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 6}, {NO_TYPE, 7}});
            INST(24, Opcode::If).CC(compiler::ConditionCode::CC_NE).SrcType(INT32).Inputs(9, 3);
        }

        BASIC_BLOCK(4, -1)
        {
            CONSTANT(25, 0xffff).s32();
            INST(13, Opcode::Return).u16().Inputs(25);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(16, Opcode::LoadObject).ref().Inputs(0);
            INST(19, Opcode::LoadObject).s32().Inputs(0);
            INST(20, Opcode::SaveState).NoVregs();
            INST(22, Opcode::CallVirtual).u16().Inputs({{REFERENCE, 16}, {INT32, 19}, {NO_TYPE, 20}});
            INST(23, Opcode::Return).u16().Inputs(22);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    ASSERT_NE(INS(3).GetDstReg(), compiler::ACC_REG_ID);
    ASSERT_EQ(INS(6).GetDstReg(), compiler::ACC_REG_ID);
    ASSERT_NE(INS(16).GetDstReg(), compiler::ACC_REG_ID);
    ASSERT_EQ(INS(19).GetDstReg(), compiler::ACC_REG_ID);
}

/*
 * Test checks if accumulator gets dirty between input and inst where input is LoadObject
 */
TEST_F(RegAccAllocTest, IsAccWriteBetweenLoadObject)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::LoadObject).s32().Inputs(0);
            INST(6, Opcode::IfImm).CC(compiler::ConditionCode::CC_NE).SrcType(INT32).Inputs(3).Imm(0);
        }

        BASIC_BLOCK(4, -1)
        {
            CONSTANT(22, 0xffff).s32();
            INST(8, Opcode::Return).u16().Inputs(22);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(21, Opcode::SubI).s32().Imm(1).Inputs(3);
            INST(16, Opcode::StoreObject).s32().Inputs(0, 21);
            INST(17, Opcode::SaveState).NoVregs();
            INST(19, Opcode::CallVirtual).u16().Inputs({{REFERENCE, 0}, {NO_TYPE, 17}});
            INST(20, Opcode::Return).u16().Inputs(19);
        }
    }

    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    ASSERT_NE(INS(3).GetDstReg(), compiler::ACC_REG_ID);
}

/*
 * Test calls with accumulator
 */
TEST_F(RegAccAllocTest, CallWithAcc)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;

        CONSTANT(0, 0).s32();
        CONSTANT(1, 1).s32();
        CONSTANT(2, 2).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::LoadString).ref().Inputs(3).TypeId(42);
            INST(5, Opcode::CallStatic)
                .v0id()
                .Inputs({{INT32, 0}, {INT32, 1}, {INT32, 2}, {REFERENCE, 4}, {NO_TYPE, 3}});
            INST(6, Opcode::LoadString).ref().Inputs(3).TypeId(43);
            INST(7, Opcode::CallStatic).v0id().Inputs({{INT32, 0}, {INT32, 1}, {REFERENCE, 6}, {NO_TYPE, 3}});
            INST(8, Opcode::LoadString).ref().Inputs(3).TypeId(44);
            INST(9, Opcode::CallStatic).v0id().Inputs({{INT32, 0}, {REFERENCE, 8}, {NO_TYPE, 3}});
            INST(10, Opcode::LoadString).ref().Inputs(3).TypeId(45);
            INST(11, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 10}, {NO_TYPE, 3}});
            INST(12, Opcode::ReturnVoid).v0id();
        }
    }
    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    EXPECT_EQ(INS(4).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(6).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(8).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(10).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();

        BASIC_BLOCK(2, 3, 4)
        {
            CONSTANT(1, 3).s32();
            INST(10, Opcode::If).CC(compiler::CC_GE).SrcType(compiler::DataType::INT32).Inputs(1, 0);
        }
        BASIC_BLOCK(3, -1)
        {
            CONSTANT(9, 8).s32();
            INST(7, Opcode::Return).s32().Inputs(9);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::Return).s32().Inputs(0);
        }
    }
    EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

    EXPECT_EQ(INS(1).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai_Call)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        BASIC_BLOCK(2, -1)
        {
            CONSTANT(1, 1).s32();
            INST(2, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5, Opcode::InitObject).ref().Inputs({{REFERENCE, 3}, {INT32, 1}, {NO_TYPE, 4}});
            INST(6, Opcode::SaveState).Inputs().SrcVregs({});
            CONSTANT(7, 11).s32();
            INST(8, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5}, {INT32, 7}, {NO_TYPE, 6}});

            INST(9, Opcode::ReturnVoid).v0id();
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_NE(INS(1).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(5).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_NE(INS(7).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai_Call_2)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        BASIC_BLOCK(2, -1)
        {
            CONSTANT(1, 1).s32();
            INST(2, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3, Opcode::LoadAndInitClass).ref().Inputs(2);
            INST(4, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5, Opcode::InitObject).ref().Inputs({{REFERENCE, 3}, {INT32, 1}, {NO_TYPE, 4}});
            INST(6, Opcode::SaveState).Inputs().SrcVregs({});
            CONSTANT(7, 11).s32();
            CONSTANT(8, 11).s32();
            INST(9, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5}, {INT32, 7}, {INT32, 8}, {NO_TYPE, 6}});

            INST(10, Opcode::Return).ref().Inputs(5);
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_EQ(INS(7).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_NE(INS(8).GetDstReg(), compiler::ACC_REG_ID);
}

TEST_F(RegAccAllocTest, Ldai_Cast)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).s32().SrcType(compiler::DataType::UINT64).Inputs(0);
            CONSTANT(2, 159).s32();
            INST(3, Opcode::Add).s32().Inputs(1, 2);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_EQ(INS(1).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(3).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(3).GetInput(1).GetInst()->GetOpcode(), Opcode::Constant);
}

TEST_F(RegAccAllocTest, Ldai_Cast2)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        BASIC_BLOCK(2, -1)
        {
            CONSTANT(2, 159).s32();
            INST(1, Opcode::Cast).s32().SrcType(compiler::DataType::INT64).Inputs(0);
            INST(3, Opcode::Add).s32().Inputs(2, 1);
            INST(4, Opcode::Return).s32().Inputs(3);
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_NE(INS(2).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(1).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(3).GetInput(0).GetInst()->GetOpcode(), Opcode::Cast);
    EXPECT_EQ(INS(3).GetInput(1).GetInst()->GetOpcode(), Opcode::Constant);
}

TEST_F(RegAccAllocTest, Ldai_Exist)
{
    pandasm::Parser p;
    auto source = std::string(R"(
        .record A {
            f32 a1 <static>
        }
        .function i32 main(){
            fmovi v1, 0x42280000
            lda v1
            ststatic A.a1
            lda v1
            f32toi32
            return
        }
        )");

    auto res = p.Parse(source);
    auto &program = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    std::string file_name = "Ldai_Exist";
    auto pfile = pandasm::AsmEmitter::Emit(file_name, program, nullptr, &maps);
    ASSERT_NE(pfile, false);

    auto old_options = panda::bytecodeopt::OPTIONS;
    panda::bytecodeopt::OPTIONS = panda::bytecodeopt::Options("--opt-level=2");
    EXPECT_TRUE(OptimizeBytecode(&program, &maps, file_name, false, true));
    panda::bytecodeopt::OPTIONS = old_options;
    bool fldai_exists = false;
    for (const auto &inst : program.function_table.find("main:()")->second.ins) {
        if (inst.opcode == panda::pandasm::Opcode::FLDAI) {
            fldai_exists = true;
            break;
        }
    }
    EXPECT_EQ(fldai_exists, true);
}

TEST_F(RegAccAllocTest, Lda_Extra1)
{
    pandasm::Parser p;
    auto source = std::string(R"(
        .function u1 main(){
            fldai 0x42280000
            f32toi32
            sta v1
            movi v2, 0x2a
            lda v1
            jeq v2, jump_label_0
            ldai 0x1
            return
        jump_label_0:
            ldai 0x0
            return
        }
        )");

    auto res = p.Parse(source);
    auto &program = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    std::string file_name = "Lda_Extra1";
    auto pfile = pandasm::AsmEmitter::Emit(file_name, program, nullptr, &maps);
    ASSERT_NE(pfile, false);

    auto old_options = panda::bytecodeopt::OPTIONS;
    panda::bytecodeopt::OPTIONS = panda::bytecodeopt::Options("--opt-level=2");
    EXPECT_TRUE(OptimizeBytecode(&program, &maps, file_name, false, true));
    panda::bytecodeopt::OPTIONS = old_options;
    bool lda_exists = false;
    for (const auto &inst : program.function_table.find("main:()")->second.ins) {
        if (inst.opcode == panda::pandasm::Opcode::LDA) {
            lda_exists = true;
            break;
        }
    }
    EXPECT_EQ(lda_exists, false);
}

TEST_F(RegAccAllocTest, Lda_Extra2)
{
    pandasm::Parser p;
    auto source = std::string(R"(
        .function i32[] main(i32 a0){
            movi v0, 0x4
            newarr v4, v0, i32[]
            mov v1, a0
            add v0, a0
            sta v0
            movi v2, 0x1
            lda v0
            starr v4, v2
            lda v1
            add2 a0
            sta v0
            movi v2, 0x2
            lda v0
            starr v4, v2
            lda v1
            add2 a0
            sta v0
            movi v1, 0x3
            lda v0
            starr v4, v1
            lda.obj v4
            return.obj
        }
        )");

    auto res = p.Parse(source);
    auto &program = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    std::string file_name = "Lda_Extra2";
    auto pfile = pandasm::AsmEmitter::Emit(file_name, program, nullptr, &maps);
    ASSERT_NE(pfile, false);

    auto old_options = panda::bytecodeopt::OPTIONS;
    panda::bytecodeopt::OPTIONS = panda::bytecodeopt::Options("--opt-level=2");
    EXPECT_TRUE(OptimizeBytecode(&program, &maps, file_name, false, true));
    panda::bytecodeopt::OPTIONS = old_options;
    int lda_amount = 0;
    for (const auto &inst : program.function_table.find("main:(i32)")->second.ins) {
        if (inst.opcode == panda::pandasm::Opcode::LDA) {
            lda_amount += 1;
        }
    }
    EXPECT_EQ(lda_amount, 1);
}

TEST_F(RegAccAllocTest, Const_Phi)
{
    auto graph = GetAllocator()->New<compiler::Graph>(GetAllocator(), GetLocalAllocator(), Arch::X86_64, false, true);
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(8, 0).f64();
        BASIC_BLOCK(2, 3)
        {
            CONSTANT(1, 6).f64();
        }
        BASIC_BLOCK(3, 4, 5)
        {
            INST(3, Opcode::Phi).Inputs(0, 7).s32();
            INST(6, Opcode::Phi).Inputs(8, 1).f64();
            INST(4, Opcode::Cmp).s32().SrcType(compiler::DataType::FLOAT64).Fcmpg(true).Inputs(6, 8);
            INST(5, Opcode::IfImm).SrcType(compiler::DataType::INT32).CC(compiler::CC_GE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(7, Opcode::AddI).Imm(1).Inputs(3).s32();
        }
        BASIC_BLOCK(5, -1)
        {
            INST(9, Opcode::Return).Inputs(3).s32();
        }
    }
    graph->RunPass<bytecodeopt::RegAccAlloc>();

    EXPECT_NE(INS(6).GetDstReg(), compiler::ACC_REG_ID);
    EXPECT_EQ(INS(4).GetDstReg(), compiler::ACC_REG_ID);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace panda::bytecodeopt::test
