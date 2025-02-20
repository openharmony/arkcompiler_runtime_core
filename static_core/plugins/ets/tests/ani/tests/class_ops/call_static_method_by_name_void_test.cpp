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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
namespace ark::ets::ani::testing {

class CallStaticMethodTest : public AniTest {
public:
    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
};

TEST_F(CallStaticMethodTest, call_static_method_by_name_void_a)
{
    ani_class cls;
    GetMethodData(&cls);

    ani_value args[2];
    ani_int arg1 = 2;
    ani_int arg2 = 3;
    args[0].i = arg1;
    args[1].i = arg2;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "voidMethod", args), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", &sum), ANI_OK);
    ASSERT_EQ(sum, args[0].i + args[1].i);
}

TEST_F(CallStaticMethodTest, call_static_method_by_name_void_v)
{
    ani_class cls;
    GetMethodData(&cls);
    ani_int a = 4;
    ani_int b = 5;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "voidMethod", a, b), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallStaticMethodTest, call_static_method_by_name_void)
{
    ani_class cls;
    GetMethodData(&cls);
    ani_int a = 6;
    ani_int b = 7;

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Void(env_, cls, "voidMethod", a, b), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallStaticMethodTest, call_static_method_by_name_void_v_invalid_name)
{
    ani_class cls;
    GetMethodData(&cls);
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, nullptr, a, b), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "sum_not_exist", a, b), ANI_NOT_FOUND);
}

TEST_F(CallStaticMethodTest, call_static_method_by_name_void_v_invalid_class)
{
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(nullptr, "voidMethod", a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_by_name_void_a_invalid_name)
{
    ani_class cls;
    GetMethodData(&cls);
    ani_value args[2];
    ani_int arg1 = 2;
    ani_int arg2 = 3;
    args[0].i = arg1;
    args[1].i = arg2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, nullptr, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "sum_not_exist", args), ANI_NOT_FOUND);
}

TEST_F(CallStaticMethodTest, call_static_method_by_name_void_a_invalid_class)
{
    ani_value args[2];
    ani_int arg1 = 2;
    ani_int arg2 = 3;
    args[0].i = arg1;
    args[1].i = arg2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(nullptr, "voidMethod", args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_by_name_void_a_invalid_args)
{
    ani_class cls;
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "voidMethod", nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)