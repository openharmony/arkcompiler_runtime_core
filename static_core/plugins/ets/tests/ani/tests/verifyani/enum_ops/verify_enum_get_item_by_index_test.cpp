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

#include <limits>
#include <string_view>
#include <thread>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, misc-non-private-member-variables-in-classes)
namespace ark::ets::ani::verify::testing {

class VerifyEnumGetItemByIndexTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindEnum("verify_enum_get_item_by_index_test.Color", &enumColor_), ANI_OK);
    }

protected:
    ani_enum enumColor_ {};
};

static ani_string CreateString(ani_env *env)
{
    ani_string str {};
    constexpr std::string_view VAL = "enum";
    EXPECT_EQ(env->String_NewUTF8(VAL.data(), VAL.size(), &str), ANI_OK);
    return str;
}

TEST_F(VerifyEnumGetItemByIndexTest, wrong_env)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->c_api->Enum_GetEnumItemByIndex(nullptr, enumColor_, 0U, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"enm", "ani_enum"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
}

TEST_F(VerifyEnumGetItemByIndexTest, wrong_enm_null)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(nullptr, 0U, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum", "reference is nullptr [ERROR]"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
}

TEST_F(VerifyEnumGetItemByIndexTest, wrong_enm_type)
{
    ani_string str = CreateString(env_);
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(reinterpret_cast<ani_enum>(str), 0U, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum", "wrong reference type: ani_string [FATAL]"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
}

TEST_F(VerifyEnumGetItemByIndexTest, wrong_result_null)
{
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(enumColor_, 0U, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *", "nullptr for storing 'ani_enum_item' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
}

TEST_F(VerifyEnumGetItemByIndexTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Enum_GetEnumItemByIndex(nullptr, nullptr, 0U, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"enm", "ani_enum", "reference is nullptr [ERROR]"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *", "nullptr for storing 'ani_enum_item' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
}

TEST_F(VerifyEnumGetItemByIndexTest, out_of_range_index)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(enumColor_, std::numeric_limits<ani_size>::max(), &result), ANI_NOT_FOUND);
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(enumColor_, -1, &result), ANI_NOT_FOUND);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(VerifyEnumGetItemByIndexTest, has_unhandled_error)
{
    ThrowError();
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(enumColor_, 0U, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"enm", "ani_enum"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyEnumGetItemByIndexTest, wrong_enm_from_local_scope)
{
    ani_enum localEnum {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindEnum("verify_enum_get_item_by_index_test.Color", &localEnum), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(localEnum, 0U, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum", "reference not found (may be deleted, out of scope, or corrupted) [FATAL]"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
}

TEST_F(VerifyEnumGetItemByIndexTest, cross_thread_enum_ref)
{
    ani_enum crossThreadEnum {};
    std::thread([this, &crossThreadEnum]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);
        ASSERT_EQ(env->FindEnum("verify_enum_get_item_by_index_test.Color", &crossThreadEnum), ANI_OK);
        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(crossThreadEnum, 0U, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enm", "ani_enum", "reference not found (may be deleted, out of scope, or corrupted) [FATAL]"},
        {"index", "ani_size"},
        {"result", "ani_enum_item *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Enum_GetEnumItemByIndex", testLines);
}

TEST_F(VerifyEnumGetItemByIndexTest, success)
{
    ani_enum_item result {};
    ASSERT_EQ(env_->Enum_GetEnumItemByIndex(enumColor_, 2U, &result), ANI_OK);
    ASSERT_NE(result, nullptr);
    ASSERT_NO_ABORT_MESSAGE();
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, misc-non-private-member-variables-in-classes)
