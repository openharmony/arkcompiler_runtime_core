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

#include "code_info_builder.h"
#include "utils/bit_memory_region-inl.h"

namespace panda::compiler {

void CodeInfoBuilder::BeginMethod(uint32_t frame_size, uint32_t vregs_count)
{
#ifndef NDEBUG
    ASSERT(!was_method_begin_);
    ASSERT(!was_stack_map_begin_);
    ASSERT(!was_inline_info_begin_);
    was_method_begin_ = true;
#endif

    header_.SetFrameSize(frame_size);
    vregs_count_ = vregs_count;
    constant_table_.Add({0});
}

void CodeInfoBuilder::EndMethod()
{
#ifndef NDEBUG
    ASSERT(was_method_begin_);
    ASSERT(!was_stack_map_begin_);
    ASSERT(!was_inline_info_begin_);
    was_method_begin_ = false;
#endif
}

void CodeInfoBuilder::BeginStackMap(uint32_t bpc, uint32_t npc, ArenaBitVector *stack_roots, uint32_t regs_roots,
                                    bool require_vreg_map, bool is_osr)
{
#ifndef NDEBUG
    ASSERT(was_method_begin_);
    ASSERT(!was_stack_map_begin_);
    ASSERT(!was_inline_info_begin_);
    was_stack_map_begin_ = true;
#endif
    inline_info_stack_.clear();
    current_vregs_.clear();

    ASSERT(stack_maps_.GetSize() == 0 || npc >= stack_maps_.GetLast()[StackMap::COLUMN_NATIVE_PC]);

    current_vregs_count_ = require_vreg_map ? vregs_count_ : 0;

    current_stack_map_ = BitTableBuilder<StackMap>::Entry();
    current_stack_map_[StackMap::COLUMN_PROPERTIES] = StackMap::CreateProperties(is_osr, require_vreg_map);
    current_stack_map_[StackMap::COLUMN_BYTECODE_PC] = bpc;
    current_stack_map_[StackMap::COLUMN_NATIVE_PC] = StackMap::PackAddress(npc, arch_);
    if (regs_roots != 0) {
        current_stack_map_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX] = roots_reg_masks_.Add({regs_roots});
    }
    if (stack_roots != nullptr && !stack_roots->empty()) {
        current_stack_map_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX] = roots_stack_masks_.Add(stack_roots->GetFixed());
    }
    // Ensure that stackmaps are inserted in sorted order
    if (stack_maps_.GetRowsCount() != 0) {
        ASSERT(current_stack_map_[StackMap::COLUMN_NATIVE_PC] >= stack_maps_.GetLast()[StackMap::COLUMN_NATIVE_PC]);
    }
}

void CodeInfoBuilder::EndStackMap()
{
#ifndef NDEBUG
    ASSERT(was_method_begin_);
    ASSERT(was_stack_map_begin_);
    ASSERT(!was_inline_info_begin_);
    was_stack_map_begin_ = false;
#endif
    if (!inline_info_stack_.empty()) {
        inline_info_stack_.back()[InlineInfo::COLUMN_IS_LAST] = static_cast<uint32_t>(true);
        current_stack_map_[StackMap::COLUMN_INLINE_INFO_INDEX] = inline_infos_.AddArray(Span(inline_info_stack_));
    }

    EmitVRegs();

    stack_maps_.Add(current_stack_map_);
}

void CodeInfoBuilder::DumpCurrentStackMap(std::ostream &stream) const
{
    stream << "Stackmap #" << stack_maps_.GetRowsCount() - 1 << ": npc=0x" << std::hex
           << StackMap::UnpackAddress(current_stack_map_[StackMap::COLUMN_NATIVE_PC], arch_) << ", bpc=0x" << std::hex
           << current_stack_map_[StackMap::COLUMN_BYTECODE_PC];
    if (current_stack_map_[StackMap::COLUMN_INLINE_INFO_INDEX] != StackMap::NO_VALUE) {
        stream << ", inline_depth=" << inline_info_stack_.size();
    }
    if (current_stack_map_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX] != StackMap::NO_VALUE ||
        current_stack_map_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX] != StackMap::NO_VALUE) {
        stream << ", roots=[";
        const char *sep = "";
        if (current_stack_map_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX] != StackMap::NO_VALUE) {
            auto &entry = roots_reg_masks_.GetEntry(current_stack_map_[StackMap::COLUMN_ROOTS_REG_MASK_INDEX]);
            stream << "r:0x" << std::hex << entry[RegisterMask::COLUMN_MASK];
            sep = ",";
        }
        if (current_stack_map_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX] != StackMap::NO_VALUE) {
            auto region = roots_stack_masks_.GetEntry(current_stack_map_[StackMap::COLUMN_ROOTS_STACK_MASK_INDEX]);
            stream << sep << "s:" << region;
        }
        stream << "]";
    }
    if (current_stack_map_[StackMap::COLUMN_VREG_MASK_INDEX] != StackMap::NO_VALUE) {
        stream << ", vregs=" << vreg_masks_.GetEntry(current_stack_map_[StackMap::COLUMN_VREG_MASK_INDEX]);
    }
}

void CodeInfoBuilder::BeginInlineInfo(void *method, uint32_t method_id, uint32_t bpc, uint32_t vregs_count)
{
#ifndef NDEBUG
    ASSERT(was_method_begin_);
    ASSERT(was_stack_map_begin_);
    was_inline_info_begin_ = true;
#endif
    BitTableBuilder<InlineInfo>::Entry inline_info;
    current_vregs_count_ += vregs_count;

    inline_info[InlineInfo::COLUMN_IS_LAST] = static_cast<uint32_t>(false);
    inline_info[InlineInfo::COLUMN_BYTECODE_PC] = bpc;
    inline_info[InlineInfo::COLUMN_VREGS_COUNT] = current_vregs_count_;
    if (method != nullptr) {
        inline_info[InlineInfo::COLUMN_METHOD_HI] = High32Bits(method);
        inline_info[InlineInfo::COLUMN_METHOD_LOW] = Low32Bits(method);
    } else {
        ASSERT(method_id != 0);
        inline_info[InlineInfo::COLUMN_METHOD_ID_INDEX] = method_ids_.Add({method_id});
    }

    inline_info_stack_.push_back(inline_info);
}

void CodeInfoBuilder::EndInlineInfo()
{
#ifndef NDEBUG
    ASSERT(was_method_begin_);
    ASSERT(was_stack_map_begin_);
    ASSERT(was_inline_info_begin_);
    was_inline_info_begin_ = false;
#endif
    ASSERT(current_vregs_.size() == current_vregs_count_);
}

void CodeInfoBuilder::AddConstant(uint64_t value, VRegInfo::Type type, bool is_acc)
{
    VRegInfo vreg(0, VRegInfo::Location::CONSTANT, type, is_acc);
    uint32_t low = value & ((1LLU << BITS_PER_UINT32) - 1);
    uint32_t hi = (value >> BITS_PER_UINT32) & ((1LLU << BITS_PER_UINT32) - 1);
    vreg.SetConstantIndices(constant_table_.Add({low}), constant_table_.Add({hi}));
    current_vregs_.push_back(vreg);
}

void CodeInfoBuilder::EmitVRegs()
{
    ASSERT(current_vregs_.size() == current_vregs_count_);
    if (current_vregs_.empty()) {
        return;
    }

    if (current_vregs_.size() > last_vregs_.size()) {
        last_vregs_.resize(current_vregs_.size(), VRegInfo::Invalid());
        vregs_last_change_.resize(current_vregs_.size());
    }

    ArenaVector<BitTableBuilder<VRegisterCatalogueIndex>::Entry> &vregs_map = vregs_map_storage_;
    ArenaBitVector &vregs_mask = vregs_mask_storage_;
    vregs_map.clear();
    vregs_mask.clear();

    for (size_t i = 0; i < current_vregs_.size(); i++) {
        auto &vreg = current_vregs_[i];
        uint32_t distatnce = stack_maps_.GetRowsCount() - vregs_last_change_[i];
        if (last_vregs_[i] != vreg || distatnce > MAX_VREG_LIVE_DISTANCE) {
            BitTableBuilder<VRegisterInfo>::Entry vreg_entry;
            vreg_entry[VRegisterInfo::COLUMN_INFO] = vreg.GetInfo();
            vreg_entry[VRegisterInfo::COLUMN_VALUE] = vreg.GetValue();
            uint32_t index = vreg.IsLive() ? vregs_catalogue_.Add(vreg_entry) : decltype(vregs_catalogue_)::NO_VALUE;
            vregs_map.push_back({index});
            vregs_mask.SetBit(i);
            last_vregs_[i] = vreg;
            vregs_last_change_[i] = stack_maps_.GetRowsCount();
        }
    }

    BitMemoryRegion rgn(vregs_mask.data(), vregs_mask.size());
    ASSERT(vregs_mask.PopCount() == vregs_map.size());
    if (vregs_mask.PopCount() != 0) {
        current_stack_map_[StackMap::COLUMN_VREG_MASK_INDEX] = vreg_masks_.Add(vregs_mask.GetFixed());
    }
    if (!current_vregs_.empty()) {
        current_stack_map_[StackMap::COLUMN_VREG_MAP_INDEX] = vregs_map_.AddArray(Span(vregs_map));
    }
}

void CodeInfoBuilder::Encode(ArenaVector<uint8_t> *stream, size_t offset)
{
    BitMemoryStreamOut out(stream, offset);

    uint32_t tables_mask = 0;
    EnumerateTables([&tables_mask](size_t index, const auto &table) {
        if (table->GetRowsCount() != 0) {
            tables_mask |= (1U << index);
        }
    });

    header_.SetTableMask(tables_mask);
    header_.SetVRegsCount(vregs_count_);
    header_.Encode(out);

    EnumerateTables([&out]([[maybe_unused]] size_t index, const auto &table) {
        if (table->GetRowsCount() != 0) {
            table->Encode(out);
        }
    });
    stream->resize(RoundUp(stream->size(), CodeInfo::SIZE_ALIGNMENT));
}

}  // namespace panda::compiler
