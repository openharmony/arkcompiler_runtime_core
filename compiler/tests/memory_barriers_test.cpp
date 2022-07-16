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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/memory_barriers.h"

namespace panda::compiler {
class MemoryBarrierTest : public GraphTest {
};

TEST_F(MemoryBarrierTest, Test1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0x2a).s64();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState).Inputs(0).SrcVregs({0});
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(1);
            INST(2, Opcode::NewArray).ref().Inputs(4, 0, 1);
            INST(3, Opcode::SaveState).Inputs(0, 2).SrcVregs({0, 1});
            INST(5, Opcode::NewObject).ref().Inputs(4, 3);
            INST(6, Opcode::SaveState).Inputs(0, 2, 5).SrcVregs({0, 1, 2});
            INST(7, Opcode::CallStatic).v0id().Inputs({{DataType::NO_TYPE, 6}});
            INST(8, Opcode::SaveState).Inputs(0, 2, 5).SrcVregs({0, 1, 2});
            INST(9, Opcode::LoadAndInitClass).ref().Inputs(8);
            INST(10, Opcode::NewObject).ref().Inputs(9, 8);
            INST(11, Opcode::SaveState).Inputs(0, 2, 5, 10).SrcVregs({0, 1, 2, 3});
            INST(12, Opcode::CallVirtual)
                .s64()
                .Inputs({{DataType::REFERENCE, 2}, {DataType::REFERENCE, 5}, {DataType::NO_TYPE, 6}});
            INST(13, Opcode::Return).ref().Inputs(10);
        }
    }
    ASSERT_EQ(INS(0).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(1).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(2).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(3).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(4).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(5).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(6).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(7).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(8).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(9).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(10).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(11).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(12).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(13).GetFlag(inst_flags::MEM_BARRIER), false);

    ASSERT_TRUE(GetGraph()->RunPass<OptimizeMemoryBarriers>());
    GraphChecker(GetGraph()).Check();

    ASSERT_EQ(INS(0).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(1).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(2).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(3).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(4).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(5).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(6).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(7).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(8).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(9).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(10).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(11).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(12).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(13).GetFlag(inst_flags::MEM_BARRIER), false);
}
}  //  namespace panda::compiler
