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
    void GetMethodData(ani_class *clsResult, ani_static_method *voidMethodResult, ani_static_method *getMethodResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("LOperations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_static_method getMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &getMethod), ANI_OK);
        ASSERT_NE(getMethod, nullptr);

        ani_static_method voidMethod;
        ASSERT_EQ(env_->Class_FindStaticMethod(cls, "voidMethod", nullptr, &voidMethod), ANI_OK);
        ASSERT_NE(voidMethod, nullptr);

        *clsResult = cls;
        *voidMethodResult = voidMethod;
        *getMethodResult = getMethod;
    }
};

TEST_F(CallStaticMethodTest, call_static_method_void)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Void(env_, cls, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallStaticMethodTest, call_static_method_void_v)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_value args[2U];
    args[0U].i = 6U;
    args[1U].i = 7U;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, voidMethod, args), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, args[0U].i + args[1U].i);
}

TEST_F(CallStaticMethodTest, call_static_method_void_null_class)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Void(env_, nullptr, voidMethod, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_null_method)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Void(env_, cls, nullptr, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_v_null_class)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void(nullptr, voidMethod, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_v_null_method)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = 6;
    ani_int b = 7;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, nullptr, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A_null_class)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_value args[2U];
    args[0U].i = 6U;
    args[1U].i = 7U;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(nullptr, voidMethod, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A_null_method)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_value args[2U];
    args[0U].i = 6U;
    args[1U].i = 7U;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A_null_args)
{
    ani_class cls;
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, voidMethod, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
