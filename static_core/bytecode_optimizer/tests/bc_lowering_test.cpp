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
#include "check_resolver.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"

namespace panda::bytecodeopt::test {

class LoweringTest : public CommonTest {};

// NOLINTBEGIN(readability-magic-numbers)

// Checks the results of lowering pass if negative operands are used
TEST_F(IrBuilderTest, Lowering)
{
    // ISA opcodes with expected lowering IR opcodes
    std::map<std::string, compiler::Opcode> opcodes = {
        {"add", compiler::Opcode::SubI}, {"sub", compiler::Opcode::AddI}, {"mul", compiler::Opcode::MulI},
        {"and", compiler::Opcode::AndI}, {"xor", compiler::Opcode::XorI}, {"or", compiler::Opcode::OrI},
        {"div", compiler::Opcode::DivI}, {"mod", compiler::Opcode::ModI},
    };

    const std::string template_source = R"(
    .function i32 main() {
        movi v0, 0x3
        movi v1, 0xffffffffffffffe2
        OPCODE v0, v1
        return
    }
    )";

    for (auto const &opcode : opcodes) {
        // Specialize template source to the current opcode
        std::string source(template_source);
        size_t start_pos = source.find("OPCODE");
        source.replace(start_pos, 6 /* OPCODE */, opcode.first);

        ASSERT_TRUE(ParseToGraph(source, "main"));
#ifndef NDEBUG
        GetGraph()->SetLowLevelInstructionsEnabled();
#endif
        GetGraph()->RunPass<CheckResolver>();
        GetGraph()->RunPass<compiler::Lowering>();
        GetGraph()->RunPass<compiler::Cleanup>();

        int32_t imm = -30;
        // Note: `AddI -30` is handled as `SubI 30`. `SubI -30` is handled as `AddI 30`.
        if (opcode.second == compiler::Opcode::AddI || opcode.second == compiler::Opcode::SubI) {
            imm = 30;
        }

        auto expected = CreateEmptyGraph();
        GRAPH(expected)
        {
            CONSTANT(1, 3).s32();

            BASIC_BLOCK(2, -1)
            {
                INST(2, opcode.second).s32().Inputs(1).Imm(imm);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }

        EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expected));
    }
}

TEST_F(LoweringTest, AddSub)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f32();
        CONSTANT(3, 12).s32();
        CONSTANT(4, 150).s32();
        CONSTANT(5, 0).s64();
        CONSTANT(6, 1.2F).f32();
        CONSTANT(7, -1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Add).u32().Inputs(0, 3);
            INST(9, Opcode::Sub).u32().Inputs(0, 3);
            INST(10, Opcode::Add).u32().Inputs(0, 4);
            INST(11, Opcode::Sub).u32().Inputs(0, 4);
            INST(12, Opcode::Add).u64().Inputs(1, 5);
            INST(13, Opcode::Sub).f32().Inputs(2, 6);
            INST(14, Opcode::Sub).u32().Inputs(0, 7);
            INST(15, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(8, 9, 10, 11, 12, 13, 14, 15);
            INST(21, Opcode::Return).b().Inputs(20);
        }
    }
#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f32();
        CONSTANT(4, 150).s32();
        CONSTANT(5, 0).s64();
        CONSTANT(6, 1.2F).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(22, Opcode::AddI).u32().Inputs(0).Imm(0xc);
            INST(23, Opcode::SubI).u32().Inputs(0).Imm(0xc);
            INST(10, Opcode::Add).u32().Inputs(0, 4);
            INST(11, Opcode::Sub).u32().Inputs(0, 4);
            INST(12, Opcode::Add).u64().Inputs(1, 5);
            INST(13, Opcode::Sub).f32().Inputs(2, 6);
            INST(24, Opcode::AddI).u32().Inputs(0).Imm(1);
            INST(19, Opcode::SaveState).NoVregs();
            INST(20, Opcode::CallStatic).b().InputsAutoType(22, 23, 10, 11, 12, 13, 24, 19);
            INST(21, Opcode::Return).b().Inputs(20);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

TEST_F(LoweringTest, MulDivMod)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f32();
        CONSTANT(3, 12).s32();
        CONSTANT(4, 150).s32();
        CONSTANT(5, 0).s64();
        CONSTANT(6, 1.2F).f32();
        CONSTANT(7, -1).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Div).s32().Inputs(0, 3);
            INST(9, Opcode::Div).u32().Inputs(0, 4);
            INST(10, Opcode::Div).u64().Inputs(1, 5);
            INST(11, Opcode::Div).f32().Inputs(2, 6);
            INST(12, Opcode::Div).s32().Inputs(0, 7);

            INST(13, Opcode::Mod).s32().Inputs(0, 3);
            INST(14, Opcode::Mod).u32().Inputs(0, 4);
            INST(15, Opcode::Mod).u64().Inputs(1, 5);
            INST(16, Opcode::Mod).f32().Inputs(2, 6);
            INST(17, Opcode::Mod).s32().Inputs(0, 7);

            INST(18, Opcode::Mul).s32().Inputs(0, 3);
            INST(19, Opcode::Mul).u32().Inputs(0, 4);
            INST(20, Opcode::Mul).u64().Inputs(1, 5);
            INST(21, Opcode::Mul).f32().Inputs(2, 6);
            INST(22, Opcode::Mul).s32().Inputs(0, 7);
            INST(31, Opcode::SaveState).NoVregs();
            INST(23, Opcode::CallStatic)
                .b()
                .InputsAutoType(8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 31);
            INST(24, Opcode::Return).b().Inputs(23);
        }
    }

#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).f32();
        CONSTANT(4, 150).s32();
        CONSTANT(5, 0).s64();
        CONSTANT(6, 1.2F).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(25, Opcode::DivI).s32().Inputs(0).Imm(0xc);
            INST(9, Opcode::Div).u32().Inputs(0, 4);
            INST(10, Opcode::Div).u64().Inputs(1, 5);
            INST(11, Opcode::Div).f32().Inputs(2, 6);
            INST(26, Opcode::DivI).s32().Inputs(0).Imm(static_cast<uint64_t>(-1));
            INST(27, Opcode::ModI).s32().Inputs(0).Imm(0xc);
            INST(14, Opcode::Mod).u32().Inputs(0, 4);
            INST(15, Opcode::Mod).u64().Inputs(1, 5);
            INST(16, Opcode::Mod).f32().Inputs(2, 6);
            INST(28, Opcode::ModI).s32().Inputs(0).Imm(static_cast<uint64_t>(-1));
            INST(29, Opcode::MulI).s32().Inputs(0).Imm(0xc);
            INST(19, Opcode::Mul).u32().Inputs(0, 4);
            INST(20, Opcode::Mul).u64().Inputs(1, 5);
            INST(21, Opcode::Mul).f32().Inputs(2, 6);
            INST(30, Opcode::MulI).s32().Inputs(0).Imm(static_cast<uint64_t>(-1));
            INST(31, Opcode::SaveState).NoVregs();
            INST(23, Opcode::CallStatic)
                .b()
                .InputsAutoType(25, 9, 10, 11, 26, 27, 14, 15, 16, 28, 29, 19, 20, 21, 30, 31);
            INST(24, Opcode::Return).b().Inputs(23);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

TEST_F(LoweringTest, Logic)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(24, 1).s64();
        CONSTANT(1, 12).s32();
        CONSTANT(2, 50).s32();
        CONSTANT(25, 0).s64();
        CONSTANT(27, 300).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Or).u32().Inputs(0, 1);
            INST(5, Opcode::Or).u32().Inputs(0, 2);
            INST(6, Opcode::And).u32().Inputs(0, 1);
            INST(8, Opcode::And).u32().Inputs(0, 2);
            INST(9, Opcode::Xor).u32().Inputs(0, 1);
            INST(11, Opcode::Xor).u32().Inputs(0, 2);
            INST(12, Opcode::Or).u8().Inputs(0, 1);
            INST(13, Opcode::And).u32().Inputs(0, 0);
            INST(26, Opcode::And).s64().Inputs(24, 25);
            INST(28, Opcode::Xor).u32().Inputs(0, 27);
            INST(29, Opcode::Or).s64().Inputs(24, 25);
            INST(30, Opcode::Xor).s64().Inputs(24, 25);
            INST(31, Opcode::And).u32().Inputs(0, 27);
            INST(32, Opcode::Or).u32().Inputs(0, 27);
            INST(15, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(3, 5, 6, 8, 9, 11, 12, 13, 26, 28, 29, 30, 31, 32, 15);
            INST(23, Opcode::Return).b().Inputs(14);
        }
    }

#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(24, 1).s64();
        CONSTANT(1, 12).s32();
        CONSTANT(25, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(33, Opcode::OrI).u32().Inputs(0).Imm(0xc);
            INST(34, Opcode::OrI).u32().Inputs(0).Imm(0x32);
            INST(35, Opcode::AndI).u32().Inputs(0).Imm(0xc);
            INST(36, Opcode::AndI).u32().Inputs(0).Imm(0x32);
            INST(37, Opcode::XorI).u32().Inputs(0).Imm(0xc);
            INST(38, Opcode::XorI).u32().Inputs(0).Imm(0x32);
            INST(12, Opcode::Or).u8().Inputs(0, 1);
            INST(13, Opcode::And).u32().Inputs(0, 0);
            INST(26, Opcode::And).s64().Inputs(24, 25);
            INST(39, Opcode::XorI).u32().Inputs(0).Imm(0x12c);
            INST(29, Opcode::Or).s64().Inputs(24, 25);
            INST(30, Opcode::Xor).s64().Inputs(24, 25);
            INST(40, Opcode::AndI).u32().Inputs(0).Imm(0x12c);
            INST(41, Opcode::OrI).u32().Inputs(0).Imm(0x12c);
            INST(15, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(33, 34, 35, 36, 37, 38, 12, 13, 26, 39, 29, 30, 40, 41, 15);
            INST(23, Opcode::Return).b().Inputs(14);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

TEST_F(LoweringTest, Shift)
{
    auto init = CreateEmptyGraph();
    GRAPH(init)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(24, 1).s64();
        CONSTANT(1, 12).s32();
        CONSTANT(2, 64).s32();
        CONSTANT(25, 0).s64();
        CONSTANT(27, 200).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Shr).u32().Inputs(0, 1);
            INST(5, Opcode::Shr).u32().Inputs(0, 2);
            INST(6, Opcode::AShr).u32().Inputs(0, 1);
            INST(8, Opcode::AShr).u32().Inputs(0, 2);
            INST(9, Opcode::Shl).u32().Inputs(0, 1);
            INST(11, Opcode::Shl).u32().Inputs(0, 2);
            INST(12, Opcode::Shl).u8().Inputs(0, 1);
            INST(13, Opcode::Shr).u32().Inputs(0, 0);
            INST(26, Opcode::Shr).s64().Inputs(24, 25);
            INST(28, Opcode::AShr).s32().Inputs(0, 27);
            INST(29, Opcode::AShr).s64().Inputs(24, 25);
            INST(30, Opcode::Shl).s64().Inputs(24, 25);
            INST(31, Opcode::Shr).s32().Inputs(0, 27);
            INST(32, Opcode::Shl).s32().Inputs(0, 27);
            INST(15, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(3, 5, 6, 8, 9, 11, 12, 13, 26, 28, 29, 30, 31, 32, 15);
            INST(23, Opcode::Return).b().Inputs(14);
        }
    }
#ifndef NDEBUG
    init->SetLowLevelInstructionsEnabled();
#endif
    init->RunPass<compiler::Lowering>();
    init->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0, 0).u32();
        PARAMETER(24, 1).s64();
        CONSTANT(1, 12).s32();
        CONSTANT(2, 64).s32();
        CONSTANT(25, 0).s64();
        CONSTANT(27, 200).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(33, Opcode::ShrI).u32().Inputs(0).Imm(0xc);
            INST(5, Opcode::Shr).u32().Inputs(0, 2);
            INST(34, Opcode::AShrI).u32().Inputs(0).Imm(0xc);
            INST(8, Opcode::AShr).u32().Inputs(0, 2);
            INST(35, Opcode::ShlI).u32().Inputs(0).Imm(0xc);
            INST(11, Opcode::Shl).u32().Inputs(0, 2);
            INST(12, Opcode::Shl).u8().Inputs(0, 1);
            INST(13, Opcode::Shr).u32().Inputs(0, 0);
            INST(26, Opcode::Shr).s64().Inputs(24, 25);
            INST(28, Opcode::AShr).s32().Inputs(0, 27);
            INST(29, Opcode::AShr).s64().Inputs(24, 25);
            INST(30, Opcode::Shl).s64().Inputs(24, 25);
            INST(31, Opcode::Shr).s32().Inputs(0, 27);
            INST(32, Opcode::Shl).s32().Inputs(0, 27);
            INST(15, Opcode::SaveState).NoVregs();
            INST(14, Opcode::CallStatic).b().InputsAutoType(33, 5, 34, 8, 35, 11, 12, 13, 26, 28, 29, 30, 31, 32, 15);
            INST(23, Opcode::Return).b().Inputs(14);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(init, expected));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace panda::bytecodeopt::test
