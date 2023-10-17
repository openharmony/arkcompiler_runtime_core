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
#include "optimizer/analysis/catch_inputs.h"

namespace panda::compiler {
class CatchInputsTest : public GraphTest {};

TEST_F(CatchInputsTest, MarkCatchInputs)
{
    static constexpr int ESCAPED_INST_ID = 6;
    // NOLINTBEGIN(readability-magic-numbers)
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).i32();
        PARAMETER(1, 1).i32();

        BASIC_BLOCK(2, 3, 4)  // try_begin
        {
            INST(3, Opcode::Try).CatchTypeIds({0});
        }

        BASIC_BLOCK(3, 7)  // try
        {
            INST(4, Opcode::Mul).i32().Inputs(0, 0);
            INST(5, Opcode::Mul).i32().Inputs(1, 1);
            INST(ESCAPED_INST_ID, Opcode::Add).i32().Inputs(4, 5);
            INST(7, Opcode::Mul).i32().Inputs(4, 5);
        }

        BASIC_BLOCK(7, 5, 4)  // try_end
        {
        }

        BASIC_BLOCK(4, 6)  // catch_begin
        {
            INST(8, Opcode::CatchPhi).i32().Inputs(ESCAPED_INST_ID);
        }

        BASIC_BLOCK(6, -1)  // catch
        {
            INST(9, Opcode::Mul).i32().Inputs(8, 8);
            INST(10, Opcode::Return).i32().Inputs(9);
        }

        BASIC_BLOCK(5, -1)
        {
            INST(11, Opcode::Return).i32().Inputs(7);
        }
    }

    BB(2).SetTryBegin(true);
    BB(3).SetTry(true);
    BB(7).SetTryEnd(true);
    BB(4).SetCatchBegin(true);
    BB(6).SetCatch(true);
    // NOLINTEND(readability-magic-numbers)

    GetGraph()->RunPass<CatchInputs>();

    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (bb == nullptr) {
            continue;
        }
        for (auto inst : bb->AllInstsSafe()) {
            // Only inst 6 should have a flag
            ASSERT_EQ(inst->GetFlag(inst_flags::Flags::CATCH_INPUT), inst->GetId() == ESCAPED_INST_ID);
        }
    }
}

TEST_F(CatchInputsTest, HandlePhi)
{
    static constexpr int PARAM_ID = 0;
    static constexpr int PHI_ID = 2;
    static constexpr int LOAD_ID = 4;
    // NOLINTBEGIN(readability-magic-numbers)
    GRAPH(GetGraph())
    {
        PARAMETER(PARAM_ID, 0).ref();

        BASIC_BLOCK(2, 3, 8)
        {
            INST(PHI_ID, Opcode::Phi).ref().Inputs(PARAM_ID, PHI_ID, LOAD_ID);
            INST(3, Opcode::IfImm).SrcType(DataType::REFERENCE).Inputs(PHI_ID).Imm(0).CC(CC_NE);
        }

        BASIC_BLOCK(8, 2, 7)
        {
            INST(7, Opcode::IfImm).SrcType(DataType::REFERENCE).Inputs(PHI_ID).Imm(0).CC(CC_NE);
        }

        BASIC_BLOCK(3, 4) {}

        BASIC_BLOCK(4, 5)
        {
            INST(LOAD_ID, Opcode::LoadObject).ref().Inputs(PHI_ID);
        }

        BASIC_BLOCK(5, 2, 6) {}

        BASIC_BLOCK(6, 7)
        {
            INST(5, Opcode::CatchPhi).ref().Inputs(PHI_ID);
        }

        BASIC_BLOCK(7, -1)
        {
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    BB(3).SetTryBegin(true);
    BB(4).SetTry(true);
    BB(5).SetTryEnd(true);
    BB(6).SetCatchBegin(true);
    BB(6).SetCatch(true);
    // NOLINTEND(readability-magic-numbers)

    GetGraph()->RunPass<CatchInputs>();

    ASSERT_TRUE(INS(PARAM_ID).GetFlag(inst_flags::Flags::CATCH_INPUT));
    ASSERT_TRUE(INS(PHI_ID).GetFlag(inst_flags::Flags::CATCH_INPUT));
    ASSERT_TRUE(INS(LOAD_ID).GetFlag(inst_flags::Flags::CATCH_INPUT));
}
}  // namespace panda::compiler