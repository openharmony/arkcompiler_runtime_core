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
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/live_registers.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
class RegAllocGraphColoringTest : public GraphTest {
public:
    SpillFillData GetParameterSpillFilll(Inst *param)
    {
        ASSERT(param->IsParameter());
        ASSERT(param->GetNext()->IsSpillFill());
        auto spill_fill = param->GetNext()->CastToSpillFill()->GetSpillFill(0);
        auto param_liveness = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetInstLifeIntervals(param);
        EXPECT_EQ(param_liveness->GetReg(), spill_fill.SrcValue());
        EXPECT_EQ(param_liveness->GetSibling()->GetReg(), spill_fill.DstValue());
        return spill_fill;
    }
};

TEST_F(RegAllocGraphColoringTest, Simple)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(2, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 1);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    EXPECT_NE(INS(0).GetDstReg(), INS(6).GetDstReg());
    EXPECT_NE(INS(0).GetDstReg(), INS(1).GetDstReg());
    EXPECT_NE(INS(0).GetDstReg(), INS(2).GetDstReg());

    auto arch = GetGraph()->GetArch();
    size_t first_callee = arch != Arch::NONE ? GetFirstCalleeReg(arch, false) : 0;
    EXPECT_LT(INS(0).CastToParameter()->GetLocationData().DstValue(), first_callee);
    EXPECT_LT(INS(1).GetDstReg(), first_callee);
    EXPECT_LT(INS(2).GetDstReg(), first_callee);
    EXPECT_LT(INS(5).GetDstReg(), first_callee);
    EXPECT_LT(INS(6).GetDstReg(), first_callee);
    EXPECT_LT(INS(7).GetDstReg(), first_callee);
}

TEST_F(RegAllocGraphColoringTest, AffineComponent)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 8)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(4, 5, 6)
        {
            INST(7, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(1, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(5, 7)
        {
            CONSTANT(9, 188);
        }

        BASIC_BLOCK(6, 7)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(7, 8)
        {
            INST(10, Opcode::Phi).u64().Inputs(9, 6);
        }

        BASIC_BLOCK(8, -1)
        {
            INST(11, Opcode::Phi).u64().Inputs(5, 10);
            INST(12, Opcode::Add).u64().Inputs(11, 1);
            INST(13, Opcode::Add).u64().Inputs(12, 0);
            INST(14, Opcode::Return).u64().Inputs(13);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    EXPECT_EQ(INS(0).GetDstReg(), INS(3).GetSrcReg(1));
    EXPECT_EQ(INS(0).GetDstReg(), INS(7).GetSrcReg(1));

    // Check affinity group
    EXPECT_EQ(INS(5).GetDstReg(), INS(6).GetDstReg());
    EXPECT_EQ(INS(5).GetDstReg(), INS(9).GetDstReg());
    EXPECT_EQ(INS(5).GetDstReg(), INS(10).GetDstReg());
    EXPECT_EQ(INS(5).GetDstReg(), INS(11).GetDstReg());
}

TEST_F(RegAllocGraphColoringTest, SimpleCall)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32 || GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 1);
        CONSTANT(3, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(3, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(6, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7, Opcode::CallStatic).b().InputsAutoType(0, 1, 2, 6);
            INST(8, Opcode::Add).u64().Inputs(1, 3);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(9, Opcode::Phi).u64().Inputs(3, 8);
            INST(10, Opcode::Add).u64().Inputs(9, 1);
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }
    auto regalloc = RegAllocGraphColoring(GetGraph());
    auto arch = GetGraph()->GetArch();
    size_t first_callee = arch != Arch::NONE ? GetFirstCalleeReg(arch, false) : 0;

    auto result = regalloc.Run();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();

    auto param1_sf = GetParameterSpillFilll(&INS(1));
    EXPECT_NE(param1_sf.DstValue(), INS(8).GetDstReg());
    EXPECT_NE(param1_sf.DstValue(), INS(2).GetDstReg());
    EXPECT_NE(param1_sf.DstValue(), INS(3).GetDstReg());

    // Check intervals in calle registers and splits
    EXPECT_LT(INS(0).GetDstReg(), first_callee);
    EXPECT_GE(param1_sf.DstValue(), first_callee);
}

// Check fallback to Linearscan (spilling is not implemented yet)
TEST_F(RegAllocGraphColoringTest, HighPressure)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32 || GetGraph()->GetCallingConvention() == nullptr) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 10);
        CONSTANT(3, 10);
        CONSTANT(4, 10);
        CONSTANT(5, 10);
        CONSTANT(6, 10);

        BASIC_BLOCK(2, -1)
        {
            INST(7, Opcode::Add).u64().Inputs(0, 1);
            INST(8, Opcode::Add).u64().Inputs(0, 2);
            INST(9, Opcode::Add).u64().Inputs(0, 3);
            INST(10, Opcode::Add).u64().Inputs(0, 4);
            INST(11, Opcode::Add).u64().Inputs(0, 5);
            INST(12, Opcode::Add).u64().Inputs(0, 6);
            INST(13, Opcode::Add).u64().Inputs(0, 7);
            INST(14, Opcode::Add).u64().Inputs(0, 8);
            INST(15, Opcode::Add).u64().Inputs(8, 7);
            INST(16, Opcode::Add).u64().Inputs(9, 8);

            INST(17, Opcode::SaveState).Inputs().SrcVregs({});
            INST(18, Opcode::CallStatic).b().InputsAutoType(0, 1, 2, 17);

            INST(20, Opcode::Add).u64().Inputs(1, 2);
            INST(21, Opcode::Add).u64().Inputs(3, 4);
            INST(22, Opcode::Add).u64().Inputs(5, 6);
            INST(23, Opcode::Add).u64().Inputs(7, 8);
            INST(24, Opcode::Add).u64().Inputs(9, 10);
            INST(25, Opcode::Add).u64().Inputs(11, 12);
            INST(26, Opcode::Add).u64().Inputs(13, 14);
            INST(27, Opcode::Add).u64().Inputs(15, 16);

            INST(60, Opcode::Return).u64().Inputs(27);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_FALSE(result);
    GraphChecker(GetGraph()).Check();

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (const auto *interv : la.GetLifeIntervals()) {
        EXPECT_EQ(interv->GetSibling(), nullptr);
        if (!interv->IsPreassigned() && !interv->IsPhysical()) {
            EXPECT_FALSE(interv->HasReg());
        }
    }
}
}  // namespace panda::compiler
