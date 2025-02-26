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

class CallStaticMethodTest : public AniTest {
public:
    void GetMethodData(ani_class *clsResult, ani_static_method *methodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method method;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "sub", "CC:C", &method), ANI_OK);
        ASSERT_NE(method, nullptr);

        *clsResult = cls;
        *methodResult = method;
    }
};

TEST_F(CallStaticMethodTest, call_static_method_char)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_char value;
    char c = 'C' - 'A';
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Char(env_, cls, method, &value, 'A', 'C'), ANI_OK);
    ASSERT_EQ(value, c);
}

TEST_F(CallStaticMethodTest, call_static_method_char_v)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_char value;
    char c = 'C' - 'A';
    ASSERT_EQ(env_->Class_CallStaticMethod_Char(cls, method, &value, 'A', 'C'), ANI_OK);
    ASSERT_EQ(value, c);
}

TEST_F(CallStaticMethodTest, call_static_method_char_A)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Char_A(cls, method, &value, args), ANI_OK);
    ASSERT_EQ(value, args[1U].c - args[0U].c);
}

TEST_F(CallStaticMethodTest, call_static_method_char_null_class)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_char value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Char(env_, nullptr, method, &value, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_null_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_char value;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Char(env_, cls, nullptr, &value, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_null_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Char(env_, cls, method, nullptr, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_V_null_class)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Char(nullptr, method, &value, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_V_null_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Char(cls, nullptr, &value, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_v_null_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ASSERT_EQ(env_->Class_CallStaticMethod_Char(cls, method, nullptr, 'A', 'C'), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_A_null_class)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Char_A(nullptr, method, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_A_null_method)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Char_A(cls, nullptr, &value, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_A_null_result)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_value args[2U];
    args[0U].c = 'A';
    args[1U].c = 'C';

    ASSERT_EQ(env_->Class_CallStaticMethod_Char_A(cls, method, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_char_A_null_args)
{
    ani_class cls;
    ani_static_method method;
    GetMethodData(&cls, &method);

    ani_char value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Char_A(cls, method, &value, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)