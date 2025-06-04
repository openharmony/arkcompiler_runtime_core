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

class ClassCallStaticMethodByNameVoidTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 5;
    static constexpr ani_int VAL2 = 6;

    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_void_test/Operations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ...)
    {
        va_list args {};
        va_start(args, name);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_V(cls, name, "II:V", args), ANI_OK);
        va_end(args);
    }
    void GetCount(ani_class cls, ani_int *value)
    {
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", ":I", value), ANI_OK);
    }
};

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_a)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[2];
    args[0].i = VAL1;
    args[1].i = VAL2;

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "voidMethod", nullptr, args), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", nullptr, &sum), ANI_OK);
    ASSERT_EQ(sum, args[0].i + args[1].i);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int sum = 0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "voidMethod", nullptr, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", nullptr, &sum), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_int sum = 0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Void(env_, cls, "voidMethod", nullptr, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "getCount", nullptr, &sum), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_v_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, nullptr, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "sum_not_exist", nullptr, VAL1, VAL2), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_v_invalid_class)
{
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(nullptr, "voidMethod", nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_a_invalid_name)
{
    ani_class cls {};
    GetMethodData(&cls);
    ani_value args[2];
    args[0].i = VAL1;
    args[1].i = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, nullptr, nullptr, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "sum_not_exist", nullptr, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_a_invalid_class)
{
    ani_value args[2];
    args[0].i = VAL1;
    args[1].i = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(nullptr, "voidMethod", nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_a_invalid_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "voidMethod", nullptr, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_combine_scenes_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_void_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "funcA", "II:V", VAL1, VAL2), ANI_OK);
    ani_int value = 0;
    GetCount(cls, &value);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "funcA", "II:V", args), ANI_OK);
    ani_int valueA = 0;
    GetCount(cls, &valueA);
    ASSERT_EQ(valueA, value + VAL1 + VAL2);

    TestFuncV(cls, "funcA", VAL1, VAL2);
    ani_int valueV = 0;
    GetCount(cls, &valueV);
    ASSERT_EQ(valueV, valueA + VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_combine_scenes_2)
{
    ani_namespace nb {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_void_test/nb;", &nb), ANI_OK);
    ani_namespace nc {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nb, "Lnc;", &nc), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(nc, "LA;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "funcA", "II:V", VAL1, VAL2), ANI_OK);
    ani_int value = 0;
    GetCount(cls, &value);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "funcA", "II:V", args), ANI_OK);
    ani_int valueA = 0;
    GetCount(cls, &valueA);
    ASSERT_EQ(valueA, value + VAL1 + VAL2);

    TestFuncV(cls, "funcA", VAL1, VAL2);
    ani_int valueV = 0;
    GetCount(cls, &valueV);
    ASSERT_EQ(valueV, valueA + VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_combine_scenes_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_void_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "funcA", "II:V", VAL1, VAL2), ANI_OK);
    ani_int value = 0;
    GetCount(cls, &value);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "funcA", "II:V", args), ANI_OK);
    ani_int valueA = 0;
    GetCount(cls, &valueA);
    ASSERT_EQ(valueA, value + VAL1 + VAL2);

    TestFuncV(cls, "funcA", VAL1, VAL2);
    ani_int valueV = 0;
    GetCount(cls, &valueV);
    ASSERT_EQ(valueV, valueA + VAL1 + VAL2);

    ani_short valueS = 0;
    const ani_int value1 = 4;
    const ani_int value2 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Short(cls, "funcA", "SS:S", &valueS, value1, value2), ANI_OK);
    ASSERT_EQ(valueS, value2 - value1);
}

TEST_F(ClassCallStaticMethodByNameVoidTest, call_static_method_by_name_void_combine_scenes_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_void_test/nd;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "funcA", "II:V", VAL1, VAL2), ANI_OK);
    ani_int value = 0;
    GetCount(cls, &value);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[2U];
    args[0U].l = VAL1;
    args[1U].l = VAL2;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void_A(cls, "funcA", "II:V", args), ANI_OK);
    ani_int valueA = 0;
    GetCount(cls, &valueA);
    ASSERT_EQ(valueA, value + VAL2 - VAL1);

    TestFuncV(cls, "funcA", VAL1, VAL2);
    ani_int valueV = 0;
    GetCount(cls, &valueV);
    ASSERT_EQ(valueV, valueA + VAL2 - VAL1);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
