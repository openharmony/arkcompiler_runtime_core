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

#include <thread>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, misc-non-private-member-variables-in-classes)
namespace ark::ets::ani::verify::testing {

class VerifyFindEnumTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
    }
};

TEST_F(VerifyFindEnumTest, wrong_env)
{
    ani_enum result {};
    ASSERT_EQ(env_->c_api->FindEnum(nullptr, "verify_find_enum_test.Color", &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enum_descriptor", "const char *"},
        {"result", "ani_enum *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
}

TEST_F(VerifyFindEnumTest, wrong_enum_descriptor_null)
{
    ani_enum result {};
    ASSERT_EQ(env_->FindEnum(nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_descriptor", "const char *", "argument is nullptr, expected const char *"},
        {"result", "ani_enum *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
}

TEST_F(VerifyFindEnumTest, wrong_enum_descriptor_invalid)
{
    ani_enum result {};
    ASSERT_EQ(env_->FindEnum("Lverify_find_enum_test/Color;", &result), ANI_INVALID_DESCRIPTOR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_descriptor", "const char *", "enum descriptor is invalid"},
        {"result", "ani_enum *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
}

TEST_F(VerifyFindEnumTest, wrong_enum_descriptor_not_found)
{
    ani_enum result {};
    ASSERT_EQ(env_->FindEnum("verify_find_enum_test.Color0000", &result), ANI_NOT_FOUND);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(VerifyFindEnumTest, wrong_enum_descriptor_not_enum_0)
{
    ani_enum result {};
    ASSERT_EQ(env_->FindEnum("std.core.Int", &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_descriptor", "const char *", "descriptor is not enum"},
        {"result", "ani_enum *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
}

TEST_F(VerifyFindEnumTest, wrong_result)
{
    ASSERT_EQ(env_->FindEnum("verify_find_enum_test.Color", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_descriptor", "const char *"},
        {"result", "ani_enum *", "nullptr for storing 'ani_enum'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
}

TEST_F(VerifyFindEnumTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->FindEnum(nullptr, nullptr, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enum_descriptor", "const char *", "argument is nullptr, expected const char *"},
        {"result", "ani_enum *", "nullptr for storing 'ani_enum'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
}

TEST_F(VerifyFindEnumTest, has_unhandled_error)
{
    ThrowError();
    ani_enum result {};
    ASSERT_EQ(env_->FindEnum("verify_find_enum_test.Color", &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"enum_descriptor", "const char *"},
        {"result", "ani_enum *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyFindEnumTest, has_unhandled_error_with_non_enum_descriptor)
{
    ThrowError();
    ani_enum result {};
    ASSERT_EQ(env_->FindEnum("std.core.Int", &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"enum_descriptor", "const char *", "descriptor is not enum"},
        {"result", "ani_enum *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyFindEnumTest, cross_thread_call_with_other_thread_env)
{
    ani_status status = ANI_OK;
    std::thread([this, &status]() {
        ani_env *attachedEnv {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &attachedEnv), ANI_OK);
        ani_enum result {};
        status = env_->FindEnum("verify_find_enum_test.Color", &result);
        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ASSERT_EQ(status, ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enum_descriptor", "const char *"},
        {"result", "ani_enum *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("FindEnum", testLines);
}

TEST_F(VerifyFindEnumTest, success)
{
    ani_enum result {};
    ASSERT_EQ(env_->FindEnum("verify_find_enum_test.Color", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    ASSERT_NO_ABORT_MESSAGE();
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, misc-non-private-member-variables-in-classes)
