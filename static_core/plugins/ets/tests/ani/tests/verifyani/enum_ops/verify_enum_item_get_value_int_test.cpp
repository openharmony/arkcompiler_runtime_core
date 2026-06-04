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

class VerifyEnumItemGetValueIntTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindEnum("verify_enum_item_get_value_int_test.ColorInt", &enumColorInt_), ANI_OK);
        ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColorInt_, "REDINT", &itemColorIntRed_), ANI_OK);
    }

protected:
    ani_enum enumColorInt_ {};
    ani_enum_item itemColorIntRed_ {};
};

TEST_F(VerifyEnumItemGetValueIntTest, wrong_env)
{
    ani_int result {};
    ASSERT_EQ(env_->c_api->EnumItem_GetValue_Int(nullptr, itemColorIntRed_, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enum_item", "ani_enum_item"},
        {"result", "ani_int *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
}

TEST_F(VerifyEnumItemGetValueIntTest, wrong_enum_item_null)
{
    ani_int result {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "reference is nullptr"},
        {"result", "ani_int *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
}

TEST_F(VerifyEnumItemGetValueIntTest, wrong_enum_item_type)
{
    ani_int result {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(reinterpret_cast<ani_enum_item>(enumColorInt_), &result), ANI_NOT_FOUND);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "wrong reference type: ani_enum"},
        {"result", "ani_int *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
}

TEST_F(VerifyEnumItemGetValueIntTest, wrong_result_null)
{
    ASSERT_EQ(env_->EnumItem_GetValue_Int(itemColorIntRed_, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item"},
        {"result", "ani_int *", "nullptr for storing 'ani_int'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
}

TEST_F(VerifyEnumItemGetValueIntTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->EnumItem_GetValue_Int(nullptr, nullptr, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enum_item", "ani_enum_item", "reference is nullptr"},
        {"result", "ani_int *", "nullptr for storing 'ani_int'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
}

TEST_F(VerifyEnumItemGetValueIntTest, has_unhandled_error)
{
    ThrowError();
    ani_int result {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(itemColorIntRed_, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"enum_item", "ani_enum_item"},
        {"result", "ani_int *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyEnumItemGetValueIntTest, wrong_enum_item_from_local_scope)
{
    ani_enum localEnum {};
    ani_enum_item localItem {};
    constexpr ani_size SCOPE_CAPACITY = 2;
    ASSERT_EQ(env_->CreateLocalScope(SCOPE_CAPACITY), ANI_OK);
    ASSERT_EQ(env_->FindEnum("verify_enum_item_get_value_int_test.ColorInt", &localEnum), ANI_OK);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(localEnum, "REDINT", &localItem), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_int result {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(localItem, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "reference not found (may be deleted, out of scope, or corrupted)"},
        {"result", "ani_int *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
}

TEST_F(VerifyEnumItemGetValueIntTest, cross_thread_enum_item_ref)
{
    ani_enum_item crossThreadItem {};
    std::thread([this, &crossThreadItem]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);
        ani_enum enm {};
        ASSERT_EQ(env->FindEnum("verify_enum_item_get_value_int_test.ColorInt", &enm), ANI_OK);
        ASSERT_EQ(env->Enum_GetEnumItemByName(enm, "REDINT", &crossThreadItem), ANI_OK);
        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_int result {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(crossThreadItem, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "reference not found (may be deleted, out of scope, or corrupted)"},
        {"result", "ani_int *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetValue_Int", testLines);
}

TEST_F(VerifyEnumItemGetValueIntTest, success)
{
    ani_int result {};
    ASSERT_EQ(env_->EnumItem_GetValue_Int(itemColorIntRed_, &result), ANI_OK);
    ani_int expectVal = 5;
    ASSERT_EQ(result, expectVal);
    ASSERT_NO_ABORT_MESSAGE();
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, misc-non-private-member-variables-in-classes)
