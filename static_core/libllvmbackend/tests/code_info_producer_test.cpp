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

#include <algorithm>
#include <vector>

#include "gmock/gmock.h"
#include "llvm_aot_compiler.h"
#include "compiler/aot/aot_builder/llvm_aot_builder.h"
#include "compiler/code_info/code_info.h"
#include "tests/unit_test.h"

namespace ark::llvmbackend {
namespace {
// NOLINTBEGIN(readability-magic-numbers)
template <typename ContainerT, typename PredT>
auto FilterVector(ContainerT &&range, PredT &&pred)
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

template <typename ContainerT, typename MapT>
auto MapVector(ContainerT &&range, MapT &&mapper)
{
    std::vector<std::remove_reference_t<decltype(mapper(*range.begin()))>> res;
    for (auto &el : range) {
        // sometimes size is not known
        res.push_back(mapper(el));  // NOLINT(performance-inefficient-vector-operation)
    }
    return res;
}

class CodeInfoProducerAccumulatorTest : public ark::compiler::AsmTest {
public:
    void SetUp() override
    {
        AsmTest::SetUp();
        ark::compiler::g_options.SetCompilerNonOptimizing(true);
    }

protected:
    void BuildCodeInfoFromSource(const char *source, const char *methodName)
    {
        ASSERT_TRUE(ParseToGraph<true>(source, methodName));

        auto *graph = GetGraph();
        auto *method = static_cast<Method *>(graph->GetMethod());
        ASSERT_NE(method, nullptr);

        ark::compiler::LLVMAotBuilder aotBuilder;
        aotBuilder.SetArch(graph->GetArch());
        aotBuilder.SetRuntime(graph->GetRuntime());

        uintptr_t codeAddress = aotBuilder.GetCurrentCodeAddress();
        auto *aotData = graph->GetAllocator()->New<ark::compiler::AotData>(
            ark::compiler::AotData::AotDataArgs {nullptr,
                                                 graph,
                                                 nullptr,
                                                 codeAddress,
                                                 aotBuilder.GetIntfInlineCacheIndex(),
                                                 {aotBuilder.GetGotPlt(), aotBuilder.GetGotVirtIndexes(),
                                                  aotBuilder.GetGotClass(), aotBuilder.GetGotString()},
                                                 {aotBuilder.GetGotIntfInlineCache(), aotBuilder.GetGotCommon()}});
        aotData->SetUseCha(true);
        graph->SetAotData(aotData);

        LLVMAotCompiler llvm(graph->GetRuntime(), graph->GetAllocator(), &aotBuilder, "", "code-info-acc.abc");
        auto res = llvm.TryAddGraph(graph);
        ASSERT_TRUE(res.HasValue()) << res.Error();
        ASSERT_TRUE(res.Value());
        ASSERT_FALSE(llvm.IsIrFailed());
        auto *pfile = method->GetPandaFile();
        ASSERT_NE(pfile, nullptr);
        auto *klass = method->GetClass();
        ASSERT_NE(klass, nullptr);

        aotBuilder.StartFile(pfile->GetFilename(), pfile->GetHeader()->checksum);
        aotBuilder.AddClassHashTable(*pfile);
        aotBuilder.StartClass(*klass);
        aotBuilder.AddMethodHeader(method, 0);
        aotBuilder.EndClass();
        aotBuilder.EndFile();

        llvm.FinishCompile();

        const auto &methods = aotBuilder.GetMethods();
        auto it = std::find_if(methods.begin(), methods.end(),
                               [method](const auto &compiledMethod) { return compiledMethod.GetMethod() == method; });
        ASSERT_NE(it, methods.end());
        ASSERT_FALSE(it->GetCodeInfo().empty());

        codeInfoData_.assign(it->GetCodeInfo().begin(), it->GetCodeInfo().end());
        codeInfo_.Decode(ark::Span<const uint8_t>(codeInfoData_.data(), codeInfoData_.size()));
    }

    void CheckFirstGeneratedStackMapVRegs(std::initializer_list<ark::compiler::VRegInfo> expectedVregs)
    {
        auto &stackMaps = codeInfo_.GetStackMaps();
        auto itStackMap =
            std::find_if(stackMaps.begin(), stackMaps.end(), [](auto &stackMap) { return stackMap.HasRegMap(); });
        ASSERT_NE(itStackMap, stackMaps.end());

        auto actualVregs = MapVector(
            FilterVector(codeInfo_.GetVRegList(*itStackMap, GetAllocator()), [](auto &vreg) { return vreg.IsLive(); }),
            [](auto vreg) {
                vreg.SetValue(0);
                return vreg;
            });
        EXPECT_FALSE(actualVregs.empty());
        EXPECT_THAT(actualVregs, ::testing::ElementsAreArray(expectedVregs));
    }

private:
    ark::compiler::CodeInfo codeInfo_;
    std::vector<uint8_t> codeInfoData_;
};

TEST_F(CodeInfoProducerAccumulatorTest, CheckCastNullFirstGeneratedStackMapHasAccumulator)
{
    auto source = R"(
.language PandaAssembly

.record R1 {
}

.record R2 {
    i32 s_f <static>
}

.function void R1._cctor_() <cctor, static> {
    ldai 0xa
    ststatic R2.s_f
    return.void
}

.function i32 main() <static> {
    initobj.short R2._ctor_:(R2)
try_begin_label_0:
    checkcast R1
    ldai 0x2
    return
try_end_label_0:
    ldstatic R2.s_f
    movi v0, 0x0
    jne v0, jump_label_2
    ldai 0x0
    return
jump_label_2:
    ldai 0x1
    return

.catchall try_begin_label_0, try_end_label_0, try_end_label_0
}

.function void R2._ctor_(R2 a0) <ctor> {
    return.void
}
)";

    BuildCodeInfoFromSource(source, "main");
    CheckFirstGeneratedStackMapVRegs(
        {compiler::VRegInfo(0U, compiler::VRegInfo::Location::SLOT, compiler::VRegInfo::Type::OBJECT,
                            compiler::VRegInfo::VRegType::ACC, 1U)});
}

TEST_F(CodeInfoProducerAccumulatorTest, LenArrayNullFirstGeneratedStackMapHasAccumulator)
{
    auto source = R"(
.language PandaAssembly

.record panda.NullPointerException <external>

.record panda.String <external>

.function i32 main() <static> {
    mov.null v0
try_begin_label_0:
    lenarr v0
    ldai 0x2
try_end_label_0:
    return
handler_begin_label_0_0:
    sta.obj v0
    call.virt.short panda.NullPointerException.getMessage:(panda.NullPointerException), v0
    jnez.obj jump_label_4
    ldai 0x0
    return
handler_begin_label_0_1:
    ldai 0x1
    return
jump_label_4:
    ldai 0x1
    return

.catch panda.NullPointerException, try_begin_label_0, try_end_label_0, handler_begin_label_0_0
.catchall try_begin_label_0, try_end_label_0, handler_begin_label_0_1
}

.function panda.String panda.NullPointerException.getMessage(panda.NullPointerException a0) <external>
)";

    BuildCodeInfoFromSource(source, "main");
    CheckFirstGeneratedStackMapVRegs(
        {compiler::VRegInfo(0U, compiler::VRegInfo::Location::CONSTANT, compiler::VRegInfo::Type::OBJECT,
                            compiler::VRegInfo::VRegType::VREG, 0U)});
}
// NOLINTEND(readability-magic-numbers)
}  // namespace
}  // namespace ark::llvmbackend
