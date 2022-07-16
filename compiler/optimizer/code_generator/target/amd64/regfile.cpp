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

/*
Register file implementation.
Reserve registers.
*/

#include "registers_description.h"
#include "target/amd64/target.h"
#include "regfile.h"

namespace panda::compiler::amd64 {
Amd64RegisterDescription::Amd64RegisterDescription(ArenaAllocator *allocator)
    : RegistersDescription(allocator, Arch::X86_64), used_regs_(allocator->Adapter())
{
}

bool Amd64RegisterDescription::IsRegUsed(ArenaVector<Reg> vec_reg, Reg reg)
{
    auto equality = [reg](Reg in) { return (reg.GetId() == in.GetId()) && (reg.GetType() == in.GetType()); };
    return (std::find_if(vec_reg.begin(), vec_reg.end(), equality) != vec_reg.end());
}

ArenaVector<Reg> Amd64RegisterDescription::GetCalleeSaved()
{
    ArenaVector<Reg> out(GetAllocator()->Adapter());
    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if (callee_saved_.Has(i)) {
            out.emplace_back(Reg(i, INT64_TYPE));
        }
        if (callee_savedv_.Has(i)) {
            out.emplace_back(Reg(i, FLOAT64_TYPE));
        }
    }
    return out;
}

void Amd64RegisterDescription::SetCalleeSaved(const ArenaVector<Reg> &regs)
{
    callee_saved_ = RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue());
    callee_savedv_ = RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue());  // empty

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        bool scalar_used = IsRegUsed(regs, Reg(i, INT64_TYPE));
        if (scalar_used) {
            callee_saved_.Add(i);
        } else {
            callee_saved_.Remove(i);
        }
        bool vector_used = IsRegUsed(regs, Reg(i, FLOAT64_TYPE));
        if (vector_used) {
            callee_savedv_.Add(i);
        } else {
            callee_savedv_.Remove(i);
        }
    }
    // Remove return-value from callee
    callee_saved_.Remove(ConvertRegNumber(asmjit::x86::rax.id()));
}

void Amd64RegisterDescription::SetUsedRegs(const ArenaVector<Reg> &regs)
{
    used_regs_ = regs;

    // Update current lists - to do not use old data
    callee_saved_ = RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue());
    caller_saved_ = RegList(GetCallerRegsMask(Arch::X86_64, false).GetValue());

    callee_savedv_ = RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue());  // empty
    caller_savedv_ = RegList(GetCallerRegsMask(Arch::X86_64, true).GetValue());

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        // IsRegUsed use used_regs_ variable
        bool scalar_used = IsRegUsed(used_regs_, Reg(i, INT64_TYPE));
        if (!scalar_used && callee_saved_.Has(i)) {
            callee_saved_.Remove(i);
        }
        if (!scalar_used && caller_saved_.Has(i)) {
            caller_saved_.Remove(i);
        }

        bool vector_used = IsRegUsed(used_regs_, Reg(i, FLOAT64_TYPE));
        if (!vector_used && callee_savedv_.Has(i)) {
            callee_savedv_.Remove(i);
        }
        if (!vector_used && caller_savedv_.Has(i)) {
            caller_savedv_.Remove(i);
        }
    }
}

}  // namespace panda::compiler::amd64
