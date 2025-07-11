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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class CreateVMTest : public ::testing::Test {
public:
    std::string BuildOptionString()
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        if (stdlib == nullptr) {
            return "";
        }
        const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
        std::string bootFileString = "--ext:--boot-panda-files=" + std::string(stdlib);

        if (abcPath != nullptr) {
            bootFileString += ":" + std::string(abcPath);
        }
        return bootFileString;
    }
};

TEST_F(CreateVMTest, create_vm_and_destroy_vm)
{
    ani_vm *vm = nullptr;
    std::string bootFileString = BuildOptionString();
    ani_option bootFileOption = {bootFileString.data(), nullptr};
    std::vector<ani_option> options;
    options.push_back(bootFileOption);
    ani_options optionsPtr = {options.size(), options.data()};

    const int32_t versionNew = 2;
    const int32_t versionOld = 0;
    ASSERT_EQ(ANI_CreateVM(nullptr, ANI_VERSION_1, &vm), ANI_INVALID_ARGS);
    ASSERT_EQ(ANI_CreateVM(&optionsPtr, versionNew, &vm), ANI_INVALID_VERSION);
    ASSERT_EQ(ANI_CreateVM(&optionsPtr, versionOld, &vm), ANI_INVALID_VERSION);
    ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm), ANI_OK);
    ASSERT_EQ(vm->c_api->DestroyVM(nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(vm->DestroyVM(), ANI_OK);
}

}  // namespace ark::ets::ani::testing
