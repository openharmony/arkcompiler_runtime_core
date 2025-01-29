/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

class CallStaticMethodTest : public AniTest {
public:
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_GetStaticMethod(cls, "or", "ZZ:Z", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }
};

TEST_F(CallStaticMethodTest, call_static_method_bool)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Boolean(env_, cls, method, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_v)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &result, args), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_null_class)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(nullptr, method, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_null_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, nullptr, &result, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_null_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, nullptr, ANI_TRUE, ANI_FALSE), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_class)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(nullptr, method, &result, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, nullptr, &result, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].z = ANI_TRUE;
    args[1U].z = ANI_FALSE;

    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_bool_A_null_args)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean_A(cls, method, &result, nullptr), ANI_INVALID_ARGS);
}

// NOTE: disabled due to #22193
TEST_F(CallStaticMethodTest, DISABLED_call_static_method_bool_overflow)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_boolean result;
    ASSERT_EQ(env_->Class_CallStaticMethod_Boolean(cls, method, &result, 10U, 20U), ANI_OK);
    ASSERT_EQ(result, ANI_FALSE);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
