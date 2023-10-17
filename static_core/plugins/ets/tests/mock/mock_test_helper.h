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

#ifndef PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H_
#define PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"

namespace panda::ets::test {

class MockEtsNapiTestBaseClass : public testing::Test {
protected:
    MockEtsNapiTestBaseClass() = default;
    explicit MockEtsNapiTestBaseClass(const char *file_name) : test_bin_file_name_(file_name) {};

    static void SetUpTestCase()
    {
        if (std::getenv("PANDA_STD_LIB") == nullptr) {
            std::cerr << "PANDA_STD_LIB not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        std::vector<EtsVMOption> options_vector;

        options_vector = {{EtsOptionType::EtsGcType, "epsilon"},
                          {EtsOptionType::EtsNoJit, nullptr},
                          {EtsOptionType::EtsBootFile, std::getenv("PANDA_STD_LIB")}};

        if (test_bin_file_name_ != nullptr) {
            options_vector.push_back({EtsOptionType::EtsBootFile, test_bin_file_name_});
        }

        EtsVMInitArgs vm_args;
        vm_args.version = ETS_NAPI_VERSION_1_0;
        vm_args.options = options_vector.data();
        vm_args.nOptions = static_cast<ets_int>(options_vector.size());

        ASSERT_TRUE(ETS_CreateVM(&vm_, &env_, &vm_args) == ETS_OK) << "Cannot create ETS VM";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm_->DestroyEtsVM() == ETS_OK) << "Cannot destroy ETS VM";
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    const char *test_bin_file_name_ {nullptr};
    EtsEnv *env_ {nullptr};
    EtsVM *vm_ {nullptr};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}  // namespace panda::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_MOCK_MOCK_TEST_HELPER_H_