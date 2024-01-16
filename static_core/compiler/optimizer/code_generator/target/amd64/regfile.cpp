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

/*
Register file implementation.
Reserve registers.
*/

#include "registers_description.h"
#include "target/amd64/target.h"
#include "regfile.h"

namespace ark::compiler::amd64 {
Amd64RegisterDescription::Amd64RegisterDescription(ArenaAllocator *allocator)
    : RegistersDescription(allocator, Arch::X86_64), usedRegs_(allocator->Adapter())
{
}

bool Amd64RegisterDescription::IsRegUsed(ArenaVector<Reg> vecReg, Reg reg)
{
    auto equality = [reg](Reg in) { return (reg.GetId() == in.GetId()) && (reg.GetType() == in.GetType()); };
    return (std::find_if(vecReg.begin(), vecReg.end(), equality) != vecReg.end());
}

ArenaVector<Reg> Amd64RegisterDescription::GetCalleeSaved()
{
    ArenaVector<Reg> out(GetAllocator()->Adapter());
    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if (calleeSaved_.Has(i)) {
            out.emplace_back(Reg(i, INT64_TYPE));
        }
        if (calleeSavedv_.Has(i)) {
            out.emplace_back(Reg(i, FLOAT64_TYPE));
        }
    }
    return out;
}

void Amd64RegisterDescription::SetCalleeSaved(const ArenaVector<Reg> &regs)
{
    calleeSaved_ = RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue());
    calleeSavedv_ = RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue());  // empty

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        bool scalarUsed = IsRegUsed(regs, Reg(i, INT64_TYPE));
        if (scalarUsed) {
            calleeSaved_.Add(i);
        } else {
            calleeSaved_.Remove(i);
        }
        bool vectorUsed = IsRegUsed(regs, Reg(i, FLOAT64_TYPE));
        if (vectorUsed) {
            calleeSavedv_.Add(i);
        } else {
            calleeSavedv_.Remove(i);
        }
    }
    // Remove return-value from callee
    calleeSaved_.Remove(ConvertRegNumber(asmjit::x86::rax.id()));
}

void Amd64RegisterDescription::SetUsedRegs(const ArenaVector<Reg> &regs)
{
    usedRegs_ = regs;

    // Update current lists - to do not use old data
    calleeSaved_ = RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue());
    callerSaved_ = RegList(GetCallerRegsMask(Arch::X86_64, false).GetValue());

    calleeSavedv_ = RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue());  // empty
    callerSavedv_ = RegList(GetCallerRegsMask(Arch::X86_64, true).GetValue());

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        // IsRegUsed use used_regs_ variable
        bool scalarUsed = IsRegUsed(usedRegs_, Reg(i, INT64_TYPE));
        if (!scalarUsed && calleeSaved_.Has(i)) {
            calleeSaved_.Remove(i);
        }
        if (!scalarUsed && callerSaved_.Has(i)) {
            callerSaved_.Remove(i);
        }

        bool vectorUsed = IsRegUsed(usedRegs_, Reg(i, FLOAT64_TYPE));
        if (!vectorUsed && calleeSavedv_.Has(i)) {
            calleeSavedv_.Remove(i);
        }
        if (!vectorUsed && callerSavedv_.Has(i)) {
            callerSavedv_.Remove(i);
        }
    }
}

}  // namespace ark::compiler::amd64
