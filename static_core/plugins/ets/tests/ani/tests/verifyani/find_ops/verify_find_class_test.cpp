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

class FindClassTest : public VerifyAniTest {};

TEST_F(FindClassTest, find_class_success)
{
    ani_class result {};
    ASSERT_EQ(env_->FindClass("verify_find_class_test.TestClass", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(FindClassTest, find_class_wrong_descriptor)
{
    ani_class result {};
    ASSERT_EQ(env_->FindClass(nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> nullDescriptorLines {
        {"env", "ani_env *"},
        {"class_descriptor", "const char *", "argument is nullptr, expected const char * [ERROR]"},
        {"result", "ani_class *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindClass", nullDescriptorLines);

    ASSERT_EQ(env_->FindClass("Lverify_find_class_test/TestClass;", &result), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> invalidDescriptorLines {
        {"env", "ani_env *"},
        {"class_descriptor", "const char *", "invalid class descriptor [ERROR]"},
        {"result", "ani_class *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindClass", invalidDescriptorLines);
}

TEST_F(FindClassTest, find_class_wrong_result_storage)
{
    ASSERT_EQ(env_->FindClass("verify_find_class_test.TestClass", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> classLines {
        {"env", "ani_env *"},
        {"class_descriptor", "const char *"},
        {"result", "ani_class *", "nullptr for storing 'ani_class' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindClass", classLines);
}

TEST_F(FindClassTest, pending_error_is_rejected)
{
    ThrowError();
    ani_class result {};
    ASSERT_EQ(env_->FindClass("verify_find_class_test.TestClass", &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error [ERROR]"},
        {"class_descriptor", "const char *"},
        {"result", "ani_class *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindClass", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
