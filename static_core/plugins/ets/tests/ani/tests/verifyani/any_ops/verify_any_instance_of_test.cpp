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

class AnyInstanceOfTest : public VerifyAniTest {};

TEST_F(AnyInstanceOfTest, wrong_env)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Any_InstanceOf(nullptr, cls, cls, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"ref", "ani_ref", "Static types are not supported"},
        {"type", "ani_ref", "Static types are not supported"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_InstanceOf", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyInstanceOfTest, wrong_ref_null)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Any_InstanceOf(env_, nullptr, cls, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "reference is nullptr"},
        {"type", "ani_ref", "Static types are not supported"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_InstanceOf", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyInstanceOfTest, wrong_type_null)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Any_InstanceOf(env_, cls, nullptr, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "Static types are not supported"},
        {"type", "ani_ref", "reference is nullptr"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_InstanceOf", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyInstanceOfTest, wrong_result_ptr)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ASSERT_EQ(env_->c_api->Any_InstanceOf(env_, cls, cls, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "Static types are not supported"},
        {"type", "ani_ref", "Static types are not supported"},
        {"result", "ani_boolean *", "nullptr for storing 'ani_boolean'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_InstanceOf", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyInstanceOfTest, rejects_non_xref_objects)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Any_InstanceOf(env_, cls, cls, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "Static types are not supported"},
        {"type", "ani_ref", "Static types are not supported"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_InstanceOf", testLines);

    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

TEST_F(AnyInstanceOfTest, throw_error)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);

    ThrowError();

    ani_boolean result {};
    ASSERT_EQ(env_->c_api->Any_InstanceOf(env_, cls, cls, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"ref", "ani_ref", "Static types are not supported"},
        {"type", "ani_ref", "Static types are not supported"},
        {"result", "ani_boolean *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Any_InstanceOf", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(cls), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
