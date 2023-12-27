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
#include "target/aarch64/target.h"
#include "regfile.h"

namespace panda::compiler::aarch64 {
Aarch64RegisterDescription::Aarch64RegisterDescription(ArenaAllocator *allocator)
    : RegistersDescription(allocator, Arch::AARCH64), usedRegs_(allocator->Adapter())
{
}

bool Aarch64RegisterDescription::IsRegUsed(ArenaVector<Reg> vecReg, Reg reg)
{
    auto equality = [reg](Reg in) { return (reg.GetId() == in.GetId()) && (reg.GetType() == in.GetType()); };
    return (std::find_if(vecReg.begin(), vecReg.end(), equality) != vecReg.end());
}

ArenaVector<Reg> Aarch64RegisterDescription::GetCalleeSaved()
{
    ArenaVector<Reg> out(GetAllocator()->Adapter());
    for (uint32_t i = 0; i <= MAX_NUM_REGS; ++i) {
        if ((calleeSavedv_.GetList() & (UINT64_C(1) << i)) != 0) {
            out.emplace_back(Reg(i, FLOAT64_TYPE));
        }
        if (i == MAX_NUM_REGS) {
            break;
        }
        if ((calleeSaved_.GetList() & (UINT64_C(1) << i)) != 0) {
            out.emplace_back(Reg(i, INT64_TYPE));
        }
    }
    return out;
}

void Aarch64RegisterDescription::SetCalleeSaved(const ArenaVector<Reg> &regs)
{
    calleeSaved_ = vixl::aarch64::kCalleeSaved;
    calleeSavedv_ = vixl::aarch64::kCalleeSavedV;

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        bool vectorUsed = IsRegUsed(regs, Reg(i, FLOAT64_TYPE));
        if (vectorUsed) {
            calleeSavedv_.Combine(i);
        } else {
            calleeSavedv_.Remove(i);
        }
        bool scalarUsed = IsRegUsed(regs, Reg(i, INT64_TYPE));
        if (scalarUsed) {
            calleeSaved_.Combine(i);
        } else {
            calleeSaved_.Remove(i);
        }
    }
    // Remove return-value from callee
    calleeSaved_.Remove(0);

    // We can safely skip saving THREAD_REG if it is in the regmask
    // of the regdescr (i.e. regalloc can not use it).
    if (GetRegMask().Test(GetThreadReg(Arch::AARCH64))) {
        calleeSaved_.Remove(GetThreadReg(Arch::AARCH64));
    }
}

void Aarch64RegisterDescription::SetUsedRegs(const ArenaVector<Reg> &regs)
{
    usedRegs_ = regs;

    // Update current lists - to do not use old data
    calleeSaved_ = vixl::aarch64::kCalleeSaved;
    callerSaved_ = vixl::aarch64::kCallerSaved;

    // Need remove return value from callee
    calleeSaved_.Remove(0);

    // We can safely skip saving THREAD_REG if it is in the regmask
    // of the regdescr (i.e. regalloc can not use it).
    if (GetRegMask().Test(GetThreadReg(Arch::AARCH64))) {
        calleeSaved_.Remove(GetThreadReg(Arch::AARCH64));
    }

    calleeSavedv_ = vixl::aarch64::kCalleeSavedV;
    callerSavedv_ = vixl::aarch64::kCallerSavedV;

    for (uint32_t i = 0; i <= MAX_NUM_REGS; ++i) {
        // IsRegUsed use used_regs_ variable
        bool scalarUsed = IsRegUsed(usedRegs_, Reg(i, INT64_TYPE));
        if (!scalarUsed && ((calleeSaved_.GetList() & (UINT64_C(1) << i)) != 0)) {
            calleeSaved_.Remove(i);
        }
        if (!scalarUsed && ((callerSaved_.GetList() & (UINT64_C(1) << i)) != 0)) {
            callerSaved_.Remove(i);
        }
        bool vectorUsed = IsRegUsed(usedRegs_, Reg(i, FLOAT64_TYPE));
        if (!vectorUsed && ((calleeSavedv_.GetList() & (UINT64_C(1) << i)) != 0)) {
            calleeSavedv_.Remove(i);
            allignmentVregCallee_ = i;
        }
        if (!vectorUsed && ((callerSavedv_.GetList() & (UINT64_C(1) << i)) != 0)) {
            callerSavedv_.Remove(i);
            allignmentVregCaller_ = i;
        }
    }
}

}  // namespace panda::compiler::aarch64
