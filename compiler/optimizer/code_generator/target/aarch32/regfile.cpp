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
#include "target/aarch32/target.h"
#include "regfile.h"

namespace panda::compiler::aarch32 {
/**
 * Default aarch32 calling convention registers
 * Callee
 *    r4-r11,r14
 *    d8-d15
 * Caller
 *    (r0-r3),r12
 *     d0-d7
 */
Aarch32RegisterDescription::Aarch32RegisterDescription(ArenaAllocator *allocator)
    : RegistersDescription(allocator, Arch::AARCH32),
      aarch32_reg_list_(allocator->Adapter()),
      used_regs_(allocator->Adapter())
{
    // Initialize Masm
    for (uint32_t i = 0; i <= MAX_NUM_REGS; ++i) {
        aarch32_reg_list_.emplace_back(Reg(i, INT32_TYPE));
        aarch32_reg_list_.emplace_back(Reg(i, FLOAT32_TYPE));
    }

    for (auto i = vixl::aarch32::r4.GetCode(); i < vixl::aarch32::r8.GetCode(); ++i) {
        caller_savedv_.set(i);
    }
}

bool Aarch32RegisterDescription::IsValid() const
{
    return !aarch32_reg_list_.empty();
}

bool Aarch32RegisterDescription::IsRegUsed(ArenaVector<Reg> vec_reg, Reg reg)
{
    auto equality = [reg](Reg in) { return (reg.GetId() == in.GetId()) && (reg.GetType() == in.GetType()); };
    return (std::find_if(vec_reg.begin(), vec_reg.end(), equality) != vec_reg.end());
}

/* static */
bool Aarch32RegisterDescription::IsTmp(Reg reg)
{
    if (reg.IsScalar()) {
        for (auto it : AARCH32_TMP_REG) {
            if (it == reg.GetId()) {
                return true;
            }
        }
        return false;
    }
    ASSERT(reg.IsFloat());
    for (auto it : AARCH32_TMP_VREG) {
        if (it == reg.GetId()) {
            return true;
        }
    }
    return false;
}

ArenaVector<Reg> Aarch32RegisterDescription::GetCalleeSaved()
{
    ArenaVector<Reg> out(GetAllocator()->Adapter());
    ASSERT(callee_saved_.size() == callee_savedv_.size());
    for (size_t i = 0; i < callee_saved_.size(); ++i) {
        if (callee_saved_.test(i)) {
            out.emplace_back(Reg(i, INT32_TYPE));
        }
        if ((callee_savedv_.test(i))) {
            out.emplace_back(Reg(i, FLOAT32_TYPE));
        }
    }
    return out;
}

void Aarch32RegisterDescription::SetCalleeSaved([[maybe_unused]] const ArenaVector<Reg> &regs)
{
    callee_saved_ = CALLEE_SAVED;
    callee_savedv_ = CALLEE_SAVEDV;
}

void Aarch32RegisterDescription::SetUsedRegs(const ArenaVector<Reg> &regs)
{
    used_regs_ = regs;

    ASSERT(callee_saved_.size() == caller_saved_.size());
    ASSERT(callee_savedv_.size() == caller_savedv_.size());

    allignment_reg_callee_ = vixl::aarch32::r10.GetCode();
    // TODO (pishin) need to resolve conflict
    allignment_reg_caller_ = vixl::aarch32::r10.GetCode();
    for (size_t i = 0; i < callee_saved_.size(); ++i) {
        // IsRegUsed use used_regs_ variable
        bool scalar_used = IsRegUsed(used_regs_, Reg(i, INT64_TYPE));
        bool is_tmp = IsTmp(Reg(i, INT32_TYPE));
        if ((!scalar_used && ((callee_saved_.test(i)))) || is_tmp) {
            callee_saved_.reset(i);
            allignment_reg_callee_ = i;
        }
        if (!scalar_used && ((caller_saved_.test(i)))) {
            allignment_reg_caller_ = i;
        }
        bool is_vtmp = IsTmp(Reg(i, FLOAT32_TYPE));

        bool vector_used = IsRegUsed(used_regs_, Reg(i, FLOAT64_TYPE));
        if ((!vector_used && ((callee_savedv_.test(i)))) || is_vtmp) {
            callee_savedv_.reset(i);
        }
        if (!vector_used && ((caller_savedv_.test(i)))) {
            caller_savedv_.reset(i);
        }
        if (i > (AVAILABLE_DOUBLE_WORD_REGISTERS << 1U)) {
            continue;
        }
        if (!scalar_used && ((callee_saved_.test(i + 1)))) {
            callee_saved_.reset(i + 1);
        }
    }

    callee_saved_.reset(vixl::aarch32::pc.GetCode());
    caller_saved_.reset(vixl::aarch32::pc.GetCode());
}

}  // namespace panda::compiler::aarch32
