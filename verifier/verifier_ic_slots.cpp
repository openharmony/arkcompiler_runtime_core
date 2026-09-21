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
#include "verifier_internal.h"

#include <algorithm>

namespace panda::verifier {

std::optional<uint64_t> Verifier::GetSlotNumberFromAnnotation(panda_file::MethodDataAccessor &method_accessor)
{
    return FindAnnotationElementValue(method_accessor, "L_ESSlotNumberAnnotation;", "SlotNumber");
}

bool Verifier::CollectIcSlotInsn(const BytecodeInstruction &bc_ins, std::vector<IcSlotInsnInfo> &ic_insns)
{
    if (!bc_ins.HasFlag(BytecodeInstruction::Flags::ONE_SLOT) &&
        !bc_ins.HasFlag(BytecodeInstruction::Flags::TWO_SLOT)) {
        return true;
    }
    // GetImmData zero-extends; GetFirstImmFromInstruction sign-extends u8 slots.
    size_t index = 0;
    const auto format = bc_ins.GetFormat();
    if (!bc_ins.HasImm(format, index)) {
        LOG(ERROR, VERIFIER) << "Fail to get the ic slot operand!";
        return false;
    }
    IcSlotInsnInfo insn_info;
    insn_info.is_two_slot = bc_ins.HasFlag(BytecodeInstruction::Flags::TWO_SLOT);
    insn_info.is_8bit_only = bc_ins.HasFlag(BytecodeInstruction::Flags::EIGHT_BIT_IC);
    insn_info.slot = static_cast<uint32_t>(bc_ins.GetImmData(index));
    ic_insns.push_back(insn_info);
    return true;
}

bool Verifier::IsIcCheckEnabled() const
{
    return panda_file::IsVersionLessOrEqual(IC_SLOT_CHECK_VERSION, file_->GetHeader()->version);
}

uint32_t Verifier::GetRuntimeSlotCount(uint32_t ann_slot_num)
{
    if (ann_slot_num >= MAX_SLOT_SIZE) {
        return MAX_SLOT_SIZE + EXTEND_SLOT_SIZE;
    }
    if (ann_slot_num >= INVALID_IC_SLOT - 1U && ann_slot_num <= INVALID_IC_SLOT) {
        return INVALID_IC_SLOT + 1U;
    }
    return ann_slot_num;
}

bool Verifier::VerifyIcSlotAllocation(const std::vector<IcSlotInsnInfo> &ic_insns, const IcSlotState &ic_state,
                                      const panda_file::File::EntityId &method_id)
{
    auto matches_assignment = [&ic_insns](const std::vector<uint32_t> &expected) {
        return ic_insns.size() == expected.size() &&
               std::equal(ic_insns.begin(), ic_insns.end(), expected.begin(),
                          [](const IcSlotInsnInfo &insn, uint32_t slot) { return insn.slot == slot; });
    };
    IcSlotAssignment assignment(ic_insns);
    std::vector<uint32_t> expected;
    assignment.RunInitial(expected);
    if (matches_assignment(expected) && assignment.GetTotal() <= ic_state.ann_slot_num) {
        return true;
    }
    if (std::find(expected.begin(), expected.end(), INVALID_IC_SLOT) != expected.end()) {
        assignment.RunRearranged(expected);
        if (matches_assignment(expected) && assignment.GetTotal() <= ic_state.ann_slot_num) {
            return true;
        }
    }
    return VerifyIcSlotRanges(ic_insns, ic_state.ann_slot_num, method_id);
}

bool Verifier::VerifyIcSlotRanges(const std::vector<IcSlotInsnInfo> &ic_insns, uint32_t ann_slot_num,
                                  const panda_file::File::EntityId &method_id)
{
    const uint32_t slot_count = GetRuntimeSlotCount(ann_slot_num);
    std::vector<std::pair<uint32_t, uint32_t>> slot_ranges;
    for (const auto &insn : ic_insns) {
        uint32_t ret = insn.is_two_slot ? 2U : 1U;
        if (insn.slot == INVALID_IC_SLOT) {
            if (slot_count <= INVALID_IC_SLOT) {
                LOG(ERROR, VERIFIER) << "Invalid ic slot index " << insn.slot << ", the slot count of the method is "
                                     << ann_slot_num << " in method 0x" << std::hex << method_id.GetOffset();
                return false;
            }
            continue;
        }
        if (static_cast<uint64_t>(insn.slot) + ret > slot_count) {
            LOG(ERROR, VERIFIER) << "Ic slot index " << insn.slot << " is out of bounds, max allowed: " << slot_count
                                 << " in method 0x" << std::hex << method_id.GetOffset();
            return false;
        }
        slot_ranges.emplace_back(insn.slot, insn.slot + ret);
    }
    std::sort(slot_ranges.begin(), slot_ranges.end());
    for (size_t i = 1; i < slot_ranges.size(); i++) {
        if (slot_ranges[i].first < slot_ranges[i - 1U].second) {
            LOG(ERROR, VERIFIER) << "Ic slot index " << slot_ranges[i].first << " overlaps the slot range of another "
                                 << "ic instruction in method 0x" << std::hex << method_id.GetOffset();
            return false;
        }
    }
    return true;
}

void Verifier::PrepareIcSlotCheck(panda_file::MethodDataAccessor &method_accessor, IcSlotState &ic_state)
{
    ic_state.ann_slot_num = 0;
    ic_state.ic_check_enabled = IsIcCheckEnabled();
    if (!ic_state.ic_check_enabled) {
        return;
    }
    const auto ann_slot_number = GetSlotNumberFromAnnotation(method_accessor);
    if (ann_slot_number.has_value()) {
        ic_state.ann_slot_num = static_cast<uint32_t>(ann_slot_number.value());
    } else {
        LOG(WARNING, VERIFIER) << "The slot number annotation is missing, the slot count is treated as zero!";
    }
}

}  // namespace panda::verifier
