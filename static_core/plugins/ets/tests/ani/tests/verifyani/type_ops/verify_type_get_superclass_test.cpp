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

class TypeGetSuperClassTest : public VerifyAniTest {};

TEST_F(TypeGetSuperClassTest, wrong_env)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_class result {};
    ASSERT_EQ(env_->c_api->Type_GetSuperClass(nullptr, cls, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope [ERROR]"},
        {"type", "ani_ref"},
        {"result", "ani_class *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_GetSuperClass", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(TypeGetSuperClassTest, wrong_type_null)
{
    ani_class result {};
    ASSERT_EQ(env_->c_api->Type_GetSuperClass(env_, nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"type", "ani_ref", "reference is nullptr [ERROR]"},
        {"result", "ani_class *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_GetSuperClass", testLines);
}

TEST_F(TypeGetSuperClassTest, wrong_type_type)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);

    ani_class result {};
    ASSERT_EQ(env_->c_api->Type_GetSuperClass(env_, reinterpret_cast<ani_type>(undef), &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"type", "ani_ref", "wrong reference type: undefined [FATAL]"},
        {"result", "ani_class *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_GetSuperClass", testLines);

    ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
}

TEST_F(TypeGetSuperClassTest, wrong_result_ptr)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Type_GetSuperClass(env_, cls, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"type", "ani_ref"},
        {"result", "ani_class *", "nullptr for storing 'ani_class' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_GetSuperClass", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(TypeGetSuperClassTest, success)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_class superClass {};
    ASSERT_EQ(env_->Type_GetSuperClass(cls, &superClass), ANI_OK);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
    if (superClass != nullptr) {
        ASSERT_EQ(env_->Reference_Delete(superClass), ANI_OK);
    }
}

TEST_F(TypeGetSuperClassTest, throw_error)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ThrowError();

    ani_class superClass {};
    ASSERT_EQ(env_->Type_GetSuperClass(cls, &superClass), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error [ERROR]"},
        {"type", "ani_ref"},
        {"result", "ani_class *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Type_GetSuperClass", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
