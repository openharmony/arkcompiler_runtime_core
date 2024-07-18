/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H
#define PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"

namespace ark::ets::test {

class MockEtsNapiTestBaseClass : public testing::Test {
protected:
    MockEtsNapiTestBaseClass() = default;
    explicit MockEtsNapiTestBaseClass(const char *fileName) : testBinFileName_(fileName) {};

    static void SetUpTestCase()
    {
        if (std::getenv("PANDA_STD_LIB") == nullptr) {
            std::cerr << "PANDA_STD_LIB not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        std::vector<EtsVMOption> optionsVector;

        optionsVector = {{EtsOptionType::ETS_GC_TYPE, "epsilon"},
                         {EtsOptionType::ETS_NO_JIT, nullptr},
                         {EtsOptionType::ETS_BOOT_FILE, std::getenv("PANDA_STD_LIB")}};

        if (testBinFileName_ != nullptr) {
            optionsVector.push_back({EtsOptionType::ETS_BOOT_FILE, testBinFileName_});
        }

        EtsVMInitArgs vmArgs;
        vmArgs.version = ETS_NAPI_VERSION_1_0;
        vmArgs.options = optionsVector.data();
        vmArgs.nOptions = static_cast<ets_int>(optionsVector.size());

        ASSERT_TRUE(ETS_CreateVM(&vm_, &env_, &vmArgs) == ETS_OK) << "Cannot create ETS VM";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm_->DestroyEtsVM() == ETS_OK) << "Cannot destroy ETS VM";
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    const char *testBinFileName_ {nullptr};
    EtsEnv *env_ {nullptr};
    EtsVM *vm_ {nullptr};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H
