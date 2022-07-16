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

#include "optimizer/optimizations/scheduler.h"

namespace panda::compiler {
class SchedulerTest : public GraphTest {
};

TEST_F(SchedulerTest, Basic)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 42);
        CONSTANT(1, 43);
        CONSTANT(2, 44);
        CONSTANT(3, 45);
        CONSTANT(4, 46);
        CONSTANT(5, 47);
        CONSTANT(6, 48);
        CONSTANT(7, 49);

        BASIC_BLOCK(2, -1)
        {
            INST(8, Opcode::Add).u64().Inputs(0, 1);
            INST(9, Opcode::Add).u64().Inputs(2, 3);
            // should be moved down
            INST(10, Opcode::Add).u64().Inputs(8, 9);

            INST(11, Opcode::Add).u64().Inputs(4, 5);
            INST(12, Opcode::Add).u64().Inputs(6, 7);
            INST(13, Opcode::Add).u64().Inputs(11, 12);
            // Grand total
            INST(14, Opcode::Add).u64().Inputs(10, 13);
            INST(15, Opcode::Return).u64().Inputs(14);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    ASSERT_EQ(INS(8).GetNext(), &INS(9));
    ASSERT_NE(INS(9).GetNext(), &INS(10));

    EXPECT_TRUE((INS(11).GetNext() == &INS(10)) || (INS(12).GetNext() == &INS(10)));

    ASSERT_EQ(INS(13).GetNext(), &INS(14));
    ASSERT_EQ(INS(14).GetNext(), &INS(15));
}

TEST_F(SchedulerTest, LoadBarrier)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(2, 42);
        CONSTANT(3, 43);
        CONSTANT(4, 44);
        CONSTANT(5, 45);
        CONSTANT(6, 46);
        CONSTANT(7, 47);
        CONSTANT(8, 48);
        CONSTANT(9, 5);  // len array

        BASIC_BLOCK(2, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(11, Opcode::Add).u64().Inputs(4, 5);
            // should be moved down
            INST(12, Opcode::Add).u64().Inputs(10, 11);

            INST(13, Opcode::Add).u64().Inputs(6, 7);

            INST(21, Opcode::SafePoint).Inputs(0, 1).SrcVregs({0, 1});
            INST(14, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(15, Opcode::BoundsCheck).s32().Inputs(9, 0, 14);
            // can't move up because of SafePoint
            INST(16, Opcode::LoadArray).u64().Inputs(1, 15);

            INST(17, Opcode::Add).u64().Inputs(8, 16);
            INST(18, Opcode::Add).u64().Inputs(13, 17);

            INST(19, Opcode::Add).u64().Inputs(12, 18);
            INST(20, Opcode::Return).u64().Inputs(19);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    ASSERT_EQ(INS(11).GetNext(), &INS(13));
    ASSERT_EQ(INS(13).GetNext(), &INS(12));
    ASSERT_EQ(INS(12).GetNext(), &INS(21));
    ASSERT_EQ(INS(21).GetNext(), &INS(14));
    ASSERT_EQ(INS(15).GetNext(), &INS(16));
    ASSERT_EQ(INS(16).GetNext(), &INS(17));
}

TEST_F(SchedulerTest, Load)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(2, 42);
        CONSTANT(3, 43);
        CONSTANT(4, 44);
        CONSTANT(5, 45);
        CONSTANT(6, 46);
        CONSTANT(7, 47);
        CONSTANT(8, 48);
        CONSTANT(9, 5);  // len array

        BASIC_BLOCK(2, -1)
        {
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(11, Opcode::Add).u64().Inputs(4, 5);
            // should be moved down
            INST(12, Opcode::Add).u64().Inputs(10, 11);

            INST(13, Opcode::Add).u64().Inputs(6, 7);

            // all three should be moved up
            INST(14, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(15, Opcode::BoundsCheck).s32().Inputs(9, 0, 14);
            INST(16, Opcode::LoadArray).u64().Inputs(1, 15);

            INST(17, Opcode::Add).u64().Inputs(8, 16);
            INST(18, Opcode::Add).u64().Inputs(13, 17);

            INST(19, Opcode::Add).u64().Inputs(12, 18);
            INST(20, Opcode::Return).u64().Inputs(19);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    ASSERT_EQ(INS(14).GetNext(), &INS(10));
    ASSERT_EQ(INS(10).GetNext(), &INS(15));
    ASSERT_EQ(INS(15).GetNext(), &INS(11));
    ASSERT_EQ(INS(11).GetNext(), &INS(16));
    ASSERT_EQ(INS(16).GetNext(), &INS(13));
    ASSERT_EQ(INS(13).GetNext(), &INS(12));
    ASSERT_EQ(INS(12).GetNext(), &INS(17));
    ASSERT_EQ(INS(17).GetNext(), &INS(18));
    ASSERT_EQ(INS(18).GetNext(), &INS(19));
    ASSERT_EQ(INS(19).GetNext(), &INS(20));
}

TEST_F(SchedulerTest, LoadI)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::BoundsCheckI).s32().Inputs(0, 2).Imm(1);
            INST(4, Opcode::StoreArrayI).u64().Inputs(1, 0).Imm(1);
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::BoundsCheckI).s32().Inputs(0, 5).Imm(0);
            INST(7, Opcode::LoadArrayI).u64().Inputs(1).Imm(0);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Scheduler>());
}

TEST_F(SchedulerTest, TrickyLoadI)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::BoundsCheckI).s32().Inputs(0, 5).Imm(0);
            // Manually moved here
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::BoundsCheckI).s32().Inputs(0, 2).Imm(1);
            INST(4, Opcode::StoreArrayI).u64().Inputs(1, 0).Imm(1);
            // But than all 3 may be moved below the load
            INST(7, Opcode::LoadArrayI).u64().Inputs(1).Imm(0);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::BoundsCheckI).s32().Inputs(0, 5).Imm(0);
            INST(7, Opcode::LoadArrayI).u64().Inputs(1).Imm(0);

            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::BoundsCheckI).s32().Inputs(0, 2).Imm(1);
            INST(4, Opcode::StoreArrayI).u64().Inputs(1, 0).Imm(1);

            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(SchedulerTest, MustAliasLoadI)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(5, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(6, Opcode::BoundsCheckI).s32().Inputs(0, 5).Imm(42);
            // Manually moved here
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::BoundsCheckI).s32().Inputs(0, 2).Imm(42);
            INST(4, Opcode::StoreArrayI).u64().Inputs(1, 0).Imm(42);
            // But than all 3 may be moved below the load
            INST(7, Opcode::LoadArrayI).u64().Inputs(1).Imm(42);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Scheduler>());
}

TEST_F(SchedulerTest, LoadPair)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(11, 41);
        CONSTANT(12, 42);
        CONSTANT(13, 43);
        CONSTANT(14, 44);
        CONSTANT(15, 45);
        CONSTANT(16, 46);
        CONSTANT(17, 47);
        CONSTANT(18, 48);
        CONSTANT(19, 49);
        CONSTANT(20, 50);
        CONSTANT(21, 51);
        CONSTANT(22, 52);
        CONSTANT(23, 53);
        CONSTANT(24, 54);

        BASIC_BLOCK(2, -1)
        {
            INST(31, Opcode::Add).u64().Inputs(11, 12);
            INST(32, Opcode::Add).u64().Inputs(13, 14);
            INST(41, Opcode::Add).u64().Inputs(31, 32);

            INST(33, Opcode::Add).u64().Inputs(15, 16);
            INST(34, Opcode::Add).u64().Inputs(17, 18);
            INST(42, Opcode::Add).u64().Inputs(33, 34);

            INST(35, Opcode::Add).u64().Inputs(19, 20);
            INST(36, Opcode::Add).u64().Inputs(21, 22);
            INST(43, Opcode::Add).u64().Inputs(35, 36);

            INST(92, Opcode::LoadArrayPair).u64().Inputs(1, 0);
            INST(93, Opcode::LoadPairPart).u64().Inputs(92).Imm(0);
            INST(94, Opcode::LoadPairPart).u64().Inputs(92).Imm(1);

            INST(37, Opcode::Add).u64().Inputs(23, 93);
            INST(38, Opcode::Add).u64().Inputs(24, 94);
            INST(44, Opcode::Add).u64().Inputs(37, 38);

            INST(51, Opcode::Add).u64().Inputs(41, 42);
            INST(52, Opcode::Add).u64().Inputs(43, 44);

            INST(61, Opcode::Add).u64().Inputs(51, 52);
            INST(62, Opcode::Return).u64().Inputs(61);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();  // index
        PARAMETER(1, 1).ref();
        CONSTANT(11, 41);
        CONSTANT(12, 42);
        CONSTANT(13, 43);
        CONSTANT(14, 44);
        CONSTANT(15, 45);
        CONSTANT(16, 46);
        CONSTANT(17, 47);
        CONSTANT(18, 48);
        CONSTANT(19, 49);
        CONSTANT(20, 50);
        CONSTANT(21, 51);
        CONSTANT(22, 52);
        CONSTANT(23, 53);
        CONSTANT(24, 54);

        BASIC_BLOCK(2, -1)
        {
            INST(92, Opcode::LoadArrayPair).u64().Inputs(1, 0);
            INST(93, Opcode::LoadPairPart).u64().Inputs(92).Imm(0);
            INST(94, Opcode::LoadPairPart).u64().Inputs(92).Imm(1);

            INST(31, Opcode::Add).u64().Inputs(11, 12);
            INST(32, Opcode::Add).u64().Inputs(13, 14);
            INST(33, Opcode::Add).u64().Inputs(15, 16);
            INST(34, Opcode::Add).u64().Inputs(17, 18);
            INST(35, Opcode::Add).u64().Inputs(19, 20);
            INST(36, Opcode::Add).u64().Inputs(21, 22);
            INST(37, Opcode::Add).u64().Inputs(23, 93);
            INST(38, Opcode::Add).u64().Inputs(24, 94);

            INST(41, Opcode::Add).u64().Inputs(31, 32);
            INST(42, Opcode::Add).u64().Inputs(33, 34);
            INST(43, Opcode::Add).u64().Inputs(35, 36);
            INST(44, Opcode::Add).u64().Inputs(37, 38);

            INST(51, Opcode::Add).u64().Inputs(41, 42);
            INST(52, Opcode::Add).u64().Inputs(43, 44);

            INST(61, Opcode::Add).u64().Inputs(51, 52);
            INST(62, Opcode::Return).u64().Inputs(61);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(SchedulerTest, NonVolatileLoadObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).s32().Inputs(1, 2);
            INST(5, Opcode::Add).s32().Inputs(1, 3);
            INST(6, Opcode::Add).s32().Inputs(2, 3);
            INST(7, Opcode::Add).s32().Inputs(4, 5);
            INST(8, Opcode::Add).s32().Inputs(6, 7);
            INST(9, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(10, Opcode::NullCheck).ref().Inputs(0, 9);
            INST(11, Opcode::LoadObject).s32().Inputs(10).TypeId(152);
            INST(12, Opcode::Add).s32().Inputs(8, 11);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(9, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::Add).s32().Inputs(1, 2);
            INST(10, Opcode::NullCheck).ref().Inputs(0, 9);
            INST(5, Opcode::Add).s32().Inputs(1, 3);
            INST(11, Opcode::LoadObject).s32().Inputs(10).TypeId(152);
            INST(6, Opcode::Add).s32().Inputs(2, 3);
            INST(7, Opcode::Add).s32().Inputs(4, 5);
            INST(8, Opcode::Add).s32().Inputs(6, 7);
            INST(12, Opcode::Add).s32().Inputs(8, 11);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(SchedulerTest, VolatileLoadObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).s32().Inputs(1, 2);
            INST(5, Opcode::Add).s32().Inputs(1, 3);
            INST(6, Opcode::Add).s32().Inputs(2, 3);
            INST(7, Opcode::Add).s32().Inputs(4, 5);
            INST(8, Opcode::Add).s32().Inputs(6, 7);
            INST(9, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(10, Opcode::NullCheck).ref().Inputs(0, 9);
            INST(11, Opcode::LoadObject).s32().Inputs(10).TypeId(152).Volatile();
            INST(12, Opcode::Add).s32().Inputs(8, 11);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).s32();
        PARAMETER(2, 2).s32();
        PARAMETER(3, 3).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Add).s32().Inputs(1, 2);
            INST(5, Opcode::Add).s32().Inputs(1, 3);
            INST(9, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(6, Opcode::Add).s32().Inputs(2, 3);
            INST(7, Opcode::Add).s32().Inputs(4, 5);
            INST(10, Opcode::NullCheck).ref().Inputs(0, 9);
            INST(8, Opcode::Add).s32().Inputs(6, 7);
            INST(11, Opcode::LoadObject).s32().Inputs(10).TypeId(152).Volatile();
            INST(12, Opcode::Add).s32().Inputs(8, 11);
            INST(13, Opcode::Return).s32().Inputs(12);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
}  // namespace panda::compiler
