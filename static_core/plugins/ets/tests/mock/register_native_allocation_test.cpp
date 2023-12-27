/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "plugins/ets/tests/mock/calling_methods_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace panda::ets::test {

static const char *g_testBinFileName = "RegisterNativeAllocationTest.abc";

class CallingMethodsTestGeneral : public CallingMethodsTestBase {
public:
    CallingMethodsTestGeneral() : CallingMethodsTestBase(g_testBinFileName) {}
};

class RegisterNativeAllocationTest : public CallingMethodsTestGeneral {
    void SetUp() override
    {
        std::vector<EtsVMOption> optionsVector;

        optionsVector = {{EtsOptionType::EtsGcType, "g1-gc"},
                         {EtsOptionType::EtsRunGcInPlace, nullptr},
                         {EtsOptionType::EtsNoJit, nullptr},
                         {EtsOptionType::EtsBootFile, std::getenv("PANDA_STD_LIB")}};

        if (testBinFileName_ != nullptr) {
            optionsVector.push_back({EtsOptionType::EtsBootFile, testBinFileName_});
        }

        EtsVMInitArgs vmArgs;
        vmArgs.version = ETS_NAPI_VERSION_1_0;
        vmArgs.options = optionsVector.data();
        vmArgs.nOptions = static_cast<ets_int>(optionsVector.size());

        ASSERT_TRUE(ETS_CreateVM(&vm_, &env_, &vmArgs) == ETS_OK) << "Cannot create ETS VM";
    }
};

TEST_F(RegisterNativeAllocationTest, testNativeAllocation)
{
    mem::MemStatsType *memStats = Thread::GetCurrent()->GetVM()->GetMemStats();

    ets_class testClass = env_->FindClass("NativeAllocationTest");
    ASSERT_NE(testClass, nullptr);

    ets_method allocMethod = env_->GetStaticp_method(testClass, "allocate_object", ":I");
    ASSERT_NE(allocMethod, nullptr);
    ASSERT_EQ(env_->CallStaticIntMethod(testClass, allocMethod), 0);

    size_t heapFreedBeforeMethod;
    {
        panda::ets::napi::ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env_));
        heapFreedBeforeMethod = memStats->GetFreedHeap();
    }

    ets_method mainMethod = env_->GetStaticp_method(testClass, "main_method", ":I");
    ASSERT_NE(mainMethod, nullptr);
    ASSERT_EQ(env_->CallStaticIntMethod(testClass, mainMethod), 0);

    size_t heapFreedAfterMethod;
    {
        panda::ets::napi::ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env_));
        heapFreedAfterMethod = memStats->GetFreedHeap();
    }

    ASSERT_GT(heapFreedAfterMethod, heapFreedBeforeMethod);
}

}  // namespace panda::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)