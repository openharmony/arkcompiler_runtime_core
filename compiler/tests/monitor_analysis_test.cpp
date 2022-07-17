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
#include "optimizer/analysis/monitor_analysis.h"

namespace panda::compiler {
class MonitorAnalysisTest : public GraphTest {
};

TEST_F(MonitorAnalysisTest, OneMonitorForOneBlock)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(1, Opcode::Monitor).v0id().Entry().Inputs(0, 4);
            INST(5, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(2, Opcode::Monitor).v0id().Exit().Inputs(0, 5);
            INST(3, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(2).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

TEST_F(MonitorAnalysisTest, OneMonitorForSeveralBlocks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).ref();
        CONSTANT(2, 10);
        CONSTANT(3, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(11, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(4, Opcode::Monitor).v0id().Entry().Inputs(1, 11);
            INST(5, Opcode::Compare).b().Inputs(0, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(7, Opcode::Mul).u64().Inputs(0, 3);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::Phi).u64().Inputs({{2, 0}, {3, 7}});
            INST(12, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Monitor).v0id().Exit().Inputs(1, 12);
            INST(10, Opcode::Return).u64().Inputs(8);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(2).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2).GetMonitorBlock());

    EXPECT_FALSE(BB(3).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(3).GetMonitorExitBlock());
    EXPECT_TRUE(BB(3).GetMonitorBlock());

    EXPECT_FALSE(BB(4).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(4).GetMonitorExitBlock());
    EXPECT_TRUE(BB(4).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

TEST_F(MonitorAnalysisTest, TwoMonitorForSeveralBlocks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).ref();
        PARAMETER(2, 3).ref();
        CONSTANT(3, 10);
        CONSTANT(4, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(14, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(5, Opcode::Monitor).v0id().Entry().Inputs(1, 14);
            INST(15, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(6, Opcode::Monitor).v0id().Entry().Inputs(2, 15);
            INST(16, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(7, Opcode::Monitor).v0id().Exit().Inputs(1, 16);
            INST(8, Opcode::Compare).b().Inputs(0, 3);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(10, Opcode::Mul).u64().Inputs(0, 4);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(11, Opcode::Phi).u64().Inputs({{2, 0}, {3, 10}});
            INST(17, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(12, Opcode::Monitor).v0id().Exit().Inputs(2, 17);
            INST(13, Opcode::Return).u64().Inputs(11);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(2).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2).GetMonitorBlock());

    EXPECT_FALSE(BB(3).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(3).GetMonitorExitBlock());
    EXPECT_TRUE(BB(3).GetMonitorBlock());

    EXPECT_FALSE(BB(4).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(4).GetMonitorExitBlock());
    EXPECT_TRUE(BB(4).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

TEST_F(MonitorAnalysisTest, OneEntryMonitorAndTwoExitMonitors)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).ref();
        CONSTANT(2, 10);
        CONSTANT(3, 2);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(4, Opcode::Monitor).v0id().Entry().Inputs(1, 12);
            INST(5, Opcode::Compare).b().Inputs(0, 2);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(13, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(7, Opcode::Monitor).v0id().Exit().Inputs(1, 13);
            INST(8, Opcode::Mul).u64().Inputs(0, 3);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(14, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(9, Opcode::Monitor).v0id().Exit().Inputs(1, 14);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(10, Opcode::Phi).u64().Inputs({{4, 0}, {3, 8}});
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
    EXPECT_TRUE(BB(2).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(2).GetMonitorExitBlock());
    EXPECT_TRUE(BB(2).GetMonitorBlock());

    EXPECT_FALSE(BB(3).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(3).GetMonitorExitBlock());
    EXPECT_TRUE(BB(3).GetMonitorBlock());

    EXPECT_FALSE(BB(4).GetMonitorEntryBlock());
    EXPECT_TRUE(BB(4).GetMonitorExitBlock());
    EXPECT_TRUE(BB(4).GetMonitorBlock());

    EXPECT_FALSE(BB(5).GetMonitorEntryBlock());
    EXPECT_FALSE(BB(5).GetMonitorExitBlock());
    EXPECT_FALSE(BB(5).GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetStartBlock()->GetMonitorBlock());

    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorEntryBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorExitBlock());
    EXPECT_FALSE(GetGraph()->GetEndBlock()->GetMonitorBlock());
}

/*
 *  Kernal case:
 *    bb 2
 *    |    \
 *    |      bb 3
 *    |    MonitorEntry
 *    |    /
 *    bb 4   - Conditional is equal to bb 2
 *    |    \
 *    |      bb 5
 *    |    MonitorExit
 *    |    /
 *    bb 6
 *
 *  The monitor analysis marks all blocks (excluding bb 2) as BlockMonitor
 */
TEST_F(MonitorAnalysisTest, KernalCase)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).ref();
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().Inputs(0, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(6, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(7, Opcode::Monitor).v0id().Entry().Inputs(1, 6);
        }
        BASIC_BLOCK(4, 5, 6)
        {
            INST(10, Opcode::Compare).b().Inputs(0, 2);
            INST(11, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(10);
        }
        BASIC_BLOCK(5, 6)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(13, Opcode::Monitor).v0id().Exit().Inputs(1, 12);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(15, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_FALSE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}

/*
 *    bb 2
 *    |    \
 *    |      bb 3
 *    |    MonitorEntry
 *    |    /
 *    bb 4
 *    |
 *    bb 5
 *    MonitorExit
 *    |
 *    bb 6
 *
 *  MonitorAnalysis must return false because
 *  - MonitorEntry is optional
 *  - MonitorExit is not optional
 */
TEST_F(MonitorAnalysisTest, InconsistentMonitorsNumberCase1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).ref();
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(4, Opcode::Compare).b().Inputs(0, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(6, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(7, Opcode::Monitor).v0id().Entry().Inputs(1, 6);
        }
        BASIC_BLOCK(4, 5) {}
        BASIC_BLOCK(5, 6)
        {
            INST(12, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(13, Opcode::Monitor).v0id().Exit().Inputs(1, 12);
        }
        BASIC_BLOCK(6, -1)
        {
            INST(15, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_FALSE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}

/*
 *    bb 2
 *    MonitorEntry
 *    |   \
 *    |    bb 3
 *    |    MonitorExit
 *    |   /
 *    bb 4
 *    MonitorExit
 *    |
 *    bb 5
 *
 *  MonitorAnalysis must return false because two Exits can happen for the single monitor
 */
TEST_F(MonitorAnalysisTest, InconsistentMonitorsNumberCase2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 2).ref();
        CONSTANT(2, 10);
        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(14, Opcode::Monitor).v0id().Entry().Inputs(1, 3);
            INST(4, Opcode::Compare).b().Inputs(0, 2);
            INST(5, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(4);
        }
        BASIC_BLOCK(3, 4)
        {
            INST(6, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(7, Opcode::Monitor).v0id().Exit().Inputs(1, 6);
        }
        BASIC_BLOCK(4, 5)
        {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(9, Opcode::Monitor).v0id().Exit().Inputs(1, 8);
        }
        BASIC_BLOCK(5, -1)
        {
            INST(15, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_FALSE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}

/*
 *  MonitorAnalysis should be Ok about Monitor Entry/Exit conunter mismatch when Throw happens within
 *  the synchronized block
 */
TEST_F(MonitorAnalysisTest, MonitorAndThrow)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).ref();
        PARAMETER(1, 2).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(4, Opcode::Monitor).v0id().Entry().Inputs(1, 3);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::Throw).Inputs(0, 5);
        }
    }
    GetGraph()->RunPass<MonitorAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<MonitorAnalysis>());
}

}  // namespace panda::compiler
