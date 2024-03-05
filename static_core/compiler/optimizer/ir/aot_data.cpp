/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::compiler {
intptr_t AotData::GetEpTableOffset() const
{
    return -(static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) * PointerSize(graph_->GetArch()));
}

intptr_t AotData::GetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc) const
{
    auto offset = slowPathData_->GetSharedSlowPathOffset(id);
    if (offset == 0) {
        return 0;
    }
    return offset - (codeAddress_ + pc + CodeInfo::GetCodeOffset(graph_->GetArch()));
}

void AotData::SetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc)
{
    slowPathData_->SetSharedSlowPathOffset(id, codeAddress_ + pc + CodeInfo::GetCodeOffset(graph_->GetArch()));
}

intptr_t AotData::GetEntrypointOffset(uint64_t pc, int32_t slotId) const
{
    // Initialize offset by offset to the origin of the entrypoint table
    intptr_t offset = GetEpTableOffset();
    // Increment/decrement offset to specified slot
    offset += slotId * static_cast<intptr_t>(PointerSize(graph_->GetArch()));
    // Decrement by sum of method code start address and current pc
    offset -= static_cast<intptr_t>(codeAddress_ + pc);
    // Decrement by header size that prepend method code
    offset -= static_cast<intptr_t>(CodeInfo::GetCodeOffset(graph_->GetArch()));
    return offset;
}

intptr_t AotData::GetPltSlotOffset(uint64_t pc, uint32_t methodId)
{
    int32_t slotId = GetPltSlotId(methodId);
    return GetEntrypointOffset(pc, slotId);
}

int32_t AotData::GetPltSlotId(uint32_t methodId)
{
    int32_t slotId;
    auto slot = gotPlt_->find({pfile_, methodId});
    if (slot != gotPlt_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotPlt_->insert({{pfile_, methodId}, slotId});
    }
    return slotId;
}

intptr_t AotData::GetVirtIndexSlotOffset(uint64_t pc, uint32_t methodId)
{
    int32_t slotId;
    auto slot = gotVirtIndexes_->find({pfile_, methodId});
    if (slot != gotVirtIndexes_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotVirtIndexes_->insert({{pfile_, methodId}, slotId});
    }
    return GetEntrypointOffset(pc, slotId);
}

intptr_t AotData::GetClassSlotOffset(uint64_t pc, uint32_t klassId, bool init)
{
    int32_t slotId = GetClassSlotId(klassId);
    return GetEntrypointOffset(pc, init ? slotId - 1 : slotId);
}

intptr_t AotData::GetStringSlotOffset(uint64_t pc, uint32_t stringId)
{
    int32_t slotId = GetStringSlotId(stringId);
    return GetEntrypointOffset(pc, slotId);
}

int32_t AotData::GetClassSlotId(uint32_t klassId)
{
    int32_t slotId;
    auto slot = gotClass_->find({pfile_, klassId});
    if (slot != gotClass_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotClass_->insert({{pfile_, klassId}, slotId});
    }
    return slotId;
}

int32_t AotData::GetStringSlotId(uint32_t stringId)
{
    int32_t slotId;
    auto slot = gotString_->find({pfile_, stringId});
    if (slot != gotString_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotString_->insert({{pfile_, stringId}, slotId});
    }
    return slotId;
}

intptr_t AotData::GetCommonSlotOffset(uint64_t pc, uint32_t id)
{
    int32_t slotId;
    auto slot = gotCommon_->find({pfile_, id});
    if (slot != gotCommon_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotCommon_->insert({{pfile_, id}, slotId});
    }
    return GetEntrypointOffset(pc, slotId);
}

uint64_t AotData::GetInfInlineCacheSlotOffset(uint64_t pc, uint64_t cacheIdx)
{
    int32_t slotId = GetIntfInlineCacheSlotId(cacheIdx);
    return GetEntrypointOffset(pc, slotId);
}

int32_t AotData::GetIntfInlineCacheSlotId(uint64_t cacheIdx)
{
    int32_t slotId;
    auto slot = gotIntfInlineCache_->find({pfile_, cacheIdx});
    if (slot != gotIntfInlineCache_->end()) {
        slotId = slot->second;
    } else {
        slotId = GetSlotId();
        gotIntfInlineCache_->insert({{pfile_, cacheIdx}, slotId});
    }
    return slotId;
}

int32_t AotData::GetSlotId() const
{
    constexpr auto IMM_3 = 3;
    constexpr auto IMM_2 = 2;
    return -1 - IMM_3 * (gotPlt_->size() + gotClass_->size()) - IMM_2 * (gotVirtIndexes_->size() + gotString_->size()) -
           gotIntfInlineCache_->size() - gotCommon_->size();
}
}  // namespace ark::compiler
