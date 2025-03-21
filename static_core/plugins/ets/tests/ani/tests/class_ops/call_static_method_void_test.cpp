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
    static constexpr ani_int VAL1 = 5U;
    static constexpr ani_int VAL2 = 6U;
    static constexpr ani_double VAL3 = 4.5;
    static constexpr ani_double VAL4 = 7.5;
    static constexpr size_t ARG_COUNT = 2U;
    void GetMethodData(ani_class *clsResult, ani_static_method *voidMethodResult, ani_static_method *getMethodResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lcall_static_method_void_test/Operations;", &cls), ANI_OK);
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
    void GetCount(ani_class cls, ani_static_method getMethod, ani_int *value)
    {
        ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, getMethod, value), ANI_OK);
    }
};

TEST_F(CallStaticMethodTest, call_static_method_void)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = VAL1;
    ani_int b = VAL2;

    ani_int sum;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Void(env_, cls, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallStaticMethodTest, call_static_method_void_v)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = VAL1;
    ani_int b = VAL2;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, voidMethod, a, b), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, a + b);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    ani_int sum;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, voidMethod, args), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethod_Int(cls, getMethod, &sum), ANI_OK);
    ASSERT_EQ(sum, args[0U].i + args[1U].i);
}

TEST_F(CallStaticMethodTest, call_static_method_void_null_class)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = VAL1;
    ani_int b = VAL2;

    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Void(env_, nullptr, voidMethod, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_null_method)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = VAL1;
    ani_int b = VAL2;

    ASSERT_EQ(env_->c_api->Class_CallStaticMethod_Void(env_, cls, nullptr, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_v_null_class)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = VAL1;
    ani_int b = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void(nullptr, voidMethod, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_v_null_method)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_int a = VAL1;
    ani_int b = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, nullptr, a, b), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A_null_class)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(nullptr, voidMethod, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A_null_method)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);
    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_A_null_args)
{
    ani_class cls {};
    ani_static_method voidMethod;
    ani_static_method getMethod;
    GetMethodData(&cls, &voidMethod, &getMethod);

    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, voidMethod, nullptr), ANI_INVALID_ARGS);
}

TEST_F(CallStaticMethodTest, call_static_method_void_combine_scenes_1)
{
    ani_class clsA {};
    ASSERT_EQ(env_->FindClass("Lcall_static_method_void_test/A;", &clsA), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "funcA", "II:V", &methodA), ANI_OK);
    ani_static_method getMethodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsA, "getCount", nullptr, &getMethodA), ANI_OK);
    ASSERT_NE(getMethodA, nullptr);

    ani_class clsB {};
    ASSERT_EQ(env_->FindClass("Lcall_static_method_void_test/B;", &clsB), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "funcB", "II:V", &methodB), ANI_OK);
    ani_static_method getMethodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(clsB, "getCount", nullptr, &getMethodB), ANI_OK);
    ASSERT_NE(getMethodB, nullptr);

    ani_int valueA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(clsA, methodA, VAL1, VAL2), ANI_OK);
    GetCount(clsA, getMethodA, &valueA);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_int valueB;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(clsB, methodB, VAL1, VAL2), ANI_OK);
    GetCount(clsB, getMethodB, &valueB);
    ASSERT_EQ(valueB, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueAA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(clsA, methodA, args), ANI_OK);
    GetCount(clsA, getMethodA, &valueAA);
    ASSERT_EQ(valueAA, valueA + VAL1 + VAL2);

    ani_int valueBA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(clsB, methodB, args), ANI_OK);
    GetCount(clsB, getMethodB, &valueBA);
    ASSERT_EQ(valueBA, valueB + VAL2 - VAL1);
}

TEST_F(CallStaticMethodTest, call_static_method_void_combine_scenes_2)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lcall_static_method_void_test/A;", &cls), ANI_OK);
    ani_static_method methodA;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "II:V", &methodA), ANI_OK);
    ani_static_method methodB;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcA", "DD:D", &methodB), ANI_OK);
    ani_static_method getMethod;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &getMethod), ANI_OK);
    ASSERT_NE(getMethod, nullptr);

    ani_int value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, methodA, VAL1, VAL2), ANI_OK);
    GetCount(cls, getMethod, &value);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, methodA, args), ANI_OK);
    GetCount(cls, getMethod, &valueA);
    ASSERT_EQ(valueA, value + VAL1 + VAL2);

    ani_double value2;
    ASSERT_EQ(env_->Class_CallStaticMethod_Double(cls, methodB, &value2, VAL3, VAL4), ANI_OK);
    ASSERT_EQ(value2, VAL3 + VAL4);
}

TEST_F(CallStaticMethodTest, call_static_method_void_combine_scenes_3)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lcall_static_method_void_test/A;", &cls), ANI_OK);
    ani_static_method method;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "funcB", "II:V", &method), ANI_OK);
    ani_static_method getMethod;
    ASSERT_EQ(env_->Class_FindStaticMethod(cls, "getCount", nullptr, &getMethod), ANI_OK);
    ASSERT_NE(getMethod, nullptr);

    ani_int value;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void(cls, method, VAL1, VAL2), ANI_OK);
    GetCount(cls, getMethod, &value);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].i = VAL1;
    args[1U].i = VAL2;
    ani_int valueA;
    ASSERT_EQ(env_->Class_CallStaticMethod_Void_A(cls, method, args), ANI_OK);
    GetCount(cls, getMethod, &valueA);
    ASSERT_EQ(valueA, value + VAL1 + VAL2);
}
}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
