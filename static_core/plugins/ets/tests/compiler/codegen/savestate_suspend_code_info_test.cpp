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

#include "gmock/gmock.h"
#include "tests/codegen/codegen_test.h"

namespace ark::compiler {
// NOLINTBEGIN(readability-magic-numbers)
namespace {
template <typename ContainerT, typename MapT>
[[maybe_unused]] auto MapVector(ContainerT &&range, MapT &&mapper)
{
    std::vector<std::remove_reference_t<decltype(mapper(*range.begin()))>> res;
    for (auto &el : range) {
        // sometimes size is not known
        res.push_back(mapper(el));  // NOLINT(performance-inefficient-vector-operation)
    }
    return res;
}

template <typename ContainerT, typename PredT>
[[maybe_unused]] auto FilterVector(ContainerT &&range, PredT &&pred)
{
    std::vector<std::remove_reference_t<decltype(*range.begin())>> res;
    for (auto &el : range) {
        if (pred(el)) {
            // sometimes size is not known
            res.push_back(el);  // NOLINT(performance-inefficient-vector-operation)
        }
    }
    return res;
}

template <typename GenT>
[[maybe_unused]] auto GenerateVector(size_t n, GenT &&generator)
{
    std::vector<std::remove_reference_t<decltype(generator(0))>> res;
    res.reserve(n);
    for (size_t i = 0; i < n; i++) {
        res.push_back(generator(i));
    }
    return res;
}

[[maybe_unused]] std::optional<StackMap> FindStackMapForBytecodePC(CodeInfo &codeInfo, size_t bpc)
{
    auto &stackMaps = codeInfo.GetStackMaps();
    if (auto it =
            std::find_if(stackMaps.begin(), stackMaps.end(), [bpc](auto &sm) { return sm.GetBytecodePc() == bpc; });
        it != stackMaps.end()) {
        return *it;
    }
    return {};
}

template <typename Allocator, typename ContainerT>
[[maybe_unused]] void CompareVRegsNoValue(CodeInfo &codeInfo, StackMap &stackMap, Allocator *allocator,
                                          ContainerT &&expectedVRegs)
{
    std::vector<VRegInfo> actualVRegs =
        MapVector(FilterVector(codeInfo.GetVRegList(stackMap, allocator), [](auto &vreg) { return vreg.IsLive(); }),
                  [](auto vreg) {
                      vreg.SetValue(0);
                      return vreg;
                  });
    EXPECT_THAT(actualVRegs, ::testing::UnorderedElementsAreArray(expectedVRegs));
}
}  // namespace

class SaveStateSuspendCodeInfoTest : public GraphTest {
public:
    static constexpr int BRIDGE_VREG = static_cast<int>(VirtualRegister::BRIDGE);

    void CreateGraph(bool isPrimitive)
    {
        auto inputType = isPrimitive ? &IrConstructor::s32 : &IrConstructor::ref;
        GRAPH(GetGraph())
        {
            (PARAMETER(0U, 0U).*inputType)();
            (PARAMETER(1U, 1U).*inputType)();
            PARAMETER(2U, 2U).ref();
            BASIC_BLOCK(2U, 3U)
            {
                INST(saveStateSuspendId_ = 10U, Opcode::SaveStateSuspend).Inputs(2U).SrcVregs({2U}).Pc(bpc_ = 30U);
            }
            BASIC_BLOCK(3U, 4U, 5U)
            {
                INST(3U, Opcode::If).CC(CC_EQ).Inputs(2U, 2U);
            }
            BASIC_BLOCK(4U, 6U) {}
            BASIC_BLOCK(5U, 6U) {}
            BASIC_BLOCK(6U, -1L)
            {
                (INST(4U, Opcode::Phi).*inputType)().Inputs({{4U, 0U}, {5U, 1U}});
                (INST(5U, Opcode::Return).*inputType)().Inputs(4U);
            }
        }
        GetGraph()->SetLanguage(SourceLanguage::ETS);
        GetGraph()->SetIsAsync(true);
        SetNumArgs(3U);
        SetNumVirtRegs(0U);
    }

    void CheckCodeInfo(bool isPrimitive)
    {
        CreateGraph(isPrimitive);

        RegAlloc(GetGraph());

        auto *saveStateSuspend = INS(saveStateSuspendId_).CastToSaveStateSuspend();
        EXPECT_THAT(
            GenerateVector(saveStateSuspend->GetInputsCount(),
                           [saveStateSuspend](size_t i) { return saveStateSuspend->GetVirtualRegister(i).Value(); }),
            ::testing::ElementsAre(2U, BRIDGE_VREG, BRIDGE_VREG));

        EXPECT_TRUE(RunCodegen(GetGraph()));

        CodeInfo codeInfo;
        ASSERT_FALSE(GetGraph()->GetCodeInfoData().empty());
        codeInfo.Decode(GetGraph()->GetCodeInfoData());
        auto maybeStackMap = FindStackMapForBytecodePC(codeInfo, bpc_);
        ASSERT_TRUE(maybeStackMap.has_value());
        auto &stackMap = *maybeStackMap;

        EXPECT_TRUE(stackMap.HasRegMap());
        EXPECT_EQ(codeInfo.GetVRegCount(stackMap), 3U + 1U + 2U);  // 3 args + 1 acc + 2 bridges

        auto inputType = isPrimitive ? VRegInfo::Type::INT32 : VRegInfo::Type::OBJECT;
        std::array vregs = {
            VRegInfo(0U, VRegInfo::Location::SLOT, VRegInfo::Type::OBJECT, VRegInfo::VRegType::VREG, 2U),  // ref param
            VRegInfo(0U, VRegInfo::Location::SLOT, inputType, VRegInfo::VRegType::VREG, 4U),  // first "fake" bridge
            VRegInfo(0U, VRegInfo::Location::SLOT, inputType, VRegInfo::VRegType::VREG, 5U),  // second "fake" bridge
        };
        CompareVRegsNoValue(codeInfo, stackMap, GetAllocator(), vregs);
    }

private:
    int saveStateSuspendId_ = -1;
    int bpc_ = -1;
};

TEST_F(SaveStateSuspendCodeInfoTest, PrimitiveBridges)
{
    CheckCodeInfo(true);
}

TEST_F(SaveStateSuspendCodeInfoTest, ObjectBridges)
{
    CheckCodeInfo(false);
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
