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

#include "unit_test.h"
#include "code_info/code_info.h"
#include "code_info/code_info_builder.h"
#include "mem/pool_manager.h"
#include <fstream>

namespace panda::compiler {

// NOLINTBEGIN(readability-magic-numbers)
class CodeInfoTest : public ::testing::Test {
public:
    CodeInfoTest()
    {
        panda::mem::MemConfig::Initialize(0U, 64_MB, 256_MB, 32_MB, 0U, 0U);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    ~CodeInfoTest() override
    {
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    NO_MOVE_SEMANTIC(CodeInfoTest);
    NO_COPY_SEMANTIC(CodeInfoTest);

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    auto EmitCode(CodeInfoBuilder &builder)
    {
        static constexpr size_t DUMMY_CODE_SIZE = 16;
        ArenaVector<uint8_t> data(GetAllocator()->Adapter());
        size_t code_offset = CodeInfo::GetCodeOffset(RUNTIME_ARCH) + DUMMY_CODE_SIZE;
        data.resize(code_offset);
        builder.Encode(&data, data.size() * BITS_PER_BYTE);
        auto *prefix = new (data.data()) CodePrefix;
        prefix->code_size = data.size();
        prefix->code_info_offset = code_offset;
        prefix->code_info_size = data.size() - code_offset;
        return data;
    }

    template <typename Callback>
    void EnumerateVRegs(const CodeInfo &code_info, const StackMap &stack_map, int inline_depth, Callback callback)
    {
        auto list = code_info.GetVRegList(stack_map, inline_depth, GetAllocator());
        for (auto vreg : list) {
            callback(vreg);
        }
    }

    template <size_t N>
    void CompareVRegs(CodeInfo &code_info, StackMap stack_map, int inline_info_index, std::array<VRegInfo, N> vregs)
    {
        std::vector<VRegInfo> vregs_in_map;
        if (inline_info_index == -1L) {
            EnumerateVRegs(code_info, stack_map, -1L, [&vregs_in_map](auto vreg) { vregs_in_map.push_back(vreg); });
        } else {
            EnumerateVRegs(code_info, stack_map, inline_info_index,
                           [&vregs_in_map](auto vreg) { vregs_in_map.push_back(vreg); });
        }
        ASSERT_EQ(vregs_in_map.size(), vregs.size());
        for (size_t i = 0; i < vregs.size(); i++) {
            vregs_in_map[i].SetIndex(0U);
            ASSERT_EQ(vregs_in_map[i], vregs[i]);
        }
    }

private:
    ArenaAllocator *allocator_ {nullptr};
};

TEST_F(CodeInfoTest, SingleStackmap)
{
    auto regs_count = GetCalleeRegsCount(RUNTIME_ARCH, false) + GetCalleeRegsCount(RUNTIME_ARCH, true) +
                      GetCallerRegsCount(RUNTIME_ARCH, false) + GetCallerRegsCount(RUNTIME_ARCH, true);
    std::array vregs = {VRegInfo(1U, VRegInfo::Location::FP_REGISTER, VRegInfo::Type::INT64, VRegInfo::VRegType::VREG),
                        VRegInfo(2U, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
                        VRegInfo(12U, VRegInfo::Location::REGISTER, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG)};
    if constexpr (!ArchTraits<RUNTIME_ARCH>::IS_64_BITS) {  // NOLINT
        vregs[1U].SetValue((vregs[1U].GetValue() << 1U) + 1U + regs_count);
    } else {  // NOLINT
        vregs[1U].SetValue(vregs[1U].GetValue() + regs_count);
    }
    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(1U, 3U);
    ArenaBitVector stack_roots(GetAllocator());
    stack_roots.resize(3U);
    stack_roots.SetBit(2U);
    std::bitset<32U> reg_roots(0U);
    reg_roots.set(12U);
    builder.BeginStackMap(10U, 20U, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(vregs[0U]);
    builder.AddVReg(vregs[1U]);
    builder.AddVReg(vregs[2U]);
    builder.EndStackMap();
    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    CodeInfo code_info(data.data());
    ASSERT_EQ(code_info.GetHeader().GetFrameSize(), 1U);
    ASSERT_EQ(code_info.GetStackMaps().GetRowsCount(), 1U);
    auto stack_map = code_info.GetStackMaps().GetRow(0U);
    ASSERT_EQ(stack_map.GetBytecodePc(), 10U);
    ASSERT_EQ(stack_map.GetNativePcUnpacked(), 20U);
    ASSERT_FALSE(stack_map.HasInlineInfoIndex());
    ASSERT_TRUE(stack_map.HasRootsRegMaskIndex());
    ASSERT_TRUE(stack_map.HasRootsStackMaskIndex());
    ASSERT_TRUE(stack_map.HasVRegMaskIndex());
    ASSERT_TRUE(stack_map.HasVRegMapIndex());

    ASSERT_EQ(Popcount(code_info.GetRootsRegMask(stack_map)), 1U);
    ASSERT_EQ(code_info.GetRootsRegMask(stack_map), 1U << 12U);

    ASSERT_EQ(code_info.GetRootsStackMask(stack_map).Popcount(), 1U);
    auto roots_region = code_info.GetRootsStackMask(stack_map);
    ASSERT_EQ(roots_region.Size(), 3U);
    ASSERT_EQ(code_info.GetRootsStackMask(stack_map).Read(0U, 3U), 1U << 2U);

    ASSERT_EQ(code_info.GetVRegMask(stack_map).Popcount(), 3U);
    ASSERT_EQ(code_info.GetVRegMask(stack_map).Size(), 3U);

    size_t index = 0;
    EnumerateVRegs(code_info, stack_map, -1, [&vregs, &index](auto vreg) {
        vreg.SetIndex(0U);
        ASSERT_EQ(vreg, vregs[index++]);
    });

    std::bitset<vregs.size()> mask(0U);
    code_info.EnumerateStaticRoots(stack_map, [&mask, &vregs](auto vreg) -> bool {
        auto it = std::find(vregs.begin(), vregs.end(), vreg);
        if (it != vregs.end()) {
            ASSERT(std::distance(vregs.begin(), it) < helpers::ToSigned(mask.size()));
            mask.set(std::distance(vregs.begin(), it));
        }
        return true;
    });
    ASSERT_EQ(Popcount(mask.to_ullong()), 2U);
    ASSERT_TRUE(mask.test(1U));
    ASSERT_TRUE(mask.test(2U));
}

TEST_F(CodeInfoTest, MultipleStackmaps)
{
    uintptr_t method_stub;
    std::array vregs = {
        VRegInfo(1U, VRegInfo::Location::REGISTER, VRegInfo::Type::INT64, VRegInfo::VRegType::VREG),
        VRegInfo(2U, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
        VRegInfo(3U, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
        VRegInfo(10U, VRegInfo::Location::FP_REGISTER, VRegInfo::Type::FLOAT64, VRegInfo::VRegType::VREG),
        VRegInfo(20U, VRegInfo::Location::SLOT, VRegInfo::Type::BOOL, VRegInfo::VRegType::VREG),
        VRegInfo(30U, VRegInfo::Location::REGISTER, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
    };
    ArenaBitVector stack_roots(GetAllocator());
    stack_roots.resize(8U);
    std::bitset<32U> reg_roots(0U);

    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(12U, 3U);

    stack_roots.SetBit(1U);
    stack_roots.SetBit(2U);
    builder.BeginStackMap(10U, 20U, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(vregs[0U]);
    builder.AddVReg(vregs[1U]);
    builder.AddVReg(VRegInfo());
    builder.BeginInlineInfo(&method_stub, 0U, 1U, 2U);
    builder.AddVReg(vregs[2U]);
    builder.AddVReg(vregs[3U]);
    builder.EndInlineInfo();
    builder.BeginInlineInfo(nullptr, 0x123456U, 2U, 1U);
    builder.AddVReg(vregs[3U]);
    builder.EndInlineInfo();
    builder.EndStackMap();

    stack_roots.Reset();
    reg_roots.reset();
    reg_roots.set(5U);
    builder.BeginStackMap(30U, 40U, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(vregs[3U]);
    builder.AddVReg(vregs[5U]);
    builder.AddVReg(vregs[4U]);
    builder.BeginInlineInfo(nullptr, 0xabcdefU, 3U, 2U);
    builder.AddVReg(vregs[1U]);
    builder.AddVReg(VRegInfo());
    builder.EndInlineInfo();
    builder.EndStackMap();

    stack_roots.Reset();
    reg_roots.reset();
    reg_roots.set(1U);
    builder.BeginStackMap(50U, 60U, &stack_roots, reg_roots.to_ullong(), true, false);
    builder.AddVReg(VRegInfo());
    builder.AddVReg(VRegInfo());
    builder.AddVReg(VRegInfo());
    builder.EndStackMap();

    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    CodeInfo code_info(data.data());

    {
        auto stack_map = code_info.GetStackMaps().GetRow(0U);
        ASSERT_EQ(stack_map.GetBytecodePc(), 10U);
        ASSERT_EQ(stack_map.GetNativePcUnpacked(), 20U);
        ASSERT_TRUE(stack_map.HasInlineInfoIndex());
        CompareVRegs(code_info, stack_map, -1L, std::array {vregs[0U], vregs[1U], VRegInfo()});

        ASSERT_TRUE(stack_map.HasInlineInfoIndex());
        auto inline_infos = code_info.GetInlineInfos(stack_map);
        ASSERT_EQ(std::distance(inline_infos.begin(), inline_infos.end()), 2U);
        auto it = inline_infos.begin();
        ASSERT_EQ(std::get<void *>(code_info.GetMethod(stack_map, 0U)), &method_stub);
        ASSERT_TRUE(it->Get(InlineInfo::COLUMN_IS_LAST));
        CompareVRegs(code_info, stack_map, it->GetRow() - stack_map.GetInlineInfoIndex(), std::array {vregs[3U]});
        ++it;
        ASSERT_FALSE(it->Get(InlineInfo::COLUMN_IS_LAST));
        ASSERT_EQ(std::get<uint32_t>(code_info.GetMethod(stack_map, 1U)), 0x123456U);
        CompareVRegs(code_info, stack_map, it->GetRow() - stack_map.GetInlineInfoIndex(),
                     std::array {vregs[2U], vregs[3U]});
    }
    {
        auto stack_map = code_info.GetStackMaps().GetRow(1U);
        ASSERT_EQ(stack_map.GetBytecodePc(), 30U);
        ASSERT_EQ(stack_map.GetNativePcUnpacked(), 40U);
        CompareVRegs(code_info, stack_map, -1L, std::array {vregs[3U], vregs[5U], vregs[4U]});

        ASSERT_TRUE(stack_map.HasInlineInfoIndex());
        auto inline_infos = code_info.GetInlineInfos(stack_map);
        ASSERT_EQ(std::distance(inline_infos.begin(), inline_infos.end()), 1U);
        ASSERT_EQ(std::get<uint32_t>(code_info.GetMethod(stack_map, 0U)), 0xabcdefU);
        ASSERT_TRUE(inline_infos[0U].Get(InlineInfo::COLUMN_IS_LAST));
        CompareVRegs(code_info, stack_map, inline_infos[0].GetRow() - stack_map.GetInlineInfoIndex(),
                     std::array {vregs[1], VRegInfo()});
    }

    {
        auto stack_map = code_info.GetStackMaps().GetRow(2U);
        ASSERT_EQ(stack_map.GetBytecodePc(), 50U);
        ASSERT_EQ(stack_map.GetNativePcUnpacked(), 60U);
        CompareVRegs(code_info, stack_map, -1L, std::array {VRegInfo(), VRegInfo(), VRegInfo()});

        ASSERT_FALSE(stack_map.HasInlineInfoIndex());
        auto inline_infos = code_info.GetInlineInfos(stack_map);
        ASSERT_EQ(std::distance(inline_infos.begin(), inline_infos.end()), 0U);
    }

    {
        auto stackmap = code_info.FindStackMapForNativePc(20U);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 20U);
        ASSERT_EQ(stackmap.GetBytecodePc(), 10U);
        stackmap = code_info.FindStackMapForNativePc(40U);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 40U);
        ASSERT_EQ(stackmap.GetBytecodePc(), 30U);
        stackmap = code_info.FindStackMapForNativePc(60U);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 60U);
        ASSERT_EQ(stackmap.GetBytecodePc(), 50U);
        stackmap = code_info.FindStackMapForNativePc(90U);
        ASSERT_FALSE(stackmap.IsValid());
    }
}

TEST_F(CodeInfoTest, Constants)
{
    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(12U, 3U);
    builder.BeginStackMap(1U, 4U, nullptr, 0U, true, false);
    builder.AddConstant(0U, VRegInfo::Type::INT64, VRegInfo::VRegType::VREG);
    builder.AddConstant(0x1234567890abcdefU, VRegInfo::Type::INT64, VRegInfo::VRegType::ACC);
    builder.AddConstant(0x12345678U, VRegInfo::Type::INT32, VRegInfo::VRegType::VREG);
    builder.EndStackMap();
    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    CodeInfo code_info(data.data());

    auto stack_map = code_info.GetStackMaps().GetRow(0U);

    auto vreg_list = code_info.GetVRegList(stack_map, GetAllocator());
    ASSERT_EQ(code_info.GetConstant(vreg_list[0U]), 0U);
    ASSERT_EQ(code_info.GetConstant(vreg_list[1U]), 0x1234567890abcdefU);
    ASSERT_EQ(code_info.GetConstant(vreg_list[2U]), 0x12345678U);
    // 0 and 0x12345678 should be deduplicated
    ASSERT_EQ(code_info.GetConstantTable().GetRowsCount(), 3U);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace panda::compiler
