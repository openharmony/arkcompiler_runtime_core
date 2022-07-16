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

#include "graph.h"
#include "code_info/code_info.h"

namespace panda::compiler {
intptr_t AotData::GetEpTableOffset() const
{
    return -(static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) * PointerSize(graph_->GetArch()));
}

intptr_t AotData::GetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc) const
{
    auto offset = slow_path_data_->GetSharedSlowPathOffset(id);
    if (offset == 0) {
        return 0;
    }
    return offset - (code_address_ + pc + CodeInfo::GetCodeOffset(graph_->GetArch()));
}

void AotData::SetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc)
{
    slow_path_data_->SetSharedSlowPathOffset(id, code_address_ + pc + CodeInfo::GetCodeOffset(graph_->GetArch()));
}

intptr_t AotData::GetEntrypointOffset(uint64_t pc, int32_t slot_id) const
{
    // Initialize offset by offset to the origin of the entrypoint table
    intptr_t offset = GetEpTableOffset();
    // Increment/decrement offset to specified slot
    offset += slot_id * PointerSize(graph_->GetArch());
    // Decrement by sum of method code start address and current pc
    offset -= (code_address_ + pc);
    // Decrement by header size that prepend method code
    offset -= CodeInfo::GetCodeOffset(graph_->GetArch());
    return offset;
}

intptr_t AotData::GetPltSlotOffset(uint64_t pc, uint32_t method_id)
{
    int32_t slot_id;
    auto slot = got_plt_->find({pfile_, method_id});
    if (slot != got_plt_->end()) {
        slot_id = slot->second;
    } else {
        slot_id = GetSlotId();
        got_plt_->insert({{pfile_, method_id}, slot_id});
    }
    return GetEntrypointOffset(pc, slot_id);
}

intptr_t AotData::GetVirtIndexSlotOffset(uint64_t pc, uint32_t method_id)
{
    int32_t slot_id;
    auto slot = got_virt_indexes_->find({pfile_, method_id});
    if (slot != got_virt_indexes_->end()) {
        slot_id = slot->second;
    } else {
        slot_id = GetSlotId();
        got_virt_indexes_->insert({{pfile_, method_id}, slot_id});
    }
    return GetEntrypointOffset(pc, slot_id);
}

intptr_t AotData::GetClassSlotOffset(uint64_t pc, uint32_t klass_id, bool init)
{
    int32_t slot_id;
    auto slot = got_class_->find({pfile_, klass_id});
    if (slot != got_class_->end()) {
        slot_id = slot->second;
    } else {
        slot_id = GetSlotId();
        got_class_->insert({{pfile_, klass_id}, slot_id});
    }
    return GetEntrypointOffset(pc, init ? slot_id - 1 : slot_id);
}

intptr_t AotData::GetStringSlotOffset(uint64_t pc, uint32_t string_id)
{
    int32_t slot_id;
    auto slot = got_string_->find({pfile_, string_id});
    if (slot != got_string_->end()) {
        slot_id = slot->second;
    } else {
        slot_id = GetSlotId();
        got_string_->insert({{pfile_, string_id}, slot_id});
    }
    return GetEntrypointOffset(pc, slot_id);
}

uint64_t AotData::GetInfInlineCacheSlotOffset(uint64_t pc, uint64_t index)
{
    uint32_t slot_id;
    auto slot = got_intf_inline_cache_->find({pfile_, index});
    if (slot != got_intf_inline_cache_->end()) {
        slot_id = slot->second;
    } else {
        slot_id = GetSlotId();
        got_intf_inline_cache_->insert({{pfile_, index}, slot_id});
    }
    return GetEntrypointOffset(pc, slot_id);
}

int32_t AotData::GetSlotId() const
{
    constexpr auto IMM_3 = 3;
    constexpr auto IMM_2 = 2;
    return -1 - IMM_3 * (got_plt_->size() + got_class_->size()) -
           IMM_2 * (got_virt_indexes_->size() + got_string_->size()) - got_intf_inline_cache_->size();
}
}  // namespace panda::compiler
