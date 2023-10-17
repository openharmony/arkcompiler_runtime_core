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

#include "check_resolver.h"
#include "common.h"

namespace panda::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(CommonTest, CheckResolverLenArray)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::NullCheck).ref().Inputs(1, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 0, 2);
            INST(6, Opcode::LoadArray).s32().Inputs(3, 5);
            INST(7, Opcode::Return).s32().Inputs(6);
        }
    }

    EXPECT_TRUE(graph->RunPass<CheckResolver>());
}

// NOLINTEND(readability-magic-numbers)

}  // namespace panda::bytecodeopt::test
