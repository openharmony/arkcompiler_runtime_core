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

#ifndef PANDA_CODE_INFO_H
#define PANDA_CODE_INFO_H

#include "code_info_tables.h"
#include "utils/bit_field.h"
#include "utils/bit_table.h"
#include "utils/cframe_layout.h"
#include "utils/small_vector.h"
#include "utils/span.h"

namespace panda::compiler {

/*
 *
 * Compiled code layout:
 * +-------------+
 * | CodePrefix  |
 * +-------------+ <- Method::CompiledCodeEntrypoint
 * | Code        |
 * +-------------+-----------------+
 * | CodeInfo    | CodeInfoHeader  |
 * |             |-----------------+----------------------+
 * |             |                 |  StackMap            |
 * |             |                 |  InlineInfo          |
 * |             |                 |  Roots Reg Mask      |
 * |             |                 |  Roots Stack Mask    |
 * |             |   Bit Tables    |  Method indexes      |
 * |             |                 |  VRegs mask          |
 * |             |                 |  VRegs map           |
 * |             |                 |  VRegs catalogue     |
 * |             |                 |  Implicit Nullchecks |
 * |             |                 |  Constants           |
 * |-------------+-----------------+----------------------+
 */

struct CodePrefix {
    static constexpr uint32_t MAGIC = 0xaccadeca;
    uint32_t magic {MAGIC};
    uint32_t code_size {};
    uint32_t code_info_offset {};
    uint32_t code_info_size {};

    static constexpr size_t STRUCT_SIZE = 16;
};

static_assert(sizeof(CodePrefix) == CodePrefix::STRUCT_SIZE);

class CodeInfoHeader {
public:
    enum Elements { PROPERTIES, CALLEE_REG_MASK, CALLEE_FP_REG_MASK, TABLE_MASK, VREGS_COUNT, SIZE };

    void SetFrameSize(uint32_t size)
    {
        ASSERT(MinimumBitsToStore(size) <= FRAME_SIZE_FIELD_WIDTH);
        FieldFrameSize::Set(size, &data_[PROPERTIES]);
    }
    uint32_t GetFrameSize() const
    {
        return FieldFrameSize::Get(data_[PROPERTIES]);
    }

    void SetCalleeRegMask(uint32_t value)
    {
        data_[CALLEE_REG_MASK] = value;
    }
    uint32_t GetCalleeRegMask() const
    {
        return data_[CALLEE_REG_MASK];
    }

    void SetCalleeFpRegMask(uint32_t value)
    {
        data_[CALLEE_FP_REG_MASK] = value;
    }
    uint32_t GetCalleeFpRegMask() const
    {
        return data_[CALLEE_FP_REG_MASK];
    }

    void SetTableMask(uint32_t value)
    {
        data_[TABLE_MASK] = value;
    }
    uint32_t GetTableMask() const
    {
        return data_[TABLE_MASK];
    }

    void SetVRegsCount(uint32_t value)
    {
        data_[VREGS_COUNT] = value;
    }
    uint32_t GetVRegsCount() const
    {
        return data_[VREGS_COUNT];
    }

    void SetHasFloatRegs(bool value)
    {
        HasFloatRegsFlag::Set(value, &data_[PROPERTIES]);
    }
    bool HasFloatRegs() const
    {
        return HasFloatRegsFlag::Get(data_[PROPERTIES]);
    }

    template <typename Container>
    void Encode(BitMemoryStreamOut<Container> &out)
    {
        VarintPack::Write(out, data_);
    }
    void Decode(BitMemoryStreamIn *in)
    {
        data_ = VarintPack::Read<SIZE>(in);
    }

private:
    std::array<uint32_t, SIZE> data_;

    static constexpr size_t FRAME_SIZE_FIELD_WIDTH = 16;
    using FieldFrameSize = BitField<uint32_t, 0, FRAME_SIZE_FIELD_WIDTH>;
    using HasFloatRegsFlag = FieldFrameSize::NextFlag;
};

class CodeInfo final {
public:
    static constexpr size_t TABLES_COUNT = 10;
    static constexpr size_t VREG_LIST_STATIC_SIZE = 16;
    static constexpr size_t ALIGNMENT = sizeof(uint64_t);
    static constexpr size_t SIZE_ALIGNMENT = sizeof(uint64_t);

    template <typename Allocator>
    using VRegList = SmallVector<VRegInfo, VREG_LIST_STATIC_SIZE, Allocator, true>;

    NO_COPY_SEMANTIC(CodeInfo);
    NO_MOVE_SEMANTIC(CodeInfo);

    CodeInfo() = default;

    CodeInfo(const void *data, size_t size)
        : CodeInfo(Span<const uint8_t>(reinterpret_cast<const uint8_t *>(data), size))
    {
    }

    explicit CodeInfo(Span<const uint8_t> code) : CodeInfo(code.data())
    {
        ASSERT(GetDataSize() <= code.size());
    }

    explicit CodeInfo(Span<uint8_t> code) : CodeInfo(code.data())
    {
        ASSERT(GetDataSize() <= code.size());
    }

    explicit CodeInfo(const void *code_entry)
    {
        ASSERT(code_entry != nullptr);
        auto prefix = reinterpret_cast<const CodePrefix *>(code_entry);
        ASSERT(prefix->magic == CodePrefix::MAGIC);
        data_ = Span(reinterpret_cast<const uint8_t *>(code_entry), prefix->code_info_offset + prefix->code_info_size);
        auto code_info = Span<const uint8_t>(&data_[prefix->code_info_offset], prefix->code_info_size);
        Decode(code_info);
    }

    virtual ~CodeInfo() = default;

    static const void *GetCodeOriginFromEntryPoint(const void *data)
    {
        return reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(data) -
                                              CodeInfo::GetCodeOffset(RUNTIME_ARCH));
    }

    static CodeInfo CreateFromCodeEntryPoint(const void *data)
    {
        ASSERT(data != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return CodeInfo(reinterpret_cast<const uint8_t *>(data) - CodeInfo::GetCodeOffset(RUNTIME_ARCH));
    }

    void Decode(Span<const uint8_t> code_info)
    {
        BitMemoryStreamIn stream(const_cast<uint8_t *>(code_info.data()), code_info.size() * BITS_PER_BYTE);
        header_.Decode(&stream);
        EnumerateTables([this, &stream](size_t index, auto member) {
            if (HasTable(index)) {
                (this->*member).Decode(&stream);
            }
        });
    }

    const CodeInfoHeader &GetHeader() const
    {
        return header_;
    }
    CodeInfoHeader &GetHeader()
    {
        return header_;
    }

    const CodePrefix *GetPrefix() const
    {
        return reinterpret_cast<const CodePrefix *>(data_.data());
    }

    uint32_t GetFrameSize() const
    {
        return GetHeader().GetFrameSize();
    }

    const uint8_t *GetData()
    {
        return data_.data();
    }

    size_t GetDataSize()
    {
        return data_.size();
    }

    const uint8_t *GetCode() const
    {
        return &data_[CodeInfo::GetCodeOffset(RUNTIME_ARCH)];
    }

    size_t GetCodeSize() const
    {
        return GetPrefix()->code_size;
    }

    Span<const uint8_t> GetCodeSpan() const
    {
        return {&data_[CodeInfo::GetCodeOffset(RUNTIME_ARCH)], GetCodeSize()};
    }

    size_t GetInfoSize() const
    {
        return GetPrefix()->code_info_size;
    }

    bool HasTable(size_t index) const
    {
        return (GetHeader().GetTableMask() & (1U << index)) != 0;
    }

    std::variant<void *, uint32_t> GetMethod(const StackMap &stackmap, int inline_depth)
    {
        ASSERT(inline_depth >= 0);
        auto inline_info = inline_infos_.GetRow(stackmap.GetInlineInfoIndex() + inline_depth);
        if (inline_info.HasMethodLow()) {
            if constexpr (ArchTraits<RUNTIME_ARCH>::IS_64_BITS) {
                uintptr_t val =
                    inline_info.GetMethodLow() | (static_cast<uint64_t>(inline_info.GetMethodHi()) << BITS_PER_UINT32);
                return reinterpret_cast<void *>(val);
            } else {
                return reinterpret_cast<void *>(inline_info.GetMethodLow());
            }
        }
        return method_ids_.GetRow(inline_info.GetMethodIdIndex()).GetId();
    }

    uint64_t GetConstant(const VRegInfo &vreg) const
    {
        ASSERT(vreg.GetLocation() == VRegInfo::Location::CONSTANT);
        uint64_t low = constant_table_.GetRow(vreg.GetConstantLowIndex()).GetValue();
        uint64_t hi = constant_table_.GetRow(vreg.GetConstantHiIndex()).GetValue();
        return low | (hi << BITS_PER_UINT32);
    }

    static size_t GetCodeOffset(Arch arch)
    {
        return RoundUp(CodePrefix::STRUCT_SIZE, GetCodeAlignment(arch));
    }

    uint32_t GetSavedCalleeRegsMask(bool is_fp) const
    {
        return is_fp ? GetHeader().GetCalleeFpRegMask() : GetHeader().GetCalleeRegMask();
    }

    auto GetVRegMask(const StackMap &stack_map)
    {
        return stack_map.HasVRegMaskIndex() ? vreg_masks_.GetBitMemoryRegion(stack_map.GetVRegMaskIndex())
                                            : BitMemoryRegion<const uint8_t>();
    }

    auto GetVRegMask(const StackMap &stack_map) const
    {
        return const_cast<CodeInfo *>(this)->GetVRegMask(stack_map);
    }

    size_t GetVRegCount(const StackMap &stack_map) const
    {
        return GetVRegMask(stack_map).Popcount();
    }

    uint32_t GetRootsRegMask(const StackMap &stack_map) const
    {
        return stack_map.HasRootsRegMaskIndex() ? roots_reg_masks_.GetRow(stack_map.GetRootsRegMaskIndex()).GetMask()
                                                : 0;
    }

    auto GetRootsStackMask(const StackMap &stack_map) const
    {
        return stack_map.HasRootsStackMaskIndex()
                   ? roots_stack_masks_.GetBitMemoryRegion(stack_map.GetRootsStackMaskIndex())
                   : BitMemoryRegion<const uint8_t>();
    }

    auto GetInlineInfos(const StackMap &stack_map)
    {
        if (!stack_map.HasInlineInfoIndex()) {
            return inline_infos_.GetRangeReversed(0, 0);
        }
        auto index = stack_map.GetInlineInfoIndex();
        uint32_t size = index;
        for (; inline_infos_.GetRow(size).GetIsLast() == 0; size++) {
        }

        return inline_infos_.GetRangeReversed(index, helpers::ToSigned(size) + 1);
    }

    auto GetInlineInfo(const StackMap &stack_map, int inline_depth) const
    {
        ASSERT(stack_map.HasInlineInfoIndex());
        CHECK_GE(GetInlineDepth(stack_map), inline_depth);
        return inline_infos_.GetRow(stack_map.GetInlineInfoIndex() + inline_depth);
    }

    int GetInlineDepth(const StackMap &stack_map) const
    {
        if (!stack_map.HasInlineInfoIndex()) {
            return -1;
        }
        int index = stack_map.GetInlineInfoIndex();
        int depth = index;
        for (; inline_infos_.GetRow(depth).GetIsLast() == 0; depth++) {
        }
        return depth - index;
    }

    StackMap FindStackMapForNativePc(uint32_t pc, Arch arch = RUNTIME_ARCH) const
    {
        auto it =
            std::lower_bound(stack_maps_.begin(), stack_maps_.end(), pc, [arch](const auto &a, uintptr_t counter) {
                return a.GetNativePcUnpacked(arch) < counter;
            });
        return (it == stack_maps_.end() || it->GetNativePcUnpacked(arch) != pc) ? stack_maps_.GetInvalidRow() : *it;
    }

    StackMap FindOsrStackMap(uint32_t pc) const
    {
        auto it = std::find_if(stack_maps_.begin(), stack_maps_.end(),
                               [pc](const auto &a) { return a.GetBytecodePc() == pc && a.IsOsr(); });
        return it == stack_maps_.end() ? stack_maps_.GetInvalidRow() : *it;
    }

    auto GetStackMap(size_t index) const
    {
        return StackMap(&stack_maps_, index);
    }

    auto &GetStackMaps()
    {
        return stack_maps_;
    }

    auto &GetVRegCatalogue()
    {
        return vregs_catalogue_;
    }

    auto &GetVRegMapTable()
    {
        return vregs_map_;
    }

    auto &GetVRegMaskTable()
    {
        return vreg_masks_;
    }

    auto &GetInlineInfosTable()
    {
        return inline_infos_;
    }

    auto &GetConstantTable()
    {
        return constant_table_;
    }

    const auto &GetImplicitNullChecksTable() const
    {
        return implicit_nullchecks_;
    }

    bool HasFloatRegs() const
    {
        return GetHeader().HasFloatRegs();
    }

    template <typename Func>
    static void EnumerateTables(Func func)
    {
        size_t index = 0;
        func(index++, &CodeInfo::stack_maps_);
        func(index++, &CodeInfo::inline_infos_);
        func(index++, &CodeInfo::roots_reg_masks_);
        func(index++, &CodeInfo::roots_stack_masks_);
        func(index++, &CodeInfo::method_ids_);
        func(index++, &CodeInfo::vreg_masks_);
        func(index++, &CodeInfo::vregs_map_);
        func(index++, &CodeInfo::vregs_catalogue_);
        func(index++, &CodeInfo::implicit_nullchecks_);
        func(index++, &CodeInfo::constant_table_);
        ASSERT(index == TABLES_COUNT);
    }

    template <typename Callback>
    void EnumerateStaticRoots(const StackMap &stack_map, Callback callback)
    {
        return EnumerateRoots<Callback, false>(stack_map, callback);
    }

    template <typename Callback>
    void EnumerateDynamicRoots(const StackMap &stack_map, Callback callback)
    {
        return EnumerateRoots<Callback, true>(stack_map, callback);
    }

    template <typename Allocator>
    VRegList<Allocator> GetVRegList(StackMap stack_map, uint32_t first_vreg, uint32_t vregs_count,
                                    Allocator *allocator = nullptr) const
    {
        if (vregs_count == 0 || !stack_map.HasRegMap()) {
            return CodeInfo::VRegList<Allocator>(allocator);
        }
        VRegList<Allocator> vreg_list(allocator);
        vreg_list.resize(vregs_count, VRegInfo());
        ASSERT(!vreg_list[0].IsLive());
        std::vector<bool> reg_set(vregs_count);

        uint32_t remaining_registers = vregs_count;
        for (int sindex = stack_map.GetRow(); sindex >= 0 && remaining_registers > 0; sindex--) {
            stack_map = GetStackMap(sindex);
            if (!stack_map.HasVRegMaskIndex()) {
                continue;
            }
            // Skip stackmaps that are not in the same inline depth
            auto vreg_mask = GetVRegMask(stack_map);
            if (vreg_mask.Size() <= first_vreg) {
                continue;
            }
            ASSERT(stack_map.HasVRegMapIndex());
            uint32_t map_index = stack_map.GetVRegMapIndex();

            map_index += vreg_mask.Popcount(0, first_vreg);
            vreg_mask = vreg_mask.Subregion(first_vreg, vreg_mask.Size() - first_vreg);

            uint32_t end = std::min<uint32_t>(vreg_mask.Size(), vregs_count);
            for (size_t i = 0; i < end; i += BITS_PER_UINT32) {
                uint32_t mask = vreg_mask.Read(i, std::min<uint32_t>(end - i, BITS_PER_UINT32));
                while (mask != 0) {
                    uint32_t reg_idx = Ctz(mask);
                    if (!reg_set[i + reg_idx]) {
                        auto vreg_index = vregs_map_.GetRow(map_index);
                        if (vreg_index.GetIndex() != StackMap::NO_VALUE) {
                            ASSERT(!vreg_list[i + reg_idx].IsLive());
                            vreg_list[i + reg_idx] = vregs_catalogue_.GetRow(vreg_index.GetIndex()).GetVRegInfo();
                            vreg_list[i + reg_idx].SetIndex(i + reg_idx);
                        }
                        remaining_registers--;
                        reg_set[i + reg_idx] = true;
                    }
                    map_index++;
                    mask ^= 1U << reg_idx;
                }
            }
        }
        return vreg_list;
    }

    template <typename Allocator>
    VRegList<Allocator> GetVRegList(StackMap stack_map, int inline_depth, Allocator *allocator = nullptr) const
    {
        if (inline_depth < 0) {
            return GetVRegList<Allocator>(stack_map, 0, GetHeader().GetVRegsCount(), allocator);
        }
        ASSERT(stack_map.HasInlineInfoIndex());
        auto inline_info = GetInlineInfo(stack_map, inline_depth);
        if (inline_info.GetVRegsCount() == 0) {
            return VRegList<Allocator>(allocator);
        }
        auto depth = inline_info.GetRow() - stack_map.GetInlineInfoIndex();
        uint32_t first =
            depth == 0 ? GetHeader().GetVRegsCount() : inline_infos_.GetRow(inline_info.GetRow() - 1).GetVRegsCount();
        ASSERT(inline_info.GetVRegsCount() >= first);
        return GetVRegList<Allocator>(stack_map, first, inline_info.GetVRegsCount() - first, allocator);
    }

    template <typename Allocator>
    VRegList<Allocator> GetVRegList(StackMap stack_map, Allocator *allocator = nullptr) const
    {
        return GetVRegList<Allocator>(stack_map, -1, allocator);
    }

    static bool VerifyCompiledEntry(uintptr_t compiled_entry)
    {
        auto codeheader = compiled_entry - GetCodeOffset(RUNTIME_ARCH);
        return (*reinterpret_cast<const uint32_t *>(codeheader) == CodePrefix::MAGIC);
    }

    void Dump(std::ostream &stream) const;

    void Dump(std::ostream &stream, const StackMap &stack_map, Arch arch = RUNTIME_ARCH) const;

    void DumpInlineInfo(std::ostream &stream, const StackMap &stack_map, int depth) const;

    size_t CountSpillSlots()
    {
        auto frame_slots = GetFrameSize() / PointerSize(RUNTIME_ARCH);
        auto spills_count = frame_slots - (static_cast<size_t>(CFrameSlots::Start()) + GetRegsCount(RUNTIME_ARCH) + 1U);
        // Reverse 'CFrameLayout::AlignSpillCount' counting
        if (RUNTIME_ARCH == Arch::AARCH32) {
            spills_count = spills_count / 2U - 1;
        }
        if (spills_count % 2U != 0) {
            spills_count--;
        }
        return spills_count;
    }

private:
    template <typename Callback, bool is_dynamic>
    void EnumerateRoots(const StackMap &stack_map, Callback callback);

    BitTable<StackMap> stack_maps_;
    BitTable<InlineInfo> inline_infos_;
    BitTable<RegisterMask> roots_reg_masks_;
    BitTable<StackMask> roots_stack_masks_;
    BitTable<MethodId> method_ids_;
    BitTable<VRegisterInfo> vregs_catalogue_;
    BitTable<VRegisterCatalogueIndex> vregs_map_;
    BitTable<VRegisterMask> vreg_masks_;
    BitTable<ImplicitNullChecks> implicit_nullchecks_;
    BitTable<ConstantTable> constant_table_;

    CodeInfoHeader header_ {};

    Span<const uint8_t> data_;
};

template <typename Callback, bool is_dynamic>
void CodeInfo::EnumerateRoots(const StackMap &stack_map, Callback callback)
{
    auto root_type = is_dynamic ? VRegInfo::Type::ANY : VRegInfo::Type::OBJECT;

    if (stack_map.HasRootsRegMaskIndex()) {
        auto reg_mask = roots_reg_masks_.GetRow(stack_map.GetRootsRegMaskIndex()).GetMask();
        ArenaBitVectorSpan vec(&reg_mask, BITS_PER_UINT32);
        for (auto reg_idx : vec.GetSetBitsIndices()) {
            if (!callback(VRegInfo(reg_idx, VRegInfo::Location::REGISTER, root_type, false))) {
                return;
            }
        }
    }
    // Simplify after renumbering stack slots
    if (stack_map.HasRootsStackMaskIndex()) {
        auto stack_slots_count = CountSpillSlots();
        auto reg_mask = roots_stack_masks_.GetBitMemoryRegion(stack_map.GetRootsStackMaskIndex());
        for (auto reg_idx : reg_mask) {
            if (reg_idx >= stack_slots_count) {
                // Parameter-slots' indexes are added to the root-mask with `stack_slots_count` offset to distinct them
                // from spill-slots
                auto param_slot_idx = reg_idx - stack_slots_count;
                reg_idx = static_cast<size_t>(CFrameLayout::StackArgSlot::Start()) - param_slot_idx -
                          static_cast<size_t>(CFrameSlots::Start());
            } else {
                if constexpr (!ArchTraits<RUNTIME_ARCH>::IS_64_BITS) {  // NOLINT
                    reg_idx = (reg_idx << 1U) + 1;
                }
                // Stack roots are began from spill/fill stack origin, so we need to adjust it according to registers
                // buffer
                reg_idx += GetRegsCount(RUNTIME_ARCH);
            }
            VRegInfo vreg(reg_idx, VRegInfo::Location::SLOT, root_type, false);
            if (!callback(vreg)) {
                return;
            }
        }
    }
}

}  // namespace panda::compiler

#endif  // PANDA_CODE_INFO_H
