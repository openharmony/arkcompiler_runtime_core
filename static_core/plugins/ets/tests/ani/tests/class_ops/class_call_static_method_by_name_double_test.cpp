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

class ClassCallStaticMethodByNameDoubleTest : public AniTest {
public:
    static constexpr ani_double VAL1 = 1.5;
    static constexpr ani_double VAL2 = 2.5;
    static constexpr size_t ARG_COUNT = 2U;

    void GetMethodData(ani_class *clsResult)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lclass_call_static_method_by_name_double_test/Operations;", &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);
        *clsResult = cls;
    }
    void TestFuncV(ani_class cls, const char *name, ani_double *value, ...)
    {
        va_list args {};
        va_start(args, value);
        ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_V(cls, name, "DD:D", value, args), ANI_OK);
        va_end(args);
    }
};

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->c_api->Class_CallStaticMethodByName_Double(env_, cls, "sum", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_v)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "sum", nullptr, &sum, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum", nullptr, &sum, args), ANI_OK);
    ASSERT_EQ(sum, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_null_class)
{
    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(nullptr, "sum", nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_double_by_name_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, nullptr, nullptr, &sum, VAL1, VAL2), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "sum_not_exist", nullptr, &sum, VAL1, VAL2),
              ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "sum", nullptr, nullptr, VAL1, VAL2), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_class)
{
    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(nullptr, "sum", nullptr, &sum, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_name)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, nullptr, nullptr, &sum, args), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum_not_exist", nullptr, &sum, args), ANI_NOT_FOUND);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_result)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum", nullptr, nullptr, args), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_A_null_args)
{
    ani_class cls {};
    GetMethodData(&cls);

    ani_double sum = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "sum", nullptr, &sum, nullptr), ANI_INVALID_ARGS);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_double_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "DD:D", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "DD:D", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_2)
{
    ani_namespace nb {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_double_test/nb;", &nb), ANI_OK);
    ani_namespace nc {};
    ASSERT_EQ(env_->Namespace_FindNamespace(nb, "Lnc;", &nc), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(nc, "LA;", &cls), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "DD:D", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "DD:D", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_double_test/na;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);

    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "DD:D", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL1 + VAL2);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "DD:D", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL1 + VAL2);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL1 + VAL2);

    ani_int value2 = 0;
    const ani_int value3 = 4;
    const ani_int value4 = 7;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "funcA", "II:I", &value2, value3, value4), ANI_OK);
    ASSERT_EQ(value2, value4 - value3);
}

TEST_F(ClassCallStaticMethodByNameDoubleTest, call_static_method_by_name_double_combine_scenes_4)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lclass_call_static_method_by_name_double_test/nd;", &ns), ANI_OK);
    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);
    ani_double value = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double(cls, "funcA", "DD:D", &value, VAL1, VAL2), ANI_OK);
    ASSERT_EQ(value, VAL2 - VAL1);

    ani_value args[ARG_COUNT];
    args[0U].d = VAL1;
    args[1U].d = VAL2;
    ani_double valueA = 0.0;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Double_A(cls, "funcA", "DD:D", &valueA, args), ANI_OK);
    ASSERT_EQ(valueA, VAL2 - VAL1);

    ani_double valueV = 0.0;
    TestFuncV(cls, "funcA", &valueV, VAL1, VAL2);
    ASSERT_EQ(valueV, VAL2 - VAL1);
}
}  // namespace ark::ets::ani::testing
// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
