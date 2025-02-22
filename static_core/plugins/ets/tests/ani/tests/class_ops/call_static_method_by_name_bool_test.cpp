/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class CallStaticClassTest : public AniTest {
public:
    void GetClassData(ani_class *clsResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
};

TEST_F(CallStaticClassTest, call_static_method_by_name_bool)
{
    ani_class cls;
    GetClassData(&cls);

    ani_boolean value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, "or", &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_v)
{
    ani_class cls;
    GetClassData(&cls);

    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "or", &value, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_A)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;
    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "or", &value, args), ANI_OK);
    ASSERT_EQ(value, ANI_TRUE);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_null_class)
{
    ani_boolean value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, nullptr, "or", &value, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_boolean value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, nullptr, &value, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_boolean value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, "aa", &value, ANI_TRUE, ANI_FALSE),
              ANI_NOT_FOUND);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Boolean(env_, cls, "or", nullptr, ANI_TRUE, ANI_FALSE),
              ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_v_null_class)
{
    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(nullptr, "or", &value, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_v_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, nullptr, &value, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_v_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "aa", &value, ANI_TRUE, ANI_FALSE), ANI_NOT_FOUND);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_v_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "or", nullptr, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_A_null_class)
{
    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(nullptr, "or", &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_A_null_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_A_error_name)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "aa", &value, args), ANI_NOT_FOUND);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_A_null_result)
{
    ani_class cls;
    GetClassData(&cls);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "or", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticClassTest, call_static_method_by_name_bool_A_null_args)
{
    ani_class cls;
    GetClassData(&cls);
    ani_boolean value;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean_A(cls, "or", &value, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)