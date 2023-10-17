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

#include "code_info.h"
#include "utils/bit_memory_region-inl.h"

namespace panda::compiler {

void CodeInfo::Dump(std::ostream &stream) const
{
    stream << "CodeInfo: vregs_num=" << GetHeader().GetVRegsCount() << ", frame_size=" << GetHeader().GetFrameSize()
           << std::endl;
    EnumerateTables([this, &stream](size_t index, auto member) {
        if (HasTable(index)) {
            const auto &table = this->*member;
            table.Dump(stream);
        }
    });
}

void CodeInfo::Dump(std::ostream &stream, const StackMap &stack_map, Arch arch) const
{
    stream << "Stackmap #" << stack_map.GetRow() << ": npc=0x" << std::hex << stack_map.GetNativePcUnpacked(arch)
           << ", bpc=0x" << std::hex << stack_map.GetBytecodePc();
    if (stack_map.HasInlineInfoIndex()) {
        stream << ", inline_depth=" << GetInlineDepth(stack_map) + 1;
    }
    if (stack_map.HasRootsRegMaskIndex() || stack_map.HasRootsStackMaskIndex()) {
        stream << ", roots=[";
        const char *sep = "";
        if (stack_map.HasRootsRegMaskIndex()) {
            stream << "r:0x" << std::hex << GetRootsRegMask(stack_map);
            sep = ",";
        }
        if (stack_map.HasRootsStackMaskIndex()) {
            auto region = GetRootsStackMask(stack_map);
            stream << sep << "s:" << region;
        }
        stream << "]";
    }
    if (stack_map.HasVRegMaskIndex()) {
        stream << ", vregs=" << GetVRegMask(stack_map);
    }
}

void CodeInfo::DumpInlineInfo(std::ostream &stream, const StackMap &stack_map, int depth) const
{
    auto ii = GetInlineInfo(stack_map, depth);
    stream << "InlineInfo #" << depth << ": bpc=0x" << std::hex << ii.GetBytecodePc() << std::dec
           << ", vregs_num: " << ii.GetVRegsCount();
}

}  // namespace panda::compiler
