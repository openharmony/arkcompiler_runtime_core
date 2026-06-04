/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

class TypeIsAssignableFromTest : public VerifyAniTest {};

TEST_F(TypeIsAssignableFromTest, wrong_env)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Type_IsAssignableFrom(nullptr, cls, cls, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"from_type", "ani_ref"},
        {"to_type", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_IsAssignableFrom", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(TypeIsAssignableFromTest, wrong_from_type_null)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Type_IsAssignableFrom(env_, nullptr, cls, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"from_type", "ani_ref", "reference is nullptr"},
        {"to_type", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_IsAssignableFrom", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(TypeIsAssignableFromTest, wrong_from_type_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Type_IsAssignableFrom(env_, reinterpret_cast<ani_type>(undef), cls, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"from_type", "ani_ref", "wrong reference type: undefined"},
        {"to_type", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_IsAssignableFrom", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
}

TEST_F(TypeIsAssignableFromTest, wrong_to_type_null)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Type_IsAssignableFrom(env_, cls, nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"from_type", "ani_ref"},
        {"to_type", "ani_ref", "reference is nullptr"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_IsAssignableFrom", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(TypeIsAssignableFromTest, wrong_to_type_type)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Type_IsAssignableFrom(env_, cls, reinterpret_cast<ani_type>(undef), &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"from_type", "ani_ref"},
        {"to_type", "ani_ref", "wrong reference type: undefined"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_IsAssignableFrom", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
}

TEST_F(TypeIsAssignableFromTest, wrong_result_ptr)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Type_IsAssignableFrom(env_, cls, cls, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"from_type", "ani_ref"},
        {"to_type", "ani_ref"},
        {"result", "ani_boolean *", "nullptr for storing 'ani_boolean'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_IsAssignableFrom", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(TypeIsAssignableFromTest, success)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->Type_IsAssignableFrom(cls, cls, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(TypeIsAssignableFromTest, throw_error)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ThrowError();

    ani_boolean result {};
    ASSERT_EQ(env_->Type_IsAssignableFrom(cls, cls, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"from_type", "ani_ref"},
        {"to_type", "ani_ref"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_IsAssignableFrom", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
