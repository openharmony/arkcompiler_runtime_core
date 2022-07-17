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
#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
inline bool operator==(const SpillFillData &lhs, const SpillFillData &rhs)
{
    return std::forward_as_tuple(lhs.SrcType(), lhs.DstType(), lhs.GetType(), lhs.SrcValue(), lhs.DstValue()) ==
           std::forward_as_tuple(rhs.SrcType(), rhs.DstType(), rhs.GetType(), rhs.SrcValue(), rhs.DstValue());
}

class RegAllocLinearScanTest : public GraphTest {
public:
    void CheckInstRegNotEqualOthersInstRegs(int check_id, std::vector<int> &&inst_ids)
    {
        for (auto id : inst_ids) {
            ASSERT_NE(INS(check_id).GetDstReg(), INS(id).GetDstReg());
        }
    }

    void CompareSpillFillInsts(SpillFillInst *lhs, SpillFillInst *rhs)
    {
        const auto &lhs_sfs = lhs->GetSpillFills();
        const auto &rhs_sfs = rhs->GetSpillFills();
        ASSERT_EQ(lhs_sfs.size(), rhs_sfs.size());

        for (size_t i = 0; i < lhs_sfs.size(); i++) {
            auto res = lhs_sfs[i] == rhs_sfs[i];
            ASSERT_TRUE(res);
        }
    }

    bool CheckImmediateSpillFill(Inst *inst, size_t input_num)
    {
        auto sf_inst = inst->GetPrev();
        auto const_input = inst->GetInput(input_num).GetInst();
        if (sf_inst->GetOpcode() != Opcode::SpillFill || !const_input->IsConst()) {
            return false;
        }
        auto graph = inst->GetBasicBlock()->GetGraph();
        auto src_reg = inst->GetSrcReg(input_num);
        for (auto sf : sf_inst->CastToSpillFill()->GetSpillFills()) {
            if (sf.SrcType() == LocationType::IMMEDIATE && (graph->GetSpilledConstant(sf.SrcValue()) == const_input) &&
                sf.DstValue() == src_reg) {
                return true;
            }
        }
        return false;
    }

    template <DataType::Type reg_type>
    void TestPhiMovesOverwriting(Graph *graph, SpillFillInst *sf, SpillFillInst *expected_sf);

protected:
    void InitUsedRegs(Graph *graph)
    {
        ArenaVector<bool> regs =
            ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
        graph->InitUsedRegs<DataType::INT64>(&regs);
        graph->InitUsedRegs<DataType::FLOAT64>(&regs);
        graph->SetStackSlotsCount(MAX_NUM_STACK_SLOTS);
    }
};

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *        /-----[2]<----\
 *        |      |      |
 *        |      v      |
 *        |     [3]-----/
 *        |
 *        \---->[4]
 *               |
 *               v
 *             [exit]
 *
 * Insts linear order:
 * ID   INST(INPUTS)    LIFE NUMBER LIFE INTERVALS      ARM64 REGS
 * ------------------------------------------
 * 0.   Constant        2           [2-22]              R4
 * 1.   Constant        4           [4-8]               R5
 * 2.   Constant        6           [6-24]              R6
 *
 * 3.   Phi (0,7)       8           [8-16][22-24]       R7
 * 4.   Phi (1,8)       8           [8-18]              R8
 * 5.   Cmp (4,0)       10          [10-12]             R9
 * 6.   If    (5)       12          -
 *
 * 7.   Mul (3,4)       16          [16-22]             R5
 * 8.   Sub (4,0)       18          [18-22]             R9
 *
 * 9.   Add (2,3)       24          [24-26]             R4
 */
TEST_F(RegAllocLinearScanTest, ARM64Regs)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        CONSTANT(1, 10);
        CONSTANT(2, 20);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {3, 7}});
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 8}});
            INST(5, Opcode::Compare).b().Inputs(4, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::Mul).u64().Inputs(3, 4);
            INST(8, Opcode::Sub).u64().Inputs(4, 0);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    CheckInstRegNotEqualOthersInstRegs(0, {1, 2, 5, 7, 8});
    CheckInstRegNotEqualOthersInstRegs(1, {2});
    CheckInstRegNotEqualOthersInstRegs(2, {5, 7, 8});
    CheckInstRegNotEqualOthersInstRegs(3, {5});
    CheckInstRegNotEqualOthersInstRegs(4, {5, 7});
    CheckInstRegNotEqualOthersInstRegs(7, {8});
}

/**
 * TODO(a.popov) Support test-case
 *
 * The way to allocate registers in this test is to spill onе of the source registers on the stack:
 * A -> r1
 * B -> r2
 * spill r1 -> STACK
 * ADD(A, B) -> r1
 *
 * Currently, sources on the stack are supported for calls only
 */
TEST_F(RegAllocLinearScanTest, DISABLED_TwoFreeRegs)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1);
        CONSTANT(1, 10);
        CONSTANT(2, 20);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {3, 7}});
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 8}});
            INST(5, Opcode::Compare).b().Inputs(4, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::Mul).u64().Inputs(3, 4);
            INST(8, Opcode::Sub).u64().Inputs(4, 0);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(11, Opcode::ReturnVoid);
        }
    }
    // Create reg_mask with 2 available general registers
    RegAllocLinearScan ra(GetGraph());
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xF3FFFFFFU : 0xFABFFFFFU;
    ra.SetRegMask(RegMask {reg_mask});
    ra.SetVRegMask(VRegMask {0});
    auto result = ra.Run();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        EXPECT_FALSE(result);
        return;
    }
    EXPECT_TRUE(result);
    GraphChecker(GetGraph()).Check();
}

/*
 *           [start]
 *              |
 *              v
 *             [2]
 *            /   \
 *           v     v
 *          [3]   [4]---\
 *         /   \        |
 *        v     v       |
 *       [5]<--[6]<-----/
 *        |
 *        v
 *      [exit]
 *
 *  SpillFill instructions will be inserted between BB3 and BB5, then between BB3 and BB6
 *  Check if these instructions are unique
 *
 */
TEST_F(RegAllocLinearScanTest, InsertSpillFillsAfterSameBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 10).u64();
        PARAMETER(2, 20).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 5, 6)
        {
            INST(5, Opcode::Mul).u64().Inputs(0, 1);
            INST(6, Opcode::Compare).b().CC(CC_EQ).Inputs(1, 2);
            INST(7, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(6);
        }

        BASIC_BLOCK(4, 6)
        {
            INST(8, Opcode::Mul).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::Phi).u64().Inputs({{3, 5}, {6, 12}});
            INST(11, Opcode::Return).u64().Inputs(10);
        }

        BASIC_BLOCK(6, 5)
        {
            INST(12, Opcode::Phi).u64().Inputs({{3, 5}, {4, 8}});
            INST(13, Opcode::Mul).u64().Inputs(12, 2);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block->AllInsts()) {
            ASSERT_EQ(inst->GetBasicBlock(), block);
        }
    }
}

TEST_F(RegAllocLinearScanTest, RzeroAssigment)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 10);
        CONSTANT(2, 20);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Phi).u64().Inputs(0, 7);
            INST(4, Opcode::Phi).u64().Inputs(1, 8);
            INST(5, Opcode::Compare).b().Inputs(4, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(3, 2)
        {
            INST(7, Opcode::Mul).u64().Inputs(3, 4);
            INST(8, Opcode::Sub).u64().Inputs(4, 0);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(13, Opcode::SaveState).Inputs(10).SrcVregs({0});
            INST(11, Opcode::CallStatic).u64().InputsAutoType(0, 13);
            INST(12, Opcode::Return).u64().Inputs(11);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto zero_reg = GetGraph()->GetZeroReg();
    // check for target arch, which has a zero register
    if (zero_reg != INVALID_REG) {
        ASSERT_EQ(INS(0).GetDstReg(), zero_reg);
        ASSERT_EQ(INS(5).GetSrcReg(1), zero_reg);
        ASSERT_EQ(INS(8).GetSrcReg(1), zero_reg);

        // Find spill-fill with moving zero constant -> phi destination register
        auto phi_resolver = BB(2).GetPredBlockByIndex(0);
        auto spill_fill_inst = phi_resolver->GetLastInst()->IsControlFlow() ? phi_resolver->GetLastInst()->GetPrev()
                                                                            : phi_resolver->GetLastInst();
        ASSERT_TRUE(spill_fill_inst->GetOpcode() == Opcode::SpillFill);
        auto spill_fills = spill_fill_inst->CastToSpillFill()->GetSpillFills();
        auto phi_reg = INS(3).GetDstReg();
        ASSERT_TRUE(phi_reg != INVALID_REG);
        auto iter = std::find_if(spill_fills.begin(), spill_fills.end(), [zero_reg, phi_reg](const SpillFillData &sf) {
            return (sf.SrcValue() == zero_reg && sf.DstValue() == phi_reg);
        });
        ASSERT_NE(iter, spill_fills.end());
    }
}

template <DataType::Type reg_type>
void RegAllocLinearScanTest::TestPhiMovesOverwriting(Graph *graph, SpillFillInst *sf, SpillFillInst *expected_sf)
{
    auto resolver = SpillFillsResolver(graph);

    // simple cyclical sf
    sf->ClearSpillFills();
    sf->AddMove(4, 5, reg_type);
    sf->AddMove(5, 4, reg_type);
    expected_sf->ClearSpillFills();
    if (graph->GetArch() != Arch::AARCH32) {  // temp is register
        Register temp_reg = DataType::IsFloatType(reg_type) ? graph->GetArchTempVReg() : graph->GetArchTempReg();
        expected_sf->AddMove(4, temp_reg, reg_type);
        expected_sf->AddMove(5, 4, reg_type);
        expected_sf->AddMove(temp_reg, 5, reg_type);
    } else {  // temp is stack slot
        StackSlot temp_slot = StackSlot(0);
        expected_sf->AddSpill(4, temp_slot, reg_type);
        expected_sf->AddMove(5, 4, reg_type);
        expected_sf->AddFill(temp_slot, 5, reg_type);
    }
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // cyclical sf with memcopy
    sf->ClearSpillFills();
    sf->AddMemCopy(4, 5, reg_type);
    sf->AddMemCopy(5, 4, reg_type);
    expected_sf->ClearSpillFills();
    if (graph->GetArch() != Arch::AARCH32) {  // temp is register
        Register temp_reg = DataType::IsFloatType(reg_type) ? graph->GetArchTempVReg() : graph->GetArchTempReg();
        expected_sf->AddFill(4, temp_reg, reg_type);
        expected_sf->AddMemCopy(5, 4, reg_type);
        expected_sf->AddSpill(temp_reg, 5, reg_type);
    } else {  // temp is stack slot
        StackSlot temp_slot = StackSlot(0);
        expected_sf->AddMemCopy(4, temp_slot, reg_type);
        expected_sf->AddMemCopy(5, 4, reg_type);
        expected_sf->AddMemCopy(temp_slot, 5, reg_type);
    }
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // cyclic sf with all move-types
    sf->ClearSpillFills();
    sf->AddMove(4, 5, reg_type);
    sf->AddSpill(5, 10, reg_type);
    sf->AddMemCopy(10, 11, reg_type);
    sf->AddFill(11, 4, reg_type);
    expected_sf->ClearSpillFills();
    expected_sf->ClearSpillFills();
    if (graph->GetArch() != Arch::AARCH32) {  // temp is register
        Register temp_reg = DataType::IsFloatType(reg_type) ? graph->GetArchTempVReg() : graph->GetArchTempReg();
        expected_sf->AddMove(4, temp_reg, reg_type);
        expected_sf->AddFill(11, 4, reg_type);
        expected_sf->AddMemCopy(10, 11, reg_type);
        expected_sf->AddSpill(5, 10, reg_type);
        expected_sf->AddMove(temp_reg, 5, reg_type);
    } else {  // temp is stack slot
        StackSlot temp_slot = StackSlot(0);
        expected_sf->AddSpill(4, temp_slot, reg_type);
        expected_sf->AddFill(11, 4, reg_type);
        expected_sf->AddMemCopy(10, 11, reg_type);
        expected_sf->AddSpill(5, 10, reg_type);
        expected_sf->AddFill(temp_slot, 5, reg_type);
    }

    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // not applied
    sf->ClearSpillFills();
    sf->AddMove(4, 5, reg_type);
    sf->AddMove(6, 7, reg_type);
    expected_sf->ClearSpillFills();
    expected_sf->AddMove(4, 5, reg_type);
    expected_sf->AddMove(6, 7, reg_type);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // comlex sf
    sf->ClearSpillFills();
    sf->AddFill(1, 15, reg_type);
    sf->AddMove(4, 5, reg_type);
    sf->AddFill(2, 16, reg_type);
    sf->AddMove(5, 6, reg_type);
    sf->AddMove(6, 7, reg_type);
    sf->AddMove(7, 9, reg_type);
    sf->AddMove(6, 4, reg_type);
    sf->AddMove(11, 12, reg_type);
    sf->AddMove(20, 19, reg_type);
    sf->AddMove(21, 20, reg_type);
    sf->AddSpill(15, 3, reg_type);
    sf->AddMove(10, 11, reg_type);
    expected_sf->ClearSpillFills();
    expected_sf->AddMove(7, 9, reg_type);
    expected_sf->AddMove(6, 7, reg_type);
    expected_sf->AddMove(5, 6, reg_type);
    expected_sf->AddMove(4, 5, reg_type);
    expected_sf->AddMove(7, 4, reg_type);

    expected_sf->AddMove(11, 12, reg_type);
    expected_sf->AddMove(10, 11, reg_type);

    expected_sf->AddFill(2, 16, reg_type);

    expected_sf->AddMove(20, 19, reg_type);
    expected_sf->AddMove(21, 20, reg_type);

    expected_sf->AddSpill(15, 3, reg_type);
    expected_sf->AddFill(1, 15, reg_type);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);
}

TEST_F(RegAllocLinearScanTest, PhiMovesOverwriting)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).u64().Inputs(0);
        }
    }

    InitUsedRegs(GetGraph());
    auto resolver = SpillFillsResolver(GetGraph());
    // Use the last available register as a temp
    auto sf = GetGraph()->CreateInstSpillFill();
    auto expected_sf = GetGraph()->CreateInstSpillFill();
    TestPhiMovesOverwriting<DataType::UINT32>(GetGraph(), sf, expected_sf);
    TestPhiMovesOverwriting<DataType::UINT64>(GetGraph(), sf, expected_sf);
    TestPhiMovesOverwriting<DataType::FLOAT64>(GetGraph(), sf, expected_sf);

    // not applied
    sf->ClearSpillFills();
    sf->AddMove(4, 5, DataType::UINT64);
    sf->AddMove(5, 4, DataType::FLOAT64);
    expected_sf->ClearSpillFills();
    expected_sf->AddMove(4, 5, DataType::UINT64);
    expected_sf->AddMove(5, 4, DataType::FLOAT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // not applied
    sf->ClearSpillFills();
    sf->AddMemCopy(0, 1, DataType::UINT64);
    expected_sf->ClearSpillFills();
    expected_sf->AddMemCopy(0, 1, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // mixed reg-types sf
    sf->ClearSpillFills();
    sf->AddFill(1, 5, DataType::FLOAT64);
    sf->AddFill(2, 7, DataType::UINT32);
    sf->AddMove(4, 5, DataType::UINT64);
    sf->AddMove(5, 6, DataType::UINT64);
    sf->AddSpill(2, 3, DataType::UINT64);
    sf->AddSpill(7, 4, DataType::UINT64);
    sf->AddMove(6, 4, DataType::UINT64);
    sf->AddMove(6, 7, DataType::FLOAT64);
    sf->AddMove(7, 9, DataType::FLOAT64);
    sf->AddMove(10, 11, DataType::UINT64);
    sf->AddMove(11, 12, DataType::FLOAT64);
    sf->AddMove(17, 16, DataType::UINT64);
    sf->AddMove(18, 17, DataType::FLOAT64);
    sf->AddSpill(15, 5, DataType::FLOAT64);
    expected_sf->ClearSpillFills();
    expected_sf->AddMove(10, 11, DataType::UINT64);
    expected_sf->AddMove(17, 16, DataType::UINT64);
    expected_sf->AddFill(1, 5, DataType::FLOAT64);

    expected_sf->AddMove(7, 9, DataType::FLOAT64);
    expected_sf->AddMove(6, 7, DataType::FLOAT64);

    expected_sf->AddMove(11, 12, DataType::FLOAT64);
    expected_sf->AddMove(18, 17, DataType::FLOAT64);
    expected_sf->AddSpill(2, 3, DataType::UINT64);

    expected_sf->AddSpill(7, 4, DataType::UINT64);
    expected_sf->AddFill(2, 7, DataType::UINT32);

    expected_sf->AddSpill(15, 5, DataType::FLOAT64);

    if (GetGraph()->GetArch() != Arch::AARCH32) {  // temp is register
        Register temp_reg = GetGraph()->GetArchTempReg();
        expected_sf->AddMove(4, temp_reg, DataType::UINT64);
        expected_sf->AddMove(6, 4, DataType::UINT64);
        expected_sf->AddMove(5, 6, DataType::UINT64);
        expected_sf->AddMove(temp_reg, 5, DataType::UINT64);
    } else {  // temp is stack slot
        StackSlot temp_slot = StackSlot(0);
        expected_sf->AddSpill(4, temp_slot, DataType::UINT64);
        expected_sf->AddMove(6, 4, DataType::UINT64);
        expected_sf->AddMove(5, 6, DataType::UINT64);
        expected_sf->AddFill(temp_slot, 5, DataType::UINT64);
    }

    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // zero-reg reordering
    sf->ClearSpillFills();
    sf->AddMove(31, 5, DataType::UINT64);
    sf->AddMove(5, 6, DataType::UINT64);
    expected_sf->ClearSpillFills();
    expected_sf->AddMove(5, 6, DataType::UINT64);
    expected_sf->AddMove(31, 5, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // find and resolve cycle in moves sequence starts with not-cycle move
    sf->ClearSpillFills();
    sf->AddMove(18, 7, DataType::UINT64);
    sf->AddMove(10, 18, DataType::UINT64);
    sf->AddMove(18, 10, DataType::UINT64);
    sf->AddSpill(7, 32, DataType::UINT64);
    expected_sf->ClearSpillFills();
    expected_sf->AddSpill(7, 32, DataType::UINT64);
    expected_sf->AddMove(18, 7, DataType::UINT64);
    expected_sf->AddMove(10, 18, DataType::UINT64);
    expected_sf->AddMove(7, 10, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // fix `moves_table_[dst].src != first_src'
    sf->ClearSpillFills();
    sf->AddMove(8, 6, DataType::UINT64);
    sf->AddMove(7, 8, DataType::UINT64);
    sf->AddMove(8, 7, DataType::UINT64);
    expected_sf->ClearSpillFills();
    expected_sf->AddMove(8, 6, DataType::UINT64);
    expected_sf->AddMove(7, 8, DataType::UINT64);
    expected_sf->AddMove(6, 7, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);

    // find and resolve cycle in moves sequence starts with not-cycle move
    sf->ClearSpillFills();
    sf->AddMove(18, 7, DataType::UINT64);
    sf->AddMove(10, 18, DataType::UINT64);
    sf->AddMove(18, 10, DataType::UINT64);
    sf->AddMove(7, 6, DataType::UINT64);
    expected_sf->ClearSpillFills();
    expected_sf->AddMove(7, 6, DataType::UINT64);
    expected_sf->AddMove(18, 7, DataType::UINT64);
    expected_sf->AddMove(10, 18, DataType::UINT64);
    expected_sf->AddMove(7, 10, DataType::UINT64);
    resolver.Resolve(sf);
    CompareSpillFillInsts(sf, expected_sf);
}

TEST_F(RegAllocLinearScanTest, DynPhiMovesOverwriting)
{
    Graph *graph = CreateDynEmptyGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        return;
    }

    GRAPH(graph)
    {
        CONSTANT(0, 0);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Cast).any().Inputs(0);
            INST(2, Opcode::Return).any().Inputs(1);
        }
    }
    InitUsedRegs(graph);
    auto resolver = SpillFillsResolver(graph);
    // Use the last available register as a temp
    auto sf = graph->CreateInstSpillFill();
    auto expected_sf = graph->CreateInstSpillFill();
    TestPhiMovesOverwriting<DataType::ANY>(graph, sf, expected_sf);
}

TEST_F(RegAllocLinearScanTest, BrokenTriangleWithEmptyBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64().DstReg(0);
        PARAMETER(1, 1).s64().DstReg(2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(3, 4) {}
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::Phi).s64().Inputs({{2, 1}, {3, 0}}).DstReg(0);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());

    // Create reg_mask with small amount available general registers to force spill-fills
    auto regalloc = RegAllocLinearScan(GetGraph());
    auto result = regalloc.Run();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        BASIC_BLOCK(2, 4, 5)
        {
            INST(2, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::INT64).Inputs(1, 0);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }
        BASIC_BLOCK(5, 4)
        {
            INST(6, Opcode::SpillFill);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(4, Opcode::Phi).s64().Inputs({{2, 0}, {5, 1}});
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(RegAllocLinearScanTest, LoadArrayPair)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(44, 0).TypeId(77);
            INST(4, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(5, Opcode::NullCheck).ref().Inputs(3, 4);
            INST(6, Opcode::LoadArrayPairI).s64().Inputs(5).Imm(0x0);
            INST(7, Opcode::LoadPairPart).s64().Inputs(6).Imm(0x0);
            INST(8, Opcode::LoadPairPart).s64().Inputs(6).Imm(0x1);

            INST(9, Opcode::SaveState).Inputs(3).SrcVregs({0});
            INST(10, Opcode::ZeroCheck).s64().Inputs(7, 9);
            INST(11, Opcode::Div).s64().Inputs(10, 8);
            INST(12, Opcode::Return).s64().Inputs(11);
        }
    }
    auto result = graph->RunPass<RegAllocLinearScan>();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto &div = INS(11);
    auto &load_pair = INS(6);
    for (size_t i = 0; i < div.GetInputsCount(); ++i) {
        if (div.GetSrcReg(0) != load_pair.GetDstReg(0)) {
            auto prev = div.GetPrev();
            ASSERT_EQ(prev->GetOpcode(), Opcode::SpillFill);
            ASSERT_EQ(prev->CastToSpillFill()->GetSpillFill(0).DstValue(), div.GetSrcReg(0));
        }
    }
}

TEST_F(RegAllocLinearScanTest, CheckInputTypeInStore)
{
    auto graph = CreateEmptyGraph();
    if (graph->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(graph)
    {
        CONSTANT(0, nullptr).ref();
        CONSTANT(1, 0x2a).s64();
        CONSTANT(2, 1.1).f64();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 2).SrcVregs({0, 1});
            INST(4, Opcode::NullCheck).ref().Inputs(0, 3);
            INST(5, Opcode::StoreObject).f64().Inputs(4, 2);
            INST(6, Opcode::Return).s64().Inputs(1);
        }
    }
    auto result = graph->RunPass<RegAllocLinearScan>();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto &store = INS(5);
    // check zero reg
    ASSERT_EQ(store.GetSrcReg(0), 31);
}

TEST_F(RegAllocLinearScanTest, NullCheckAsPhiInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();
        PARAMETER(3, 3).ref();
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::If).SrcType(DataType::INT64).CC(CC_EQ).Inputs(1, 2);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(6, Opcode::SaveState).Inputs(1, 2).SrcVregs({0, 1});
            INST(7, Opcode::NullCheck).ref().Inputs(0, 6);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::Phi).ref().Inputs(7, 3);
            INST(12, Opcode::Return).ref().Inputs(11);
        }
    }

    EXPECT_TRUE(graph->RunPass<RegAllocLinearScan>());
    auto phi_location = Location::MakeRegister(INS(11).GetDstReg());
    auto param_location = INS(0).CastToParameter()->GetLocationData().GetDst();
    if (phi_location != param_location) {
        auto sf_inst = BB(3).GetLastInst();
        ASSERT_TRUE(sf_inst->IsSpillFill());
        auto spill_fill = sf_inst->CastToSpillFill()->GetSpillFill(0);
        EXPECT_EQ(spill_fill.GetSrc(), param_location);
        EXPECT_EQ(spill_fill.GetDst(), phi_location);
    }
}

TEST_F(RegAllocLinearScanTest, MultiDestInstruction)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1000);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, -1)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(2, Opcode::NewArray).ref().Inputs(44, 0).TypeId(1);
            INST(3, Opcode::LoadArrayPairI).s32().Inputs(2).Imm(0x0);
            INST(4, Opcode::LoadPairPart).s32().Inputs(3).Imm(0x0);
            INST(5, Opcode::LoadPairPart).s32().Inputs(3).Imm(0x1);
            INST(6, Opcode::Add).s32().Inputs(1, 4);
            INST(7, Opcode::Add).s32().Inputs(6, 5);
            INST(8, Opcode::Return).s32().Inputs(7);
        }
    }
    // Run regalloc without free regs to push all dst on the stack
    auto regalloc = RegAllocLinearScan(graph);
    RegAllocResolver(graph).ResolveCatchPhis();
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0x000FFFFFU : 0xAAAAAAFFU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(VRegMask {});
    regalloc.Run();

    auto load_pair = &INS(3);
    ASSERT_NE(load_pair->GetDstReg(0), load_pair->GetDstReg(1));
    ASSERT_EQ(INS(6).GetSrcReg(1), load_pair->GetDstReg(0));
    ASSERT_EQ(INS(7).GetSrcReg(1), load_pair->GetDstReg(1));
}

/**
 * Create COUNT constants and assign COUNT registers for them
 */
TEST_F(RegAllocLinearScanTest, DynamicRegsMask)
{
    auto graph = CreateEmptyBytecodeGraph();
    graph->CreateStartBlock();
    graph->CreateEndBlock();

    const size_t COUNT = 100;
    auto save_point = graph->CreateInstSafePoint();
    auto block = CreateEmptyBlock(graph);
    block->AppendInst(save_point);
    block->AppendInst(graph->CreateInstReturnVoid());
    graph->GetStartBlock()->AddSucc(block);
    block->AddSucc(graph->GetEndBlock());

    for (size_t i = 0; i < COUNT; i++) {
        save_point->AppendInput(graph->FindOrCreateConstant(i));
        save_point->SetVirtualRegister(i, VirtualRegister(i, false));
    }
    auto result = graph->RunPass<RegAllocLinearScan>(EmptyRegMask());
    ASSERT_TRUE(result);
    for (size_t i = 1; i < COUNT; i++) {
        ASSERT_EQ(graph->FindOrCreateConstant(i)->GetDstReg(), i);
    }
}

TEST_F(RegAllocLinearScanTest, MultiDestAsCallInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1000);
        CONSTANT(1, 1);
        BASIC_BLOCK(2, -1)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(2, Opcode::NewArray).ref().Inputs(44, 0).TypeId(1);
            INST(3, Opcode::LoadArrayPairI).s32().Inputs(2).Imm(0x0);
            INST(4, Opcode::LoadPairPart).s32().Inputs(3).Imm(0x0);
            INST(5, Opcode::LoadPairPart).s32().Inputs(3).Imm(0x1);
            INST(6, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7, Opcode::CallStatic).s32().InputsAutoType(4, 5, 6);
            INST(8, Opcode::ZeroCheck).s32().Inputs(5, 6);
            INST(9, Opcode::Div).s32().Inputs(4, 8);
            INST(10, Opcode::Return).s32().Inputs(9);
        }
    }
    auto regalloc = RegAllocLinearScan(graph);
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0x000FFFFFU : 0xAAAAAAAFU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto load_arr = &INS(3);
    auto div = &INS(9);
    auto call_inst = INS(7).CastToCallStatic();
    auto spill_fill = call_inst->GetPrev()->CastToSpillFill();
    // Check split before call
    for (auto i = 0U; i < 2U; i++) {
        ASSERT_EQ(spill_fill->GetSpillFill(i).SrcValue(), load_arr->GetDstReg(i));
        if (load_arr->GetDstReg(i) == call_inst->GetSrcReg(i)) {
            // LoadArrayPairI -> R1, R2 (caller-saved assigned)
            // R1 -> Rx, R2 -> Ry
            // call(R1, R2)
            ASSERT_EQ(div->GetSrcReg(i), spill_fill->GetSpillFill(i).DstValue());
        } else {
            // LoadArrayPairI -> RX, RY (callee-saved assigned)
            // RX -> R1, RY -> R2
            // call(R1, R2)
            ASSERT_EQ(div->GetSrcReg(i), spill_fill->GetSpillFill(i).SrcValue());
        }
    }
}

TEST_F(RegAllocLinearScanTest, MultiDestInLoop)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 1000);

        BASIC_BLOCK(2, 3)
        {
            INST(44, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68);
            INST(5, Opcode::NewArray).ref().Inputs(44, 2).TypeId(1);
        }
        BASIC_BLOCK(3, 5, 4)
        {
            INST(6, Opcode::Phi).s32().Inputs(0, 12);  // pair part
            INST(7, Opcode::Phi).s32().Inputs(1, 13);  // pair part
            INST(8, Opcode::Phi).s32().Inputs(0, 10);  // add
            INST(9, Opcode::Phi).s32().Inputs(0, 15);  // add
            INST(16, Opcode::SafePoint).Inputs(5, 6, 7).SrcVregs({0, 1, 2});
            INST(10, Opcode::AddI).s32().Inputs(8).Imm(1);
            INST(11, Opcode::LoadArrayPair).s32().Inputs(5, 10);
            INST(12, Opcode::LoadPairPart).s32().Inputs(11).Imm(0);
            INST(13, Opcode::LoadPairPart).s32().Inputs(11).Imm(1);
            INST(14, Opcode::If).SrcType(DataType::INT32).CC(CC_EQ).Inputs(12, 13);
        }
        BASIC_BLOCK(4, 3)
        {
            INST(15, Opcode::SubI).s32().Inputs(9).Imm(2);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(20, Opcode::Return).s32().Inputs(9);
        }
    }
    auto regalloc = RegAllocLinearScan(graph);
    regalloc.SetRegMask(RegMask {0xFFFFAAAA});
    ASSERT_TRUE(regalloc.Run());

    auto load_pair = &INS(11);
    EXPECT_NE(load_pair->GetDstReg(0), INVALID_REG);
    EXPECT_NE(load_pair->GetDstReg(1), INVALID_REG);
    // LoadArrayPair, SubI and AddI should use unique registers
    std::set<Register> regs;
    regs.insert(INS(10).GetDstReg());
    regs.insert(INS(15).GetDstReg());
    regs.insert(load_pair->GetDstReg(0));
    regs.insert(load_pair->GetDstReg(1));
    EXPECT_EQ(regs.size(), 4U);
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedCallInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(3, Opcode::CallStatic).u64().Inputs({{DataType::UINT64, 0}, {DataType::NO_TYPE, 2}});
            INST(4, Opcode::SaveState).Inputs(0, 3).SrcVregs({0, 3});
            INST(5, Opcode::CallStatic)
                .u64()
                .Inputs({{DataType::UINT64, 0}, {DataType::UINT64, 3}, {DataType::NO_TYPE, 4}});
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0));
    auto call0 = (&INS(3))->CastToCallStatic();
    auto call1 = (&INS(5))->CastToCallStatic();
    // split at save state to force usage of stack location as call's input
    auto param_split0 = param0->SplitAt(la.GetInstLifeIntervals(&INS(2))->GetBegin() - 1U, GetAllocator());
    param_split0->SetLocation(Location::MakeStackSlot(42));
    param_split0->SetType(DataType::UINT64);

    auto param_split1 = param_split0->SplitAt(la.GetInstLifeIntervals(call1)->GetBegin() - 1U, GetAllocator());
    param_split1->SetReg(6);
    param_split1->SetType(DataType::UINT64);

    graph->SetStackSlotsCount(MAX_NUM_STACK_SLOTS);
    auto regalloc = RegAllocLinearScan(graph);
    regalloc.SetRegMask(RegMask {0x000FFFFFU});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto call0_sf = call0->GetPrev()->CastToSpillFill()->GetSpillFill(0);
    EXPECT_EQ(call0_sf.SrcType(), LocationType::STACK);
    EXPECT_EQ(call0_sf.SrcValue(), 42);

    const auto &call1_sfs = call1->GetPrev()->CastToSpillFill()->GetSpillFills();
    auto it = std::find_if(call1_sfs.begin(), call1_sfs.end(),
                           [](auto sf) { return sf.SrcType() == LocationType::REGISTER && sf.SrcValue() == 6; });
    EXPECT_NE(it, call1_sfs.end());
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedSaveStateInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::CallVirtual).u64().Inputs({{DataType::REFERENCE, 3}, {DataType::NO_TYPE, 2}});
            INST(5, Opcode::Add).u64().Inputs(0, 4);
            INST(6, Opcode::Return).u64().Inputs(5);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0));
    auto call = &INS(4);
    auto param_split = param0->SplitAt(la.GetInstLifeIntervals(call)->GetBegin() - 1U, GetAllocator());
    static constexpr auto REG_FOR_SPLIT = Register(20);
    param_split->SetReg(REG_FOR_SPLIT);
    param_split->SetType(DataType::UINT64);

    auto regalloc = RegAllocLinearScan(graph);
    // on AARCH32 use only caller-saved register pairs
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFFAU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto null_check = &INS(3);
    auto call_save_state = call->GetInput(call->GetInputsCount() - 1).GetInst()->CastToSaveState();
    auto null_check_save_state = null_check->GetInput(null_check->GetInputsCount() - 1).GetInst()->CastToSaveState();

    ASSERT_NE(call_save_state, null_check_save_state);
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedInstInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 0);
            INST(3, Opcode::Add).u64().Inputs(0, 2);
            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0));
    auto param_split0 = param0->SplitAt(la.GetInstLifeIntervals(&INS(3))->GetBegin() - 1U, GetAllocator());
    static constexpr auto REG_FOR_SPLIT = Register(20);
    param_split0->SetReg(REG_FOR_SPLIT);
    param_split0->SetType(DataType::UINT64);

    auto regalloc = RegAllocLinearScan(graph);
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto add_sf = INS(3).GetPrev();
    ASSERT_TRUE(add_sf->IsSpillFill());

    EXPECT_EQ(INS(3).GetSrcReg(0), REG_FOR_SPLIT);
    SpillFillData expected_sf {LocationType::REGISTER, LocationType::REGISTER, param0->GetReg(), REG_FOR_SPLIT,
                               DataType::UINT64};
    EXPECT_EQ(add_sf->CastToSpillFill()->GetSpillFill(0), expected_sf);
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedSafePointInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SafePoint).Inputs(0).SrcVregs({0});
            INST(3, Opcode::Return).u64().Inputs(0);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0));
    auto sp = &INS(2);
    auto param_split = param0->SplitAt(la.GetInstLifeIntervals(sp)->GetBegin() - 1U, GetAllocator());
    static constexpr auto STACK_SLOT = StackSlot(1);
    param_split->SetLocation(Location::MakeStackSlot(STACK_SLOT));
    param_split->SetType(DataType::UINT64);
    auto ret_split = param_split->SplitAt(la.GetInstLifeIntervals(&INS(3))->GetBegin() - 1U, GetAllocator());
    ret_split->SetReg(0);
    ret_split->SetType(DataType::UINT64);

    graph->SetStackSlotsCount(MAX_NUM_STACK_SLOTS);
    auto regalloc = RegAllocLinearScan(graph);
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(VRegMask {});
    regalloc.SetSlotsCount(MAX_NUM_STACK_SLOTS);
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
}

TEST_F(RegAllocLinearScanTest, ResolveSegmentedPhiInput)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Compare).b().Inputs(0, 1);
            INST(3, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(2);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(4, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(4, 5)
        {
            INST(5, Opcode::Sub).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(0, 5);
            INST(7, Opcode::Phi).u64().Inputs(4, 1);
            INST(8, Opcode::Mul).u64().Inputs(6, 7);
            INST(9, Opcode::Return).u64().Inputs(8);
        }
    }

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();
    auto param0 = la.GetInstLifeIntervals(&INS(0));
    auto param_split0 = param0->SplitAt(la.GetInstLifeIntervals(&INS(4))->GetBegin() - 1U, GetAllocator());
    static constexpr auto REG_FOR_SPLIT = Register(20);
    param_split0->SetReg(REG_FOR_SPLIT);
    param_split0->SetType(DataType::UINT64);

    auto regalloc = RegAllocLinearScan(graph);
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(VRegMask {});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    auto last_inst = INS(4).GetBasicBlock()->GetLastInst();
    ASSERT_TRUE(last_inst->IsSpillFill());
    bool spill_fill_found = false;
    SpillFillData expected_sf {LocationType::REGISTER, LocationType::REGISTER, REG_FOR_SPLIT, INS(6).GetDstReg(),
                               DataType::UINT64};

    for (auto &sf : last_inst->CastToSpillFill()->GetSpillFills()) {
        spill_fill_found |= expected_sf == sf;
    }
    ASSERT_TRUE(spill_fill_found);
}

/**
 *  Currently `CatchPhi` can be visited by `LinearScan` in the `BytecodeOptimizer` mode only, where spilts are not
 * posible, so that there is assertion in the `LinearScan` that interval in not splitted, which breaks this tests
 */
TEST_F(RegAllocLinearScanTest, DISABLED_ResolveSegmentedCatchPhiInputs)
{
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();
        PARAMETER(2, 2).b();

        BASIC_BLOCK(2, 3, 5)
        {
            INST(3, Opcode::Try).CatchTypeIds({0});
        }

        BASIC_BLOCK(3, 4)
        {
            INST(10, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(4, Opcode::CallStatic).b().InputsAutoType(0, 1, 2, 10);
            INST(11, Opcode::SaveState).Inputs(0, 1, 2, 4).SrcVregs({0, 1, 2, 3});
            INST(9, Opcode::CallStatic).b().InputsAutoType(0, 1, 2, 11);
            INST(5, Opcode::And).b().Inputs(4, 9);
        }

        BASIC_BLOCK(4, 6, 5) {}  // Try-end

        BASIC_BLOCK(5, -1)
        {
            INST(7, Opcode::CatchPhi).b().Inputs(2, 4);
            INST(8, Opcode::Return).b().Inputs(7);
        }

        BASIC_BLOCK(6, -1)
        {
            INST(6, Opcode::Return).b().Inputs(5);
        }
    }

    BB(2).SetTryId(0);
    BB(3).SetTryId(0);
    BB(4).SetTryId(0);

    auto catch_phi = (&INS(7))->CastToCatchPhi();
    catch_phi->AppendThrowableInst(&INS(4));
    catch_phi->AppendThrowableInst(&INS(9));

    graph->AppendThrowableInst(&INS(4), &BB(5));
    graph->AppendThrowableInst(&INS(9), &BB(5));
    INS(3).CastToTry()->SetTryEndBlock(&BB(4));

    graph->RunPass<LivenessAnalyzer>();
    auto &la = graph->GetAnalysis<LivenessAnalyzer>();

    auto con = la.GetInstLifeIntervals(&INS(2));
    auto streq = &INS(4);
    auto con_split = con->SplitAt(la.GetInstLifeIntervals(streq)->GetBegin() - 1, GetAllocator());

    auto constexpr split_reg = 10;
    con_split->SetReg(split_reg);
    con_split->SetType(DataType::UINT32);

    auto regalloc = RegAllocLinearScan(graph, EmptyRegMask());
    auto result = regalloc.Run();
    ASSERT_TRUE(result);

    auto catch_phi_reg = la.GetInstLifeIntervals(&INS(7))->GetReg();
    auto sf_before_ins4 = (&INS(4))->GetPrev()->CastToSpillFill();
    EXPECT_EQ(std::count(sf_before_ins4->GetSpillFills().begin(), sf_before_ins4->GetSpillFills().end(),
                         SpillFillData {LocationType::REGISTER, LocationType::REGISTER, split_reg, catch_phi_reg,
                                        DataType::UINT32}),
              1);

    auto ins4_reg = la.GetInstLifeIntervals(&INS(4))->GetReg();
    auto sf_before_ins9 = (&INS(9))->GetPrev()->CastToSpillFill();
    EXPECT_EQ(std::count(sf_before_ins9->GetSpillFills().begin(), sf_before_ins9->GetSpillFills().end(),
                         SpillFillData {LocationType::REGISTER, LocationType::REGISTER, ins4_reg, catch_phi_reg,
                                        DataType::UINT32}),
              1);
}

TEST_F(RegAllocLinearScanTest, RematConstants)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 1).s32();
        CONSTANT(2, 2).s32();
        CONSTANT(13, 1000).s32();
        CONSTANT(3, 0.5).f64();
        CONSTANT(4, 10.0).f64();
        CONSTANT(5, 26.66).f64();
        CONSTANT(14, 2.0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(6, Opcode::Add).s32().Inputs(0, 1);
            INST(7, Opcode::Add).s32().Inputs(6, 2);
            INST(15, Opcode::Add).s32().Inputs(7, 13);
            INST(20, Opcode::SaveState).NoVregs();
            INST(8, Opcode::CallStatic).f64().InputsAutoType(15, 3, 4, 20);
            INST(9, Opcode::Add).f64().Inputs(4, 5);
            INST(10, Opcode::Add).f64().Inputs(8, 3);
            INST(16, Opcode::Add).f64().Inputs(10, 14);
            INST(21, Opcode::SaveState).NoVregs();
            INST(11, Opcode::CallStatic).f64().InputsAutoType(0, 1, 2, 3, 4, 5, 13, 14, 16, 21);
            INST(12, Opcode::Return).f64().Inputs(11);
        }
    }

    auto regalloc = RegAllocLinearScan(graph);
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFE1U : 0xFABFFFFFU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(VRegMask {reg_mask});
    auto result = regalloc.Run();
    if (graph->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);

    // Check inserted to the graph spill-fills
    EXPECT_TRUE(CheckImmediateSpillFill(&INS(15), 1));
    EXPECT_TRUE(CheckImmediateSpillFill(&INS(10), 1));
    EXPECT_TRUE(CheckImmediateSpillFill(&INS(16), 1));

    // Check call instruction's spill-fills
    auto call_inst = INS(11).CastToCallStatic();
    auto spill_fill = call_inst->GetPrev()->CastToSpillFill();
    for (auto sf : spill_fill->GetSpillFills()) {
        if (sf.SrcType() == LocationType::IMMEDIATE) {
            auto input_const = graph->GetSpilledConstant(sf.SrcValue());
            auto it = std::find_if(call_inst->GetInputs().begin(), call_inst->GetInputs().end(),
                                   [input_const](Input input) { return input.GetInst() == input_const; });
            EXPECT_NE(it, call_inst->GetInputs().end());
        } else {
            EXPECT_TRUE(sf.GetSrc().IsAnyRegister());
        }
    }
}

TEST_F(RegAllocLinearScanTest, LoadPairPartDiffRegisters)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(11, 1).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::NullCheck).ref().Inputs(0, 1);
            INST(3, Opcode::LoadArrayPairI).s64().Inputs(2).Imm(0x0);
            INST(4, Opcode::LoadPairPart).s64().Inputs(3).Imm(0x0);
            INST(5, Opcode::LoadPairPart).s64().Inputs(3).Imm(0x1);
            INST(10, Opcode::Add).s64().Inputs(5, 11);
            INST(6, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7, Opcode::CallStatic).s64().InputsAutoType(4, 5, 10, 6);
            INST(8, Opcode::Return).s64().Inputs(7);
        }
    }
    auto regalloc = RegAllocLinearScan(GetGraph());
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xF3FFFFFFU : 0xFAFFFFFFU;
    regalloc.SetRegMask(RegMask {reg_mask});
    auto result = regalloc.Run();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    auto load_pair_i = &INS(3);
    EXPECT_NE(load_pair_i->GetDstReg(0), load_pair_i->GetDstReg(1));
}

TEST_F(RegAllocLinearScanTest, SpillRegistersAroundCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).f32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::CallStatic).u32().InputsAutoType(0, 1, 2);
            INST(4, Opcode::Add).u32().Inputs(0, 3);
            INST(5, Opcode::Return).u32().Inputs(4);
        }
    }
    auto regalloc = RegAllocLinearScan(GetGraph());
    ASSERT_TRUE(regalloc.Run());

    // parameter 0 should be splitted before call and split should be used by add
    auto spill_fill = (&INS(3))->GetPrev();
    EXPECT_TRUE(spill_fill->IsSpillFill());
    auto sf = spill_fill->CastToSpillFill()->GetSpillFill(0);
    auto param_dst = (&INS(0))->GetDstReg();
    auto call_src = (&INS(3))->GetSrcReg(0);
    auto add_src = (&INS(4))->GetSrcReg(0);
    ASSERT_EQ(sf.SrcValue(), param_dst);
    if (call_src == param_dst) {
        // param -> R1 (caller-saved assigned)
        // R1 -> Rx
        // call(R1, ..)
        ASSERT_EQ(add_src, sf.DstValue());
    } else {
        // param -> Rx (callee-saved assigned)
        // Rx -> R1
        // call(R1, ..)
        ASSERT_EQ(add_src, sf.SrcValue());
    }
    // all caller-saved regs should be spilled at call
    auto &lr = GetGraph()->GetAnalysis<LiveRegisters>();
    auto graph = GetGraph();
    lr.VisitIntervalsWithLiveRegisters(&INS(3), [graph](const auto &li) {
        auto rd = graph->GetRegisters();
        auto caller_mask =
            DataType::IsFloatType(li->GetType()) ? rd->GetCallerSavedVRegMask() : rd->GetCallerSavedRegMask();
        ASSERT_FALSE(caller_mask.Test(li->GetReg()));
    });
}

TEST_F(RegAllocLinearScanTest, SplitCallIntervalAroundNextCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u32();
        PARAMETER(1, 1).u32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::CallStatic).InputsAutoType(0, 1, 2).f64();
            INST(4, Opcode::Add).Inputs(3, 3).f64();
            INST(5, Opcode::SaveState).Inputs(3, 4).SrcVregs({3, 4});
            INST(6, Opcode::CallStatic).InputsAutoType(5).u64();
            INST(7, Opcode::Return).Inputs(3).f64();
        }
    }

    auto regalloc = RegAllocLinearScan(GetGraph());
    uint32_t reg_mask = GetGraph()->GetArch() != Arch::AARCH32 ? 0xFFFFFFF0U : 0xFFFFFFAAU;
    regalloc.SetRegMask(RegMask {reg_mask});
    regalloc.SetVRegMask(RegMask {reg_mask});
    ASSERT_TRUE(regalloc.Run());

    // Spill after last use before next call
    auto spill = INS(4).GetNext()->CastToSpillFill()->GetSpillFill(0);
    // Fill before firt use after call
    auto fill = INS(7).GetPrev()->CastToSpillFill()->GetSpillFill(0);
    auto call_reg = Location::MakeFpRegister(INS(3).GetDstReg());

    EXPECT_EQ(spill.GetSrc(), call_reg);
    EXPECT_EQ(spill.GetDst(), fill.GetSrc());
    EXPECT_EQ(call_reg, fill.GetDst());
}

static bool CheckInstsDstRegs(Inst *inst0, const Inst *inst1, Register reg)
{
    EXPECT_EQ(inst0->GetDstReg(), reg);
    EXPECT_EQ(inst1->GetDstReg(), reg);

    // Should be spill-fill before second instruction to spill the first one;
    for (auto inst : InstIter(*inst0->GetBasicBlock(), inst0)) {
        if (inst == inst1) {
            break;
        }
        if (inst->IsSpillFill()) {
            auto sfs = inst->CastToSpillFill()->GetSpillFills();
            auto it = std::find_if(sfs.cbegin(), sfs.cend(), [reg](auto sf) {
                return sf.SrcValue() == reg && sf.SrcType() == LocationType::REGISTER;
            });
            if (it != sfs.cend()) {
                return true;
            }
        }
    }
    return false;
}

TEST_F(RegAllocLinearScanTest, PreassignedRegisters)
{
    // Check with different masks:
    RegMask full_mask {0x0U};           // All registers are available for RA
    RegMask short_mask {0xFFFFFFAAU};   // only {R0, R2, R4, R6} are available for RA
    RegMask short_mask_invert {0x55U};  // {R0, R2, R4, R6} are NOT available for RA

    for (auto &mask : {full_mask, short_mask, short_mask_invert}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            BASIC_BLOCK(2, -1)
            {
                INST(10, Opcode::SaveState).Inputs().SrcVregs({});
                INST(0, Opcode::CallStatic).InputsAutoType(10).u32().DstReg(0);
                INST(1, Opcode::CallStatic).InputsAutoType(10).u32().DstReg(1);
                INST(2, Opcode::CallStatic).InputsAutoType(10).u32().DstReg(2);
                INST(3, Opcode::Add).Inputs(0, 1).u32().DstReg(0);
                INST(4, Opcode::Add).Inputs(1, 2).u32().DstReg(1);
                INST(5, Opcode::Add).Inputs(0, 2).u32().DstReg(2);
                INST(6, Opcode::Add).Inputs(4, 5).u32();
                INST(7, Opcode::SaveState).Inputs().SrcVregs({});
                INST(8, Opcode::CallStatic).InputsAutoType(0, 1, 2, 3, 4, 5, 6, 7).u32();
                INST(9, Opcode::Return).Inputs(8).u32();
            }
        }
        auto regalloc = RegAllocLinearScan(graph);
        regalloc.SetRegMask(mask);
        ASSERT_TRUE(regalloc.Run());
        ASSERT_TRUE(CheckInstsDstRegs(&INS(0), &INS(3), Register(0)));
        ASSERT_TRUE(CheckInstsDstRegs(&INS(1), &INS(4), Register(1)));
        ASSERT_TRUE(CheckInstsDstRegs(&INS(2), &INS(5), Register(2)));
    }
}

TEST_F(RegAllocLinearScanTest, Select3Regs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s64();
        PARAMETER(1, 1).s64();
        PARAMETER(2, 2).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).s64().Inputs(0, 1);
            INST(4, Opcode::Select).s64().SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(0, 1, 2, 3);
            INST(5, Opcode::Return).s64().Inputs(4);
        }
    }

    auto regalloc = RegAllocLinearScan(GetGraph());
    auto reg_mask = GetGraph()->GetArch() == Arch::AARCH32 ? 0xFFFFFFABU : 0xFFFFFFF1U;
    regalloc.SetRegMask(RegMask {reg_mask});
    auto result = regalloc.Run();
    ASSERT_FALSE(result);
}

TEST_F(RegAllocLinearScanTest, TwoInstsWithZeroReg)
{
    auto zero_reg = GetGraph()->GetZeroReg();
    if (zero_reg == INVALID_REG) {
        return;
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0, 0).s64();
        CONSTANT(1, nullptr).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs().SrcVregs({});
            INST(3, Opcode::CallStatic).InputsAutoType(1, 2).s64();
            INST(4, Opcode::Add).s64().Inputs(0, 3);
            INST(5, Opcode::Return).ref().Inputs(1);
        }
    }

    auto regalloc = RegAllocLinearScan(GetGraph());
    regalloc.Run();
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    auto const0 = la.GetInstLifeIntervals(&INS(0));
    auto nullptr_inst = la.GetInstLifeIntervals(&INS(1));
    // Constant and Nullptr should not be split
    EXPECT_EQ(const0->GetSibling(), nullptr);
    EXPECT_EQ(nullptr_inst->GetSibling(), nullptr);
    // Intervals' regs and dst's regs should be 'zero_reg'
    EXPECT_EQ(const0->GetReg(), zero_reg);
    EXPECT_EQ(nullptr_inst->GetReg(), zero_reg);
    EXPECT_EQ(INS(0).GetDstReg(), zero_reg);
    EXPECT_EQ(INS(1).GetDstReg(), zero_reg);
    // Src's reg should be 'zero_reg'
    EXPECT_EQ(INS(4).GetSrcReg(0), zero_reg);
    if (INS(5).GetPrev()->IsSpillFill()) {
        auto sf = INS(5).GetPrev()->CastToSpillFill();
        EXPECT_EQ(sf->GetSpillFill(0).SrcValue(), zero_reg);
        EXPECT_EQ(sf->GetSpillFill(0).DstValue(), INS(5).GetSrcReg(0));
    } else {
        EXPECT_EQ(INS(5).GetSrcReg(0), zero_reg);
    }
}
}  // namespace panda::compiler
