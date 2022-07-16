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

#include "assembler/assembly-emitter.h"
#include "canonicalization.h"
#include "codegen.h"
#include "common.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "reg_encoder.h"

namespace panda::bytecodeopt::test {

TEST_F(CommonTest, RegEncoderF32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0.0).f32();
        CONSTANT(1, 0.0).f32();
        CONSTANT(2, 0.0).f32();
        CONSTANT(3, 0.0).f32();
        CONSTANT(4, 0.0).f32();
        CONSTANT(5, 0.0).f32();
        CONSTANT(6, 0.0).f32();
        CONSTANT(7, 0.0).f32();
        CONSTANT(8, 0.0).f32();
        CONSTANT(9, 0.0).f32();
        CONSTANT(10, 0.0).f32();
        CONSTANT(11, 0.0).f32();
        CONSTANT(12, 0.0).f32();
        CONSTANT(13, 0.0).f32();
        CONSTANT(14, 0.0).f32();
        CONSTANT(15, 0.0).f32();
        CONSTANT(16, 0.0).f32();
        CONSTANT(17, 0.0).f32();
        CONSTANT(18, 0.0).f32();
        CONSTANT(19, 0.0).f32();
        CONSTANT(20, 0.0).f32();
        CONSTANT(21, 0.0).f32();
        CONSTANT(22, 0.0).f32();
        CONSTANT(23, 0.0).f32();
        CONSTANT(24, 0.0).f32();
        CONSTANT(25, 0.0).f32();
        CONSTANT(26, 0.0).f32();
        CONSTANT(27, 0.0).f32();
        CONSTANT(28, 0.0).f32();
        CONSTANT(29, 0.0).f32();
        CONSTANT(30, 0.0).f32();
        CONSTANT(31, 0.0).f32();

        CONSTANT(32, 1.0).f64();
        CONSTANT(33, 2.0).f64();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;
            INST(40, Opcode::Sub).f64().Inputs(32, 33);
            INST(41, Opcode::Return).f64().Inputs(40);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(32, 1.0).f64();
        CONSTANT(33, 2.0).f64();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;
            INST(40, Opcode::Sub).f64().Inputs(32, 33);
            INST(41, Opcode::Return).f64().Inputs(40);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(CommonTest, RegEncoderHoldingSpillFillInst)
{
    RuntimeInterfaceMock interface(2);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&interface);
    GRAPH(graph)
    {
        using namespace compiler::DataType;

        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).b();
        CONSTANT(26, 0xfffffffffffffffa).s64();
        CONSTANT(27, 0x6).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::LoadObject).s64().Inputs(0);
            INST(10, Opcode::LoadObject).s64().Inputs(0);
            INST(11, Opcode::Add).s64().Inputs(10, 4);
            CONSTANT(52, 0x5265c00).s64();
            INST(15, Opcode::Div).s64().Inputs(11, 52);
            INST(16, Opcode::Mul).s64().Inputs(15, 52);
            INST(20, Opcode::Sub).s64().Inputs(16, 10);
            CONSTANT(53, 0x2932e00).s64();
            INST(22, Opcode::Add).s64().Inputs(20, 53);
            INST(25, Opcode::IfImm).SrcType(BOOL).CC(compiler::CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 4) {}

        BASIC_BLOCK(4, -1)
        {
            INST(28, Opcode::Phi).s64().Inputs(26, 27);
            CONSTANT(54, 0x36ee80).s64();
            INST(31, Opcode::Mul).s64().Inputs(28, 54);
            INST(32, Opcode::Add).s64().Inputs(31, 22);
            INST(33, Opcode::SaveState).NoVregs();
            INST(35, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0}, {INT64, 32}, {NO_TYPE, 33}});
            INST(36, Opcode::SaveState).NoVregs();
            INST(37, Opcode::LoadAndInitClass).ref().Inputs(36);
            INST(39, Opcode::SaveState).NoVregs();
            INST(58, Opcode::InitObject).ref().Inputs({{REFERENCE, 37}, {REFERENCE, 0}, {NO_TYPE, 39}});
            CONSTANT(55, 0.0093026).f64();
            CONSTANT(56, 0.0098902).f64();
            CONSTANT(57, 0x1388).s64();
            INST(45, Opcode::SaveState).NoVregs();
            INST(47, Opcode::CallStatic)
                .s64()
                .Inputs({{REFERENCE, 0},
                         {REFERENCE, 58},
                         {BOOL, 1},
                         {FLOAT64, 55},
                         {FLOAT64, 56},
                         {INT64, 57},
                         {NO_TYPE, 45}});
            INST(48, Opcode::SaveState).NoVregs();
            INST(50, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0}, {INT64, 4}, {NO_TYPE, 48}});
            INST(51, Opcode::Return).s64().Inputs(47);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        using namespace compiler::DataType;

        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).b();
        CONSTANT(26, 0xfffffffffffffffa).s64();
        CONSTANT(27, 0x6).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::LoadObject).s64().Inputs(0);
            INST(10, Opcode::LoadObject).s64().Inputs(0);
            INST(11, Opcode::Add).s64().Inputs(10, 4);
            CONSTANT(52, 0x5265c00).s64();
            INST(15, Opcode::Div).s64().Inputs(11, 52);
            INST(16, Opcode::Mul).s64().Inputs(15, 52);
            INST(20, Opcode::Sub).s64().Inputs(16, 10);
            CONSTANT(53, 0x2932e00).s64();
            INST(22, Opcode::Add).s64().Inputs(20, 53);
            INST(25, Opcode::IfImm).SrcType(BOOL).CC(compiler::CC_EQ).Imm(0).Inputs(1);
        }

        BASIC_BLOCK(3, 4)
        {
            // SpillFill added
            INST(60, Opcode::SpillFill);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(28, Opcode::Phi).s64().Inputs(26, 27);
            CONSTANT(54, 0x36ee80).s64();
            INST(31, Opcode::Mul).s64().Inputs(28, 54);
            INST(32, Opcode::Add).s64().Inputs(31, 22);
            INST(33, Opcode::SaveState).NoVregs();
            INST(35, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0}, {INT64, 32}, {NO_TYPE, 33}});
            INST(36, Opcode::SaveState).NoVregs();
            INST(37, Opcode::LoadAndInitClass).ref().Inputs(36);
            INST(39, Opcode::SaveState).NoVregs();
            INST(58, Opcode::InitObject).ref().Inputs({{REFERENCE, 37}, {REFERENCE, 0}, {NO_TYPE, 39}});
            CONSTANT(55, 0.0093026).f64();
            CONSTANT(56, 0.0098902).f64();
            CONSTANT(57, 0x1388).s64();
            INST(45, Opcode::SaveState).NoVregs();

            // SpillFill added
            INST(70, Opcode::SpillFill);
            INST(47, Opcode::CallStatic)
                .s64()
                .Inputs({{REFERENCE, 0},
                         {REFERENCE, 58},
                         {BOOL, 1},
                         {FLOAT64, 55},
                         {FLOAT64, 56},
                         {INT64, 57},
                         {NO_TYPE, 45}});
            INST(48, Opcode::SaveState).NoVregs();
            INST(50, Opcode::CallVirtual).v0id().Inputs({{REFERENCE, 0}, {INT64, 4}, {NO_TYPE, 48}});
            INST(51, Opcode::Return).s64().Inputs(47);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(CommonTest, RegEncoderStoreObject)
{
    // This test covers function CheckWidthVisitor::VisitStoreObject
    RuntimeInterfaceMock interface(4);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&interface);
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        PARAMETER(3, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;

            INST(4, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 0}, {NO_TYPE, 4}});
            INST(9, Opcode::StoreObject).ref().Inputs(0, 2);
            INST(12, Opcode::StoreObject).s32().Inputs(0, 3);
            INST(15, Opcode::LoadObject).ref().Inputs(1);
            INST(16, Opcode::SaveState).NoVregs();
            INST(18, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 15}, {NO_TYPE, 16}});
            INST(19, Opcode::SaveState).NoVregs();
            INST(21, Opcode::LoadClass).ref().Inputs(19);
            INST(20, Opcode::CheckCast).Inputs(18, 21, 19);
            INST(23, Opcode::StoreObject).ref().Inputs(0, 18);
            INST(26, Opcode::LoadObject).ref().Inputs(1);
            INST(27, Opcode::SaveState).NoVregs();
            INST(29, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 26}, {NO_TYPE, 27}});
            INST(30, Opcode::SaveState).NoVregs();
            INST(32, Opcode::LoadClass).ref().Inputs(30);
            INST(31, Opcode::CheckCast).Inputs(29, 32, 30);
            INST(34, Opcode::StoreObject).ref().Inputs(0, 29);
            INST(37, Opcode::LoadObject).ref().Inputs(1);
            INST(38, Opcode::SaveState).NoVregs();
            INST(40, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 37}, {NO_TYPE, 38}});
            INST(41, Opcode::SaveState).NoVregs();
            INST(43, Opcode::LoadClass).ref().Inputs(41);
            INST(42, Opcode::CheckCast).Inputs(40, 43, 41);
            INST(45, Opcode::StoreObject).ref().Inputs(0, 40);
            INST(48, Opcode::LoadObject).ref().Inputs(1);
            INST(49, Opcode::SaveState).NoVregs();
            INST(51, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 48}, {NO_TYPE, 49}});
            INST(52, Opcode::SaveState).NoVregs();
            INST(54, Opcode::LoadClass).ref().Inputs(52);
            INST(53, Opcode::CheckCast).Inputs(51, 54, 52);
            INST(56, Opcode::StoreObject).ref().Inputs(0, 51);
            INST(57, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    EXPECT_FALSE(graph->RunPass<compiler::Cleanup>());

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).ref();
        PARAMETER(3, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;

            INST(4, Opcode::SaveState).NoVregs();
            INST(6, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 0}, {NO_TYPE, 4}});
            INST(9, Opcode::StoreObject).ref().Inputs(0, 2);
            INST(12, Opcode::StoreObject).s32().Inputs(0, 3);
            INST(15, Opcode::LoadObject).ref().Inputs(1);
            INST(16, Opcode::SaveState).NoVregs();
            INST(18, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 15}, {NO_TYPE, 16}});
            INST(60, Opcode::SaveState).NoVregs();
            INST(21, Opcode::LoadClass).ref().Inputs(60);
            INST(20, Opcode::CheckCast).Inputs(18, 21, 60);
            INST(23, Opcode::StoreObject).ref().Inputs(0, 18);
            INST(26, Opcode::LoadObject).ref().Inputs(1);
            INST(27, Opcode::SaveState).NoVregs();
            INST(29, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 26}, {NO_TYPE, 27}});
            INST(63, Opcode::SaveState).NoVregs();
            INST(32, Opcode::LoadClass).ref().Inputs(63);
            INST(31, Opcode::CheckCast).Inputs(29, 32, 63);
            INST(34, Opcode::StoreObject).ref().Inputs(0, 29);
            INST(37, Opcode::LoadObject).ref().Inputs(1);
            INST(38, Opcode::SaveState).NoVregs();
            INST(40, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 37}, {NO_TYPE, 38}});
            INST(41, Opcode::SaveState).NoVregs();
            INST(43, Opcode::LoadClass).ref().Inputs(41);
            INST(42, Opcode::CheckCast).Inputs(40, 43, 41);
            INST(45, Opcode::StoreObject).ref().Inputs(0, 40);
            INST(48, Opcode::LoadObject).ref().Inputs(1);
            INST(49, Opcode::SaveState).NoVregs();
            INST(51, Opcode::CallVirtual).ref().Inputs({{REFERENCE, 48}, {NO_TYPE, 49}});
            INST(52, Opcode::SaveState).NoVregs();
            INST(54, Opcode::LoadClass).ref().Inputs(52);
            INST(53, Opcode::CheckCast).Inputs(51, 54, 52);
            INST(56, Opcode::StoreObject).ref().Inputs(0, 51);
            INST(57, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

// Check processing instructions with the same args by RegEncoder.
TEST_F(CommonTest, RegEncoderSameArgsInst)
{
    auto src_graph = CreateEmptyGraph();
    ArenaVector<bool> reg_mask(254, false, src_graph->GetLocalAllocator()->Adapter());
    src_graph->InitUsedRegs<compiler::DataType::INT64>(&reg_mask);
    GRAPH(src_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::StoreArray).s32().Inputs(0, 1, 2).SrcReg(0, 17).SrcReg(1, 17).SrcReg(2, 5);
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    src_graph->RunPass<RegEncoder>();

    auto opt_graph = CreateEmptyGraph();
    opt_graph->InitUsedRegs<compiler::DataType::INT64>(&reg_mask);
    GRAPH(opt_graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SpillFill);
            INST(4, Opcode::StoreArray).s32().Inputs(0, 1, 2);
            INST(5, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(src_graph, opt_graph));

    for (auto bb : src_graph->GetBlocksRPO()) {
        for (auto inst : bb->AllInstsSafe()) {
            if (inst->GetOpcode() == Opcode::StoreArray) {
                ASSERT_TRUE(inst->GetSrcReg(0) == inst->GetSrcReg(1));
            }
        }
    }
}

}  // namespace panda::bytecodeopt::test
