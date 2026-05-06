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

class ObjectGetTypeTest : public VerifyAniTest {};

TEST_F(ObjectGetTypeTest, wrong_env)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_type resultType {};
    ASSERT_EQ(env_->c_api->Object_GetType(nullptr, cls, &resultType), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"object", "ani_object"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetType", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(ObjectGetTypeTest, wrong_object_null)
{
    ani_type resultType {};
    ASSERT_EQ(env_->c_api->Object_GetType(env_, nullptr, &resultType), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetType", testLines);
}

TEST_F(ObjectGetTypeTest, wrong_result_ptr)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Object_GetType(env_, cls, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object"},
        {"result", "ani_ref *", "wrong pointer for storing 'ani_type'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetType", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(ObjectGetTypeTest, wrong_object_type)
{
    ani_ref undef {};
    ASSERT_EQ(env_->GetUndefined(&undef), ANI_OK);
    ani_type resultType {};

    ASSERT_EQ(env_->c_api->Object_GetType(env_, reinterpret_cast<ani_object>(undef), &resultType), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"object", "ani_object", "wrong reference type: undefined"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetType", testLines);

    ASSERT_EQ(env_->Reference_Delete(undef), ANI_OK);
}

TEST_F(ObjectGetTypeTest, success)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_type resultType {};
    ASSERT_EQ(env_->Object_GetType(cls, &resultType), ANI_OK);
    ASSERT_NE(resultType, nullptr);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(resultType), ANI_OK);
}

TEST_F(ObjectGetTypeTest, throw_error)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ThrowError();

    ani_type resultType {};
    ASSERT_EQ(env_->Object_GetType(cls, &resultType), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"object", "ani_object"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Object_GetType", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing