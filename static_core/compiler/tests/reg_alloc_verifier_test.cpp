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

#include "macros.h"
#include "unit_test.h"
#include "optimizer/analysis/reg_alloc_verifier.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
namespace panda::compiler {
class RegAllocVerifierTest : public GraphTest {
public:
    RegAllocVerifierTest() : default_verify_option_(OPTIONS.IsCompilerVerifyRegalloc())
    {
        // Avoid fatal errors in the negative-tests
        OPTIONS.SetCompilerVerifyRegalloc(false);
    }

    ~RegAllocVerifierTest() override
    {
        OPTIONS.SetCompilerVerifyRegalloc(default_verify_option_);
    }

    NO_COPY_SEMANTIC(RegAllocVerifierTest);
    NO_MOVE_SEMANTIC(RegAllocVerifierTest);

private:
    bool default_verify_option_ {};
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RegAllocVerifierTest, VerifyBranchelessCode)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Mul).u64().Inputs(0, 2);
            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyBranching)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 42);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Mul).u64().Inputs(0, 0);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Mul).u64().Inputs(1, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(4, 5);
            INST(7, Opcode::Return).u64().Inputs(6);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Phi).u64().Inputs(0, 7);
            INST(3, Opcode::Phi).u64().Inputs(1, 6);
            INST(4, Opcode::Compare).b().Inputs(2, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(6, Opcode::Add).u64().Inputs(3, 0);
            INST(7, Opcode::SubI).u64().Imm(1).Inputs(2);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::Return).u64().Inputs(3);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifySingleBlockLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 0);

        BASIC_BLOCK(2, 2, 3)
        {
            INST(2, Opcode::Phi).u64().Inputs(0, 3);
            INST(3, Opcode::SubI).u64().Imm(1).Inputs(2);
            INST(4, Opcode::Compare).b().Inputs(3, 1);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }

        BASIC_BLOCK(3, -1)
        {
            INST(6, Opcode::Return).u64().Inputs(3);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, LoadPair)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadArrayPairI).s64().Inputs(2).Imm(0x0);
            INST(4, Opcode::LoadPairPart).s64().Inputs(3).Imm(0x0);
            INST(5, Opcode::LoadPairPart).s64().Inputs(3).Imm(0x1);
            INST(6, Opcode::SaveState).Inputs(2, 4, 5).SrcVregs({2, 4, 5});
            INST(7, Opcode::CallStatic).v0id().InputsAutoType(4, 5, 6);
            INST(8, Opcode::Add).s64().Inputs(4, 5);
            INST(9, Opcode::Return).s64().Inputs(8);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, TestVoidMethodCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::CallStatic).v0id().InputsAutoType(0, 1);
            INST(3, Opcode::ReturnVoid).v0id();
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, ZeroReg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0);
        CONSTANT(2, nullptr);

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::StoreObject).u64().Inputs(4, 1);
            INST(6, Opcode::StoreObject).ref().Inputs(4, 2);
            INST(7, Opcode::ReturnVoid).v0id();
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, TooManyParameters)
{
    GRAPH(GetGraph())
    {
        for (int i = 0; i < 32; i++) {
            PARAMETER(i, i).u64();
        }

        BASIC_BLOCK(2, -1)
        {
            for (int i = 0; i < 32; i++) {
                INST(32 + i, Opcode::Add).u64().Inputs(i, 32 + i - 1);
            }
            INST(64, Opcode::Return).u64().Inputs(63);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, ReorderSpillFill)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        CONSTANT(1, 2);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SpillFill);
            INST(3, Opcode::Add).u64().Inputs(0, 1);
            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    INS(0).SetDstReg(0);
    INS(1).SetDstReg(1);
    INS(3).SetSrcReg(0, 1);
    INS(3).SetSrcReg(1, 0);
    INS(3).SetDstReg(0);
    INS(4).SetSrcReg(0, 0);

    auto sf = INS(2).CastToSpillFill();
    // swap 0 and 1: should be 0 -> tmp, 1 -> 0, tmp -> 1, but we intentionally reorder it
    sf->AddMove(1, 0, DataType::UINT64);
    sf->AddMove(0, 16, DataType::UINT64);
    sf->AddMove(16, 1, DataType::UINT64);

    ArenaVector<bool> regs = ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
    GetGraph()->InitUsedRegs<DataType::INT64>(&regs);
    GetGraph()->InitUsedRegs<DataType::FLOAT64>(&regs);
    ASSERT_FALSE(RegAllocVerifier(GetGraph(), false).Run());
}

TEST_F(RegAllocVerifierTest, UseIncorrectInputReg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Mul).u64().Inputs(0, 1);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    INS(3).SetSrcReg(0, 15);
    ASSERT_FALSE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, CallSpillFillReordering)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::CallStatic).InputsAutoType(1, 0, 2).v0id();
            INST(4, Opcode::ReturnVoid).v0id();
        }
    }

    // Manually allocate registers to ensure that parameters 0 and 1 will be swapped
    // at CallStatic call site. That swap requires additional resolving move.
    INS(0).CastToParameter()->SetLocationData(
        {LocationType::REGISTER, LocationType::REGISTER, 1, 1, DataType::Type::INT32});
    INS(0).SetDstReg(1);

    INS(1).CastToParameter()->SetLocationData(
        {LocationType::REGISTER, LocationType::REGISTER, 2, 2, DataType::Type::INT32});
    INS(1).SetDstReg(2);

    ArenaVector<bool> regs = ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
    GetGraph()->InitUsedRegs<DataType::INT64>(&regs);
    GetGraph()->InitUsedRegs<DataType::FLOAT64>(&regs);

    RegAllocLinearScan(GetGraph()).Run();
    ASSERT_TRUE(RegAllocVerifier(GetGraph(), false).Run());
}

TEST_F(RegAllocVerifierTest, VerifyIndirectCall)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        // TODO (zwx1004932) Support spill/fills in entrypoints frame and enable test
        GTEST_SKIP();
    }
    GetGraph()->SetMode(GraphMode::FastPath());
    auto ptr_type = Is64BitsArch(GetGraph()->GetArch()) ? DataType::Type::UINT64 : DataType::Type::UINT32;

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).type(ptr_type);
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::CallIndirect).InputsAutoType(0, 1, 2).u64();
            INST(4, Opcode::Return).Inputs(3).u64();
        }
    }

    GetGraph()->RunPass<RegAllocLinearScan>();
    // Inputs overlapping: Input #2 has fixed location r1, Input #0 has dst r1
    // Failed verification is correct for now. Fix it.
    ASSERT_FALSE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyInlinedCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).Inputs(0, 1).u64();
            INST(3, Opcode::Mul).Inputs(0, 2).u64();
            INST(4, Opcode::SaveState).Inputs(0, 1, 2, 3).SrcVregs({0, 1, 2, 3});
            INST(5, Opcode::CallStatic).Inlined().InputsAutoType(3, 2, 1, 0, 4).u64();
            INST(6, Opcode::ReturnInlined).Inputs(4).v0id();
            INST(7, Opcode::ReturnVoid).v0id();
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_EQ(INS(5).GetDstReg(), INVALID_REG);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyEmptyStartBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            CONSTANT(0, 0xABCDABCDABCDABCDLL);
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::CallStatic).InputsAutoType(0, 1).u64();
            INST(3, Opcode::Mul).Inputs(0, 2).u64();
            INST(4, Opcode::Return).Inputs(3).u64();
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    // set incorrect regs
    INS(3).SetDstReg(INS(3).GetDstReg() + 1);
    ASSERT_FALSE(RegAllocVerifier(GetGraph()).Run());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
