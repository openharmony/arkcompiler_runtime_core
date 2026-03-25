/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"

namespace ark::compiler {
class TestGraphCloner : public GraphCloner {
public:
    using GraphCloner::GetClone;
    using GraphCloner::GraphCloner;
};

class RegAllocCommonTest : public GraphTest {
public:
    template <typename Checker>
    void RunRegAllocatorsAndCheck(Graph *graph, Checker checker) const
    {
        RunRegAllocatorsAndCheck(graph, graph->GetArchUsedRegs(), checker);
    }

    template <typename Checker>
    void RunRegAllocatorsAndCheck(Graph *graph, RegMask mask, Checker checker) const
    {
        RunRegAllocatorsAndCheckWithPrepare(
            graph, mask, []([[maybe_unused]] Graph *preparedGraph, [[maybe_unused]] TestGraphCloner *cloner) {},
            checker);
    }

    template <typename Prepare, typename Checker>
    void RunRegAllocatorsAndCheckWithPrepare(Graph *graph, Prepare prepare, Checker checker) const
    {
        RunRegAllocatorsAndCheckWithPrepare(graph, graph->GetArchUsedRegs(), prepare, checker);
    }

    template <typename Prepare, typename Checker>
    void RunRegAllocatorsAndCheckWithPrepare(Graph *graph, RegMask mask, Prepare prepare, Checker checker) const
    {
        if (graph->GetCallingConvention() == nullptr) {
            return;
        }
        auto clonerLs = TestGraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());
        auto graphLs = clonerLs.CloneGraph();
        prepare(graphLs, &clonerLs);
        auto rals = RegAllocLinearScan(graphLs);
        rals.SetRegMask(mask);
        ASSERT_TRUE(rals.Run());
        checker(graphLs, &clonerLs);

        // RegAllocGraphColoring is not supported for AARCH32
        if (graph->GetArch() == Arch::AARCH32) {
            return;
        }
        auto clonerGc = TestGraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());
        auto graphGc = clonerGc.CloneGraph();
        prepare(graphGc, &clonerGc);
        auto ragc = RegAllocGraphColoring(graphGc);
        ragc.SetRegMask(mask);
        ASSERT_TRUE(ragc.Run());
        checker(graphGc, &clonerGc);
    }

protected:
    template <DataType::Type REG_TYPE>
    void TestParametersLocations() const;
};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(TestParametersLocationsUINT64, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();
        PARAMETER(5U, 5U).u64();
        PARAMETER(6U, 6U).u64();
        PARAMETER(7U, 7U).u64();
        PARAMETER(8U, 8U).u64();
        PARAMETER(9U, 9U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(30U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 30U);
            INST(12U, Opcode::StoreObject).u64().Inputs(11U, 1U);
            INST(13U, Opcode::StoreObject).u64().Inputs(11U, 2U);
            INST(14U, Opcode::StoreObject).u64().Inputs(11U, 3U);
            INST(15U, Opcode::StoreObject).u64().Inputs(11U, 4U);
            INST(16U, Opcode::StoreObject).u64().Inputs(11U, 5U);
            INST(17U, Opcode::StoreObject).u64().Inputs(11U, 6U);
            INST(18U, Opcode::StoreObject).u64().Inputs(11U, 7U);
            INST(19U, Opcode::StoreObject).u64().Inputs(11U, 8U);
            INST(20U, Opcode::StoreObject).u64().Inputs(11U, 9U);
            INST(31U, Opcode::ReturnVoid).v0id();
        }
    }
}
SRC_GRAPH(TestParametersLocationsUINT32, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u32();
        PARAMETER(2U, 2U).u32();
        PARAMETER(3U, 3U).u32();
        PARAMETER(4U, 4U).u32();
        PARAMETER(5U, 5U).u32();
        PARAMETER(6U, 6U).u32();
        PARAMETER(7U, 7U).u32();
        PARAMETER(8U, 8U).u32();
        PARAMETER(9U, 9U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(30U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 30U);
            INST(12U, Opcode::StoreObject).u32().Inputs(11U, 1U);
            INST(13U, Opcode::StoreObject).u32().Inputs(11U, 2U);
            INST(14U, Opcode::StoreObject).u32().Inputs(11U, 3U);
            INST(15U, Opcode::StoreObject).u32().Inputs(11U, 4U);
            INST(16U, Opcode::StoreObject).u32().Inputs(11U, 5U);
            INST(17U, Opcode::StoreObject).u32().Inputs(11U, 6U);
            INST(18U, Opcode::StoreObject).u32().Inputs(11U, 7U);
            INST(19U, Opcode::StoreObject).u32().Inputs(11U, 8U);
            INST(20U, Opcode::StoreObject).u32().Inputs(11U, 9U);
            INST(31U, Opcode::ReturnVoid).v0id();
        }
    }
}

template <DataType::Type REG_TYPE>
void RegAllocCommonTest::TestParametersLocations() const
{
    auto graph = CreateEmptyGraph();
    if constexpr (DataType::UINT64 == REG_TYPE) {
        src_graph::TestParametersLocationsUINT64::CREATE(graph);
    } else {
        src_graph::TestParametersLocationsUINT32::CREATE(graph);
    }

    RunRegAllocatorsAndCheck(graph, [type = REG_TYPE](Graph *checkGraph, [[maybe_unused]] TestGraphCloner *cloner) {
        auto arch = checkGraph->GetArch();
        unsigned slotInc = Is64Bits(type, arch) && !Is64BitsArch(arch) ? 2U : 1U;

        unsigned paramsOnRegisters = 0;
        if (Arch::AARCH64 == checkGraph->GetArch()) {
            /**
             * Test case for Arch::AARCH64:
             *
             * - Parameters [arg0 - arg6] are placed in the registers [r1-r7]
             * - All other Parameters are placed in stack slots [slot0 - ...]
             */
            paramsOnRegisters = 7;
        } else if (Arch::AARCH32 == checkGraph->GetArch()) {
            /**
             * Test case for Arch::AARCH32:
             * - ref-Parameter (arg0) is placed in the r1 register
             * - If arg1 is 64-bit Parameter, it is placed in the [r2-r3] registers
             * - If arg1, arg2 are 32-bit Parameters, they are placed in the [r2-r3] registers
             * - All other Parameters are placed in stack slots [slot0 - ...]
             */
            paramsOnRegisters = (type == DataType::UINT64) ? 2U : 3U;
        }

        std::map<Location, Inst *> assignedLocations;
        unsigned index = 0;
        unsigned argSlot = 0;
        for (auto paramInst : checkGraph->GetParameters()) {
            // Check intial locations
            auto srcLocation = paramInst->CastToParameter()->GetLocationData().GetSrc();
            if (index < paramsOnRegisters) {
                EXPECT_EQ(srcLocation.GetKind(), LocationType::REGISTER);
                EXPECT_EQ(srcLocation.GetValue(), index + 1U);
            } else {
                EXPECT_EQ(srcLocation.GetKind(), LocationType::STACK_PARAMETER);
                EXPECT_EQ(srcLocation.GetValue(), argSlot);
                argSlot += slotInc;
            }

            // Check that assigned locations do not overlap
            auto dstLocation = paramInst->CastToParameter()->GetLocationData().GetDst();
            EXPECT_EQ(assignedLocations.count(dstLocation), 0U);
            assignedLocations.insert({dstLocation, paramInst});

            index++;
        }
    });
}

TEST_F(RegAllocCommonTest, ParametersLocation)
{
    TestParametersLocations<DataType::UINT64>();
    TestParametersLocations<DataType::UINT32>();
}

static void LocationsNoSplitsCheckBB(BasicBlock *bb)
{
    for (auto inst : bb->AllInsts()) {
        EXPECT_FALSE(inst->IsSpillFill());
        if (inst->NoDest()) {
            ASSERT(inst->IsSaveState() || inst->GetOpcode() == Opcode::Return);
        } else {
            EXPECT_NE(inst->GetDstReg(), INVALID_REG);
        }
    }
}

template <class Predicate>
static bool HasSpillFillDataAfter(const Inst *inst, Predicate pred)
{
    for (auto nextInst = inst->GetNext(); nextInst != nullptr; nextInst = nextInst->GetNext()) {
        if (!nextInst->IsSpillFill()) {
            continue;
        }
        const auto &spillFills = nextInst->CastToSpillFill()->GetSpillFills();
        if (std::any_of(spillFills.begin(), spillFills.end(), pred)) {
            return true;
        }
    }
    return false;
}

template <class Predicate>
static bool HasSpillFillDataBefore(const Inst *inst, Predicate pred)
{
    auto prevInst = inst->GetPrev();
    if (prevInst == nullptr || !prevInst->IsSpillFill()) {
        return false;
    }
    const auto &spillFills = prevInst->CastToSpillFill()->GetSpillFills();
    return std::any_of(spillFills.begin(), spillFills.end(), pred);
}

static std::vector<uint32_t> CollectRegisterToStackSpillSlotsBefore(const Inst *inst)
{
    std::vector<uint32_t> spillSlots;
    auto prevInst = inst->GetPrev();
    if (prevInst == nullptr || !prevInst->IsSpillFill()) {
        return spillSlots;
    }
    const auto &spillFills = prevInst->CastToSpillFill()->GetSpillFills();
    spillSlots.reserve(spillFills.size());
    for (const auto &spill : spillFills) {
        if (spill.SrcType() == LocationType::REGISTER && spill.DstType() == LocationType::STACK) {
            spillSlots.push_back(spill.DstValue());
        }
    }
    std::sort(spillSlots.begin(), spillSlots.end());
    spillSlots.erase(std::unique(spillSlots.begin(), spillSlots.end()), spillSlots.end());
    return spillSlots;
}

static void SetStackParameterLocation(Inst *inst, uint32_t stackParamSlot)
{
    ASSERT_NE(inst, nullptr);
    ASSERT_TRUE(inst->IsParameter());
    inst->CastToParameter()->SetLocationData(
        {LocationType::STACK_PARAMETER, LocationType::INVALID, stackParamSlot, GetInvalidReg(), DataType::UINT32});
}

static bool IsStackLikeLocation(LocationType locationType)
{
    return locationType == LocationType::STACK || locationType == LocationType::STACK_PARAMETER;
}

static void CheckStackValueReloadedAfterSuspend(const Inst *suspendInst, const Inst *postUseInst, uint32_t stackSlot,
                                                [[maybe_unused]] LocationType srcType)
{
    ASSERT_NE(suspendInst, nullptr);
    ASSERT_NE(postUseInst, nullptr);
    EXPECT_FALSE(HasSpillFillDataBefore(suspendInst, [stackSlot](const SpillFillData &sf) {
        return IsStackLikeLocation(sf.SrcType()) && sf.DstType() == LocationType::REGISTER &&
               sf.SrcValue() == stackSlot;
    }));
    auto postUseReg = postUseInst->GetSrcReg(0U);
    EXPECT_TRUE(HasSpillFillDataAfter(suspendInst, [stackSlot, postUseReg](const SpillFillData &sf) {
        return IsStackLikeLocation(sf.SrcType()) && sf.DstType() == LocationType::REGISTER &&
               sf.SrcValue() == stackSlot && sf.DstValue() == postUseReg;
    }));
}

static void CheckStackValueNotReloadedAfterSuspend(const Inst *suspendInst, uint32_t stackSlot,
                                                   [[maybe_unused]] LocationType srcType)
{
    ASSERT_NE(suspendInst, nullptr);
    EXPECT_FALSE(HasSpillFillDataAfter(suspendInst, [stackSlot](const SpillFillData &sf) {
        return IsStackLikeLocation(sf.SrcType()) && sf.DstType() == LocationType::REGISTER &&
               sf.SrcValue() == stackSlot;
    }));
}

TEST_F(RegAllocCommonTest, SaveStateSuspendRegSpillAndReload)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(10U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u32().Inputs(0U, 0U);
            INST(3U, Opcode::Add).u32().Inputs(2U, 0U);
            INST(4U, Opcode::SaveStateSuspend).Inputs(10U, 0U, 2U, 3U).SrcVregs({4U, 0U, 1U, 2U});
            INST(5U, Opcode::Add).u32().Inputs(2U, 3U);
            INST(6U, Opcode::Add).u32().Inputs(5U, 0U);
            INST(7U, Opcode::Return).u32().Inputs(6U);
        }
    }

    auto suspend = &INS(4U);
    RunRegAllocatorsAndCheck(graph, [suspend]([[maybe_unused]] Graph *checkGraph, TestGraphCloner *cloner) {
        auto clonedSuspend = cloner->GetClone(suspend);
        ASSERT_NE(clonedSuspend, nullptr);
        auto spillSlots = CollectRegisterToStackSpillSlotsBefore(clonedSuspend);
        ASSERT_FALSE(spillSlots.empty());
        for (auto slot : spillSlots) {
            auto hasReload = HasSpillFillDataAfter(clonedSuspend, [slot](const SpillFillData &sf) {
                return sf.SrcType() == LocationType::STACK && sf.DstType() == LocationType::REGISTER &&
                       sf.SrcValue() == slot;
            });
            EXPECT_TRUE(hasReload);
        }
    });
}

TEST_F(RegAllocCommonTest, SaveStateSuspendRegSpillWithoutReload)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        PARAMETER(10U, 2U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u32().Inputs(0U, 0U);
            INST(3U, Opcode::Add).u32().Inputs(2U, 0U);
            INST(4U, Opcode::Add).u32().Inputs(3U, 2U);
            INST(5U, Opcode::Add).u32().Inputs(4U, 3U);
            INST(6U, Opcode::Add).u32().Inputs(5U, 4U);
            INST(7U, Opcode::SaveStateSuspend)
                .Inputs(10U, 0U, 2U, 3U, 4U, 5U, 6U)
                .SrcVregs({4U, 0U, 1U, 2U, 3U, 4U, 5U});
            INST(8U, Opcode::Return).u32().Inputs(1U);
        }
    }

    auto suspend = &INS(7U);
    RunRegAllocatorsAndCheck(graph, [suspend]([[maybe_unused]] Graph *checkGraph, TestGraphCloner *cloner) {
        auto clonedSuspend = cloner->GetClone(suspend);
        ASSERT_NE(clonedSuspend, nullptr);
        auto spillSlots = CollectRegisterToStackSpillSlotsBefore(clonedSuspend);
        for (auto slot : spillSlots) {
            auto hasReload = HasSpillFillDataAfter(clonedSuspend, [slot](const SpillFillData &sf) {
                return sf.SrcType() == LocationType::STACK && sf.DstType() == LocationType::REGISTER &&
                       sf.SrcValue() == slot;
            });
            EXPECT_FALSE(hasReload);
        }
    });
}

TEST_F(RegAllocCommonTest, SaveStateSuspendStackInputReloadAfterResume)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(10U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveStateSuspend).Inputs(10U, 0U).SrcVregs({4U, 0U});
            INST(3U, Opcode::Add).u32().Inputs(0U, 0U);
            INST(4U, Opcode::Return).u32().Inputs(3U);
        }
    }

    auto valueInst = &INS(0U);
    auto suspend = &INS(2U);
    auto postUseInst = &INS(3U);
    const uint32_t stackParamSlot = 0U;
    RunRegAllocatorsAndCheckWithPrepare(
        graph,
        [valueInst]([[maybe_unused]] Graph *preparedGraph, TestGraphCloner *cloner) {
            SetStackParameterLocation(cloner->GetClone(valueInst), stackParamSlot);
        },
        [suspend, postUseInst]([[maybe_unused]] Graph *checkGraph, TestGraphCloner *cloner) {
            CheckStackValueReloadedAfterSuspend(cloner->GetClone(suspend), cloner->GetClone(postUseInst),
                                                stackParamSlot, LocationType::STACK_PARAMETER);
        });
}

TEST_F(RegAllocCommonTest, SaveStateSuspendStackInputWithoutReload)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(10U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveStateSuspend).Inputs(10U, 0U).SrcVregs({4U, 0U});
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }

    auto valueInst = &INS(0U);
    auto suspend = &INS(2U);
    const uint32_t stackParamSlot = 0U;
    RunRegAllocatorsAndCheckWithPrepare(
        graph,
        [valueInst]([[maybe_unused]] Graph *preparedGraph, TestGraphCloner *cloner) {
            SetStackParameterLocation(cloner->GetClone(valueInst), stackParamSlot);
        },
        [suspend]([[maybe_unused]] Graph *checkGraph, TestGraphCloner *cloner) {
            CheckStackValueNotReloadedAfterSuspend(cloner->GetClone(suspend), stackParamSlot,
                                                   LocationType::STACK_PARAMETER);
        });
}

TEST_F(RegAllocCommonTest, LocationsNoSplits)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 2U).s32();
        CONSTANT(2U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u32().Inputs(0U, 1U);
            INST(4U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5U, Opcode::CallStatic).InputsAutoType(3U, 2U, 0U, 1U, 4U).u32();
            INST(6U, Opcode::Return).u32().Inputs(5U);
        }
    }

    if (graph->GetArch() == Arch::AARCH32) {
        // Enable after full registers mask support the ARM32
        return;
    }

    // Check that there are no spill-fills in the graph
    RunRegAllocatorsAndCheck(graph, [](Graph *checkGraph, [[maybe_unused]] TestGraphCloner *cloner) {
        for (auto bb : checkGraph->GetBlocksRPO()) {
            LocationsNoSplitsCheckBB(bb);
        }
    });
}

TEST_F(RegAllocCommonTest, ImplicitNullCheckStackMap)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::NullCheck).ref().Inputs(5U, 6U);
            INST(8U, Opcode::LoadObject).s32().Inputs(7U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
    }
    INS(7U).CastToNullCheck()->SetImplicit(true);

    RunRegAllocatorsAndCheck(graph, [](Graph *checkGraph, [[maybe_unused]] TestGraphCloner *cloner) {
        auto bb = checkGraph->GetStartBlock()->GetSuccessor(0U);
        // Find null_check
        Inst *nullCheck = nullptr;
        for (auto inst : bb->AllInsts()) {
            if (inst->IsNullCheck()) {
                nullCheck = inst;
                break;
            }
        }
        ASSERT(nullCheck != nullptr);
        auto saveState = nullCheck->GetSaveState();
        // Check that save_state's inputs are added to the roots
        auto roots = saveState->GetRootsRegsMask();
        for (auto input : saveState->GetInputs()) {
            auto reg = input.GetInst()->GetDstReg();
            EXPECT_NE(reg, INVALID_REG);
            EXPECT_TRUE(roots.test(reg));
        }
    });
}

TEST_F(RegAllocCommonTest, DynMethodNargsParamReserve)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::Intrinsic)
                .any()
                .Inputs({{DataType::ANY, 1U}, {DataType::ANY, 0U}, {DataType::NO_TYPE, 2U}});
            INST(4U, Opcode::Return).any().Inputs(3U);
        }
    }

    RunRegAllocatorsAndCheck(graph, [](Graph *checkGraph, [[maybe_unused]] TestGraphCloner *cloner) {
        auto reg = Target(checkGraph->GetArch()).GetParamRegId(1U);

        for (auto inst : checkGraph->GetStartBlock()->Insts()) {
            if (!inst->IsSpillFill()) {
                continue;
            }
            auto sfs = inst->CastToSpillFill()->GetSpillFills();
            auto it = std::find_if(sfs.cbegin(), sfs.cend(), [reg](auto sf) {
                return sf.DstValue() == reg && sf.DstType() == LocationType::REGISTER;
            });
            ASSERT_EQ(it, sfs.cend());
        }
    });
}

TEST_F(RegAllocCommonTest, TempRegisters)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32().DstReg(10U);
        CONSTANT(1U, 2U).s32().DstReg(11U);
        CONSTANT(2U, 3U).s32().DstReg(12U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(4U, Opcode::CallStatic).InputsAutoType(0U, 1U, 2U, 3U).v0id().SetFlag(inst_flags::REQUIRE_TMP);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }

    auto checker = [](Graph *checkGraph, [[maybe_unused]] TestGraphCloner *cloner) {
        auto bb = checkGraph->GetStartBlock()->GetSuccessor(0U);
        Inst *callInst = nullptr;
        for (auto inst : bb->Insts()) {
            if (inst->IsCall()) {
                callInst = inst;
                break;
            }
        }
        ASSERT_TRUE(callInst != nullptr);
        auto tmpLocation = callInst->GetTmpLocation();
        EXPECT_TRUE(tmpLocation.IsFixedRegister());

        // Check that temp register doesn't overlay input registers
        for (size_t i = 0; i < callInst->GetInputsCount(); i++) {
            EXPECT_NE(tmpLocation, callInst->GetLocation(i));
        }
    };
    RunRegAllocatorsAndCheck(graph, RegMask {0xFFFFFFE1}, checker);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
