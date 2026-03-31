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

class VerifyEnumGetItemByNameTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindEnum("verify_enum_get_item_by_name_test.Color", &enumColor_), ANI_OK);
    }

protected:
    ani_enum enumColor_ {};
};

static ani_object CreateObject(ani_env *env)
{
    ani_class cls {};
    EXPECT_EQ(env->FindClass("std.core.Object", &cls), ANI_OK);
    ani_method ctor {};
    EXPECT_EQ(env->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object obj {};
    EXPECT_EQ(env->Object_New(cls, ctor, &obj), ANI_OK);
    return obj;
}

TEST_F(VerifyEnumGetItemByNameTest, wrong_env)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->c_api->Enum_GetEnumItemByName(nullptr, enumColor_, "RED", &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enm", "ani_enum"},
        {"name", "const char *"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
}

TEST_F(VerifyEnumGetItemByNameTest, wrong_enm_type_object)
{
    ani_object obj = CreateObject(env_);
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(static_cast<ani_enum>(obj), "RED", &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum", "wrong reference type: ani_object"},
        {"name", "const char *"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
}

TEST_F(VerifyEnumGetItemByNameTest, wrong_name_null)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColor_, nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum"},
        {"name", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
}

TEST_F(VerifyEnumGetItemByNameTest, wrong_name_not_found)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColor_, "PURPLE", &result), ANI_NOT_FOUND);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(VerifyEnumGetItemByNameTest, wrong_result_null)
{
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColor_, "RED", nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum"},
        {"name", "const char *"},
        {"result", "ani_enum_item *", "wrong pointer for storing 'ani_enum_item'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
}

TEST_F(VerifyEnumGetItemByNameTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Enum_GetEnumItemByName(nullptr, nullptr, nullptr, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enm", "ani_enum", "wrong reference"},
        {"name", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"result", "ani_enum_item *", "wrong pointer for storing 'ani_enum_item'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
}

TEST_F(VerifyEnumGetItemByNameTest, has_unhandled_error)
{
    ThrowError();
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColor_, "RED", &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"enm", "ani_enum"},
        {"name", "const char *"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyEnumGetItemByNameTest, has_unhandled_error_with_wrong_name_null)
{
    ThrowError();
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColor_, nullptr, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"enm", "ani_enum"},
        {"name", "const char *", "wrong pointer to use as argument in 'const char *'"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyEnumGetItemByNameTest, has_unhandled_error_with_wrong_enm_type_object)
{
    ani_object obj = CreateObject(env_);
    ThrowError();
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(static_cast<ani_enum>(obj), "RED", &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"enm", "ani_enum", "wrong reference type: ani_object"},
        {"name", "const char *"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyEnumGetItemByNameTest, wrong_enm_from_local_scope)
{
    ani_enum localEnum {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindEnum("verify_enum_get_item_by_name_test.Color", &localEnum), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(localEnum, "RED", &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum", "wrong reference"},
        {"name", "const char *"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
}

TEST_F(VerifyEnumGetItemByNameTest, cross_thread_enum_ref)
{
    ani_enum crossThreadEnum {};
    std::thread([this, &crossThreadEnum]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);
        ASSERT_EQ(env->FindEnum("verify_enum_get_item_by_name_test.Color", &crossThreadEnum), ANI_OK);
        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(crossThreadEnum, "RED", &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum", "wrong reference"},
        {"name", "const char *"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByName", testLines);
}

TEST_F(VerifyEnumGetItemByNameTest, success)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColor_, "GREEN", &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    ASSERT_NO_ABORT_MESSAGE();
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, misc-non-private-member-variables-in-classes)
