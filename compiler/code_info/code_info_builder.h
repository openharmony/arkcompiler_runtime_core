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

#ifndef PANDA_CODE_INFO_BUILDER_H
#define PANDA_CODE_INFO_BUILDER_H

#include "code_info.h"
#include "utils/arena_containers.h"
#include "utils/bit_vector.h"

namespace panda::compiler {

class CodeInfoBuilder {
public:
    CodeInfoBuilder(Arch arch, ArenaAllocator *allocator)
        : arch_(arch),
          stack_maps_(allocator),
          inline_infos_(allocator),
          roots_reg_masks_(allocator),
          roots_stack_masks_(allocator),
          method_ids_(allocator),
          vregs_catalogue_(allocator),
          vregs_map_(allocator),
          vreg_masks_(allocator),
          implicit_nullchecks_(allocator),
          constant_table_(allocator),
          current_vregs_(allocator->Adapter()),
          last_vregs_(allocator->Adapter()),
          vregs_last_change_(allocator->Adapter()),
          inline_info_stack_(allocator->Adapter()),
          vregs_map_storage_(allocator->Adapter()),
          vregs_mask_storage_(allocator)
    {
    }

    NO_COPY_SEMANTIC(CodeInfoBuilder);
    NO_MOVE_SEMANTIC(CodeInfoBuilder);
    ~CodeInfoBuilder() = default;

    void BeginMethod(uint32_t frame_size, uint32_t vregs_count);

    void EndMethod();

    void BeginStackMap(uint32_t bpc, uint32_t npc, ArenaBitVector *stack_roots, uint32_t regs_roots,
                       bool require_vreg_map, bool is_osr);

    void EndStackMap();

    void BeginInlineInfo(void *method, uint32_t method_id, uint32_t bpc, uint32_t vregs_count);

    void EndInlineInfo();

    void AddVReg(VRegInfo reg)
    {
        // Constant should be added via `AddConstant` method
        ASSERT(reg.GetLocation() != VRegInfo::Location::CONSTANT);
        current_vregs_.push_back(reg);
    }

    void AddConstant(uint64_t value, VRegInfo::Type type, bool is_acc);

    void SetFrameSize(uint32_t size)
    {
        header_.SetFrameSize(size);
    }

    void Encode(ArenaVector<uint8_t> *stream, size_t offset = 0);

    void SetSavedCalleeRegsMask(uint32_t mask, uint32_t vmask)
    {
        header_.SetCalleeRegMask(mask);
        header_.SetCalleeFpRegMask(vmask);
    }

    void AddImplicitNullCheck(uint32_t instruction_native_pc, uint32_t offset)
    {
        implicit_nullchecks_.Add({instruction_native_pc, offset});
    }

    void SetHasFloatRegs(bool has)
    {
        header_.SetHasFloatRegs(has);
    }

    template <typename Func>
    constexpr void EnumerateTables(Func func)
    {
        size_t index = 0;
        func(index++, &stack_maps_);
        func(index++, &inline_infos_);
        func(index++, &roots_reg_masks_);
        func(index++, &roots_stack_masks_);
        func(index++, &method_ids_);
        func(index++, &vreg_masks_);
        func(index++, &vregs_map_);
        func(index++, &vregs_catalogue_);
        func(index++, &implicit_nullchecks_);
        func(index++, &constant_table_);
        ASSERT(index == CodeInfo::TABLES_COUNT);
    }

    void DumpCurrentStackMap(std::ostream &stream) const;

private:
    void EmitVRegs();

private:
    Arch arch_;
    uint32_t vregs_count_ {0};
    uint32_t current_vregs_count_ {0};

    CodeInfoHeader header_ {};

    // Tables
    BitTableBuilder<StackMap> stack_maps_;
    BitTableBuilder<InlineInfo> inline_infos_;
    BitTableBuilder<RegisterMask> roots_reg_masks_;
    BitmapTableBuilder roots_stack_masks_;
    BitTableBuilder<MethodId> method_ids_;
    BitTableBuilder<VRegisterInfo> vregs_catalogue_;
    BitTableBuilder<VRegisterCatalogueIndex> vregs_map_;
    BitmapTableBuilder vreg_masks_;
    BitTableBuilder<ImplicitNullChecks> implicit_nullchecks_;
    BitTableBuilder<ConstantTable> constant_table_;

    // Auxiliary containers
    BitTableBuilder<StackMap>::Entry current_stack_map_;
    ArenaVector<VRegInfo> current_vregs_;
    ArenaVector<VRegInfo> last_vregs_;
    ArenaVector<uint32_t> vregs_last_change_;
    ArenaVector<BitTableBuilder<InlineInfo>::Entry> inline_info_stack_;
    ArenaVector<BitTableBuilder<VRegisterCatalogueIndex>::Entry> vregs_map_storage_;
    ArenaBitVector vregs_mask_storage_;

#ifndef NDEBUG
    bool was_method_begin_ {false};
    bool was_stack_map_begin_ {false};
    bool was_inline_info_begin_ {false};
#endif

    static constexpr size_t MAX_VREG_LIVE_DISTANCE = 32;
};

}  // namespace panda::compiler

#endif  // PANDA_CODE_INFO_BUILDER_H
