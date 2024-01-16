/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)
class CodeInfoTest : public ::testing::Test {
public:
    CodeInfoTest()
    {
        ark::mem::MemConfig::Initialize(0U, 64_MB, 256_MB, 32_MB, 0U, 0U);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    ~CodeInfoTest() override
    {
        delete allocator_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
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
        size_t codeOffset = CodeInfo::GetCodeOffset(RUNTIME_ARCH) + DUMMY_CODE_SIZE;
        data.resize(codeOffset);
        builder.Encode(&data, data.size() * BITS_PER_BYTE);
        auto *prefix = new (data.data()) CodePrefix;
        prefix->codeSize = data.size();
        prefix->codeInfoOffset = codeOffset;
        prefix->codeInfoSize = data.size() - codeOffset;
        return data;
    }

    template <typename Callback>
    void EnumerateVRegs(const CodeInfo &codeInfo, const StackMap &stackMap, int inlineDepth, Callback callback)
    {
        auto list = codeInfo.GetVRegList(stackMap, inlineDepth, GetAllocator());
        for (auto vreg : list) {
            callback(vreg);
        }
    }

    template <size_t N>
    void CompareVRegs(CodeInfo &codeInfo, StackMap stackMap, int inlineInfoIndex, std::array<VRegInfo, N> vregs)
    {
        std::vector<VRegInfo> vregsInMap;
        if (inlineInfoIndex == -1L) {
            EnumerateVRegs(codeInfo, stackMap, -1L, [&vregsInMap](auto vreg) { vregsInMap.push_back(vreg); });
        } else {
            EnumerateVRegs(codeInfo, stackMap, inlineInfoIndex,
                           [&vregsInMap](auto vreg) { vregsInMap.push_back(vreg); });
        }
        ASSERT_EQ(vregsInMap.size(), vregs.size());
        for (size_t i = 0; i < vregs.size(); i++) {
            vregsInMap[i].SetIndex(0U);
            ASSERT_EQ(vregsInMap[i], vregs[i]);
        }
    }

private:
    ArenaAllocator *allocator_ {nullptr};
};

TEST_F(CodeInfoTest, SingleStackmap)
{
    auto regsCount = GetCalleeRegsCount(RUNTIME_ARCH, false) + GetCalleeRegsCount(RUNTIME_ARCH, true) +
                     GetCallerRegsCount(RUNTIME_ARCH, false) + GetCallerRegsCount(RUNTIME_ARCH, true);
    std::array vregs = {VRegInfo(1U, VRegInfo::Location::FP_REGISTER, VRegInfo::Type::INT64, VRegInfo::VRegType::VREG),
                        VRegInfo(2U, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
                        VRegInfo(12U, VRegInfo::Location::REGISTER, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG)};
    if constexpr (!ArchTraits<RUNTIME_ARCH>::IS_64_BITS) {  // NOLINT
        vregs[1U].SetValue((vregs[1U].GetValue() << 1U) + 1U + regsCount);
    } else {  // NOLINT
        vregs[1U].SetValue(vregs[1U].GetValue() + regsCount);
    }
    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(1U, 3U);
    ArenaBitVector stackRoots(GetAllocator());
    stackRoots.resize(3U);
    stackRoots.SetBit(2U);
    std::bitset<32U> regRoots(0U);
    regRoots.set(12U);
    builder.BeginStackMap(10U, 20U, &stackRoots, regRoots.to_ullong(), true, false);
    builder.AddVReg(vregs[0U]);
    builder.AddVReg(vregs[1U]);
    builder.AddVReg(vregs[2U]);
    builder.EndStackMap();
    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    BitMemoryStreamIn in(data.data(), 0U, data.size() * BITS_PER_BYTE);
    CodeInfo codeInfo(data.data());
    ASSERT_EQ(codeInfo.GetHeader().GetFrameSize(), 1U);
    ASSERT_EQ(codeInfo.GetStackMaps().GetRowsCount(), 1U);
    auto stackMap = codeInfo.GetStackMaps().GetRow(0U);
    ASSERT_EQ(stackMap.GetBytecodePc(), 10U);
    ASSERT_EQ(stackMap.GetNativePcUnpacked(), 20U);
    ASSERT_FALSE(stackMap.HasInlineInfoIndex());
    ASSERT_TRUE(stackMap.HasRootsRegMaskIndex());
    ASSERT_TRUE(stackMap.HasRootsStackMaskIndex());
    ASSERT_TRUE(stackMap.HasVRegMaskIndex());
    ASSERT_TRUE(stackMap.HasVRegMapIndex());

    ASSERT_EQ(Popcount(codeInfo.GetRootsRegMask(stackMap)), 1U);
    ASSERT_EQ(codeInfo.GetRootsRegMask(stackMap), 1U << 12U);

    ASSERT_EQ(codeInfo.GetRootsStackMask(stackMap).Popcount(), 1U);
    auto rootsRegion = codeInfo.GetRootsStackMask(stackMap);
    ASSERT_EQ(rootsRegion.Size(), 3U);
    ASSERT_EQ(codeInfo.GetRootsStackMask(stackMap).Read(0U, 3U), 1U << 2U);

    ASSERT_EQ(codeInfo.GetVRegMask(stackMap).Popcount(), 3U);
    ASSERT_EQ(codeInfo.GetVRegMask(stackMap).Size(), 3U);

    size_t index = 0;
    EnumerateVRegs(codeInfo, stackMap, -1, [&vregs, &index](auto vreg) {
        vreg.SetIndex(0U);
        ASSERT_EQ(vreg, vregs[index++]);
    });

    std::bitset<vregs.size()> mask(0U);
    codeInfo.EnumerateStaticRoots(stackMap, [&mask, &vregs](auto vreg) -> bool {
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
    uintptr_t methodStub;
    std::array vregs = {
        VRegInfo(1U, VRegInfo::Location::REGISTER, VRegInfo::Type::INT64, VRegInfo::VRegType::VREG),
        VRegInfo(2U, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
        VRegInfo(3U, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
        VRegInfo(10U, VRegInfo::Location::FP_REGISTER, VRegInfo::Type::FLOAT64, VRegInfo::VRegType::VREG),
        VRegInfo(20U, VRegInfo::Location::SLOT, VRegInfo::Type::BOOL, VRegInfo::VRegType::VREG),
        VRegInfo(30U, VRegInfo::Location::REGISTER, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG),
    };
    ArenaBitVector stackRoots(GetAllocator());
    stackRoots.resize(8U);
    std::bitset<32U> regRoots(0U);

    CodeInfoBuilder builder(RUNTIME_ARCH, GetAllocator());
    builder.BeginMethod(12U, 3U);

    stackRoots.SetBit(1U);
    stackRoots.SetBit(2U);
    builder.BeginStackMap(10U, 20U, &stackRoots, regRoots.to_ullong(), true, false);
    builder.AddVReg(vregs[0U]);
    builder.AddVReg(vregs[1U]);
    builder.AddVReg(VRegInfo());
    builder.BeginInlineInfo(&methodStub, 0U, 1U, 2U);
    builder.AddVReg(vregs[2U]);
    builder.AddVReg(vregs[3U]);
    builder.EndInlineInfo();
    builder.BeginInlineInfo(nullptr, 0x123456U, 2U, 1U);
    builder.AddVReg(vregs[3U]);
    builder.EndInlineInfo();
    builder.EndStackMap();

    stackRoots.Reset();
    regRoots.reset();
    regRoots.set(5U);
    builder.BeginStackMap(30U, 40U, &stackRoots, regRoots.to_ullong(), true, false);
    builder.AddVReg(vregs[3U]);
    builder.AddVReg(vregs[5U]);
    builder.AddVReg(vregs[4U]);
    builder.BeginInlineInfo(nullptr, 0xabcdefU, 3U, 2U);
    builder.AddVReg(vregs[1U]);
    builder.AddVReg(VRegInfo());
    builder.EndInlineInfo();
    builder.EndStackMap();

    stackRoots.Reset();
    regRoots.reset();
    regRoots.set(1U);
    builder.BeginStackMap(50U, 60U, &stackRoots, regRoots.to_ullong(), true, false);
    builder.AddVReg(VRegInfo());
    builder.AddVReg(VRegInfo());
    builder.AddVReg(VRegInfo());
    builder.EndStackMap();

    builder.EndMethod();

    ArenaVector<uint8_t> data = EmitCode(builder);

    CodeInfo codeInfo(data.data());

    {
        auto stackMap = codeInfo.GetStackMaps().GetRow(0U);
        ASSERT_EQ(stackMap.GetBytecodePc(), 10U);
        ASSERT_EQ(stackMap.GetNativePcUnpacked(), 20U);
        ASSERT_TRUE(stackMap.HasInlineInfoIndex());
        CompareVRegs(codeInfo, stackMap, -1L, std::array {vregs[0U], vregs[1U], VRegInfo()});

        ASSERT_TRUE(stackMap.HasInlineInfoIndex());
        auto inlineInfos = codeInfo.GetInlineInfos(stackMap);
        ASSERT_EQ(std::distance(inlineInfos.begin(), inlineInfos.end()), 2U);
        auto it = inlineInfos.begin();
        ASSERT_EQ(std::get<void *>(codeInfo.GetMethod(stackMap, 0U)), &methodStub);
        ASSERT_TRUE(it->Get(InlineInfo::COLUMN_IS_LAST));
        CompareVRegs(codeInfo, stackMap, it->GetRow() - stackMap.GetInlineInfoIndex(), std::array {vregs[3U]});
        ++it;
        ASSERT_FALSE(it->Get(InlineInfo::COLUMN_IS_LAST));
        ASSERT_EQ(std::get<uint32_t>(codeInfo.GetMethod(stackMap, 1U)), 0x123456U);
        CompareVRegs(codeInfo, stackMap, it->GetRow() - stackMap.GetInlineInfoIndex(),
                     std::array {vregs[2U], vregs[3U]});
    }
    {
        auto stackMap = codeInfo.GetStackMaps().GetRow(1U);
        ASSERT_EQ(stackMap.GetBytecodePc(), 30U);
        ASSERT_EQ(stackMap.GetNativePcUnpacked(), 40U);
        CompareVRegs(codeInfo, stackMap, -1L, std::array {vregs[3U], vregs[5U], vregs[4U]});

        ASSERT_TRUE(stackMap.HasInlineInfoIndex());
        auto inlineInfos = codeInfo.GetInlineInfos(stackMap);
        ASSERT_EQ(std::distance(inlineInfos.begin(), inlineInfos.end()), 1U);
        ASSERT_EQ(std::get<uint32_t>(codeInfo.GetMethod(stackMap, 0U)), 0xabcdefU);
        ASSERT_TRUE(inlineInfos[0U].Get(InlineInfo::COLUMN_IS_LAST));
        CompareVRegs(codeInfo, stackMap, inlineInfos[0].GetRow() - stackMap.GetInlineInfoIndex(),
                     std::array {vregs[1], VRegInfo()});
    }

    {
        auto stackMap = codeInfo.GetStackMaps().GetRow(2U);
        ASSERT_EQ(stackMap.GetBytecodePc(), 50U);
        ASSERT_EQ(stackMap.GetNativePcUnpacked(), 60U);
        CompareVRegs(codeInfo, stackMap, -1L, std::array {VRegInfo(), VRegInfo(), VRegInfo()});

        ASSERT_FALSE(stackMap.HasInlineInfoIndex());
        auto inlineInfos = codeInfo.GetInlineInfos(stackMap);
        ASSERT_EQ(std::distance(inlineInfos.begin(), inlineInfos.end()), 0U);
    }

    {
        auto stackmap = codeInfo.FindStackMapForNativePc(20U);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 20U);
        ASSERT_EQ(stackmap.GetBytecodePc(), 10U);
        stackmap = codeInfo.FindStackMapForNativePc(40U);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 40U);
        ASSERT_EQ(stackmap.GetBytecodePc(), 30U);
        stackmap = codeInfo.FindStackMapForNativePc(60U);
        ASSERT_TRUE(stackmap.IsValid());
        ASSERT_EQ(stackmap.GetNativePcUnpacked(), 60U);
        ASSERT_EQ(stackmap.GetBytecodePc(), 50U);
        stackmap = codeInfo.FindStackMapForNativePc(90U);
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

    CodeInfo codeInfo(data.data());

    auto stackMap = codeInfo.GetStackMaps().GetRow(0U);

    auto vregList = codeInfo.GetVRegList(stackMap, GetAllocator());
    ASSERT_EQ(codeInfo.GetConstant(vregList[0U]), 0U);
    ASSERT_EQ(codeInfo.GetConstant(vregList[1U]), 0x1234567890abcdefU);
    ASSERT_EQ(codeInfo.GetConstant(vregList[2U]), 0x12345678U);
    // 0 and 0x12345678 should be deduplicated
    ASSERT_EQ(codeInfo.GetConstantTable().GetRowsCount(), 3U);
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
