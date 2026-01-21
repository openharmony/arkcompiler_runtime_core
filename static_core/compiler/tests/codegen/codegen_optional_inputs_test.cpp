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

#include <sys/types.h>
#include "codegen_test.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)
class OptionalInputsTest : public CodegenTest {
public:
    template <typename Callback>
    LoadInst *CreateGraph(Callback &&createLoadArray)
    {
        GRAPH(GetGraph())
        {
            PARAMETER(0U, 0U).ptr();  // passed to optional input
            PARAMETER(1U, 1U).ref();  // array
            PARAMETER(2U, 2U).u64();  // index
            BASIC_BLOCK(2U, -1L)
            {
                createLoadArray(INST(3U, Opcode::LoadArray).u64(), 1U, 2U, 0U);
                INST(4U, Opcode::Return).u64().Inputs(3U);
            }
        }
        SetNumVirtRegs(0);
        auto *bb = GetGraph()->GetVectorBlocks()[2U];
        auto *loadArray = bb->GetFirstInst()->CastToLoadArray();
        return loadArray;
    }

    bool CompileGraph()
    {
        InPlaceCompilerTaskRunner taskRunner;
        taskRunner.GetContext().SetGraph(GetGraph());
        bool success = true;
        taskRunner.AddCallbackOnFail(
            [&success]([[maybe_unused]] InPlaceCompilerContext &compilerCtx) { success = false; });
        RunOptimizations<INPLACE_MODE>(std::move(taskRunner));
        return success;
    }

    template <size_t N>
    uint64_t ExecGraph(uint64_t arg1, std::array<uint64_t, N> arg2, uint64_t arg3)
    {
        auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
        auto codeExit = codeEntry + GetGraph()->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetParameter(0U, arg1);
        auto param1 = GetExecModule().CreateArray(arg2.data(), arg2.size(), GetObjectAllocator());
        GetExecModule().SetParameter(1U, reinterpret_cast<uint64_t>(param1));
        auto param2 = CutValue<uint64_t>(arg3, DataType::INT64);
        GetExecModule().SetParameter(2U, param2);
        GetExecModule().Execute();
        return GetExecModule().GetRetValue();
    }
};

TEST_F(OptionalInputsTest, OptionalInputs)
{
    auto *loadArray =
        this->CreateGraph([](auto &ir, unsigned arrInput, unsigned idxInput, unsigned optInput) -> decltype(auto) {
            return ir.Inputs(arrInput, idxInput, optInput);
        });
    EXPECT_NE(loadArray->GetGCBarrierEntrypoint(), nullptr);
    EXPECT_EQ(loadArray->GetInputsCount(), 3U);

    EXPECT_TRUE(CompileGraph());

    EXPECT_EQ(ExecGraph<3U>(0U, {1U, 2U, 3U}, 1U), 2U);
}

TEST_F(OptionalInputsTest, OptionalInputsAdd)
{
    auto *loadArray = this->CreateGraph(
        [](auto &ir, unsigned arrInput, unsigned idxInput, [[maybe_unused]] unsigned optInput) -> decltype(auto) {
            return ir.Inputs(arrInput, idxInput);
        });
    EXPECT_EQ(loadArray->GetGCBarrierEntrypoint(), nullptr);
    EXPECT_EQ(loadArray->GetInputsCount(), 2U);
    auto *entrypoint = *GetGraph()->GetParameters().begin();
    loadArray->SetGCBarrierEntrypoint(entrypoint);
    EXPECT_EQ(loadArray->GetInputsCount(), 3U);
    EXPECT_EQ(loadArray->GetGCBarrierEntrypoint(), entrypoint);

    EXPECT_TRUE(CompileGraph());

    EXPECT_EQ(ExecGraph<3U>(0U, {1U, 2U, 3U}, 1U), 2U);
}

TEST_F(OptionalInputsTest, OptionalInputsRemove)
{
    auto *loadArray =
        this->CreateGraph([](auto &ir, unsigned arrInput, unsigned idxInput, unsigned optInput) -> decltype(auto) {
            return ir.Inputs(arrInput, idxInput, optInput);
        });
    EXPECT_NE(loadArray->GetGCBarrierEntrypoint(), nullptr);
    EXPECT_EQ(loadArray->GetInputsCount(), 3U);
    loadArray->ResetGCBarrierEntrypoint();
    EXPECT_EQ(loadArray->GetGCBarrierEntrypoint(), nullptr);
    EXPECT_EQ(loadArray->GetInputsCount(), 2U);

    EXPECT_TRUE(CompileGraph());

    EXPECT_EQ(ExecGraph<3U>(0U, {1U, 2U, 3U}, 1U), 2U);
}
// NOLINTEND(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)

}  // namespace ark::compiler
