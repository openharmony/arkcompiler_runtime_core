/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <sstream>
#include "unit_test.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "optimizer/optimizations/regalloc/reg_type.h"
namespace panda::compiler {
inline bool operator==(const SpillFillData &lhs, const SpillFillData &rhs)
{
    return std::forward_as_tuple(lhs.SrcType(), lhs.DstType(), lhs.GetType(), lhs.SrcValue(), lhs.DstValue()) ==
           std::forward_as_tuple(rhs.SrcType(), rhs.DstType(), rhs.GetType(), rhs.SrcValue(), rhs.DstValue());
}

class RegAllocResolverTest : public GraphTest {
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

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RegAllocResolverTest, ResolveFixedInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::AddI).u64().Inputs(2U).Imm(1U);
            INST(3U, Opcode::StoreArray).u64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(5U);
        }
    }

    /*
     * Manually split intervals and allocate registers as follows:
     * - set fixed registers for v3's parameters: r0, r2 and r1 respectively;
     * - move v1 to r2 before v5;
     * - spill v0 from r0 to stack before v3;
     * - move v2 to some reg before v5 and then to r0 before v3.
     *
     * RegAllocResolver may choose r0-split for v0, because it ends right before v3,
     * but it's incorrect because r0 will be overridden by v2 (and SplitResolver will
     * create separate SpillFill for it).
     * This, correct moves for v3 params are:
     * - load v0 from stack slot into r0;
     * - move v2 from r0 into r1;
     */
    Target target(GetGraph()->GetArch());
    auto store_inst = &INS(3U);
    store_inst->SetLocation(0U, Location::MakeRegister(target.GetParamRegId(0U)));
    store_inst->SetLocation(1U, Location::MakeRegister(target.GetParamRegId(2U)));
    store_inst->SetLocation(2U, Location::MakeRegister(target.GetParamRegId(1U)));

    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    ASSERT_TRUE(la.Run());

    auto param_0 = &INS(0U);
    auto param_1 = &INS(1U);
    auto param_2 = &INS(2U);

    auto param_0_interval = la.GetInstLifeIntervals(param_0);
    auto param_1_interval = la.GetInstLifeIntervals(param_1);
    auto param_2_interval = la.GetInstLifeIntervals(param_2);
    auto store_interval = la.GetInstLifeIntervals(store_inst);
    auto add_interval = la.GetInstLifeIntervals(&INS(5U));

    param_0_interval->SetLocation(Location::MakeRegister(target.GetParamRegId(0U)));
    param_1_interval->SetLocation(Location::MakeRegister(target.GetParamRegId(1U)));
    param_2_interval->SetLocation(Location::MakeRegister(target.GetParamRegId(2U)));
    add_interval->SetLocation(Location::MakeRegister(target.GetParamRegId(3U)));

    param_0_interval->SplitAt(store_interval->GetBegin() - 1U, GetGraph()->GetAllocator())
        ->SetLocation(Location::MakeStackSlot(0U));
    param_1_interval->SplitAt(add_interval->GetBegin() - 1U, GetGraph()->GetAllocator())
        ->SetLocation(Location::MakeRegister(target.GetParamRegId(2U)));
    auto p2_split_1 = param_2_interval->SplitAt(add_interval->GetBegin() - 1U, GetGraph()->GetAllocator());
    p2_split_1->SetLocation(Location::MakeRegister(6U));
    auto p2_split_2 = p2_split_1->SplitAt(store_interval->GetBegin() - 1U, GetGraph()->GetAllocator());
    p2_split_2->SetLocation(Location::MakeRegister(target.GetParamRegId(0U)));
    la.GetInstLifeIntervals(&INS(4U))->SetLocation(Location::MakeRegister(target.GetReturnRegId()));
    la.GetTmpRegInterval(store_inst)->SetLocation(Location::MakeRegister(8U));

    InitUsedRegs(GetGraph());
    RegAllocResolver resolver(GetGraph());
    resolver.Resolve();

    auto prev = store_inst->GetPrev();
    ASSERT_TRUE(prev != nullptr && prev->GetOpcode() == Opcode::SpillFill);
    auto sf = prev->CastToSpillFill();
    auto sf_data = sf->GetSpillFills();
    ASSERT_EQ(sf_data.size(), 2U);

    std::vector<SpillFillData> expected_sf {
        SpillFillData {LocationType::REGISTER, LocationType::REGISTER, 0U, 1U, DataType::UINT64},
        SpillFillData {LocationType::STACK, LocationType::REGISTER, 0U, 0U,
                       ConvertRegType(GetGraph(), DataType::REFERENCE)}};

    for (auto &exp_sf : expected_sf) {
        bool found = false;
        for (auto &eff_sf : sf_data) {
            if (eff_sf == exp_sf) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::stringstream f;
            for (auto &esf : sf_data) {
                f << sf_data::ToString(esf, GetGraph()->GetArch()) << ", ";
            }
            ASSERT_TRUE(found) << "Spill fill " << sf_data::ToString(exp_sf, GetGraph()->GetArch())
                               << "  not found among " << f.str();
        }
    }
}
// NOLINTEND(readability-magic-numbers)
}  // namespace panda::compiler
