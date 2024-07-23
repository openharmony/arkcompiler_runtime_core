/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <iostream>
#include <ostream>

#include <gtest/gtest.h>

#include "libpandabase/utils/logger.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/class.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/object_header.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"

namespace ark::ets::test {

class EtsVMInitPreallocTest : public testing::Test {
public:
    EtsVMInitPreallocTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(false);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);

        Runtime::Create(options);
    }

    ~EtsVMInitPreallocTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsVMInitPreallocTest);
    NO_MOVE_SEMANTIC(EtsVMInitPreallocTest);
};

TEST_F(EtsVMInitPreallocTest, PreallocatedOOMObjectTest)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    auto etsVm = coroutine->GetPandaVM();
    auto oomObj = etsVm->GetOOMErrorObject();

    ASSERT_NE(oomObj, nullptr);

    auto oomClass = oomObj->ClassAddr<ark::Class>();
    ASSERT_NE(oomClass->GetDescriptor(), nullptr);

    auto ctx = coroutine->GetLanguageContext();
    auto descriptorGot = utf::Mutf8AsCString(oomClass->GetDescriptor());
    auto descriptorExp = utf::Mutf8AsCString(ctx.GetOutOfMemoryErrorClassDescriptor());
    ASSERT_STREQ(descriptorGot, descriptorExp);
}

}  // namespace ark::ets::test
