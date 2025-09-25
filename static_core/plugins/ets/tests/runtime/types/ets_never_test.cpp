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

#include <gtest/gtest.h>

#include "ets_coroutine.h"
#include "ets_vm.h"

#include "plugins/ets/runtime/types/ets_typeapi_field.h"
#include "plugins/ets/runtime/types/ets_typeapi_method.h"
#include "plugins/ets/runtime/types/ets_typeapi_parameter.h"
#include "plugins/ets/runtime/types/ets_typeapi_type.h"

namespace ark::ets::test {

class EtsNeverTypeAPITest : public testing::Test {
public:
    EtsNeverTypeAPITest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(true);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("g1-gc");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsNeverTypeAPITest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsNeverTypeAPITest);
    NO_MOVE_SEMANTIC(EtsNeverTypeAPITest);

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

TEST_F(EtsNeverTypeAPITest, NeverTypeBaseClass)
{
    auto *neverClass = vm_->GetClassLinker()->GetClass("LN;");
    ASSERT(neverClass != nullptr);
    ASSERT_EQ(neverClass->GetBase(), nullptr);
}

TEST_F(EtsNeverTypeAPITest, NeverTypeMethods)
{
    auto *neverClass = vm_->GetClassLinker()->GetClass("LN;");
    ASSERT(neverClass != nullptr);
    auto methods = neverClass->GetMethods();
    ASSERT_EQ(methods.size(), 0);
}

TEST_F(EtsNeverTypeAPITest, NeverTypeFields)
{
    auto *neverClass = vm_->GetClassLinker()->GetClass("LN;");
    ASSERT(neverClass != nullptr);
    auto fields = neverClass->GetRuntimeClass()->GetInstanceFields();
    ASSERT_EQ(fields.size(), 0);
}

}  // namespace ark::ets::test