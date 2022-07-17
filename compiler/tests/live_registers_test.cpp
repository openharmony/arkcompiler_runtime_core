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
#include "optimizer/analysis/live_registers.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"

namespace panda::compiler {

class LiveRegistersTest : public GraphTest {
};

TEST_F(LiveRegistersTest, EmptyIntervals)
{
    auto intervals = ArenaVector<LifeIntervals *>(GetGraph()->GetAllocator()->Adapter());
    ASSERT_EQ(LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph()), nullptr);
}

TEST_F(LiveRegistersTest, IntervalsWithoutRegisters)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0, 42)));
    ASSERT_EQ(LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph()), nullptr);
}

TEST_F(LiveRegistersTest, IntervalsWithRegisters)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0, 10)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0, 2)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(2, 3)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(5, 6)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(6, 8)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(8, 10)));

    Register reg {0};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    RegMask mask {};
    tree->VisitIntervals(5, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b1001}, mask);

    mask.reset();
    tree->VisitIntervals(11, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {}, mask);

    mask.reset();
    tree->VisitIntervals(8, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b110001}, mask);

    mask.reset();
    tree->VisitIntervals(4, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b1}, mask);

    // Not-live splits at target life-position
    mask.reset();
    tree->VisitIntervals<false>(8, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b100001}, mask);
}

TEST_F(LiveRegistersTest, IntervalsWithHole)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0, 2)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(8, 10)));

    Register reg {0};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    RegMask mask {};
    tree->VisitIntervals(5, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {}, mask);

    mask.reset();
    tree->VisitIntervals(0, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b1}, mask);

    mask.reset();
    tree->VisitIntervals(9, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b10}, mask);
}

TEST_F(LiveRegistersTest, IntervalsOutOfRange)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(4, 6)));

    Register reg {0};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    size_t count = 0;
    tree->VisitIntervals(1, [&count]([[maybe_unused]] const auto &li) { count++; });
    ASSERT_EQ(count, 0);

    count = 0;
    tree->VisitIntervals(42, [&count]([[maybe_unused]] const auto &li) { count++; });
    ASSERT_EQ(count, 0);
}

TEST_F(LiveRegistersTest, MultipleBranches)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0, 80)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0, 39)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(21, 39)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(21, 29)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(26, 29)));

    Register reg {0};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    RegMask mask {};
    tree->VisitIntervals(27, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b11111}, mask);
}

TEST_F(LiveRegistersTest, LiveRegisterForGraph)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 42);

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).u64().Inputs(0);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    ASSERT_TRUE(result);
    ASSERT_TRUE(GetGraph()->RunPass<LiveRegisters>());

    auto con = &INS(0);

    auto &lr = GetGraph()->GetAnalysis<LiveRegisters>();
    lr.VisitIntervalsWithLiveRegisters(con, []([[maybe_unused]] const auto &li) { UNREACHABLE(); });

    size_t count = 0;
    lr.VisitIntervalsWithLiveRegisters(&INS(1), [&con, &count]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(li->GetInst(), con);
        count++;
    });
    ASSERT_EQ(count, 1);
}

TEST_F(LiveRegistersTest, LiveSplits)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    auto interval = alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0, 30));
    intervals.push_back(interval);

    interval->SetType(DataType::Type::UINT64);
    interval->SetReg(0);

    auto split0 = interval->SplitAt(9, alloc);
    split0->SetLocation(Location::MakeStackSlot(0));
    auto split1 = split0->SplitAt(19, alloc);
    split1->SetReg(1);

    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    std::vector<std::pair<LifeNumber, RegMask::ValueType>> ln2mask = {
        {7, 0b1}, {9, 0b1}, {11, 0b0}, {19, 0b10}, {30, 0b10}};

    for (auto [ln, expected_mask] : ln2mask) {
        RegMask mask {};
        tree->VisitIntervals(ln, [&mask]([[maybe_unused]] const auto &li) {
            ASSERT_EQ(mask.test(li->GetReg()), false);
            mask.set(li->GetReg());
        });
        EXPECT_EQ(RegMask {expected_mask}, mask) << "Wrong mask @ life number " << ln;
    }
}

TEST_F(LiveRegistersTest, IntervalWithLifetimeHole)
{
    auto graph = GetGraph();
    if (graph->GetCallingConvention() == nullptr) {
        GTEST_SKIP();
    }

    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        CONSTANT(2, 0);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Mul).Inputs(0, 1).u64();
            INST(4, Opcode::Compare).b().Inputs(3, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(4).Imm(0);
        }

        BASIC_BLOCK(3, 5)
        {
            INST(6, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            // interval for inst 3 will have a lifetime hole at this call
            INST(7, Opcode::CallStatic).InputsAutoType(6).v0id();
        }

        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::Mul).Inputs(3, 3).u64();
            INST(9, Opcode::SaveState).Inputs(0, 1, 2, 8).SrcVregs({0, 1, 2, 8});
            INST(10, Opcode::CallStatic).InputsAutoType(9).v0id();
        }

        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::Return).Inputs(0).u64();
        }
    }

    auto result = graph->RunPass<RegAllocLinearScan>();
    ASSERT_TRUE(result);
    ASSERT_TRUE(graph->RunPass<LiveRegisters>());

    auto &lr = graph->GetAnalysis<LiveRegisters>();
    lr.VisitIntervalsWithLiveRegisters<false>(&INS(7), [graph](const auto &li) {
        auto rd = graph->GetRegisters();
        auto caller_mask =
            DataType::IsFloatType(li->GetType()) ? rd->GetCallerSavedVRegMask() : rd->GetCallerSavedRegMask();
        ASSERT_FALSE(caller_mask.Test(li->GetReg())) << "There should be no live caller-saved registers at call, but "
                                                        "a register for following instruction is alive: "
                                                     << *(li->GetInst());
    });
}
}  // namespace panda::compiler
