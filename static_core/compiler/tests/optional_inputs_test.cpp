/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "optimizer/ir/inst.h"
#include "optimizer/ir/ir_constructor.h"
#include "unit_test.h"
#include "tests/inst_generator.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)
class OptionalInputsTest : public CommonTest {
public:
    template <Opcode OPCODE>
    auto *CreateInst()
    {
        return Inst::New<BaseTypeOf<OPCODE>>(GetAllocator(), OPCODE);
    }
};

TEST_F(OptionalInputsTest, TestAccessorsPresent)
{
    auto *parameter = CreateInst<Opcode::Parameter>();
    auto checkInst = [parameter](Inst *inst) {
        if (InstHasGCBarrierEntrypointInput(inst->GetOpcode())) {
            EXPECT_EQ(GetInstGCBarrierEntrypoint(inst), nullptr);
            SetInstGCBarrierEntrypoint(inst, parameter);
            EXPECT_EQ(GetInstGCBarrierEntrypoint(inst), parameter);
            ResetInstGCBarrierEntrypoint(inst);
            EXPECT_EQ(GetInstGCBarrierEntrypoint(inst), nullptr);
        }
    };

#define HANDLE_INST(OPCODE, ...) checkInst(CreateInst<Opcode::OPCODE>());  // CC-OFF(G.PRE.09) code generation
    OPCODE_LIST(HANDLE_INST);
#undef HANDLE_INST
}

TEST_F(OptionalInputsTest, TestAccessorsAbsent)
{
    // Death tests are very slow, check one random instruction
    auto *parameter = CreateInst<Opcode::Parameter>();
    auto *inst = CreateInst<Opcode::NullCheck>();
    EXPECT_DEATH(GetInstGCBarrierEntrypoint(inst), "");
    EXPECT_DEATH(SetInstGCBarrierEntrypoint(inst, parameter), "");
    EXPECT_DEATH(ResetInstGCBarrierEntrypoint(inst), "");
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
