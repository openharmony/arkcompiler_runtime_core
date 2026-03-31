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

class VerifyEnumItemGetIndexTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();
        ASSERT_EQ(env_->FindEnum("verify_enum_item_get_index_test.Color", &enumColor_), ANI_OK);
        ASSERT_EQ(env_->Enum_GetEnumItemByName(enumColor_, "RED", &itemColorRed_), ANI_OK);
    }

protected:
    ani_enum enumColor_ {};
    ani_enum_item itemColorRed_ {};
};

TEST_F(VerifyEnumItemGetIndexTest, wrong_env)
{
    ani_size result {};
    ASSERT_EQ(env_->c_api->EnumItem_GetIndex(nullptr, itemColorRed_, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enum_item", "ani_enum_item"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
}

TEST_F(VerifyEnumItemGetIndexTest, wrong_enum_item_null)
{
    ani_size result {};
    ASSERT_EQ(env_->EnumItem_GetIndex(nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "wrong reference"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
}

TEST_F(VerifyEnumItemGetIndexTest, wrong_enum_item_type)
{
    ani_size result {};
    ASSERT_EQ(env_->EnumItem_GetIndex(reinterpret_cast<ani_enum_item>(enumColor_), &result), ANI_INVALID_TYPE);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "wrong reference type: ani_enum"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
}

TEST_F(VerifyEnumItemGetIndexTest, wrong_result_null)
{
    ASSERT_EQ(env_->EnumItem_GetIndex(itemColorRed_, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
}

TEST_F(VerifyEnumItemGetIndexTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->EnumItem_GetIndex(nullptr, nullptr, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"enum_item", "ani_enum_item", "wrong reference"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
}

TEST_F(VerifyEnumItemGetIndexTest, has_unhandled_error)
{
    ThrowError();
    ani_size result {};
    ASSERT_EQ(env_->EnumItem_GetIndex(itemColorRed_, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"enum_item", "ani_enum_item"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(VerifyEnumItemGetIndexTest, wrong_enum_item_from_local_scope)
{
    ani_enum localEnum {};
    ani_enum_item localItem {};
    ASSERT_EQ(env_->CreateLocalScope(1), ANI_OK);
    ASSERT_EQ(env_->FindEnum("verify_enum_item_get_index_test.Color", &localEnum), ANI_OK);
    ASSERT_EQ(env_->Enum_GetEnumItemByName(localEnum, "RED", &localItem), ANI_OK);
    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_size result {};
    ASSERT_EQ(env_->EnumItem_GetIndex(localItem, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "wrong reference"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
}

TEST_F(VerifyEnumItemGetIndexTest, cross_thread_enum_item_ref)
{
    ani_enum_item crossThreadItem {};
    std::thread([this, &crossThreadItem]() {
        ani_env *env {};
        ASSERT_EQ(vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env), ANI_OK);
        ani_enum enm {};
        ASSERT_EQ(env->FindEnum("verify_enum_item_get_index_test.Color", &enm), ANI_OK);
        ASSERT_EQ(env->Enum_GetEnumItemByName(enm, "RED", &crossThreadItem), ANI_OK);
        ASSERT_EQ(vm_->DetachCurrentThread(), ANI_OK);
    }).join();

    ani_size result {};
    ASSERT_EQ(env_->EnumItem_GetIndex(crossThreadItem, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"enum_item", "ani_enum_item", "wrong reference"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("EnumItem_GetIndex", testLines);
}

TEST_F(VerifyEnumItemGetIndexTest, success)
{
    ani_size result {};
    ASSERT_EQ(env_->EnumItem_GetIndex(itemColorRed_, &result), ANI_OK);
    ASSERT_EQ(result, 0U);
    ASSERT_NO_ABORT_MESSAGE();
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, misc-non-private-member-variables-in-classes)
