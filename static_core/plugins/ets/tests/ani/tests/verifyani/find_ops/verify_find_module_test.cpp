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

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class FindModuleTest : public VerifyAniTest {};

TEST_F(FindModuleTest, find_module_success)
{
    ani_module result {};
    ASSERT_EQ(env_->FindModule("verify_find_module_test", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(FindModuleTest, find_module_wrong_descriptor)
{
    ani_module result {};
    ASSERT_EQ(env_->FindModule(nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> nullDescriptorLines {
        {"env", "ani_env *"},
        {"module_descriptor", "const char *", "argument is nullptr, expected const char * [ERROR]"},
        {"result", "ani_module *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindModule", nullDescriptorLines);

    ASSERT_EQ(env_->FindModule("Lverify_find_module_test;", &result), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"module_descriptor", "const char *", "invalid module descriptor [ERROR]"},
        {"result", "ani_module *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindModule", invalidDescriptorLines);
}

TEST_F(FindModuleTest, find_module_wrong_result_storage)
{
    ASSERT_EQ(env_->FindModule("verify_find_module_test", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> moduleLines {
        {"env", "ani_env *"},
        {"module_descriptor", "const char *"},
        {"result", "ani_module *", "nullptr for storing 'ani_module' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindModule", moduleLines);
}

}  // namespace ark::ets::ani::verify::testing
