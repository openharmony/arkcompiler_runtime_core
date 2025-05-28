/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "utils/utf.h"

namespace ark::ets::test {

class EtsUnionTest : public testing::Test {
public:
    EtsUnionTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsUnionTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsUnionTest);
    NO_MOVE_SEMANTIC(EtsUnionTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        vm_ = coroutine_->GetPandaVM();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsUnionTest, CreateUnionClass)
{
    pandasm::Parser p;

    const char *source =
        ".language eTS\n"
        ".record std.core.Object <external>\n"
        ".record A <ets.extends=std.core.Object, access.record=public> {}\n"
        ".record B <ets.extends=A, access.record=public> {}\n"
        ".record C <ets.extends=A, access.record=public> {}\n"
        ".record D <ets.extends=std.core.Object, access.record=public> {}\n"

        ".record {UA,B} <external>\n"
        ".record {UB,C} <external>\n"
        ".record {UA,B,C} <external>\n"
        ".record {UA,D} <external>\n"
        ".record {UA,{UB,C}[],D} <external>";

    auto res = p.Parse(source);
    ASSERT_EQ(p.ShowError().err, pandasm::Error::ErrorType::ERR_NONE);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    ASSERT(classLinker);
    classLinker->AddPandaFile(std::move(pf));

    EtsClassLinker *etsClassLinker = vm_->GetClassLinker();

    std::map<PandaString, PandaString> descMap {{"{ULA;LB;}", "LA;"},
                                                {"{ULB;LC;}", "LA;"},
                                                {"{ULA;LB;LC;}", "LA;"},
                                                {"{ULA;LB;[LC;}", "Lstd/core/Object;"},
                                                {"{ULA;LD;[{ULB;LC;}}", "Lstd/core/Object;"}};

    for (const auto &[descFull, descLUB] : descMap) {
        EtsClass *klass = etsClassLinker->GetClass(descFull.c_str());
        ASSERT_NE(klass, nullptr);
        EXPECT_EQ(PandaString(klass->GetDescriptor()), descLUB);
    }
}

}  // namespace ark::ets::test
