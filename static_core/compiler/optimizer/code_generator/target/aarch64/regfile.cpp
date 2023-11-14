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
#include "target/aarch64/target.h"
#include "regfile.h"

namespace panda::compiler::aarch64 {
Aarch64RegisterDescription::Aarch64RegisterDescription(ArenaAllocator *allocator)
    : RegistersDescription(allocator, Arch::AARCH64), used_regs_(allocator->Adapter())
{
}

bool Aarch64RegisterDescription::IsRegUsed(ArenaVector<Reg> vec_reg, Reg reg)
{
    auto equality = [reg](Reg in) { return (reg.GetId() == in.GetId()) && (reg.GetType() == in.GetType()); };
    return (std::find_if(vec_reg.begin(), vec_reg.end(), equality) != vec_reg.end());
}

ArenaVector<Reg> Aarch64RegisterDescription::GetCalleeSaved()
{
    ArenaVector<Reg> out(GetAllocator()->Adapter());
    for (uint32_t i = 0; i <= MAX_NUM_REGS; ++i) {
        if ((callee_savedv_.GetList() & (UINT64_C(1) << i)) != 0) {
            out.emplace_back(Reg(i, FLOAT64_TYPE));
        }
        if (i == MAX_NUM_REGS) {
            break;
        }
        if ((callee_saved_.GetList() & (UINT64_C(1) << i)) != 0) {
            out.emplace_back(Reg(i, INT64_TYPE));
        }
    }
    return out;
}

void Aarch64RegisterDescription::SetCalleeSaved(const ArenaVector<Reg> &regs)
{
    callee_saved_ = vixl::aarch64::kCalleeSaved;
    callee_savedv_ = vixl::aarch64::kCalleeSavedV;

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        bool vector_used = IsRegUsed(regs, Reg(i, FLOAT64_TYPE));
        if (vector_used) {
            callee_savedv_.Combine(i);
        } else {
            callee_savedv_.Remove(i);
        }
        bool scalar_used = IsRegUsed(regs, Reg(i, INT64_TYPE));
        if (scalar_used) {
            callee_saved_.Combine(i);
        } else {
            callee_saved_.Remove(i);
        }
    }
    // Remove return-value from callee
    callee_saved_.Remove(0);

    // We can safely skip saving THREAD_REG if it is in the regmask
    // of the regdescr (i.e. regalloc can not use it).
    if (GetRegMask().Test(GetThreadReg(Arch::AARCH64))) {
        callee_saved_.Remove(GetThreadReg(Arch::AARCH64));
    }
}

void Aarch64RegisterDescription::SetUsedRegs(const ArenaVector<Reg> &regs)
{
    used_regs_ = regs;

    // Update current lists - to do not use old data
    callee_saved_ = vixl::aarch64::kCalleeSaved;
    caller_saved_ = vixl::aarch64::kCallerSaved;

    // Need remove return value from callee
    callee_saved_.Remove(0);

    // We can safely skip saving THREAD_REG if it is in the regmask
    // of the regdescr (i.e. regalloc can not use it).
    if (GetRegMask().Test(GetThreadReg(Arch::AARCH64))) {
        callee_saved_.Remove(GetThreadReg(Arch::AARCH64));
    }

    callee_savedv_ = vixl::aarch64::kCalleeSavedV;
    caller_savedv_ = vixl::aarch64::kCallerSavedV;

    for (uint32_t i = 0; i <= MAX_NUM_REGS; ++i) {
        // IsRegUsed use used_regs_ variable
        bool scalar_used = IsRegUsed(used_regs_, Reg(i, INT64_TYPE));
        if (!scalar_used && ((callee_saved_.GetList() & (UINT64_C(1) << i)) != 0)) {
            callee_saved_.Remove(i);
        }
        if (!scalar_used && ((caller_saved_.GetList() & (UINT64_C(1) << i)) != 0)) {
            caller_saved_.Remove(i);
        }
        bool vector_used = IsRegUsed(used_regs_, Reg(i, FLOAT64_TYPE));
        if (!vector_used && ((callee_savedv_.GetList() & (UINT64_C(1) << i)) != 0)) {
            callee_savedv_.Remove(i);
            allignment_vreg_callee_ = i;
        }
        if (!vector_used && ((caller_savedv_.GetList() & (UINT64_C(1) << i)) != 0)) {
            caller_savedv_.Remove(i);
            allignment_vreg_caller_ = i;
        }
    }
}

}  // namespace panda::compiler::aarch64
